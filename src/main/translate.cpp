/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
static void setJsonValue(nlohmann::json &j, const std::string &key, const std::string &value)
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

static bool setJsonValueIfNotExist(nlohmann::json &j, const std::string &key, const std::string &value, bool noBlank)
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
            else
                j.erase(key);
            return true;
        }
        return false; // Nothing changed
    }

    std::string subKey = key.substr(0, dot);
    return setJsonValueIfNotExist(j[subKey], key.substr(dot + 1), value, noBlank);
}
#endif

static std::string getJsonValue(nlohmann::json &j, const std::string &key, const std::string &defVal)
{
    auto dot = key.find(".");
    if(dot == std::string::npos)
    {
        if(!j.contains(key))
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
    FILE *in = Files::utf8_fopen(inPath.c_str(), "w");
    if(!in)
        return false;

    std::fprintf(in, "%s\n", inData.c_str());
    std::fclose(in);

    return ret;
}
#endif


XTechTranslate::XTechTranslate()
{
    // List of all translatable strings of the engine
    m_engineMap =
    {
        {"menu.main.mainStartGame",        &g_mainMenu.mainStartGame},
        {"menu.main.main1PlayerGame",      &g_mainMenu.main1PlayerGame},
        {"menu.main.mainMultiplayerGame",  &g_mainMenu.mainMultiplayerGame},
        {"menu.main.mainBattleGame",       &g_mainMenu.mainBattleGame},
        {"menu.main.mainEditor",           &g_mainMenu.mainEditor},
        {"menu.main.mainOptions",          &g_mainMenu.mainOptions},
        {"menu.main.mainExit",             &g_mainMenu.mainExit},

        {"menu.loading",                   &g_mainMenu.loading},

        {"languageName",                   &g_mainMenu.languageName},
        {"pluralRules",                    &g_mainMenu.pluralRules},

        {"menu.editor.newWorld",           &g_mainMenu.editorNewWorld},
        {"menu.editor.errorResolution",    &g_mainMenu.editorErrorResolution},
        {"menu.editor.errorMissingResources", &g_mainMenu.editorErrorMissingResources},
        {"menu.editor.promptNewWorldName", &g_mainMenu.editorPromptNewWorldName},

        {"menu.game.gameNoEpisodesToPlay", &g_mainMenu.gameNoEpisodesToPlay},
        {"menu.game.gameNoBattleLevels",   &g_mainMenu.gameNoBattleLevels},
        {"menu.game.gameBattleRandom",     &g_mainMenu.gameBattleRandom},

        {"menu.game.gameSlotContinue",     &g_mainMenu.gameSlotContinue},
        {"menu.game.gameSlotNew",          &g_mainMenu.gameSlotNew},

        {"menu.game.gameCopySave",         &g_mainMenu.gameCopySave},
        {"menu.game.gameEraseSave",        &g_mainMenu.gameEraseSave},

        {"menu.game.gameSourceSlot",       &g_mainMenu.gameSourceSlot},
        {"menu.game.gameTargetSlot",       &g_mainMenu.gameTargetSlot},
        {"menu.game.gameEraseSlot",        &g_mainMenu.gameEraseSlot},

        {"menu.game.phraseScore",          &g_mainMenu.phraseScore},
        {"menu.game.phraseTime",           &g_mainMenu.phraseTime},

        {"menu.battle.errorNoLevels",      &g_mainMenu.errorBattleNoLevels},

        {"menu.options.optionsModeFullScreen",   &g_mainMenu.optionsModeFullScreen},
        {"menu.options.optionsModeWindowed",     &g_mainMenu.optionsModeWindowed},
        {"menu.options.optionsViewCredits",      &g_mainMenu.optionsViewCredits},
        {"menu.options.restartEngine",           &g_mainMenu.optionsRestartEngine},
        {"menu.options.render",                  &g_mainMenu.optionsRender},
        {"menu.options.renderX",                 &g_mainMenu.optionsRenderX},
        {"menu.options.scale.label",             &g_mainMenu.optionsScaleMode},
        {"menu.options.scale.integer",           &g_mainMenu.optionsScaleInteger},
        {"menu.options.scale.nearest",           &g_mainMenu.optionsScaleNearest},
        {"menu.options.scale.linear",            &g_mainMenu.optionsScaleLinear},

        {"menu.character.charSelTitle",    &g_mainMenu.connectCharSelTitle},
        {"menu.character.startGame",       &g_mainMenu.connectStartGame},
        {"menu.character.selectCharacter", &g_mainMenu.selectCharacter},


        {"menu.controls.controlsTitle",     &g_mainMenu.controlsTitle},
        {"menu.controls.controlsConnected", &g_mainMenu.controlsConnected},
        {"menu.controls.controlsDeleteKey", &g_mainMenu.controlsDeleteKey},
        {"menu.controls.controlsDeviceTypes", &g_mainMenu.controlsDeviceTypes},
        {"menu.controls.controlsInUse", &g_mainMenu.controlsInUse},
        {"menu.controls.controlsNotInUse", &g_mainMenu.controlsNotInUse},

        {"menu.controls.wordProfiles", &g_mainMenu.wordProfiles},
        {"menu.controls.wordButtons", &g_mainMenu.wordButtons},

        {"menu.controls.controlsReallyDeleteProfile", &g_mainMenu.controlsReallyDeleteProfile},
        {"menu.controls.controlsNewProfile", &g_mainMenu.controlsNewProfile},
        {"menu.controls.caseInvalid",        &g_controlsStrings.sharedCaseInvalid},

        {"menu.controls.profile.activateProfile", &g_mainMenu.controlsActivateProfile},
        {"menu.controls.profile.renameProfile",   &g_mainMenu.controlsRenameProfile},
        {"menu.controls.profile.deleteProfile",   &g_mainMenu.controlsDeleteProfile},
        {"menu.controls.profile.playerControls",  &g_mainMenu.controlsPlayerControls},
        {"menu.controls.profile.cursorControls",  &g_mainMenu.controlsCursorControls},
        {"menu.controls.profile.editorControls",  &g_mainMenu.controlsEditorControls},
        {"menu.controls.profile.hotkeys",         &g_mainMenu.controlsHotkeys},

        {"menu.controls.options.rumble",            &g_mainMenu.controlsOptionRumble},
        {"menu.controls.options.batteryStatus",     &g_mainMenu.controlsOptionBatteryStatus},
        {"menu.controls.options.groundPoundButton", &g_mainMenu.controlsOptionGroundPoundButton},
        {"menu.controls.options.maxPlayers",        &g_controlsStrings.sharedOptionMaxPlayers},

        {"menu.controls.buttons.up",      &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Up]},
        {"menu.controls.buttons.down",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Down]},
        {"menu.controls.buttons.left",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Left]},
        {"menu.controls.buttons.right",   &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Right]},
        {"menu.controls.buttons.jump",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Jump]},
        {"menu.controls.buttons.run",     &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Run]},
        {"menu.controls.buttons.altJump", &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::AltJump]},
        {"menu.controls.buttons.altRun",  &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::AltRun]},
        {"menu.controls.buttons.start",   &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Start]},
        {"menu.controls.buttons.drop",    &Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Drop]},

        {"menu.controls.cursor.up",        &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorUp]},
        {"menu.controls.cursor.down",      &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorDown]},
        {"menu.controls.cursor.left",      &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorLeft]},
        {"menu.controls.cursor.right",     &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::CursorRight]},
        {"menu.controls.cursor.primary",   &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Primary]},
        {"menu.controls.cursor.secondary", &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Secondary]},
        {"menu.controls.cursor.tertiary",  &Controls::CursorControls::g_button_name_UI[Controls::CursorControls::Tertiary]},

        {"menu.controls.editor.scrollUp",      &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollUp]},
        {"menu.controls.editor.scrollDown",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollDown]},
        {"menu.controls.editor.scrollLeft",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollLeft]},
        {"menu.controls.editor.scrollRight",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ScrollRight]},
        {"menu.controls.editor.fastScroll",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::FastScroll]},
        {"menu.controls.editor.modeSelect",    &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ModeSelect]},
        {"menu.controls.editor.modeErase",     &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::ModeErase]},
        {"menu.controls.editor.prevSection",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::PrevSection]},
        {"menu.controls.editor.nextSection",   &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::NextSection]},
        {"menu.controls.editor.switchScreens", &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::SwitchScreens]},
        {"menu.controls.editor.testPlay",      &Controls::EditorControls::g_button_name_UI[Controls::EditorControls::TestPlay]},

        {"menu.controls.hotkeys.fullscreen",  &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::Fullscreen]},
        {"menu.controls.hotkeys.screenshot",  &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::Screenshot]},
        {"menu.controls.hotkeys.recordGif",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::RecordGif]},
        {"menu.controls.hotkeys.debugInfo",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::DebugInfo]},
        {"menu.controls.hotkeys.enterCheats", &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::EnterCheats]},
        {"menu.controls.hotkeys.toggleHUD",   &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::ToggleHUD]},
        {"menu.controls.hotkeys.legacyPause", &Controls::Hotkeys::g_button_name_UI[Controls::Hotkeys::LegacyPause]},

