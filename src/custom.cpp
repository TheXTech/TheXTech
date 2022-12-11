/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"

#include "custom.h"
#include "compat.h"

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
    RangeArrI<int, 0, maxNPCType, 0> NPCFrameOffsetX;
    RangeArrI<int, 0, maxNPCType, 0> NPCFrameOffsetY;
    RangeArrI<int, 0, maxNPCType, 0> NPCWidth;
    RangeArrI<int, 0, maxNPCType, 0> NPCHeight;
    RangeArrI<int, 0, maxNPCType, 0> NPCWidthGFX;
    RangeArrI<int, 0, maxNPCType, 0> NPCHeightGFX;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsAShell;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsABlock;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsAHit1Block;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsABonus;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsACoin;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsAVine;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsAnExit;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsAParaTroopa;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsCheep;
    RangeArrI<bool, 0, maxNPCType, false> NPCJumpHurt;
    RangeArrI<bool, 0, maxNPCType, false> NPCNoClipping;
    RangeArrI<int, 0, maxNPCType, 0> NPCScore;
    RangeArrI<bool, 0, maxNPCType, false> NPCCanWalkOn;
    RangeArrI<bool, 0, maxNPCType, false> NPCGrabFromTop;
    RangeArrI<bool, 0, maxNPCType, false> NPCTurnsAtCliffs;
    RangeArrI<bool, 0, maxNPCType, false> NPCWontHurt;
    RangeArrI<bool, 0, maxNPCType, false> NPCMovesPlayer;
    RangeArrI<bool, 0, maxNPCType, false> NPCStandsOnPlayer;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsGrabbable;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsBoot;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsYoshi;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsToad;
    RangeArrI<bool, 0, maxNPCType, false> NPCNoYoshi;
    RangeArrI<bool, 0, maxNPCType, false> NPCForeground;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsABot;
    RangeArrI<bool, 0, maxNPCType, false> NPCDefaultMovement;
    RangeArrI<bool, 0, maxNPCType, false> NPCIsVeggie;
    RangeArr<float, 0, maxNPCType> NPCSpeedvar;
    RangeArrI<bool, 0, maxNPCType, false> NPCNoFireBall;
    RangeArrI<bool, 0, maxNPCType, false> NPCNoIceBall;
    RangeArrI<bool, 0, maxNPCType, false> NPCNoGravity;

    RangeArrI<int, 0, maxNPCType, 0> NPCFrame;
    RangeArrI<int, 0, maxNPCType, 0> NPCFrameSpeed;
    RangeArrI<int, 0, maxNPCType, 0> NPCFrameStyle;
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
}

