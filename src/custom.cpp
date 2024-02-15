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

#include "globals.h"
#include "global_dirs.h"
#include "load_gfx.h"

#include "sdl_proxy/sdl_stdinc.h"

#include "custom.h"
#include "compat.h"
#include "npc_traits.h"

#include <utility>

#include <IniProcessor/ini_processing.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include <PGE_File_Formats/file_formats.h>
#include <fmt_format_ne.h>


static struct PlayerBackup
{
    struct FramePos
    {
        int x;
        int y;
    } p[maxPlayerFrames + 1];

    struct Calibration
    {
        int w;
        int h;
        int h_duck;
        int grubX;
        int grubY;
    } c[numStates];
} s_playerFramesBackup[numCharacters];

typedef RangeArrI<int, 0, maxPlayerFrames, 0> PlayerOffsetArray;

static PlayerOffsetArray *s_playerFrameX[numCharacters + 1] = {
    nullptr, &MarioFrameX, &LuigiFrameX, &PeachFrameX, &ToadFrameX, &LinkFrameX
};
static PlayerOffsetArray *s_playerFrameY[numCharacters + 1] = {
    nullptr, &MarioFrameY, &LuigiFrameY, &PeachFrameY, &ToadFrameY, &LinkFrameY
};

const char *s_playerFileName[] = {nullptr, "mario", "luigi", "peach", "toad", "link"};


static struct NPCDefaults_t
{
    RangeArr<NPCTraits_t, 0, maxNPCType> NPCTraits;
//End Type
} s_NPCDefaults;


void LoadCustomNPC(int A, std::string cFileName);
void LoadCustomPlayer(int character, int state, std::string cFileName);


void SavePlayerDefaults()
{
    pLogDebug("Saving Player defaults...");

    DirListCI PlayerDir;
    std::string PlayerPathRes;

    // Load global customization configs first
    for(int C = 1; C <= numCharacters; ++C)
    {
        PlayerDir.setCurDir(AppPath + "graphics/" + s_playerFileName[C]);

        for(int S = 1; S <= numStates; ++S)
        {
            // Global override of player setup
            PlayerPathRes = PlayerDir.resolveFileCaseExistsAbs(fmt::format_ne("{0}-{1}.ini", s_playerFileName[C], S));
            if(!PlayerPathRes.empty())
                LoadCustomPlayer(C, S, PlayerPathRes);
        }
    }

    // Then, backup all parameters
    for(int p = 1; p <= numCharacters; ++p)
    {
        auto &pb = s_playerFramesBackup[p - 1];
        for(int j = 0; j <= maxPlayerFrames; ++j)
        {
            pb.p[j].x = (*s_playerFrameX[p])[j];
            pb.p[j].y = (*s_playerFrameY[p])[j];
        }

        for(int j = 1; j <= numStates; ++j)
        {
            pb.c[j - 1].w = Physics.PlayerWidth[p][j];
            pb.c[j - 1].h = Physics.PlayerHeight[p][j];
            pb.c[j - 1].h_duck = Physics.PlayerDuckHeight[p][j];
            pb.c[j - 1].grubX = Physics.PlayerGrabSpotX[p][j];
            pb.c[j - 1].grubY = Physics.PlayerGrabSpotY[p][j];
        }
    }
}

void LoadPlayerDefaults()
{
    pLogDebug("Restoring Player defaults...");

    for(int p = 1; p <= numCharacters; ++p)
    {
        auto &pb = s_playerFramesBackup[p - 1];
        for(int j = 0; j <= maxPlayerFrames; ++j)
        {
            (*s_playerFrameX[p])[j] = pb.p[j].x;
            (*s_playerFrameY[p])[j] = pb.p[j].y;
        }

        for(int j = 1; j <= numStates; ++j)
        {
            Physics.PlayerWidth[p][j] = pb.c[j - 1].w;
            Physics.PlayerHeight[p][j] = pb.c[j - 1].h;
            Physics.PlayerDuckHeight[p][j] = pb.c[j - 1].h_duck;
            Physics.PlayerGrabSpotX[p][j] = pb.c[j - 1].grubX;
            Physics.PlayerGrabSpotY[p][j] = pb.c[j - 1].grubY;
        }
    }
}

/**
 * @brief Applies bug-fixes to default settings of NPC objects according to compatibility settings
 */
SDL_FORCE_INLINE void loadNpcSetupFixes()
{
    // TODO: Implement settings fixing logic here!
    if(g_compatibility.custom_powerup_collect_score)
    {
        NPCTraits[9].Score = 6; // Set Default Scores for PowerUps (6=1000)
        NPCTraits[184].Score = 6; // Mushrooms
        NPCTraits[185].Score = 6;
        NPCTraits[249].Score = 6;
        NPCTraits[250].Score = 6;
        NPCTraits[14].Score = 6; // Fire Flowers
        NPCTraits[182].Score = 6;
        NPCTraits[183].Score = 6;
        NPCTraits[34].Score = 6; // Leaf
        NPCTraits[169].Score = 6; // Tanooki
        NPCTraits[170].Score = 6; // Hammer Suit
        NPCTraits[264].Score = 6; // Ice Flowers
        NPCTraits[277].Score = 6;
    }
}

