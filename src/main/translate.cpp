/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Logger/logger.h>
#include <DirManager/dirman.h>
#include <AppPath/app_path.h>
#include <Utils/files.h>
#include <Utils/strings.h>
#include <json/json_rwops_input.hpp>
#include <json/json.hpp>
#include <fmt_format_ne.h>

#ifndef SDL_SDL_STDINC_H
#include "sdl_proxy/sdl_stdinc.h"
#endif

#ifndef GLOBALS_H
#include "globals.h"
#endif

#include "fontman/font_manager.h"
#include "core/language.h"

#include "main/translate.h"
#include "main/menu_main.h"
#include "main/game_info.h"
#include "main/outro_loop.h"
#include "main/game_strings.h"
#include "main/hints.h"

#include "controls.h"
#include "config.h"
#include "control/controls_strings.h"

#include "editor/editor_strings.h"
#include "editor/editor_custom.h"

#include "script/luna/luna.h"


enum class PluralRules
{
    SingularOnly,
    OneIsSingular,
    Slavic,
};

static PluralRules s_CurrentPluralRules = PluralRules::OneIsSingular;

const std::string& LanguageFormatSlavic(int number, const std::string& singular, const std::string& dual, const std::string& plural)
{
    if(((number % 100) != 11) && ((number % 10) == 1))
        return singular;
    else if((number % 10) >= 2 && (number % 10) <= 4 && (number % 100 < 10 || number % 100 > 20))
        return dual;
    else
        return plural;
}

const std::string& LanguageFormatNumber(int number, const std::string& singular, const std::string& dual, const std::string& plural)
{
    // Ensure it's always positive number
    number = SDL_abs(number);

    if(s_CurrentPluralRules == PluralRules::OneIsSingular)
        return (number == 1) ? singular : plural;
    else if(s_CurrentPluralRules == PluralRules::Slavic)
        return LanguageFormatSlavic(number, singular, dual, plural);
    else // if(s_CurrentPluralRules == PluralRules::SingularOnly)
        return singular;
}

#ifndef THEXTECH_DISABLE_LANG_TOOLS
static void setJsonValue(nlohmann::ordered_json &j, const std::string &key, const std::string &value)
{
    auto dot = key.find(".");
    if(dot == std::string::npos)
    {
        j[key] = value;
        return;
    }

    std::string subKey = key.substr(0, dot);
    setJsonValue(j[subKey], key.substr(dot + 1), value);
}

static bool setJsonValueIfNotExist(nlohmann::ordered_json &j, const std::string &key, const std::string &value, bool noBlank)
{
    auto dot = key.find(".");
    if(dot == std::string::npos)
    {
        if(!j.contains(key) || j.is_null())
        {
            std::printf("-- ++ Added new string: %s = %s\n", key.c_str(), value.c_str());
            std::fflush(stdout);
            if(!noBlank || !value.empty())
                j[key] = value;
            else if(j.contains(key))
                j.erase(key);
            return true;
        }
        return false; // Nothing changed
    }

    std::string subKey = key.substr(0, dot);
    bool ret = setJsonValueIfNotExist(j[subKey], key.substr(dot + 1), value, noBlank);

    if(noBlank && (j[subKey].is_null() || j[subKey].empty()))
        j.erase(subKey);

    return ret;
}
#endif

static std::string getJsonValue(nlohmann::ordered_json &j, const std::string &key, const std::string &defVal)
{
    auto dot = key.find(".");
    if(dot == std::string::npos)
    {
        if(!j.contains(key) || j[key].is_null())
            return defVal;

        auto out = j.value(key, defVal);

        if(out.empty()) // For empty lines, return default value
            return defVal;

        return out;
    }

    std::string subKey = key.substr(0, dot);
    return getJsonValue(j[subKey], key.substr(dot + 1), defVal);
}

#ifndef THEXTECH_DISABLE_LANG_TOOLS
static bool saveFile(const std::string &inPath, const std::string &inData)
{
    bool ret = true;
    SDL_RWops *in = Files::open_file(inPath, "w");
    if(!in)
        return false;

    SDL_RWwrite(in, inData.c_str(), 1, inData.size());
    SDL_RWwrite(in, "\n", 1, 1);

    SDL_RWclose(in);

    return ret;
}
#endif


