#include "globals.h"
#include "sorting.h"
#include "layers.h"
#include "write_common.h"
#include "sound.h"
#include <PGE_File_Formats/file_formats.h>
#include "Logger/logger.h"

void SaveLevel(std::string FilePath, int format, int version)   // saves the level
{
    LevelData out;
    LevelBlock block;
    LevelBGO bgo;
    LevelSection section;
    LevelNPC npc;
    LevelDoor warp;
    LevelPhysEnv pez;
    LevelLayer layer;
    LevelSMBX64Event evt;
    PlayerPoint player;

    FileFormats::CreateLevelData(out);

    int A = 0;
    int B = 0;
    int C = 0;
//    int starCount = 0;

    // put NPC types 60, 62, 64, 66, and 78-83 first. (why?)
    for(A = 1; A <= numNPCs; A++)
    {
        if(NPC[A].Type == 60 || NPC[A].Type == 62 || NPC[A].Type == 64 || NPC[A].Type == 66 || (NPC[A].Type >= 78 && NPC[A].Type <= 83))
        {
            // swap it with the first NPC that isn't one of the special ones
            // this started as B = 1 but C + 1 will work now.
            for(B = C + 1; B < A; B++)
            {
                if (!(NPC[B].Type == 60 || NPC[B].Type == 62 || NPC[B].Type == 64 || NPC[B].Type == 66 || (NPC[B].Type >= 78 && NPC[B].Type <= 83)))
                {
                    std::swap(NPC[A], NPC[B]);
                    break;
                }
            }
            C++;
            // we know that the first C slots are all these types
        }
        // C++ was here but that is a logical flaw.
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

//    starCount = 0;
//    for (int A = 1; A < numNPCs; A++)
//    {
//        if(NPC[A].Type == 97 || NPC[A].Type == 196)
//            starCount ++;
//    }
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
//    fwritenum(f, curRelease);
//    fwritenum(f, starCount);
//    fwritestr(f, LevelName);

    for(int i = 0; i <= numSections; ++i)
    {
        auto &s = level[i];

        section.id = i;
        section.size_left = s.X;
        section.size_top  = s.Y;
        section.size_right = s.Width;
        section.size_bottom  = s.Height;
        section.music_id = bgMusic[i];
        section.bgcolor = bgColor[i];
        section.wrap_h = LevelWrap[i];
        section.wrap_v = LevelVWrap[i];
        section.OffScreenEn = OffScreenExit[i];
        section.background = Background2[i];
        section.lock_left_scroll = NoTurnBack[i];
        section.underwater = UnderWater[i];
        section.music_file = CustomMusic[i];

        if(out.sections.size() >= size_t(i))
            out.sections.push_back(section);
        else
            out.sections[i] = section;
    }

//    for(B = 0; B <= 20; B++)
//    {
//        fwritenum(f, level[B].X);
//        fwritenum(f, level[B].Y);
//        fwritenum(f, level[B].Height);
//        fwritenum(f, level[B].Width);
//        fwritenum(f, bgMusic[B]);
//        fwritenum(f, bgColor[B]);
//        fwritebool(f, LevelWrap[B]);
//        fwritebool(f, OffScreenExit[B]);
//        fwritenum(f, Background2[B]);
//        fwritebool(f, NoTurnBack[B]);
//        fwritebool(f, UnderWater[B]);
//        fwritestr(f, CustomMusic[B]);
//    }

    for(int i = 1; i <= 2; ++i)
    {
        auto &p = PlayerStart[i];
        player.id = i;
        player.x = p.X;
        player.y = p.Y;
        player.w = p.Width;
        player.h = p.Height;
        player.direction = p.Direction;
        out.players.push_back(player);
    }

//    for(A = 1; A <= 2; A++)
//    {
//        fwritenum(f, PlayerStart[A].X);
//        fwritenum(f, PlayerStart[A].Y);
//        fwritenum(f, PlayerStart[A].Width);
//        fwritenum(f, PlayerStart[A].Height);
//    }

    for(int i = 1; i <= numBlock; ++i)
    {
        auto &b = Block[i];

        block.id = b.Type;
        block.x = b.Location.X;
        block.y = b.Location.Y;
        block.w = b.Location.Width;
        block.h = b.Location.Height;

        if(b.Special >= 1000)
            block.npc_id = b.Special - 1000;
        else if(b.Special > 0)
            block.npc_id = -b.Special;
        else
            block.npc_id = 0;

        block.invisible = b.Invis;
        block.slippery = b.Slippy;
        block.layer = b.Layer;

        // fix this to update as needed
        if(block.layer.empty())
            block.layer = "Default";

        block.event_destroy = b.TriggerDeath;
        block.event_hit = b.TriggerHit;
        block.event_emptylayer = b.TriggerLast;

        block.meta.array_id = (out.blocks_array_id++);

        out.blocks.push_back(block);
    }

//    for(A = 1; A <= numBlock; A++)
//    {
//        fwritenum(f, Block[A].Location.X);
//        fwritenum(f, Block[A].Location.Y);
//        fwritenum(f, Block[A].Location.Height);
//        fwritenum(f, Block[A].Location.Width);
//        fwritenum(f, Block[A].Type);
//        fwritenum(f, Block[A].Special);
//        fwritebool(f, Block[A].Invis);
//        fwritebool(f, Block[A].Slippy);
//        // fix this to update as needed
//        if (Block[A].Layer == "")
//            Block[A].Layer = "Default";
//        fwritestr(f, Block[A].Layer);
//        fwritestr(f, Block[A].TriggerDeath);
//        fwritestr(f, Block[A].TriggerHit);
//        fwritestr(f, Block[A].TriggerLast);
//    }

//    fprintf(f, "next\r\n");
    for(int i = 1; i < numBackground; ++i)
    {
        auto &b = Background[i];

        bgo.id = b.Type;
        bgo.x = b.Location.X;
        bgo.y = b.Location.Y;
        bgo.layer = b.Layer;

        bgo.z_mode = b.zMode;
        bgo.z_offset = b.zOffset;
        bgo.smbx64_sp = bgo.z_mode == LevelBGO::ZDefault ? b.SortPriority : -1;

        // fix this to update as needed
        if(bgo.layer.empty())
            bgo.layer = "Default";

        bgo.meta.array_id = out.bgo_array_id++;
        out.bgo.push_back(bgo);
    }

//    for(A = 1; A <= numBackground; A++)
//    {
//        fwritenum(f, Background[A].Location.X);
//        fwritenum(f, Background[A].Location.Y);
//        fwritenum(f, Background[A].Type);
//        // fix this to update as needed
//        if(bgo.layer.empty())
//           bgoblock.layer = "Default";
//        fwritestr(f, Background[A].Layer);
//    }

//    fprintf(f, "next\r\n");
    for(int i = 1; i < numNPCs; ++i)
    {
        auto &n = NPC[i];

        npc.id = n.Type;
        npc.x = n.Location.X;
        npc.y = n.Location.Y;
        npc.direct = n.Direction;

        if(n.Type == 91 || n.Type == 96 || n.Type == 283 || n.Type == 284)
            npc.contents = n.Special;

        if(n.Type == 288 || n.Type == 289 || (n.Type == 91 && n.Special == 288))
            npc.special_data = n.Special2;
        else if(n.Type == 260 || NPCIsAParaTroopa[n.Type] || NPCIsCheep[n.Type])
            npc.special_data = n.Special;
        else if(n.Type == 86)
            npc.special_data = n.Special7;

        npc.generator = n.Generator;
        npc.generator_direct = n.GeneratorDirection;
        npc.generator_period = n.GeneratorTimeMax;
        npc.generator_type = n.GeneratorEffect;
        npc.attach_layer = n.AttLayer;

        npc.msg = n.Text;
        npc.friendly = n.Inert;
        npc.nomove = n.Stuck;
        npc.is_boss = n.Legacy;

        npc.layer = n.Layer;

        npc.event_activate = n.TriggerActivate;
        npc.event_die = n.TriggerDeath;
        npc.event_talk = n.TriggerTalk;
        npc.event_emptylayer = n.TriggerLast;

        // fix this to update as needed
        if(npc.layer.empty())
            npc.layer = "Default";

        npc.meta.array_id = out.npc_array_id++;
        out.npc.push_back(npc);
    }


//    for(A = 1; A <= numNPCs; A++)
//    {
//        fwritenum(f, NPC[A].Location.X);
//        fwritenum(f, NPC[A].Location.Y);
//        fwritenum(f, NPC[A].Direction);
//        fwritenum(f, NPC[A].Type);

//        if (NPC[A].Type == 91 || NPC[A].Type == 96 || NPCIsAParaTroopa[NPC[A].Type] || NPC[A].Type == 283 || NPC[A].Type == 284 || NPCIsCheep[NPC[A].Type] || NPC[A].Type == 260)
//            fwritenum(f, NPC[A].Special);
//        // "potion"
//        if (NPC[A].Type == 288 || NPC[A].Type == 289 || (NPC[A].Type == 91 && NPC[A].Special == 288))
//            fwritenum(f, NPC[A].Special2);

//        fwritebool(f, NPC[A].Generator);

//        if (NPC[A].Generator)
//        {
//            fwritenum(f, NPC[A].GeneratorDirection);
//            fwritenum(f, NPC[A].GeneratorEffect);
//            fwritenum(f, NPC[A].GeneratorTimeMax);
//        }

//        fwritestr_multiline(f, NPC[A].Text);
//        fwritebool(f, NPC[A].Inert);
//        fwritebool(f, NPC[A].Stuck);
//        fwritebool(f, NPC[A].Legacy);

//        if (NPC[A].Layer.empty())
//            NPC[A].Layer = "Default";

//        fwritestr(f, NPC[A].Layer);
//        fwritestr(f, NPC[A].TriggerActivate);
//        fwritestr(f, NPC[A].TriggerDeath);
//        fwritestr(f, NPC[A].TriggerTalk);
//        fwritestr(f, NPC[A].TriggerLast);
//        fwritestr(f, NPC[A].AttLayer);
//    }

//    fprintf(f, "next\r\n");

    for(int i = 1; i < numWarps; ++i)
    {
        auto &w = Warp[i];

        warp.ix = w.Entrance.X;
        warp.iy = w.Entrance.Y;
        warp.ox = w.Exit.X;
        warp.oy = w.Exit.Y;
        warp.isSetIn = w.PlacedEnt;
        warp.isSetOut = w.PlacedExit;
        warp.idirect = w.Direction;
        warp.odirect = w.Direction2;

        warp.type = w.Effect;
        warp.two_way = w.twoWay;
        warp.lname = w.level;

        warp.warpto = w.LevelWarp;
        warp.lvl_i = w.LevelEnt;

        warp.lvl_o = w.MapWarp;
        warp.world_x = w.MapX;
        warp.world_y = w.MapY;

        warp.stars = w.Stars;
        warp.layer = w.Layer;
        warp.unknown = w.Hidden;

        warp.novehicles = w.NoYoshi;
        warp.allownpc = w.WarpNPC;
        warp.locked = w.Locked;

        warp.cannon_exit = w.cannonExit;
        warp.cannon_exit_speed = w.cannonExitSpeed;
        warp.event_enter = w.eventEnter;
        warp.stars_msg = w.StarsMsg;
        warp.star_num_hide = w.noPrintStars;
        warp.hide_entering_scene = w.noEntranceScene;

        // fix this to update as needed
        if(warp.layer.empty())
            warp.layer = "Default";

        warp.meta.array_id = out.doors_array_id++;
        out.doors.push_back(warp);
    }


//    for(A = 1; A <= numWarps + 1; A++)
//    {
//        if (!Warp[A].PlacedEnt || !Warp[A].PlacedExit) continue;
//        fwritenum(f, Warp[A].Entrance.X);
//        fwritenum(f, Warp[A].Entrance.Y);
//        fwritenum(f, Warp[A].Exit.X);
//        fwritenum(f, Warp[A].Exit.Y);
//        fwritenum(f, Warp[A].Direction);
//        fwritenum(f, Warp[A].Direction2);
//        fwritenum(f, Warp[A].Effect);

//        fwritestr(f, Warp[A].level);
//        fwritenum(f, Warp[A].LevelWarp);
//        fwritebool(f, Warp[A].LevelEnt);

//        fwritebool(f, Warp[A].MapWarp);
//        fwritenum(f, Warp[A].MapX);
//        fwritenum(f, Warp[A].MapY);

//        fwritenum(f, Warp[A].Stars);

//        if (Warp[A].Layer == "")
//            Warp[A].Layer = "Default";
//        fwritestr(f, Warp[A].Layer);
//        fwritebool(f, Warp[A].Hidden);

//        fwritebool(f, Warp[A].NoYoshi);
//        fwritebool(f, Warp[A].WarpNPC);
//        fwritebool(f, Warp[A].Locked);
//    }

//    fprintf(f, "next\r\n");

    for(int i = 1; i < numWater; ++i)
    {
        auto &p = Water[A];

        pez.x = p.Location.X;
        pez.y = p.Location.Y;
        pez.w = p.Location.Width;
        pez.h = p.Location.Height;
        pez.buoy = p.Buoy;
        pez.env_type = p.Quicksand ? LevelPhysEnv::ENV_QUICKSAND : LevelPhysEnv::ENV_WATER;
        pez.layer = p.Layer;

        // fix this to update as needed
        if(pez.layer.empty())
            pez.layer = "Default";

        pez.meta.array_id = out.physenv_array_id++;
        out.physez.push_back(pez);
    }

//    for(A = 1; A <= numWater; A++)
//    {
//        fwritenum(f, Water[A].Location.X);
//        fwritenum(f, Water[A].Location.Y);
//        fwritenum(f, Water[A].Location.Width);
//        fwritenum(f, Water[A].Location.Height);
//        fwritenum(f, Water[A].Buoy);
//        fwritebool(f, Water[A].Quicksand);
//        fwritestr(f, Water[A].Layer);
//    }

//    fprintf(f, "next\r\n");

    for(int i = 0; i < numLayers; ++i)
    {
        auto &l = Layer[A];

        layer.name = l.Name;
        layer.hidden = l.Hidden;

        layer.meta.array_id = out.layers_array_id++;
        out.layers.push_back(layer);
    }

//    // (removed code that made sure Name and Hidden were synced with the UI)
//    for(A = 0; A <= 100; A++)
//    {
//        if (Layer[A].Name == "") break;
//        fwritestr(f, Layer[A].Name);
//        fwritebool(f, Layer[A].Hidden);
//    }

//    fprintf(f, "next\r\n");
    for(int i = 0; i < numEvents; ++i)
    {
        auto &e = Events[i];

        evt.name = e.Name;
        evt.msg = e.Text;
        evt.sound_id = e.Sound;
        evt.end_game = e.EndGame;
        evt.layers_hide = e.HideLayer;
        evt.layers_show = e.ShowLayer;
        evt.layers_toggle = e.ToggleLayer;

        LevelEvent_Sets s;

        for(int j = 0; j < numSections; ++j)
        {
            auto &ss = e.section[j];

            s.music_id = ss.music_id;
            s.background_id = ss.background_id;
            s.music_file = ss.music_file;

            s.position_left = ss.position.X;
            s.position_top = ss.position.Y;
            s.position_right = ss.position.Width;
            s.position_bottom = ss.position.Height;

            s.autoscrol = ss.autoscroll;
            s.autoscrol_x = ss.autoscroll_x;
            s.autoscrol_y = ss.autoscroll_y;
            s.autoscroll_style = LevelEvent_Sets::AUTOSCROLL_SIMPLE;

            if(j >= (int)evt.sets.size())
                evt.sets.push_back(s);
            else
                evt.sets[j] = s;
        }

        evt.trigger = e.TriggerEvent;
        evt.trigger_timer = e.TriggerDelay;

        evt.nosmoke = e.LayerSmoke;

        evt.ctrl_altjump = e.Controls.AltJump;
        evt.ctrl_altrun = e.Controls.AltRun;
        evt.ctrl_down   = e.Controls.Down;
        evt.ctrl_drop   = e.Controls.Drop;
        evt.ctrl_jump   = e.Controls.Jump;
        evt.ctrl_left   = e.Controls.Left;
        evt.ctrl_right  = e.Controls.Right;
        evt.ctrl_run    = e.Controls.Run;
        evt.ctrl_start  = e.Controls.Start;
        evt.ctrl_up     = e.Controls.Up;

        evt.autostart = e.AutoStart;
        evt.movelayer = e.MoveLayer;
        evt.layer_speed_x = e.SpeedX;
        evt.layer_speed_y = e.SpeedY;

        evt.move_camera_x = e.AutoX;
        evt.move_camera_y = e.AutoY;
        evt.scroll_section = e.AutoSection;

        evt.meta.array_id = out.events_array_id++;
        out.events.push_back(evt);
    }

//    for(A = 0; A <= 100; A++)
//    {
//        if(Events[A].Name.empty())
//            break;

//        fwritestr(f, Events[A].Name);
//        fwritestr_multiline(f, Events[A].Text);
//        fwritenum(f, Events[A].Sound);
//        fwritenum(f, Events[A].EndGame);

//        for(B = 0; B <= 20; B++)
//        {
//            if(B < (int)Events[A].HideLayer.size())
//                fwritestr(f, Events[A].HideLayer[B]);
//            else
//                fwritestr(f, "");
//            if(B < (int)Events[A].ShowLayer.size())
//                fwritestr(f, Events[A].ShowLayer[B]);
//            else
//                fwritestr(f, "");
//            if(B < (int)Events[A].ToggleLayer.size())
//                fwritestr(f, Events[A].ToggleLayer[B]);
//            else
//                fwritestr(f, "");
//        }

//        for(B = 0; B <= 20; B++)
//        {
//            fwritenum(f, Events[A].section[B].music_id);
//            fwritenum(f, Events[A].section[B].background_id);
//            fwritenum(f, Events[A].section[B].position.X);
//            fwritenum(f, Events[A].section[B].position.Y);
//            fwritenum(f, Events[A].section[B].position.Height);
//            fwritenum(f, Events[A].section[B].position.Width);
//        }

//        fwritestr(f, Events[A].TriggerEvent);
//        fwritenum(f, Events[A].TriggerDelay);
//        fwritebool(f, Events[A].LayerSmoke);

//        fwritebool(f, Events[A].Controls.AltJump);
//        fwritebool(f, Events[A].Controls.AltRun);
//        fwritebool(f, Events[A].Controls.Down);
//        fwritebool(f, Events[A].Controls.Drop);
//        fwritebool(f, Events[A].Controls.Jump);
//        fwritebool(f, Events[A].Controls.Left);
//        fwritebool(f, Events[A].Controls.Right);
//        fwritebool(f, Events[A].Controls.Run);
//        fwritebool(f, Events[A].Controls.Start);
//        fwritebool(f, Events[A].Controls.Up);

//        fwritebool(f, Events[A].AutoStart);
//        fwritestr(f, Events[A].MoveLayer);
//        fwritefloat(f, Events[A].SpeedX);
//        fwritefloat(f, Events[A].SpeedY);

//        fwritefloat(f, Events[A].AutoX);
//        fwritefloat(f, Events[A].AutoY);
//        fwritenum(f, Events[A].AutoSection);
//    }

//    fclose(f);

    if(!FileFormats::SaveLevelFile(out, FilePath, (FileFormats::LevelFileFormat)format, version))
    {
        pLogWarning("Error while saving the level file: %s", out.meta.ERROR_info.c_str());
        return;
    }

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