#ifdef CONTROLS_KEYBOARD_STRINGS
        {"menu.controls.types.keyboard",    &g_controlsStrings.nameKeyboard},
        {"menu.controls.options.textEntryStyle",    &g_controlsStrings.keyboardOptionTextEntryStyle},
        {"menu.controls.caseMouse",         &g_controlsStrings.caseMouse},
#endif

#if defined(CONTROLS_KEYBOARD_STRINGS) || defined(CONTROLS_JOYSTICK_STRINGS)
        {"menu.controls.types.gamepad",     &g_controlsStrings.nameGamepad},
#endif

#if defined(CONTROLS_JOYSTICK_STRINGS)
        {"menu.controls.phraseNewProfOldJoy",&g_controlsStrings.phraseNewProfOldJoy},
        {"menu.controls.types.oldJoystick", &g_controlsStrings.nameOldJoy},
#endif

#if defined(CONTROLS_TOUCHSCREEN_STRINGS)
        {"menu.controls.types.touchscreen", &g_controlsStrings.nameTouchscreen},

        {"menu.controls.caseTouch",         &g_controlsStrings.caseTouch},

        {"menu.controls.touchscreen.option.layoutStyle",    &g_controlsStrings.touchscreenOptionLayoutStyle},
        {"menu.controls.touchscreen.option.scaleFactor",    &g_controlsStrings.touchscreenOptionScaleFactor},
        {"menu.controls.touchscreen.option.scaleDPad",      &g_controlsStrings.touchscreenOptionScaleDPad},
        {"menu.controls.touchscreen.option.scaleButtons",   &g_controlsStrings.touchscreenOptionScaleButtons},
        {"menu.controls.touchscreen.option.sStartSpacing",  &g_controlsStrings.touchscreenOptionSStartSpacing},
        {"menu.controls.touchscreen.option.resetLayout",    &g_controlsStrings.touchscreenOptionResetLayout},
        {"menu.controls.touchscreen.option.interfaceStyle", &g_controlsStrings.touchscreenOptionInterfaceStyle},
        {"menu.controls.touchscreen.option.feedbackStrength", &g_controlsStrings.touchscreenOptionFeedbackStrength},
        {"menu.controls.touchscreen.option.feedbackLength", &g_controlsStrings.touchscreenOptionFeedbackLength},
        {"menu.controls.touchscreen.option.holdRun",        &g_controlsStrings.touchscreenOptionHoldRun},
        {"menu.controls.touchscreen.option.showCodeButton", &g_controlsStrings.touchscreenOptionShowCodeButton},


        {"menu.controls.touchscreen.layout.tight",          &g_controlsStrings.touchscreenLayoutTight},
        {"menu.controls.touchscreen.layout.tinyOld",        &g_controlsStrings.touchscreenLayoutTinyOld},
        {"menu.controls.touchscreen.layout.phoneOld",       &g_controlsStrings.touchscreenLayoutPhoneOld},
        {"menu.controls.touchscreen.layout.longOld",        &g_controlsStrings.touchscreenLayoutLongOld},
        {"menu.controls.touchscreen.layout.phabletOld",     &g_controlsStrings.touchscreenLayoutPhabletOld},
        {"menu.controls.touchscreen.layout.tabletOld",      &g_controlsStrings.touchscreenLayoutTabletOld},
        {"menu.controls.touchscreen.layout.standard",       &g_controlsStrings.touchscreenLayoutStandard},

        {"menu.controls.touchscreen.style.actions",         &g_controlsStrings.touchscreenStyleActions},
        {"menu.controls.touchscreen.style.ABXY",            &g_controlsStrings.touchscreenStyleABXY},
        {"menu.controls.touchscreen.style.XODA",            &g_controlsStrings.touchscreenStyleXODA},
