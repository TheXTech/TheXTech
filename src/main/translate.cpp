/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "translate.h"
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

#if TR_MAP_TYPE == 1 && !defined(THEXTECH_DISABLE_LANG_TOOLS)
#   include <sorting/tinysort.h>
#endif


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


void XTechTranslate::insert(TrList &list, const char *key, std::string *value)
{
#if defined(DEBUG_BUILD) && TR_MAP_TYPE == 1
    for(auto &k : list)
    {
        if(k.first.compare(key) == 0)
            SDL_assert_release(false && "Duplicated translation line detected!");
    }
#endif
    list.TR_MAP_INSERT({key, value});
}

void XTechTranslate::insert(TrList &list, std::string &&key, std::string *value)
{
#if defined(DEBUG_BUILD) && TR_MAP_TYPE == 1
    for(auto &k : list)
    {
        if(k.first.compare(key) == 0)
            SDL_assert_release(false && "Duplicated translation line detected!");
    }
#endif
    list.TR_MAP_INSERT({key, value});
}

XTechTranslate::XTechTranslate()
{
    // List of all translatable strings of the engine
    m_engineMap.clear();

    insert(m_engineMap, "menu.main.mainPlayEpisode",            &g_mainMenu.mainPlayEpisode);
    insert(m_engineMap, "menu.main.main1PlayerGame",            &g_mainMenu.main1PlayerGame);
    insert(m_engineMap, "menu.main.mainMultiplayerGame",        &g_mainMenu.mainMultiplayerGame);
    insert(m_engineMap, "menu.main.mainBattleGame",             &g_mainMenu.mainBattleGame);
    insert(m_engineMap, "menu.main.mainEditor",                 &g_mainMenu.mainEditor);
    insert(m_engineMap, "menu.main.mainOptions",                &g_mainMenu.mainOptions);
    insert(m_engineMap, "menu.main.mainExit",                   &g_mainMenu.mainExit);

    insert(m_engineMap, "menu.loading",                         &g_mainMenu.loading);

    insert(m_engineMap, "languageName",                         &g_mainMenu.languageName);
    insert(m_engineMap, "pluralRules",                          &g_mainMenu.pluralRules);

    insert(m_engineMap, "menu.editor.battles",                  &g_mainMenu.editorBattles);
    insert(m_engineMap, "menu.editor.newWorld",                 &g_mainMenu.editorNewWorld);
    insert(m_engineMap, "menu.editor.makeFor",                  &g_mainMenu.editorMakeFor);
    insert(m_engineMap, "menu.editor.errorMissingResources",    &g_mainMenu.editorErrorMissingResources);
    insert(m_engineMap, "menu.editor.promptNewWorldName",       &g_mainMenu.editorPromptNewWorldName);

    insert(m_engineMap, "menu.game.gameNoEpisodesToPlay",       &g_mainMenu.gameNoEpisodesToPlay);
    insert(m_engineMap, "menu.game.gameNoBattleLevels",         &g_mainMenu.gameNoBattleLevels);
    insert(m_engineMap, "menu.game.gameBattleRandom",           &g_mainMenu.gameBattleRandom);

    insert(m_engineMap, "menu.game.warnEpCompat",               &g_mainMenu.warnEpCompat);

    insert(m_engineMap, "menu.game.gameSlotContinue",           &g_mainMenu.gameSlotContinue);
    insert(m_engineMap, "menu.game.gameSlotNew",                &g_mainMenu.gameSlotNew);

    insert(m_engineMap, "menu.game.gameCopySave",               &g_mainMenu.gameCopySave);
    insert(m_engineMap, "menu.game.gameEraseSave",              &g_mainMenu.gameEraseSave);

    insert(m_engineMap, "menu.game.gameSourceSlot",             &g_mainMenu.gameSourceSlot);
    insert(m_engineMap, "menu.game.gameTargetSlot",             &g_mainMenu.gameTargetSlot);
    insert(m_engineMap, "menu.game.gameEraseSlot",              &g_mainMenu.gameEraseSlot);

    insert(m_engineMap, "menu.game.phraseScore",                &g_mainMenu.phraseScore);
    insert(m_engineMap, "menu.game.phraseTime",                 &g_mainMenu.phraseTime);

    insert(m_engineMap, "menu.battle.errorNoLevels",            &g_mainMenu.errorBattleNoLevels);

    insert(m_engineMap, "menu.options.restartEngine",           &g_mainMenu.optionsRestartEngine);

    insert(m_engineMap, "menu.character.selectCharacter",       &g_mainMenu.selectCharacter);


    insert(m_engineMap, "menu.controls.controlsTitle",          &g_mainMenu.controlsTitle);
    insert(m_engineMap, "menu.controls.controlsConnected",      &g_mainMenu.controlsConnected);
    insert(m_engineMap, "menu.controls.controlsDeleteKey",      &g_mainMenu.controlsDeleteKey);
    insert(m_engineMap, "menu.controls.controlsDeviceTypes",    &g_mainMenu.controlsDeviceTypes);
    insert(m_engineMap, "menu.controls.controlsInUse",          &g_mainMenu.controlsInUse);
    insert(m_engineMap, "menu.controls.controlsNotInUse",       &g_mainMenu.controlsNotInUse);

    insert(m_engineMap, "menu.controls.wordProfiles",           &g_mainMenu.wordProfiles);
    insert(m_engineMap, "menu.controls.wordButtons",            &g_mainMenu.wordButtons);

    insert(m_engineMap, "menu.controls.controlsReallyDeleteProfile", &g_mainMenu.controlsReallyDeleteProfile);
    insert(m_engineMap, "menu.controls.controlsNewProfile",     &g_mainMenu.controlsNewProfile);
    insert(m_engineMap, "menu.controls.caseInvalid",            &g_controlsStrings.sharedCaseInvalid);

    insert(m_engineMap, "menu.controls.profile.renameProfile",  &g_mainMenu.controlsRenameProfile);
    insert(m_engineMap, "menu.controls.profile.deleteProfile",  &g_mainMenu.controlsDeleteProfile);
    insert(m_engineMap, "menu.controls.profile.playerControls", &g_mainMenu.controlsPlayerControls);
    insert(m_engineMap, "menu.controls.profile.cursorControls", &g_mainMenu.controlsCursorControls);
    insert(m_engineMap, "menu.controls.profile.editorControls", &g_mainMenu.controlsEditorControls);
    insert(m_engineMap, "menu.controls.profile.hotkeys",        &g_mainMenu.controlsHotkeys);

    insert(m_engineMap, "menu.controls.options.rumble",         &g_mainMenu.controlsOptionRumble);
    insert(m_engineMap, "menu.controls.options.batteryStatus",  &g_mainMenu.controlsOptionBatteryStatus);
    insert(m_engineMap, "menu.controls.options.maxPlayers",     &g_controlsStrings.sharedOptionMaxPlayers);

    insert(m_engineMap, "menu.controls.buttons.up",      &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Up]);
    insert(m_engineMap, "menu.controls.buttons.down",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Down]);
    insert(m_engineMap, "menu.controls.buttons.left",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Left]);
    insert(m_engineMap, "menu.controls.buttons.right",   &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Right]);
    insert(m_engineMap, "menu.controls.buttons.jump",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Jump]);
    insert(m_engineMap, "menu.controls.buttons.run",     &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Run]);
    insert(m_engineMap, "menu.controls.buttons.altJump", &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::AltJump]);
    insert(m_engineMap, "menu.controls.buttons.altRun",  &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::AltRun]);
    insert(m_engineMap, "menu.controls.buttons.start",   &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Start]);
    insert(m_engineMap, "menu.controls.buttons.drop",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Drop]);

    insert(m_engineMap, "menu.controls.cursor.up",        &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorUp]);
    insert(m_engineMap, "menu.controls.cursor.down",      &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorDown]);
    insert(m_engineMap, "menu.controls.cursor.left",      &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorLeft]);
    insert(m_engineMap, "menu.controls.cursor.right",     &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorRight]);
    insert(m_engineMap, "menu.controls.cursor.primary",   &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Primary]);
    insert(m_engineMap, "menu.controls.cursor.secondary", &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Secondary]);
    insert(m_engineMap, "menu.controls.cursor.tertiary",  &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Tertiary]);

    insert(m_engineMap, "menu.controls.editor.scrollUp",      &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollUp]);
    insert(m_engineMap, "menu.controls.editor.scrollDown",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollDown]);
    insert(m_engineMap, "menu.controls.editor.scrollLeft",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollLeft]);
    insert(m_engineMap, "menu.controls.editor.scrollRight",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollRight]);
    insert(m_engineMap, "menu.controls.editor.fastScroll",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::FastScroll]);
    insert(m_engineMap, "menu.controls.editor.modeSelect",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ModeSelect]);
    insert(m_engineMap, "menu.controls.editor.modeErase",     &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ModeErase]);
    insert(m_engineMap, "menu.controls.editor.prevSection",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::PrevSection]);
    insert(m_engineMap, "menu.controls.editor.nextSection",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::NextSection]);
    insert(m_engineMap, "menu.controls.editor.switchScreens", &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::SwitchScreens]);
    insert(m_engineMap, "menu.controls.editor.testPlay",      &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::TestPlay]);