XTechTranslate::XTechTranslate()
{
    // List of all translatable strings of the engine
    m_engineMap.clear();

    m_engineMap.insert({"menu.main.mainPlayEpisode",        &g_mainMenu.mainPlayEpisode});
    m_engineMap.insert({"menu.main.main1PlayerGame",      &g_mainMenu.main1PlayerGame});
    m_engineMap.insert({"menu.main.mainMultiplayerGame",  &g_mainMenu.mainMultiplayerGame});
    m_engineMap.insert({"menu.main.mainBattleGame",       &g_mainMenu.mainBattleGame});
    m_engineMap.insert({"menu.main.mainEditor",           &g_mainMenu.mainEditor});
    m_engineMap.insert({"menu.main.mainOptions",          &g_mainMenu.mainOptions});
    m_engineMap.insert({"menu.main.mainExit",             &g_mainMenu.mainExit});

    m_engineMap.insert({"menu.loading",                   &g_mainMenu.loading});

    m_engineMap.insert({"languageName",                   &g_mainMenu.languageName});
    m_engineMap.insert({"pluralRules",                    &g_mainMenu.pluralRules});

    m_engineMap.insert({"menu.editor.battles",            &g_mainMenu.editorBattles});
    m_engineMap.insert({"menu.editor.newWorld",           &g_mainMenu.editorNewWorld});
    m_engineMap.insert({"menu.editor.makeFor",            &g_mainMenu.editorMakeFor});
    m_engineMap.insert({"menu.editor.errorMissingResources", &g_mainMenu.editorErrorMissingResources});
    m_engineMap.insert({"menu.editor.promptNewWorldName", &g_mainMenu.editorPromptNewWorldName});

    m_engineMap.insert({"menu.game.gameNoEpisodesToPlay", &g_mainMenu.gameNoEpisodesToPlay});
    m_engineMap.insert({"menu.game.gameNoBattleLevels",   &g_mainMenu.gameNoBattleLevels});
    m_engineMap.insert({"menu.game.gameBattleRandom",     &g_mainMenu.gameBattleRandom});

    m_engineMap.insert({"menu.game.warnEpCompat",         &g_mainMenu.warnEpCompat});

    m_engineMap.insert({"menu.game.gameSlotContinue",     &g_mainMenu.gameSlotContinue});
    m_engineMap.insert({"menu.game.gameSlotNew",          &g_mainMenu.gameSlotNew});

    m_engineMap.insert({"menu.game.gameCopySave",         &g_mainMenu.gameCopySave});
    m_engineMap.insert({"menu.game.gameEraseSave",        &g_mainMenu.gameEraseSave});

    m_engineMap.insert({"menu.game.gameSourceSlot",       &g_mainMenu.gameSourceSlot});
    m_engineMap.insert({"menu.game.gameTargetSlot",       &g_mainMenu.gameTargetSlot});
    m_engineMap.insert({"menu.game.gameEraseSlot",        &g_mainMenu.gameEraseSlot});

    m_engineMap.insert({"menu.game.phraseScore",          &g_mainMenu.phraseScore});
    m_engineMap.insert({"menu.game.phraseTime",           &g_mainMenu.phraseTime});

    m_engineMap.insert({"menu.battle.errorNoLevels",      &g_mainMenu.errorBattleNoLevels});

    m_engineMap.insert({"menu.options.restartEngine",           &g_mainMenu.optionsRestartEngine});

    m_engineMap.insert({"menu.character.selectCharacter", &g_mainMenu.selectCharacter});


    m_engineMap.insert({"menu.controls.controlsTitle",     &g_mainMenu.controlsTitle});
    m_engineMap.insert({"menu.controls.controlsConnected", &g_mainMenu.controlsConnected});
    m_engineMap.insert({"menu.controls.controlsDeleteKey", &g_mainMenu.controlsDeleteKey});
    m_engineMap.insert({"menu.controls.controlsDeviceTypes", &g_mainMenu.controlsDeviceTypes});
    m_engineMap.insert({"menu.controls.controlsInUse", &g_mainMenu.controlsInUse});
    m_engineMap.insert({"menu.controls.controlsNotInUse", &g_mainMenu.controlsNotInUse});

    m_engineMap.insert({"menu.controls.wordProfiles", &g_mainMenu.wordProfiles});
    m_engineMap.insert({"menu.controls.wordButtons", &g_mainMenu.wordButtons});

    m_engineMap.insert({"menu.controls.controlsReallyDeleteProfile", &g_mainMenu.controlsReallyDeleteProfile});
    m_engineMap.insert({"menu.controls.controlsNewProfile", &g_mainMenu.controlsNewProfile});
    m_engineMap.insert({"menu.controls.caseInvalid",        &g_controlsStrings.sharedCaseInvalid});

    m_engineMap.insert({"menu.controls.profile.renameProfile",   &g_mainMenu.controlsRenameProfile});
    m_engineMap.insert({"menu.controls.profile.deleteProfile",   &g_mainMenu.controlsDeleteProfile});
    m_engineMap.insert({"menu.controls.profile.playerControls",  &g_mainMenu.controlsPlayerControls});
    m_engineMap.insert({"menu.controls.profile.cursorControls",  &g_mainMenu.controlsCursorControls});
    m_engineMap.insert({"menu.controls.profile.editorControls",  &g_mainMenu.controlsEditorControls});
    m_engineMap.insert({"menu.controls.profile.hotkeys",         &g_mainMenu.controlsHotkeys});

    m_engineMap.insert({"menu.controls.options.rumble",            &g_mainMenu.controlsOptionRumble});
    m_engineMap.insert({"menu.controls.options.batteryStatus",     &g_mainMenu.controlsOptionBatteryStatus});
    m_engineMap.insert({"menu.controls.options.maxPlayers",        &g_controlsStrings.sharedOptionMaxPlayers});

    m_engineMap.insert({"menu.controls.buttons.up",      &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Up]});
    m_engineMap.insert({"menu.controls.buttons.down",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Down]});
    m_engineMap.insert({"menu.controls.buttons.left",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Left]});
    m_engineMap.insert({"menu.controls.buttons.right",   &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Right]});
    m_engineMap.insert({"menu.controls.buttons.jump",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Jump]});
    m_engineMap.insert({"menu.controls.buttons.run",     &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Run]});
    m_engineMap.insert({"menu.controls.buttons.altJump", &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::AltJump]});
    m_engineMap.insert({"menu.controls.buttons.altRun",  &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::AltRun]});
    m_engineMap.insert({"menu.controls.buttons.start",   &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Start]});
    m_engineMap.insert({"menu.controls.buttons.drop",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Drop]});

    m_engineMap.insert({"menu.controls.cursor.up",        &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorUp]});
    m_engineMap.insert({"menu.controls.cursor.down",      &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorDown]});
    m_engineMap.insert({"menu.controls.cursor.left",      &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorLeft]});
    m_engineMap.insert({"menu.controls.cursor.right",     &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorRight]});
    m_engineMap.insert({"menu.controls.cursor.primary",   &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Primary]});
    m_engineMap.insert({"menu.controls.cursor.secondary", &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Secondary]});
    m_engineMap.insert({"menu.controls.cursor.tertiary",  &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Tertiary]});

    m_engineMap.insert({"menu.controls.editor.scrollUp",      &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollUp]});
    m_engineMap.insert({"menu.controls.editor.scrollDown",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollDown]});
    m_engineMap.insert({"menu.controls.editor.scrollLeft",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollLeft]});
    m_engineMap.insert({"menu.controls.editor.scrollRight",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollRight]});
    m_engineMap.insert({"menu.controls.editor.fastScroll",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::FastScroll]});
    m_engineMap.insert({"menu.controls.editor.modeSelect",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ModeSelect]});
    m_engineMap.insert({"menu.controls.editor.modeErase",     &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ModeErase]});
    m_engineMap.insert({"menu.controls.editor.prevSection",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::PrevSection]});
    m_engineMap.insert({"menu.controls.editor.nextSection",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::NextSection]});
    m_engineMap.insert({"menu.controls.editor.switchScreens", &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::SwitchScreens]});
    m_engineMap.insert({"menu.controls.editor.testPlay",      &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::TestPlay]});

#ifndef RENDER_FULLSCREEN_ALWAYS
    m_engineMap.insert({"menu.controls.hotkeys.fullscreen",  &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::Fullscreen]});
#endif
#ifdef USE_SCREENSHOTS_AND_RECS
    m_engineMap.insert({"menu.controls.hotkeys.screenshot",  &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::Screenshot]});
#endif
#ifdef PGE_ENABLE_VIDEO_REC
    m_engineMap.insert({"menu.controls.hotkeys.recordGif",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::RecordGif]});
#endif
    m_engineMap.insert({"menu.controls.hotkeys.debugInfo",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::DebugInfo]});
    m_engineMap.insert({"menu.controls.hotkeys.vanillaCam",  &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::VanillaCam]});
    m_engineMap.insert({"menu.controls.hotkeys.enterCheats", &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::EnterCheats]});
    m_engineMap.insert({"menu.controls.hotkeys.toggleHUD",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::ToggleHUD]});
    m_engineMap.insert({"menu.controls.hotkeys.legacyPause", &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::LegacyPause]});

#ifdef CONTROLS_KEYBOARD_STRINGS
    m_engineMap.insert({"menu.controls.types.keyboard",    &g_controlsStrings.nameKeyboard});
    m_engineMap.insert({"menu.controls.options.textEntryStyle",    &g_controlsStrings.keyboardOptionTextEntryStyle});
    m_engineMap.insert({"menu.controls.caseMouse",         &g_controlsStrings.caseMouse});
#endif

#if defined(CONTROLS_KEYBOARD_STRINGS) || defined(CONTROLS_JOYSTICK_STRINGS)
    m_engineMap.insert({"menu.controls.types.gamepad",     &g_controlsStrings.nameGamepad});
#endif

#if defined(CONTROLS_JOYSTICK_STRINGS)
    m_engineMap.insert({"menu.controls.phraseNewProfOldJoy",&g_controlsStrings.phraseNewProfOldJoy});
    m_engineMap.insert({"menu.controls.joystickSimpleEditor",&g_controlsStrings.joystickSimpleEditor});
    m_engineMap.insert({"menu.controls.types.oldJoystick", &g_controlsStrings.nameOldJoy});
#endif

#if defined(CONTROLS_TOUCHSCREEN_STRINGS)
    m_engineMap.insert({"menu.controls.types.touchscreen", &g_controlsStrings.nameTouchscreen});

    m_engineMap.insert({"menu.controls.caseTouch",         &g_controlsStrings.caseTouch});

    m_engineMap.insert({"menu.controls.touchscreen.option.layoutStyle",    &g_controlsStrings.touchscreenOptionLayoutStyle});
    m_engineMap.insert({"menu.controls.touchscreen.option.scaleFactor",    &g_controlsStrings.touchscreenOptionScaleFactor});
    m_engineMap.insert({"menu.controls.touchscreen.option.scaleDPad",      &g_controlsStrings.touchscreenOptionScaleDPad});
    m_engineMap.insert({"menu.controls.touchscreen.option.scaleButtons",   &g_controlsStrings.touchscreenOptionScaleButtons});
    m_engineMap.insert({"menu.controls.touchscreen.option.sStartSpacing",  &g_controlsStrings.touchscreenOptionSStartSpacing});
    m_engineMap.insert({"menu.controls.touchscreen.option.resetLayout",    &g_controlsStrings.touchscreenOptionResetLayout});
    m_engineMap.insert({"menu.controls.touchscreen.option.interfaceStyle", &g_controlsStrings.touchscreenOptionInterfaceStyle});
    m_engineMap.insert({"menu.controls.touchscreen.option.feedbackStrength", &g_controlsStrings.touchscreenOptionFeedbackStrength});
    m_engineMap.insert({"menu.controls.touchscreen.option.feedbackLength", &g_controlsStrings.touchscreenOptionFeedbackLength});
    m_engineMap.insert({"menu.controls.touchscreen.option.holdRun",        &g_controlsStrings.touchscreenOptionHoldRun});
    m_engineMap.insert({"menu.controls.touchscreen.option.showCodeButton", &g_controlsStrings.touchscreenOptionShowCodeButton});


    m_engineMap.insert({"menu.controls.touchscreen.layout.tight",          &g_controlsStrings.touchscreenLayoutTight});
    m_engineMap.insert({"menu.controls.touchscreen.layout.tinyOld",        &g_controlsStrings.touchscreenLayoutTinyOld});
    m_engineMap.insert({"menu.controls.touchscreen.layout.phoneOld",       &g_controlsStrings.touchscreenLayoutPhoneOld});
    m_engineMap.insert({"menu.controls.touchscreen.layout.longOld",        &g_controlsStrings.touchscreenLayoutLongOld});
    m_engineMap.insert({"menu.controls.touchscreen.layout.phabletOld",     &g_controlsStrings.touchscreenLayoutPhabletOld});
    m_engineMap.insert({"menu.controls.touchscreen.layout.tabletOld",      &g_controlsStrings.touchscreenLayoutTabletOld});
    m_engineMap.insert({"menu.controls.touchscreen.layout.standard",       &g_controlsStrings.touchscreenLayoutStandard});

    m_engineMap.insert({"menu.controls.touchscreen.style.actions",         &g_controlsStrings.touchscreenStyleActions});
    m_engineMap.insert({"menu.controls.touchscreen.style.ABXY",            &g_controlsStrings.touchscreenStyleABXY});
    m_engineMap.insert({"menu.controls.touchscreen.style.XODA",            &g_controlsStrings.touchscreenStyleXODA});
