#include "../globals.h"
#include "../game_main.h"
#include "../npc.h"
#include "../load_gfx.h"
#include "../custom.h"
#include "../sound.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>

void OpenLevel(std::string FilePath)
{
    // USE PGE-FL here
}

void ClearLevel()
{
    int A = 0;
    int B = 0;
    NPC_t blankNPC;
    Water_t blankwater;
    Warp_t blankWarp;
    Block_t blankBlock;
    Background_t BlankBackground;
    Location_t BlankLocation;
    Events_t blankEvent;
    NPCScore[274] = 6;
    LevelName = "";
    LoadNPCDefaults();
    noUpdate = true;
    BlocksSorted = true;
    qScreen = false;
    UnloadCustomGFX();
    for(A = 1; A <= newEventNum; A++)
    {
        NewEvent[A] = "";
        newEventDelay[A] = 0;
    }
    for(A = 0; A <= maxSections; A++)
    {
        AutoX[A] = 0;
        AutoY[A] = 0;
    }
    newEventNum = 0;
    for(A = 0; A <= 100; A++)
    {
        Events[A] = blankEvent;
        for(B = 0; B <= maxSections; B++)
        {
            Events[A].Background[B] = -1;
            Events[A].Music[B] = -1;
            Events[A].level[B].X = -1;
        }
    }
    for(A = 0; A <= maxWater; A++)
        Water[A] = blankwater;
    numWater = 0;
    Events[0].Name = "Level - Start";
    Events[1].Name = "P Switch - Start";
    Events[2].Name = "P Switch - End";
    curMusic = 0;
    curStars = 0;
    maxStars = 0;
    PSwitchTime = 0;
    PSwitchStop = 0;
    BeltDirection = 1;
    StopMusic();
    Layer[0].Name = "Default";
    Layer[0].Hidden = false;
    Layer[1].Name = "Destroyed Blocks";
    Layer[1].Hidden = true;
    Layer[2].Name = "Spawned NPCs";
    Layer[2].Hidden = false;
    for(A = 0; A <= 100; A++)
    {
        if(A > 2)
        {
            Layer[A].Name = "";
            Layer[A].Hidden = false;
        }
        Layer[A].SpeedX = 0;
        Layer[A].SpeedY = 0;
    }

//    If LevelEditor = True Or MagicHand = True Then
//        MessageText = ""
//        frmNPCs.chkMessage.Value = 0
//        frmBlocks.chkFill.Value = 0
//        frmEvents.txtEvent.Text = ""
//        noUpdate = True
//        frmEvents.RefreshEvents
//        frmLayers.lstLayer.Clear
//        frmLayers.lstLayer.AddItem "Default"
//        frmLayers.lstLayer.AddItem "Destroyed Blocks"
//        frmLayers.lstLayer.AddItem "Spawned NPCs"
//        frmLayers.lstLayer.Selected(1) = False
//        frmLayers.lstLayer.Selected(2) = True
//        frmLayers.lstLayer.Selected(0) = True
//        frmLayers.cmdDelete.Enabled = False
//        frmLayers.txtLayer.Enabled = False
//        frmLevelEditor.optCursor(13).Value = True
//    End If

    for(A = -128; A <= maxNPCs; A++)
        NPC[A] = blankNPC;
    numNPCs = 0;
    for(A = 1; A <= maxBlocks; A++)
        Block[A] = blankBlock;
    numBlock = 0;
    for(A = 1; A <= maxBackgrounds; A++)
        Background[A] = BlankBackground;
    for(A = 0; A <= maxSections; A++)
    {
        Background2[A] = 0;
        bgColor[A] = 0xF89868;
        bgMusic[A] = 0;
        level[A] = BlankLocation;
        LevelWrap[A] = false;
        LevelChop[A] = 0;
        NoTurnBack[A] = false;
        UnderWater[A] = false;
        OffScreenExit[A] = false;
        CustomMusic[A] = "";
    }
    for(A = 1; A <= numWarps; A++)
        Warp[A] = blankWarp;
    numEffects = 0;
    numBackground = 0;
    PlayerStart[1] = BlankLocation;
    PlayerStart[2] = BlankLocation;

//    If LevelEditor = True Then
//        Unload frmLevelAdv
//        noUpdate = True
//        numPlayers = 0
//        frmLevelSettings.txtMusic.Enabled = False
//        frmLevelSettings.txtMusic.Text = ""
//        frmLevelSettings.txtMusic.Enabled = True
//        frmLevelSettings.optBackgroundColor(1).Value = 1
//        frmLevelSettings.cmdWrap.Caption = "Off"
//        frmLevelSettings.optMusic(bgMusic(0)).Value = True
//        vScreenX(1) = 0
//        vScreenY(1) = 0
//        curSection = 0
//        level(curSection).Height = 20000 * (curSection - maxSections / 2)
//        level(curSection).Height = Int(level(curSection).Height / 32) * 32
//        level(curSection).Y = level(curSection).Height - 600
//        level(curSection).X = 20000 * (curSection - maxSections / 2)
//        level(curSection).X = Int(level(curSection).X / 32) * 32
//        level(curSection).Width = level(curSection).X + 800
//        vScreenY(1) = -(level(curSection).Height - 600)
//        vScreenX(1) = -level(curSection).X
//        numWarps = 1
//        frmLevelSettings.optSection(0).Value = True
//        frmLevelSettings.optBackground(0).Value = True
//        frmLevelSettings.optLevel(0).Value = True
//        For A = 1 To frmLevelSettings.optBackground.Count - 1
//            frmLevelSettings.optBackground(A).Value = False
//        Next A
//    End If

    noUpdate = false;
}

void FindStars()
{
    int A = 0;
    int B = 0;
    std::string newInput = "";
    LevelData head;

    for(A = 1; A <= numWarps; A++)
    {
        auto &tempVar = Warp[A];
        if(tempVar.level != "")
        {
            std::string lFile = FileNamePath + tempVar.level;
            if(Files::fileExists(lFile))
            {
                if(FileFormats::OpenLevelFileHeader(lFile, head))
                {
                    tempVar.maxStars = head.stars;
                    tempVar.curStars = 0;
                    for(B = 1; B <= numStars; B++)
                    {
                        if(SDL_strcasecmp(Star[B].level.c_str(), tempVar.level.c_str()) == 0)
                            tempVar.curStars++;
                    }
                }
            }

        }
    }
}