#ifndef RENDER_FULLSCREEN_ALWAYS
    insert(m_engineMap, "menu.controls.hotkeys.fullscreen",  &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::Fullscreen]);
#endif
#ifdef USE_SCREENSHOTS_AND_RECS
    insert(m_engineMap, "menu.controls.hotkeys.screenshot",  &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::Screenshot]);
#endif
#ifdef PGE_ENABLE_VIDEO_REC
    insert(m_engineMap, "menu.controls.hotkeys.recordGif",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::RecordGif]);
#endif
    insert(m_engineMap, "menu.controls.hotkeys.debugInfo",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::DebugInfo]);
    insert(m_engineMap, "menu.controls.hotkeys.vanillaCam",  &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::VanillaCam]);
    insert(m_engineMap, "menu.controls.hotkeys.enterCheats", &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::EnterCheats]);
    insert(m_engineMap, "menu.controls.hotkeys.toggleHUD",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::ToggleHUD]);
    insert(m_engineMap, "menu.controls.hotkeys.legacyPause", &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::LegacyPause]);

#ifdef CONTROLS_KEYBOARD_STRINGS
    insert(m_engineMap, "menu.controls.types.keyboard",    &g_controlsStrings.nameKeyboard);
    insert(m_engineMap, "menu.controls.options.textEntryStyle",    &g_controlsStrings.keyboardOptionTextEntryStyle);
    insert(m_engineMap, "menu.controls.caseMouse",         &g_controlsStrings.caseMouse);
#endif

#if defined(CONTROLS_KEYBOARD_STRINGS) || defined(CONTROLS_JOYSTICK_STRINGS)
    insert(m_engineMap, "menu.controls.types.gamepad",     &g_controlsStrings.nameGamepad);
#endif

#if defined(CONTROLS_JOYSTICK_STRINGS)
    insert(m_engineMap, "menu.controls.phraseNewProfOldJoy",&g_controlsStrings.phraseNewProfOldJoy);
    insert(m_engineMap, "menu.controls.types.oldJoystick", &g_controlsStrings.nameOldJoy);
#endif

#if defined(CONTROLS_JOYSTICK_STRINGS) || defined(CONTROLS_WII_STRINGS)
    insert(m_engineMap, "menu.controls.joystickSimpleEditor",&g_controlsStrings.joystickSimpleEditor);
#endif

#if defined(CONTROLS_TOUCHSCREEN_STRINGS)
    insert(m_engineMap, "menu.controls.types.touchscreen", &g_controlsStrings.nameTouchscreen);

    insert(m_engineMap, "menu.controls.caseTouch",         &g_controlsStrings.caseTouch);

    insert(m_engineMap, "menu.controls.touchscreen.option.layoutStyle",    &g_controlsStrings.touchscreenOptionLayoutStyle);
    insert(m_engineMap, "menu.controls.touchscreen.option.scaleFactor",    &g_controlsStrings.touchscreenOptionScaleFactor);
    insert(m_engineMap, "menu.controls.touchscreen.option.scaleDPad",      &g_controlsStrings.touchscreenOptionScaleDPad);
    insert(m_engineMap, "menu.controls.touchscreen.option.scaleButtons",   &g_controlsStrings.touchscreenOptionScaleButtons);
    insert(m_engineMap, "menu.controls.touchscreen.option.sStartSpacing",  &g_controlsStrings.touchscreenOptionSStartSpacing);
    insert(m_engineMap, "menu.controls.touchscreen.option.resetLayout",    &g_controlsStrings.touchscreenOptionResetLayout);
    insert(m_engineMap, "menu.controls.touchscreen.option.interfaceStyle", &g_controlsStrings.touchscreenOptionInterfaceStyle);
    insert(m_engineMap, "menu.controls.touchscreen.option.feedbackStrength", &g_controlsStrings.touchscreenOptionFeedbackStrength);
    insert(m_engineMap, "menu.controls.touchscreen.option.feedbackLength", &g_controlsStrings.touchscreenOptionFeedbackLength);
    insert(m_engineMap, "menu.controls.touchscreen.option.holdRun",        &g_controlsStrings.touchscreenOptionHoldRun);
    insert(m_engineMap, "menu.controls.touchscreen.option.showCodeButton", &g_controlsStrings.touchscreenOptionShowCodeButton);


    insert(m_engineMap, "menu.controls.touchscreen.layout.tight",          &g_controlsStrings.touchscreenLayoutTight);
    insert(m_engineMap, "menu.controls.touchscreen.layout.tinyOld",        &g_controlsStrings.touchscreenLayoutTinyOld);
    insert(m_engineMap, "menu.controls.touchscreen.layout.phoneOld",       &g_controlsStrings.touchscreenLayoutPhoneOld);
    insert(m_engineMap, "menu.controls.touchscreen.layout.longOld",        &g_controlsStrings.touchscreenLayoutLongOld);
    insert(m_engineMap, "menu.controls.touchscreen.layout.phabletOld",     &g_controlsStrings.touchscreenLayoutPhabletOld);
    insert(m_engineMap, "menu.controls.touchscreen.layout.tabletOld",      &g_controlsStrings.touchscreenLayoutTabletOld);
    insert(m_engineMap, "menu.controls.touchscreen.layout.standard",       &g_controlsStrings.touchscreenLayoutStandard);

    insert(m_engineMap, "menu.controls.touchscreen.style.actions",         &g_controlsStrings.touchscreenStyleActions);
    insert(m_engineMap, "menu.controls.touchscreen.style.ABXY",            &g_controlsStrings.touchscreenStyleABXY);
    insert(m_engineMap, "menu.controls.touchscreen.style.XODA",            &g_controlsStrings.touchscreenStyleXODA);
