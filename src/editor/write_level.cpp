#include "globals.h"
#include "sorting.h"
#include "layers.h"
#include "write_common.h"
#include "sound.h"

void SaveLevel(std::string FilePath)   // saves the level
{
    FILE* f = fopen(FilePath.c_str(), "wb");
    if (!f)
        return;

    int A = 0;
    int B = 0;
    int C = 0;
    int starCount = 0;
    // put NPC types 60, 62, 64, 66, and 78-83 first. (why?)
    for(A = 1; A <= numNPCs; A++)
    {
        if (NPC[A].Type == 60 || NPC[A].Type == 62 || NPC[A].Type == 64 || NPC[A].Type == 66 || (NPC[A].Type >= 78 && NPC[A].Type <= 83))
        {
            // swap it with the first NPC that isn't one of the special ones
            // this started as B = 1 but C + 1 will work now.
            for (B = C+1; B < A; B++)
            {
                if (!(NPC[B].Type == 60 || NPC[B].Type == 62 || NPC[B].Type == 64 || NPC[B].Type == 66 || (NPC[B].Type >= 78 && NPC[B].Type <= 83)))
                {
                    std::swap(NPC[A], NPC[B]);
                    break;
                }
            }
            C ++;
            // we know that the first C slots are all these types
        }
        // C ++ was here but that is a logical flaw.
    }
    qSortNPCsY(1, C);
    qSortNPCsY(C + 1, numNPCs);
    qSortBlocksX(1, numBlock);
    B = 1;
    for(A = 2; A <= numBlock; A++)
    {
        if(Block[A].Location.X > Block[B].Location.X)
        {
            qSortBlocksY(B, A - 1);
            B = A;
        }
    }
    qSortBlocksY(B, A - 1);
    qSortBackgrounds(1, numBackground);
    FindSBlocks();

    syncLayersTrees_AllBlocks();
    syncLayers_AllBGOs();
    syncLayers_AllNPCs();

    starCount = 0;
    for (int A = 1; A < numNPCs; A++)
    {
        if(NPC[A].Type == 97 || NPC[A].Type == 196)
            starCount ++;
    }
    // NPCyFix
    // Split filepath
    // For A = Len(FilePath) To 1 Step -1
    //     If Mid(FilePath, A, 1) = "/" Or Mid(FilePath, A, 1) = "\\" Then Exit For
    // Next A
    // FileNamePath = Left(FilePath, (A))
    // FileName = Right(FilePath, (Len(FilePath) - A))
    // FullFileName = FilePath
    // If Right(FileNamePath, 2) = "\\" Then
    //     FileNamePath = Left(FileNamePath, Len(FileNamePath) - 1)
    // End If
    fwritenum(f, curRelease);
    fwritenum(f, starCount);
    fwritestr(f, LevelName);
    for(B = 0; B <= 20; B++)
    {
        fwritenum(f, level[B].X);
        fwritenum(f, level[B].Y);
        fwritenum(f, level[B].Height);
        fwritenum(f, level[B].Width);
        fwritenum(f, bgMusic[B]);
        fwritenum(f, bgColor[B]);
        fwritebool(f, LevelWrap[B]);
        fwritebool(f, OffScreenExit[B]);
        fwritenum(f, Background2[B]);
        fwritebool(f, NoTurnBack[B]);
        fwritebool(f, UnderWater[B]);
        fwritestr(f, CustomMusic[B]);
    }
    for(A = 1; A <= 2; A++)
    {
        fwritenum(f, PlayerStart[A].X);
        fwritenum(f, PlayerStart[A].Y);
        fwritenum(f, PlayerStart[A].Width);
        fwritenum(f, PlayerStart[A].Height);
    }
    for(A = 1; A <= numBlock; A++)
    {
        fwritenum(f, Block[A].Location.X);
        fwritenum(f, Block[A].Location.Y);
        fwritenum(f, Block[A].Location.Height);
        fwritenum(f, Block[A].Location.Width);
        fwritenum(f, Block[A].Type);
        fwritenum(f, Block[A].Special);
        fwritebool(f, Block[A].Invis);
        fwritebool(f, Block[A].Slippy);
        // fix this to update as needed
        if (Block[A].Layer == "")
            Block[A].Layer = "Default";
        fwritestr(f, Block[A].Layer);
        fwritestr(f, Block[A].TriggerDeath);
        fwritestr(f, Block[A].TriggerHit);
        fwritestr(f, Block[A].TriggerLast);
    }
    fprintf(f, "next\r\n");
    for(A = 1; A <= numBackground; A++)
    {
        fwritenum(f, Background[A].Location.X);
        fwritenum(f, Background[A].Location.Y);
        fwritenum(f, Background[A].Type);
        // fix this to update as needed
        if (Background[A].Layer == "")
            Background[A].Layer = "Default";
        fwritestr(f, Background[A].Layer);
    }
    fprintf(f, "next\r\n");
    for(A = 1; A <= numNPCs; A++)
    {
        fwritenum(f, NPC[A].Location.X);
        fwritenum(f, NPC[A].Location.Y);
        fwritenum(f, NPC[A].Direction);
        fwritenum(f, NPC[A].Type);
        if (NPC[A].Type == 91 || NPC[A].Type == 96 || NPCIsAParaTroopa[NPC[A].Type] || NPC[A].Type == 283 || NPC[A].Type == 284 || NPCIsCheep[NPC[A].Type] || NPC[A].Type == 260)
            fwritenum(f, NPC[A].Special);
        // "potion"
        if (NPC[A].Type == 288 || NPC[A].Type == 289 || (NPC[A].Type == 91 && NPC[A].Special == 288))
            fwritenum(f, NPC[A].Special2);
        fwritebool(f, NPC[A].Generator);
        if (NPC[A].Generator)
        {
            fwritenum(f, NPC[A].GeneratorDirection);
            fwritenum(f, NPC[A].GeneratorEffect);
            fwritenum(f, NPC[A].GeneratorTimeMax);
        }
        fwritestr_multiline(f, NPC[A].Text);
        fwritebool(f, NPC[A].Inert);
        fwritebool(f, NPC[A].Stuck);
        fwritebool(f, NPC[A].Legacy);
        if (NPC[A].Layer == "")
            NPC[A].Layer = "Default";
        fwritestr(f, NPC[A].Layer);
        fwritestr(f, NPC[A].TriggerActivate);
        fwritestr(f, NPC[A].TriggerDeath);
        fwritestr(f, NPC[A].TriggerTalk);
        fwritestr(f, NPC[A].TriggerLast);
        fwritestr(f, NPC[A].AttLayer);
    }
    fprintf(f, "next\r\n");
    for(A = 1; A <= numWarps + 1; A++)
    {
        if (!Warp[A].PlacedEnt || !Warp[A].PlacedExit) continue;
        fwritenum(f, Warp[A].Entrance.X);
        fwritenum(f, Warp[A].Entrance.Y);
        fwritenum(f, Warp[A].Exit.X);
        fwritenum(f, Warp[A].Exit.Y);
        fwritenum(f, Warp[A].Direction);
        fwritenum(f, Warp[A].Direction2);
        fwritenum(f, Warp[A].Effect);

        fwritestr(f, Warp[A].level);
        fwritenum(f, Warp[A].LevelWarp);
        fwritebool(f, Warp[A].LevelEnt);

        fwritebool(f, Warp[A].MapWarp);
        fwritenum(f, Warp[A].MapX);
        fwritenum(f, Warp[A].MapY);

        fwritenum(f, Warp[A].Stars);

        if (Warp[A].Layer == "")
            Warp[A].Layer = "Default";
        fwritestr(f, Warp[A].Layer);
        fwritebool(f, Warp[A].Hidden);

        fwritebool(f, Warp[A].NoYoshi);
        fwritebool(f, Warp[A].WarpNPC);
        fwritebool(f, Warp[A].Locked);
    }
    fprintf(f, "next\r\n");
    for(A = 1; A <= numWater; A++)
    {
        fwritenum(f, Water[A].Location.X);
        fwritenum(f, Water[A].Location.Y);
        fwritenum(f, Water[A].Location.Width);
        fwritenum(f, Water[A].Location.Height);
        fwritenum(f, Water[A].Buoy);
        fwritebool(f, Water[A].Quicksand);
        fwritestr(f, Water[A].Layer);
    }
    fprintf(f, "next\r\n");
    // (removed code that made sure Name and Hidden were synced with the UI)
    for(A = 0; A <= 100; A++)
    {
        if (Layer[A].Name == "") break;
        fwritestr(f, Layer[A].Name);
        fwritebool(f, Layer[A].Hidden);
    }
    fprintf(f, "next\r\n");
    for(A = 0; A <= 100; A++)
    {
        if(Events[A].Name == "") break;
        fwritestr(f, Events[A].Name);
        fwritestr_multiline(f, Events[A].Text);
        fwritenum(f, Events[A].Sound);
        fwritenum(f, Events[A].EndGame);
        for(B = 0; B <= 20; B++)
        {
            if(B < Events[A].HideLayer.size())
                fwritestr(f, Events[A].HideLayer[B]);
            else
                fwritestr(f, "");
            if(B < Events[A].ShowLayer.size())
                fwritestr(f, Events[A].ShowLayer[B]);
            else
                fwritestr(f, "");
            if(B < Events[A].ToggleLayer.size())
                fwritestr(f, Events[A].ToggleLayer[B]);
            else
                fwritestr(f, "");
        }
        for(B = 0; B <= 20; B++)
        {
            fwritenum(f, Events[A].section[B].music_id);
            fwritenum(f, Events[A].section[B].background_id);
            fwritenum(f, Events[A].section[B].position.X);
            fwritenum(f, Events[A].section[B].position.Y);
            fwritenum(f, Events[A].section[B].position.Height);
            fwritenum(f, Events[A].section[B].position.Width);
        }
        fwritestr(f, Events[A].TriggerEvent);
        fwritenum(f, Events[A].TriggerDelay);
        fwritebool(f, Events[A].LayerSmoke);

        fwritebool(f, Events[A].Controls.AltJump);
        fwritebool(f, Events[A].Controls.AltRun);
        fwritebool(f, Events[A].Controls.Down);
        fwritebool(f, Events[A].Controls.Drop);
        fwritebool(f, Events[A].Controls.Jump);
        fwritebool(f, Events[A].Controls.Left);
        fwritebool(f, Events[A].Controls.Right);
        fwritebool(f, Events[A].Controls.Run);
        fwritebool(f, Events[A].Controls.Start);
        fwritebool(f, Events[A].Controls.Up);

        fwritebool(f, Events[A].AutoStart);
        fwritestr(f, Events[A].MoveLayer);
        fwritefloat(f, Events[A].SpeedX);
        fwritefloat(f, Events[A].SpeedY);

        fwritefloat(f, Events[A].AutoX);
        fwritefloat(f, Events[A].AutoY);
        fwritenum(f, Events[A].AutoSection);
    }
    fclose(f);

    // the rest of this stuff is all meant to be appropriately loading data
    // from the chosen folder
    // LoadNPCDefaults
    // If Dir(FileNamePath & Left(FileName, Len(FileName) - 4) & "\*.txt") <> "" Then
    //     FindCustomNPCs FileNamePath & Left(FileName, Len(FileName) - 4)
    // Else
    //     FindCustomNPCs
    // End If

    // UnloadCustomGFX
    // LoadCustomGFX

    // LoadCustomGFX2 FileNamePath & Left(FileName, Len(FileName) - 4)
    PlaySound(12);
}
