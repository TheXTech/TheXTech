#include "../globals.h"
#include "../game_main.h"
#include "../npc.h"
#include "../load_gfx.h"
#include "../custom.h"
#include "../sound.h"
#include "../blocks.h"
#include "../graphics.h"
#include "../sorting.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>

void OpenLevel(std::string FilePath)
{
    std::string newInput = "";
    int FileRelease = 0;
    int A = 0;
    int B = 0;
    int C = 0;
    bool tempBool = false;
    int mSections = 0;
    Location_t tempLocation;
    qScreen = false;
    ClearLevel();
    BlockSound();
    FreezeNPCs = false;
    CoinMode = false;
    LevelData lvl;

    if(!Files::hasSuffix(FilePath, ".lvl") && !Files::hasSuffix(FilePath, ".lvlx"))
    {
        if(Files::fileExists(FilePath + ".lvlx"))
            FilePath += ".lvlx";
        else
            FilePath += ".lvl";
    }

    FileFormats::OpenLevelFile(FilePath, lvl);
    FileFormats::smbx64LevelPrepare(lvl);
    FileFormats::smbx64LevelSortBlocks(lvl);
    FileFormats::smbx64LevelSortBGOs(lvl);

//    if(StringHelper::toLower(FilePath.substr(FilePath.length() - 4)) != ".lvl" && StringHelper::toLower(FilePath.substr(FilePath.length() - 4)) != ".dat")
//        FilePath = FilePath + ".lvl";
//    for(A = FilePath.length(); A >= 1; A--)
//    {
//        if(FilePath.substr(A - 1, 1) == "/" || FilePath.substr(A - 1, 1) == "\\")
//            break;
//    }

    FileName = lvl.meta.filename;//FilePath.substr(FilePath.length() - (FilePath.length() - A));
    FileNamePath = lvl.meta.path + "/";//FilePath.substr(0, (A));
    if(FileNamePath.substr(FileNamePath.length() - 2) == "\\\\")
        FileNamePath = FileNamePath.substr(0, FileNamePath.length() - 1);
    FullFileName = FilePath;
    numBlock = 0;
    numBackground = 0;
    numNPCs = 0;
    numWarps = 0;

// Load Custom Stuff
    if(DirMan::exists(FileNamePath + FileName))
        FindCustomNPCs(FileNamePath + FileName);
    else
        FindCustomNPCs();
    LoadCustomGFX();
    if(DirMan::exists(FileNamePath + FileName))
        LoadCustomGFX2(FileNamePath + FileName);
// Blah

    if(FilePath == ".lvl" || FilePath == ".lvlx")
        return;

//// VB TO C++ CONVERTER WARNING: VB to C++ Converter converts from VB(.NET), not VB6:
//        Open FilePath For Input As #1;
//        Input #1, FileRelease;
//        if(FileRelease > curRelease)
//        {
//            MsgBox "You are using an old version of SMBX that is incompatible with this file. Please visit www.SuperMarioBrothers.org to get the latest updates.", Microsoft::VisualBasic::Constants::vbCritical, "Please click OK so the game can crash.";
//            KillIt();
//        }
//        if(FileRelease >= 17)
//            Input #1, maxStars;
    maxStars = lvl.stars;
//        if(FileRelease >= 60)
//            Input #1, LevelName;
    LevelName = lvl.LevelName;
//        if(FileRelease <= 7)
//            mSections = 5;
//        else
//            mSections = maxSections;
    mSections = maxSections;

    B = 0;
    for(auto & s : lvl.sections)
    {
        level[B].X = s.size_left;
        level[B].Y = s.size_top;
        level[B].Height = s.size_bottom;
        level[B].Width = s.size_right;
        LevelREAL[B] = level[B];
        bgMusic[B] = int(s.music_id);
        bgColor[B] = s.bgcolor;
        LevelWrap[B] = s.wrap_h;
        OffScreenExit[B] = s.OffScreenEn;
        Background2[B] = int(s.background);
        Background2REAL[B] = Background2[B];
        NoTurnBack[B] = s.lock_left_scroll;
        UnderWater[B] = s.underwater;
        CustomMusic[B] = s.music_file;
        B++;
        if(B > maxSections)
            break;
    }


    A = 1;
    for(auto &p : lvl.players)
    {
        PlayerStart[A].X = p.x;
        PlayerStart[A].Y = p.y;
        PlayerStart[A].Width = p.w;
        PlayerStart[A].Height = p.h;
        A++;
        if(A > 2)
            break;
    }


    for(auto &b : lvl.blocks)
    {
        numBlock++;
        if(numBlock > maxBlocks)
        {
            numBlock = maxBlocks;
            break;
        }

        Block[numBlock].Location.X = b.x;
        Block[numBlock].Location.Y = b.y;
        Block[numBlock].Location.Height = b.h;
        Block[numBlock].Location.Width = b.w;
        Block[numBlock].Type = int(b.id);
        Block[numBlock].DefaultType = Block[numBlock].Type;
        Block[numBlock].Special = int(b.npc_id);
        if(Block[numBlock].Special == 100)
            Block[numBlock].Special = 1009;
        if(Block[numBlock].Special == 102)
            Block[numBlock].Special = 1014;
        if(Block[numBlock].Special == 103)
            Block[numBlock].Special = 1034;
        if(Block[numBlock].Special == 105)
            Block[numBlock].Special = 1095;
        Block[numBlock].DefaultSpecial = Block[numBlock].Special;
        Block[numBlock].Invis = b.invisible;
        Block[numBlock].Slippy = b.slippery;
        Block[numBlock].Layer = b.layer;
        Block[numBlock].TriggerDeath = b.event_destroy;
        Block[numBlock].TriggerHit = b.event_hit;
        Block[numBlock].TriggerLast = b.event_emptylayer;
    }

    for(auto &b : lvl.bgo)
    {
        numBackground++;
        if(numBackground > maxBackgrounds)
        {
            numBackground = maxBackgrounds;
            break;
        }
        Background[numBackground].Location.X = b.x;
        Background[numBackground].Location.Y = b.y;
        Background[numBackground].Type = int(b.id);
        Background[numBackground].Layer = b.layer;
        Background[numBackground].Location.Width = GFXBackgroundWidth[Background[numBackground].Type];
        Background[numBackground].Location.Height = BackgroundHeight[Background[numBackground].Type];
    }


    for(auto &n : lvl.npc)
    {
        numNPCs++;
        if(numNPCs > maxNPCs)
        {
            numNPCs = maxNPCs;
            break;
        }

        NPC[numNPCs].Location.X = n.x;
        NPC[numNPCs].Location.Y = n.y;
        if(LevelEditor == false)
            NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - 0.01;
        NPC[numNPCs].Direction = n.direct;
        NPC[numNPCs].Type = int(n.id);
        if(NPC[numNPCs].Type == 91 || NPC[numNPCs].Type == 96 || NPC[numNPCs].Type == 283 || NPC[numNPCs].Type == 284)
        {
            NPC[numNPCs].Special = n.special_data;
            NPC[numNPCs].DefaultSpecial = NPC[numNPCs].Special;
        }
        if(NPC[numNPCs].Type == 288 || NPC[numNPCs].Type == 289 || (NPC[numNPCs].Type == 91 && NPC[numNPCs].Special == 288))
        {
            NPC[numNPCs].Special2 = n.special_data2;
            NPC[numNPCs].DefaultSpecial2 = NPC[numNPCs].Special2;
        }

        if(FileRelease >= 15)
        {
            if(NPCIsAParaTroopa[NPC[numNPCs].Type] == true)
            {
                NPC[numNPCs].Special = n.special_data;
                NPC[numNPCs].DefaultSpecial = NPC[numNPCs].Special;
            }
        }
        if(NPCIsCheep[NPC[numNPCs].Type] == true)
        {
            NPC[numNPCs].Special = n.special_data;
            NPC[numNPCs].DefaultSpecial = NPC[numNPCs].Special;
        }
        if(NPC[numNPCs].Type == 260)
        {
            NPC[numNPCs].Special = n.special_data;
            NPC[numNPCs].DefaultSpecial = NPC[numNPCs].Special;
        }
        NPC[numNPCs].Generator = n.generator;
        if(NPC[numNPCs].Generator == true)
        {
            NPC[numNPCs].GeneratorDirection = n.generator_direct;
            NPC[numNPCs].GeneratorEffect = n.generator_type;
            NPC[numNPCs].GeneratorTimeMax = n.generator_period;
        }

        NPC[numNPCs].Text = n.msg;

        NPC[numNPCs].Inert = n.friendly;
        if(NPC[numNPCs].Type == 151)
            NPC[numNPCs].Inert = true;
        NPC[numNPCs].Stuck = n.nomove;
        NPC[numNPCs].DefaultStuck = NPC[numNPCs].Stuck;

        NPC[numNPCs].Legacy = n.is_boss;

        NPC[numNPCs].Layer = n.layer;
        NPC[numNPCs].TriggerActivate = n.event_activate;
        NPC[numNPCs].TriggerDeath = n.event_die;
        NPC[numNPCs].TriggerTalk = n.event_talk;
        NPC[numNPCs].TriggerLast = n.event_emptylayer;
        NPC[numNPCs].AttLayer = n.attach_layer;

        NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
        NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
        NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
        NPC[numNPCs].DefaultLocation = NPC[numNPCs].Location;
        NPC[numNPCs].DefaultDirection = NPC[numNPCs].Direction;
        NPC[numNPCs].TimeLeft = 1;
        NPC[numNPCs].Active = true;
        NPC[numNPCs].JustActivated = 1;

        CheckSectionNPC(numNPCs);

        if(NPC[numNPCs].Type == 97 || NPC[numNPCs].Type == 196)
        {
            tempBool = false;
            for(B = 1; B <= numStars; B++)
            {
                if(Star[B].level == FileName && (Star[B].Section == NPC[numNPCs].Section || Star[B].Section == -1))
                    tempBool = true;
            }

            if(tempBool == true)
            {
                NPC[numNPCs].Special = 1;
                NPC[numNPCs].DefaultSpecial = 1;
                if(NPC[numNPCs].Type == 196)
                    NPC[numNPCs].Killed = 9;
            }
        }
    }


//        while(!Microsoft::VisualBasic::FileSystem::EOF(1))
//        {
//            Input #1, newInput;
//            if(newInput == "next")
//                break;
//            numWarps = numWarps + 1;
//            Warp[numWarps].PlacedEnt = true;
//            Warp[numWarps].PlacedExit = true;
//            Warp[numWarps].Entrance.X = newInput;
//            Input #1, Warp[numWarps].Entrance.Y;
//            Input #1, Warp[numWarps].Exit.X;
//            Input #1, Warp[numWarps].Exit.Y;
//            Input #1, Warp[numWarps].Direction;
//            Input #1, Warp[numWarps].Direction2;
//            Input #1, Warp[numWarps].Effect;
//            if(FileRelease >= 3)
//            {
//                Input #1, Warp[numWarps].level;
//                Input #1, Warp[numWarps].LevelWarp;
//                Input #1, Warp[numWarps].LevelEnt;
//            }
//            if(FileRelease >= 4)
//            {
//                Input #1, Warp[numWarps].MapWarp;
//                Input #1, Warp[numWarps].MapX;
//                Input #1, Warp[numWarps].MapY;
//            }
//            if(FileRelease >= 7)
//                Input #1, Warp[numWarps].Stars;
//            if(FileRelease >= 12)
//            {
//                Input #1, Warp[numWarps].Layer;
//                Input #1, Warp[numWarps].Hidden;
//            }
//            if(FileRelease >= 23)
//                Input #1, Warp[numWarps].NoYoshi;
//            if(FileRelease >= 25)
//                Input #1, Warp[numWarps].WarpNPC;
//            if(FileRelease >= 26)
//                Input #1, Warp[numWarps].Locked;
//            Warp[numWarps].Entrance.Height = 32;
//            Warp[numWarps].Entrance.Width = 32;
//            Warp[numWarps].Exit.Height = 32;
//            Warp[numWarps].Exit.Width = 32;
//        }


//        if(FileRelease >= 29)
//        {
//            A = 0;
//            while(!Microsoft::VisualBasic::FileSystem::EOF(1))
//            {
//                Input #1, newInput;
//                if(newInput == "next")
//                    break;
//                numWater = numWater + 1;
//                Water[numWater].Location.X = newInput;
//                Input #1, Water[numWater].Location.Y;
//                Input #1, Water[numWater].Location.Width;
//                Input #1, Water[numWater].Location.Height;
//                Input #1, Water[numWater].Buoy;
//                if(FileRelease >= 62)
//                    Input #1, Water[numWater].Quicksand;
//                Input #1, Water[numWater].Layer;
//            }
//        }


//        A = 0;
//        while(!Microsoft::VisualBasic::FileSystem::EOF(1))
//        {
//            Input #1, newInput;
//            if(newInput == "next")
//                break;
//            Layer[A].Name = newInput;
//            Input #1, Layer[A].Hidden;
//            if(Layer[A].Hidden == true)
//            {
//                HideLayer Layer[A].Name, true;
//            }
//            if(LevelEditor == true || MagicHand == true)
//            {
//                if(Layer[A].Name.ToLower() != "default" && Layer[A].Name.ToLower() != "destroyed blocks" && Layer[A].Name.ToLower() != "spawned npcs" && Layer[A].Name != "")
//                {
//                    frmLayers::lstLayer::AddItem Layer[A].Name;
//                    if(Layer[A].Hidden == false)
//                        frmLayers::lstLayer::Selected(frmLayers::lstLayer::ListCount - 1) = true;
//                }
//            }
//            A = A + 1;
//        }
//        A = 0;
//        while(!Microsoft::VisualBasic::FileSystem::EOF(1))
//        {
//            Input #1, newInput;
//            if(newInput == "next")
//                break;
//            Events[A].Name = newInput;
//            if(FileRelease >= 11)
//                Input #1, Events[A].Text;
//            if(FileRelease >= 14)
//                Input #1, Events[A].Sound;
//            if(FileRelease >= 18)
//                Input #1, Events[A].EndGame;
//            for(B = 0; B <= 20; B++)
//            {
//                Input #1, Events[A].HideLayer(B);
//                Input #1, Events[A].ShowLayer(B);
//                if(FileRelease >= 14)
//                    Input #1, Events[A].ToggleLayer(B);
//            }
//            if(FileRelease >= 13)
//            {
//                for(B = 0; B <= maxSections; B++)
//                {
//                    Input #1, Events[A].Music(B);
//                    Input #1, Events[A].Background[B];
//                    Input #1, Events[A].level[B].X;
//                    Input #1, Events[A].level[B].Y;
//                    Input #1, Events[A].level[B].Height;
//                    Input #1, Events[A].level[B].Width;
//                }
//            }
//            if(FileRelease >= 26)
//            {
//                Input #1, Events[A].TriggerEvent;
//                Input #1, Events[A].TriggerDelay;
//            }
//            if(FileRelease >= 27)
//                Input #1, Events[A].LayerSmoke;
//            if(FileRelease >= 28)
//            {
//                Input #1, Events[A].Controls.AltJump;
//                Input #1, Events[A].Controls.AltRun;
//                Input #1, Events[A].Controls.Down;
//                Input #1, Events[A].Controls.Drop;
//                Input #1, Events[A].Controls.Jump;
//                Input #1, Events[A].Controls.Left;
//                Input #1, Events[A].Controls.Right;
//                Input #1, Events[A].Controls.Run;
//                Input #1, Events[A].Controls.Start;
//                Input #1, Events[A].Controls.Up;
//            }
//            if(FileRelease >= 32)
//            {
//                Input #1, Events[A].AutoStart;
//                Input #1, Events[A].MoveLayer;
//                Input #1, Events[A].SpeedX;
//                Input #1, Events[A].SpeedY;
//            }
//            if(FileRelease >= 33)
//            {
//                Input #1, Events[A].AutoX;
//                Input #1, Events[A].AutoY;
//                Input #1, Events[A].AutoSection;
//            }
//            A = A + 1;
//        }
//    Close #1;


    FindBlocks();
    UpdateBackgrounds();
    FindSBlocks();



//    if(LevelEditor == true || MagicHand == true)
//    {
//        frmEvents::lstEvent.ListIndex = 0;
//        frmLayers::lstLayer.ListIndex = 0;
//        frmEvents::RefreshEvents;
//    }

//    if(LevelEditor == true)
//    {
//        ResetNPC EditorCursor.NPC.Type;
//        curSection = 0;
//        vScreenY[1] = -(level[curSection].Height - 600);
//        vScreenX[1] = -level[curSection].X;
//        numWarps = numWarps + 1;
//        for(A = 0; A < frmLevelSettings::optBackground.Count; A++)
//        {
//            if(Background2[0] == A)
//                frmLevelSettings::optBackground(A).Value = true;
//            else
//                frmLevelSettings::optBackground(A).Value = false;
//        }
//        for(A = 1; A <= frmLevelSettings::optBackgroundColor.Count; A++)
//        {
//            if(bgColor[0] == frmLevelSettings::optBackgroundColor(A).BackColor)
//            {
//                frmLevelSettings::optBackgroundColor(A).Value = true;
//                break;
//            }
//        }
//        frmLevelSettings::optMusic(bgMusic[0]).Value = true;
//        if(LevelWrap[0] == true)
//            frmLevelSettings::cmdWrap.Caption = "On";
//        else
//            frmLevelSettings::cmdWrap.Caption = "Off";
//        if(UnderWater[0] == true)
//            frmLevelSettings::cmdWater.Caption = "On";
//        else
//            frmLevelSettings::cmdWater.Caption = "Off";
//        if(OffScreenExit[0] == true)
//            frmLevelSettings::cmdExit.Caption = "On";
//        else
//            frmLevelSettings::cmdExit.Caption = "Off";
//        frmLevelSettings::txtMusic.Enabled = false;
//        frmLevelSettings::txtMusic.Text = CustomMusic[0];
//        frmLevelSettings::txtMusic.Enabled = true;
//        if(nPlay.Online == true && nPlay.Mode == 1) // sync to server
//        {
//            Netplay::sendData "j" + LB + "d" + LocalNick + " has loaded " + FileName + "." + LB + "w1" + LB + EoT;
//            frmChat.txtChat = frmChat::txtChat + LocalNick + " has loaded " + FileName + "." + LB;
//            frmChat::txtChat.SelStart = frmChat::txtChat.Text.Length;
//            PlaySound(47);
//            SoundPause[47] = 2;
//            for(A = 1; A <= 15; A++)
//            {
//                if(nPlay.ClientCon(A) == true)
//                {
//                    Netplay::InitSync A;
//                }
//            }
//        }
//    }
//    else
    {
        FindStars();
        LevelMacro = 0;
        for(A = 0; A <= maxSections; A++)
        {
            if(int(level[A].Height - level[A].Y) == 608)
                level[A].Y = level[A].Y + 8;
        }

        B = numBackground;
        for(A = 1; A <= numWarps; A++)
        {
            if(Warp[A].Effect == 2 && Warp[A].Stars > numStars)
            {
                B = B + 1;
                numLocked = numLocked + 1;
                Background[B].Layer = Warp[A].Layer;
                Background[B].Hidden = Warp[A].Hidden;
                Background[B].Location.Width = 24;
                Background[B].Location.Height = 24;
                Background[B].Location.Y = Warp[A].Entrance.Y - Background[B].Location.Height;
                Background[B].Location.X = Warp[A].Entrance.X + Warp[A].Entrance.Width / 2.0 - Background[B].Location.Width / 2.0;
                Background[B].Type = 160;
            }
            else if(Warp[A].Effect == 2 && Warp[A].Locked == true) // For locks
            {
                B = B + 1;
                numLocked = numLocked + 1;
                Background[B].Layer = Warp[A].Layer;
                Background[B].Hidden = Warp[A].Hidden;
                Background[B].Location = Warp[A].Entrance;
                Background[B].Type = 98;
                Background[B].Location.Width = 16;
            }
        }
    }

    SoundPause[13] = 100;
    overTime = 0;
    GoalTime = SDL_GetTicks() + 1000;
    fpsCount = 0;
    fpsTime = 0;
    cycleCount = 0;
    gameTime = 0;
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