#endif // #if defined(CONTROLS_TOUCHSCREEN_STRINGS)

#if defined(CONTROLS_16M_STRINGS)
    insert(m_engineMap, "menu.controls.tDS.buttonA",      &g_controlsStrings.tdsButtonA);
    insert(m_engineMap, "menu.controls.tDS.buttonB",      &g_controlsStrings.tdsButtonB);
    insert(m_engineMap, "menu.controls.tDS.buttonX",      &g_controlsStrings.tdsButtonX);
    insert(m_engineMap, "menu.controls.tDS.buttonY",      &g_controlsStrings.tdsButtonY);
    insert(m_engineMap, "menu.controls.tDS.buttonL",      &g_controlsStrings.tdsButtonL);
    insert(m_engineMap, "menu.controls.tDS.buttonR",      &g_controlsStrings.tdsButtonR);
    insert(m_engineMap, "menu.controls.tDS.buttonSelect", &g_controlsStrings.tdsButtonSelect);
    insert(m_engineMap, "menu.controls.tDS.buttonStart",  &g_controlsStrings.tdsButtonStart);

    insert(m_engineMap, "menu.controls.tDS.casePen",      &g_controlsStrings.tdsCasePen);
#endif // #ifdef CONTROLS_16M_STRINGS

#if defined(CONTROLS_3DS_STRINGS)
    insert(m_engineMap, "menu.controls.tDS.buttonZL",     &g_controlsStrings.tdsButtonZL);
    insert(m_engineMap, "menu.controls.tDS.buttonZR",     &g_controlsStrings.tdsButtonZR);
    insert(m_engineMap, "menu.controls.tDS.dPad",         &g_controlsStrings.tdsDpad);
    insert(m_engineMap, "menu.controls.tDS.tStick",       &g_controlsStrings.tdsTstick);
    insert(m_engineMap, "menu.controls.tDS.cStick",       &g_controlsStrings.tdsCstick);
#endif // #ifdef CONTROLS_3DS_STRINGS

#if defined(CONTROLS_WII_STRINGS)
    insert(m_engineMap, "menu.controls.wii.typeWiimote",         &g_controlsStrings.wiiTypeWiimote);
    insert(m_engineMap, "menu.controls.wii.typeNunchuck",        &g_controlsStrings.wiiTypeNunchuck);
    insert(m_engineMap, "menu.controls.wii.typeClassic",         &g_controlsStrings.wiiTypeClassic);
    insert(m_engineMap, "menu.controls.wii.typeGamecube",        &g_controlsStrings.wiiTypeGamecube);
    insert(m_engineMap, "menu.controls.wii.phraseNewNunchuck",   &g_controlsStrings.wiiPhraseNewNunchuck);
    insert(m_engineMap, "menu.controls.wii.phraseNewClassic",    &g_controlsStrings.wiiPhraseNewClassic);

    insert(m_engineMap, "menu.controls.wii.wiimote.dPad",        &g_controlsStrings.wiiDpad);
    insert(m_engineMap, "menu.controls.wii.wiimote.buttonA",     &g_controlsStrings.wiiButtonA);
    insert(m_engineMap, "menu.controls.wii.wiimote.buttonB",     &g_controlsStrings.wiiButtonB);
    insert(m_engineMap, "menu.controls.wii.wiimote.buttonMinus", &g_controlsStrings.wiiButtonMinus);
    insert(m_engineMap, "menu.controls.wii.wiimote.buttonPlus",  &g_controlsStrings.wiiButtonPlus);
    insert(m_engineMap, "menu.controls.wii.wiimote.buttonHome",  &g_controlsStrings.wiiButtonHome);
    insert(m_engineMap, "menu.controls.wii.wiimote.button2",     &g_controlsStrings.wiiButton2);
    insert(m_engineMap, "menu.controls.wii.wiimote.button1",     &g_controlsStrings.wiiButton1);
    insert(m_engineMap, "menu.controls.wii.wiimote.shake",       &g_controlsStrings.wiiShake);
    insert(m_engineMap, "menu.controls.wii.wiimote.caseIR",      &g_controlsStrings.wiiCaseIR);

    insert(m_engineMap, "menu.controls.wii.nunchuck.prefixN",    &g_controlsStrings.wiiPrefixNunchuck);
    insert(m_engineMap, "menu.controls.wii.nunchuck.buttonZ",    &g_controlsStrings.wiiButtonZ);
    insert(m_engineMap, "menu.controls.wii.nunchuck.buttonC",    &g_controlsStrings.wiiButtonC);

    insert(m_engineMap, "menu.controls.wii.classic.lStick",      &g_controlsStrings.wiiLStick);
    insert(m_engineMap, "menu.controls.wii.classic.rStick",      &g_controlsStrings.wiiRStick);
    insert(m_engineMap, "menu.controls.wii.classic.buttonZL",    &g_controlsStrings.wiiButtonZL);
    insert(m_engineMap, "menu.controls.wii.classic.buttonZR",    &g_controlsStrings.wiiButtonZR);
    insert(m_engineMap, "menu.controls.wii.classic.buttonLT",    &g_controlsStrings.wiiButtonLT);
    insert(m_engineMap, "menu.controls.wii.classic.buttonRT",    &g_controlsStrings.wiiButtonRT);
    insert(m_engineMap, "menu.controls.wii.classic.buttonX",     &g_controlsStrings.wiiButtonX);
    insert(m_engineMap, "menu.controls.wii.classic.buttonY",     &g_controlsStrings.wiiButtonY);
