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

#include "integrator.h"
#ifdef ENABLE_XTECH_DISCORD_RPC
#include "int_discorcrpc.h"
static DiscorcRPC s_discord;
#endif

static bool s_integrationsLoaded = false;
static std::string s_prevEpisodeName;
static std::string s_prevLevelName;
static std::string s_prevEditorFile;

static void clearAllPrev()
{
    s_prevEpisodeName.clear();
    s_prevLevelName.clear();
    s_prevEditorFile.clear();
}

void Integrator::initIntegrations()
{
    if(s_integrationsLoaded)
        return;

#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.init();
#endif

    s_integrationsLoaded = true;
}

void Integrator::quitIntegrations()
{
    if(!s_integrationsLoaded)
        return;

#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.clear();
    s_discord.quit();
#endif

    s_integrationsLoaded = false;
}

void Integrator::setGameName(const std::string& gameName, const std::string& iconName)
{
#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.setGameName(gameName);
    if(!iconName.empty())
        s_discord.setIconName(iconName);
    s_discord.update();
#endif
}

void Integrator::setEpisodeName(const std::string& episodeName)
{
    if(s_prevEpisodeName == episodeName)
        return;

#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.setEpisodeName(episodeName);
    s_discord.update();
#endif

    clearAllPrev();
    s_prevEpisodeName = episodeName;
}

void Integrator::clearEpisodeName()
{
    if(s_prevEpisodeName.empty())
        return;

#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.setEpisodeName(std::string());
    s_discord.update();
#endif

    s_prevEpisodeName.clear();
}

void Integrator::setLevelName(const std::string& levelName)
{
    if(s_prevLevelName == levelName)
        return;

#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.setLevelName(levelName);
    s_discord.update();
#endif

    clearAllPrev();
    s_prevLevelName = levelName;
}

void Integrator::clearLevelName()
{
    if(s_prevLevelName.empty())
        return;

#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.setLevelName(std::string());
    s_discord.update();
#endif

    s_prevLevelName.clear();
}

void Integrator::setEditorFile(const std::string& editorFile)
{
    if(s_prevEditorFile == editorFile)
        return;

#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.setEditorFile(editorFile);
    s_discord.update();
#endif

    clearAllPrev();
    s_prevEditorFile = editorFile;
}

void Integrator::clearEditorFile()
{
    if(s_prevEditorFile.empty())
        return;

#ifdef ENABLE_XTECH_DISCORD_RPC
    s_discord.setEditorFile(std::string());
    s_discord.update();
#endif

    s_prevEditorFile.clear();
}
