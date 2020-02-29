/*
 * A2xTech - A platform game engine ported from old source code for VB6
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

#include "../globals.h"
#include "../game_main.h"
#include "../npc.h"
#include "../load_gfx.h"
#include "../custom.h"
#include "../sound.h"
#include "../sorting.h"
#include "../layers.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>


void addMissingLvlSuffix(std::string &fileName)
{
    if(!fileName.empty() && !Files::hasSuffix(fileName, ".lvl") && !Files::hasSuffix(fileName, ".lvlx"))
    {
        bool isAbsolute = Files::isAbsolute(fileName);
        bool lvlxExists;

        if(isAbsolute)
            lvlxExists = Files::fileExists(fileName + ".lvlx");
        else
            lvlxExists = Files::fileExists(FileNamePath + fileName + ".lvlx");

        if(lvlxExists)
            fileName += ".lvlx";
        else
            fileName += ".lvl";
    }
}


void OpenLevel(std::string FilePath)
{
    std::string newInput;
//    int FileRelease = 0;
    int A = 0;
    int B = 0;
    int checkPointId = 0;
//    int C = 0;
//    bool tempBool = false;
//    int mSections = 0;
//    Location_t tempLocation;

    qScreen = false;
    ClearLevel();
    BlockSound();
    FreezeNPCs = false;
    CoinMode = false;
    LevelData lvl;

    addMissingLvlSuffix(FilePath);
//    if(!Files::hasSuffix(FilePath, ".lvl") && !Files::hasSuffix(FilePath, ".lvlx"))
//    {
//        if(Files::fileExists(FilePath + ".lvlx"))
//            FilePath += ".lvlx";
//        else
//            FilePath += ".lvl";
//    }

    FileFormats::OpenLevelFile(FilePath, lvl);
    FileFormats::smbx64LevelPrepare(lvl);
    FileFormats::smbx64LevelSortBlocks(lvl);
    FileFormats::smbx64LevelSortBGOs(lvl);

    FileNameFull = Files::basename(FilePath);
    FileName = lvl.meta.filename;
    FileNamePath = lvl.meta.path + "/";
//    if(FileNamePath.substr(FileNamePath.length() - 2) == "/")
//        FileNamePath = FileNamePath.substr(0, FileNamePath.length() - 1);
    FullFileName = FilePath;

    IsEpisodeIntro = (StartLevel == FileNameFull);

    numBlock = 0;
    numBackground = 0;
    numNPCs = 0;
    numWarps = 0;

    numLayers = 0;
    numEvents = 0;

// Load Custom Stuff
//    if(DirMan::exists(FileNamePath + FileName))
//        FindCustomNPCs(FileNamePath + FileName);
//    else
    FindCustomNPCs();
    LoadCustomGFX();
    LoadCustomSound();
//    if(DirMan::exists(FileNamePath + FileName)) // Useless now
//        LoadCustomGFX2(FileNamePath + FileName);
// Blah

    if(FilePath == ".lvl" || FilePath == ".lvlx")
        return;

    maxStars = lvl.stars;
    LevelName = lvl.LevelName;

    B = 0;
    for(auto & s : lvl.sections)
    {
        level[B].X = double(s.size_left);
        level[B].Y = double(s.size_top);
        level[B].Height = double(s.size_bottom);
        level[B].Width = double(s.size_right);
        LevelREAL[B] = level[B];
        bgMusic[B] = int(s.music_id);
        bgMusicREAL[B] = bgMusic[B];
        bgColor[B] = s.bgcolor;
        LevelWrap[B] = s.wrap_h;
        LevelVWrap[B] = s.wrap_v; // EXTRA
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

    for(A = 1; A <= 2; A++) // Fill with zeroes
    {
        PlayerStart[A].X = 0;
        PlayerStart[A].Y = 0;
        PlayerStart[A].Width = 0;
        PlayerStart[A].Height = 0;
    }

    A = 1;
    for(auto &p : lvl.players)
    {
        PlayerStart[A].X = double(p.x);
        PlayerStart[A].Y = double(p.y);
        PlayerStart[A].Width = double(p.w);
        PlayerStart[A].Height = double(p.h);
        PlayerStart[A].Direction = p.direction;
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

        Block[numBlock] = Block_t();

        Block[numBlock].Location.X = double(b.x);
        Block[numBlock].Location.Y = double(b.y);
        Block[numBlock].Location.Height = double(b.h);
        Block[numBlock].Location.Width = double(b.w);
        Block[numBlock].Type = int(b.id);
        Block[numBlock].DefaultType = Block[numBlock].Type;
        Block[numBlock].Special = int(b.npc_id > 0 ? b.npc_id + 1000 : -1 * b.npc_id);
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

        Background[numBackground] = Background_t();

        Background[numBackground].Location.X = double(b.x);
        Background[numBackground].Location.Y = double(b.y);
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

        NPC[numNPCs] = NPC_t();

        NPC[numNPCs].Location.X = n.x;
        NPC[numNPCs].Location.Y = n.y;
        if(!LevelEditor)
            NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - 0.01;
        NPC[numNPCs].Direction = n.direct;
        NPC[numNPCs].Type = int(n.id);

        if(NPC[numNPCs].Type == 91 || NPC[numNPCs].Type == 96 || NPC[numNPCs].Type == 283 || NPC[numNPCs].Type == 284)
        {
            NPC[numNPCs].Special = n.contents;
            NPC[numNPCs].DefaultSpecial = NPC[numNPCs].Special;
        }
        if(NPC[numNPCs].Type == 288 || NPC[numNPCs].Type == 289 || (NPC[numNPCs].Type == 91 && int(NPC[numNPCs].Special) == 288))
        {
            NPC[numNPCs].Special2 = n.special_data2;
            NPC[numNPCs].DefaultSpecial2 = NPC[numNPCs].Special2;
        }

        if(NPCIsAParaTroopa[NPC[numNPCs].Type])
        {
            NPC[numNPCs].Special = n.special_data;
            NPC[numNPCs].DefaultSpecial = NPC[numNPCs].Special;
        }

        if(NPCIsCheep[NPC[numNPCs].Type])
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
        if(NPC[numNPCs].Generator)
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

        if(NPC[numNPCs].Type == 192) // Is a checkpoint
        {
            checkPointId++;
            NPC[numNPCs].Special = checkPointId;
            NPC[numNPCs].DefaultSpecial = NPC[numNPCs].Special;
        }
        else if(NPC[numNPCs].Type == 97 || NPC[numNPCs].Type == 196) // Is a star
        {
            bool tempBool = false;
            for(B = 1; B <= numStars; B++)
            {
                if(Star[B].level == FileName && (Star[B].Section == NPC[numNPCs].Section || Star[B].Section == -1))
                    tempBool = true;
            }

            if(tempBool)
            {
                NPC[numNPCs].Special = 1;
                NPC[numNPCs].DefaultSpecial = 1;
                if(NPC[numNPCs].Type == 196)
                    NPC[numNPCs].Killed = 9;
            }
        }
    }

    std::vector<int> twoWayWarps;
    for(auto &w : lvl.doors)
    {
        numWarps++;
        if(numWarps > maxWarps)
        {
            numWarps = maxWarps;
            break;
        }

        Warp[numWarps] = Warp_t();

        Warp[numWarps].PlacedEnt = true;
        Warp[numWarps].PlacedExit = true;
        Warp[numWarps].Entrance.X = w.ix;
        Warp[numWarps].Entrance.Y = w.iy;
        Warp[numWarps].Exit.X = w.ox;
        Warp[numWarps].Exit.Y = w.oy;
        Warp[numWarps].Direction = w.idirect;
        Warp[numWarps].Direction2 = w.odirect;
        Warp[numWarps].Effect = w.type;
        Warp[numWarps].level = w.lname;
        Warp[numWarps].LevelWarp = int(w.warpto);
        Warp[numWarps].LevelEnt = w.lvl_i;

        Warp[numWarps].MapWarp = w.lvl_o;
        Warp[numWarps].MapX = int(w.world_x);
        Warp[numWarps].MapY = int(w.world_y);

        Warp[numWarps].Stars = w.stars;
        Warp[numWarps].Layer = w.layer;
        Warp[numWarps].Hidden = w.unknown;

        Warp[numWarps].NoYoshi = w.novehicles;
        Warp[numWarps].WarpNPC = w.allownpc;
        Warp[numWarps].Locked = w.locked;

        Warp[numWarps].cannonExit = w.cannon_exit;
        Warp[numWarps].cannonExitSpeed = w.cannon_exit_speed;
        Warp[numWarps].eventEnter = w.event_enter;
        Warp[numWarps].StarsMsg = w.stars_msg;

        Warp[numWarps].Entrance.Height = 32;
        Warp[numWarps].Entrance.Width = 32;
        Warp[numWarps].Exit.Height = 32;
        Warp[numWarps].Exit.Width = 32;
        if(w.two_way)
            twoWayWarps.push_back(numWarps);
    }

    if(!twoWayWarps.empty())
    {
        for(auto &tww : twoWayWarps)
        {
            numWarps++;
            if(numWarps > maxWarps)
            {
                numWarps = maxWarps;
                break;
            }

            Warp[numWarps] = Warp[tww];
            Warp[numWarps].Exit = Warp[tww].Entrance;
            Warp[numWarps].Entrance = Warp[tww].Exit;
            Warp[numWarps].Direction2 = Warp[tww].Direction;
            Warp[numWarps].Direction = Warp[tww].Direction2;
        }
    }

    for(auto &w : lvl.physez)
    {
        numWater++;
        if(numWater > maxWater)
        {
            numWater = maxWater;
            break;
        }

        Water[numWater] = Water_t();

        Water[numWater].Location.X = w.x;
        Water[numWater].Location.Y = w.y;
        Water[numWater].Location.Width = w.w;
        Water[numWater].Location.Height = w.h;
        Water[numWater].Buoy = w.buoy;
        Water[numWater].Quicksand = w.env_type;
        Water[numWater].Layer = w.layer;
    }

    A = 0;
    for(auto &l : lvl.layers)
    {
        Layer[A] = Layer_t();

        Layer[A].Name = l.name;
        Layer[A].Hidden = l.hidden;
        if(Layer[A].Hidden)
        {
            HideLayer(Layer[A].Name, true);
        }
//        if(LevelEditor == true || MagicHand == true)
//        {
//            // Add into listbox
//        }
        A++;
        numLayers++;
        if(numLayers > maxLayers)
        {
            numLayers = maxLayers;
            break;
        }
    }

    A = 0;
    for(auto &e : lvl.events)
    {
        Events[A] = Events_t();

        Events[A].Name = e.name;
        Events[A].Text = e.msg;
        Events[A].Sound = int(e.sound_id);
        Events[A].EndGame = int(e.end_game);

        int hideLayersNum = int(e.layers_hide.size());
        int showLayersNum = int(e.layers_show.size());
        int toggleLayersNum = int(e.layers_toggle.size());

        for(B = 0; B <= maxSections; B++)
        {
            Events[A].HideLayer[B].clear();
            Events[A].ShowLayer[B].clear();
            Events[A].ToggleLayer[B].clear();
        }

        for(B = 0; B <= maxSections; B++)
        {
            if(B < hideLayersNum)
                Events[A].HideLayer[B] = e.layers_hide[size_t(B)];
            if(B < showLayersNum)
                Events[A].ShowLayer[B] = e.layers_show[size_t(B)];
            if(B < toggleLayersNum)
                Events[A].ToggleLayer[B] = e.layers_toggle[size_t(B)];
        }

        int maxSets = int(e.sets.size());
        if(maxSets > maxSections)
            maxSets = maxSections;

        for(B = 0; B <= maxSections; B++)
        {
            Events[A].Music[B] = LevelEvent_Sets::LESet_Nothing;
            Events[A].Background[B] = LevelEvent_Sets::LESet_Nothing;
            Events[A].level[B].X = LevelEvent_Sets::LESet_Nothing;
            Events[A].level[B].Y = 0;
            Events[A].level[B].Height = 0;
            Events[A].level[B].Width = 0;
        }

        for(B = 0; B <= maxSets; B++)
        {
            auto &s = e.sets[size_t(B)];
            Events[A].Music[B] = int(s.music_id);
            Events[A].Background[B] = int(s.background_id);
            Events[A].level[B].X = s.position_left;
            Events[A].level[B].Y = s.position_top;
            Events[A].level[B].Height = s.position_bottom;
            Events[A].level[B].Width = s.position_right;
        }

        Events[A].TriggerEvent = e.trigger;
        Events[A].TriggerDelay = e.trigger_timer;

        Events[A].LayerSmoke = e.nosmoke;

        Events[A].Controls.AltJump = e.ctrl_altjump;
        Events[A].Controls.AltRun = e.ctrl_altrun;
        Events[A].Controls.Down = e.ctrl_down;
        Events[A].Controls.Drop = e.ctrl_drop;
        Events[A].Controls.Jump = e.ctrl_jump;
        Events[A].Controls.Left = e.ctrl_left;
        Events[A].Controls.Right = e.ctrl_right;
        Events[A].Controls.Run = e.ctrl_run;
        Events[A].Controls.Start = e.ctrl_start;
        Events[A].Controls.Up = e.ctrl_up;

        Events[A].AutoStart = e.autostart;
        Events[A].MoveLayer = e.movelayer;
        Events[A].SpeedX = float(e.layer_speed_x);
        Events[A].SpeedY = float(e.layer_speed_y);

        Events[A].AutoX = float(e.move_camera_x);
        Events[A].AutoY = float(e.move_camera_y);
        Events[A].AutoSection = int(e.scroll_section);

        A++;
        numEvents++;
        if(numEvents > maxEvents)
        {
            numEvents = maxEvents;
            break;
        }
    }

    FindBlocks();
    qSortBackgrounds(1, numBackground);
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
        for(A = 0; A <= maxSections; A++) // Automatically correct 608 section height to 600
        {
//            if(int(level[A].Height - level[A].Y) == 608)
//                level[A].Y = level[A].Y + 8;
            int height = int(level[A].Height - level[A].Y);
            if(height > 600 && height < 610)
                level[A].Y = level[A].Height - 600; // Better and cleaner logic
        }

        B = numBackground;
        for(A = 1; A <= numWarps; A++)
        {
            if(Warp[A].Effect == 2 && Warp[A].Stars > numStars)
            {
                B++;
                numLocked++;
                Background[B] = Background_t();
                Background[B].Layer = Warp[A].Layer;
                Background[B].Hidden = Warp[A].Hidden;
                Background[B].Location.Width = 24;
                Background[B].Location.Height = 24;
                Background[B].Location.Y = Warp[A].Entrance.Y - Background[B].Location.Height;
                Background[B].Location.X = Warp[A].Entrance.X + Warp[A].Entrance.Width / 2.0 - Background[B].Location.Width / 2.0;
                Background[B].Type = 160;
            }
            else if(Warp[A].Effect == 2 && Warp[A].Locked) // For locks
            {
                B++;
                numLocked++;
                Background[B] = Background_t();
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
    NPC_t blankNPC = NPC_t();
    Water_t blankwater = Water_t();
    Warp_t blankWarp = Warp_t();
    Block_t blankBlock = Block_t();
    Background_t BlankBackground = Background_t();
    Location_t BlankLocation = Location_t();
    Events_t blankEvent = Events_t();
    NPCScore[274] = 6;
    LevelName.clear();
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

    numEvents = 0;
    newEventNum = 0;
    for(A = 0; A <= maxEvents; A++)
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
    Layer[0] = Layer_t();
    Layer[0].Name = "Default";
    Layer[0].Hidden = false;
    Layer[1] = Layer_t();
    Layer[1].Name = "Destroyed Blocks";
    Layer[1].Hidden = true;
    Layer[2] = Layer_t();
    Layer[2].Name = "Spawned NPCs";
    Layer[2].Hidden = false;

    numLayers = 0;
    for(A = 0; A <= maxLayers; A++)
    {
        Layer[A] = Layer_t();
        if(A > 2)
        {
            Layer[A].Name = "";
            Layer[A].Hidden = false;
        }
        Layer[A].SpeedX = 0;
        Layer[A].SpeedY = 0;
    }

//    If LevelEditor = True Or MagicHand = True Then
    if(LevelEditor || MagicHand)
    {
//        MessageText = ""
        MessageText.clear();
//        frmNPCs.chkMessage.Value = 0
//        frmBlocks.chkFill.Value = 0
//        frmEvents.txtEvent.Text = ""
//        noUpdate = True
        noUpdate = true;
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
    }

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
        LevelVWrap[A] = false;
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
    std::string newInput;
    LevelData head;

    for(A = 1; A <= numWarps; A++)
    {
        auto &warp = Warp[A];
        if(!warp.level.empty())
        {
            std::string lFile = FileNamePath + warp.level;
            addMissingLvlSuffix(lFile);
            if(Files::fileExists(lFile))
            {
                if(FileFormats::OpenLevelFileHeader(lFile, head))
                {
                    warp.maxStars = head.stars;
                    warp.curStars = 0;
                    for(B = 1; B <= numStars; B++)
                    {
                        if(SDL_strcasecmp(Star[B].level.c_str(), warp.level.c_str()) == 0)
                            warp.curStars++;
                    }
                }
            }

        }
    }
}