#endif // #ifdef CONTROLS_WII_STRINGS

    insert(m_engineMap, "menu.wordNo",         &g_mainMenu.wordNo);
    insert(m_engineMap, "menu.wordYes",        &g_mainMenu.wordYes);
    insert(m_engineMap, "menu.caseNone",       &g_mainMenu.caseNone);
    insert(m_engineMap, "menu.wordOn",         &g_mainMenu.wordOn);
    insert(m_engineMap, "menu.wordOff",        &g_mainMenu.wordOff);
    insert(m_engineMap, "menu.wordShow",       &g_mainMenu.wordShow);
    insert(m_engineMap, "menu.wordHide",       &g_mainMenu.wordHide);
    insert(m_engineMap, "menu.wordPlayer",     &g_mainMenu.wordPlayer);
    insert(m_engineMap, "menu.wordProfile",    &g_mainMenu.wordProfile);
    insert(m_engineMap, "menu.wordBack",       &g_mainMenu.wordBack);
    insert(m_engineMap, "menu.wordResume",     &g_mainMenu.wordResume);
    insert(m_engineMap, "menu.wordWaiting",    &g_mainMenu.wordWaiting);

    insert(m_engineMap, "menu.abbrevMilliseconds", &g_mainMenu.abbrevMilliseconds);


    insert(m_engineMap, "outro.gameCredits",           &g_outroScreen.gameCredits);
    insert(m_engineMap, "outro.engineCredits",         &g_outroScreen.engineCredits);
    insert(m_engineMap, "outro.originalBy",            &g_outroScreen.originalBy);
    insert(m_engineMap, "outro.nameAndrewSpinks",      &g_outroScreen.nameAndrewSpinks);
    insert(m_engineMap, "outro.cppPortDevelopers",     &g_outroScreen.cppPortDevelopers);
    insert(m_engineMap, "outro.nameVitalyNovichkov",   &g_outroScreen.nameVitalyNovichkov);
    insert(m_engineMap, "outro.qualityControl",        &g_outroScreen.qualityControl);
    insert(m_engineMap, "outro.psVitaPortBy",          &g_outroScreen.psVitaPortBy);
    insert(m_engineMap, "outro.levelDesign",           &g_outroScreen.levelDesign);
    insert(m_engineMap, "outro.customSprites",         &g_outroScreen.customSprites);
    insert(m_engineMap, "outro.specialThanks",         &g_outroScreen.specialThanks);


    insert(m_engineMap, "game.msgbox.sysInfoTitle",                &g_gameStrings.msgBoxTitleInfo);
    insert(m_engineMap, "game.msgbox.sysInfoWarning",              &g_gameStrings.msgBoxTitleWarning);
    insert(m_engineMap, "game.msgbox.sysInfoError",                &g_gameStrings.msgBoxTitleError);

    insert(m_engineMap, "game.loader.statusLoadData",              &g_gameStrings.loaderStatusLoadData);
    insert(m_engineMap, "game.loader.statusLoadFile",              &g_gameStrings.loaderStatusLoadFile);
    insert(m_engineMap, "game.loader.statusGameInfo",              &g_gameStrings.loaderStatusGameInfo);
    insert(m_engineMap, "game.loader.statusTranslations",          &g_gameStrings.loaderStatusTranslations);
    insert(m_engineMap, "game.loader.statusAssetPacks",            &g_gameStrings.loaderStatusAssetPacks);
    insert(m_engineMap, "game.loader.statusFinishing",             &g_gameStrings.loaderStatusFinishing);

    insert(m_engineMap, "game.error.openFileFailed",               &g_gameStrings.errorOpenFileFailed);
#if defined(THEXTECH_INTERPROC_SUPPORTED) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    insert(m_engineMap, "game.error.openIPCDataFailed",            &g_gameStrings.errorOpenIPCDataFailed);
#endif
    insert(m_engineMap, "game.error.errorTooOldEngine",            &g_gameStrings.errorTooOldEngine);
    insert(m_engineMap, "game.error.errorTooOldGameAssets",        &g_gameStrings.errorTooOldGameAssets);
    insert(m_engineMap, "game.error.errorInvalidEnterWarp",        &g_gameStrings.errorInvalidEnterWarp);
    insert(m_engineMap, "game.error.errorNoStartPoint",            &g_gameStrings.errorNoStartPoint);
#if defined(THEXTECH_INTERPROC_SUPPORTED) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    insert(m_engineMap, "game.error.IPCTimeOut",                   &g_gameStrings.errorIPCTimeOut);
#endif

    insert(m_engineMap, "game.error.warpNeedStarCount",            &g_gameStrings.warpNeedStarCount);

    insert(m_engineMap, "game.message.scanningLevels",             &g_gameStrings.messageScanningLevels);
    insert(m_engineMap, "game.format.minutesSeconds",              &g_gameStrings.formatMinutesSeconds);

#if defined(THEXTECH_INTERPROC_SUPPORTED) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    insert(m_engineMap, "game.ipcStatus.waitingInput",             &g_gameStrings.ipcStatusWaitingInput);
    insert(m_engineMap, "game.ipcStatus.dataTransferStarted",      &g_gameStrings.ipcStatusDataTransferStarted);
    insert(m_engineMap, "game.ipcStatus.dataAccepted",             &g_gameStrings.ipcStatusDataAccepted);
    insert(m_engineMap, "game.ipcStatus.dataValid",                &g_gameStrings.ipcStatusDataValid);
    insert(m_engineMap, "game.ipcStatus.errorTimeout",             &g_gameStrings.ipcStatusErrorTimeout);
    insert(m_engineMap, "game.ipcStatus.loadingdone",              &g_gameStrings.ipcStatusLoadingDone);
#endif

#if !defined(NO_WINDOW_FOCUS_TRACKING) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    insert(m_engineMap, "game.screenPaused",               &g_gameStrings.screenPaused);
#endif

    insert(m_engineMap, "game.pause.continue",             &g_gameStrings.pauseItemContinue);
    insert(m_engineMap, "game.pause.restartLevel",         &g_gameStrings.pauseItemRestartLevel);
    insert(m_engineMap, "game.pause.resetCheckpoints",     &g_gameStrings.pauseItemResetCheckpoints);
    insert(m_engineMap, "game.pause.quitTesting",          &g_gameStrings.pauseItemQuitTesting);
    insert(m_engineMap, "game.pause.returnToEditor",       &g_gameStrings.pauseItemReturnToEditor);
    insert(m_engineMap, "game.pause.playerSetup",          &g_gameStrings.pauseItemPlayerSetup);
    insert(m_engineMap, "game.pause.enterCode",            &g_gameStrings.pauseItemEnterCode);
    insert(m_engineMap, "game.pause.saveAndContinue",      &g_gameStrings.pauseItemSaveAndContinue);
    insert(m_engineMap, "game.pause.saveAndQuit",          &g_gameStrings.pauseItemSaveAndQuit);
    insert(m_engineMap, "game.pause.quit",                 &g_gameStrings.pauseItemQuit);

    insert(m_engineMap, "game.connect.reconnectTitle",            &g_gameStrings.connectReconnectTitle);

    insert(m_engineMap, "game.connect.phrasePressAButton",        &g_gameStrings.connectPressAButton);

    insert(m_engineMap, "game.connect.phraseTestProfile",        &g_gameStrings.connectTestProfile);
    insert(m_engineMap, "game.connect.phraseHoldStart",           &g_gameStrings.connectHoldStart);
    insert(m_engineMap, "game.connect.wordDisconnect",            &g_gameStrings.connectDisconnect);

    insert(m_engineMap, "game.connect.phraseForceResume",         &g_gameStrings.connectForceResume);
    insert(m_engineMap, "game.connect.phraseDropPX",              &g_gameStrings.connectDropPX);

    insert(m_engineMap, "game.connect.phraseWaitingForInput",     &g_gameStrings.connectWaitingForInputDevice);
    insert(m_engineMap, "game.connect.splitPressSelect_1",        &g_gameStrings.connectPressSelectForControlsOptions_P1);
    insert(m_engineMap, "game.connect.splitPressSelect_2",        &g_gameStrings.connectPressSelectForControlsOptions_P2);

    insert(m_engineMap, "game.connect.phraseDropMe",              &g_gameStrings.connectDropMe);

