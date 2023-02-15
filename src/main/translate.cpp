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

#include "globals.h"

#include "fontman/font_manager.h"
#include "core/language.h"

#include "main/translate.h"
#include "main/menu_main.h"
#include "main/game_info.h"
#include "main/outro_loop.h"
#include "main/game_strings.h"

#include "editor/editor_strings.h"
#include "editor/editor_custom.h"


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

static bool setJsonValueIfNotExist(nlohmann::json &j, const std::string &key, const std::string &value)
{
    auto dot = key.find(".");
    if(dot == std::string::npos)
    {
        if(!j.contains(key) || j.is_null())
        {
            std::printf("-- ++ Added new string: %s = %s\n", key.c_str(), value.c_str());
            std::fflush(stdout);
            j[key] = value;
            return true;
        }
        return false; // Nothing changed
    }

    std::string subKey = key.substr(0, dot);
    return setJsonValueIfNotExist(j[subKey], key.substr(dot + 1), value);
}

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

static bool dumpFile(const std::string &inPath, std::string &outData)
{
    off_t end;
    bool ret = true;
    FILE *in = Files::utf8_fopen(inPath.c_str(), "r");
    if(!in)
        return false;

    outData.clear();

    std::fseek(in, 0, SEEK_END);
    end = std::ftell(in);
    std::fseek(in, 0, SEEK_SET);

    outData.resize(end);

    if(std::fread((void*)outData.data(), 1, outData.size(), in) != outData.size())
        ret = false;

    std::fclose(in);

    return ret;
}

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