#endif // #if defined(CONTROLS_TOUCHSCREEN_STRINGS)

#if defined(CONTROLS_16M_STRINGS)
    m_engineMap.insert({"menu.controls.tDS.buttonA",      &g_controlsStrings.tdsButtonA});
    m_engineMap.insert({"menu.controls.tDS.buttonB",      &g_controlsStrings.tdsButtonB});
    m_engineMap.insert({"menu.controls.tDS.buttonX",      &g_controlsStrings.tdsButtonX});
    m_engineMap.insert({"menu.controls.tDS.buttonY",      &g_controlsStrings.tdsButtonY});
    m_engineMap.insert({"menu.controls.tDS.buttonL",      &g_controlsStrings.tdsButtonL});
    m_engineMap.insert({"menu.controls.tDS.buttonR",      &g_controlsStrings.tdsButtonR});
    m_engineMap.insert({"menu.controls.tDS.buttonSelect", &g_controlsStrings.tdsButtonSelect});
    m_engineMap.insert({"menu.controls.tDS.buttonStart",  &g_controlsStrings.tdsButtonStart});

    m_engineMap.insert({"menu.controls.tDS.casePen",      &g_controlsStrings.tdsCasePen});
#endif // #ifdef CONTROLS_16M_STRINGS

#if defined(CONTROLS_3DS_STRINGS)
    m_engineMap.insert({"menu.controls.tDS.buttonZL",     &g_controlsStrings.tdsButtonZL});
    m_engineMap.insert({"menu.controls.tDS.buttonZR",     &g_controlsStrings.tdsButtonZR});
    m_engineMap.insert({"menu.controls.tDS.dPad",         &g_controlsStrings.tdsDpad});
    m_engineMap.insert({"menu.controls.tDS.tStick",       &g_controlsStrings.tdsTstick});
    m_engineMap.insert({"menu.controls.tDS.cStick",       &g_controlsStrings.tdsCstick});
#endif // #ifdef CONTROLS_3DS_STRINGS

#if defined(CONTROLS_WII_STRINGS)
    m_engineMap.insert({"menu.controls.wii.typeWiimote",         &g_controlsStrings.wiiTypeWiimote});
    m_engineMap.insert({"menu.controls.wii.typeNunchuck",        &g_controlsStrings.wiiTypeNunchuck});
    m_engineMap.insert({"menu.controls.wii.typeClassic",         &g_controlsStrings.wiiTypeClassic});
    m_engineMap.insert({"menu.controls.wii.phraseNewNunchuck",   &g_controlsStrings.wiiPhraseNewNunchuck});
    m_engineMap.insert({"menu.controls.wii.phraseNewClassic",    &g_controlsStrings.wiiPhraseNewClassic});

    m_engineMap.insert({"menu.controls.wii.wiimote.dPad",        &g_controlsStrings.wiiDpad});
    m_engineMap.insert({"menu.controls.wii.wiimote.buttonA",     &g_controlsStrings.wiiButtonA});
    m_engineMap.insert({"menu.controls.wii.wiimote.buttonB",     &g_controlsStrings.wiiButtonB});
    m_engineMap.insert({"menu.controls.wii.wiimote.buttonMinus", &g_controlsStrings.wiiButtonMinus});
    m_engineMap.insert({"menu.controls.wii.wiimote.buttonPlus",  &g_controlsStrings.wiiButtonPlus});
    m_engineMap.insert({"menu.controls.wii.wiimote.buttonHome",  &g_controlsStrings.wiiButtonHome});
    m_engineMap.insert({"menu.controls.wii.wiimote.button2",     &g_controlsStrings.wiiButton2});
    m_engineMap.insert({"menu.controls.wii.wiimote.button1",     &g_controlsStrings.wiiButton1});
    m_engineMap.insert({"menu.controls.wii.wiimote.shake",       &g_controlsStrings.wiiShake});
    m_engineMap.insert({"menu.controls.wii.wiimote.caseIR",      &g_controlsStrings.wiiCaseIR});

    m_engineMap.insert({"menu.controls.wii.nunchuck.prefixN",    &g_controlsStrings.wiiPrefixNunchuck});
    m_engineMap.insert({"menu.controls.wii.nunchuck.buttonZ",    &g_controlsStrings.wiiButtonZ});
    m_engineMap.insert({"menu.controls.wii.nunchuck.buttonC",    &g_controlsStrings.wiiButtonC});

    m_engineMap.insert({"menu.controls.wii.classic.lStick",      &g_controlsStrings.wiiLStick});
    m_engineMap.insert({"menu.controls.wii.classic.rStick",      &g_controlsStrings.wiiRStick});
    m_engineMap.insert({"menu.controls.wii.classic.buttonZL",    &g_controlsStrings.wiiButtonZL});
    m_engineMap.insert({"menu.controls.wii.classic.buttonZR",    &g_controlsStrings.wiiButtonZR});
    m_engineMap.insert({"menu.controls.wii.classic.buttonLT",    &g_controlsStrings.wiiButtonLT});
    m_engineMap.insert({"menu.controls.wii.classic.buttonRT",    &g_controlsStrings.wiiButtonRT});
    m_engineMap.insert({"menu.controls.wii.classic.buttonX",     &g_controlsStrings.wiiButtonX});
    m_engineMap.insert({"menu.controls.wii.classic.buttonY",     &g_controlsStrings.wiiButtonY});