#if defined(THEXTECH_ENABLE_EDITOR) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    insert(m_engineMap, "editor.block.pickContents",       &g_editorStrings.pickBlockContents);

    insert(m_engineMap, "editor.block.letterWidth",        &g_editorStrings.blockLetterWidth);
    insert(m_engineMap, "editor.block.letterHeight",       &g_editorStrings.blockLetterHeight);
    insert(m_engineMap, "editor.block.canBreak",           &g_editorStrings.blockCanBreak);
    insert(m_engineMap, "editor.block.canBreakTooltip",    &g_editorStrings.blockTooltipCanBreak);
    insert(m_engineMap, "editor.block.slick",              &g_editorStrings.blockSlick);
    insert(m_engineMap, "editor.block.invis",              &g_editorStrings.blockInvis);
    insert(m_engineMap, "editor.block.inside",             &g_editorStrings.blockInside);

    insert(m_engineMap, "editor.warp.title",               &g_editorStrings.warpTitle);
    insert(m_engineMap, "editor.warp.placing",             &g_editorStrings.warpPlacing);
    insert(m_engineMap, "editor.warp.in",                  &g_editorStrings.warpIn);
    insert(m_engineMap, "editor.warp.out",                 &g_editorStrings.warpOut);
    insert(m_engineMap, "editor.warp.dir",                 &g_editorStrings.warpDir);
    insert(m_engineMap, "editor.warp.twoWay",              &g_editorStrings.warpTwoWay);
    insert(m_engineMap, "editor.warp.style.style",         &g_editorStrings.warpStyle);
    insert(m_engineMap, "editor.warp.style.pipe",          &g_editorStrings.warpStylePipe);
    insert(m_engineMap, "editor.warp.style.door",          &g_editorStrings.warpStyleDoor);
    insert(m_engineMap, "editor.warp.style.blipInstant",   &g_editorStrings.warpStyleBlipInstant);
    insert(m_engineMap, "editor.warp.style.portal",        &g_editorStrings.warpStylePortal);
    insert(m_engineMap, "editor.warp.effect",              &g_editorStrings.warpEffect);
    insert(m_engineMap, "editor.warp.allow",               &g_editorStrings.warpAllow);
    insert(m_engineMap, "editor.warp.item",                &g_editorStrings.warpItem);
    insert(m_engineMap, "editor.warp.ride",                &g_editorStrings.warpRide);
    insert(m_engineMap, "editor.warp.cannonExit",          &g_editorStrings.warpCannonExit);
    insert(m_engineMap, "editor.warp.speed",               &g_editorStrings.warpSpeed);
    insert(m_engineMap, "editor.warp.needStarCount",       &g_editorStrings.warpNeedStarCount);
    insert(m_engineMap, "editor.warp.needKey",             &g_editorStrings.warpNeedKey);
    insert(m_engineMap, "editor.warp.needFloor",           &g_editorStrings.warpNeedFloor);
    insert(m_engineMap, "editor.warp.starLockMessage",     &g_editorStrings.warpStarLockMessage);
    insert(m_engineMap, "editor.warp.toMap",               &g_editorStrings.warpToMap);
    insert(m_engineMap, "editor.warp.lvlWarp",             &g_editorStrings.warpLvlWarp);
    insert(m_engineMap, "editor.warp.target",              &g_editorStrings.warpTarget);
    insert(m_engineMap, "editor.warp.to",                  &g_editorStrings.warpTo);
    insert(m_engineMap, "editor.warp.showStartScene",      &g_editorStrings.warpShowStartScene);
    insert(m_engineMap, "editor.warp.showStarCount",       &g_editorStrings.warpShowStarCount);

    insert(m_engineMap, "editor.water.title",              &g_editorStrings.waterTitle);

    insert(m_engineMap, "editor.npc.inContainer",          &g_editorStrings.npcInContainer);
    insert(m_engineMap, "editor.npc.inertNice",            &g_editorStrings.npcInertNice);
    insert(m_engineMap, "editor.npc.stuckStop",            &g_editorStrings.npcStuckStop);
    insert(m_engineMap, "editor.npc.props.active",         &g_editorStrings.npcPropertyActive);
    insert(m_engineMap, "editor.npc.props.attachSurface",  &g_editorStrings.npcPropertyAttachSurface);
    insert(m_engineMap, "editor.npc.props.facing",         &g_editorStrings.npcPropertyFacing);
    insert(m_engineMap, "editor.npc.abbrevGen",            &g_editorStrings.npcAbbrevGen);

    insert(m_engineMap, "editor.npc.ai.aiIs",              &g_editorStrings.npcAiIs);
    insert(m_engineMap, "editor.npc.ai.target",            &g_editorStrings.npcAiTarget);
    insert(m_engineMap, "editor.npc.ai.jump",              &g_editorStrings.npcAiJump);
    insert(m_engineMap, "editor.npc.ai.leap",              &g_editorStrings.npcAiLeap);
    insert(m_engineMap, "editor.npc.ai.swim",              &g_editorStrings.npcAiSwim);
    insert(m_engineMap, "editor.npc.ai.LR",                &g_editorStrings.npcAiLR);
    insert(m_engineMap, "editor.npc.ai.UD",                &g_editorStrings.npcAiUD);

    insert(m_engineMap, "editor.npc.ai.headerCustomAi",    &g_editorStrings.npcCustomAi);

    insert(m_engineMap, "editor.npc.ai.use1_0Ai",          &g_editorStrings.npcUse1_0Ai);
    insert(m_engineMap, "editor.npc.tooltipExpandSection", &g_editorStrings.npcTooltipExpandSection);

    insert(m_engineMap, "editor.npc.gen.header",           &g_editorStrings.npcGenHeader);
    insert(m_engineMap, "editor.npc.gen.direction",        &g_editorStrings.npcGenDirection);
    insert(m_engineMap, "editor.npc.gen.effectIs",         &g_editorStrings.npcGenEffectIs);
    insert(m_engineMap, "editor.npc.gen.effectWarp",       &g_editorStrings.npcGenEffectWarp);
    insert(m_engineMap, "editor.npc.gen.effectShoot",      &g_editorStrings.npcGenEffectShoot);

    insert(m_engineMap, "editor.wordNPC.nominative",       &g_editorStrings.wordNPC);
    insert(m_engineMap, "editor.wordNPC.genitive",         &g_editorStrings.wordNPCGenitive);

    insert(m_engineMap, "editor.wordEvent.nominative",     &g_editorStrings.wordEvent);
    insert(m_engineMap, "editor.wordEvent.genitive",       &g_editorStrings.wordEventGenitive);
    insert(m_engineMap, "editor.wordEvent.typeLabel",      &g_editorStrings.phraseTypeLabelEvent);

    insert(m_engineMap, "editor.wordCoins",                &g_editorStrings.wordCoins);

    insert(m_engineMap, "editor.wordEnabled",              &g_editorStrings.wordEnabled);
    insert(m_engineMap, "editor.wordText",                 &g_editorStrings.wordText);
    insert(m_engineMap, "editor.wordInstant",              &g_editorStrings.wordInstant);
    insert(m_engineMap, "editor.wordMode",                 &g_editorStrings.wordMode);
    insert(m_engineMap, "editor.wordHeight",               &g_editorStrings.wordHeight);
    insert(m_engineMap, "editor.wordWidth",                &g_editorStrings.wordWidth);

    insert(m_engineMap, "editor.labelSortLayer",           &g_editorStrings.labelSortLayer);
    insert(m_engineMap, "editor.labelSortOffset",          &g_editorStrings.labelSortOffset);

    insert(m_engineMap, "editor.phraseTextOf",             &g_editorStrings.phraseTextOf);
    insert(m_engineMap, "editor.phraseSectionIndex",       &g_editorStrings.phraseSectionIndex);
    insert(m_engineMap, "editor.phraseRadiusIndex",        &g_editorStrings.phraseRadiusIndex);
    insert(m_engineMap, "editor.phraseWarpIndex",          &g_editorStrings.phraseWarpIndex);
    insert(m_engineMap, "editor.phraseGenericIndex",       &g_editorStrings.phraseGenericIndex);
    insert(m_engineMap, "editor.phraseDelayIsMs",          &g_editorStrings.phraseDelayIsMs);
    insert(m_engineMap, "editor.phraseCountMore",          &g_editorStrings.phraseCountMore);
    insert(m_engineMap, "editor.mapPos",                   &g_editorStrings.mapPos);
    insert(m_engineMap, "editor.phraseAreYouSure",         &g_editorStrings.phraseAreYouSure);
    insert(m_engineMap, "editor.pageBlankOfBlank",         &g_editorStrings.pageBlankOfBlank);

    insert(m_engineMap, "editor.letterUp",                 &g_editorStrings.letterUp);
    insert(m_engineMap, "editor.letterDown",               &g_editorStrings.letterDown);
    insert(m_engineMap, "editor.letterLeft",               &g_editorStrings.letterLeft);
    insert(m_engineMap, "editor.letterRight",              &g_editorStrings.letterRight);
    insert(m_engineMap, "editor.letterCoordX",             &g_editorStrings.letterCoordX);
    insert(m_engineMap, "editor.letterCoordY",             &g_editorStrings.letterCoordY);

    insert(m_engineMap, "editor.toggleMagicBlock",         &g_editorStrings.toggleMagicBlock);

    insert(m_engineMap, "editor.testPlay.magicHand",       &g_editorStrings.testMagicHand);
    insert(m_engineMap, "editor.testPlay.char",            &g_editorStrings.testChar);
    insert(m_engineMap, "editor.testPlay.power",           &g_editorStrings.testPower);
    insert(m_engineMap, "editor.testPlay.boot",            &g_editorStrings.testBoot);
    insert(m_engineMap, "editor.testPlay.pet",             &g_editorStrings.testPet);

    insert(m_engineMap, "editor.events.header",            &g_editorStrings.eventsHeader);

    insert(m_engineMap, "editor.events.letter.activate",    &g_editorStrings.eventsLetterActivate);
    insert(m_engineMap, "editor.events.letter.death",       &g_editorStrings.eventsLetterDeath);
    insert(m_engineMap, "editor.events.letter.talk",        &g_editorStrings.eventsLetterTalk);
    insert(m_engineMap, "editor.events.letter.layerClear",  &g_editorStrings.eventsLetterLayerClear);
    insert(m_engineMap, "editor.events.letter.hit",         &g_editorStrings.eventsLetterHit);
    insert(m_engineMap, "editor.events.letter.destroy",     &g_editorStrings.eventsLetterDestroy);
    insert(m_engineMap, "editor.events.letter.enter",       &g_editorStrings.eventsLetterEnter);

    insert(m_engineMap, "editor.events.label.next",        &g_editorStrings.eventsLabelNext);
    insert(m_engineMap, "editor.events.label.activate",    &g_editorStrings.eventsLabelActivate);
    insert(m_engineMap, "editor.events.label.death",       &g_editorStrings.eventsLabelDeath);
    insert(m_engineMap, "editor.events.label.talk",        &g_editorStrings.eventsLabelTalk);
    insert(m_engineMap, "editor.events.label.layerClear",  &g_editorStrings.eventsLabelLayerClear);
    insert(m_engineMap, "editor.events.label.hit",         &g_editorStrings.eventsLabelHit);
    insert(m_engineMap, "editor.events.label.destroy",     &g_editorStrings.eventsLabelDestroy);
    insert(m_engineMap, "editor.events.label.enter",       &g_editorStrings.eventsLabelEnter);

    insert(m_engineMap, "editor.events.desc.activate",     &g_editorStrings.eventsDescActivate);
    insert(m_engineMap, "editor.events.desc.death",        &g_editorStrings.eventsDescDeath);
    insert(m_engineMap, "editor.events.desc.talk",         &g_editorStrings.eventsDescTalk);
    insert(m_engineMap, "editor.events.desc.layerClear",   &g_editorStrings.eventsDescLayerClear);
    insert(m_engineMap, "editor.events.desc.hit",          &g_editorStrings.eventsDescHit);
    insert(m_engineMap, "editor.events.desc.destroy",      &g_editorStrings.eventsDescDestroy);
    insert(m_engineMap, "editor.events.desc.enter",        &g_editorStrings.eventsDescEnter);

    insert(m_engineMap, "editor.events.desc.phraseTriggersWhenTemplate",   &g_editorStrings.eventsDescPhraseTriggersWhenTemplate);
    insert(m_engineMap, "editor.events.desc.phraseTriggersAfterTemplate",   &g_editorStrings.eventsDescPhraseTriggersAfterTemplate);

    insert(m_engineMap, "editor.events.deletion.deletingEvent",    &g_editorStrings.eventsDeletingEvent);
    insert(m_engineMap, "editor.events.deletion.confirm",          &g_editorStrings.eventsDeletionConfirm);
    insert(m_engineMap, "editor.events.deletion.cancel",           &g_editorStrings.eventsDeletionCancel);

    insert(m_engineMap, "editor.events.promptEventText",  &g_editorStrings.eventsPromptEventText);
    insert(m_engineMap, "editor.events.promptEventName",  &g_editorStrings.eventsPromptEventName);
    insert(m_engineMap, "editor.events.itemNewEvent",     &g_editorStrings.eventsItemNewEvent);

    insert(m_engineMap, "editor.events.controlsForEventN",      &g_editorStrings.eventsControlsForEvent);
    insert(m_engineMap, "editor.events.settingsForEvent",       &g_editorStrings.eventsSettingsForEvent);

    insert(m_engineMap, "editor.events.layers.headerShow",      &g_editorStrings.eventsHeaderShow);
    insert(m_engineMap, "editor.events.layers.headerHide",      &g_editorStrings.eventsHeaderHide);
    insert(m_engineMap, "editor.events.layers.headerToggle",    &g_editorStrings.eventsHeaderToggle);
    insert(m_engineMap, "editor.events.layers.headerMove",      &g_editorStrings.eventsHeaderMove);

    insert(m_engineMap, "editor.events.sections.actionKeep",    &g_editorStrings.eventsActionKeep);
    insert(m_engineMap, "editor.events.sections.actionReset",   &g_editorStrings.eventsActionReset);
    insert(m_engineMap, "editor.events.sections.actionSet",     &g_editorStrings.eventsActionSet);

    insert(m_engineMap, "editor.events.sections.propMusic",     &g_editorStrings.eventsCaseMusic);
    insert(m_engineMap, "editor.events.sections.propBackground", &g_editorStrings.eventsCaseBackground);
    insert(m_engineMap, "editor.events.sections.propBounds",    &g_editorStrings.eventsCaseBounds);

    insert(m_engineMap, "editor.events.sections.phraseAllSections",     &g_editorStrings.eventsPhraseAllSections);

    insert(m_engineMap, "editor.events.props.autostart",     &g_editorStrings.eventsPropAutostart);
    insert(m_engineMap, "editor.events.props.sound",         &g_editorStrings.eventsPropSound);
    insert(m_engineMap, "editor.events.props.endGame",       &g_editorStrings.eventsPropEndGame);
    insert(m_engineMap, "editor.events.props.controls",      &g_editorStrings.eventsPropControls);
    insert(m_engineMap, "editor.events.props.layerSmoke",    &g_editorStrings.eventsPropLayerSmoke);

    insert(m_engineMap, "editor.events.headerTriggerEvent",  &g_editorStrings.eventsHeaderTriggerEvent);

    insert(m_engineMap, "editor.level.levelName",            &g_editorStrings.levelName);
    insert(m_engineMap, "editor.level.startPos",             &g_editorStrings.levelStartPos);
    insert(m_engineMap, "editor.level.pathBG",               &g_editorStrings.levelPathBG);
    insert(m_engineMap, "editor.level.bigBG",                &g_editorStrings.levelBigBG);
    insert(m_engineMap, "editor.level.gameStart",            &g_editorStrings.levelGameStart);
    insert(m_engineMap, "editor.level.alwaysVis",            &g_editorStrings.levelAlwaysVis);
    insert(m_engineMap, "editor.level.pathUnlocks",          &g_editorStrings.levelPathUnlocks);

    insert(m_engineMap, "editor.section.scroll",             &g_editorStrings.sectionScroll);
    insert(m_engineMap, "editor.section.horizWrap",          &g_editorStrings.sectionHorizWrap);
    insert(m_engineMap, "editor.section.vertWrap",           &g_editorStrings.sectionVertWrap);
    insert(m_engineMap, "editor.section.underwater",         &g_editorStrings.sectionUnderwater);
    insert(m_engineMap, "editor.section.noTurnBack",         &g_editorStrings.sectionNoTurnBack);
    insert(m_engineMap, "editor.section.offscreenExit",      &g_editorStrings.sectionOffscreenExit);

    insert(m_engineMap, "editor.world.name",                 &g_editorStrings.worldName);
    insert(m_engineMap, "editor.world.introLevel",           &g_editorStrings.worldIntroLevel);
    insert(m_engineMap, "editor.world.hubWorld",             &g_editorStrings.worldHubWorld);
    insert(m_engineMap, "editor.world.retryOnFail",          &g_editorStrings.worldRetryOnFail);
    insert(m_engineMap, "editor.world.totalStars",           &g_editorStrings.worldTotalStars);
    insert(m_engineMap, "editor.world.allowChars",           &g_editorStrings.worldAllowChars);
    insert(m_engineMap, "editor.world.phraseCreditIndex",    &g_editorStrings.worldCreditIndex);

    insert(m_engineMap, "editor.select.soundForEventN",              &g_editorStrings.selectSoundForEvent);
    insert(m_engineMap, "editor.select.sectBlankPropBlankForEventN", &g_editorStrings.selectSectBlankPropBlankForEvent);
    insert(m_engineMap, "editor.select.allSectPropBlankForEventN",   &g_editorStrings.selectAllSectPropBlankForEvent);
    insert(m_engineMap, "editor.select.sectionBlankPropBlank",     &g_editorStrings.selectSectionBlankPropBlank);
    insert(m_engineMap, "editor.select.pathBlankUnlock",           &g_editorStrings.selectPathBlankUnlock);
    insert(m_engineMap, "editor.select.warpTransitEffect",         &g_editorStrings.selectWarpTransitionEffect);
    insert(m_engineMap, "editor.select.worldMusic",                &g_editorStrings.selectWorldMusic);

    insert(m_engineMap, "editor.layers.header",              &g_editorStrings.layersHeader);

    insert(m_engineMap, "editor.layers.label",               &g_editorStrings.labelLayer);
    insert(m_engineMap, "editor.layers.labelAttached",       &g_editorStrings.layersLabelAttached);
    insert(m_engineMap, "editor.layers.abbrevAttLayer",      &g_editorStrings.layersAbbrevAttLayer);
    insert(m_engineMap, "editor.layers.default",             &g_editorStrings.layersLayerDefault);

    insert(m_engineMap, "editor.layers.labelAttachedLayer",  &g_editorStrings.layersLabelAttachedLayer);
    insert(m_engineMap, "editor.layers.labelMoveLayer",      &g_editorStrings.layersLabelMoveLayer);

    insert(m_engineMap, "editor.layers.deletion.header",                   &g_editorStrings.layersDeletionHeader);
    insert(m_engineMap, "editor.layers.deletion.preserveLayerContents",    &g_editorStrings.layersDeletionPreserveLayerContents);
    insert(m_engineMap, "editor.layers.deletion.confirmPreserve",          &g_editorStrings.layersDeletionConfirmPreserve);
    insert(m_engineMap, "editor.layers.deletion.confirmDelete",            &g_editorStrings.layersDeletionConfirmDelete);
    insert(m_engineMap, "editor.layers.deletion.cancel",                   &g_editorStrings.layersDeletionCancel);

    insert(m_engineMap, "editor.layers.desc.att", &g_editorStrings.layersDescAtt);

    insert(m_engineMap, "editor.layers.promptLayerName", &g_editorStrings.layersPromptLayerName);
    insert(m_engineMap, "editor.layers.itemNewLayer",    &g_editorStrings.layersItemNewLayer);

    insert(m_engineMap, "editor.file.actionClearLevel",            &g_editorStrings.fileActionClearLevel);
    insert(m_engineMap, "editor.file.actionClearWorld",            &g_editorStrings.fileActionClearWorld);
    insert(m_engineMap, "editor.file.actionOpen",                  &g_editorStrings.fileActionOpen);
    insert(m_engineMap, "editor.file.actionRevert",                &g_editorStrings.fileActionRevert);
    insert(m_engineMap, "editor.file.actionExit",                  &g_editorStrings.fileActionExit);
    insert(m_engineMap, "editor.file.confirmSaveBeforeAction",     &g_editorStrings.fileConfirmationSaveBeforeAction);
    insert(m_engineMap, "editor.file.confirmConfirmAction",        &g_editorStrings.fileConfirmationConfirmAction);
    insert(m_engineMap, "editor.file.confirmConvertFormatTo",      &g_editorStrings.fileConfirmationConvertFormatTo);
    insert(m_engineMap, "editor.file.optionYesSaveThenAction",     &g_editorStrings.fileOptionYesSaveThenAction);
    insert(m_engineMap, "editor.file.optionActionWithoutSave",     &g_editorStrings.fileOptionActionWithoutSave);
    insert(m_engineMap, "editor.file.optionCancelAction",          &g_editorStrings.fileOptionCancelAction);
    insert(m_engineMap, "editor.file.optionProceedWithConversion", &g_editorStrings.fileOptionProceedWithConversion);
    insert(m_engineMap, "editor.file.optionCancelConversion",      &g_editorStrings.fileOptionCancelConversion);

    insert(m_engineMap, "editor.file.labelCurrentFile",    &g_editorStrings.fileLabelCurrentFile);
    insert(m_engineMap, "editor.file.labelFormat",         &g_editorStrings.fileLabelFormat);

    insert(m_engineMap, "editor.file.formatModern",        &g_editorStrings.fileFormatModern);
    insert(m_engineMap, "editor.file.formatLegacy",        &g_editorStrings.fileFormatLegacy);

    insert(m_engineMap, "editor.file.sectionLevel",        &g_editorStrings.fileSectionLevel);
    insert(m_engineMap, "editor.file.sectionWorld",        &g_editorStrings.fileSectionWorld);
    insert(m_engineMap, "editor.file.commandNew",          &g_editorStrings.fileCommandNew);
    insert(m_engineMap, "editor.file.commandOpen",         &g_editorStrings.fileCommandOpen);
    insert(m_engineMap, "editor.file.commandSave",         &g_editorStrings.fileCommandSave);
    insert(m_engineMap, "editor.file.commandSaveAs",       &g_editorStrings.fileCommandSaveAs);

    insert(m_engineMap, "editor.file.convert.descNew",             &g_editorStrings.fileConvertDesc);