void SaveNPCDefaults()
{
    DirListCI NPCDir = DirListCI(AppPath + "graphics/npc/");
    std::string npcPathRes;

    for(int A = 1; A <= maxNPCType; A++)
    {
        NPCTraits[A].TFrames = 0;
        NPCTraits[A].FrameSpeed = 8;
        NPCTraits[A].FrameStyle = 0;

        // Global override of NPC setup
        npcPathRes = NPCDir.resolveFileCaseExistsAbs(fmt::format_ne("npc-{0}.txt", A));

        if(!npcPathRes.empty())
            LoadCustomNPC(A, npcPathRes);

        s_NPCDefaults.NPCTraits[A] = NPCTraits[A];
    }
}

void LoadNPCDefaults()
{
    int A = 0;
    for(A = 1; A <= maxNPCType; A++)
        NPCTraits[A] = s_NPCDefaults.NPCTraits[A];

    loadNpcSetupFixes();
}

void FindCustomPlayers(const char* preview_players_from)
{
    pLogDebug("Trying to load custom Player configs...");

    std::string playerPath, playerPathC;

    if(preview_players_from)
        g_dirEpisode.setCurDir(preview_players_from);
    else
    {
        g_dirEpisode.setCurDir(FileNamePath);
        g_dirCustom.setCurDir(FileNamePath + FileName);
    }

    for(int C = 1; C <= numCharacters; ++C)
    {
        for(int S = 1; S <= numStates; ++S)
        {
            // Episode-wide custom player setup
            playerPath = g_dirEpisode.resolveFileCaseExistsAbs(fmt::format_ne("{1}-{0}.ini", S, s_playerFileName[C]));

            // Level-wide custom player setup
            if(!preview_players_from)
                playerPathC = g_dirCustom.resolveFileCaseExistsAbs(fmt::format_ne("{1}-{0}.ini", S, s_playerFileName[C]));

            if(!playerPath.empty())
                LoadCustomPlayer(C, S, playerPath);
            if(!playerPathC.empty())
                LoadCustomPlayer(C, S, playerPathC);
        }
    }
}

static inline int convIndexCoorToSpriteIndex(short x, short y)
{
    return (y + 10 * x) - 49;
}

void LoadCustomPlayer(int character, int state, std::string cFileName)
{
    pLogDebug("Loading %s...", cFileName.c_str());

    IniProcessing hitBoxFile(cFileName);
    if(!hitBoxFile.isOpened())
    {
        pLogWarning("Can't open the player calibration file: %s", cFileName.c_str());
        return;
    }

    const short UNDEFINED = 0x7FFF;
    short width = UNDEFINED;
    short height = UNDEFINED;
    short height_duck = UNDEFINED;
    short grab_offset_x = UNDEFINED;
    short grab_offset_y = UNDEFINED;
    bool isUsed = true;
    short offsetX = UNDEFINED;
    short offsetY = UNDEFINED;

    hitBoxFile.beginGroup("common");
    //normal
    hitBoxFile.read("width", width, UNDEFINED);
    hitBoxFile.read("height", height, UNDEFINED);
    //duck
    hitBoxFile.read("height-duck", height_duck, UNDEFINED);

    //grab offsets
    hitBoxFile.read("grab-offset-x", grab_offset_x, UNDEFINED);
    hitBoxFile.read("grab-offset-y", grab_offset_y, UNDEFINED);

    hitBoxFile.endGroup();

    for(int x = 0; x < 10; x++)
    {
        for(int y = 0; y < 10; y++)
        {
            isUsed = true;
            offsetX = UNDEFINED;
            offsetY = UNDEFINED;

            std::string tFrame = fmt::format("frame-{0}-{1}", x, y);

            if(!hitBoxFile.contains(tFrame))
                continue; // Skip not existing frame

            hitBoxFile.beginGroup(tFrame);
            hitBoxFile.read("used", isUsed, true);
            if(isUsed) //--> skip this frame
            {
                //Offset relative to
                hitBoxFile.read("offsetX", offsetX, UNDEFINED);
                hitBoxFile.read("offsetY", offsetY, UNDEFINED);
                if(offsetX != UNDEFINED && offsetY != UNDEFINED)
                {
                    (*s_playerFrameX[character])[convIndexCoorToSpriteIndex(x, y) + state * 100] = -offsetX;
                    (*s_playerFrameY[character])[convIndexCoorToSpriteIndex(x, y) + state * 100] = -offsetY;
                }
            }
            hitBoxFile.endGroup();
        }
    }

    if(width != UNDEFINED)
        Physics.PlayerWidth[character][state] = width;
    if(height != UNDEFINED)
        Physics.PlayerHeight[character][state] = height;
    if(height_duck != UNDEFINED)
        Physics.PlayerDuckHeight[character][state] = height_duck;
    if(grab_offset_x != UNDEFINED)
        Physics.PlayerGrabSpotX[character][state] = grab_offset_x;
    if(grab_offset_y != UNDEFINED)
        Physics.PlayerGrabSpotY[character][state] = grab_offset_y;
}