#endif // #if defined(CONTROLS_TOUCHSCREEN_STRINGS)

#if defined(CONTROLS_16M_STRINGS)
        {"menu.controls.tDS.buttonA",      &g_controlsStrings.tdsButtonA},
        {"menu.controls.tDS.buttonB",      &g_controlsStrings.tdsButtonB},
        {"menu.controls.tDS.buttonX",      &g_controlsStrings.tdsButtonX},
        {"menu.controls.tDS.buttonY",      &g_controlsStrings.tdsButtonY},
        {"menu.controls.tDS.buttonL",      &g_controlsStrings.tdsButtonL},
        {"menu.controls.tDS.buttonR",      &g_controlsStrings.tdsButtonR},
        {"menu.controls.tDS.buttonSelect", &g_controlsStrings.tdsButtonSelect},
        {"menu.controls.tDS.buttonStart",  &g_controlsStrings.tdsButtonStart},

        {"menu.controls.tDS.casePen",      &g_controlsStrings.tdsCasePen},
#endif // #ifdef CONTROLS_16M_STRINGS

#if defined(CONTROLS_3DS_STRINGS)
        {"menu.controls.tDS.buttonZL",     &g_controlsStrings.tdsButtonZL},
        {"menu.controls.tDS.buttonZR",     &g_controlsStrings.tdsButtonZR},
        {"menu.controls.tDS.dPad",         &g_controlsStrings.tdsDpad},
        {"menu.controls.tDS.tStick",       &g_controlsStrings.tdsTstick},
        {"menu.controls.tDS.cStick",       &g_controlsStrings.tdsCstick},
#endif // #ifdef CONTROLS_3DS_STRINGS

#if defined(CONTROLS_WII_STRINGS)
        {"menu.controls.wii.typeWiimote",         &g_controlsStrings.wiiTypeWiimote},
        {"menu.controls.wii.typeNunchuck",        &g_controlsStrings.wiiTypeNunchuck},
        {"menu.controls.wii.typeClassic",         &g_controlsStrings.wiiTypeClassic},
        {"menu.controls.wii.phraseNewNunchuck",   &g_controlsStrings.wiiPhraseNewNunchuck},
        {"menu.controls.wii.phraseNewClassic",    &g_controlsStrings.wiiPhraseNewClassic},

        {"menu.controls.wii.wiimote.dPad",        &g_controlsStrings.wiiDpad},
        {"menu.controls.wii.wiimote.buttonA",     &g_controlsStrings.wiiButtonA},
        {"menu.controls.wii.wiimote.buttonB",     &g_controlsStrings.wiiButtonB},
        {"menu.controls.wii.wiimote.buttonMinus", &g_controlsStrings.wiiButtonMinus},
        {"menu.controls.wii.wiimote.buttonPlus",  &g_controlsStrings.wiiButtonPlus},
        {"menu.controls.wii.wiimote.buttonHome",  &g_controlsStrings.wiiButtonHome},
        {"menu.controls.wii.wiimote.button2",     &g_controlsStrings.wiiButton2},
        {"menu.controls.wii.wiimote.button1",     &g_controlsStrings.wiiButton1},
        {"menu.controls.wii.wiimote.shake",       &g_controlsStrings.wiiShake},
        {"menu.controls.wii.wiimote.caseIR",      &g_controlsStrings.wiiCaseIR},

        {"menu.controls.wii.nunchuck.prefixN",    &g_controlsStrings.wiiPrefixNunchuck},
        {"menu.controls.wii.nunchuck.buttonZ",    &g_controlsStrings.wiiButtonZ},
        {"menu.controls.wii.nunchuck.buttonC",    &g_controlsStrings.wiiButtonC},

        {"menu.controls.wii.classic.lStick",      &g_controlsStrings.wiiLStick},
        {"menu.controls.wii.classic.rStick",      &g_controlsStrings.wiiRStick},
        {"menu.controls.wii.classic.buttonZL",    &g_controlsStrings.wiiButtonZL},
        {"menu.controls.wii.classic.buttonZR",    &g_controlsStrings.wiiButtonZR},
        {"menu.controls.wii.classic.buttonLT",    &g_controlsStrings.wiiButtonLT},
        {"menu.controls.wii.classic.buttonRT",    &g_controlsStrings.wiiButtonRT},
        {"menu.controls.wii.classic.buttonX",     &g_controlsStrings.wiiButtonX},
        {"menu.controls.wii.classic.buttonY",     &g_controlsStrings.wiiButtonY},