#if 0
    insert(m_engineMap, "editor.file.convert._38aUnsupported",     &g_editorStrings.fileConvert38aUnsupported);
    insert(m_engineMap, "editor.file.convert.formatUnknown",       &g_editorStrings.fileConvertFormatUnknown);

    insert(m_engineMap, "editor.file.convert.featureWarpTransit",      &g_editorStrings.fileConvertFeatureWarpTransit);
    insert(m_engineMap, "editor.file.convert.featureWarpNeedsStand",   &g_editorStrings.fileConvertFeatureWarpNeedsStand);
    insert(m_engineMap, "editor.file.convert.featureWarpCannonExit",   &g_editorStrings.fileConvertFeatureWarpCannonExit);
    insert(m_engineMap, "editor.file.convert.featureWarpEnterEvent",   &g_editorStrings.fileConvertFeatureWarpEnterEvent);
    insert(m_engineMap, "editor.file.convert.featureWarpCustomStarsMsg", &g_editorStrings.fileConvertFeatureWarpCustomStarsMsg);
    insert(m_engineMap, "editor.file.convert.featureWarpNoPrintStars", &g_editorStrings.fileConvertFeatureWarpNoPrintStars);
    insert(m_engineMap, "editor.file.convert.featureWarpNoStartScene", &g_editorStrings.fileConvertFeatureWarpNoStartScene);
    insert(m_engineMap, "editor.file.convert.featureWarpPortal",       &g_editorStrings.fileConvertFeatureWarpPortal);

    insert(m_engineMap, "editor.file.convert.featureEventCustomMusic", &g_editorStrings.fileConvertFeatureEventCustomMusic);
    insert(m_engineMap, "editor.file.convert.featureEventAutoscroll",  &g_editorStrings.fileConvertFeatureEventAutoscroll);

    insert(m_engineMap, "editor.file.convert.featureNPCVariant",       &g_editorStrings.fileConvertFeatureNPCVariant);
    insert(m_engineMap, "editor.file.convert.featureBlockForceSmashable", &g_editorStrings.fileConvertFeatureBlockForceSmashable);
    insert(m_engineMap, "editor.file.convert.featureBgoOrder",         &g_editorStrings.fileConvertFeatureBgoOrder);

    insert(m_engineMap, "editor.file.convert.featureCustomWorldMusic", &g_editorStrings.fileConvertFeatureCustomWorldMusic);
    insert(m_engineMap, "editor.file.convert.featureWorldStarDisplay", &g_editorStrings.fileConvertFeatureWorldStarDisplay);
    insert(m_engineMap, "editor.file.convert.featureLevelStarDisplay", &g_editorStrings.fileConvertFeatureLevelStarDisplay);
    insert(m_engineMap, "editor.file.convert.featureWorldMapSections", &g_editorStrings.fileConvertFeatureWorldMapSections);