void FindCustomNPCs(/*std::string cFilePath*/)
{
    pLogDebug("Trying to load custom NPC configs...");

    //const std::string GfxRoot = AppPath + "graphics/";
    std::string /*npcPathG,*/ npcPath, npcPathC;
    // DirMan searchDir(FileNamePath);
//    std::set<std::string> existingFiles;
//    std::vector<std::string> files;
//    searchDir.getListOfFiles(files, {".txt"});
//    for(auto &p : files)
//        existingFiles.insert(FileNamePath + p);

    g_dirEpisode.setCurDir(FileNamePath);
    g_dirCustom.setCurDir(FileNamePath + FileName);

//    if(DirMan::exists(FileNamePath + FileName))
//    {
//        DirMan searchDataDir(FileNamePath + FileName);
//        searchDataDir.getListOfFiles(files, {".png", ".gif"});
//        for(auto &p : files)
//            existingFiles.insert(FileNamePath + FileName  + "/"+ p);
//    }

    for(int A = 1; A < maxNPCType; ++A)
    {
        // Episode-wide custom NPC setup
        npcPath = g_dirEpisode.resolveFileCaseExistsAbs(fmt::format_ne("npc-{0}.txt", A));
        // Level-wide custom NPC setup
        npcPathC = g_dirCustom.resolveFileCaseExistsAbs(fmt::format_ne("npc-{0}.txt", A));

        if(!npcPath.empty())
            LoadCustomNPC(A, npcPath);
        if(!npcPathC.empty())
            LoadCustomNPC(A, npcPathC);
    }
}

void LoadCustomNPC(int A, std::string cFileName)
{
    NPCConfigFile npc;
    FileFormats::ReadNpcTXTFileF(std::move(cFileName), npc, true);

    auto& traits = NPCTraits[A];

    if(npc.en_gfxoffsetx)
        traits.FrameOffsetX = npc.gfxoffsetx;
    if(npc.en_gfxoffsety)
        traits.FrameOffsetY = npc.gfxoffsety;
    if(npc.en_width)
        traits.TWidth = int(npc.width);
    if(npc.en_height)
        traits.THeight = int(npc.height);
    if(npc.en_gfxwidth)
        traits.WidthGFX = int(npc.gfxwidth);
    if(npc.en_gfxheight)
        traits.HeightGFX = int(npc.gfxheight);
    if(npc.en_score)
        traits.Score = int(npc.score);
    if(npc.en_playerblock)
        traits.MovesPlayer = npc.playerblock;
    if(npc.en_playerblocktop)
        traits.CanWalkOn = npc.playerblocktop;
    if(npc.en_npcblock)
        traits.IsABlock = npc.npcblock;
    if(npc.en_npcblocktop)
        traits.IsAHit1Block = npc.npcblocktop;
    if(npc.en_grabside)
        traits.IsGrabbable = npc.grabside;
    if(npc.en_grabtop)
        traits.GrabFromTop = npc.grabtop;
    if(npc.en_jumphurt)
        traits.JumpHurt = npc.jumphurt;
    if(npc.en_nohurt)
        traits.WontHurt = npc.nohurt;
    if(npc.en_noblockcollision)
        traits.NoClipping = npc.noblockcollision;
    if(npc.en_cliffturn)
        traits.TurnsAtCliffs = npc.cliffturn;
    if(npc.en_noyoshi)
        traits.NoYoshi = npc.noyoshi;
    if(npc.en_foreground)
        traits.Foreground = npc.foreground;
    if(npc.en_speed)
        traits.Speedvar = float(npc.speed);
    if(npc.en_nofireball)
        traits.NoFireBall = npc.nofireball;
    if(npc.en_noiceball)
        traits.NoIceBall = npc.noiceball;
    if(npc.en_nogravity)
        traits.NoGravity = npc.nogravity;
    if(npc.en_frames)
        traits.TFrames = int(npc.frames);
    if(npc.en_framespeed)
        traits.FrameSpeed = int(npc.framespeed);
    if(npc.en_framestyle)
        traits.FrameStyle = int(npc.framestyle);
    if(npc.en_usedefaultcam)
        traits.UseDefaultCam = npc.usedefaultcam;
}

void LoadCustomPlayerPreviews(const char* preview_players_from)
{
    FindCustomPlayers(preview_players_from);
    LoadCustomGFX(false, preview_players_from);
}

void UnloadCustomPlayerPreviews()
{
    LoadPlayerDefaults();
    UnloadPlayerPreviewGFX();
}
