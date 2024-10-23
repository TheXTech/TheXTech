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

#include "sdl_proxy/sdl_stdinc.h"

#ifdef THEXTECH_INTERPROC_SUPPORTED
#   include <InterProcess/intproc.h>
#endif
#include <Logger/logger.h>
#include <Utils/elapsed_timer.h>
#include <pge_delay.h>
#include <fmt_format_ne.h>

#include "globals.h"
#include "config.h"
#include "editor.h"
#include "graphics.h"
#include "sound.h"
#include "collision.h"
#include "npc.h"
#include "blocks.h"
#include "sorting.h"
#include "player.h"
#include "effect.h"
#include "layers.h"
#include "game_main.h"
#include "change_res.h"
#include "main/level_file.h"
#include "main/cheat_code.h"
#include "main/trees.h"
#include "main/game_globals.h"
#include "main/screen_connect.h"
#include "main/screen_quickreconnect.h"
#include "main/game_strings.h"
#include "main/speedrunner.h"
#include "load_gfx.h"
#include "core/render.h"
#include "core/window.h"
#include "core/events.h"
#include "fontman/font_manager.h"
#include "npc/section_overlap.h"

#include "controls.h"

#include "pseudo_vb.h"
#include "npc_traits.h"
#include "npc_id.h"
#include "eff_id.h"

#include "write_level.h"
#include "write_world.h"
#include "editor/new_editor.h"

#include "editor/magic_block.h"
#include "editor/editor_custom.h"

#include <PGE_File_Formats/file_formats.h>

// static int ScrollDelay = 0; // slows down the camera movement when scrolling through a level
//Public Declare Function GetCursorPos Lib "user32" (lpPoint As POINTAPI) As long long;

std::string Backup_FullFileName;

Point_t CursorPos;

bool MouseCancel = false;
bool HasCursor = false;
// bool NoReallyKillIt = false; //Unused
int curSection = 0;

bool enableAutoAlign = true;

int last_vScreenX[maxSections+1];
int last_vScreenY[maxSections+1];

// backup values
int curSection_b = 0;
int last_vScreenX_b[maxSections+1];
int last_vScreenY_b[maxSections+1];

// number of frames to show section
int editor_section_toast = 0;

// buffer for scrolling since we can only scroll in 32-pixel increments
int scroll_buffer_x = 0;
int scroll_buffer_y = 0;

// to prevent constant replacement of tiled items during "replace_existing" mode
Location_t last_EC_loc;

// the first f stands for "fixed"
constexpr bool ffEqual(double i, double j)
{
    return (i - j > -0.1) && (i - j < 0.1);
}

void ResetSectionScrolls()
{
    int p1_section = 0;

    // use screen 0 for section scrolls
    const Screen_t& screen = Screens[0];

    for(int i = 0; i <= maxSections; i++)
    {
        // initialize the section
        if(level[i].Height == level[i].Y)
        {
            level[i].Height = (-200000 + 20000 * i);
            level[i].Y = level[i].Height - 600;
            level[i].X = (-200000 + 20000 * i);
            level[i].Width = level[i].X + 800;

            // initialize player positions
            if(i == 0)
            {
                for(int p = 1; p <= 2; p++)
                {
                    PlayerStart[p].Width = Physics.PlayerWidth[p][2];
                    PlayerStart[p].Height = Physics.PlayerHeight[p][2];
                    PlayerStart[p].X = level[i].X + 128 - 64 * (p - 1);
                    PlayerStart[p].Y = level[i].Height - 32 - PlayerStart[p].Height;
                }
            }
        }

        // normally start at bottom-left
        last_vScreenY[i] = -(level[i].Height - screen.H);
        last_vScreenX[i] = -(level[i].X);

        // if player start is in section, start there instead
        for(int p = 1; p <= 2; p++)
        {
            if(PlayerStart[p].X >= level[i].X && PlayerStart[p].X + PlayerStart[p].Width <= level[i].Width
                && PlayerStart[p].Y >= level[i].Y && PlayerStart[p].Y + PlayerStart[p].Height <= level[i].Height)
            {
                // center on player
                last_vScreenX[i] = -(PlayerStart[p].X + PlayerStart[p].Width / 2 - screen.W / 2);
                last_vScreenY[i] = -(PlayerStart[p].Y + PlayerStart[p].Height / 2 - screen.H / 2);

                // check section bounds
                if(-last_vScreenX[i] < level[i].X)
                    last_vScreenX[i] = -level[i].X;
                else if(-last_vScreenX[i] + screen.W > level[i].Width)
                    last_vScreenX[i] = -(level[i].Width - screen.W);

                if(-last_vScreenY[i] < level[i].Y)
                    last_vScreenY[i] = -level[i].Y;
                else if(-last_vScreenY[i] + screen.H > level[i].Height)
                    last_vScreenY[i] = -(level[i].Height - screen.H);

                // save P1 section
                if(p == 1)
                    p1_section = i;

                // don't check other player
                break;
            }
        }

        // center on section if screen bigger
        if(level[i].Width - level[i].X < screen.W)
            last_vScreenX[i] += screen.W / 2 - (level[i].Width - level[i].X) / 2;
        if(level[i].Height - level[i].Y < screen.H)
            last_vScreenY[i] = -level[i].Y + screen.H / 2 - (level[i].Height - level[i].Y) / 2;

        // align to grid
        if(std::fmod((last_vScreenY[i] + 8), 32) != 0.0)
            last_vScreenY[i] = static_cast<int>(round((last_vScreenY[i] + 8) / 32)) * 32 - 8;
        if(std::fmod(last_vScreenX[i], 32) != 0.0)
            last_vScreenX[i] = static_cast<int>(round(last_vScreenX[i] / 32)) * 32;
    }

    curSection = p1_section;
    vScreen[1].Y = last_vScreenY[curSection];
    vScreen[1].X = last_vScreenX[curSection];
}

void SetSection(int i)
{
    if(curMusic != bgMusic[i])
        StartMusic(i);
    else if(bgMusic[i] == 24)
    {
        if(curSection >= 0)
            if(CustomMusic[curSection] != CustomMusic[i])
                StartMusic(i);
    }

    if(curSection != i)
    {
        PlaySound(SFX_Slide);
        editor_section_toast = 66;
    }

    last_vScreenY[curSection] = vScreen[1].Y;
    last_vScreenX[curSection] = vScreen[1].X;
    curSection = i;
    vScreen[1].Y = last_vScreenY[curSection];
    vScreen[1].X = last_vScreenX[curSection];
}

void EditorBackup()
{
    last_vScreenY[curSection] = vScreen[1].Y;
    last_vScreenX[curSection] = vScreen[1].X;
    for(int i = 0; i <= maxSections; i++)
    {
        last_vScreenX_b[i] = last_vScreenX[i];
        last_vScreenY_b[i] = last_vScreenY[i];
    }
    curSection_b = curSection;
}

void EditorRestore()
{
    for(int i = 0; i <= maxSections; i++)
    {
        last_vScreenX[i] = last_vScreenX_b[i];
        last_vScreenY[i] = last_vScreenY_b[i];
    }
    curSection = curSection_b;
    vScreen[1].X = last_vScreenX_b[curSection];
    vScreen[1].Y = last_vScreenY_b[curSection];
    SetSection(curSection);
    vScreen[1].X = last_vScreenX_b[curSection];
    vScreen[1].Y = last_vScreenY_b[curSection];
}

void EditorCursor_t::ClearStrings()
{
    FreeS(this->NPC.Text);
    FreeS(this->Warp.StarsMsg);
    FreeS(this->Warp.level);
    FreeS(this->WorldMusic.MusicFile);
}

enum InteractFlags
{
    IF_AltMode =  1,
    IF_ResizeT =  2,
    IF_ResizeB =  4,
    IF_ResizeL =  8,
    IF_ResizeR = 16,
};

void UpdateInteract();
template<class LocType>
void InteractResize(LocType& loc, int min, int snap);

void InteractResizeSection(SpeedlessLocation_t& section);

// this sub handles the level editor
// it is still called when the player is testing a level in the editor in windowed mode
void UpdateEditor()
{
    if(!MagicHand)
        Controls::PollInputMethod();

    int A = 0;
    // int B = 0;
//    int C = 0;
    int qLevel = 0;
    bool CanPlace = false; // Determines if something is in the way
//    bool grabBool = false;
    Location_t tempLocation;
//    if(Debugger == true)
//        frmLevelDebugger::UpdateDisplay;
    GameMenu = false;

    GetEditorControls();

    if(!SharedCursor.Primary && !EditorControls.SwitchScreens && !EditorControls.TestPlay)
    {
        MouseRelease = true;
        MouseCancel = false;
        if(EditorCursor.SubMode > 0 && EditorCursor.Mode == OptCursor_t::LVL_ERASER)
            EditorCursor.SubMode = 0;
    }

    if(EditorCursor.Y < 40)
    {
        MouseCancel = true;
        EditorCursor.InteractMode = 0;
        EditorCursor.InteractFlags = 0;
        EditorCursor.InteractIndex = 0;
    }

    bool MouseClick_Current = SharedCursor.Primary && !MouseCancel;

    if(LevelEditor)
        numPlayers = 0;

    if(MagicHand)
    {
        // keep the cursor's position correct when the player moves!
        MouseMove(EditorCursor.X, EditorCursor.Y, true);
    }
    else
    {
        FreezeNPCs = false;
        LevelMacro = LEVELMACRO_OFF;
        LevelMacroCounter = 0;
    }

    if(!MagicHand)
    {
        if(EditorControls.PrevSection && !WorldEditor)
        {
            if(ScrollRelease)
            {
                ScrollRelease = false;
                if(curSection != 0)
                    SetSection(curSection - 1);
            }
        }
        else if(EditorControls.NextSection && !WorldEditor)
        {
            if(ScrollRelease)
            {
                ScrollRelease = false;
                if(curSection != maxSections)
                    SetSection(curSection + 1);
            }
        }
        else
            ScrollRelease = true;

        if(std::fmod((vScreen[1].Y + 8), 32) != 0.0)
            vScreen[1].Y = static_cast<int>(floor(static_cast<double>(vScreen[1].Y / 32))) * 32 - 8;
        if(std::fmod(vScreen[1].X, 32) != 0.0)
            vScreen[1].X = static_cast<int>(floor(static_cast<double>(vScreen[1].X / 32))) * 32;
    }
    else
    {
        curSection = Player[1].Section;
    }


    // if(!XWindow::hasWindowMouseFocus() || SharedCursor.X < 0 || SharedCursor.Y > XRender::TargetW || SharedCursor.Y < 0 || SharedCursor.Y > XRender::TargetH)
    //     HideCursor();

    if(LevelEditor || MagicHand)
    {
        int scroll_required = EditorControls.FastScroll ? 16 : 32;

        if(MagicHand)
        {
            scroll_buffer_x = 0;
            scroll_buffer_y = 0;
        }

        // if(ScrollDelay <= 0)
        {
            int to_scroll_x = scroll_buffer_x / scroll_required;
            int to_scroll_y = scroll_buffer_y / scroll_required;

            if(to_scroll_x)
            {
                vScreen[1].X -= 32 * to_scroll_x;
                EditorCursor.Location.X += 32 * to_scroll_x;
                scroll_buffer_x -= to_scroll_x * scroll_required;
            }

            if(to_scroll_y)
            {
                vScreen[1].Y -= 32 * to_scroll_y;
                EditorCursor.Location.Y += 32 * to_scroll_y;
                scroll_buffer_y -= to_scroll_y * scroll_required;
            }
        }
        // else
        //     ScrollDelay -= 1;
        SetCursor();

        // this is where objects are placed/grabbed/deleted

#ifdef THEXTECH_INTERPROC_SUPPORTED
        if(IntProc::isEnabled())
            UpdateInterprocess();
#endif

        UpdateInteract();

        if(MouseClick_Current && !editorScreen.active && EditorCursor.Y > 40)
        {
            CanPlace = true;
            if(EditorCursor.Mode == OptCursor_t::LVL_SELECT)
            {
                if(EditorCursor.InteractMode == OptCursor_t::LVL_PLAYERSTART) // Player start points
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);

                    EditorCursor.Mode = OptCursor_t::LVL_PLAYERSTART;
                    EditorCursor.SubMode = 3 + A;

                    EditorCursor.Location = PlayerStart[A];
                    PlayerStart[A] = PlayerStart_t();
                    MouseMove(EditorCursor.X, EditorCursor.Y);
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */
                }
                else if(EditorCursor.InteractMode == OptCursor_t::LVL_SECTION)
                {
                    MouseRelease = false;
                    InteractResizeSection(level[curSection]);
                    UpdateSectionOverlaps(curSection);
                }

                // event section resize
                if(EditorCursor.InteractMode == OptCursor_t::LVL_EVENTS)
                {
                    int A = EditorCursor.InteractIndex;
                    InteractResizeSection(Events[A].section[curSection].position);
                    MouseRelease = false;
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_NPCS) // NPCs
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);

                    EditorCursor.Mode = OptCursor_t::LVL_NPCS;
                    EditorCursor.NPC = NPC[A];
                    EditorCursor.NPC.Hidden = false;
                    EditorCursor.Layer = NPC[A].Layer;
                    EditorCursor.Location = NPC[A].Location;
                    EditorCursor.Location.X = NPC[A].Location.X;
                    EditorCursor.Location.Y = NPC[A].Location.Y;
                    SetCursor();
                    ResetNPC(EditorCursor.NPC.Type);

                    NPC[A].DefaultType = NPCID_NULL;
                    KillNPC(A, 9);

                    editorScreen.FocusNPC();
                    MouseRelease = false;
                    MouseCancel = true;

#ifdef THEXTECH_INTERPROC_SUPPORTED
                    if(IntProc::isEnabled()) // Report the taken block into the Editor
                    {
                        LevelNPC n;
                        n.id = EditorCursor.NPC.Type;
                        n.direct = EditorCursor.NPC.Direction;

                        n.generator = EditorCursor.NPC.Generator;
                        if(n.generator)
                        {
                            n.generator_direct = EditorCursor.NPC.GeneratorDirection;
                            n.generator_type = EditorCursor.NPC.GeneratorEffect;
                            n.generator_period = EditorCursor.NPC.GeneratorTimeMax;
                        }

                        if(n.id == 91 || n.id == 96 || n.id == 283 || n.id == 284)
                            n.contents = (long)EditorCursor.NPC.Special;

                        if(n.id == 288 || n.id == 289 || (n.id == 91 && int(EditorCursor.NPC.Special) == 288))
                            n.special_data = EditorCursor.NPC.Special2;

                        if(NPCIsAParaTroopa(n.id) || NPCTraits[n.id].IsFish || n.id == 260)
                            n.special_data = (long)EditorCursor.NPC.Special;

                        if(n.id == 86)
                            n.special_data = (long)EditorCursor.NPC.Variant;

                        n.msg = GetS(EditorCursor.NPC.Text);
                        n.friendly = EditorCursor.NPC.Inert;
                        n.nomove = EditorCursor.NPC.Stuck;
                        n.is_boss = EditorCursor.NPC.Legacy;

                        n.layer = GetL(EditorCursor.NPC.Layer);
                        n.event_activate = GetE(EditorCursor.NPC.TriggerActivate);
                        n.event_die = GetE(EditorCursor.NPC.TriggerDeath);
                        n.event_emptylayer = GetE(EditorCursor.NPC.TriggerLast);
                        n.event_talk = GetE(EditorCursor.NPC.TriggerTalk);
                        n.attach_layer = GetL(EditorCursor.NPC.AttLayer);
                        IntProc::sendTakenNPC(n);
                    }
#endif
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_BLOCKS && EditorCursor.InteractFlags > 1) // resizing block
                {
                    MouseRelease = false;

                    Location_t& iLoc = Block[EditorCursor.InteractIndex].Location;
                    InteractResize(iLoc, 64, 32);

                    syncLayersTrees_Block(EditorCursor.InteractIndex);
                }
                else if(EditorCursor.InteractMode == OptCursor_t::LVL_BLOCKS) // Blocks
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);

                    EditorCursor.Mode = OptCursor_t::LVL_BLOCKS;
                    EditorCursor.Block = Block[A];
                    EditorCursor.Layer = Block[A].Layer;
                    EditorCursor.Location.X = Block[A].Location.X;
                    EditorCursor.Location.Y = Block[A].Location.Y;
                    EditorCursor.Location.Width = Block[A].Location.Width;
                    EditorCursor.Location.Height = Block[A].Location.Height;
                    SetCursor();

                    Location_t loc = Block[A].Location;
                    int type = Block[A].Type;
                    KillBlock(A, false);

                    MagicBlock::MagicBlock(type, loc);

                    editorScreen.FocusBlock();
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */

#ifdef THEXTECH_INTERPROC_SUPPORTED
                    if(IntProc::isEnabled()) // Report the taken block into the Editor
                    {
                        LevelBlock block;
                        block.id = EditorCursor.Block.Type;
                        block.w = EditorCursor.Location.Width;
                        block.h = EditorCursor.Location.Height;
                        block.invisible = EditorCursor.Block.Invis;
                        block.slippery = EditorCursor.Block.Slippy;
                        block.layer = GetL(EditorCursor.Block.Layer);
                        if(EditorCursor.Block.Special >= 1000)
                            block.npc_id = EditorCursor.Block.Special - 1000;
                        else if(EditorCursor.Block.Special <= 0)
                            block.npc_id = 0;
                        else if(EditorCursor.Block.Special < 1000)
                            block.npc_id = -EditorCursor.Block.Special;
                        block.event_hit = GetE(EditorCursor.Block.TriggerHit);
                        block.event_emptylayer = GetE(EditorCursor.Block.TriggerLast);
                        block.event_destroy = GetE(EditorCursor.Block.TriggerDeath);
                        IntProc::sendTakenBlock(block);
                    }
#endif // THEXTECH_INTERPROC_SUPPORTED
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_WARPS) // Warps
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);
                    MouseRelease = false;
                    EditorCursor.Mode = OptCursor_t::LVL_WARPS;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */

                    if(EditorCursor.InteractFlags == 0)
                    {
                        Warp[A].PlacedEnt = false;
                        EditorCursor.SubMode = 1;

                        if(Warp[A].LevelEnt || EditorCursor.Warp.MapWarp || EditorCursor.Warp.level != STRINGINDEX_NONE)
                            Warp[A].PlacedExit = false;

                    }
                    else
                    {
                        Warp[A].PlacedExit = false;
                        EditorCursor.SubMode = 2;

                        // TODO: additional testing of these situations
                        if(Warp[A].LevelEnt || EditorCursor.Warp.MapWarp || EditorCursor.Warp.level != STRINGINDEX_NONE)
                            Warp[A].PlacedEnt = false;
                    }

                    EditorCursor.Warp = Warp[A];
                    if(!Warp[A].PlacedEnt && !Warp[A].PlacedExit)
                        KillWarp(A);
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_BGOS) // BGOs
                {
                    int A = EditorCursor.InteractIndex;

                    PlaySound(SFX_Grab);

                    EditorCursor.Mode = OptCursor_t::LVL_BGOS;
                    EditorCursor.Background = Background[A];
                    EditorCursor.Layer = Background[A].Layer;
                    EditorCursor.Location.X = Background[A].Location.X;
                    EditorCursor.Location.Y = Background[A].Location.Y;
                    SetCursor();

                    Location_t loc = static_cast<Location_t>(Background[A].Location);
                    int type = Background[A].Type;

                    Background[A] = Background[numBackground];
                    numBackground--;

                    editorScreen.FocusBGO();
                    if(MagicHand)
                    {
                        qSortBackgrounds(1, numBackground);
                        UpdateBackgrounds();
                        syncLayers_AllBGOs();
                    }
                    else
                    {
                        syncLayers_BGO(A);
                        syncLayers_BGO(numBackground+1);
                    }

                    MagicBlock::MagicBackground(type, loc);

                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */

#ifdef THEXTECH_INTERPROC_SUPPORTED
                    if(IntProc::isEnabled()) // Report the taken block into the Editor
                    {
                        LevelBGO b;
                        b.id = EditorCursor.Background.Type;
                        b.layer = GetL(EditorCursor.Background.Layer);
                        b.z_mode = EditorCursor.Background.GetCustomLayer();
                        b.z_offset = EditorCursor.Background.GetCustomOffset();
                        IntProc::sendTakenBGO(b);
                    }
#endif // THEXTECH_INTERPROC_SUPPORTED
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_WATER && EditorCursor.InteractFlags > 1) // resizing water
                {
                    MouseRelease = false;

                    SpeedlessLocation_t& iLoc = Water[EditorCursor.InteractIndex].Location;
                    InteractResize(iLoc, 32, 32);

                    syncLayers_Water(EditorCursor.InteractIndex);
                }
                else if(EditorCursor.InteractMode == OptCursor_t::LVL_WATER) // water
                {
                    int A = EditorCursor.InteractIndex;

                    PlaySound(SFX_Grab);
                    EditorCursor.Mode = OptCursor_t::LVL_WATER;
                    EditorCursor.Location = static_cast<Location_t>(Water[A].Location);
                    EditorCursor.Layer = Water[A].Layer;
                    EditorCursor.Water = Water[A];
                    Water[A] = Water[numWater];
                    numWater--;
                    syncLayers_Water(A);
                    syncLayers_Water(numWater+1);
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_AREA && EditorCursor.InteractFlags > 1) // resizing world map area
                {
                    MouseRelease = false;

                    IntegerLocation_t& iLoc = WorldArea[EditorCursor.InteractIndex].Location;
                    InteractResize(iLoc, 32, 32);
                }
                else if(EditorCursor.InteractMode == OptCursor_t::WLD_AREA) // World map areas
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);
                    EditorCursor.Mode = OptCursor_t::WLD_AREA;
                    EditorCursor.Location = static_cast<Location_t>(WorldArea[A].Location);
                    SetCursor();
                    EditorCursor.WorldArea = WorldArea[A];
                    if(A != numWorldAreas)
                        WorldArea[A] = WorldArea[numWorldAreas];
                    numWorldAreas--;
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_MUSIC) // World map music
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);
                    EditorCursor.Mode = OptCursor_t::WLD_MUSIC;
                    EditorCursor.Location = static_cast<Location_t>(WorldMusic[A].Location);
                    SetCursor();
                    EditorCursor.WorldMusic = WorldMusic[A];
                    if(A != numWorldMusic)
                    {
                        WorldMusic[A] = WorldMusic[numWorldMusic];
                        treeWorldMusicUpdate(&WorldMusic[A]);
                    }
                    treeWorldMusicRemove(&WorldMusic[numWorldMusic]);
                    numWorldMusic--;
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_PATHS) // World paths
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);
                    EditorCursor.Mode = OptCursor_t::WLD_PATHS;
                    EditorCursor.Location = static_cast<Location_t>(WorldPath[A].Location);
                    EditorCursor.WorldPath = WorldPath[A];
                    SetCursor();
                    if(A != numWorldPaths)
                    {
                        WorldPath[A] = WorldPath[numWorldPaths];
                        treeWorldPathUpdate(&WorldPath[A]);
                    }
                    treeWorldPathRemove(&WorldPath[numWorldPaths]);
                    numWorldPaths--;
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_SCENES) // World scenes
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);
                    EditorCursor.Mode = OptCursor_t::WLD_SCENES;
                    EditorCursor.Location = static_cast<Location_t>(Scene[A].Location);
                    EditorCursor.Scene = Scene[A];
                    SetCursor();
                    MouseMove(EditorCursor.X, EditorCursor.Y);
                    // this maintains the order of the scenes
                    // but makes for a hellish quadtree update
                    for(int B = A; B < numScenes; B++)
                    {
                        Scene[B] = Scene[B + 1];
                        treeWorldSceneUpdate(&Scene[B]);
                    }
                    treeWorldSceneRemove(&Scene[numScenes]);
                    numScenes--;
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_LEVELS) // World map level points
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);
                    EditorCursor.Mode = OptCursor_t::WLD_LEVELS;
                    EditorCursor.Location = static_cast<Location_t>(WorldLevel[A].Location);
                    EditorCursor.WorldLevel = WorldLevel[A];
                    SetCursor();
                    if(A != numWorldLevels)
                    {
                        WorldLevel[A] = WorldLevel[numWorldLevels];
                        treeWorldLevelUpdate(&WorldLevel[A]);
                    }
                    treeWorldLevelRemove(&WorldLevel[numWorldLevels]);
                    numWorldLevels--;
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_TILES) // World map tiles
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Grab);
                    EditorCursor.Mode = OptCursor_t::WLD_TILES;
                    EditorCursor.Location = static_cast<Location_t>(Tile[A].Location);
                    EditorCursor.Tile = Tile[A];
                    SetCursor();

                    Location_t loc = static_cast<Location_t>(Tile[A].Location);
                    int type = Tile[A].Type;

                    if(A != numTiles)
                    {
                        Tile[A] = Tile[numTiles];
                        treeWorldTileUpdate(&Tile[A]);
                    }
                    treeWorldTileRemove(&Tile[numTiles]);

                    numTiles--;

                    MagicBlock::MagicTile(type, loc);

                    editorScreen.FocusTile();
                    MouseRelease = false;
                    MouseCancel = true; /* Simulate "Focus out" inside of SMBX Editor */
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::LVL_ERASER && (SharedCursor.Move || MouseRelease)) // Eraser
            {
                if(EditorCursor.InteractMode == OptCursor_t::LVL_NPCS)
                {
                    int A = EditorCursor.InteractIndex;

                    if(iRand(2) == 0)
                        NPC[A].Location.SpeedX = double(Physics.NPCShellSpeed / 2);
                    else
                        NPC[A].Location.SpeedX = -double(Physics.NPCShellSpeed / 2);

                    NPC[A].DefaultType = NPCID_NULL;
                    if(NPC[A]->IsABonus || NPC[A]->IsACoin)
                        KillNPC(A, 4); // Kill the bonus/coin
                    else
                        KillNPC(A, 2); // Kill the NPC

                    MouseRelease = false;

                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::LVL_NPCS;
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_BLOCKS)
                {
                    int A = EditorCursor.InteractIndex;

                    Location_t loc = Block[A].Location;
                    int type = Block[A].Type;
                    KillBlock(A);

                    MagicBlock::MagicBlock(type, loc);

                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::LVL_BLOCKS;
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_WARPS)
                {
                    int A = EditorCursor.InteractIndex;
                    KillWarp(A);
                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::LVL_WARPS;
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_BGOS)
                {
                    int A = EditorCursor.InteractIndex;

                    Location_t loc = static_cast<Location_t>(Background[A].Location);
                    int type = Background[A].Type;

                    auto &b = Background[A];
                    b.Location.X += b.Location.Width / 2.0 - EffectWidth[10] / 2;
                    b.Location.Y += b.Location.Height / 2.0 - EffectHeight[10] / 2;
                    NewEffect(EFFID_SMOKE_S3, static_cast<Location_t>(b.Location));
                    PlaySound(SFX_Smash);

                    Background[A] = Background[numBackground];
                    numBackground--;

                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::LVL_BGOS;

                    if(MagicHand)
                    {
                        qSortBackgrounds(1, numBackground);
                        UpdateBackgrounds();
                        syncLayers_AllBGOs();
                        syncLayers_BGO(numBackground + 1);
                    }
                    else
                    {
                        syncLayers_BGO(A);
                        syncLayers_BGO(numBackground + 1);
                    }

                    MagicBlock::MagicBackground(type, loc);
                }

                if(EditorCursor.InteractMode == OptCursor_t::LVL_WATER)
                {
                    int A = EditorCursor.InteractIndex;
                    PlaySound(SFX_Smash);
                    Water[A] = Water[numWater];
                    numWater--;
                    syncLayers_Water(A);
                    syncLayers_Water(numWater + 1);
                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::LVL_WATER;
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_AREA)
                {
                    int A = EditorCursor.InteractIndex;
                    for(int X = 16; X < WorldArea[A].Location.Width; X += 32)
                    {
                        for(int Y = 16; Y < WorldArea[A].Location.Height; Y += 32)
                        {
                            tempLocation = static_cast<Location_t>(WorldArea[A].Location);
                            tempLocation.X += X - EffectWidth[10] / 2;
                            tempLocation.Y += Y - EffectHeight[10] / 2;
                            NewEffect(EFFID_SMOKE_S3, tempLocation);
                        }
                    }

                    PlaySound(SFX_ShellHit);
                    if(A != numWorldAreas)
                        WorldArea[A] = WorldArea[numWorldAreas];
                    numWorldAreas--;
                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::WLD_AREA;
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_MUSIC)
                {
                    int A = EditorCursor.InteractIndex;
                    tempLocation = static_cast<Location_t>(WorldMusic[A].Location);
                    tempLocation.X += tempLocation.Width / 2.0 - EffectWidth[10] / 2;
                    tempLocation.Y += tempLocation.Height / 2.0 - EffectHeight[10] / 2;
                    NewEffect(EFFID_SMOKE_S3, tempLocation);
                    PlaySound(SFX_ShellHit);
                    if(A != numWorldMusic)
                    {
                        WorldMusic[A] = WorldMusic[numWorldMusic];
                        treeWorldMusicUpdate(&WorldMusic[A]);
                    }
                    treeWorldMusicRemove(&WorldMusic[numWorldMusic]);
                    numWorldMusic--;
                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::WLD_MUSIC;
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_PATHS)
                {
                    int A = EditorCursor.InteractIndex;
                    tempLocation = static_cast<Location_t>(WorldPath[A].Location);
                    tempLocation.X += tempLocation.Width / 2.0 - EffectWidth[10] / 2;
                    tempLocation.Y += tempLocation.Height / 2.0 - EffectHeight[10] / 2;
                    NewEffect(EFFID_SMOKE_S3, tempLocation);
                    PlaySound(SFX_ShellHit);
                    if(A != numWorldPaths)
                    {
                        WorldPath[A] = WorldPath[numWorldPaths];
                        treeWorldPathUpdate(&WorldPath[A]);
                    }
                    treeWorldPathRemove(&WorldPath[numWorldPaths]);
                    numWorldPaths--;
                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::WLD_PATHS;
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_SCENES)
                {
                    int A = EditorCursor.InteractIndex;
                    tempLocation = static_cast<Location_t>(Scene[A].Location);
                    tempLocation.X += tempLocation.Width / 2.0 - EffectWidth[10] / 2;
                    tempLocation.Y += tempLocation.Height / 2.0 - EffectHeight[10] / 2;
                    NewEffect(EFFID_SMOKE_S3, tempLocation);
                    PlaySound(SFX_ShellHit);
                    for(int B = A; B < numScenes; B++)
                    {
                        Scene[B] = Scene[B + 1];
                        treeWorldSceneUpdate(&Scene[B]);
                    }
                    treeWorldSceneRemove(&Scene[numScenes]);
                    numScenes--;
                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::WLD_SCENES;
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_LEVELS)
                {
                    int A = EditorCursor.InteractIndex;
                    tempLocation = static_cast<Location_t>(WorldLevel[A].Location);
                    tempLocation.X += tempLocation.Width / 2.0 - EffectWidth[10] / 2;
                    tempLocation.Y += tempLocation.Height / 2.0 - EffectHeight[10] / 2;
                    NewEffect(EFFID_SMOKE_S3, tempLocation);
                    PlaySound(SFX_ShellHit);
                    if(A != numWorldLevels)
                    {
                        WorldLevel[A] = WorldLevel[numWorldLevels];
                        treeWorldLevelUpdate(&WorldLevel[A]);
                    }
                    treeWorldLevelRemove(&WorldLevel[numWorldLevels]);
                    numWorldLevels--;
                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::WLD_LEVELS;
                }

                if(EditorCursor.InteractMode == OptCursor_t::WLD_TILES)
                {
                    int A = EditorCursor.InteractIndex;
                    tempLocation = static_cast<Location_t>(Tile[A].Location);
                    tempLocation.X += tempLocation.Width / 2.0 - EffectWidth[10] / 2;
                    tempLocation.Y += tempLocation.Height / 2.0 - EffectHeight[10] / 2;
                    NewEffect(EFFID_SMOKE_S3, tempLocation);
                    PlaySound(SFX_ShellHit);

                    Location_t loc = static_cast<Location_t>(Tile[A].Location);
                    int type = Tile[A].Type;

                    if(A != numTiles)
                    {
                        Tile[A] = Tile[numTiles];
                        treeWorldTileUpdate(&Tile[A]);
                    }
                    treeWorldTileRemove(&Tile[numTiles]);

                    numTiles--;

                    MagicBlock::MagicTile(type, loc);

                    MouseRelease = false;
                    if(EditorCursor.SubMode == 0)
                        EditorCursor.SubMode = OptCursor_t::WLD_TILES;
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::LVL_WATER) // Water
            {
                if(MouseRelease)
                {
                    MouseRelease = false;
                    CanPlace = true;
                    for(A = 1; A <= numWater; A++)
                    {
                        if(ffEqual(Water[A].Location.X, EditorCursor.Location.X) &&
                           ffEqual(Water[A].Location.Y, EditorCursor.Location.Y) &&
                           ffEqual(Water[A].Location.Height, EditorCursor.Location.Height) &&
                           ffEqual(Water[A].Location.Width, EditorCursor.Location.Width))
                        {
                            CanPlace = false;
                            break;
                        }
                    }

                    if(CanPlace)
                    {
                        numWater++;
                        Water[numWater] = EditorCursor.Water;
                        syncLayers_Water(numWater);
                    }
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::LVL_BLOCKS) // Blocks
            {
                for(A = numBlock; A >= 1; A--)
                {
                    if(!MouseRelease || (!BlockIsSizable[Block[A].Type] && !BlockIsSizable[EditorCursor.Block.Type]))
                    {
                        // ignore sizable background blocks in Magic Block mode
                        if(MagicBlock::enabled && BlockIsSizable[Block[A].Type] && !BlockIsSizable[EditorCursor.Block.Type])
                            continue;

                        if(CursorCollision(EditorCursor.Location, Block[A].Location) && !Block[A].Hidden)
                        {
                            if(!(MagicBlock::enabled && MagicBlock::replace_existing) || (!MouseRelease && CheckCollision(Block[A].Location, last_EC_loc)))
                            {
                                CanPlace = false;
                                break;
                            }
                            else
                            {
                                Location_t loc = Block[A].Location;
                                int type = Block[A].Type;
                                KillBlock(A, false);
                                MagicBlock::MagicBlock(type, loc);
                            }
                        }
                    }
                    else
                    {
                        if(Block[A].Type == EditorCursor.Block.Type)
                        {
                            if(ffEqual(EditorCursor.Location.X, Block[A].Location.X) &&
                               ffEqual(EditorCursor.Location.Y, Block[A].Location.Y))
                            {
                                if(MouseRelease)
                                    pLogDebug("Sizable block was rejected at block at EC Loc (%f, %f), other block loc (%f, %f)", EditorCursor.Location.X, EditorCursor.Location.Y, Block[A].Location.X, Block[A].Location.Y);

                                CanPlace = false;
                                break;
                            }
                        }
                    }
                }

                if(CanPlace)
                {
                    last_EC_loc = EditorCursor.Location;
                    last_EC_loc.X += 1;
                    last_EC_loc.Y += 1;
                    last_EC_loc.Width -= 2;
                    last_EC_loc.Height -= 2;
                }

                if(!BlockIsSizable[EditorCursor.Block.Type] && EditorCursor.Block.Type != 370 && CanPlace)
                {
                    for(A = 1; A <= numNPCs; A++)
                    {
                        if(NPC[A].Type != NPCID_ITEM_BURIED && NPC[A].Type != NPCID_FIRE_DISK && NPC[A].Type != NPCID_FIRE_CHAIN)
                        {
                            if(CursorCollision(EditorCursor.Location, NPC[A].Location) && !NPC[A].Hidden && NPC[A].Active)
                            {
                                CanPlace = false;
                                break;
                            }
                        }
                    }
                    for(A = 1; A <= 2; A++)
                    {
                        if(CursorCollision(EditorCursor.Location, PlayerStart[A]) && !MagicHand)
                        {
                            CanPlace = false;
                            break;
                        }
                    }
                }

                if(CanPlace) // Nothing is in the way
                {
//                    if(frmBlocks::chkFill.Value == 1)
//                    {
//                        BlockFill(EditorCursor.Block.Location); // and nuke an application with memory overflow error >:D
//                        if(MagicHand == true)
//                        {
//                            for(A = -FLBlocks; A <= FLBlocks; A++)
//                            {
//                                FirstBlock[A] = 1;
//                                LastBlock[A] = numBlock;
//                            }
//                            BlocksSorted = false;
//                        }
//                        FindSBlocks();
//                    }
//                    else
                    {
                        if(numBlock < maxBlocks) // Not out of blocks
                        {
                            MouseRelease = false;
                            numBlock++;
                            Block[numBlock] = EditorCursor.Block;
                            Block[numBlock].DefaultType = Block[numBlock].Type;
                            Block[numBlock].DefaultSpecial = Block[numBlock].Special;
                            syncLayersTrees_Block(numBlock);

                            MagicBlock::MagicBlock(numBlock);
#if 0
                            if(MagicHand)
                            {
                                for(A = -FLBlocks; A <= FLBlocks; A++)
                                {
                                    FirstBlock[A] = 1;
                                    LastBlock[A] = numBlock;
                                }
                                BlocksSorted = false;
                            }
#endif
                        }
                        // FindSBlocks();
//                        if(nPlay.Online == true)
//                            Netplay::sendData Netplay::AddBlock(numBlock);
                    }

                }
            }
            else if(EditorCursor.Mode == OptCursor_t::LVL_PLAYERSTART && !MagicHand) // player start points
            {
                if(EditorCursor.SubMode >= 4)
                {
                    // printf("Trying to place player at %f, %f...\n", EditorCursor.Location.X, EditorCursor.Location.Y);
                    int B = EditorCursor.SubMode - 3;

                    for(A = 1; A <= 2; A++)
                    {
                        if(CursorCollision(EditorCursor.Location, PlayerStart[A]) && A != B)
                            CanPlace = false;
                    }
                    for(A = 1; A <= numBlock; A++)
                    {
                        if(CursorCollision(EditorCursor.Location, Block[A].Location) && !Block[A].Hidden && !Block[A].Invis && !BlockIsSizable[Block[A].Type] && !BlockNoClipping[Block[A].Type] && BlockOnlyHitspot1[Block[A].Type] == false && BlockSlope[Block[A].Type] == 0 && BlockSlope2[Block[A].Type] == 0)
                        {
                            CanPlace = false;
                            break;
                        }
                    }
                    if(CanPlace)
                    {
                        if(EditorCursor.SubMode == 4)
                            PlayerStart[1] = EditorCursor.Location;
                        else
                            PlayerStart[2] = EditorCursor.Location;
                    }
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::LVL_BGOS) // Backgrounds
            {
                for(A = 1; A <= numBackground; A++)
                {
                    const int ctype = EditorCursor.Background.Type;
                    const int btype = Background[A].Type;
                    bool same_type = (ctype == btype);

                    if(MagicBlock::enabled && ctype > 0 && ctype <= maxBackgroundType && btype > 0 && btype <= maxBackgroundType
                        && EditorCustom::bgo_family_by_type[ctype - 1] != EditorCustom::FAMILY_NONE
                        && EditorCustom::bgo_family_by_type[ctype - 1] == EditorCustom::bgo_family_by_type[btype - 1])
                    {
                        same_type = true;
                    }

                    if(same_type)
                    {
                        if(CursorCollision(EditorCursor.Location, Background[A].Location) && !Background[A].Hidden)
                        {
                            CanPlace = false;
                            break;
                        }
                    }
                }

                if(CanPlace) // Nothing is in the way
                {
                    if(numBackground < maxBackgrounds) // Not out of backgrounds
                    {
                        numBackground++;
                        Background[numBackground] = EditorCursor.Background;
                        syncLayers_BGO(numBackground);

                        MagicBlock::MagicBackground(numBackground);

                        if(MagicHand)
                        {
                            qSortBackgrounds(1, numBackground);
                            UpdateBackgrounds();
                            // ugh
                            syncLayers_AllBGOs();
                        }
                    }
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::LVL_NPCS) // NPCs
            {
                if(EditorCursor.NPC.Type != 91 && EditorCursor.NPC.Type != 259 && EditorCursor.NPC.Type != 260)
                {
                    for(A = 1; A <= numBlock; A++)
                    {
                        if(!BlockIsSizable[Block[A].Type])
                        {
                            if(CursorCollision(EditorCursor.Location, Block[A].Location) &&
                               !Block[A].Hidden && BlockSlope[Block[A].Type] == 0 && BlockSlope2[Block[A].Type] == 0)
                            {
                                CanPlace = false;
                                break;
                            }
                        }
                    }
                }

                for(A = 1; A <= numNPCs; A++)
                {
                    if(CursorCollision(EditorCursor.Location, NPC[A].Location) && !NPC[A].Hidden && NPC[A].Active && (NPC[A].Type != NPCID_LIFT_SAND || EditorCursor.NPC.Type == 159))
                    {
                        if(!NPC[A].Generator || NPC[A].Type == EditorCursor.NPC.Type)
                        {
                            if((EditorCursor.NPC.Type != 208 && NPC[A].Type != NPCID_BOSS_CASE) || (EditorCursor.NPC.Type == 208 && NPC[A].Type == NPCID_BOSS_CASE))
                            {
                                if(!NPC[A]->IsAVine)
                                {
                                    CanPlace = false;
                                    break;
                                }
                            }
                        }
                    }
                }

                for(A = 1; A <= 2; A++)
                {
                    if(CursorCollision(EditorCursor.Location, PlayerStart[A]) && !MagicHand)
                        CanPlace = false;
                }

                if(!MouseRelease)
                    CanPlace = false;

                if(CanPlace) // Nothing is in the way
                {
                    if(numNPCs < maxNPCs - 20) // Not out of npcs
                    {
                        MouseRelease = false;
                        numNPCs++;
//                        if(frmNPCs::Bubble.Caption == "Yes" && frmNPCs::optNPCDirection(1).Value == true)
//                        {
//                            EditorCursor.NPC.Direction = 0;
//                            EditorCursor.NPC.DefaultDirection = 0;
//                        }

                        NPC[numNPCs] = EditorCursor.NPC;
                        if(NPC[numNPCs].Text != STRINGINDEX_NONE)
                        {
                            NPC[numNPCs].Text = STRINGINDEX_NONE;
                            SetS(NPC[numNPCs].Text, GetS(EditorCursor.NPC.Text));
                        }
//                        Netplay::sendData Netplay::AddNPC(numNPCs);
                        if(!MagicHand)
                        {
                            // ugh
                            NPCSort();
                            syncLayers_AllNPCs();
                        }

                        if(MagicHand)
                        {
                            auto &n = NPC[numNPCs];
                            n.FrameCount = 0;
                            n.Active = true;
                            n.TimeLeft = 10;
                            n.DefaultDirection = n.Direction;
                            n.DefaultLocationX = n.Location.X;
                            n.DefaultLocationY = n.Location.Y;
                            n.DefaultSpecial = n.Special;
                            CheckSectionNPC(numNPCs);
                        }
                        syncLayers_NPC(numNPCs);
                    }
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS) // Warps
            {
                // find an incomplete warp slot
                int numWarpsMax = numWarps + 1;
                for(A = 1; A <= numWarpsMax; A++)
                {
                    if(!Warp[A].PlacedEnt || !Warp[A].PlacedExit)
                        break;
                }

                if(A > numWarps)
                    numWarps = A;

                if(EditorCursor.SubMode == 1 || EditorCursor.Warp.level != STRINGINDEX_NONE || EditorCursor.Warp.LevelEnt || EditorCursor.Warp.MapWarp)
                {
                    EditorCursor.Warp.Entrance = static_cast<SpeedlessLocation_t>(EditorCursor.Location);
                    MouseCancel = true;
                    MouseRelease = false;
                    EditorCursor.Warp.PlacedEnt = true;
                }
                if(EditorCursor.SubMode == 2 || EditorCursor.Warp.level != STRINGINDEX_NONE || EditorCursor.Warp.LevelEnt || EditorCursor.Warp.MapWarp)
                {
                    EditorCursor.Warp.Exit = static_cast<SpeedlessLocation_t>(EditorCursor.Location);
                    MouseCancel = true;
                    MouseRelease = false;
                    EditorCursor.Warp.PlacedExit = true;
                }

                Warp[A] = EditorCursor.Warp;
                Warp[A].Layer = EditorCursor.Layer;

                // de-duplicate strings
                if(Warp[A].level != STRINGINDEX_NONE)
                {
                    Warp[A].level = STRINGINDEX_NONE;
                    SetS(Warp[A].level, GetS(EditorCursor.Warp.level));
                }
                if(Warp[A].StarsMsg != STRINGINDEX_NONE)
                {
                    Warp[A].StarsMsg = STRINGINDEX_NONE;
                    SetS(Warp[A].StarsMsg, GetS(EditorCursor.Warp.StarsMsg));
                }

                if(Warp[A].PlacedEnt && Warp[A].PlacedExit)
                {
                    EditorCursor.Warp.PlacedEnt = false;
                    EditorCursor.Warp.PlacedExit = false;
                    EditorCursor.SubMode = 1;
                }
                else if(Warp[A].PlacedEnt)
                    EditorCursor.SubMode = 2;
                else
                    EditorCursor.SubMode = 1;

                syncLayers_Warp(A);
//                if(nPlay.Online == true)
//                    Netplay::sendData Netplay::AddWarp[A];
            }
            else if(EditorCursor.Mode == OptCursor_t::WLD_TILES) // Tiles
            {
                for(A = numTiles; A >= 1; A--)
                {
                    if(CursorCollision(EditorCursor.Location, Tile[A].Location))
                    {
                        if(!(MagicBlock::enabled && MagicBlock::replace_existing) || (!MouseRelease && CheckCollision(Tile[A].Location, last_EC_loc)))
                        {
                            CanPlace = false;
                            break;
                        }
                        else
                        {
                            Location_t loc = static_cast<Location_t>(Tile[A].Location);
                            int type = Tile[A].Type;

                            if(A != numTiles)
                            {
                                Tile[A] = Tile[numTiles];
                                treeWorldTileUpdate(&Tile[A]);
                            }
                            treeWorldTileRemove(&Tile[numTiles]);
                            numTiles--;

                            MagicBlock::MagicTile(type, loc);
                        }
                    }
                }

                MouseRelease = false;

                if(CanPlace) // Nothing is in the way
                {
                    last_EC_loc = EditorCursor.Location;
                    last_EC_loc.X += 1;
                    last_EC_loc.Y += 1;
                    last_EC_loc.Width -= 2;
                    last_EC_loc.Height -= 2;

                    if(numTiles < maxTiles) // Not out of blocks
                    {
                        numTiles++;
                        Tile[numTiles] = EditorCursor.Tile;
                        treeWorldTileAdd(&Tile[numTiles]);

                        MagicBlock::MagicTile(numTiles);
                    }
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::WLD_SCENES) // Scenery
            {
                for(A = 1; A <= numScenes; A++)
                {
                    if(CursorCollision(EditorCursor.Location, Scene[A].Location))
                    {
                        if(EditorCursor.Scene.Type == Scene[A].Type)
                        {
                            if(ffEqual(EditorCursor.Scene.Location.X, Scene[A].Location.X) &&
                               ffEqual(EditorCursor.Scene.Location.Y, Scene[A].Location.Y))
                            {
                                CanPlace = false;
                                break;
                            }
                        }
                    }
                }

                for(A = 1; A <= numWorldLevels; A++)
                {
                    if(CursorCollision(EditorCursor.Location, WorldLevel[A].Location))
                    {
                        CanPlace = false;
                        break;
                    }
                }

                if(CanPlace)
                {
                    if(numScenes < maxScenes)
                    {
                        numScenes++;
                        Scene[numScenes] = EditorCursor.Scene;
                        treeWorldSceneAdd(&Scene[numScenes]);
                    }
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::WLD_LEVELS) // Level
            {
                // can do something fancy with the quadtrees here

                for(A = 1; A <= numWorldPaths; A++)
                {
                    if(CursorCollision(EditorCursor.Location, WorldPath[A].Location))
                    {
                        CanPlace = false;
                        break;
                    }
                }

                for(A = 1; A <= numScenes; A++)
                {
                    if(CursorCollision(EditorCursor.Location, Scene[A].Location))
                    {
                        CanPlace = false;
                        break;
                    }
                }

                for(A = 1; A <= numWorldLevels; A++)
                {
                    if(CursorCollision(EditorCursor.Location, WorldLevel[A].Location))
                    {
                        CanPlace = false;
                        qLevel = A;
                        UNUSED(qLevel);
                        break;
                    }
                }

                if(CanPlace)
                {
                    if(numWorldLevels < maxWorldLevels)
                    {
                        numWorldLevels++;
                        WorldLevel[numWorldLevels] = EditorCursor.WorldLevel;
                        treeWorldLevelAdd(&WorldLevel[numWorldLevels]);
                    }
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::WLD_PATHS) // Paths
            {
                // can do something fancy with the quadtrees here

                for(A = 1; A <= numWorldPaths; A++)
                {
                    if(CursorCollision(EditorCursor.Location, WorldPath[A].Location))
                    {
                        CanPlace = false;
                        break;
                    }
                }

                for(A = 1; A <= numWorldLevels; A++)
                {
                    if(CursorCollision(EditorCursor.Location, WorldLevel[A].Location))
                    {
                        CanPlace = false;
                        break;
                    }
                }

                if(CanPlace)
                {
                    if(numWorldPaths < maxWorldPaths)
                    {
                        numWorldPaths++;
                        WorldPath[numWorldPaths] = EditorCursor.WorldPath;
                        treeWorldPathAdd(&WorldPath[numWorldPaths]);
                    }
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::WLD_MUSIC) // Music
            {
                for(A = 1; A <= numWorldMusic; A++)
                {
                    if(CursorCollision(EditorCursor.Location, WorldMusic[A].Location))
                    {
                        CanPlace = false;
                        break;
                    }
                }

                if(CanPlace)
                {
                    EditorCursor.WorldMusic.Location = static_cast<TinyLocation_t>(EditorCursor.Location);
                    numWorldMusic++;
                    WorldMusic[numWorldMusic] = EditorCursor.WorldMusic;
                    // de-duplicate music file
                    if(EditorCursor.WorldMusic.MusicFile != STRINGINDEX_NONE)
                    {
                        WorldMusic[numWorldMusic].MusicFile = STRINGINDEX_NONE;
                        SetS(WorldMusic[numWorldMusic].MusicFile, GetS(EditorCursor.WorldMusic.MusicFile));
                    }
                    treeWorldMusicAdd(&WorldMusic[numWorldMusic]);
                }
            }
            else if(EditorCursor.Mode == OptCursor_t::WLD_AREA && MouseRelease) // Areas
            {
                for(A = 1; A <= numWorldAreas; A++)
                {
                    if(CursorCollision(EditorCursor.Location,
                        newLoc(WorldArea[A].Location.X, WorldArea[A].Location.Y, 32, 32)))
                    {
                        CanPlace = false;
                        break;
                    }
                }

                if(CanPlace)
                {
                    EditorCursor.WorldArea.Location.X = EditorCursor.Location.X;
                    EditorCursor.WorldArea.Location.Y = EditorCursor.Location.Y;
                    numWorldAreas++;
                    WorldArea[numWorldAreas] = EditorCursor.WorldArea;
                    MouseRelease = false;
                }
            }
        }

        if(!MagicHand)
        {
            for(A = 1; A <= numNPCs; A++)
            {
                // .Frame = EditorNPCFrame(.Type, .Direction)
                NPCFrames(A);
            }
        }
    }

#ifdef THEXTECH_INTERPROC_SUPPORTED
    if(!MagicHand || !IntProc::isEnabled())
#endif
    {
        editorScreen.UpdateEditorScreen(EditorScreen::CallMode::Logic);
    }
}

#ifdef THEXTECH_INTERPROC_SUPPORTED
void UpdateInterprocess()
{
    if(!IntProc::hasCommand())
        return;

    IntProc::cmdLock();

    //Recive external commands!
    switch(IntProc::commandType())
    {
    case IntProc::MsgBox:
    {
        MessageText = IntProc::getCMD();
        PauseGame(PauseCode::Message);
        break;
    }

    case IntProc::Cheat:
    {
        cheats_setBuffer(IntProc::getCMD());
        break;
    }

    case IntProc::SetLayer:
    {
        EditorCursor.Layer = FindLayer(IntProc::getCMD());

        EditorCursor.Block.Layer = EditorCursor.Layer;
        EditorCursor.Background.Layer = EditorCursor.Layer;
        EditorCursor.NPC.Layer = EditorCursor.Layer;
        break;
    }

    case IntProc::SetNumStars:
    {
        auto s = IntProc::getCMD();
        int starsNew = SDL_atoi(s.c_str());
        if(numStars < starsNew) // Can't decrease stars number
        {
            numStars = starsNew;
            CheckAfterStarTake(true);
        }
        break;
    }

    case IntProc::PlaceItem:
    {
        std::string raw = IntProc::getCMD();
        pLogDebug(raw.c_str());
        LevelData got;
        PGE_FileFormats_misc::RawTextInput raw_file(&raw);
        FileFormats::ReadExtendedLvlFile(raw_file, got);

        if(!got.meta.ReadFileValid)
        {
            pLogDebug(got.meta.ERROR_info.c_str());
            break;
        }

        if(raw.compare(0, 11, "BLOCK_PLACE") == 0)
        {
            if(got.blocks.empty())
                break;

            const LevelBlock &b = got.blocks[0];

            if(EditorCursor.Mode != OptCursor_t::LVL_BLOCKS ||
               EditorCursor.Block.Type != int(b.id))
                PlaySound(SFX_Grab);

            EditorCursor.Layer = FindLayer(b.layer);

            EditorCursor.Mode = OptCursor_t::LVL_BLOCKS;
            EditorCursor.Block = Block_t();
            EditorCursor.Block.Type = int(b.id);
            EditorCursor.Location.X = b.x;
            EditorCursor.Location.Y = b.y;
            EditorCursor.Location.Width = b.w;
            EditorCursor.Location.Height = b.h;
            if(EditorCursor.Block.Type <= maxBlockType && BlockIsSizable[EditorCursor.Block.Type])
            {
                EditorCursor.Block.Location.Width = 128;
                EditorCursor.Block.Location.Height = 128;
                EditorCursor.Location.Width = 128;
                EditorCursor.Location.Height = 128;
            }
            EditorCursor.Block.Invis = b.invisible;
            EditorCursor.Block.Slippy = b.slippery;
            EditorCursor.Block.Special = b.npc_id > 0 ? int(b.npc_id + 1000) : int(-b.npc_id);
            EditorCursor.Block.Layer = FindLayer(b.layer);
            EditorCursor.Block.TriggerHit = FindEvent(b.event_hit);
            EditorCursor.Block.TriggerLast = FindEvent(b.event_emptylayer);
            EditorCursor.Block.TriggerDeath = FindEvent(b.event_destroy);

            if(EditorCursor.Block.Type > maxBlockType) // Avoid out of range crash
                EditorCursor.Block.Type = 1;

            SetCursor();
        }
        else if(raw.compare(0, 9, "BGO_PLACE") == 0)
        {
            if(got.bgo.empty())
                break;

            const LevelBGO &b = got.bgo[0];

            if(EditorCursor.Mode != OptCursor_t::LVL_BGOS ||
               EditorCursor.Background.Type != int(b.id))
                PlaySound(SFX_Grab);

            EditorCursor.Layer = FindLayer(b.layer);

            EditorCursor.Mode = OptCursor_t::LVL_BGOS;
            EditorCursor.Background = Background_t();
            EditorCursor.Background.Type = int(b.id);
            EditorCursor.Location.X = b.x;
            EditorCursor.Location.Y = b.y;
            EditorCursor.Background.Layer = FindLayer(b.layer);
            EditorCursor.Background.SetSortPriority(b.z_mode, std::round(b.z_offset));

            if(EditorCursor.Background.Type > maxBackgroundType) // Avoid out of range crash
                EditorCursor.Background.Type = 1;

            SetCursor();
        }
        else if(raw.compare(0, 9, "NPC_PLACE") == 0)
        {
            if(got.npc.empty())
                break;

            const LevelNPC &n = got.npc[0];

            if(EditorCursor.Mode != OptCursor_t::LVL_NPCS ||
               EditorCursor.NPC.Type != int(n.id))
                PlaySound(SFX_Grab);

            EditorCursor.Layer = FindLayer(n.layer);

            EditorCursor.Mode = OptCursor_t::LVL_NPCS;
            EditorCursor.NPC = NPC_t();
            EditorCursor.NPC.Type = NPCID(n.id);
            EditorCursor.NPC.Hidden = false;
            EditorCursor.Location.X = n.x;
            EditorCursor.Location.Y = n.y;
            EditorCursor.NPC.Direction = n.direct;

            if(n.id > maxNPCType) // Avoid out of range crash
                EditorCursor.NPC.Type = NPCID(1);

            if(EditorCursor.NPC.Type == NPCID_ITEM_BURIED || EditorCursor.NPC.Type == NPCID_ITEM_POD || EditorCursor.NPC.Type == NPCID_ITEM_THROWER || EditorCursor.NPC.Type == NPCID_ITEM_BUBBLE)
            {
                EditorCursor.NPC.Special = (vbint_t)n.contents;
                EditorCursor.NPC.DefaultSpecial = EditorCursor.NPC.Special;
            }
            if(EditorCursor.NPC.Type == 288 || EditorCursor.NPC.Type == 289 || (EditorCursor.NPC.Type == 91 && int(EditorCursor.NPC.Special) == 288))
            {
                EditorCursor.NPC.Special2 = (vbint_t)n.special_data;
                EditorCursor.NPC.DefaultSpecial2 = EditorCursor.NPC.Special2;
            }

            if(NPCIsAParaTroopa(EditorCursor.NPC))
            {
                EditorCursor.NPC.Special = (vbint_t)n.special_data;
                EditorCursor.NPC.DefaultSpecial = EditorCursor.NPC.Special;
            }

            if(EditorCursor.NPC->IsFish)
            {
                EditorCursor.NPC.Special = (vbint_t)n.special_data;
                EditorCursor.NPC.DefaultSpecial = EditorCursor.NPC.Special;
            }

            if(EditorCursor.NPC.Type == 260)
            {
                EditorCursor.NPC.Special = (vbint_t)n.special_data;
                EditorCursor.NPC.DefaultSpecial = EditorCursor.NPC.Special;
            }

            if(EditorCursor.NPC.Type == 86)
            {
                if(n.special_data >= 0 && n.special_data < 256)
                    EditorCursor.NPC.Variant = (uint8_t)n.special_data;
                else
                    pLogWarning("Out of range Variant value %ld from IntProc", (long)n.special_data);
            }

            EditorCursor.NPC.Generator = n.generator;
            if(EditorCursor.NPC.Generator)
            {
                EditorCursor.NPC.GeneratorDirection = n.generator_direct;
                EditorCursor.NPC.GeneratorEffect = n.generator_type;
                EditorCursor.NPC.GeneratorTimeMax = n.generator_period;
            }

            if(!n.msg.empty())
                SetS(EditorCursor.NPC.Text, n.msg);

            EditorCursor.NPC.Inert = n.friendly;
            if(EditorCursor.NPC.Type == 151)
                EditorCursor.NPC.Inert = true;
            EditorCursor.NPC.Stuck = n.nomove;
            EditorCursor.NPC.DefaultStuck = EditorCursor.NPC.Stuck;

            EditorCursor.NPC.Legacy = n.is_boss;

            EditorCursor.NPC.Layer = FindLayer(n.layer);
            EditorCursor.NPC.TriggerActivate = FindEvent(n.event_activate);
            EditorCursor.NPC.TriggerDeath = FindEvent(n.event_die);
            EditorCursor.NPC.TriggerTalk = FindEvent(n.event_talk);
            EditorCursor.NPC.TriggerLast = FindEvent(n.event_emptylayer);
            EditorCursor.NPC.AttLayer = FindLayer(n.attach_layer);

            EditorCursor.NPC.DefaultType = EditorCursor.NPC.Type;
            EditorCursor.NPC.Location.Width = EditorCursor.NPC->TWidth;
            EditorCursor.NPC.Location.Height = EditorCursor.NPC->THeight;
            EditorCursor.NPC.DefaultLocationX = EditorCursor.NPC.Location.X;
            EditorCursor.NPC.DefaultLocationY = EditorCursor.NPC.Location.Y;
            EditorCursor.NPC.DefaultDirection = EditorCursor.NPC.Direction;
            EditorCursor.NPC.TimeLeft = 1;
            EditorCursor.NPC.Active = true;
            EditorCursor.NPC.JustActivated = 1;
            SetCursor();
        }
        else
            PlaySound(SFX_Fireworks);

        break;
    }
    }

    IntProc::cmdUnLock();
}
#endif // THEXTECH_INTERPROC_SUPPORTED

// int EditorNPCFrame(const NPCID A, vbint_t C, int N)
// {
//     return EditorNPCFrame(A, C, N);
// }

int EditorNPCFrame(const NPCID A, vbint_t& C, int N)
{
    int ret = 0;
// find the default left/right frames for NPCs

    if(A > maxNPCType)
        return ret;
    int B = 0;
    int D = 0;
    int E = 0;
    B = C;
    while(B == 0)
        B = iRand(3) - 1;

    if(!LevelEditor)
        C = B;
    if(A == 241)
        ret = 4;
    if(A == 195)
        ret = 3;

    // suits
    if(N > 0)
    {
        if(A == 169 || A == 170)
        {
            E = 0;
            for(D = 1; D <= numPlayers; D++)
            {
                if(!Player[D].Dead && Player[D].Section == NPC[N].Section && Player[D].Character != 3 &&
                    Player[D].Character != 4 && Player[D].TimeToLive == 0)
                {
                    if(E == 0 || std::abs(NPC[N].Location.X + NPC[N].Location.Width / 2.0 -
                                          (Player[D].Location.X + Player[D].Location.Width / 2.0)) +
                                 std::abs(NPC[N].Location.Y + NPC[N].Location.Height / 2.0 -
                                          (Player[D].Location.Y + Player[D].Location.Height / 2.0)) < D)
                    {
                        E = CInt(std::abs(NPC[N].Location.X + NPC[N].Location.Width / 2.0 - (Player[D].Location.X + Player[D].Location.Width / 2.0)) +
                                 std::abs(NPC[N].Location.Y + NPC[N].Location.Height / 2.0 - (Player[D].Location.Y + Player[D].Location.Height / 2.0)));
                        if(Player[D].Character == 5)
                            ret = 1;
                        else
                            ret = 0;
                    }
                }
            }
        }
    }


    if(A == 135 || A == 4 || A == 6 || A == 19 || A == 20 || A == 23 || A == 25 || A == 28 || A == 36 || A == 38 ||
       A == 42 || A == 43 || A == 44 || A == 193 || A == 35 || A == 191 || A == 52 || A == 72 || A == 77 || A == 108 ||
       (A >= 109 && A <= 112) || (A >= 121 && A <= 124) || A == 125 || (A >= 129 && A <= 132) || A == 136 || A == 158 ||
        A == 164 || A == 163 || A == 162 || A == 165 || A == 166 || A == 189 || A == 199 || A == 209 || A == 207 ||
        A == 229 || A == 230 || A == 232 || A == 236 || A == 233 || A == 173 || A == 175 || A == 177 ||
        A == 178 || A == 176) // Koopa troopas / Shy guy
    {
        if(int(B) == -1)
            ret = 0;
        else
            ret = 2;
    }

    // Bullet Bills
    if(A == 17 || A == 18 || A == 29 || A == 31 || A == 84 || A == 94 || A == 198 ||
       NPCIsYoshi(A) || A == 101 || A == 102 || A == 181 || A == 81)
    {
        if(int(B) == -1)
            ret = 0;
        else
            ret = 1;
    }

    // Hammer Bros.
    if(A == 29 || A == 55 || A == 75 || A == 78 || A == 168 || A == 234)
    {
        if(int(B) == -1)
            ret = 0;
        else
            ret = 3;
    }

    if(A == 34)
    {
        if(int(B) == -1)
            ret = 1;
        else
            ret = 0;
    }

    if(A == 201)
    {
        if(int(B) == -1)
            ret = 0;
        else
            ret = 8;
    }

    if(A == 137)
    {
        if(int(B) == -1)
            ret = 0;
        else
            ret = 6;
    }

    if(A == 86 || (A >= 117 && A <= 120) || A == 200)
    {
        if(int(B) == -1)
            ret = 0;
        else
            ret = 5;
    }

    // winged koopa / bob-omb buddy
    if(A == 76 || A == 107 || A == 160 || A == 161 || A == 167 || A == 203 || A == 204)
    {
        if(int(B) == -1)
            ret = 0;
        else
            ret = 4;
    }

    // Birdo
    if(A == 39 || A == 208)
    {
        if(int(B) == -1)
            ret = 0;
        else
            ret = 5;
    }

    if(A == 45)
        ret = BlockFrame[4];

    if(A == 56)
    {
        ret = SpecialFrame[2];
        if(int(B) == 1)
            ret += 4;
    }

    if(A == 57) // smb3 belt
    {
        if(int(B) == -1)
            ret = SpecialFrame[4];
        else
            ret = 3 - SpecialFrame[4];
    }

    if(A == 60 || A == 62 || A == 64 || A == 66)
    {
        if(int(B) == -1)
            ret = 1;
    }

    return ret;
}

inline void s_CloseProps()
{
#ifdef THEXTECH_INTERPROC_SUPPORTED
    // Tell the IPC Editor to close the properties dialog
    if(IntProc::isWorking())
        IntProc::sendCloseProperties();
#endif
}

void GetEditorControls()
{
    if(GamePaused != PauseCode::None)
    {
        if(HasCursor)
            HideCursor();
        return;
    }

    if(SharedCursor.Move)
    {
        MouseMove(SharedCursor.X, SharedCursor.Y);
    }

    if(SharedCursor.Secondary || (EditorControls.ModeSelect && !MagicHand))
    {
        if(EditorCursor.Mode != OptCursor_t::LVL_SELECT && EditorCursor.Mode != OptCursor_t::LVL_ERASER)
            s_CloseProps();

        EditorCursor.Mode = OptCursor_t::LVL_SELECT;
        SetCursor();
    }

    if(SharedCursor.Tertiary || (EditorControls.ModeErase && !MagicHand))
    {
        if(EditorCursor.Mode != OptCursor_t::LVL_SELECT && EditorCursor.Mode != OptCursor_t::LVL_ERASER)
            s_CloseProps();

        EditorCursor.Mode = OptCursor_t::LVL_ERASER;
        SetCursor();
    }

    if(MagicHand)
        return;

    if(!WorldEditor && EditorControls.TestPlay && MouseRelease)
    {
        EditorBackup();
        Backup_FullFileName = FullFileName;
        FullFileName = FullFileName + "tst";
        SaveLevel(FullFileName, FileFormat);

        if(g_config.EnableInterLevelFade)
            g_levelScreenFader.setupFader(4, 0, 65, ScreenFader::S_FADE);
        else
            g_levelScreenFader.setupFader(65, 0, 65, ScreenFader::S_FADE);
        editorWaitForFade();

        // force reconnect on leveltest start
        Controls::ClearInputMethods();

        HasCursor = false;
        zTestLevel(editorScreen.test_magic_hand);
    }
    if(EditorControls.SwitchScreens && MouseRelease)
    {
#ifdef __3DS__
        int win_x, win_y;
        XRender::mapFromScreen(SharedCursor.X, SharedCursor.Y, &win_x, &win_y);
#endif

        editorScreen.active = !editorScreen.active;

#ifdef __3DS__
        int m_x, m_y;
        XRender::mapToScreen(win_x, win_y, &m_x, &m_y);
        SharedCursor.X = m_x;
        SharedCursor.Y = m_y;
        MouseMove(m_x, m_y);
#endif

        HasCursor = false;
        MouseRelease = false;
        MenuMouseRelease = false;
        PlaySound(SFX_Pause);
    }

    if(EditorControls.ScrollDown || EditorControls.ScrollUp || EditorControls.ScrollLeft || EditorControls.ScrollRight)
    {
        scroll_buffer_x += EditorControls.ScrollRight;
        scroll_buffer_x -= EditorControls.ScrollLeft;
        scroll_buffer_y += EditorControls.ScrollDown;
        scroll_buffer_y -= EditorControls.ScrollUp;
    }
    else
    {
        scroll_buffer_x = 0;
        scroll_buffer_y = 0;
    }
}

void SetCursor()
{
//    EditorCursor.Layer = frmLayers::lstLayer::List(frmLayers::lstLayer::ListIndex);
    if(EditorCursor.Mode == OptCursor_t::LVL_ERASER) // Eraser
    {
        EditorCursor.Location.Width = 18;
        EditorCursor.Location.Height = 8;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_SELECT || EditorCursor.Mode == 14) // Selection
    {
        EditorCursor.Location.Width = 4;
        EditorCursor.Location.Height = 4;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_WATER) // Water
    {
//        EditorCursor.Location.Height = frmWater::WaterH * 32;
//        EditorCursor.Location.Width = frmWater::WaterW * 32;
        EditorCursor.Location.Height = EditorCursor.Water.Location.Height;
        EditorCursor.Location.Width = EditorCursor.Water.Location.Width;
        EditorCursor.Water.Location.X = EditorCursor.Location.X;
        EditorCursor.Water.Location.Y = EditorCursor.Location.Y;
        EditorCursor.Water.Buoy = 0; // frmWater.scrBuoy / 100
        EditorCursor.Water.Layer = EditorCursor.Layer;
//        if(frmWater::Quicksand.Caption == "Yes")
            // EditorCursor.Water.Quicksand = false;
//        else
//            EditorCursor.Water.Quicksand = false;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_BLOCKS) // Blocks
    {
        if(EditorCursor.Block.Type <= 0)
            EditorCursor.Block.Type = 1;
        if(EditorCursor.Block.Type > maxBlockType)
            EditorCursor.Block.Type = 1;

        EditorCursor.Block.Location.X = EditorCursor.Location.X;
        EditorCursor.Block.Location.Y = EditorCursor.Location.Y;
        EditorCursor.Block.Layer = EditorCursor.Layer;
//        EditorCursor.Block.TriggerHit = frmAdvancedBlock::TriggerHit.Text;
//        EditorCursor.Block.TriggerDeath = frmAdvancedBlock::TriggerDeath.Text;
//        EditorCursor.Block.TriggerLast = frmAdvancedBlock::TriggerLast.Text;
//        for(A = 1; A <= frmBlocks::Block.Count; A++)
//        {
//            if(frmBlocks::Block(A).Value == true && frmBlocks::Block(A).Visible == true)
//            {
//                EditorCursor.Block.Type = A;
//                break;
//            }
//        }
        if(!BlockIsSizable[EditorCursor.Block.Type])
        {
            if(EditorCursor.Block.Location.Width <= 0)
            {
                if(BlockWidth[EditorCursor.Block.Type] > 0)
                    EditorCursor.Block.Location.Width = BlockWidth[EditorCursor.Block.Type];
                else
                    EditorCursor.Block.Location.Width = 32;
            }

            if(EditorCursor.Block.Location.Height <= 0)
            {
                if(BlockHeight[EditorCursor.Block.Type] > 0)
                    EditorCursor.Block.Location.Height = BlockHeight[EditorCursor.Block.Type];
                else
                    EditorCursor.Block.Location.Height = 32;
            }
        }

        EditorCursor.Location.Width = EditorCursor.Block.Location.Width;
        EditorCursor.Location.Height = EditorCursor.Block.Location.Height;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_PLAYERSTART) // Level
    {
        if(EditorCursor.SubMode == 4)
        {
            EditorCursor.Location.Width = Physics.PlayerWidth[1][2]; // Mario
            EditorCursor.Location.Height = Physics.PlayerHeight[1][2];
        }
        else if(EditorCursor.SubMode == 5)
        {
            EditorCursor.Location.Width = Physics.PlayerWidth[2][2]; // Luigi
            EditorCursor.Location.Height = Physics.PlayerHeight[2][2];
        }
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_BGOS) // Background
    {
        if(EditorCursor.Background.Type <= 0)
            EditorCursor.Background.Type = 1;
        if(EditorCursor.Background.Type > maxBackgroundType)
            EditorCursor.Background.Type = 1;

        EditorCursor.Background.Layer = EditorCursor.Layer;
        EditorCursor.Background.Location.X = EditorCursor.Location.X;
        EditorCursor.Background.Location.Y = EditorCursor.Location.Y;
        EditorCursor.Background.Location.Width = BackgroundWidth[EditorCursor.Background.Type];
        EditorCursor.Background.Location.Height = BackgroundHeight[EditorCursor.Background.Type];
        EditorCursor.Location.Width = EditorCursor.Background.Location.Width;
        EditorCursor.Location.Height = EditorCursor.Background.Location.Height;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_NPCS) // NPCs
    {
        int t = EditorCursor.NPC.Type;
        // Container NPCs are handled elsewhere in new editor
        if(MagicHand)
        {
            if(t != 91 && t != 96 && t != 283 && t != 284 && !NPCTraits[t].IsFish && !NPCIsAParaTroopa(t) && t != NPCID_FIRE_CHAIN)
                EditorCursor.NPC.Special = 0;
            if(t != 288 && t != 289 && t != 91 && t != 260)
                EditorCursor.NPC.Special2 = 0;
        }
        EditorCursor.NPC.Special3 = 0;
        EditorCursor.NPC.Special4 = 0;
        EditorCursor.NPC.Special5 = 0;
        // EditorCursor.NPC.Special6 = 0;
        EditorCursor.NPC.SpecialX = 0.0;
        EditorCursor.NPC.SpecialY = 0.0;
        EditorCursor.NPC.Layer = EditorCursor.Layer;
        EditorCursor.NPC.Location = EditorCursor.Location;

        if(EditorCursor.NPC->TWidth > 0)
            EditorCursor.NPC.Location.Width = EditorCursor.NPC->TWidth;
        else
            EditorCursor.NPC.Location.Width = 32;
        if(EditorCursor.NPC->THeight > 0)
            EditorCursor.NPC.Location.Height = EditorCursor.NPC->THeight;
        else
            EditorCursor.NPC.Location.Height = 32;
        EditorCursor.Location.Width = EditorCursor.NPC.Location.Width;
        EditorCursor.Location.Height = EditorCursor.NPC.Location.Height;

        EditorCursor.Location.SpeedX = 0.0;
        EditorCursor.Location.SpeedY = 0.0;

        EditorCursor.NPC.Frame = EditorNPCFrame(EditorCursor.NPC.Type, EditorCursor.NPC.Direction);
        EditorCursor.NPC.Active = true;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS) // Warps
    {
        EditorCursor.Warp.Layer = EditorCursor.Layer;
        EditorCursor.Location.Width = 32;
        EditorCursor.Location.Height = 32;
        // EditorCursor.Warp is now the canonical Warp object.
        // It stores the warp's entrance and exit until the warp is placed,
        // instead of finding and modifying an existing warp.
        // EditorCursor.Warp.Entrance = EditorCursor.Location;
        // EditorCursor.Warp.Exit = EditorCursor.Location;
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_TILES) // Tiles
    {
        if(EditorCursor.Tile.Type == 0)
            EditorCursor.Tile.Type = 1;
        EditorCursor.Location.Width = TileWidth[EditorCursor.Tile.Type];
        EditorCursor.Location.Height = TileHeight[EditorCursor.Tile.Type];
        EditorCursor.Tile.Location = static_cast<TinyLocation_t>(EditorCursor.Location);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_SCENES) // Scene
    {
        if(EditorCursor.Scene.Type == 0)
            EditorCursor.Scene.Type = 1;
        EditorCursor.Location.Width = SceneWidth[EditorCursor.Scene.Type];
        EditorCursor.Location.Height = SceneHeight[EditorCursor.Scene.Type];
        EditorCursor.Scene.Location = static_cast<TinyLocation_t>(EditorCursor.Location);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_LEVELS) // Levels
    {
        if(EditorCursor.WorldLevel.Type == 0)
            EditorCursor.WorldLevel.Type = 1;
        EditorCursor.Location.Width = 32;
        EditorCursor.Location.Height = 32;
        EditorCursor.WorldLevel.Location = static_cast<TinyLocation_t>(EditorCursor.Location);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_PATHS) // Paths
    {
        if(EditorCursor.WorldPath.Type == 0)
            EditorCursor.WorldPath.Type = 1;
        EditorCursor.Location.Width = 32;
        EditorCursor.Location.Height = 32;
        EditorCursor.WorldPath.Location = static_cast<TinyLocation_t>(EditorCursor.Location);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_MUSIC) // World Music
    {
        EditorCursor.Location.Height = 32;
        EditorCursor.Location.Width = 32;
        EditorCursor.WorldMusic.Location = static_cast<TinyLocation_t>(EditorCursor.Location);
        // make it play the music
        if(g_isWorldMusicNotSame(EditorCursor.WorldMusic))
            g_playWorldMusic(EditorCursor.WorldMusic);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_AREA) // World Area
    {
        EditorCursor.Location.Height = 32;
        EditorCursor.Location.Width = 32;
        EditorCursor.WorldArea.Location.X = EditorCursor.Location.X;
        EditorCursor.WorldArea.Location.Y = EditorCursor.Location.Y;

        if(EditorCursor.WorldArea.Location.Width < 32)
            EditorCursor.WorldArea.Location.Width = 32;

        if(EditorCursor.WorldArea.Location.Height < 32)
            EditorCursor.WorldArea.Location.Height = 32;
    }
}

void PositionCursor()
{
//    if(EditorCursor.Mode == 4 && frmNPCs::Buried.Caption == "Yes")
//        EditorCursor.Location.Y += 16;

    if(EditorCursor.Mode == OptCursor_t::LVL_SELECT)
        return;

    if(!enableAutoAlign)
    {
        EditorCursor.Location.X += -EditorCursor.Location.Width + 4;
        EditorCursor.Location.Y += -EditorCursor.Location.Height + 12;
        return;
    }

    if(EditorCursor.Mode == OptCursor_t::LVL_PLAYERSTART)
        EditorCursor.Location.X -= 14;

    if(EditorCursor.Mode == OptCursor_t::LVL_PLAYERSTART || EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        if(!(EditorCursor.Mode == OptCursor_t::LVL_NPCS && EditorCursor.NPC.Type == 52))
        {
            if(std::fmod(EditorCursor.Location.Width, 32) != 0.0)
            {
                if(EditorCursor.Location.Width > 32)
                    EditorCursor.Location.X += -std::fmod(EditorCursor.Location.Width, 32) / 2.0;
                else if(EditorCursor.Location.Width <= 16)
                {
                    EditorCursor.Location.X += std::fmod(32, EditorCursor.Location.Width) / 2.0;
                    EditorCursor.Location.X += (32 - EditorCursor.Location.Width) / 2.0;
                }
                else if(EditorCursor.Location.Width < 32)
                    EditorCursor.Location.X += std::fmod(32, EditorCursor.Location.Width) / 2.0;
                else
                    EditorCursor.Location.X += std::fmod(32, EditorCursor.Location.Width) / 2.0;
            }

            if(std::fmod(EditorCursor.Location.Height, 32) != 0.0)
                EditorCursor.Location.Y += -std::fmod(EditorCursor.Location.Height, 32);
        }
    }

    if(EditorCursor.Mode == OptCursor_t::LVL_BGOS)
    {
        if(EditorCursor.Background.Type == 13) // End level container
        {
            EditorCursor.Location.X -= 12;
            EditorCursor.Location.Y -= 12;
        }
        if(EditorCursor.Background.Type == 156 || EditorCursor.Background.Type == 157)
            EditorCursor.Location.Y += 16;
    }

    else if(EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        if(EditorCursor.NPC.Type == 245 || EditorCursor.NPC.Type == 8 ||
           EditorCursor.NPC.Type == 270 || EditorCursor.NPC.Type == 93 ||
           EditorCursor.NPC.Type == 180 || EditorCursor.NPC.Type ==179 ||
           EditorCursor.NPC.Type == 37 || EditorCursor.NPC.Type ==51) // Piranha Plants
            EditorCursor.Location.X += 16;
        else if(EditorCursor.NPC.Type == 197)
        {
            EditorCursor.Location.X -= 8;
            EditorCursor.Location.Y += 16;
        }

//        if(frmNPCs::Buried.Caption == "Yes")
        if(EditorCursor.NPC.Type == 91)
        {
            if(!enableAutoAlign)
                EditorCursor.Location.Y += 0/*16*/;
            else
                EditorCursor.Location.Y += 16/*32*/;
        }
        else if(EditorCursor.NPC.Type == 105)
            EditorCursor.Location.Y += 22;
        else if(EditorCursor.NPC.Type == 106)
            EditorCursor.Location.Y += 16;
        else if(EditorCursor.NPC.Type == 260)
            EditorCursor.Location.Y -= 8;
    }

    if(EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        if(EditorCursor.NPC->THeight < 32)
            EditorCursor.Location.Y += 32;
    }
}

void HideCursor()
{
    // printf("Hiding cursor...\n");
    EditorCursor.Location.X = vScreen[1].X - 800;
    EditorCursor.X = float(vScreen[1].X - 800);
    EditorCursor.Location.Y = vScreen[1].Y - 600;
    EditorCursor.Y = float(vScreen[1].Y - 600);
    HasCursor = false;
    EditorControls.ScrollDown = false;
    EditorControls.ScrollRight = false;
    SharedCursor.Primary = false;
    MouseCancel = true;
    EditorControls.ScrollLeft = false;
    EditorControls.ScrollUp = false;
    SharedCursor.GoOffscreen();
}

void KillWarp(int A)
{
    Warp_t blankWarp;
    Warp[A] = Warp[numWarps];
    Warp[numWarps] = blankWarp;
    numWarps--;
    syncLayers_Warp(A);
    syncLayers_Warp(numWarps+1);
}

void zTestLevel(bool magicHand, bool interProcess)
{
    int A = 0;
    Player_t blankPlayer;
    qScreen = false;
    qScreen_canonical = false;

#ifndef THEXTECH_INTERPROC_SUPPORTED
    UNUSED(interProcess);
#endif

    for(A = 1; A <= numCharacters; A++)
    {
        SavedChar[A] = blankPlayer;
        SavedChar[A].State = 1;
        SavedChar[A].Character = A;
    }

    if(numPlayers == 0)
        numPlayers = editorScreen.num_test_players;

    if(BattleMode && numPlayers < 2)
        numPlayers = 2;

    for(A = 1; A <= numNPCs; A++)
    {
        auto &n = NPC[A];
        // If .Generator = True Then
        n.TimeLeft = 0;
        n.Active = false;
        // End If
    }

    GamePaused = PauseCode::None;
    MessageText.clear();
    MessageTextMap.clear();
//  frmNPCs::chkMessage.Value = 0;
    for(int i = 1; i <= numPlayers; i++)
    {
        BattleLives[i] = 3;
        Player[i].Hearts = 1;
    }
    BattleIntro = 150;
    BattleWinner = 0;
    BattleOutro = 0;
//  frmLevelEditor::mnuOnline.Enabled = false;
    StopMusic();
    Score = 0;
    Coins = 0;
    Lives = 3;
    g_100s = 3;

    if(Checkpoint.empty()) // Don't reset players when resume at the checkpoint
    {
        if(g_ClonedPlayerMode)
        {
            for(A = 1; A <= numPlayers; A++)
            {
                Player[A] = Player_t();
                Player[A].Hearts = 0;
                Player[A].State = testPlayer[1].State;
                Player[A].HeldBonus = NPCID_NULL;
                Player[A].Dead = false;
                Player[A].Mount = testPlayer[1].Mount;
                Player[A].MountType = testPlayer[1].MountType;
                Player[A].Character = testPlayer[1].Character;
                Player[A].UnStart = false;
                if(Player[A].Character == 0)
                    Player[A].Character = 1;
            }
        }
        else
        {
            for(A = numPlayers; A >= 1; A--)
            {
                Player[A] = Player_t();
                Player[A].State = testPlayer[A].State;
                Player[A].HeldBonus = NPCID_NULL;
                Player[A].Dead = false;
                Player[A].Mount = testPlayer[A].Mount;
                Player[A].MountType = testPlayer[A].MountType;
                Player[A].Character = testPlayer[A].Character;
                if(Player[A].Character == 0)
                    Player[A].Character = A;
                SavedChar[Player[A].Character] = Player[A];
                Player[A].UnStart = false;
            }
        }

        StartWarp = testStartWarp;
    }

    LevelEditor = false;
    TestLevel = true;

    UpdateInternalRes();

    // assign players to screens
    InitScreens();
    for(A = 1; A <= numPlayers; A++)
    {
        Screens_AssignPlayer(A, *l_screen);
        if(g_ClonedPlayerMode)
            break;
    }

    SetupPlayers();
    MagicHand = magicHand;
    FontManager::clearAllCustomFonts();

    // this clears the cached medals and stars data from the level
    LevelWarpSaveEntries.clear();

    if(TestFullscreen)
    {
#ifndef RENDER_FULLSCREEN_ALWAYS
        ChangeScreen();
#endif
        XEvents::doEvents();
        MagicHand = false;
    }

    // in speedrun mode, confirm that controls are set up before game starts
    if(g_config.speedrun_mode != 0 && !Controls::Update())
    {
        ClearLevel();
        // force players offscreen
        for(int i = 1; i <= maxLocalPlayers; i++)
            Player[i].Location.X = -20000.0;

        LevelBeatCode = -3;
        QuickReconnectScreen::g_active = true;
        PauseGame(PauseCode::PauseScreen);
        LevelBeatCode = 0;
    }

#ifdef THEXTECH_INTERPROC_SUPPORTED
    if(interProcess)
    {
        pLogDebug("ICP: Requesting editor for a file....");
        IntProc::sendMessage("CMD:CONNECT_TO_ENGINE");
        ElapsedTimer time;
        time.start();
        //wait for accepting of level data
        bool timeOut = false;
        int attempts = 0;

        pLogDebug("ICP: Waiting reply....");
        IntProc::setState("Waiting for input data...");
        while(!IntProc::hasLevelData())
        {
            UpdateLoad();

            //Abort loading process and exit from game if window was closed
            if(!GameIsActive)
                return;

            if(!IntProc::levelReceivingInProcess() && time.elapsed() > 1500)
            {
                pLogDebug("ICP: Waiting #%d....", attempts);
                IntProc::sendMessage("CMD:CONNECT_TO_ENGINE"); // Re-ask again
                time.restart();
                attempts += 1;
            }

            if(attempts > 4)
            {
                pLogWarning("ICP: Wait timeout");
                timeOut = true;
                IntProc::setState("ERROR: Wait time out.");
                UpdateLoad();
                PGE_Delay(1000);
                GameIsActive = false;
                return;
            }
            PGE_Delay(2);
        }

        if(!timeOut && !OpenLevelData(IntProc::editor->m_acceptedLevel, IntProc::editor->m_accepted_lvl_path)) //-V560
        {
            pLogWarning("Bad file format!");
            GameIsActive = false;
            return;
        }

        OpenLevelDataPost();

        pLogDebug("ICP: Done, starting a game....");
        IntProc::setState("Done. Starting game...");
    }
    else
#endif // THEXTECH_INTERPROC_SUPPORTED
    {
        if(!OpenLevel(FullFileName))
        {
            MessageText = fmt::format_ne(g_gameStrings.errorOpenFileFailed, FullFileName);
            PauseGame(PauseCode::Message);
            ErrorQuit = true;
        }
    }

    // reset Drop/Add allowed characters
    ConnectScreen::SaveChars();

    int waitms = (g_config.speedrun_mode != 0) ? 750 : 0;
    GameThing(waitms, 0);

    SetupScreens();
    LevelSelect = false;
    EndLevel = false;
    editorScreen.active = false;
    ReturnWarp = 0;

    EditorCursor.Mode = OptCursor_t::LVL_SELECT;
}

static constexpr int s_resize_border = 8;

template<class LocType>
static inline int s_find_flags(const LocType& loc)
{
    // require cursor to be at least nearby
    if(    EditorCursor.Location.X < loc.X - s_resize_border
        || EditorCursor.Location.X > loc.X + loc.Width + s_resize_border
        || EditorCursor.Location.Y < loc.Y - s_resize_border
        || EditorCursor.Location.Y > loc.Y + loc.Height + s_resize_border)
    {
        return 0;
    }

    int found_flags = 0;

    if(EditorCursor.Location.X < loc.X + s_resize_border)
        found_flags |= IF_ResizeL;
    else if(EditorCursor.Location.X > loc.X + loc.Width - s_resize_border)
        found_flags |= IF_ResizeR;

    if(EditorCursor.Location.Y < loc.Y + s_resize_border)
        found_flags |= IF_ResizeT;
    else if(EditorCursor.Location.Y > loc.Y + loc.Height - s_resize_border)
        found_flags |= IF_ResizeB;

    return found_flags;
}

static inline int s_find_flags_section(const SpeedlessLocation_t& loc)
{
    // require cursor to be at least nearby
    if(    EditorCursor.Location.X < loc.X - s_resize_border
        || EditorCursor.Location.X > loc.Width + s_resize_border
        || EditorCursor.Location.Y < loc.Y - s_resize_border
        || EditorCursor.Location.Y > loc.Height + s_resize_border)
    {
        return 0;
    }

    int found_flags = 0;

    if(EditorCursor.Location.X < loc.X + s_resize_border)
        found_flags |= IF_ResizeL;
    else if(EditorCursor.Location.X > loc.Width - s_resize_border)
        found_flags |= IF_ResizeR;

    if(EditorCursor.Location.Y < loc.Y + s_resize_border)
        found_flags |= IF_ResizeT;
    else if(EditorCursor.Location.Y > loc.Height - s_resize_border)
        found_flags |= IF_ResizeB;

    return found_flags;
}

void UpdateInteract()
{
    // only update items in select mode (mouse up or just pressed) or erase mode
    bool mouse_held = (SharedCursor.Primary && !MouseRelease);
    bool select_mode = EditorCursor.Mode == OptCursor_t::LVL_SELECT;
    bool erase_mode = EditorCursor.Mode == OptCursor_t::LVL_ERASER;

    if(select_mode && mouse_held)
        return;

    EditorCursor.InteractMode = 0;
    EditorCursor.InteractFlags = 0;
    EditorCursor.InteractIndex = 0;
    EditorCursor.InteractX = EditorCursor.Location.X;
    EditorCursor.InteractY = EditorCursor.Location.Y;

     if(!select_mode && !erase_mode)
        return;

    if(MouseCancel)
        return;

   // class filter for eraser
    int need_class = 0;
    if(erase_mode && EditorCursor.SubMode > 0)
        need_class = EditorCursor.SubMode;

    if(!WorldEditor)
    {
        // player start points
        if(!MagicHand && select_mode)
        {
            for(int A = 1; A <= 2; A++)
            {
                if(CursorCollision(EditorCursor.Location, PlayerStart[A]))
                {
                    EditorCursor.InteractMode = OptCursor_t::LVL_PLAYERSTART;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // NPCs
        // never sizable, so only do this if nothing found yet
        if(EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::LVL_NPCS))
        {
            for(int A : treeNPCQuery(EditorCursor.Location, SORTMODE_ID))
            {
                Location_t tempLocation = NPC[A].Location;

                if(NPC[A].Type == NPCID_ITEM_BURIED) // Herb's container offset
                    tempLocation.Y -= 16;

                if(CursorCollision(EditorCursor.Location, tempLocation) && !NPC[A].Hidden)
                {
                    EditorCursor.InteractMode = OptCursor_t::LVL_NPCS;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // non-sizable blocks (same condition)
        if(EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::LVL_BLOCKS))
        {
            for(int A : treeBlockQuery(EditorCursor.Location, SORTMODE_ID))
            {
                if(BlockIsSizable[Block[A].Type])
                    continue;

                if(CursorCollision(EditorCursor.Location, Block[A].Location) && !Block[A].Hidden)
                {
                    EditorCursor.InteractMode = OptCursor_t::LVL_BLOCKS;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // warps (same condition for now, until warps become sizable)
        if(!MagicHand && EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::LVL_WARPS))
        {
            for(int A = 1; A <= numWarps; A++)
            {
                if(CursorCollision(EditorCursor.Location, Warp[A].Entrance) && !Warp[A].Hidden)
                {
                    EditorCursor.InteractMode = OptCursor_t::LVL_WARPS;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
                else if(CursorCollision(EditorCursor.Location, Warp[A].Exit) && !Warp[A].Hidden)
                {
                    EditorCursor.InteractMode = OptCursor_t::LVL_WARPS;
                    EditorCursor.InteractFlags = IF_AltMode;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // BGOs
        // never sizable, so only do this if nothing found yet
        if(EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::LVL_BGOS))
        {
            // more difficult to iterate backwards, but that's what we need to do here
            auto sentinel = treeBackgroundQuery(EditorCursor.Location, SORTMODE_Z);
            for(auto i = sentinel.end(); i > sentinel.begin();)
            {
                int A = *(--i);

                if(CursorCollision(EditorCursor.Location, Background[A].Location) && !Background[A].Hidden)
                {
                    EditorCursor.InteractMode = OptCursor_t::LVL_BGOS;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // Sizable blocks
        // now things get exciting
        if(((select_mode && EditorCursor.InteractFlags < 2) || EditorCursor.InteractMode == 0)
            && (!need_class || need_class == OptCursor_t::LVL_BLOCKS)) // Sizable blocks
        {
            for(int A : treeBlockQuery(EditorCursor.Location, SORTMODE_ID))
            {
                if(!BlockIsSizable[Block[A].Type] || Block[A].Hidden)
                    continue;

                int found_flags = (select_mode) ? s_find_flags(Block[A].Location) : 0;

                if(found_flags || CursorCollision(EditorCursor.Location, Block[A].Location))
                {
                    if(found_flags || EditorCursor.InteractMode == 0)
                    {
                        EditorCursor.InteractMode = OptCursor_t::LVL_BLOCKS;
                        EditorCursor.InteractFlags = found_flags;
                        EditorCursor.InteractIndex = A;
                        break;
                    }
                }
            }
        }

        // Water boxes
        // can resize
        if(!MagicHand
            && ((select_mode && EditorCursor.InteractFlags < 2) || EditorCursor.InteractMode == 0)
            && (!need_class || need_class == OptCursor_t::LVL_WATER))
        {
            for(int A : treeWaterQuery(EditorCursor.Location, SORTMODE_ID))
            {
                if(Water[A].Hidden)
                    continue;

                int found_flags = (select_mode) ? s_find_flags(Water[A].Location) : 0;

                if(found_flags || CursorCollision(EditorCursor.Location, Water[A].Location))
                {
                    if(found_flags || EditorCursor.InteractMode == 0)
                    {
                        EditorCursor.InteractMode = OptCursor_t::LVL_WATER;
                        EditorCursor.InteractFlags = found_flags;
                        EditorCursor.InteractIndex = A;
                        break;
                    }
                }
            }
        }

        // event section borders
        if(!MagicHand && select_mode && EditorCursor.InteractFlags < 2)
        {
            for(int A = numEvents - 1; A >= 0; A--)
            {
                const auto& sectPos = Events[A].section[curSection].position;
                if(sectPos.X == EventSection_t::LESet_Nothing || sectPos.X == EventSection_t::LESet_ResetDefault)
                    continue;

                int found_flags = s_find_flags_section(sectPos);

                if(found_flags)
                {
                    EditorCursor.InteractMode = OptCursor_t::LVL_EVENTS;
                    EditorCursor.InteractFlags = found_flags;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // section borders
        if(!MagicHand && select_mode && EditorCursor.InteractFlags < 2)
        {
            int found_flags = s_find_flags_section(level[curSection]);

            if(found_flags)
            {
                EditorCursor.InteractMode = OptCursor_t::LVL_SECTION;
                EditorCursor.InteractFlags = found_flags;
                EditorCursor.InteractIndex = 0;
            }
        }
    }
    else
    {
        // world map areas
        // can resize
        if(((select_mode && EditorCursor.InteractFlags < 2) || EditorCursor.InteractMode == 0)
             && (!need_class || need_class == OptCursor_t::WLD_AREA))
        {
            for(int A = numWorldAreas; A >= 1; A--)
            {
                int found_flags = (select_mode) ? s_find_flags(WorldArea[A].Location) : 0;

                // count as collision if on corner, or if in top-left
                if(found_flags || (EditorCursor.InteractMode == 0 && CursorCollision(EditorCursor.Location, newLoc(WorldArea[A].Location.X, WorldArea[A].Location.Y, 32, 32))))
                {
                    EditorCursor.InteractMode = OptCursor_t::WLD_AREA;
                    EditorCursor.InteractFlags = found_flags;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // world map music
        if(EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::WLD_MUSIC))
        {
            for(int A : treeWorldMusicQuery(EditorCursor.Location, SORTMODE_NONE))
            {
                if(CursorCollision(EditorCursor.Location, WorldMusic[A].Location))
                {
                    EditorCursor.InteractMode = OptCursor_t::WLD_MUSIC;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // world paths
        if(EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::WLD_PATHS))
        {
            for(int A : treeWorldPathQuery(EditorCursor.Location, SORTMODE_NONE))
            {
                if(CursorCollision(EditorCursor.Location, WorldPath[A].Location))
                {
                    EditorCursor.InteractMode = OptCursor_t::WLD_PATHS;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // world sceneries
        if(EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::WLD_SCENES))
        {
            // harder to go backwards, but that's all we're doing here.
            // it's a good thing that the sentinel's scope ends quickly,
            // otherwise it would take a long time for the result vector
            // to rejoin the pool. -- ds-sloth
            auto sentinel = treeWorldSceneQuery(EditorCursor.Location, SORTMODE_ID);
            for(auto i = sentinel.end(); i > sentinel.begin();)
            {
                int A = *(--i);

                if(CursorCollision(EditorCursor.Location, Scene[A].Location))
                {
                    EditorCursor.InteractMode = OptCursor_t::WLD_SCENES;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // world levels
        if(EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::WLD_LEVELS))
        {
            for(int A : treeWorldLevelQuery(EditorCursor.Location, SORTMODE_NONE))
            {
                if(CursorCollision(EditorCursor.Location, WorldLevel[A].Location))
                {
                    EditorCursor.InteractMode = OptCursor_t::WLD_LEVELS;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }

        // world tiles
        if(EditorCursor.InteractMode == 0 && (!need_class || need_class == OptCursor_t::WLD_TILES))
        {
            for(int A : treeWorldTileQuery(EditorCursor.Location, SORTMODE_NONE))
            {
                if(CursorCollision(EditorCursor.Location, Tile[A].Location))
                {
                    EditorCursor.InteractMode = OptCursor_t::WLD_TILES;
                    EditorCursor.InteractFlags = 0;
                    EditorCursor.InteractIndex = A;
                    break;
                }
            }
        }
    }
}

template<class LocType>
void InteractResize(LocType& loc, int min, int snap)
{
    if(EditorCursor.InteractFlags & IF_ResizeL)
    {
        double dl = EditorCursor.Location.X - loc.X;
        if(dl > snap && loc.Width > min)
        {
            loc.X += snap;
            loc.Width -= snap;
            PlaySound(SFX_Saw);
        }
        else if(dl < -snap)
        {
            loc.X -= snap;
            loc.Width += snap;
            PlaySound(SFX_Saw);
        }
    }

    if(EditorCursor.InteractFlags & IF_ResizeR)
    {
        double dr = EditorCursor.Location.X - (loc.X + loc.Width);
        if(dr < -snap && loc.Width > min)
        {
            loc.Width -= snap;
            PlaySound(SFX_Saw);
        }
        else if(dr > snap)
        {
            loc.Width += snap;
            PlaySound(SFX_Saw);
        }
    }

    if(EditorCursor.InteractFlags & IF_ResizeT)
    {
        double dt = EditorCursor.Location.Y - loc.Y;
        if(dt > snap && loc.Height > min)
        {
            loc.Y += snap;
            loc.Height -= snap;
            PlaySound(SFX_Saw);
        }
        else if(dt < -snap)
        {
            loc.Y -= snap;
            loc.Height += snap;
            PlaySound(SFX_Saw);
        }
    }

    if(EditorCursor.InteractFlags & IF_ResizeB)
    {
        double db = EditorCursor.Location.Y - (loc.Y + loc.Height);
        if(db < -snap && loc.Height > min)
        {
            loc.Height -= snap;
            PlaySound(SFX_Saw);
        }
        else if(db > snap)
        {
            loc.Height += snap;
            PlaySound(SFX_Saw);
        }
    }
}

void InteractResizeSection(SpeedlessLocation_t& section)
{
    bool resized = false;

    if(EditorCursor.InteractFlags & IF_ResizeT)
    {
        int new_Y = static_cast<int>(round(static_cast<double>(EditorCursor.Location.Y / 32))) * 32;
        if(section.Height - new_Y < 600)
            new_Y = section.Height - 600;

        if(new_Y != section.Y)
        {
            section.Y = new_Y;
            resized = true;
        }
    }

    if(EditorCursor.InteractFlags & IF_ResizeL)
    {
        int new_X = static_cast<int>(round(static_cast<double>(EditorCursor.Location.X / 32))) * 32;
        if(section.Width - new_X < 800)
            new_X = section.Width - 800;

        if(new_X != section.X)
        {
            section.X = new_X;
            resized = true;
        }
    }

    if(EditorCursor.InteractFlags & IF_ResizeR)
    {
        int new_Width = static_cast<int>(round(static_cast<double>(EditorCursor.Location.X / 32))) * 32;
        if(new_Width - section.X < 800)
            new_Width = section.X + 800;

        if(new_Width != section.Width)
        {
            section.Width = new_Width;
            resized = true;
        }
    }

    if(EditorCursor.InteractFlags & IF_ResizeB)
    {
        int new_Height = static_cast<int>(round(static_cast<double>(EditorCursor.Location.Y / 32))) * 32;
        if(new_Height - section.Y < 600)
            new_Height = section.Y + 600;

        if(new_Height != section.Height)
        {
            section.Height = new_Height;
            resized = true;
        }
    }

    if(resized)
        PlaySound(SFX_Saw);
}

void MouseMove(float X, float Y, bool /*nCur*/)
{
    EditorCursor.X = X;
    EditorCursor.Y = Y;

    HasCursor = true;

    // figure out which VScreen to use
    int A = 1;
    if(SingleCoop > 0)
        A = SingleCoop;
    else if(l_screen->Type == 5 && vScreen[2].Visible)
    {
        if(X < float(vScreen[2].TargetX() + vScreen[2].Width))
        {
            if(X > float(vScreen[2].TargetX()))
            {
                if(Y < float(vScreen[2].TargetY() + vScreen[2].Height))
                {
                    if(Y > float(vScreen[2].TargetY()))
                        A = 2;
                }
            }
        }
    }
    else
        A = 1;

    X -= vScreen[A].TargetX();
    Y -= vScreen[A].TargetY();

    if(XRender::TargetOverscanX && WorldEditor)
        X -= XRender::TargetOverscanX;

    // translate into layer coordinates to snap to layer's grid
    if(MagicHand && EditorCursor.Layer != LAYER_NONE)
    {
        X -= Layer[EditorCursor.Layer].OffsetX;
        Y -= Layer[EditorCursor.Layer].OffsetY;
    }

    if(EditorCursor.Mode == OptCursor_t::LVL_ERASER || EditorCursor.Mode == OptCursor_t::LVL_SELECT /*|| frmLevelEditor::chkAlign.Value == 0*/)
    {
        EditorCursor.Location.X = double(X) - vScreen[A].X;
        EditorCursor.Location.Y = double(Y) - vScreen[A].Y;
        PositionCursor();
    }
    else
    {
        if(MagicHand)
        {
            if(std::fmod((vScreen[A].Y + 8), 32) != 0.0)
                vScreen[A].Y = static_cast<int>(floor(static_cast<double>(vScreen[A].Y / 32))) * 32 - 8;
            if(std::fmod(vScreen[A].X, 32) != 0.0)
                vScreen[A].X = static_cast<int>(floor(static_cast<double>(vScreen[A].X / 32))) * 32;
        }

        // 16x16 alignment
        if(
            (EditorCursor.Mode == OptCursor_t::LVL_BLOCKS &&
            (EditorCursor.Block.Type == 534 || EditorCursor.Block.Type == 535 ||
             EditorCursor.Block.Type == 536 || EditorCursor.Block.Type == 537)) ||

             EditorCursor.Mode == OptCursor_t::LVL_WARPS ||

            (EditorCursor.Mode == OptCursor_t::LVL_NPCS &&
             EditorCursor.NPC.Type == 52) ||

            (EditorCursor.Mode == OptCursor_t::LVL_BGOS &&
             (EditorCursor.Background.Type == 71 ||
              EditorCursor.Background.Type == 72 ||
              EditorCursor.Background.Type == 73 ||
              EditorCursor.Background.Type == 141 ||
              EditorCursor.Background.Type == 74 ||
              EditorCursor.Background.Type == 70 ||
              EditorCursor.Background.Type == 100)) ||

              (EditorCursor.Mode == OptCursor_t::LVL_NPCS &&
               (EditorCursor.NPC.Generator ||
                EditorCursor.NPC.Type == 209 || EditorCursor.NPC.Type == 256 ||
                EditorCursor.NPC.Type == 257 || EditorCursor.NPC.Type == 260))
        )
        {
            if(!(ffEqual(EditorCursor.Location.X, double(std::floor(X / 16)) * 16 - vScreen[A].X) &&
                 ffEqual(EditorCursor.Location.Y + 8, double(std::floor(Y / 16)) * 16 - vScreen[A].Y)) )
            {
                EditorCursor.Location.X = double(std::floor(X / 16)) * 16 - vScreen[A].X;
                EditorCursor.Location.Y = double(std::floor(Y / 16)) * 16 - vScreen[A].Y;
                EditorCursor.Location.Y -= 8;
                PositionCursor();
            }
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_PLAYERSTART)
        {
            if(!(EditorCursor.Location.X == static_cast<float>(floor(X / 8)) * 8 - vScreen[A].X && EditorCursor.Location.Y + 8 == static_cast<float>(floor(Y / 8)) * 8 - vScreen[A].Y))
            {
                EditorCursor.Location.X = static_cast<float>(floor(X / 8)) * 8 - vScreen[A].X;
                EditorCursor.Location.Y = static_cast<float>(floor(Y / 8)) * 8 - vScreen[A].Y;
                EditorCursor.Location.Y -= 8;
                PositionCursor();
            }
        }
        else if(EditorCursor.Mode == OptCursor_t::WLD_SCENES)
        {
            EditorCursor.Location.X = double(std::floor(X / 16)) * 16 - vScreen[A].X;
            EditorCursor.Location.Y = double(std::floor(Y / 16)) * 16 - vScreen[A].Y;
            EditorCursor.Location.Y -= 8;
            PositionCursor();
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_WATER)
        {
            EditorCursor.Location.X = double(std::floor(X / 16)) * 16 - vScreen[A].X;
            EditorCursor.Location.Y = double(std::floor(Y / 16)) * 16 - vScreen[A].Y;
            EditorCursor.Location.Y -= 8;
            PositionCursor();
        }
        else // Everything also align as 32x32
        {
            EditorCursor.Location.X = double(std::floor(X / 32)) * 32 - vScreen[A].X;
            EditorCursor.Location.Y = double(std::floor(Y / 32)) * 32 - vScreen[A].Y;
            EditorCursor.Location.Y -= 8;
            PositionCursor();
        }
    }
//    if(nPlay.Online == true && nCur == true)
//    {
//        if(nPlay.Mode == 0)
//            Netplay::sendData "f" + std::to_string(X) - vScreenX(A) + "|" + std::to_string(Y) - vScreenY(A); // Netplay
//        else
//        {
//            SetCursor();
//            Netplay::sendData std::string("f") + "0|" + std::to_string(X) - vScreenX(A) + "|" + std::to_string(Y) - vScreenY(A); // Netplay
//        }
//    }

    // translate from layer coordinates to screen coordinates
    if(MagicHand && EditorCursor.Layer != LAYER_NONE)
    {
        EditorCursor.Location.X += Layer[EditorCursor.Layer].OffsetX;
        EditorCursor.Location.Y += Layer[EditorCursor.Layer].OffsetY;
    }
}

void ResetNPC(NPCID A)
{
    NPC_t blankNPC;
    NPC[0] = blankNPC;
    NPC[0].Frame = EditorNPCFrame(A, EditorCursor.NPC.Direction);
    EditorCursor.NPC.Frame = NPC[0].Frame;
    EditorCursor.NPC.FrameCount = NPC[0].FrameCount;
    NPC[0].Frame = 0;
}
