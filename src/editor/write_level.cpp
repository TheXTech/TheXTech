#include "globals.h"
#include "sorting.h"
#include "layers.h"
#include "write_common.h"
#include "sound.h"
#include "npc_id.h"
#include "npc_special_data.h"
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

    for(int i = 0; i < numSections; ++i)
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

        // swapped order of operands because
        // previous code did not make sense
        if(size_t(i) >= out.sections.size())
            out.sections.push_back(section);
        else
            out.sections[i] = section;
    }

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
        block.layer = GetL(b.Layer);

        // fix this to update as needed
        if(block.layer.empty())
            block.layer = "Default";

        block.event_destroy = GetE(b.TriggerDeath);
        block.event_hit = GetE(b.TriggerHit);
        block.event_emptylayer = GetE(b.TriggerLast);

        // NEW: legacy behavior for spin block
        if(b.Type == 90)
            block.special_data = b.Special2;

        block.meta.array_id = (out.blocks_array_id++);

        out.blocks.push_back(block);
    }

    for(int i = 1; i <= numBackground; ++i)
    {
        auto &b = Background[i];

        bgo.id = b.Type;
        bgo.x = b.Location.X;
        bgo.y = b.Location.Y;
        bgo.layer = GetL(b.Layer);

        bgo.z_mode = b.zMode;
        bgo.z_offset = b.zOffset;
        bgo.smbx64_sp = bgo.z_mode == LevelBGO::ZDefault ? b.SortPriority : -1;

        // fix this to update as needed
        if(bgo.layer.empty())
            bgo.layer = "Default";

        bgo.meta.array_id = out.bgo_array_id++;
        out.bgo.push_back(bgo);
    }

    for(int i = 1; i <= numNPCs; ++i)
    {
        auto &n = NPC[i];

        npc.id = n.Type;
        npc.x = n.Location.X;
        npc.y = n.Location.Y;
        npc.direct = n.Direction;

        if(n.Type == NPCID_BURIEDPLANT || n.Type == NPCID_YOSHIEGG ||
           n.Type == NPCID_BUBBLE || n.Type == NPCID_LAKITU_SMW)
        {
            npc.contents = n.Special;
        }

        // Warp Section pointer
        if(n.Type == NPCID_POTION || n.Type == NPCID_POTIONDOOR ||
          (n.Type == NPCID_BURIEDPLANT && n.Special == NPCID_POTION))
        {
            npc.special_data = n.Special2;
        }
        // AI / firebar length
        else if(n.Type == NPCID_FIREBAR || NPCIsAParaTroopa[n.Type] || NPCIsCheep[n.Type])
        {
            npc.special_data = n.Special;
        }
        // Legacy and custom behaviors
        else if(find_Special7_Data(n.Type) != nullptr)
        {
            npc.special_data = n.Special7;
        }

        npc.generator = n.Generator;
        npc.generator_direct = n.GeneratorDirection;
        npc.generator_period = n.GeneratorTimeMax;
        npc.generator_type = n.GeneratorEffect;
        npc.attach_layer = GetL(n.AttLayer);

        npc.msg = GetS(n.Text);
        npc.friendly = n.Inert;
        npc.nomove = n.Stuck;
        npc.is_boss = n.Legacy;

        npc.layer = GetL(n.Layer);

        npc.event_activate = GetE(n.TriggerActivate);
        npc.event_die = GetE(n.TriggerDeath);
        npc.event_talk = GetE(n.TriggerTalk);
        npc.event_emptylayer = GetE(n.TriggerLast);

        // fix this to update as needed
        if(npc.layer.empty())
            npc.layer = "Default";

        npc.meta.array_id = out.npc_array_id++;
        out.npc.push_back(npc);
    }

    for(int i = 1; i <= numWarps; ++i)
    {
        auto &w = Warp[i];

        // no case where user would want to save incomplete warp in classic editor
        if(!w.PlacedEnt || !w.PlacedExit)
            continue;

        warp.ix = w.Entrance.X;
        warp.iy = w.Entrance.Y;
        warp.ox = w.Exit.X;
        warp.oy = w.Exit.Y;
        warp.isSetIn = w.PlacedEnt;
        warp.isSetOut = w.PlacedExit;
        warp.idirect = w.Direction;
        warp.odirect = w.Direction2;

        warp.type = w.Effect;
        warp.lname = GetS(w.level);

        warp.warpto = w.LevelWarp;
        warp.lvl_i = w.LevelEnt;

        warp.lvl_o = w.MapWarp;
        warp.world_x = w.MapX;
        warp.world_y = w.MapY;

        warp.stars = w.Stars;
        warp.layer = GetL(w.Layer);
        warp.unknown = w.Hidden;

        warp.novehicles = w.NoYoshi;
        warp.allownpc = w.WarpNPC;
        warp.locked = w.Locked;

        // custom fields:
        warp.two_way = w.twoWay;

        warp.cannon_exit = w.cannonExit;
        warp.cannon_exit_speed = w.cannonExitSpeed;
        warp.event_enter = GetE(w.eventEnter);
        warp.stars_msg = GetS(w.StarsMsg);
        warp.star_num_hide = w.noPrintStars;
        warp.hide_entering_scene = w.noEntranceScene;

        warp.stood_state_required = w.stoodRequired;
        warp.transition_effect = w.transitEffect;

        // fix this to update as needed
        if(warp.layer.empty())
            warp.layer = "Default";

        warp.meta.array_id = out.doors_array_id++;
        out.doors.push_back(warp);
    }

    for(int i = 1; i <= numWater; ++i)
    {
        auto &p = Water[i];

        pez.x = p.Location.X;
        pez.y = p.Location.Y;
        pez.w = p.Location.Width;
        pez.h = p.Location.Height;
        pez.buoy = p.Buoy;
        pez.env_type = p.Quicksand ? LevelPhysEnv::ENV_QUICKSAND : LevelPhysEnv::ENV_WATER;
        pez.layer = GetL(p.Layer);

        // fix this to update as needed
        if(pez.layer.empty())
            pez.layer = "Default";

        pez.meta.array_id = out.physenv_array_id++;
        out.physez.push_back(pez);
    }

    for(int i = 0; i < numLayers; ++i)
    {
        auto &l = Layer[i];

        layer.name = l.Name;
        layer.hidden = l.Hidden;

        layer.meta.array_id = out.layers_array_id++;
        out.layers.push_back(layer);
    }

    for(int i = 0; i < numEvents; ++i)
    {
        auto &e = Events[i];

        evt.name = e.Name;
        evt.msg = GetS(e.Text);
        evt.sound_id = e.Sound;
        evt.end_game = e.EndGame;
        evt.layers_hide.clear();
        for(layerindex_t i : e.HideLayer)
        {
            evt.layers_hide.push_back(GetL(i));
        }
        evt.layers_show.clear();
        for(layerindex_t i : e.ShowLayer)
        {
            evt.layers_show.push_back(GetL(i));
        }
        evt.layers_toggle.clear();
        for(layerindex_t i : e.ToggleLayer)
        {
            evt.layers_toggle.push_back(GetL(i));
        }

        LevelEvent_Sets s;

        for(int j = 0; j < numSections; ++j)
        {
            auto &ss = e.section[j];

            s.id = j;

            s.music_id = ss.music_id;
            s.background_id = ss.background_id;
            s.music_file = GetS(ss.music_file);

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

        evt.trigger = GetE(e.TriggerEvent);
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
        evt.movelayer = GetL(e.MoveLayer);
        evt.layer_speed_x = e.SpeedX;
        evt.layer_speed_y = e.SpeedY;

        evt.move_camera_x = e.AutoX;
        evt.move_camera_y = e.AutoY;
        evt.scroll_section = e.AutoSection;

        evt.meta.array_id = out.events_array_id++;
        out.events.push_back(evt);
    }

    if(!FileFormats::SaveLevelFile(out, FilePath, (FileFormats::LevelFileFormat)format, version))
    {
        pLogWarning("Error while saving the level file: %s", out.meta.ERROR_info.c_str());
        PlaySound(SFX_Smash);
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
    PlaySound(SFX_GotItem);
}