XTechTranslate::XTechTranslate()
{
    // List of all translatable strings of the engine
    m_engineMap =
    {
        {"menu.main.mainStartGame",        &g_mainMenu.mainStartGame},
        {"menu.main.main1PlayerGame",      &g_mainMenu.main1PlayerGame},
        {"menu.main.mainStartGame",        &g_mainMenu.mainStartGame},
        {"menu.main.main1PlayerGame",      &g_mainMenu.main1PlayerGame},
        {"menu.main.mainMultiplayerGame",  &g_mainMenu.mainMultiplayerGame},
        {"menu.main.mainBattleGame",       &g_mainMenu.mainBattleGame},
        {"menu.main.mainEditor",           &g_mainMenu.mainEditor},
        {"menu.main.mainOptions",          &g_mainMenu.mainOptions},
        {"menu.main.mainExit",             &g_mainMenu.mainExit},

        {"menu.loading",                   &g_mainMenu.loading},

        {"languageName",                   &g_mainMenu.languageName},

        {"menu.editor.newWorld",           &g_mainMenu.editorNewWorld},
        {"menu.editor.errorResolution",    &g_mainMenu.editorErrorResolution},
        {"menu.editor.errorMissingResources", &g_mainMenu.editorErrorMissingResources},
        {"menu.editor.promptNewWorldName", &g_mainMenu.editorPromptNewWorldName},

        {"menu.character.charSelTitle",    &g_mainMenu.charSelTitle},

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

        {"menu.wordPlayer",                &g_mainMenu.wordPlayer},
        {"menu.wordProfile",               &g_mainMenu.wordProfile},
        {"menu.reconnectTitle",            &g_mainMenu.reconnectTitle},
        {"menu.dropAddTitle",              &g_mainMenu.dropAddTitle},
        {"menu.playerSelStartGame",        &g_mainMenu.playerSelStartGame},
        {"menu.phrasePressAButton",        &g_mainMenu.phrasePressAButton},
        {"menu.phraseTestControls",        &g_mainMenu.phraseTestControls},
        {"menu.wordDisconnect",            &g_mainMenu.wordDisconnect},
        {"menu.phraseHoldStartToReturn",   &g_mainMenu.phraseHoldStartToReturn},
        {"menu.wordBack",                  &g_mainMenu.wordBack},
        {"menu.wordResume",                &g_mainMenu.wordResume},
        {"menu.wordWaiting",               &g_mainMenu.wordWaiting},
        {"menu.phraseForceResume",         &g_mainMenu.phraseForceResume},
        {"menu.phraseDropOthers",          &g_mainMenu.phraseDropOthers},
        {"menu.phraseDropSelf",            &g_mainMenu.phraseDropSelf},
        {"menu.phraseChangeChar",          &g_mainMenu.phraseChangeChar},
        {"menu.selectCharacter",           &g_mainMenu.selectCharacter},

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

        {"menu.wordNo", &g_mainMenu.wordNo},
        {"menu.wordYes", &g_mainMenu.wordYes},
        {"menu.caseNone", &g_mainMenu.caseNone},


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

        // FIXME: wait until we have a generic name for stars / leeks
        // {"game.error.warpNeedStarLeekCount",            &g_gameStrings.warpNeedStarCount},


#ifdef THEXTECH_ENABLE_EDITOR
        {"editor.block.pickContents1",      &g_editorStrings.pickBlockContents1},
        {"editor.block.pickContents2",      &g_editorStrings.pickBlockContents2},

        {"editor.npc.inContainer",          &g_editorStrings.npcInContainer},
        {"editor.npc.inertNice",            &g_editorStrings.npcInertNice},
        {"editor.npc.stuckStop",            &g_editorStrings.npcStuckStop},
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
        {"editor.npc.gen.delayIsMs",        &g_editorStrings.npcGenDelayIsMs},

        {"editor.wordNPC.nominative",       &g_editorStrings.wordNPC},
        {"editor.wordNPC.genitive",         &g_editorStrings.wordNPCGenitive},

        {"editor.wordEvent.nominative",     &g_editorStrings.wordEvent},
        {"editor.wordEvent.genitive",       &g_editorStrings.wordEventGenitive},

        {"editor.wordCoins",                &g_editorStrings.wordCoins},

        {"editor.wordEnabled",              &g_editorStrings.wordEnabled},
        {"editor.wordText",                 &g_editorStrings.wordText},
        {"editor.phraseTextOf",             &g_editorStrings.phraseTextOf},
        {"editor.phraseSectionIndex",       &g_editorStrings.phraseSectionIndex},
        {"editor.phraseRadiusIndex",        &g_editorStrings.phraseRadiusIndex},

        {"editor.events.header",            &g_editorStrings.eventsHeader},
        {"editor.events.letterActivate",    &g_editorStrings.eventsLetterActivate},
        {"editor.events.letterDeath",       &g_editorStrings.eventsLetterDeath},
        {"editor.events.letterTalk",        &g_editorStrings.eventsLetterTalk},
        {"editor.events.letterLayerClear",  &g_editorStrings.eventsLetterLayerClear},

        {"editor.layer.label",              &g_editorStrings.labelLayer},
        {"editor.layer.labelAbbrevAttLayer", &g_editorStrings.labelAbbrevAttLayer},
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

        // FIXME: wait until we have a generic name for stars / leeks
        // {"objects.wordStarLeekAccusativeSingle", &g_gameInfo.wordStarAccusativeSingle},
        // {"objects.wordStarLeekAccusativePlural", &g_gameInfo.wordStarAccusativePlural},
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

void XTechTranslate::updateLanguages()
{
#ifndef THEXTECH_DISABLE_LANG_TOOLS
    std::vector<std::string> list;
    DirMan langs(AppPathManager::languagesDir());

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

            if(!dumpFile(fullFilePath, data))
            {
                std::printf("Warning: Failed to load the translation file %s: can't open file\n", fullFilePath.c_str());
                continue;
            }

            nlohmann::json langFile = nlohmann::json::parse(data);

            for(const auto &k : trList)
            {
                const std::string &res = *k.second;
                changed |= setJsonValueIfNotExist(langFile, k.first, isEnglish ? res : std::string());
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
#endif
}



bool XTechTranslate::translate()
{
    if(!FontManager::isInitied())
    {
        pLogWarning("Translations aren't supported without new font engine loaded (the 'fonts' directory is required)");
        return false;
    }

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
            if(!dumpFile(file, data))
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
