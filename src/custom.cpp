/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "globals.h"
#include "custom.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include <PGE_File_Formats/file_formats.h>
#include <fmt_format_ne.h>

static DirListCI s_dirEpisode;
static DirListCI s_dirCustom;

void LoadCustomNPC(int A, std::string cFileName);

void SaveNPCDefaults()
{
    int A = 0;
    for(A = 1; A <= maxNPCType; A++)
    {
        NPCDefaults.NPCFrameOffsetX[A] = NPCFrameOffsetX[A];
        NPCDefaults.NPCFrameOffsetY[A] = NPCFrameOffsetY[A];
        NPCDefaults.NPCWidth[A] = NPCWidth[A];
        NPCDefaults.NPCHeight[A] = NPCHeight[A];
        NPCDefaults.NPCWidthGFX[A] = NPCWidthGFX[A];
        NPCDefaults.NPCHeightGFX[A] = NPCHeightGFX[A];
        NPCDefaults.NPCIsAShell[A] = NPCIsAShell[A];
        NPCDefaults.NPCIsABlock[A] = NPCIsABlock[A];
        NPCDefaults.NPCIsAHit1Block[A] = NPCIsAHit1Block[A];
        NPCDefaults.NPCIsABonus[A] = NPCIsABonus[A];
        NPCDefaults.NPCIsACoin[A] = NPCIsACoin[A];
        NPCDefaults.NPCIsAVine[A] = NPCIsAVine[A];
        NPCDefaults.NPCIsAnExit[A] = NPCIsAnExit[A];
        NPCDefaults.NPCIsAParaTroopa[A] = NPCIsAParaTroopa[A];
        NPCDefaults.NPCIsCheep[A] = NPCIsCheep[A];
        NPCDefaults.NPCJumpHurt[A] = NPCJumpHurt[A];
        NPCDefaults.NPCNoClipping[A] = NPCNoClipping[A];
        NPCDefaults.NPCScore[A] = NPCScore[A];
        NPCDefaults.NPCCanWalkOn[A] = NPCCanWalkOn[A];
        NPCDefaults.NPCGrabFromTop[A] = NPCGrabFromTop[A];
        NPCDefaults.NPCTurnsAtCliffs[A] = NPCTurnsAtCliffs[A];
        NPCDefaults.NPCWontHurt[A] = NPCWontHurt[A];
        NPCDefaults.NPCMovesPlayer[A] = NPCMovesPlayer[A];
        NPCDefaults.NPCStandsOnPlayer[A] = NPCStandsOnPlayer[A];
        NPCDefaults.NPCIsGrabbable[A] = NPCIsGrabbable[A];
        NPCDefaults.NPCIsBoot[A] = NPCIsBoot[A];
        NPCDefaults.NPCIsYoshi[A] = NPCIsYoshi[A];
        NPCDefaults.NPCIsToad[A] = NPCIsToad[A];
        NPCDefaults.NPCNoYoshi[A] = NPCNoYoshi[A];
        NPCDefaults.NPCForeground[A] = NPCForeground[A];
        NPCDefaults.NPCIsABot[A] = NPCIsABot[A];
        NPCDefaults.NPCDefaultMovement[A] = NPCDefaultMovement[A];
        NPCDefaults.NPCIsVeggie[A] = NPCIsVeggie[A];
        NPCDefaults.NPCSpeedvar[A] = NPCSpeedvar[A];
        NPCDefaults.NPCNoFireBall[A] = NPCNoFireBall[A];
        NPCDefaults.NPCNoIceBall[A] = NPCNoIceBall[A];
        NPCDefaults.NPCNoGravity[A] = NPCNoGravity[A];
        NPCFrameSpeed[A] = 8;
    }
}