#endif // #ifdef CONTROLS_WII_STRINGS

    m_engineMap.insert({"menu.wordNo",         &g_mainMenu.wordNo});
    m_engineMap.insert({"menu.wordYes",        &g_mainMenu.wordYes});
    m_engineMap.insert({"menu.caseNone",       &g_mainMenu.caseNone});
    m_engineMap.insert({"menu.wordOn",         &g_mainMenu.wordOn});
    m_engineMap.insert({"menu.wordOff",        &g_mainMenu.wordOff});
    m_engineMap.insert({"menu.wordShow",       &g_mainMenu.wordShow});
    m_engineMap.insert({"menu.wordHide",       &g_mainMenu.wordHide});
    m_engineMap.insert({"menu.wordPlayer",     &g_mainMenu.wordPlayer});
    m_engineMap.insert({"menu.wordProfile",    &g_mainMenu.wordProfile});
    m_engineMap.insert({"menu.wordBack",       &g_mainMenu.wordBack});
    m_engineMap.insert({"menu.wordResume",     &g_mainMenu.wordResume});
    m_engineMap.insert({"menu.wordWaiting",    &g_mainMenu.wordWaiting});

    m_engineMap.insert({"menu.abbrevMilliseconds", &g_mainMenu.abbrevMilliseconds});


    m_engineMap.insert({"outro.gameCredits",           &g_outroScreen.gameCredits});
    m_engineMap.insert({"outro.engineCredits",         &g_outroScreen.engineCredits});
    m_engineMap.insert({"outro.originalBy",            &g_outroScreen.originalBy});
    m_engineMap.insert({"outro.nameAndrewSpinks",      &g_outroScreen.nameAndrewSpinks});
    m_engineMap.insert({"outro.cppPortDevelopers",     &g_outroScreen.cppPortDevelopers});
    m_engineMap.insert({"outro.nameVitalyNovichkov",   &g_outroScreen.nameVitalyNovichkov});
    m_engineMap.insert({"outro.qualityControl",        &g_outroScreen.qualityControl});
    m_engineMap.insert({"outro.psVitaPortBy",          &g_outroScreen.psVitaPortBy});
    m_engineMap.insert({"outro.levelDesign",           &g_outroScreen.levelDesign});
    m_engineMap.insert({"outro.customSprites",         &g_outroScreen.customSprites});
    m_engineMap.insert({"outro.specialThanks",         &g_outroScreen.specialThanks});


    m_engineMap.insert({"game.msgbox.sysInfoTitle",                &g_gameStrings.msgBoxTitleInfo});
    m_engineMap.insert({"game.msgbox.sysInfoWarning",              &g_gameStrings.msgBoxTitleWarning});
    m_engineMap.insert({"game.msgbox.sysInfoError",                &g_gameStrings.msgBoxTitleError});

    m_engineMap.insert({"game.error.openFileFailed",               &g_gameStrings.errorOpenFileFailed});
    m_engineMap.insert({"game.error.errorInvalidEnterWarp",        &g_gameStrings.errorInvalidEnterWarp});
    m_engineMap.insert({"game.error.errorNoStartPoint",            &g_gameStrings.errorNoStartPoint});

    m_engineMap.insert({"game.error.warpNeedStarCount",            &g_gameStrings.warpNeedStarCount});

    m_engineMap.insert({"game.message.scanningLevels",             &g_gameStrings.messageScanningLevels});
    m_engineMap.insert({"game.format.minutesSeconds",              &g_gameStrings.formatMinutesSeconds});

    m_engineMap.insert({"game.pause.continue",             &g_gameStrings.pauseItemContinue});
    m_engineMap.insert({"game.pause.restartLevel",         &g_gameStrings.pauseItemRestartLevel});
    m_engineMap.insert({"game.pause.resetCheckpoints",     &g_gameStrings.pauseItemResetCheckpoints});
    m_engineMap.insert({"game.pause.quitTesting",          &g_gameStrings.pauseItemQuitTesting});
    m_engineMap.insert({"game.pause.returnToEditor",       &g_gameStrings.pauseItemReturnToEditor});
    m_engineMap.insert({"game.pause.playerSetup",          &g_gameStrings.pauseItemPlayerSetup});
    m_engineMap.insert({"game.pause.enterCode",            &g_gameStrings.pauseItemEnterCode});
    m_engineMap.insert({"game.pause.saveAndContinue",      &g_gameStrings.pauseItemSaveAndContinue});
    m_engineMap.insert({"game.pause.saveAndQuit",          &g_gameStrings.pauseItemSaveAndQuit});
    m_engineMap.insert({"game.pause.quit",                 &g_gameStrings.pauseItemQuit});

    m_engineMap.insert({"game.connect.reconnectTitle",            &g_gameStrings.connectReconnectTitle});

    m_engineMap.insert({"game.connect.phrasePressAButton",        &g_gameStrings.connectPressAButton});

    m_engineMap.insert({"game.connect.phraseTestProfile",        &g_gameStrings.connectTestProfile});
    m_engineMap.insert({"game.connect.phraseHoldStart",           &g_gameStrings.connectHoldStart});
    m_engineMap.insert({"game.connect.wordDisconnect",            &g_gameStrings.connectDisconnect});

    m_engineMap.insert({"game.connect.phraseForceResume",         &g_gameStrings.connectForceResume});
    m_engineMap.insert({"game.connect.phraseDropPX",              &g_gameStrings.connectDropPX});

    m_engineMap.insert({"game.connect.phraseWaitingForInput",     &g_gameStrings.connectWaitingForInputDevice});
    m_engineMap.insert({"game.connect.splitPressSelect_1",        &g_gameStrings.connectPressSelectForControlsOptions_P1});
    m_engineMap.insert({"game.connect.splitPressSelect_2",        &g_gameStrings.connectPressSelectForControlsOptions_P2});

    m_engineMap.insert({"game.connect.phraseDropMe",              &g_gameStrings.connectDropMe});