void SaveNPCDefaults()
{
    DirListCI NPCDir = DirListCI(AppPath + "graphics/npc/");
    std::string npcPathRes;

    NPCFrame.fill(0);
    NPCFrameSpeed.fill(8);
    NPCFrameStyle.fill(0);

    for(int A = 1; A <= maxNPCType; A++)
    {
        // Global override of NPC setup
        npcPathRes = NPCDir.resolveFileCaseExistsAbs(fmt::format_ne("npc-{0}.txt", A));

        if(!npcPathRes.empty())
            LoadCustomNPC(A, npcPathRes);

        s_NPCDefaults.NPCFrameOffsetX[A] = NPCFrameOffsetX[A];
        s_NPCDefaults.NPCFrameOffsetY[A] = NPCFrameOffsetY[A];
        s_NPCDefaults.NPCWidth[A] = NPCWidth[A];
        s_NPCDefaults.NPCHeight[A] = NPCHeight[A];
        s_NPCDefaults.NPCWidthGFX[A] = NPCWidthGFX[A];
        s_NPCDefaults.NPCHeightGFX[A] = NPCHeightGFX[A];
        s_NPCDefaults.NPCIsAShell[A] = NPCIsAShell[A];
        s_NPCDefaults.NPCIsABlock[A] = NPCIsABlock[A];
        s_NPCDefaults.NPCIsAHit1Block[A] = NPCIsAHit1Block[A];
        s_NPCDefaults.NPCIsABonus[A] = NPCIsABonus[A];
        s_NPCDefaults.NPCIsACoin[A] = NPCIsACoin[A];
        s_NPCDefaults.NPCIsAVine[A] = NPCIsAVine[A];
        s_NPCDefaults.NPCIsAnExit[A] = NPCIsAnExit[A];
        s_NPCDefaults.NPCIsAParaTroopa[A] = NPCIsAParaTroopa[A];
        s_NPCDefaults.NPCIsCheep[A] = NPCIsCheep[A];
        s_NPCDefaults.NPCJumpHurt[A] = NPCJumpHurt[A];
        s_NPCDefaults.NPCNoClipping[A] = NPCNoClipping[A];
        s_NPCDefaults.NPCScore[A] = NPCScore[A];
        s_NPCDefaults.NPCCanWalkOn[A] = NPCCanWalkOn[A];
        s_NPCDefaults.NPCGrabFromTop[A] = NPCGrabFromTop[A];
        s_NPCDefaults.NPCTurnsAtCliffs[A] = NPCTurnsAtCliffs[A];
        s_NPCDefaults.NPCWontHurt[A] = NPCWontHurt[A];
        s_NPCDefaults.NPCMovesPlayer[A] = NPCMovesPlayer[A];
        s_NPCDefaults.NPCStandsOnPlayer[A] = NPCStandsOnPlayer[A];
        s_NPCDefaults.NPCIsGrabbable[A] = NPCIsGrabbable[A];
        s_NPCDefaults.NPCIsBoot[A] = NPCIsBoot[A];
        s_NPCDefaults.NPCIsYoshi[A] = NPCIsYoshi[A];
        s_NPCDefaults.NPCIsToad[A] = NPCIsToad[A];
        s_NPCDefaults.NPCNoYoshi[A] = NPCNoYoshi[A];
        s_NPCDefaults.NPCForeground[A] = NPCForeground[A];
        s_NPCDefaults.NPCIsABot[A] = NPCIsABot[A];
        s_NPCDefaults.NPCDefaultMovement[A] = NPCDefaultMovement[A];
        s_NPCDefaults.NPCIsVeggie[A] = NPCIsVeggie[A];
        s_NPCDefaults.NPCSpeedvar[A] = NPCSpeedvar[A];
        s_NPCDefaults.NPCNoFireBall[A] = NPCNoFireBall[A];
        s_NPCDefaults.NPCNoIceBall[A] = NPCNoIceBall[A];
        s_NPCDefaults.NPCNoGravity[A] = NPCNoGravity[A];

        s_NPCDefaults.NPCFrame[A] = NPCFrame[A];
        s_NPCDefaults.NPCFrameSpeed[A] = NPCFrameSpeed[A];
        s_NPCDefaults.NPCFrameStyle[A] = NPCFrameStyle[A];
    }
}

void LoadNPCDefaults()
{
    int A = 0;
    for(A = 1; A <= maxNPCType; A++)
    {
        NPCFrameOffsetX[A] = s_NPCDefaults.NPCFrameOffsetX[A];
        NPCFrameOffsetY[A] = s_NPCDefaults.NPCFrameOffsetY[A];
        NPCWidth[A] = s_NPCDefaults.NPCWidth[A];
        NPCHeight[A] = s_NPCDefaults.NPCHeight[A];
        NPCWidthGFX[A] = s_NPCDefaults.NPCWidthGFX[A];
        NPCHeightGFX[A] = s_NPCDefaults.NPCHeightGFX[A];
        NPCIsAShell[A] = s_NPCDefaults.NPCIsAShell[A];
        NPCIsABlock[A] = s_NPCDefaults.NPCIsABlock[A];
        NPCIsAHit1Block[A] = s_NPCDefaults.NPCIsAHit1Block[A];
        NPCIsABonus[A] = s_NPCDefaults.NPCIsABonus[A];
        NPCIsACoin[A] = s_NPCDefaults.NPCIsACoin[A];
        NPCIsAVine[A] = s_NPCDefaults.NPCIsAVine[A];
        NPCIsAnExit[A] = s_NPCDefaults.NPCIsAnExit[A];
        NPCIsAParaTroopa[A] = s_NPCDefaults.NPCIsAParaTroopa[A];
        NPCIsCheep[A] = s_NPCDefaults.NPCIsCheep[A];
        NPCJumpHurt[A] = s_NPCDefaults.NPCJumpHurt[A];
        NPCNoClipping[A] = s_NPCDefaults.NPCNoClipping[A];
        NPCScore[A] = s_NPCDefaults.NPCScore[A];
        NPCCanWalkOn[A] = s_NPCDefaults.NPCCanWalkOn[A];
        NPCGrabFromTop[A] = s_NPCDefaults.NPCGrabFromTop[A];
        NPCTurnsAtCliffs[A] = s_NPCDefaults.NPCTurnsAtCliffs[A];
        NPCWontHurt[A] = s_NPCDefaults.NPCWontHurt[A];
        NPCMovesPlayer[A] = s_NPCDefaults.NPCMovesPlayer[A];
        NPCStandsOnPlayer[A] = s_NPCDefaults.NPCStandsOnPlayer[A];
        NPCIsGrabbable[A] = s_NPCDefaults.NPCIsGrabbable[A];
        NPCIsBoot[A] = s_NPCDefaults.NPCIsBoot[A];
        NPCIsYoshi[A] = s_NPCDefaults.NPCIsYoshi[A];
        NPCIsToad[A] = s_NPCDefaults.NPCIsToad[A];
        NPCNoYoshi[A] = s_NPCDefaults.NPCNoYoshi[A];
        NPCForeground[A] = s_NPCDefaults.NPCForeground[A];
        NPCIsABot[A] = s_NPCDefaults.NPCIsABot[A];
        NPCDefaultMovement[A] = s_NPCDefaults.NPCDefaultMovement[A];
        NPCIsVeggie[A] = s_NPCDefaults.NPCIsVeggie[A];
        NPCSpeedvar[A] = s_NPCDefaults.NPCSpeedvar[A];
        NPCNoFireBall[A] = s_NPCDefaults.NPCNoFireBall[A];
        NPCNoIceBall[A] = s_NPCDefaults.NPCNoIceBall[A];
        NPCNoGravity[A] = s_NPCDefaults.NPCNoGravity[A];

        NPCFrame[A] = s_NPCDefaults.NPCFrame[A];
        NPCFrameSpeed[A] = s_NPCDefaults.NPCFrameSpeed[A];
        NPCFrameStyle[A] = s_NPCDefaults.NPCFrameStyle[A];
    }

    loadNpcSetupFixes();
}

