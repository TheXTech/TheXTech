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

#include "sdl_proxy/sdl_stdinc.h"
#include "int_discorcrpc.h"
#include <Logger/logger.h>

#include <time.h>
#include <discord_rpc.h>


static constexpr const char* APPLICATION_ID = XTECH_DISCORD_APPID;
static_assert(APPLICATION_ID != nullptr, "Discord Application ID is undefined. Please set the -DTHEXTECH_DISCORD_APPID=<app-id> CMake argument.");


static void handleDiscordReady(const DiscordUser* connectedUser)
{
    pLogDebug("Discord: connected to user %s#%s - %s",
              connectedUser->username,
              connectedUser->discriminator,
              connectedUser->userId);
}

static void handleDiscordDisconnected(int errcode, const char* message)
{
    pLogDebug("Discord: disconnected (%d: %s)", errcode, message);
}

static void handleDiscordError(int errcode, const char* message)
{
    pLogDebug("Discord: error (%d: %s)", errcode, message);
}

static void handleDiscordJoin(const char* secret)
{
    pLogDebug("Discord: join (%s)", secret);
}

static void handleDiscordSpectate(const char* secret)
{
    pLogDebug("Discord: spectate (%s)", secret);
}

static void handleDiscordJoinRequest(const DiscordUser* request)
{
    (void)request;
}


void DiscorcRPC::clearAllLabels()
{
    episodeName.clear();
    levelName.clear();
    editorFile.clear();
}

DiscorcRPC::DiscorcRPC()
{}

DiscorcRPC::~DiscorcRPC()
{
    clear();
    quit();
}

void DiscorcRPC::init()
{
    pLogDebug("Discord: Initializing...");

    DiscordEventHandlers handlers;
    SDL_memset(&handlers, 0, sizeof(handlers));
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordError;
    handlers.joinGame = handleDiscordJoin;
    handlers.spectateGame = handleDiscordSpectate;
    handlers.joinRequest = handleDiscordJoinRequest;
    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

void DiscorcRPC::quit()
{
    pLogDebug("Discord: Quitting...");
    Discord_Shutdown();
}

void DiscorcRPC::setGameName(const std::string& game)
{
    gameName = game;
    if(gameName.size() > 128)
        gameName.resize(128);

    // HEURISTIC icon name (for assets packs that has "status-icon-name" unset):
    //   it's impossible to dynamiclally upload custom images,
    //   So, there is a pre-defined list of available assets
    if(gameName == "Adventures of Demo")
        iconName = "assets_aod";
    else if(gameName == "A Super Mario Bros. X Thing" || gameName == "A Second Mario Bros. X Thing")
        iconName = "assets_a2xt_ptts";
    else if(gameName == "Nostalgic Paradise")
        iconName = "assets_nostalgic_paradise";
    else if(gameName == "Super Talking Time Bros." || gameName == "Super Talking Time Bros. 2.5")
        iconName = "assets_talking_time_bros";
    else if(gameName == "Super Mario Bros. X Nostalgie")
        iconName = "assets_smbx_nostalgie";
    else if(gameName == "Super Mario Bros. X")
        iconName = "assets_smbx";
    else
        iconName.clear();
}

void DiscorcRPC::setEpisodeName(const std::string& ep)
{
    clearAllLabels();
    episodeName = ep;
    if(episodeName.size() > 128)
        episodeName.resize(128);
}

void DiscorcRPC::setLevelName(const std::string& lev)
{
    clearAllLabels();
    levelName = lev;
    if(levelName.size() > 128)
        levelName.resize(128);
}

void DiscorcRPC::setEditorFile(const std::string& fil)
{
    clearAllLabels();
    editorFile = fil;
    if(editorFile.size() > 128)
        editorFile.resize(128);
}

void DiscorcRPC::setIconName(const std::string& icon)
{
    iconName = icon;
    if(iconName.size() > 32)
        iconName.resize(32);
}

void DiscorcRPC::update()
{
    pLogDebug("Discord: Updating state...");

    char buffer[128];

    DiscordRichPresence discordPresence;
    SDL_memset(&discordPresence, 0, sizeof(discordPresence));

    discordPresence.details = gameName.c_str();

    if(!levelName.empty())
    {
        SDL_snprintf(buffer, 128, "Playing level: %s", levelName.c_str());
        discordPresence.state = buffer;
    }
    else if(!episodeName.empty())
    {
        SDL_snprintf(buffer, 128, "Playing Episode: %s", episodeName.c_str());
        discordPresence.state = buffer;
    }
    else if(!editorFile.empty())
    {
        SDL_snprintf(buffer, 128, "Editing file: %s", editorFile.c_str());
        discordPresence.state = buffer;
    }

    discordPresence.startTimestamp = time(0);
    discordPresence.endTimestamp = 0;
    discordPresence.largeImageKey = iconName.empty() ? "thextech_default_icon" : iconName.c_str();
    discordPresence.largeImageText = "TheXTech";
    discordPresence.smallImageKey = iconName.empty() ? "" : "thextech_default_icon";
    discordPresence.smallImageText = gameName.c_str();
    discordPresence.partyId = "";
    discordPresence.partySize = 0;
    discordPresence.partyMax = 0;
    discordPresence.partyPrivacy = DISCORD_PARTY_PRIVATE;
    discordPresence.matchSecret = "";
    discordPresence.joinSecret = "";
    discordPresence.spectateSecret = "";
    discordPresence.instance = 0;
    Discord_UpdatePresence(&discordPresence);

#ifdef DISCORD_DISABLE_IO_THREAD
    Discord_UpdateConnection();
#endif
    Discord_RunCallbacks();
}

void DiscorcRPC::clear()
{
    pLogDebug("Discord: Clearing the presence...");
    Discord_ClearPresence();

#ifdef DISCORD_DISABLE_IO_THREAD
    Discord_UpdateConnection();
#endif
    Discord_RunCallbacks();
}