#ifdef THEXTECH_ENABLE_EDITOR
    m_engineMap.insert({"editor.block.pickContents",       &g_editorStrings.pickBlockContents});

    m_engineMap.insert({"editor.block.letterWidth",        &g_editorStrings.blockLetterWidth});
    m_engineMap.insert({"editor.block.letterHeight",       &g_editorStrings.blockLetterHeight});
    m_engineMap.insert({"editor.block.canBreak",           &g_editorStrings.blockCanBreak});
    m_engineMap.insert({"editor.block.canBreakTooltip",    &g_editorStrings.blockTooltipCanBreak});
    m_engineMap.insert({"editor.block.slick",              &g_editorStrings.blockSlick});
    m_engineMap.insert({"editor.block.invis",              &g_editorStrings.blockInvis});
    m_engineMap.insert({"editor.block.inside",             &g_editorStrings.blockInside});

    m_engineMap.insert({"editor.warp.title",               &g_editorStrings.warpTitle});
    m_engineMap.insert({"editor.warp.placing",             &g_editorStrings.warpPlacing});
    m_engineMap.insert({"editor.warp.in",                  &g_editorStrings.warpIn});
    m_engineMap.insert({"editor.warp.out",                 &g_editorStrings.warpOut});
    m_engineMap.insert({"editor.warp.dir",                 &g_editorStrings.warpDir});
    m_engineMap.insert({"editor.warp.twoWay",              &g_editorStrings.warpTwoWay});
    m_engineMap.insert({"editor.warp.style.style",         &g_editorStrings.warpStyle});
    m_engineMap.insert({"editor.warp.style.pipe",          &g_editorStrings.warpStylePipe});
    m_engineMap.insert({"editor.warp.style.door",          &g_editorStrings.warpStyleDoor});
    m_engineMap.insert({"editor.warp.style.blipInstant",   &g_editorStrings.warpStyleBlipInstant});
    m_engineMap.insert({"editor.warp.style.portal",        &g_editorStrings.warpStylePortal});
    m_engineMap.insert({"editor.warp.effect",              &g_editorStrings.warpEffect});
    m_engineMap.insert({"editor.warp.allow",               &g_editorStrings.warpAllow});
    m_engineMap.insert({"editor.warp.item",                &g_editorStrings.warpItem});
    m_engineMap.insert({"editor.warp.ride",                &g_editorStrings.warpRide});
    m_engineMap.insert({"editor.warp.cannonExit",          &g_editorStrings.warpCannonExit});
    m_engineMap.insert({"editor.warp.speed",               &g_editorStrings.warpSpeed});
    m_engineMap.insert({"editor.warp.needStarCount",       &g_editorStrings.warpNeedStarCount});
    m_engineMap.insert({"editor.warp.needKey",             &g_editorStrings.warpNeedKey});
    m_engineMap.insert({"editor.warp.needFloor",           &g_editorStrings.warpNeedFloor});
    m_engineMap.insert({"editor.warp.starLockMessage",     &g_editorStrings.warpStarLockMessage});
    m_engineMap.insert({"editor.warp.toMap",               &g_editorStrings.warpToMap});
    m_engineMap.insert({"editor.warp.lvlWarp",             &g_editorStrings.warpLvlWarp});
    m_engineMap.insert({"editor.warp.target",              &g_editorStrings.warpTarget});
    m_engineMap.insert({"editor.warp.to",                  &g_editorStrings.warpTo});
    m_engineMap.insert({"editor.warp.showStartScene",      &g_editorStrings.warpShowStartScene});
    m_engineMap.insert({"editor.warp.showStarCount",       &g_editorStrings.warpShowStarCount});

    m_engineMap.insert({"editor.water.title",              &g_editorStrings.waterTitle});

    m_engineMap.insert({"editor.npc.inContainer",          &g_editorStrings.npcInContainer});
    m_engineMap.insert({"editor.npc.inertNice",            &g_editorStrings.npcInertNice});
    m_engineMap.insert({"editor.npc.stuckStop",            &g_editorStrings.npcStuckStop});
    m_engineMap.insert({"editor.npc.props.active",         &g_editorStrings.npcPropertyActive});
    m_engineMap.insert({"editor.npc.props.attachSurface",  &g_editorStrings.npcPropertyAttachSurface});
    m_engineMap.insert({"editor.npc.props.facing",         &g_editorStrings.npcPropertyFacing});
    m_engineMap.insert({"editor.npc.abbrevGen",            &g_editorStrings.npcAbbrevGen});

    m_engineMap.insert({"editor.npc.ai.aiIs",              &g_editorStrings.npcAiIs});
    m_engineMap.insert({"editor.npc.ai.target",            &g_editorStrings.npcAiTarget});
    m_engineMap.insert({"editor.npc.ai.jump",              &g_editorStrings.npcAiJump});
    m_engineMap.insert({"editor.npc.ai.leap",              &g_editorStrings.npcAiLeap});
    m_engineMap.insert({"editor.npc.ai.swim",              &g_editorStrings.npcAiSwim});
    m_engineMap.insert({"editor.npc.ai.LR",                &g_editorStrings.npcAiLR});
    m_engineMap.insert({"editor.npc.ai.UD",                &g_editorStrings.npcAiUD});

    m_engineMap.insert({"editor.npc.ai.headerCustomAi",    &g_editorStrings.npcCustomAi});

    m_engineMap.insert({"editor.npc.ai.use1_0Ai",          &g_editorStrings.npcUse1_0Ai});
    m_engineMap.insert({"editor.npc.tooltipExpandSection", &g_editorStrings.npcTooltipExpandSection});

    m_engineMap.insert({"editor.npc.gen.header",           &g_editorStrings.npcGenHeader});
    m_engineMap.insert({"editor.npc.gen.direction",        &g_editorStrings.npcGenDirection});
    m_engineMap.insert({"editor.npc.gen.effectIs",         &g_editorStrings.npcGenEffectIs});
    m_engineMap.insert({"editor.npc.gen.effectWarp",       &g_editorStrings.npcGenEffectWarp});
    m_engineMap.insert({"editor.npc.gen.effectShoot",      &g_editorStrings.npcGenEffectShoot});

    m_engineMap.insert({"editor.wordNPC.nominative",       &g_editorStrings.wordNPC});
    m_engineMap.insert({"editor.wordNPC.genitive",         &g_editorStrings.wordNPCGenitive});

    m_engineMap.insert({"editor.wordEvent.nominative",     &g_editorStrings.wordEvent});
    m_engineMap.insert({"editor.wordEvent.genitive",       &g_editorStrings.wordEventGenitive});
    m_engineMap.insert({"editor.wordEvent.typeLabel",      &g_editorStrings.phraseTypeLabelEvent});

    m_engineMap.insert({"editor.wordCoins",                &g_editorStrings.wordCoins});

    m_engineMap.insert({"editor.wordEnabled",              &g_editorStrings.wordEnabled});
    m_engineMap.insert({"editor.wordText",                 &g_editorStrings.wordText});
    m_engineMap.insert({"editor.wordInstant",              &g_editorStrings.wordInstant});
    m_engineMap.insert({"editor.wordMode",                 &g_editorStrings.wordMode});
    m_engineMap.insert({"editor.wordHeight",               &g_editorStrings.wordHeight});
    m_engineMap.insert({"editor.wordWidth",                &g_editorStrings.wordWidth});

    m_engineMap.insert({"editor.labelSortLayer",           &g_editorStrings.labelSortLayer});
    m_engineMap.insert({"editor.labelSortOffset",          &g_editorStrings.labelSortOffset});

    m_engineMap.insert({"editor.phraseTextOf",             &g_editorStrings.phraseTextOf});
    m_engineMap.insert({"editor.phraseSectionIndex",       &g_editorStrings.phraseSectionIndex});
    m_engineMap.insert({"editor.phraseRadiusIndex",        &g_editorStrings.phraseRadiusIndex});
    m_engineMap.insert({"editor.phraseWarpIndex",          &g_editorStrings.phraseWarpIndex});
    m_engineMap.insert({"editor.phraseGenericIndex",       &g_editorStrings.phraseGenericIndex});
    m_engineMap.insert({"editor.phraseDelayIsMs",          &g_editorStrings.phraseDelayIsMs});
    m_engineMap.insert({"editor.phraseCountMore",          &g_editorStrings.phraseCountMore});
    m_engineMap.insert({"editor.mapPos",                   &g_editorStrings.mapPos});
    m_engineMap.insert({"editor.phraseAreYouSure",         &g_editorStrings.phraseAreYouSure});
    m_engineMap.insert({"editor.pageBlankOfBlank",         &g_editorStrings.pageBlankOfBlank});

    m_engineMap.insert({"editor.letterUp",                 &g_editorStrings.letterUp});
    m_engineMap.insert({"editor.letterDown",               &g_editorStrings.letterDown});
    m_engineMap.insert({"editor.letterLeft",               &g_editorStrings.letterLeft});
    m_engineMap.insert({"editor.letterRight",              &g_editorStrings.letterRight});
    m_engineMap.insert({"editor.letterCoordX",             &g_editorStrings.letterCoordX});
    m_engineMap.insert({"editor.letterCoordY",             &g_editorStrings.letterCoordY});

    m_engineMap.insert({"editor.toggleMagicBlock",         &g_editorStrings.toggleMagicBlock});

    m_engineMap.insert({"editor.testPlay.magicHand",       &g_editorStrings.testMagicHand});
    m_engineMap.insert({"editor.testPlay.char",            &g_editorStrings.testChar});
    m_engineMap.insert({"editor.testPlay.power",           &g_editorStrings.testPower});
    m_engineMap.insert({"editor.testPlay.boot",            &g_editorStrings.testBoot});
    m_engineMap.insert({"editor.testPlay.pet",             &g_editorStrings.testPet});

    m_engineMap.insert({"editor.events.header",            &g_editorStrings.eventsHeader});

    m_engineMap.insert({"editor.events.letter.activate",    &g_editorStrings.eventsLetterActivate});
    m_engineMap.insert({"editor.events.letter.death",       &g_editorStrings.eventsLetterDeath});
    m_engineMap.insert({"editor.events.letter.talk",        &g_editorStrings.eventsLetterTalk});
    m_engineMap.insert({"editor.events.letter.layerClear",  &g_editorStrings.eventsLetterLayerClear});
    m_engineMap.insert({"editor.events.letter.hit",         &g_editorStrings.eventsLetterHit});
    m_engineMap.insert({"editor.events.letter.destroy",     &g_editorStrings.eventsLetterDestroy});
    m_engineMap.insert({"editor.events.letter.enter",       &g_editorStrings.eventsLetterEnter});

    m_engineMap.insert({"editor.events.label.next",        &g_editorStrings.eventsLabelNext});
    m_engineMap.insert({"editor.events.label.activate",    &g_editorStrings.eventsLabelActivate});
    m_engineMap.insert({"editor.events.label.death",       &g_editorStrings.eventsLabelDeath});
    m_engineMap.insert({"editor.events.label.talk",        &g_editorStrings.eventsLabelTalk});
    m_engineMap.insert({"editor.events.label.layerClear",  &g_editorStrings.eventsLabelLayerClear});
    m_engineMap.insert({"editor.events.label.hit",         &g_editorStrings.eventsLabelHit});
    m_engineMap.insert({"editor.events.label.destroy",     &g_editorStrings.eventsLabelDestroy});
    m_engineMap.insert({"editor.events.label.enter",       &g_editorStrings.eventsLabelEnter});

    m_engineMap.insert({"editor.events.desc.activate",     &g_editorStrings.eventsDescActivate});
    m_engineMap.insert({"editor.events.desc.death",        &g_editorStrings.eventsDescDeath});
    m_engineMap.insert({"editor.events.desc.talk",         &g_editorStrings.eventsDescTalk});
    m_engineMap.insert({"editor.events.desc.layerClear",   &g_editorStrings.eventsDescLayerClear});
    m_engineMap.insert({"editor.events.desc.hit",          &g_editorStrings.eventsDescHit});
    m_engineMap.insert({"editor.events.desc.destroy",      &g_editorStrings.eventsDescDestroy});
    m_engineMap.insert({"editor.events.desc.enter",        &g_editorStrings.eventsDescEnter});

    m_engineMap.insert({"editor.events.desc.phraseTriggersWhenTemplate",   &g_editorStrings.eventsDescPhraseTriggersWhenTemplate});
    m_engineMap.insert({"editor.events.desc.phraseTriggersAfterTemplate",   &g_editorStrings.eventsDescPhraseTriggersAfterTemplate});

    m_engineMap.insert({"editor.events.deletion.deletingEvent",    &g_editorStrings.eventsDeletingEvent});
    m_engineMap.insert({"editor.events.deletion.confirm",          &g_editorStrings.eventsDeletionConfirm});
    m_engineMap.insert({"editor.events.deletion.cancel",           &g_editorStrings.eventsDeletionCancel});

    m_engineMap.insert({"editor.events.promptEventText",  &g_editorStrings.eventsPromptEventText});
    m_engineMap.insert({"editor.events.promptEventName",  &g_editorStrings.eventsPromptEventName});
    m_engineMap.insert({"editor.events.itemNewEvent",     &g_editorStrings.eventsItemNewEvent});

    m_engineMap.insert({"editor.events.controlsForEventN",      &g_editorStrings.eventsControlsForEvent});
    m_engineMap.insert({"editor.events.settingsForEvent",       &g_editorStrings.eventsSettingsForEvent});

    m_engineMap.insert({"editor.events.layers.headerShow",      &g_editorStrings.eventsHeaderShow});
    m_engineMap.insert({"editor.events.layers.headerHide",      &g_editorStrings.eventsHeaderHide});
    m_engineMap.insert({"editor.events.layers.headerToggle",    &g_editorStrings.eventsHeaderToggle});
    m_engineMap.insert({"editor.events.layers.headerMove",      &g_editorStrings.eventsHeaderMove});

    m_engineMap.insert({"editor.events.sections.actionKeep",    &g_editorStrings.eventsActionKeep});
    m_engineMap.insert({"editor.events.sections.actionReset",   &g_editorStrings.eventsActionReset});
    m_engineMap.insert({"editor.events.sections.actionSet",     &g_editorStrings.eventsActionSet});

    m_engineMap.insert({"editor.events.sections.propMusic",     &g_editorStrings.eventsCaseMusic});
    m_engineMap.insert({"editor.events.sections.propBackground", &g_editorStrings.eventsCaseBackground});
    m_engineMap.insert({"editor.events.sections.propBounds",    &g_editorStrings.eventsCaseBounds});

    m_engineMap.insert({"editor.events.sections.phraseAllSections",     &g_editorStrings.eventsPhraseAllSections});

    m_engineMap.insert({"editor.events.props.autostart",     &g_editorStrings.eventsPropAutostart});
    m_engineMap.insert({"editor.events.props.sound",         &g_editorStrings.eventsPropSound});
    m_engineMap.insert({"editor.events.props.endGame",       &g_editorStrings.eventsPropEndGame});
    m_engineMap.insert({"editor.events.props.controls",      &g_editorStrings.eventsPropControls});
    m_engineMap.insert({"editor.events.props.layerSmoke",    &g_editorStrings.eventsPropLayerSmoke});

    m_engineMap.insert({"editor.events.headerTriggerEvent",  &g_editorStrings.eventsHeaderTriggerEvent});

    m_engineMap.insert({"editor.level.levelName",            &g_editorStrings.levelName});
    m_engineMap.insert({"editor.level.startPos",             &g_editorStrings.levelStartPos});
    m_engineMap.insert({"editor.level.pathBG",               &g_editorStrings.levelPathBG});
    m_engineMap.insert({"editor.level.bigBG",                &g_editorStrings.levelBigBG});
    m_engineMap.insert({"editor.level.gameStart",            &g_editorStrings.levelGameStart});
    m_engineMap.insert({"editor.level.alwaysVis",            &g_editorStrings.levelAlwaysVis});
    m_engineMap.insert({"editor.level.pathUnlocks",          &g_editorStrings.levelPathUnlocks});

    m_engineMap.insert({"editor.section.scroll",             &g_editorStrings.sectionScroll});
    m_engineMap.insert({"editor.section.horizWrap",          &g_editorStrings.sectionHorizWrap});
    m_engineMap.insert({"editor.section.vertWrap",           &g_editorStrings.sectionVertWrap});
    m_engineMap.insert({"editor.section.underwater",         &g_editorStrings.sectionUnderwater});
    m_engineMap.insert({"editor.section.noTurnBack",         &g_editorStrings.sectionNoTurnBack});
    m_engineMap.insert({"editor.section.offscreenExit",      &g_editorStrings.sectionOffscreenExit});

    m_engineMap.insert({"editor.world.name",                 &g_editorStrings.worldName});
    m_engineMap.insert({"editor.world.introLevel",           &g_editorStrings.worldIntroLevel});
    m_engineMap.insert({"editor.world.hubWorld",             &g_editorStrings.worldHubWorld});
    m_engineMap.insert({"editor.world.retryOnFail",          &g_editorStrings.worldRetryOnFail});
    m_engineMap.insert({"editor.world.totalStars",           &g_editorStrings.worldTotalStars});
    m_engineMap.insert({"editor.world.allowChars",           &g_editorStrings.worldAllowChars});
    m_engineMap.insert({"editor.world.phraseCreditIndex",    &g_editorStrings.worldCreditIndex});

    m_engineMap.insert({"editor.select.soundForEventN",              &g_editorStrings.selectSoundForEvent});
    m_engineMap.insert({"editor.select.sectBlankPropBlankForEventN", &g_editorStrings.selectSectBlankPropBlankForEvent});
    m_engineMap.insert({"editor.select.allSectPropBlankForEventN",   &g_editorStrings.selectAllSectPropBlankForEvent});
    m_engineMap.insert({"editor.select.sectionBlankPropBlank",     &g_editorStrings.selectSectionBlankPropBlank});
    m_engineMap.insert({"editor.select.pathBlankUnlock",           &g_editorStrings.selectPathBlankUnlock});
    m_engineMap.insert({"editor.select.warpTransitEffect",         &g_editorStrings.selectWarpTransitionEffect});
    m_engineMap.insert({"editor.select.worldMusic",                &g_editorStrings.selectWorldMusic});

    m_engineMap.insert({"editor.layers.header",              &g_editorStrings.layersHeader});

    m_engineMap.insert({"editor.layers.label",               &g_editorStrings.labelLayer});
    m_engineMap.insert({"editor.layers.labelAttached",       &g_editorStrings.layersLabelAttached});
    m_engineMap.insert({"editor.layers.abbrevAttLayer",      &g_editorStrings.layersAbbrevAttLayer});
    m_engineMap.insert({"editor.layers.default",             &g_editorStrings.layersLayerDefault});

    m_engineMap.insert({"editor.layers.labelAttachedLayer",  &g_editorStrings.layersLabelAttachedLayer});
    m_engineMap.insert({"editor.layers.labelMoveLayer",      &g_editorStrings.layersLabelMoveLayer});

    m_engineMap.insert({"editor.layers.deletion.header",                   &g_editorStrings.layersDeletionHeader});
    m_engineMap.insert({"editor.layers.deletion.preserveLayerContents",    &g_editorStrings.layersDeletionPreserveLayerContents});
    m_engineMap.insert({"editor.layers.deletion.confirmPreserve",          &g_editorStrings.layersDeletionConfirmPreserve});
    m_engineMap.insert({"editor.layers.deletion.confirmDelete",            &g_editorStrings.layersDeletionConfirmDelete});
    m_engineMap.insert({"editor.layers.deletion.cancel",                   &g_editorStrings.layersDeletionCancel});

    m_engineMap.insert({"editor.layers.desc.att", &g_editorStrings.layersDescAtt});

    m_engineMap.insert({"editor.layers.promptLayerName", &g_editorStrings.layersPromptLayerName});
    m_engineMap.insert({"editor.layers.itemNewLayer",    &g_editorStrings.layersItemNewLayer});

    m_engineMap.insert({"editor.file.actionClearLevel",            &g_editorStrings.fileActionClearLevel});
    m_engineMap.insert({"editor.file.actionClearWorld",            &g_editorStrings.fileActionClearWorld});
    m_engineMap.insert({"editor.file.actionOpen",                  &g_editorStrings.fileActionOpen});
    m_engineMap.insert({"editor.file.actionRevert",                &g_editorStrings.fileActionRevert});
    m_engineMap.insert({"editor.file.actionExit",                  &g_editorStrings.fileActionExit});
    m_engineMap.insert({"editor.file.confirmSaveBeforeAction",     &g_editorStrings.fileConfirmationSaveBeforeAction});
    m_engineMap.insert({"editor.file.confirmConfirmAction",        &g_editorStrings.fileConfirmationConfirmAction});
    m_engineMap.insert({"editor.file.confirmConvertFormatTo",      &g_editorStrings.fileConfirmationConvertFormatTo});
    m_engineMap.insert({"editor.file.optionYesSaveThenAction",     &g_editorStrings.fileOptionYesSaveThenAction});
    m_engineMap.insert({"editor.file.optionActionWithoutSave",     &g_editorStrings.fileOptionActionWithoutSave});
    m_engineMap.insert({"editor.file.optionCancelAction",          &g_editorStrings.fileOptionCancelAction});
    m_engineMap.insert({"editor.file.optionProceedWithConversion", &g_editorStrings.fileOptionProceedWithConversion});
    m_engineMap.insert({"editor.file.optionCancelConversion",      &g_editorStrings.fileOptionCancelConversion});

    m_engineMap.insert({"editor.file.labelCurrentFile",    &g_editorStrings.fileLabelCurrentFile});
    m_engineMap.insert({"editor.file.labelFormat",         &g_editorStrings.fileLabelFormat});

    m_engineMap.insert({"editor.file.formatModern",        &g_editorStrings.fileFormatModern});
    m_engineMap.insert({"editor.file.formatLegacy",        &g_editorStrings.fileFormatLegacy});

    m_engineMap.insert({"editor.file.sectionLevel",        &g_editorStrings.fileSectionLevel});
    m_engineMap.insert({"editor.file.sectionWorld",        &g_editorStrings.fileSectionWorld});
    m_engineMap.insert({"editor.file.commandNew",          &g_editorStrings.fileCommandNew});
    m_engineMap.insert({"editor.file.commandOpen",         &g_editorStrings.fileCommandOpen});
    m_engineMap.insert({"editor.file.commandSave",         &g_editorStrings.fileCommandSave});
    m_engineMap.insert({"editor.file.commandSaveAs",       &g_editorStrings.fileCommandSaveAs});

    m_engineMap.insert({"editor.file.convert.descNew",             &g_editorStrings.fileConvertDesc});