#endif // #ifdef CONTROLS_WII_STRINGS

        {"menu.wordNo",         &g_mainMenu.wordNo},
        {"menu.wordYes",        &g_mainMenu.wordYes},
        {"menu.caseNone",       &g_mainMenu.caseNone},
        {"menu.wordOn",         &g_mainMenu.wordOn},
        {"menu.wordOff",        &g_mainMenu.wordOff},
        {"menu.wordShow",       &g_mainMenu.wordShow},
        {"menu.wordHide",       &g_mainMenu.wordHide},
        {"menu.wordPlayer",     &g_mainMenu.wordPlayer},
        {"menu.wordProfile",    &g_mainMenu.wordProfile},
        {"menu.wordBack",       &g_mainMenu.wordBack},
        {"menu.wordResume",     &g_mainMenu.wordResume},
        {"menu.wordWaiting",    &g_mainMenu.wordWaiting},
        {"menu.wordLanguage",    &g_mainMenu.wordLanguage},

        {"menu.abbrevMilliseconds", &g_mainMenu.abbrevMilliseconds},


        {"outro.gameCredits",           &g_outroScreen.gameCredits},
        {"outro.engineCredits",         &g_outroScreen.engineCredits},
        {"outro.originalBy",            &g_outroScreen.originalBy},
        {"outro.nameAndrewSpinks",      &g_outroScreen.nameAndrewSpinks},
        {"outro.cppPortDevelopers",     &g_outroScreen.cppPortDevelopers},
        {"outro.nameVitalyNovichkov",   &g_outroScreen.nameVitalyNovichkov},
        {"outro.psVitaPortBy",          &g_outroScreen.psVitaPortBy},
        {"outro.levelDesign",           &g_outroScreen.levelDesign},
        {"outro.customSprites",         &g_outroScreen.customSprites},
        {"outro.specialThanks",         &g_outroScreen.specialThanks},


        {"game.controls.phrasePlayerDisconnected",  &g_gameStrings.controlsPhrasePlayerDisconnected},
        {"game.controls.phrasePlayerConnected",     &g_gameStrings.controlsPhrasePlayerConnected},

        {"game.error.openFileFailed",               &g_gameStrings.errorOpenFileFailed},
        {"game.error.errorInvalidEnterWarp",        &g_gameStrings.errorInvalidEnterWarp},
        {"game.error.errorNoStartPoint",            &g_gameStrings.errorNoStartPoint},

        {"game.error.warpNeedStarCount",            &g_gameStrings.warpNeedStarCount},

        {"game.pause.continue",             &g_gameStrings.pauseItemContinue},
        {"game.pause.restartLevel",         &g_gameStrings.pauseItemRestartLevel},
        {"game.pause.resetCheckpoints",     &g_gameStrings.pauseItemResetCheckpoints},
        {"game.pause.quitTesting",          &g_gameStrings.pauseItemQuitTesting},
        {"game.pause.returnToEditor",       &g_gameStrings.pauseItemReturnToEditor},
        {"game.pause.dropAddPlayers",       &g_gameStrings.pauseItemDropAddPlayers},
        {"game.pause.enterCode",            &g_gameStrings.pauseItemEnterCode},
        {"game.pause.saveAndContinue",      &g_gameStrings.pauseItemSaveAndContinue},
        {"game.pause.saveAndQuit",          &g_gameStrings.pauseItemSaveAndQuit},
        {"game.pause.quit",                 &g_gameStrings.pauseItemQuit},

        {"game.connect.reconnectTitle",            &g_gameStrings.connectReconnectTitle},
        {"game.connect.dropAddTitle",              &g_gameStrings.connectDropAddTitle},

        {"game.connect.phrasePressAButton",        &g_gameStrings.connectPressAButton},

        {"game.connect.phraseTestControls",        &g_gameStrings.connectTestControls},
        {"game.connect.phraseHoldStart",           &g_gameStrings.connectHoldStart},
        {"game.connect.wordDisconnect",            &g_gameStrings.connectDisconnect},

        {"game.connect.phraseForceResume",         &g_gameStrings.connectForceResume},
        {"game.connect.phraseDropPX",              &g_gameStrings.connectDropPX},

        {"game.connect.phraseWaitingForInput",     &g_gameStrings.connectWaitingForInputDevice},
        {"game.connect.splitPressSelect_1",        &g_gameStrings.connectPressSelectForControlsOptions_P1},
        {"game.connect.splitPressSelect_2",        &g_gameStrings.connectPressSelectForControlsOptions_P2},

        {"game.connect.phraseChangeChar",          &g_gameStrings.connectChangeChar},
        {"game.connect.phraseSetControls",         &g_gameStrings.connectSetControls},
        {"game.connect.phraseDropMe",              &g_gameStrings.connectDropMe},

        {"game.connect.phraseStartToResume",       &g_gameStrings.connectPressStartToResume},
        {"game.connect.phraseStartToForceRes",     &g_gameStrings.connectPressStartToForceResume},