void FindCustomPlayers()
{
    pLogDebug("Trying to load custom Player configs...");

    std::string playerPath, playerPathC;
    g_dirEpisode.setCurDir(FileNamePath);
    g_dirCustom.setCurDir(FileNamePath + FileName);

    for(int C = 1; C <= numCharacters; ++C)
    {
        for(int S = 1; S <= numStates; ++S)
        {
            // Episode-wide custom player setup
            playerPath = g_dirEpisode.resolveFileCaseExistsAbs(fmt::format_ne("{1}-{0}.ini", S, s_playerFileName[C]));
            // Level-wide custom player setup
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

    if(npc.en_gfxoffsetx)
        NPCFrameOffsetX[A] = npc.gfxoffsetx;
    if(npc.en_gfxoffsety)
        NPCFrameOffsetY[A] = npc.gfxoffsety;
    if(npc.en_width)
        NPCWidth[A] = int(npc.width);
    if(npc.en_height)
        NPCHeight[A] = int(npc.height);
    if(npc.en_gfxwidth)
        NPCWidthGFX[A] = int(npc.gfxwidth);
    if(npc.en_gfxheight)
        NPCHeightGFX[A] = int(npc.gfxheight);
    if(npc.en_score)
        NPCScore[A] = int(npc.score);
    if(npc.en_playerblock)
        NPCMovesPlayer[A] = npc.playerblock;
    if(npc.en_playerblocktop)
        NPCCanWalkOn[A] = npc.playerblocktop;
    if(npc.en_npcblock)
        NPCIsABlock[A] = npc.npcblock;
    if(npc.en_npcblocktop)
        NPCIsAHit1Block[A] = npc.npcblocktop;
    if(npc.en_grabside)
        NPCIsGrabbable[A] = npc.grabside;
    if(npc.en_grabtop)
        NPCGrabFromTop[A] = npc.grabtop;
    if(npc.en_jumphurt)
        NPCJumpHurt[A] = npc.jumphurt;
    if(npc.en_nohurt)
        NPCWontHurt[A] = npc.nohurt;
    if(npc.en_noblockcollision)
        NPCNoClipping[A] = npc.noblockcollision;
    if(npc.en_cliffturn)
        NPCTurnsAtCliffs[A] = npc.cliffturn;
    if(npc.en_noyoshi)
        NPCNoYoshi[A] = npc.noyoshi;
    if(npc.en_foreground)
        NPCForeground[A] = npc.foreground;
    if(npc.en_speed)
        NPCSpeedvar[A] = float(npc.speed);
    if(npc.en_nofireball)
        NPCNoFireBall[A] = npc.nofireball;
    if(npc.en_noiceball)
        NPCNoIceBall[A] = npc.noiceball;
    if(npc.en_nogravity)
        NPCNoGravity[A] = npc.nogravity;
    if(npc.en_frames)
        NPCFrame[A] = int(npc.frames);
    if(npc.en_framespeed)
        NPCFrameSpeed[A] = int(npc.framespeed);
    if(npc.en_framestyle)
        NPCFrameStyle[A] = int(npc.framestyle);
}