#if 0
    m_engineMap.insert({"editor.file.convert._38aUnsupported",     &g_editorStrings.fileConvert38aUnsupported});
    m_engineMap.insert({"editor.file.convert.formatUnknown",       &g_editorStrings.fileConvertFormatUnknown});

    m_engineMap.insert({"editor.file.convert.featureWarpTransit",      &g_editorStrings.fileConvertFeatureWarpTransit});
    m_engineMap.insert({"editor.file.convert.featureWarpNeedsStand",   &g_editorStrings.fileConvertFeatureWarpNeedsStand});
    m_engineMap.insert({"editor.file.convert.featureWarpCannonExit",   &g_editorStrings.fileConvertFeatureWarpCannonExit});
    m_engineMap.insert({"editor.file.convert.featureWarpEnterEvent",   &g_editorStrings.fileConvertFeatureWarpEnterEvent});
    m_engineMap.insert({"editor.file.convert.featureWarpCustomStarsMsg", &g_editorStrings.fileConvertFeatureWarpCustomStarsMsg});
    m_engineMap.insert({"editor.file.convert.featureWarpNoPrintStars", &g_editorStrings.fileConvertFeatureWarpNoPrintStars});
    m_engineMap.insert({"editor.file.convert.featureWarpNoStartScene", &g_editorStrings.fileConvertFeatureWarpNoStartScene});
    m_engineMap.insert({"editor.file.convert.featureWarpPortal",       &g_editorStrings.fileConvertFeatureWarpPortal});

    m_engineMap.insert({"editor.file.convert.featureEventCustomMusic", &g_editorStrings.fileConvertFeatureEventCustomMusic});
    m_engineMap.insert({"editor.file.convert.featureEventAutoscroll",  &g_editorStrings.fileConvertFeatureEventAutoscroll});

    m_engineMap.insert({"editor.file.convert.featureNPCVariant",       &g_editorStrings.fileConvertFeatureNPCVariant});
    m_engineMap.insert({"editor.file.convert.featureBlockForceSmashable", &g_editorStrings.fileConvertFeatureBlockForceSmashable});
    m_engineMap.insert({"editor.file.convert.featureBgoOrder",         &g_editorStrings.fileConvertFeatureBgoOrder});

    m_engineMap.insert({"editor.file.convert.featureCustomWorldMusic", &g_editorStrings.fileConvertFeatureCustomWorldMusic});
    m_engineMap.insert({"editor.file.convert.featureWorldStarDisplay", &g_editorStrings.fileConvertFeatureWorldStarDisplay});
    m_engineMap.insert({"editor.file.convert.featureLevelStarDisplay", &g_editorStrings.fileConvertFeatureLevelStarDisplay});
    m_engineMap.insert({"editor.file.convert.featureWorldMapSections", &g_editorStrings.fileConvertFeatureWorldMapSections});