#ifdef THEXTECH_ENABLE_EDITOR
        {"editor.block.pickContents",       &g_editorStrings.pickBlockContents},

        {"editor.block.letterWidth",        &g_editorStrings.blockLetterWidth},
        {"editor.block.letterHeight",       &g_editorStrings.blockLetterHeight},
        {"editor.block.canBreak",           &g_editorStrings.blockCanBreak},
        {"editor.block.canBreakTooltip",    &g_editorStrings.blockTooltipCanBreak},
        {"editor.block.slick",              &g_editorStrings.blockSlick},
        {"editor.block.invis",              &g_editorStrings.blockInvis},
        {"editor.block.inside",             &g_editorStrings.blockInside},

        {"editor.warp.title",               &g_editorStrings.warpTitle},
        {"editor.warp.placing",             &g_editorStrings.warpPlacing},
        {"editor.warp.in",                  &g_editorStrings.warpIn},
        {"editor.warp.out",                 &g_editorStrings.warpOut},
        {"editor.warp.dir",                 &g_editorStrings.warpDir},
        {"editor.warp.twoWay",              &g_editorStrings.warpTwoWay},
        {"editor.warp.style.style",         &g_editorStrings.warpStyle},
        {"editor.warp.style.pipe",          &g_editorStrings.warpStylePipe},
        {"editor.warp.style.door",          &g_editorStrings.warpStyleDoor},
        {"editor.warp.style.blipInstant",   &g_editorStrings.warpStyleBlipInstant},
        {"editor.warp.style.portal",        &g_editorStrings.warpStylePortal},
        {"editor.warp.effect",              &g_editorStrings.warpEffect},
        {"editor.warp.allow",               &g_editorStrings.warpAllow},
        {"editor.warp.item",                &g_editorStrings.warpItem},
        {"editor.warp.ride",                &g_editorStrings.warpRide},
        {"editor.warp.cannonExit",          &g_editorStrings.warpCannonExit},
        {"editor.warp.speed",               &g_editorStrings.warpSpeed},
        {"editor.warp.needStarCount",       &g_editorStrings.warpNeedStarCount},
        {"editor.warp.needKey",             &g_editorStrings.warpNeedKey},
        {"editor.warp.needFloor",           &g_editorStrings.warpNeedFloor},
        {"editor.warp.starLockMessage",     &g_editorStrings.warpStarLockMessage},
        {"editor.warp.toMap",               &g_editorStrings.warpToMap},
        {"editor.warp.lvlWarp",             &g_editorStrings.warpLvlWarp},
        {"editor.warp.target",              &g_editorStrings.warpTarget},
        {"editor.warp.to",                  &g_editorStrings.warpTo},
        {"editor.warp.showStartScene",      &g_editorStrings.warpShowStartScene},
        {"editor.warp.showStarCount",       &g_editorStrings.warpShowStarCount},

        {"editor.water.title",              &g_editorStrings.waterTitle},

        {"editor.npc.inContainer",          &g_editorStrings.npcInContainer},
        {"editor.npc.inertNice",            &g_editorStrings.npcInertNice},
        {"editor.npc.stuckStop",            &g_editorStrings.npcStuckStop},
        {"editor.npc.props.active",         &g_editorStrings.npcPropertyActive},
        {"editor.npc.props.attachSurface",  &g_editorStrings.npcPropertyAttachSurface},
        {"editor.npc.props.facing",         &g_editorStrings.npcPropertyFacing},
        {"editor.npc.abbrevGen",            &g_editorStrings.npcAbbrevGen},

        {"editor.npc.ai.aiIs",              &g_editorStrings.npcAiIs},
        {"editor.npc.ai.target",            &g_editorStrings.npcAiTarget},
        {"editor.npc.ai.jump",              &g_editorStrings.npcAiJump},
        {"editor.npc.ai.leap",              &g_editorStrings.npcAiLeap},
        {"editor.npc.ai.swim",              &g_editorStrings.npcAiSwim},
        {"editor.npc.ai.LR",                &g_editorStrings.npcAiLR},
        {"editor.npc.ai.UD",                &g_editorStrings.npcAiUD},

        {"editor.npc.ai.headerCustomAi",    &g_editorStrings.npcCustomAi},

        {"editor.npc.ai.use1_0Ai",          &g_editorStrings.npcUse1_0Ai},
        {"editor.npc.tooltipExpandSection", &g_editorStrings.npcTooltipExpandSection},

        {"editor.npc.gen.header",           &g_editorStrings.npcGenHeader},
        {"editor.npc.gen.direction",        &g_editorStrings.npcGenDirection},
        {"editor.npc.gen.effectIs",         &g_editorStrings.npcGenEffectIs},
        {"editor.npc.gen.effectWarp",       &g_editorStrings.npcGenEffectWarp},
        {"editor.npc.gen.effectShoot",      &g_editorStrings.npcGenEffectShoot},

        {"editor.wordNPC.nominative",       &g_editorStrings.wordNPC},
        {"editor.wordNPC.genitive",         &g_editorStrings.wordNPCGenitive},

        {"editor.wordEvent.nominative",     &g_editorStrings.wordEvent},
        {"editor.wordEvent.genitive",       &g_editorStrings.wordEventGenitive},
        {"editor.wordEvent.typeLabel",      &g_editorStrings.phraseTypeLabelEvent},

        {"editor.wordCoins",                &g_editorStrings.wordCoins},

        {"editor.wordEnabled",              &g_editorStrings.wordEnabled},
        {"editor.wordText",                 &g_editorStrings.wordText},
        {"editor.wordInstant",              &g_editorStrings.wordInstant},
        {"editor.wordMode",                 &g_editorStrings.wordMode},
        {"editor.wordHeight",               &g_editorStrings.wordHeight},
        {"editor.wordWidth",                &g_editorStrings.wordWidth},

        {"editor.phraseTextOf",             &g_editorStrings.phraseTextOf},
        {"editor.phraseSectionIndex",       &g_editorStrings.phraseSectionIndex},
        {"editor.phraseRadiusIndex",        &g_editorStrings.phraseRadiusIndex},
        {"editor.phraseWarpIndex",          &g_editorStrings.phraseWarpIndex},
        {"editor.phraseGenericIndex",       &g_editorStrings.phraseGenericIndex},
        {"editor.phraseDelayIsMs",          &g_editorStrings.phraseDelayIsMs},
        {"editor.phraseCountMore",          &g_editorStrings.phraseCountMore},
        {"editor.mapPos",                   &g_editorStrings.mapPos},
        {"editor.phraseAreYouSure",         &g_editorStrings.phraseAreYouSure},
        {"editor.pageBlankOfBlank",         &g_editorStrings.pageBlankOfBlank},

        {"editor.letterUp",                 &g_editorStrings.letterUp},
        {"editor.letterDown",               &g_editorStrings.letterDown},
        {"editor.letterLeft",               &g_editorStrings.letterLeft},
        {"editor.letterRight",              &g_editorStrings.letterRight},
        {"editor.letterCoordX",             &g_editorStrings.letterCoordX},
        {"editor.letterCoordY",             &g_editorStrings.letterCoordY},

        {"editor.toggleMagicBlock",         &g_editorStrings.toggleMagicBlock},

        {"editor.testPlay.magicHand",       &g_editorStrings.testMagicHand},
        {"editor.testPlay.char",            &g_editorStrings.testChar},
        {"editor.testPlay.power",           &g_editorStrings.testPower},
        {"editor.testPlay.boot",            &g_editorStrings.testBoot},
        {"editor.testPlay.pet",             &g_editorStrings.testPet},

        {"editor.events.header",            &g_editorStrings.eventsHeader},

        {"editor.events.letter.activate",    &g_editorStrings.eventsLetterActivate},
        {"editor.events.letter.death",       &g_editorStrings.eventsLetterDeath},
        {"editor.events.letter.talk",        &g_editorStrings.eventsLetterTalk},
        {"editor.events.letter.layerClear",  &g_editorStrings.eventsLetterLayerClear},
        {"editor.events.letter.hit",         &g_editorStrings.eventsLetterHit},
        {"editor.events.letter.destroy",     &g_editorStrings.eventsLetterDestroy},
        {"editor.events.letter.enter",       &g_editorStrings.eventsLetterEnter},

        {"editor.events.label.next",        &g_editorStrings.eventsLabelNext},
        {"editor.events.label.activate",    &g_editorStrings.eventsLabelActivate},
        {"editor.events.label.death",       &g_editorStrings.eventsLabelDeath},
        {"editor.events.label.talk",        &g_editorStrings.eventsLabelTalk},
        {"editor.events.label.layerClear",  &g_editorStrings.eventsLabelLayerClear},
        {"editor.events.label.hit",         &g_editorStrings.eventsLabelHit},
        {"editor.events.label.destroy",     &g_editorStrings.eventsLabelDestroy},
        {"editor.events.label.enter",       &g_editorStrings.eventsLabelEnter},

        {"editor.events.desc.activate",     &g_editorStrings.eventsDescActivate},
        {"editor.events.desc.death",        &g_editorStrings.eventsDescDeath},
        {"editor.events.desc.talk",         &g_editorStrings.eventsDescTalk},
        {"editor.events.desc.layerClear",   &g_editorStrings.eventsDescLayerClear},
        {"editor.events.desc.hit",          &g_editorStrings.eventsDescHit},
        {"editor.events.desc.destroy",      &g_editorStrings.eventsDescDestroy},
        {"editor.events.desc.enter",        &g_editorStrings.eventsDescEnter},

        {"editor.events.desc.phraseTriggersWhenTemplate",   &g_editorStrings.eventsDescPhraseTriggersWhenTemplate},
        {"editor.events.desc.phraseTriggersAfterTemplate",   &g_editorStrings.eventsDescPhraseTriggersAfterTemplate},

        {"editor.events.deletion.deletingEvent",    &g_editorStrings.eventsDeletingEvent},
        {"editor.events.deletion.confirm",          &g_editorStrings.eventsDeletionConfirm},
        {"editor.events.deletion.cancel",           &g_editorStrings.eventsDeletionCancel},

        {"editor.events.promptEventText",  &g_editorStrings.eventsPromptEventText},
        {"editor.events.promptEventName",  &g_editorStrings.eventsPromptEventName},
        {"editor.events.itemNewEvent",     &g_editorStrings.eventsItemNewEvent},

        {"editor.events.bounds.shouldEvent",     &g_editorStrings.eventsShouldEvent},
        {"editor.events.bounds.changeSectionBoundsToCurrent",     &g_editorStrings.eventsChangeSectionBoundsToCurrent},
        {"editor.events.bounds.changeAllSectionBoundsToCurrent",     &g_editorStrings.eventsChangeAllSectionBoundsToCurrent},

        {"editor.events.controlsForEventN",      &g_editorStrings.eventsControlsForEvent},
        {"editor.events.settingsForEvent",       &g_editorStrings.eventsSettingsForEvent},

        {"editor.events.layers.headerShow",      &g_editorStrings.eventsHeaderShow},
        {"editor.events.layers.headerHide",      &g_editorStrings.eventsHeaderHide},
        {"editor.events.layers.headerToggle",    &g_editorStrings.eventsHeaderToggle},
        {"editor.events.layers.headerMove",      &g_editorStrings.eventsHeaderMove},

        {"editor.events.sections.actionKeep",    &g_editorStrings.eventsActionKeep},
        {"editor.events.sections.actionReset",   &g_editorStrings.eventsActionReset},
        {"editor.events.sections.actionSet",     &g_editorStrings.eventsActionSet},

        {"editor.events.sections.propMusic",     &g_editorStrings.eventsCaseMusic},
        {"editor.events.sections.propBackground", &g_editorStrings.eventsCaseBackground},
        {"editor.events.sections.propBounds",    &g_editorStrings.eventsCaseBounds},

        {"editor.events.sections.phraseAllSections",     &g_editorStrings.eventsPhraseAllSections},

        {"editor.events.props.autostart",     &g_editorStrings.eventsPropAutostart},
        {"editor.events.props.sound",         &g_editorStrings.eventsPropSound},
        {"editor.events.props.endGame",       &g_editorStrings.eventsPropEndGame},
        {"editor.events.props.controls",      &g_editorStrings.eventsPropControls},

        {"editor.events.headerTriggerEvent",  &g_editorStrings.eventsHeaderTriggerEvent},

        {"editor.level.levelName",            &g_editorStrings.levelName},
        {"editor.level.startPos",             &g_editorStrings.levelStartPos},
        {"editor.level.pathBG",               &g_editorStrings.levelPathBG},
        {"editor.level.bigBG",                &g_editorStrings.levelBigBG},
        {"editor.level.gameStart",            &g_editorStrings.levelGameStart},
        {"editor.level.alwaysVis",            &g_editorStrings.levelAlwaysVis},
        {"editor.level.pathUnlocks",          &g_editorStrings.levelPathUnlocks},

        {"editor.section.setBounds",          &g_editorStrings.sectionSetBounds},
        {"editor.section.scroll",             &g_editorStrings.sectionScroll},
        {"editor.section.horizWrap",          &g_editorStrings.sectionHorizWrap},
        {"editor.section.underwater",         &g_editorStrings.sectionUnderwater},
        {"editor.section.noTurnBack",         &g_editorStrings.sectionNoTurnBack},
        {"editor.section.offscreenExit",      &g_editorStrings.sectionOffscreenExit},

        {"editor.world.name",                 &g_editorStrings.worldName},
        {"editor.world.introLevel",           &g_editorStrings.worldIntroLevel},
        {"editor.world.hubWorld",             &g_editorStrings.worldHubWorld},
        {"editor.world.retryOnFail",          &g_editorStrings.worldRetryOnFail},
        {"editor.world.totalStars",           &g_editorStrings.worldTotalStars},
        {"editor.world.allowChars",           &g_editorStrings.worldAllowChars},
        {"editor.world.phraseCreditIndex",    &g_editorStrings.worldCreditIndex},

        {"editor.select.soundForEventN",              &g_editorStrings.selectSoundForEvent},
        {"editor.select.sectBlankPropBlankForEventN", &g_editorStrings.selectSectBlankPropBlankForEvent},
        {"editor.select.allSectPropBlankForEventN",   &g_editorStrings.selectAllSectPropBlankForEvent},
        {"editor.select.sectionBlankPropBlank",     &g_editorStrings.selectSectionBlankPropBlank},
        {"editor.select.pathBlankUnlock",           &g_editorStrings.selectPathBlankUnlock},
        {"editor.select.warpTransitEffect",         &g_editorStrings.selectWarpTransitionEffect},
        {"editor.select.worldMusic",                &g_editorStrings.selectWorldMusic},

        {"editor.layers.header",              &g_editorStrings.layersHeader},

        {"editor.layers.label",               &g_editorStrings.labelLayer},
        {"editor.layers.labelAttached",       &g_editorStrings.layersLabelAttached},
        {"editor.layers.abbrevAttLayer",      &g_editorStrings.layersAbbrevAttLayer},
        {"editor.layers.default",             &g_editorStrings.layersLayerDefault},

        {"editor.layers.labelAttachedLayer",  &g_editorStrings.layersLabelAttachedLayer},
        {"editor.layers.labelMoveLayer",      &g_editorStrings.layersLabelMoveLayer},

        {"editor.layers.deletion.header",                   &g_editorStrings.layersDeletionHeader},
        {"editor.layers.deletion.preserveLayerContents",    &g_editorStrings.layersDeletionPreserveLayerContents},
        {"editor.layers.deletion.confirmPreserve",          &g_editorStrings.layersDeletionConfirmPreserve},
        {"editor.layers.deletion.confirmDelete",            &g_editorStrings.layersDeletionConfirmDelete},
        {"editor.layers.deletion.cancel",                   &g_editorStrings.layersDeletionCancel},

        {"editor.layers.desc.att", &g_editorStrings.layersDescAtt},

        {"editor.layers.promptLayerName", &g_editorStrings.layersPromptLayerName},
        {"editor.layers.itemNewLayer",    &g_editorStrings.layersItemNewLayer},

        {"editor.file.actionClearLevel",            &g_editorStrings.fileActionClearLevel},
        {"editor.file.actionClearWorld",            &g_editorStrings.fileActionClearWorld},
        {"editor.file.actionOpen",                  &g_editorStrings.fileActionOpen},
        {"editor.file.actionRevert",                &g_editorStrings.fileActionRevert},
        {"editor.file.actionExit",                  &g_editorStrings.fileActionExit},
        {"editor.file.confirmSaveBeforeAction",     &g_editorStrings.fileConfirmationSaveBeforeAction},
        {"editor.file.confirmConfirmAction",        &g_editorStrings.fileConfirmationConfirmAction},
        {"editor.file.confirmConvertFormatTo",      &g_editorStrings.fileConfirmationConvertFormatTo},
        {"editor.file.optionYesSaveThenAction",     &g_editorStrings.fileOptionYesSaveThenAction},
        {"editor.file.optionActionWithoutSave",     &g_editorStrings.fileOptionActionWithoutSave},
        {"editor.file.optionCancelAction",          &g_editorStrings.fileOptionCancelAction},
        {"editor.file.optionProceedWithConversion", &g_editorStrings.fileOptionProceedWithConversion},
        {"editor.file.optionCancelConversion",      &g_editorStrings.fileOptionCancelConversion},

        {"editor.file.labelCurrentFile",    &g_editorStrings.fileLabelCurrentFile},
        {"editor.file.labelFormat",         &g_editorStrings.fileLabelFormat},

        {"editor.file.formatModern",        &g_editorStrings.fileFormatModern},
        {"editor.file.formatLegacy",        &g_editorStrings.fileFormatLegacy},

        {"editor.file.sectionLevel",        &g_editorStrings.fileSectionLevel},
        {"editor.file.sectionWorld",        &g_editorStrings.fileSectionWorld},
        {"editor.file.commandNew",          &g_editorStrings.fileCommandNew},
        {"editor.file.commandOpen",         &g_editorStrings.fileCommandOpen},
        {"editor.file.commandSave",         &g_editorStrings.fileCommandSave},
        {"editor.file.commandSaveAs",       &g_editorStrings.fileCommandSaveAs},

        {"editor.file.convert.desc",                &g_editorStrings.fileConvertDesc},
        {"editor.file.convert.noIssues",            &g_editorStrings.fileConvertNoIssues},
        {"editor.file.convert.featuresWillBeLost",  &g_editorStrings.fileConvertFeaturesWillBeLost},

        {"editor.file.convert._38aUnsupported",     &g_editorStrings.fileConvert38aUnsupported},
        {"editor.file.convert.formatUnknown",       &g_editorStrings.fileConvertFormatUnknown},

        {"editor.file.convert.featureWarpTransit",      &g_editorStrings.fileConvertFeatureWarpTransit},
        {"editor.file.convert.featureWarpNeedsStand",   &g_editorStrings.fileConvertFeatureWarpNeedsStand},
        {"editor.file.convert.featureWarpCannonExit",   &g_editorStrings.fileConvertFeatureWarpCannonExit},
        {"editor.file.convert.featureWarpEnterEvent",   &g_editorStrings.fileConvertFeatureWarpEnterEvent},
        {"editor.file.convert.featureWarpCustomStarsMsg", &g_editorStrings.fileConvertFeatureWarpCustomStarsMsg},
        {"editor.file.convert.featureWarpNoPrintStars", &g_editorStrings.fileConvertFeatureWarpNoPrintStars},
        {"editor.file.convert.featureWarpNoStartScene", &g_editorStrings.fileConvertFeatureWarpNoStartScene},
        {"editor.file.convert.featureWarpPortal",       &g_editorStrings.fileConvertFeatureWarpPortal},

        {"editor.file.convert.featureEventCustomMusic", &g_editorStrings.fileConvertFeatureEventCustomMusic},
        {"editor.file.convert.featureEventAutoscroll",  &g_editorStrings.fileConvertFeatureEventAutoscroll},

        {"editor.file.convert.featureNPCVariant",       &g_editorStrings.fileConvertFeatureNPCVariant},
        {"editor.file.convert.featureBlockForceSmashable", &g_editorStrings.fileConvertFeatureBlockForceSmashable},

        {"editor.file.convert.featureCustomWorldMusic", &g_editorStrings.fileConvertFeatureCustomWorldMusic},
        {"editor.file.convert.featureWorldStarDisplay", &g_editorStrings.fileConvertFeatureWorldStarDisplay},
        {"editor.file.convert.featureLevelStarDisplay", &g_editorStrings.fileConvertFeatureLevelStarDisplay},

        {"editor.browser.newFile",          &g_editorStrings.browserNewFile},
        {"editor.browser.saveFile",         &g_editorStrings.browserSaveFile},
        {"editor.browser.openFile",         &g_editorStrings.browserOpenFile},

        {"editor.browser.itemNewFile",      &g_editorStrings.browserItemNewFile},
        {"editor.browser.itemNewFolder",    &g_editorStrings.browserItemNewFolder},

        {"editor.browser.askOverwriteFile", &g_editorStrings.browserAskOverwriteFile},

        {"editor.tooltip.select",   &g_editorStrings.tooltipSelect},
        {"editor.tooltip.erase",    &g_editorStrings.tooltipErase},
        {"editor.tooltip.eraseAll", &g_editorStrings.tooltipEraseAll},
        {"editor.tooltip.blocks",   &g_editorStrings.tooltipBlocks},
        {"editor.tooltip.BGOs",     &g_editorStrings.tooltipBGOs},
        {"editor.tooltip.NPCs",     &g_editorStrings.tooltipNPCs},
        {"editor.tooltip.warps",    &g_editorStrings.tooltipWarps},
        {"editor.tooltip.water",    &g_editorStrings.tooltipWater},
        {"editor.tooltip.settings", &g_editorStrings.tooltipSettings},
        {"editor.tooltip.layers",   &g_editorStrings.tooltipLayers},
        {"editor.tooltip.events",   &g_editorStrings.tooltipEvents},
        {"editor.tooltip.tiles",    &g_editorStrings.tooltipTiles},
        {"editor.tooltip.scenes",   &g_editorStrings.tooltipScenes},
        {"editor.tooltip.levels",   &g_editorStrings.tooltipLevels},
        {"editor.tooltip.paths",    &g_editorStrings.tooltipPaths},
        {"editor.tooltip.music",    &g_editorStrings.tooltipMusic},
        {"editor.tooltip.file",     &g_editorStrings.tooltipFile},
        {"editor.tooltip.show",     &g_editorStrings.tooltipShow},