void LoadNPCDefaults()
{
    int A = 0;
    for(A = 1; A <= maxNPCType; A++)
    {
        NPCFrameOffsetX[A] = NPCDefaults.NPCFrameOffsetX[A];
        NPCFrameOffsetY[A] = NPCDefaults.NPCFrameOffsetY[A];
        NPCWidth[A] = NPCDefaults.NPCWidth[A];
        NPCHeight[A] = NPCDefaults.NPCHeight[A];
        NPCWidthGFX[A] = NPCDefaults.NPCWidthGFX[A];
        NPCHeightGFX[A] = NPCDefaults.NPCHeightGFX[A];
        NPCIsAShell[A] = NPCDefaults.NPCIsAShell[A];
        NPCIsABlock[A] = NPCDefaults.NPCIsABlock[A];
        NPCIsAHit1Block[A] = NPCDefaults.NPCIsAHit1Block[A];
        NPCIsABonus[A] = NPCDefaults.NPCIsABonus[A];
        NPCIsACoin[A] = NPCDefaults.NPCIsACoin[A];
        NPCIsAVine[A] = NPCDefaults.NPCIsAVine[A];
        NPCIsAnExit[A] = NPCDefaults.NPCIsAnExit[A];
        NPCIsAParaTroopa[A] = NPCDefaults.NPCIsAParaTroopa[A];
        NPCIsCheep[A] = NPCDefaults.NPCIsCheep[A];
        NPCJumpHurt[A] = NPCDefaults.NPCJumpHurt[A];
        NPCNoClipping[A] = NPCDefaults.NPCNoClipping[A];
        NPCScore[A] = NPCDefaults.NPCScore[A];
        NPCCanWalkOn[A] = NPCDefaults.NPCCanWalkOn[A];
        NPCGrabFromTop[A] = NPCDefaults.NPCGrabFromTop[A];
        NPCTurnsAtCliffs[A] = NPCDefaults.NPCTurnsAtCliffs[A];
        NPCWontHurt[A] = NPCDefaults.NPCWontHurt[A];
        NPCMovesPlayer[A] = NPCDefaults.NPCMovesPlayer[A];
        NPCStandsOnPlayer[A] = NPCDefaults.NPCStandsOnPlayer[A];
        NPCIsGrabbable[A] = NPCDefaults.NPCIsGrabbable[A];
        NPCIsBoot[A] = NPCDefaults.NPCIsBoot[A];
        NPCIsYoshi[A] = NPCDefaults.NPCIsYoshi[A];
        NPCIsToad[A] = NPCDefaults.NPCIsToad[A];
        NPCNoYoshi[A] = NPCDefaults.NPCNoYoshi[A];
        NPCForeground[A] = NPCDefaults.NPCForeground[A];
        NPCIsABot[A] = NPCDefaults.NPCIsABot[A];
        NPCDefaultMovement[A] = NPCDefaults.NPCDefaultMovement[A];
        NPCIsVeggie[A] = NPCDefaults.NPCIsVeggie[A];
        NPCSpeedvar[A] = NPCDefaults.NPCSpeedvar[A];
        NPCNoFireBall[A] = NPCDefaults.NPCNoFireBall[A];
        NPCNoIceBall[A] = NPCDefaults.NPCNoIceBall[A];
        NPCNoGravity[A] = NPCDefaults.NPCNoGravity[A];
        NPCFrame[A] = 0;
        NPCFrameSpeed[A] = 8;
        NPCFrameStyle[A] = 0;
    }
}

void FindCustomNPCs(/*std::string cFilePath*/)
{
    DirMan searchDir(FileNamePath);
    std::set<std::string> existingFiles;
    std::vector<std::string> files;
    searchDir.getListOfFiles(files, {".txt"});
    for(auto &p : files)
        existingFiles.insert(FileNamePath + p);

    s_dirEpisode.setCurDir(FileNamePath);
    s_dirCustom.setCurDir(FileNamePath + FileName);

    if(DirMan::exists(FileNamePath + FileName))
    {
        DirMan searchDataDir(FileNamePath + FileName);
        searchDataDir.getListOfFiles(files, {".png", ".gif"});
        for(auto &p : files)
            existingFiles.insert(FileNamePath + FileName  + "/"+ p);
    }

    for(int A = 1; A < maxNPCType; ++A)
    {
        std::string npcPath = FileNamePath + s_dirEpisode.resolveFileCase(fmt::format_ne("npc-{0}.txt", A));
        std::string npcPathC = FileNamePath + FileName + "/" + s_dirCustom.resolveFileCase(fmt::format_ne("npc-{0}.txt", A));
        if(Files::fileExists(npcPath))
            LoadCustomNPC(A, npcPath);
        if(Files::fileExists(npcPathC))
            LoadCustomNPC(A, npcPathC);
    }
}

void LoadCustomNPC(int A, std::string cFileName)
{
    NPCConfigFile npc;
    FileFormats::ReadNpcTXTFileF(cFileName, npc, true);

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