#endif

    m_engineMap.insert({"editor.browser.newFile",          &g_editorStrings.browserNewFile});
    m_engineMap.insert({"editor.browser.saveFile",         &g_editorStrings.browserSaveFile});
    m_engineMap.insert({"editor.browser.openFile",         &g_editorStrings.browserOpenFile});

    m_engineMap.insert({"editor.browser.itemNewFile",      &g_editorStrings.browserItemNewFile});
    m_engineMap.insert({"editor.browser.itemNewFolder",    &g_editorStrings.browserItemNewFolder});

    m_engineMap.insert({"editor.browser.askOverwriteFile", &g_editorStrings.browserAskOverwriteFile});

    m_engineMap.insert({"editor.tooltip.select",   &g_editorStrings.tooltipSelect});
    m_engineMap.insert({"editor.tooltip.erase",    &g_editorStrings.tooltipErase});
    m_engineMap.insert({"editor.tooltip.eraseAll", &g_editorStrings.tooltipEraseAll});
    m_engineMap.insert({"editor.tooltip.blocks",   &g_editorStrings.tooltipBlocks});
    m_engineMap.insert({"editor.tooltip.BGOs",     &g_editorStrings.tooltipBGOs});
    m_engineMap.insert({"editor.tooltip.NPCs",     &g_editorStrings.tooltipNPCs});
    m_engineMap.insert({"editor.tooltip.warps",    &g_editorStrings.tooltipWarps});
    m_engineMap.insert({"editor.tooltip.water",    &g_editorStrings.tooltipWater});
    m_engineMap.insert({"editor.tooltip.settings", &g_editorStrings.tooltipSettings});
    m_engineMap.insert({"editor.tooltip.layers",   &g_editorStrings.tooltipLayers});
    m_engineMap.insert({"editor.tooltip.events",   &g_editorStrings.tooltipEvents});
    m_engineMap.insert({"editor.tooltip.tiles",    &g_editorStrings.tooltipTiles});
    m_engineMap.insert({"editor.tooltip.scenes",   &g_editorStrings.tooltipScenes});
    m_engineMap.insert({"editor.tooltip.levels",   &g_editorStrings.tooltipLevels});
    m_engineMap.insert({"editor.tooltip.paths",    &g_editorStrings.tooltipPaths});
    m_engineMap.insert({"editor.tooltip.music",    &g_editorStrings.tooltipMusic});
    m_engineMap.insert({"editor.tooltip.area",     &g_editorStrings.tooltipArea});
    m_engineMap.insert({"editor.tooltip.file",     &g_editorStrings.tooltipFile});
    m_engineMap.insert({"editor.tooltip.show",     &g_editorStrings.tooltipShow});