#endif // THEXTECH_ENABLE_EDITOR
    };

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

        {"objects.wordFails", &gDemoCounterTitleDefault}
    };

    for(int i = 1; i <= numCharacters; ++i)
        m_assetsMap.insert({fmt::format_ne("character.name{0}", i), &g_gameInfo.characterName[i]});

#ifdef THEXTECH_ENABLE_EDITOR
    // adds dynamic fields to the asset map
    EditorCustom::Load(this);
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

    Controls::InitStrings();
    g_controlsStrings = ControlsStrings_t();

    s_CurrentPluralRules = PluralRules::OneIsSingular;
}

void XTechTranslate::exportTemplate()
{
#ifndef THEXTECH_DISABLE_LANG_TOOLS
    reset();

    try
    {
        nlohmann::json langFile;

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
            std::string data;

            if(!Files::dumpFile(fullFilePath, data))
            {
                std::printf("Warning: Failed to load the translation file %s: can't open file\n", fullFilePath.c_str());
                continue;
            }

            nlohmann::json langFile = nlohmann::json::parse(data);

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
            std::printf("Warning: Failed to load the translation file %s: %s\n", fullFilePath.c_str(), e.what());
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

        gDemoCounterTitle = gDemoCounterTitleDefault;
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
        std::string data;
        nlohmann::json langFile;

        if(Files::fileExists(file))
        {
            // Engine translations
            if(!Files::dumpFile(file, data))
            {
                pLogWarning("Failed to load the %s translation file %s: can't open file", trTypeName, file.c_str());
                return false;
            }

            langFile = nlohmann::json::parse(data);

            for(auto &k : list)
            {
                std::string &res = *k.second;
                res = getJsonValue(langFile, k.first, *k.second);
            }
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