#endif

    insert(m_engineMap, "editor.browser.newFile",          &g_editorStrings.browserNewFile);
    insert(m_engineMap, "editor.browser.saveFile",         &g_editorStrings.browserSaveFile);
    insert(m_engineMap, "editor.browser.openFile",         &g_editorStrings.browserOpenFile);

    insert(m_engineMap, "editor.browser.itemNewFile",      &g_editorStrings.browserItemNewFile);
    insert(m_engineMap, "editor.browser.itemNewFolder",    &g_editorStrings.browserItemNewFolder);

    insert(m_engineMap, "editor.browser.askOverwriteFile", &g_editorStrings.browserAskOverwriteFile);

    insert(m_engineMap, "editor.tooltip.select",   &g_editorStrings.tooltipSelect);
    insert(m_engineMap, "editor.tooltip.erase",    &g_editorStrings.tooltipErase);
    insert(m_engineMap, "editor.tooltip.eraseAll", &g_editorStrings.tooltipEraseAll);
    insert(m_engineMap, "editor.tooltip.blocks",   &g_editorStrings.tooltipBlocks);
    insert(m_engineMap, "editor.tooltip.BGOs",     &g_editorStrings.tooltipBGOs);
    insert(m_engineMap, "editor.tooltip.NPCs",     &g_editorStrings.tooltipNPCs);
    insert(m_engineMap, "editor.tooltip.warps",    &g_editorStrings.tooltipWarps);
    insert(m_engineMap, "editor.tooltip.water",    &g_editorStrings.tooltipWater);
    insert(m_engineMap, "editor.tooltip.settings", &g_editorStrings.tooltipSettings);
    insert(m_engineMap, "editor.tooltip.layers",   &g_editorStrings.tooltipLayers);
    insert(m_engineMap, "editor.tooltip.events",   &g_editorStrings.tooltipEvents);
    insert(m_engineMap, "editor.tooltip.tiles",    &g_editorStrings.tooltipTiles);
    insert(m_engineMap, "editor.tooltip.scenes",   &g_editorStrings.tooltipScenes);
    insert(m_engineMap, "editor.tooltip.levels",   &g_editorStrings.tooltipLevels);
    insert(m_engineMap, "editor.tooltip.paths",    &g_editorStrings.tooltipPaths);
    insert(m_engineMap, "editor.tooltip.music",    &g_editorStrings.tooltipMusic);
    insert(m_engineMap, "editor.tooltip.area",     &g_editorStrings.tooltipArea);
    insert(m_engineMap, "editor.tooltip.file",     &g_editorStrings.tooltipFile);
    insert(m_engineMap, "editor.tooltip.show",     &g_editorStrings.tooltipShow);