#endif // THEXTECH_ENABLE_EDITOR
    // };

#ifdef THEXTECH_ENABLE_EDITOR
    for(int i = 0; i < 6; ++i)
    {
        m_engineMap.insert({fmt::format_ne("editor.warp.transit.name{0}", i), &g_editorStrings.listWarpTransitNames[i]});
    }
#endif


    m_assetsMap =
    {
        {"languageName", &g_mainMenu.languageName},

        {"objects.wordStarAccusativeSingular",      &g_gameInfo.wordStarAccusativeSingular},
        {"objects.wordStarAccusativeDualOrCounter", &g_gameInfo.wordStarAccusativeDual_Cnt},
        {"objects.wordStarAccusativePlural",        &g_gameInfo.wordStarAccusativePlural},

        {"objects.wordFails", &g_gameInfo.fails_counter_title}
    };

    for(int i = 1; i <= numCharacters; ++i)
        m_assetsMap.insert({fmt::format_ne("character.name{0}", i), &g_gameInfo.characterName[i]});

    // reset all strings for options to hardcoded defaults
    g_options.reset_options();

    // add config fields to engine map
    g_options.make_translation(*this);

#ifdef THEXTECH_ENABLE_EDITOR
    // adds dynamic fields to the asset map
    EditorCustom::Load(this);
#endif

    XHints::InitTranslations(*this);
}

void XTechTranslate::reset()
{
    initOutroContent();
    initMainMenu();
    initGameStrings();

#ifdef THEXTECH_ENABLE_EDITOR
    initEditorStrings();

    // don't need to reset EditorCustom because we reloaded it in the initializer
    // it would be dangerous to reload it here because it would invalidate a lot of references
#endif

    // likewise, unsafe to reset g_options here because it would invalidate pointers to data inside vectors
    // g_options.reset_options();

    Controls::InitStrings();
    g_controlsStrings = ControlsStrings_t();

    XHints::ResetStrings();

    s_CurrentPluralRules = PluralRules::OneIsSingular;
}

void XTechTranslate::exportTemplate()
{
#ifndef THEXTECH_DISABLE_LANG_TOOLS
    reset();

    try
    {
        nlohmann::ordered_json langFile;

        for(auto &k : m_engineMap)
        {
#ifdef DEBUG_BUILD
            std::printf("-- writing %s -> %s\n", k.first.c_str(), k.second->c_str());
#endif
            setJsonValue(langFile, k.first, *k.second);
        }

        std::printf("Lang file data (Engine): \n\n%s\n\n", langFile.dump(4, ' ', false).c_str());
        std::fflush(stdout);

        langFile.clear();

        for(auto &k : m_assetsMap)
        {
#ifdef DEBUG_BUILD
            std::printf("-- writing %s -> %s\n", k.first.c_str(), k.second->c_str());
#endif
            setJsonValue(langFile, k.first, *k.second);
        }

        std::printf("Lang file data (Assets): \n\n%s\n\n", langFile.dump(4, ' ', false).c_str());
        std::fflush(stdout);
    }
    catch(const std::exception &e)
    {
        std::printf("JSON: Caught an exception: %s", e.what());
        std::fflush(stdout);
    }
#endif
}

void XTechTranslate::updateLanguages(const std::string &outPath, bool noBlank)
{
#ifndef THEXTECH_DISABLE_LANG_TOOLS
    std::vector<std::string> list;
    DirMan langs(outPath.empty() ? AppPathManager::languagesDir() : outPath);

    if(!langs.exists())
    {
        std::printf("Can't open the languages directory: %s", langs.absolutePath().c_str());
        std::fflush(stdout);
        return;
    }

    if(!langs.getListOfFiles(list, {".json"}))
    {
        std::printf("Can't show the content of the languages directory: %s", langs.absolutePath().c_str());
        std::fflush(stdout);
        return;
    }

    reset();

    for(auto &f : list)
    {
        bool isEnglish = Strings::endsWith(f, "_en.json");
        std::printf("-- Processing file %s...\n", f.c_str());
        std::string fullFilePath = langs.absolutePath() + "/" + f;
        bool changed = false;

        TrList &trList = Strings::startsWith(f, "thextech_") ? m_engineMap : m_assetsMap;

        try
        {
            Files::Data data = Files::load_file(fullFilePath);

            if(!data.valid())
            {
                std::printf("Warning: Failed to load the translation file %s: can't open file\n", fullFilePath.c_str());
                continue;
            }

            nlohmann::ordered_json langFile = nlohmann::ordered_json::parse(data.begin(), data.end());

            for(const auto &k : trList)
            {
                const std::string &res = *k.second;
                changed |= setJsonValueIfNotExist(langFile, k.first, isEnglish ? res : std::string(), noBlank);
            }

            if(!changed)
            {
                std::printf("Lang file remain unchanged!\n");
                std::fflush(stdout);
                continue;
            }

            if(saveFile(fullFilePath, langFile.dump(4, ' ', false)))
            {
                std::printf("Lang file has been updated!\n");
                std::fflush(stdout);
            }
            else
            {
                std::printf("Error: Failed to write out the file %s!\n", fullFilePath.c_str());
                std::fflush(stdout);
            }
        }
        catch(const std::exception &e)
        {
            std::printf("Warning: Failed to process the translation file %s: %s\n", fullFilePath.c_str(), e.what());
        }
    }

    std::fflush(stdout);
#else
    UNUSED(outPath);
    UNUSED(noBlank);
#endif
}



bool XTechTranslate::translate()
{
    if(!FontManager::isInitied() || FontManager::isLegacy())
    {
        pLogWarning("Translations aren't supported without new font engine loaded (the 'fonts' directory is required)");
        return false;
    }

    s_CurrentPluralRules = PluralRules::OneIsSingular;

    const std::string &langEngineFile = XLanguage::getEngineFile();
    const std::string &langAssetsFile = XLanguage::getAssetsFile();

    if(!langEngineFile.empty() && Files::fileExists(langEngineFile))
    {
        // Engine translations
        if(!translateFile(langEngineFile, m_engineMap, "engine"))
            pLogWarning("Failed to apply the engine translation file %s", langEngineFile.c_str());
    }

    if(!langAssetsFile.empty() && Files::fileExists(langAssetsFile))
    {
        // assets translations
        if(!translateFile(langAssetsFile, m_assetsMap, "assets"))
            pLogWarning("Failed to apply the assets translation file %s", langAssetsFile.c_str());
    }

    if(langEngineFile.empty() && langAssetsFile.empty())
        return false; // No language was found at all

    if(!Files::fileExists(langEngineFile) && !Files::fileExists(langAssetsFile))
        return false; // Files are not exists, do nothing

    if(g_mainMenu.pluralRules == "one-is-singular")
        s_CurrentPluralRules = PluralRules::OneIsSingular;
    else if(g_mainMenu.pluralRules == "singular-only")
        s_CurrentPluralRules = PluralRules::SingularOnly;
    else if(g_mainMenu.pluralRules == "slavic")
        s_CurrentPluralRules = PluralRules::Slavic;

    return true;
}

bool XTechTranslate::translateFile(const std::string& file, TrList& list, const char *trTypeName)
{
    try
    {
        // Engine translations
        Files::Data data = Files::load_file(file);

        if(!data.valid())
        {
            pLogWarning("Failed to load the %s translation file %s: can't open file", trTypeName, file.c_str());
            return false;
        }

        nlohmann::ordered_json langFile = nlohmann::ordered_json::parse(data.begin(), data.end());

        for(auto &k : list)
        {
            std::string &res = *k.second;
            res = getJsonValue(langFile, k.first, *k.second);
        }
    }
    catch(const std::exception &e)
    {
        reset();
        pLogWarning("Failed to load the %s translation file %s: %s", trTypeName, file.c_str(), e.what());
        return false;
    }

    return true;
}

void ReloadTranslations()
{
    XLanguage::resolveLanguage(g_config.language);

    XTechTranslate translator;
    translator.reset();
    if(translator.translate())
    {
        pLogDebug("Reloaded translation for language %s-%s",
                  CurrentLanguage.c_str(),
                  CurrentLangDialect.empty() ? "??" : CurrentLangDialect.c_str());
    }
}