#endif // THEXTECH_ENABLE_EDITOR
    // };

#ifdef THEXTECH_ENABLE_EDITOR
    for(int i = 0; i < 6; ++i)
    {
        insert(m_engineMap, fmt::format_ne("editor.warp.transit.name{0}", i), &g_editorStrings.listWarpTransitNames[i]);
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
        insert(m_assetsMap, fmt::format_ne("character.name{0}", i), &g_gameInfo.characterName[i]);

    // reset all strings for options to hardcoded defaults
    g_options.reset_options();

    // add config fields to engine map
    g_options.make_translation(*this);

#ifdef THEXTECH_ENABLE_EDITOR
    // adds dynamic fields to the asset map
    EditorCustom::Load(this);
#endif

    XHints::InitTranslations(*this);

#if TR_MAP_TYPE == 1 && !defined(THEXTECH_DISABLE_LANG_TOOLS)
    tinysort(m_engineMap.begin(), m_engineMap.end(),
        [](const TrEntry &a, const TrEntry &b)->bool
        {
            return a.first.compare(b.first) < 0;
        });

    tinysort(m_assetsMap.begin(), m_assetsMap.end(),
        [](const TrEntry &a, const TrEntry &b)->bool
        {
            return a.first.compare(b.first) < 0;
        });
#endif
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

