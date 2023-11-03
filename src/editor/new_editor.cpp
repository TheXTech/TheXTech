/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <algorithm>

#include <PGE_File_Formats/file_formats.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"
#include <Integrator/integrator.h>
#include <fontman/font_manager.h>

#include "core/render.h"

#include "../globals.h"
#include "../gfx.h"
#include "../layers.h"
#include "../graphics.h"
#include "../sound.h"
#include "../main/level_file.h"
#include "../main/world_file.h"
#include "../game_main.h"
#include "main/game_globals.h"
#include "main/world_globals.h"

#include "config.h"
#include "npc_id.h"
#include "npc_special_data.h"
#include "controls.h"

#include "editor.h"
#include "editor/new_editor.h"
#include "editor/write_level.h"
#include "editor/write_world.h"

#include "editor/magic_block.h"
#include "editor/editor_custom.h"
#include "editor/editor_strings.h"

#include "main/menu_main.h"
#include "main/screen_textentry.h"
#include "main/game_info.h"
#include "main/translate.h"

template<typename... Args>
inline void SuperPrintR(EditorScreen::CallMode mode, Args... args)
{
    if(mode == EditorScreen::CallMode::Render)
        SuperPrint(args...);
}

template<typename... Args>
inline void SuperPrintRightR(EditorScreen::CallMode mode, Args... args)
{
    if(mode == EditorScreen::CallMode::Render)
        SuperPrintRightAlign(args...);
}

template<typename... Args>
inline void SuperPrintCenterR(EditorScreen::CallMode mode, Args... args)
{
    if(mode == EditorScreen::CallMode::Render)
        SuperPrintCenter(args...);
}

inline PGE_Size FontManager_printTextOptiPxR(EditorScreen::CallMode mode,
                                             std::string text,
                                             int x, int y,
                                             size_t max_pixels_lenght,
                                             int font = FontManager::DefaultRaster,
                                             float Red = 1.0, float Green = 1.0, float Blue = 1.0, float Alpha = 1.0,
                                             uint32_t ttf_FontSize = 14)
{
    if(mode == EditorScreen::CallMode::Render)
        return FontManager::printTextOptiPx(text, x, y,
                                            max_pixels_lenght,
                                            font, Red, Green, Blue, Alpha, ttf_FontSize);
    else
        return FontManager::optimizeTextPx(text, max_pixels_lenght, font, ttf_FontSize);
}

constexpr auto LESet_Nothing = EventSection_t::LESet_Nothing;
constexpr auto LESet_ResetDefault = EventSection_t::LESet_ResetDefault;

constexpr int e_ScreenW = 640;
constexpr int e_ScreenH = 480;

static const std::vector<std::string> c_musicFormats =
{
    // MPEG 1 Layer III (LibMAD)
    ".mp3",

    // OGG Vorbis, FLAC, amd Opus (LibOGG, LibVorbis, LibFLAC, libOpus)
    ".ogg", ".flac", ".opus",

    // Uncompressed audio data
    ".wav", ".aiff",

    // MIDI
    ".mid", ".midi", ".rmi", ".mus", ".kar", ".xmi", ".cmf",

    // Id Music File (OPL2 raw) / Imago Orpheus (Tracker music)
    ".imf", ".wlf",

    // Modules (Tracker music)
    ".mod", ".it", ".s3m", ".669", ".med", ".xm", ".amf",
    ".apun", ".dsm", ".far", ".gdm", ".mtm",
    ".okt", ".stm", ".stx", ".ult", ".uni", ".mptm",

    // GAME EMU (Chiptunes)
    ".ay", ".gbs", ".gym", ".hes", ".kss", ".nsf",
    ".nsfe", ".sap", ".spc", ".vgm", ".vgz",

    // PXTONE
    ".pttune", ".ptcop"
};

int e_CursorX, e_CursorY;

void DisableCursorNew()
{
    EditorCursor.Location.X = vScreen[1].X - 800;
    EditorCursor.X = float(vScreen[1].X - 800);
    EditorCursor.Location.Y = vScreen[1].Y - 600;
    EditorCursor.Y = float(vScreen[1].Y - 600);
    HasCursor = false;

    e_CursorX = -50;
    e_CursorY = -50;
}

// static const std::vector<std::string> list_backgrounds_names = {"None", "Set 1", "Underground", "Night", "Night 2", "Overworld", "Castle", "Mushrooms", "Desert", "", "Set 2", "Trees", "Underground", "Castle", "Clouds", "Night - Hills", "Night - Desert", "Cliff", "Warehouse", "Dungeon", "Set 3", "Blocks", "Hills", "Dungeon", "Pipes", "Bonus", "Clouds", "Desert", "Dungeon 2", "Ship", "Forest", "Battle", "Waterfall", "Tanks", "Final Boss", "Shroom Dealer", "Castle", "Snow Trees", "Clouds 2", "Snow Hills", "Cave", "Cave 2", "Underwater", "World", "Trees", "Mansion", "Forest", "Bonus", "Night", "Cave", "Clouds", "Hills", "Hills 2", "Hills 4", "Hills 3", "Castle", "Castle 2", "Underwater", "Desert Night", "", "Misc.", "Space Base", "Space Ship", "Space Swamp", "Space Crater", "Secret Mine"};
// static const std::vector<int16_t> list_backgrounds_indices = {0, -1, 7, 8, 9, 10, 41, 50, 51, -1, -1, 5, 25, 44, 48, 49, 52, 53, 54, 57, -1, 1, 2, 3, 4, 6, 13, 14, 15, 17, 20, 21, 22, 23, 24, 26, 27, 35, 36, 37, 38, 39, 56, -1, 12, 18, 19, 28, 29, 30, 31, 11, 32, 33, 34, 42, 43, 55, 58, -1, -1, 47, 46, 45, 16, 40};

// static const std::vector<std::string> list_music_names = {"None", "Custom", "Set 1", "Overworld", "Underground", "Dungeon", "Water", "Set 2", "Overworld", "Underground", "Boss", "Final Boss", "Set 3", "Overworld", "Sky", "Underground", "Dungeon", "Water", "Roaming Enemy", "Boss", "World", "Overworld", "Mansion", "Sky", "Cave", "Dungeon", "Water", "Boss", "RPG", "Bachelor Pad", "Town", "Forest", "Seaside", "Pond", "Clouds", "Battle", "64", "Main Theme", "Cave", "Snow", "Desert", "Water", "Castle", "Boss", "Fight", "Knight", "Underground", "Temple", "Steampunk", "Pinball", "Space", "Red Swamp", "Crater", "Item Room", "Final Boss", "Remake", "Misc.", "Jungle Village", "Ice Mountain", "Title Theme", "Beach", "Fusion Reactor", "Bouncy Race", "Remake", "Heroic Woods", "Cornered!"};
// static const std::vector<int16_t> list_music_indices = {0, 24, -1, 9, 7, 42, 46, -1, 5, 25, 15, 43, -1, 1, 2, 4, 3, 47, 54, 6, -1, 10, 17, 28, 29, 41, 48, 51, -1, 30, 34, 16, 31, 32, 33, 21, -1, 27, 50, 35, 14, 49, 26, 36, -1, 40, 52, 39, 19, 53, -1, 11, 12, 44, 45, 22, -1, 38, 37, 55, 18, 20, 56, 13, 23, 8};

// static const std::vector<std::string> list_world_music_names = {"None", "SET 3", "World 1", "World 2", "World 3", "World 4", "World 5", "World 6", "World 7", "World 8", "WORLD", "Theme", "Cave", "Island", "Forest", "Dungeon", "Sky", "Special", "NEW", "Theme"};
// static const std::vector<int16_t> list_world_music_indices = {0, -1, 1, 6, 8, 2, 11, 10, 3, 9, -1, 4, 16, 15, 7, 13, 14, 12, -1, 5};

// static const std::array<std::string, 10> list_level_exit_names = {"ANY", "NONE", "ROULETTE", "? ORB", "LEAVE", "KEYHOLE", "CLEAR ORB", "WARP", "STAR", "BAR END"};

const char* e_tooltip = nullptr;

void EditorScreen::EnsureWorld()
{
    if(WorldEditor) return;
    ClearLevel();
    WorldEditor = true;
}

void EditorScreen::EnsureLevel()
{
    if(!WorldEditor) return;
    ClearWorld();
    WorldEditor = false;
}

void EditorScreen::ResetCursor()
{
    EditorCursor.Mode = OptCursor_t::LVL_SELECT;

    EditorCursor.Block = Block_t();
    EditorCursor.Block.Type = 1;
    EditorCursor.Background = Background_t();
    EditorCursor.Background.Type = 1;
    EditorCursor.NPC = NPC_t();
    EditorCursor.NPC.Type = 1;
    EditorCursor.NPC.Direction = -1;
    EditorCursor.Water = Water_t();
    EditorCursor.Warp = Warp_t();
    EditorCursor.Warp.WarpNPC = true;
    EditorCursor.Warp.Direction = 3;
    EditorCursor.Warp.Direction2 = 3;
    EditorCursor.Warp.MapX = -1;
    EditorCursor.Warp.MapY = -1;
    EditorCursor.Warp.Effect = 1;

    EditorCursor.Tile = Tile_t();
    EditorCursor.Tile.Type = 1;
    EditorCursor.Scene = Scene_t();
    EditorCursor.Scene.Type = 1;
    EditorCursor.WorldLevel = WorldLevel_t();
    EditorCursor.WorldLevel.Type = 1;
    EditorCursor.WorldLevel.WarpX = -1;
    EditorCursor.WorldLevel.WarpY = -1;
    EditorCursor.WorldPath = WorldPath_t();
    EditorCursor.WorldPath.Type = 1;
    EditorCursor.WorldMusic = WorldMusic_t();

    m_special_page = SPECIAL_PAGE_NONE;
    m_special_subpage = 0;
    m_NPC_page = 0;
    m_Block_page = 0;
    m_BGO_page = 0;
    m_Warp_page = WARP_PAGE_MAIN;
    m_layers_page = 0;
    m_events_page = 0;
    m_sounds_page = 0;
    m_tile_page = 0;
    m_music_page = 0;
    m_background_page = 0;
    m_current_event = 0;

    MagicBlock::enabled = false;

    if(testStartWarp > numWarps && LevelEditor)
        testStartWarp = numWarps;

    FocusNPC();
    FocusBlock();
    FocusBGO();
    FocusTile();
}

bool AllowBubble()
{
    int type;
    if(EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
        || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)
        type = EditorCursor.NPC.Special;
    else
        type = EditorCursor.NPC.Type;
    if(type == 134) return true;
    if(NPCHeight[type] > 36 || NPCWidth[type] > 36
        || NPCWidthGFX[type] > 36 || NPCHeightGFX[type] > 36)
    {
        int W = NPCWidth[type];
        int H = NPCHeight[type];
        if(NPCWidthGFX[type] > W) W = NPCWidthGFX[type];
        if(NPCHeightGFX[type] > H) H = NPCHeightGFX[type];
        if((W <= 32 && H <= 54) || (H <= 32 && W <= 54))
            return true;
        else
            return false;
    }
    else
        return true;
}

void SetEditorNPCType(int type)
{
    int prev_type;

    if(EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
        || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)
    {
        prev_type = EditorCursor.NPC.Special;
        EditorCursor.NPC.Special = type;
    }
    else
    {
        prev_type = EditorCursor.NPC.Type;
        EditorCursor.NPC.Type = type;

        // can't have a murderous default, reset to 1 for ParaTroopas
        if(NPCIsAParaTroopa[type] && !NPCIsAParaTroopa[prev_type])
            EditorCursor.NPC.Special = 1;

        // reset special for NPCs that don't allow it
        if(!(NPCIsCheep[type] || NPCIsAParaTroopa[type] || type == NPCID_FIRE_CHAIN))
            EditorCursor.NPC.Special = 0;

        // reset special if it's out of range
        if(!(type == NPCID_FIRE_CHAIN) && EditorCursor.NPC.Special > 5)
            EditorCursor.NPC.Special = 0;
    }

    // reset Special2 for NPCs that don't allow it
    if(!(EditorCursor.NPC.Type == NPCID_MAGIC_DOOR || EditorCursor.NPC.Type == NPCID_DOOR_MAKER
        || (EditorCursor.NPC.Type == NPCID_ITEM_BURIED && EditorCursor.NPC.Special == NPCID_DOOR_MAKER)))
    {
        EditorCursor.NPC.Special2 = 0;
    }

    // reset legacy for the NPCs that don't allow it
    if(!(type == NPCID_MINIBOSS || type == NPCID_SPIT_BOSS || type == NPCID_VILLAIN_S3))
        EditorCursor.NPC.Legacy = false;

    // reset Variant data across different NPCs
    if(find_Variant_Data(prev_type) != find_Variant_Data(type))
    {
        if(FileFormat == FileFormats::LVL_PGEX)
        {
            EditorCursor.NPC.Variant = find_modern_Variant(type);
        }
        else
            EditorCursor.NPC.Variant = 0;
    }

    // reset Variant data for NPCs that don't support it
    if(find_Variant_Data(type) == nullptr && !(type == NPCID_STAR_COLLECT || type == NPCID_STAR_EXIT))
        EditorCursor.NPC.Variant = 0;

    // turn into new type if can't be in bubble anymore
    if(EditorCursor.NPC.Type == 283 && !AllowBubble())
    {
        EditorCursor.NPC.Type = type;
        EditorCursor.NPC.Special = 0;
    }

    // force a direction if they don't allow neutral direction (conveyers, moving platform blocks)
    if(EditorCursor.NPC.Direction == 0 && (type == 57 || type == 60 || type == 62 || type == 64 || type == 66))
        EditorCursor.NPC.Direction = -1;

    ResetNPC(type);
}

void SetEditorBlockType(int type)
{
    if(BlockIsSizable[type])
    {
        if(EditorCursor.Block.Location.Width < 64.)
            EditorCursor.Block.Location.Width = 64.;
        if(EditorCursor.Block.Location.Height < 64.)
            EditorCursor.Block.Location.Height = 64.;
    }
    else
    {
        EditorCursor.Block.Location.Width = 0.;
        EditorCursor.Block.Location.Height = 0.;
    }

    if(type == 5 || type == 88 || type == 193 || type == 224)
    {
        if(EditorCursor.Block.Special == 0)
            EditorCursor.Block.Special = 1;
    }
    else if(type != 60 && type != 188 && type != 4 && type != 226 && type != 55 && type != 159 && type != 226
        && type != 55 && type != 90 && type != 170 && type != 171 && type != 172 && type != 173
        && type != 174 && type != 175 && type != 176 && type != 177 && type != 178 && type != 179 && type != 180
        && type != 181 && type != 622 && type != 623 && type != 624 && type != 625 && type != 626 && type != 627
        && type != 628 && type != 629 && type != 631 && type != 632)
    {
        EditorCursor.Block.Special = 0;
    }

    // only update slipperiness if it is currently what you would expect.
    if(EditorCursor.Block.Slippy ==
        (EditorCursor.Block.Type == 189 || EditorCursor.Block.Type == 190 || EditorCursor.Block.Type == 191
            || EditorCursor.Block.Type == 270 || EditorCursor.Block.Type == 271 || EditorCursor.Block.Type == 272
            || EditorCursor.Block.Type == 620 || EditorCursor.Block.Type == 621 || EditorCursor.Block.Type == 633
            || EditorCursor.Block.Type == 634 || EditorCursor.Block.Type == 241 || EditorCursor.Block.Type == 242))
    {
        EditorCursor.Block.Slippy = (type == 189 || type == 190 || type == 191 || type == 270 || type == 271 || type == 272 || type == 620 || type == 621 || type == 633 || type == 634 || type == 241 || type == 242);
    }

    EditorCursor.Block.Type = type;
}

void EditorScreen::FocusNPC()
{
    int type;
    if(m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS)
        type = EditorCursor.Block.Special - 1000;
    else if(EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
        || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)
        type = EditorCursor.NPC.Special;
    else
        type = EditorCursor.NPC.Type;
    if(m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS && (type == 10 || type == 9 || type == 90 || type == 14 || type == 264 || type == 34 || type == 169 || type == 170 || type == 226 || type == 287 || type == 33 || type == 185 || type == 187 || type == 183 || type == 188 || type == 277 || type == 95 || type == 31 || type == 227 || type == 88 || type == 184 || type == 186 || type == 182 || type == 153 || type == 138 || type == 249 || type == 134 || type == 241 || type == 240 || type == 152 || type == 250 || type == 254 || type == 251 || type == 252 || type == 253))
        m_NPC_page = -1;
    else if(!EditorCustom::npc_families.empty() && type >= 1 && type <= maxNPCType && EditorCustom::npc_family_by_type[type - 1] != EditorCustom::FAMILY_NONE)
        m_NPC_page = EditorCustom::npc_families[EditorCustom::npc_family_by_type[type - 1]].page;
    else if(m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS)
        m_NPC_page = -1;
    else
        m_NPC_page = 0;
}

void EditorScreen::FocusBlock()
{
    int type = EditorCursor.Block.Type;
    if(!EditorCustom::block_families.empty() && type >= 1 && type <= maxBlockType && EditorCustom::block_family_by_type[type - 1] != EditorCustom::FAMILY_NONE)
        m_Block_page = EditorCustom::block_families[EditorCustom::block_family_by_type[type - 1]].page;
    else
        m_Block_page = 0;
}

void EditorScreen::FocusBGO()
{
    int type = EditorCursor.Background.Type;
    if(!EditorCustom::bgo_families.empty() && type >= 1 && type <= maxBackgroundType && EditorCustom::bgo_family_by_type[type - 1] != EditorCustom::FAMILY_NONE)
        m_BGO_page = EditorCustom::bgo_families[EditorCustom::bgo_family_by_type[type - 1]].page;
    else
        m_BGO_page = 0;
}

void EditorScreen::FocusTile()
{
    int type = EditorCursor.Tile.Type;
    if(!EditorCustom::tile_families.empty() && type >= 1 && type <= maxTileType && EditorCustom::tile_family_by_type[type - 1] != EditorCustom::FAMILY_NONE)
        m_tile_page = EditorCustom::tile_families[EditorCustom::tile_family_by_type[type - 1]].page;
    else
        m_tile_page = 0;
}

bool EditorScreen::UpdateButton(CallMode mode, int x, int y, StdPicture &im, bool sel,
    int src_x, int src_y, int src_w, int src_h, const char* tooltip)
{
    // the button is 32x32 and outlined by a 36x36 box
    bool coll = false;
    if(e_CursorX >= x && e_CursorX < x + 32
        && e_CursorY >= y && e_CursorY < y + 32)
    {
        coll = true;
    }

    // just do the simple logic if in logic mode
    if(mode == CallMode::Logic)
    {
        bool ret = (MenuMouseRelease && coll);
        if(ret)
            PlaySound(SFX_Saw);
        return ret;
    }

    // otherwise, fully render!
    if(coll && tooltip)
        e_tooltip = tooltip;

    // outline:
    if(sel)
    {
        if(coll && SharedCursor.Primary)
            XRender::renderRect(x - 2, y - 2, 36, 36, 0.f, 0.5f, 0.f, 1.0f, true);
        else
            XRender::renderRect(x - 2, y - 2, 36, 36, 0.f, 1.0f, 0.f, 1.0f, true);
    }
    else if(coll && SharedCursor.Primary)
        XRender::renderRect(x - 2, y - 2, 36, 36, 0.f, 0.f, 0.f, 1.0f, true);
    else
        XRender::renderRect(x - 2, y - 2, 36, 36, 1.f, 1.f, 1.f, 0.5f, true);

    // background:
    if(SharedCursor.Primary && coll)
        XRender::renderRect(x, y, 32, 32, 0.2f, 0.2f, 0.2f, true);
    else
        XRender::renderRect(x, y, 32, 32, 0.5f, 0.5f, 0.5f, true);

    // scale and center image
    int dst_x, dst_y, dst_h, dst_w;
    if(src_w > 32 && src_w >= src_h)
    {
        dst_h = (src_h * 32) / src_w;
        dst_w = 32;
    }
    else if(src_h > 32 && src_h > src_w)
    {
        dst_w = (src_w * 32) / src_h;
        dst_h = 32;
    }
    else
    {
        dst_w = src_w;
        dst_h = src_h;
    }
    dst_x = x + 16 - dst_w / 2;
    dst_y = y + 16 - dst_h / 2;

    if(dst_w > src_w)
        D_pLogCriticalNA("Editor upscaling image (should never happen)");


    XRender::renderTextureScaleEx((double)dst_x, (double)dst_y, (double)dst_w, (double)dst_h, im, src_x, src_y, src_w, src_h);
    return false;
}

bool EditorScreen::UpdateCheckBox(CallMode mode, int x, int y, bool sel, const char* tooltip)
{
    if(sel)
        return this->UpdateButton(mode, x, y, GFX.EIcons, sel, 0, 32*Icon::check, 32, 32, tooltip);
    else
        return this->UpdateButton(mode, x, y, GFX.EIcons, sel, 0, 0, 1, 1, tooltip);
}

bool EditorScreen::UpdateNPCButton(CallMode mode, int x, int y, int type, bool sel)
{
    int draw_width, draw_height;
    if(NPCWidthGFX[type] == 0)
    {
        draw_width = NPCWidth[type];
        draw_height = NPCHeight[type];
    }
    else
    {
        draw_width = NPCWidthGFX[type];
        draw_height = NPCHeightGFX[type];
    }

    return UpdateButton(mode, x, y, GFXNPC[type], sel, 0, 0, draw_width, draw_height);
}

void EditorScreen::UpdateNPC(CallMode mode, int x, int y, int type)
{
    if((type < 1) || (type >= maxNPCType))
    {
        pLogWarning("Attempted to render NPC type %d, but the max NPC type is %d!", type, maxNPCType);
        return;
    }
    if(m_special_page != SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        bool sel = (EditorCursor.NPC.Type == type ||
            (EditorCursor.NPC.Special == type &&
                (EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
                    || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)));
        if(UpdateNPCButton(mode, x, y, type, sel) && !sel)
            SetEditorNPCType(type);
    }
    else
    {
        bool sel = (EditorCursor.Block.Special == type + 1000);
        if(UpdateNPCButton(mode, x, y, type, sel) && !sel)
            EditorCursor.Block.Special = type + 1000;
    }
}

void EditorScreen::UpdateNPCGrid(CallMode mode, int x, int y, const int* types, int n_npcs, int n_cols)
{
    for(int i = 0; i < n_npcs; i ++)
    {
        int type = types[i];
        if(!type)
            continue;

        int row = i / n_cols;
        int col = i % n_cols;
        UpdateNPC(mode, x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateNPCScreen(CallMode mode)
{
    // NPC GUI
    if(mode == CallMode::Render)
    {
        XRender::renderRect(e_ScreenW - 200, 40, 200, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);
        XRender::renderRect(0, 40, 40, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);
        XRender::renderRect(38, 40, 2, e_ScreenH - 40, 0.25f, 0.0f, 0.5f, 1.0f, true);
    }

    if(m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS && UpdateButton(mode, e_ScreenW - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_NONE;
    }

    // Page selector
    int last_category = -1;
    int index = 0;

    for(const EditorCustom::ItemPage_t& page : EditorCustom::npc_pages)
    {
        if(page.category != last_category)
        {
            last_category = page.category;

            if(mode == CallMode::Render && index != 0)
                XRender::renderRect(0, 40 + -2 + (40 * index), 40, 4, 0.25f, 0.0f, 0.5f, 1.0f, true);
        }

        index++;

        if(UpdateNPCButton(mode, 4, 4 + (40 * index), page.icon, m_NPC_page == index))
            m_NPC_page = index;
    }

    if(m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS && mode == CallMode::Render)
        XRender::renderRect(0, (40 * index) + 40 + -2, 40, 4, 0.25f, 0.0f, 0.5f, 1.0f, true);

    if(m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS && UpdateNPCButton(mode, 4, 4 + (40 * index) + 40, 10, m_NPC_page == -1))
        m_NPC_page = -1;


    if(m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS && mode == CallMode::Render)
    {
        FontManager::printTextOptiPx(g_editorStrings.pickBlockContents,
                                     e_ScreenW - 200, 90,
                                     230,
                                     FontManager::fontIdFromSmbxFont(3));
//        SuperPrint(g_editorStrings.pickBlockContents1, 3, e_ScreenW - 200, 90);
//        SuperPrint(g_editorStrings.pickBlockContents2, 3, e_ScreenW - 200, 110);
    }

    if(m_special_page != SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        // Containers
        SuperPrintCenterR(mode, g_editorStrings.npcInContainer, 3, e_ScreenW - 20, 40);
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 60 + 4, GFXNPC[NPCID_ITEM_BURIED], EditorCursor.NPC.Type == NPCID_ITEM_BURIED, 0, 0, NPCWidth[NPCID_ITEM_BURIED], NPCHeight[NPCID_ITEM_BURIED]))
        {
            if(EditorCursor.NPC.Type == NPCID_ITEM_BURIED)
            {
                EditorCursor.NPC.Type = EditorCursor.NPC.Special;
                EditorCursor.NPC.Special = 0;
            }
            else if(!(EditorCursor.NPC.Type == NPCID_ITEM_BURIED || EditorCursor.NPC.Type == NPCID_ITEM_POD
                || EditorCursor.NPC.Type == NPCID_ITEM_BUBBLE || EditorCursor.NPC.Type == NPCID_ITEM_THROWER))
            {
                EditorCursor.NPC.Special = EditorCursor.NPC.Type;
                EditorCursor.NPC.Type = NPCID_ITEM_BURIED;
            }
            else
            {
                EditorCursor.NPC.Type = NPCID_ITEM_BURIED;
            }
        }
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 100 + 4, GFXNPC[NPCID_ITEM_POD], EditorCursor.NPC.Type == NPCID_ITEM_POD))
        {
            if(EditorCursor.NPC.Type == NPCID_ITEM_POD)
            {
                EditorCursor.NPC.Type = EditorCursor.NPC.Special;
                EditorCursor.NPC.Special = 0;
            }
            else if(!(EditorCursor.NPC.Type == NPCID_ITEM_BURIED || EditorCursor.NPC.Type == NPCID_ITEM_POD
                || EditorCursor.NPC.Type == NPCID_ITEM_BUBBLE || EditorCursor.NPC.Type == NPCID_ITEM_THROWER))
            {
                EditorCursor.NPC.Special = EditorCursor.NPC.Type;
                EditorCursor.NPC.Type = NPCID_ITEM_POD;
            }
            else
            {
                EditorCursor.NPC.Type = NPCID_ITEM_POD;
            }
        }
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 140 + 4, GFXNPC[NPCID_ITEM_THROWER], EditorCursor.NPC.Type == NPCID_ITEM_THROWER, 0, 0, NPCWidthGFX[NPCID_ITEM_THROWER], NPCHeightGFX[NPCID_ITEM_THROWER]))
        {
            if(EditorCursor.NPC.Type == NPCID_ITEM_THROWER)
            {
                EditorCursor.NPC.Type = EditorCursor.NPC.Special;
                EditorCursor.NPC.Special = 0;
            }
            else if(!(EditorCursor.NPC.Type == NPCID_ITEM_BURIED || EditorCursor.NPC.Type == NPCID_ITEM_POD
                || EditorCursor.NPC.Type == NPCID_ITEM_BUBBLE || EditorCursor.NPC.Type == NPCID_ITEM_THROWER))
            {
                EditorCursor.NPC.Special = EditorCursor.NPC.Type;
                EditorCursor.NPC.Type = NPCID_ITEM_THROWER;
            }
            else
            {
                EditorCursor.NPC.Type = NPCID_ITEM_THROWER;
            }
        }
        if(AllowBubble())
        {
            if(UpdateButton(mode, e_ScreenW - 40 + 4, 180 + 4, GFXNPC[NPCID_ITEM_BUBBLE], EditorCursor.NPC.Type == NPCID_ITEM_BUBBLE, 0, 0, NPCWidthGFX[NPCID_ITEM_BUBBLE], NPCHeightGFX[NPCID_ITEM_BUBBLE]))
            {
                if(EditorCursor.NPC.Type == NPCID_ITEM_BUBBLE)
                {
                    EditorCursor.NPC.Type = EditorCursor.NPC.Special;
                    EditorCursor.NPC.Special = 0;
                }
                else if(!(EditorCursor.NPC.Type == NPCID_ITEM_BURIED || EditorCursor.NPC.Type == NPCID_ITEM_POD
                    || EditorCursor.NPC.Type == NPCID_ITEM_BUBBLE || EditorCursor.NPC.Type == NPCID_ITEM_THROWER))
                {
                    EditorCursor.NPC.Special = EditorCursor.NPC.Type;
                    EditorCursor.NPC.Type = NPCID_ITEM_BUBBLE;
                }
                else
                {
                    EditorCursor.NPC.Type = NPCID_ITEM_BUBBLE;
                }
            }
        }

        // Various properties that depend on type, get the real one here:
        int type;
        if(EditorCursor.NPC.Type == NPCID_ITEM_BURIED || EditorCursor.NPC.Type == NPCID_ITEM_POD
            || EditorCursor.NPC.Type == NPCID_ITEM_BUBBLE || EditorCursor.NPC.Type == NPCID_ITEM_THROWER)
        {
            type = EditorCursor.NPC.Special;
        }
        else
        {
            type = EditorCursor.NPC.Type;
        }

        // Direction
        Icon::Icons dir_neg_icon = Icon::left;
        Icon::Icons dir_pos_icon = Icon::right;

        bool show_random = true;
        if(type == NPCID_CONVEYOR || type == NPCID_YEL_PLATFORM || type == NPCID_BLU_PLATFORM || type == NPCID_GRN_PLATFORM || type == NPCID_RED_PLATFORM)
            show_random = false;

        // "direction" means different things for these types
        if(mode == CallMode::Render)
        {
            if(type == NPCID_YEL_PLATFORM || type == NPCID_BLU_PLATFORM || type == NPCID_GRN_PLATFORM || type == NPCID_RED_PLATFORM || type == NPCID_PLATFORM_S3)
            {
                SuperPrintCenter(g_editorStrings.npcPropertyActive, 3, e_ScreenW - 120, 40);
                dir_neg_icon = Icon::x;
                dir_pos_icon = Icon::check;
            }
            else if(type == NPCID_FIRE_DISK || type == NPCID_FIRE_CHAIN)
            {
                SuperPrintCenter(g_editorStrings.npcPropertyAttachSurface, 3, e_ScreenW - 120, 40);
                dir_neg_icon = Icon::bottom;
                dir_pos_icon = Icon::top;
            }
            else
            {
                SuperPrintCenter(g_editorStrings.npcPropertyFacing, 3, e_ScreenW - 120, 40);
                if(type == NPCID_PLATFORM_S1 || (NPCIsAParaTroopa[type] && EditorCursor.NPC.Special == 3))
                {
                    dir_neg_icon = Icon::up;
                    dir_pos_icon = Icon::down;
                }
            }
        }

        // show and apply direction
        if(UpdateButton(mode, e_ScreenW - 180 + 4, 60 + 4, GFX.EIcons, EditorCursor.NPC.Direction == -1, 0, 32 * dir_neg_icon, 32, 32))
            EditorCursor.NPC.Direction = -1;
        if(show_random && UpdateButton(mode, e_ScreenW - 140 + 4, 60 + 4, GFX.EIcons, EditorCursor.NPC.Direction == 0, 0, 32 * Icon::unk, 32, 32))
            EditorCursor.NPC.Direction = 0;
        if(UpdateButton(mode, e_ScreenW - 100 + 4, 60 + 4, GFX.EIcons, EditorCursor.NPC.Direction == 1, 0, 32 * dir_pos_icon, 32, 32))
            EditorCursor.NPC.Direction = 1;

        // Inert ("nice") and Stuck ("stop")
        // The sign (NPC ID 151) is always nice.
        if(type == NPCID_SIGN)
            EditorCursor.NPC.Inert = true;
        else
        {
            SuperPrintRightR(mode, g_editorStrings.npcInertNice, 3, e_ScreenW - 130, 100);
            if(UpdateCheckBox(mode, e_ScreenW - 160 + 4, 120 + 4, EditorCursor.NPC.Inert))
                EditorCursor.NPC.Inert = !EditorCursor.NPC.Inert;
        }

        SuperPrintR(mode, g_editorStrings.npcStuckStop, 3, e_ScreenW - 110, 100);
        if(UpdateCheckBox(mode, e_ScreenW - 120 + 4, 120 + 4, EditorCursor.NPC.Stuck))
            EditorCursor.NPC.Stuck = !EditorCursor.NPC.Stuck;

        // Text
        if(EditorCursor.NPC.Inert)
        {
            MessageText = GetS(EditorCursor.NPC.Text);
            BuildUTF8CharMap(MessageText, MessageTextMap);
            SuperPrintRightR(mode, g_editorStrings.wordText, 3, e_ScreenW - 130, 160);
            if(UpdateButton(mode, e_ScreenW - 160 + 4, 180 + 4, GFX.EIcons, EditorCursor.NPC.Text != STRINGINDEX_NONE, 0, 32*Icon::pencil, 32, 32))
            {
                DisableCursorNew();

                std::string&& prompt = fmt::format_ne(g_editorStrings.phraseTextOf, g_editorStrings.wordNPCGenitive);
                SetS(EditorCursor.NPC.Text, TextEntryScreen::Run(prompt, GetS(EditorCursor.NPC.Text)));

                MouseMove(SharedCursor.X, SharedCursor.Y);
            }
        }

        // Generator
        SuperPrintR(mode, g_editorStrings.npcAbbrevGen, 3, e_ScreenW - 110, 160);
        if(UpdateButton(mode, e_ScreenW - 120 + 4, 180 + 4, GFX.EIcons, EditorCursor.NPC.Generator, 0, 32*Icon::subscreen, 32, 32))
            m_NPC_page = -2;

        // Behavior
        if(NPCIsAParaTroopa[EditorCursor.NPC.Type])
        {
            // Describe current AI if valid
            if(mode == CallMode::Render)
            {
                std::string ai_invalid;
                const std::string* ai_name = &ai_invalid;

                int index = (int)EditorCursor.NPC.Special;

                if(index >= 0 && index < 4)
                {
                    const std::string* map[] = {
                        &g_editorStrings.npcAiTarget,
                        &g_editorStrings.npcAiJump,
                        &g_editorStrings.npcAiLR,
                        &g_editorStrings.npcAiUD,
                    };
                    ai_name = map[index];
                }

                std::string&& ai_is = fmt::format_ne(g_editorStrings.npcAiIs, *ai_name);

                SuperPrint(ai_is, 3, e_ScreenW - 200, 220);
            }

            if(UpdateButton(mode, e_ScreenW - 200 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 1, 0, 32*Icon::hop, 32, 32))
                EditorCursor.NPC.Special = 1;
            if(UpdateButton(mode, e_ScreenW - 160 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 0, 0, 32*Icon::target, 32, 32))
                EditorCursor.NPC.Special = 0;
            if(UpdateButton(mode, e_ScreenW - 120 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 2, 0, 32*Icon::lr, 32, 32))
                EditorCursor.NPC.Special = 2;
            if(UpdateButton(mode, e_ScreenW - 80 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 3, 0, 32*Icon::ud, 32, 32))
                EditorCursor.NPC.Special = 3;
        }

        if(NPCIsCheep[EditorCursor.NPC.Type])
        {
            // Describe current AI if valid
            if(mode == CallMode::Render)
            {
                std::string ai_invalid;
                const std::string* ai_name = &ai_invalid;

                int index = (int)EditorCursor.NPC.Special;

                if(index >= 0 && index < 5)
                {
                    const std::string* map[] =
                    {
                        &g_editorStrings.npcAiSwim,
                        &g_editorStrings.npcAiJump,
                        &g_editorStrings.npcAiLeap,
                        &g_editorStrings.npcAiLR,
                        &g_editorStrings.npcAiUD,
                    };
                    ai_name = map[index];
                }

                std::string&& ai_is = fmt::format_ne(g_editorStrings.npcAiIs, *ai_name);

                SuperPrint(ai_is, 3, e_ScreenW - 200, 220);
            }

            if(UpdateButton(mode, e_ScreenW - 200 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 0, 0, 32*Icon::wave, 32, 32))
                EditorCursor.NPC.Special = 0;
            if(UpdateButton(mode, e_ScreenW - 160 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 1, 0, 32*Icon::hop, 32, 32))
                EditorCursor.NPC.Special = 1;
            if(UpdateButton(mode, e_ScreenW - 120 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 2, 0, 32*Icon::leap, 32, 32))
                EditorCursor.NPC.Special = 2;
            if(UpdateButton(mode, e_ScreenW - 80 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 3, 0, 32*Icon::lr, 32, 32))
                EditorCursor.NPC.Special = 3;
            if(UpdateButton(mode, e_ScreenW - 40 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Special == 4, 0, 32*Icon::ud, 32, 32))
                EditorCursor.NPC.Special = 4;
        }

        if(type == NPCID_MINIBOSS || type == NPCID_SPIT_BOSS || type == NPCID_VILLAIN_S3)
        {
            SuperPrintR(mode, g_editorStrings.npcUse1_0Ai, 3, e_ScreenW - 200, 220);
            if(UpdateButton(mode, e_ScreenW - 200 + 4, 240 + 4, GFX.EIcons, EditorCursor.NPC.Legacy, 0, 32*Icon::_10, 32, 32))
                EditorCursor.NPC.Legacy = true;
            if(UpdateButton(mode, e_ScreenW - 160 + 4, 240 + 4, GFX.EIcons, !EditorCursor.NPC.Legacy, 0, 32*Icon::_1x, 32, 32))
                EditorCursor.NPC.Legacy = false;
        }

        if(EditorCursor.NPC.Type == NPCID_FIRE_CHAIN)
        {
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.phraseRadiusIndex, (int)EditorCursor.NPC.Special), 3, e_ScreenW - 200, 220);

            if(EditorCursor.NPC.Special > 0 && UpdateButton(mode, e_ScreenW - 160 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
                EditorCursor.NPC.Special --;
            if(UpdateButton(mode, e_ScreenW - 120 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
                EditorCursor.NPC.Special ++;
        }

        if(EditorCursor.NPC.Type == NPCID_MAGIC_DOOR || EditorCursor.NPC.Type == NPCID_DOOR_MAKER
            || (EditorCursor.NPC.Type == NPCID_ITEM_BURIED && EditorCursor.NPC.Special == NPCID_DOOR_MAKER))
        {
            std::string&& dest_section = fmt::format_ne(g_editorStrings.phraseSectionIndex, (int)(EditorCursor.NPC.Special2) + 1);
            SuperPrintR(mode, dest_section, 3, e_ScreenW - 200, 220);

            if(EditorCursor.NPC.Special2 > 0 && UpdateButton(mode, e_ScreenW - 160 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
                EditorCursor.NPC.Special2 --;
            if(EditorCursor.NPC.Special2 < 20 && UpdateButton(mode, e_ScreenW - 120 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
                EditorCursor.NPC.Special2 ++;
        }

        // multistars
        if((type == NPCID_STAR_COLLECT || type == NPCID_STAR_EXIT) && FileFormat == FileFormats::LVL_PGEX)
        {
            std::string&& star_index = EditorCursor.NPC.Variant ? fmt::format_ne(g_editorStrings.phraseGenericIndex, (int)(EditorCursor.NPC.Variant)) : g_editorStrings.fileFormatLegacy;
            SuperPrintCenterR(mode, star_index, 3, e_ScreenW - 120, 220);

            if(EditorCursor.NPC.Variant > 0 && UpdateButton(mode, e_ScreenW - 160 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
                EditorCursor.NPC.Variant --;
            if(EditorCursor.NPC.Variant < 20 && UpdateButton(mode, e_ScreenW - 120 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
                EditorCursor.NPC.Variant ++;
        }

        const NPC_Variant_Data_t* data = find_Variant_Data(EditorCursor.NPC.Type);

        // special case for NPCID_VILLAIN_S3 since it also has the Legacy button
        if(EditorCursor.NPC.Type == NPCID_VILLAIN_S3 && FileFormat == FileFormats::LVL_PGEX)
        {
            if(UpdateButton(mode, e_ScreenW - 40 + 4, 240 + 4, GFXBlock[4], EditorCursor.NPC.Variant == 1,
                0, 32 * BlockFrame[4], 32, 32, g_editorStrings.npcTooltipExpandSection.c_str()))
            {
                if(EditorCursor.NPC.Variant == 0)
                    EditorCursor.NPC.Variant = 1;
                else
                    EditorCursor.NPC.Variant = 0;
            }
        }
        else if(data && FileFormat == FileFormats::LVL_PGEX)
        {
            int i;
            bool valid;

            i = data->find_current(EditorCursor.NPC.Variant);
            valid = data->strings[i] != nullptr;

            if(mode == CallMode::Render)
            {
                SuperPrint(g_editorStrings.npcCustomAi, 3, e_ScreenW - 180, 220);
                if(valid)
                    SuperPrint(data->strings[i], 3, e_ScreenW - 160, 242);
                else
                    SuperPrint(std::to_string(EditorCursor.NPC.Variant), 3, e_ScreenW - 160, 242);
            }

            // only show it if it will (i) reset, or (ii) have something to go to.
            // short-circuit evaluation keeps this from accessing outside of the valid range
            bool show_prev_button = (!valid || i != 0);
            bool show_next_button = (!valid || data->strings[i + 1]);
            if(show_prev_button && UpdateButton(mode, e_ScreenW - 200 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            {
                if(!valid)
                    EditorCursor.NPC.Variant = data->values[0];
                else
                    EditorCursor.NPC.Variant = data->values[i-1];
            }
            if(show_next_button && UpdateButton(mode, e_ScreenW - 40 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            {
                if(!valid)
                    EditorCursor.NPC.Variant = data->values[0];
                else
                    EditorCursor.NPC.Variant = data->values[i + 1];
            }
        }

        // Events
        if(mode == CallMode::Render)
        {
            SuperPrintRightAlign(g_editorStrings.eventsHeader, 3, e_ScreenW - 40, 294);
            SuperPrint(g_editorStrings.eventsLetterActivate   + GetE(EditorCursor.NPC.TriggerActivate), 3, e_ScreenW - 200, 320);
            SuperPrint(g_editorStrings.eventsLetterDeath      + GetE(EditorCursor.NPC.TriggerDeath), 3, e_ScreenW - 200, 340);
            SuperPrint(g_editorStrings.eventsLetterTalk       + GetE(EditorCursor.NPC.TriggerTalk), 3, e_ScreenW - 200, 360);
            SuperPrint(g_editorStrings.eventsLetterLayerClear + GetE(EditorCursor.NPC.TriggerLast), 3, e_ScreenW - 200, 380);
        }
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 280 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_OBJ_TRIGGERS;

        // Layers
        if(mode == CallMode::Render)
        {
            SuperPrintRightAlign(g_editorStrings.labelLayer, 3, e_ScreenW - 40, 414);
            if(EditorCursor.NPC.Layer == LAYER_NONE)
                SuperPrint(g_editorStrings.layersLayerDefault, 3, e_ScreenW - 200, 440);
            else
                SuperPrint(GetL(EditorCursor.NPC.Layer), 3, e_ScreenW - 200, 440);
            if(EditorCursor.NPC.AttLayer != LAYER_NONE && EditorCursor.NPC.AttLayer != LAYER_DEFAULT)
                SuperPrint(g_editorStrings.layersAbbrevAttLayer + GetL(EditorCursor.NPC.AttLayer), 3, e_ScreenW - 200, 460);
        }
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 400 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_OBJ_LAYER;
    }


    // NPC selector
    if(!EditorCustom::npc_pages.empty() && m_NPC_page > 0 && m_NPC_page <= (int)EditorCustom::npc_pages.size())
    {
        const EditorCustom::ItemPage_t& page = EditorCustom::npc_pages[m_NPC_page - 1];

        for(auto it = page.begin; it != page.end; ++it)
        {
            const EditorCustom::ItemFamily& family = **it;

            if(mode == CallMode::Render)
            {
                int pix_len = SuperTextPixLen(family.name, 3);

                if(40 + family.X * 40 + pix_len > e_ScreenW - 200)
                    SuperPrint(family.name, 3, e_ScreenW - 200 - 4 - pix_len, 40 + family.Y * 20);
                else
                    SuperPrint(family.name, 3, 40 + family.X * 40 + 4, 40 + family.Y * 20);
            }

            UpdateNPCGrid(mode, 40 + family.X * 40, 60 + family.Y * 20, family.layout_pod.types.data(), family.layout_pod.types.size(), family.layout_pod.cols);
        }
    }


    // GENERATOR SETTINGS SCREEN
    if(m_NPC_page == -2 && m_special_page != SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        SuperPrintR(mode, g_editorStrings.npcGenHeader, 3, 50, 44);

        if(UpdateButton(mode, e_ScreenW - 200 - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
            FocusNPC();

        SuperPrintR(mode, g_editorStrings.wordEnabled, 3, 50, 110);

        if(UpdateCheckBox(mode, 280 + 4, 100 + 4, EditorCursor.NPC.Generator))
        {
            EditorCursor.NPC.Generator = true;
            if(!EditorCursor.NPC.GeneratorDirection)
                EditorCursor.NPC.GeneratorDirection = 1;
            if(!EditorCursor.NPC.GeneratorEffect)
                EditorCursor.NPC.GeneratorEffect = 1;
            if(EditorCursor.NPC.GeneratorTimeMax < 1.f)
                EditorCursor.NPC.GeneratorTimeMax = 1.f;
        }

        if(UpdateButton(mode, 320 + 4, 100 + 4, GFX.EIcons, !EditorCursor.NPC.Generator, 0, 32*Icon::x, 32, 32))
            EditorCursor.NPC.Generator = false;

        if(EditorCursor.NPC.Generator)
        {
            // direction change
            SuperPrintR(mode, g_editorStrings.npcGenDirection, 3, 50, 150);

            if(UpdateButton(mode, 280 + 4, 140 + 4, GFX.EIcons, EditorCursor.NPC.GeneratorDirection == 1, 0, 32*Icon::up, 32, 32))
                EditorCursor.NPC.GeneratorDirection = 1;
            if(UpdateButton(mode, 320 + 4, 140 + 4, GFX.EIcons, EditorCursor.NPC.GeneratorDirection == 3, 0, 32*Icon::down, 32, 32))
                EditorCursor.NPC.GeneratorDirection = 3;
            if(UpdateButton(mode, 360 + 4, 140 + 4, GFX.EIcons, EditorCursor.NPC.GeneratorDirection == 2, 0, 32*Icon::left, 32, 32))
                EditorCursor.NPC.GeneratorDirection = 2;
            if(UpdateButton(mode, 400 + 4, 140 + 4, GFX.EIcons, EditorCursor.NPC.GeneratorDirection == 4, 0, 32*Icon::right, 32, 32))
                EditorCursor.NPC.GeneratorDirection = 4;

            // effect change
            const std::string& effect =
                   (EditorCursor.NPC.GeneratorEffect == 1) ? g_editorStrings.npcGenEffectWarp
                : ((EditorCursor.NPC.GeneratorEffect == 2) ? g_editorStrings.npcGenEffectShoot
                : g_mainMenu.caseNone);
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.npcGenEffectIs, effect), 3, 50, 190);

            if(UpdateButton(mode, 280 + 4, 180 + 4, GFX.EIcons, EditorCursor.NPC.GeneratorEffect == 1, 0, 32*Icon::bottom, 32, 32))
                EditorCursor.NPC.GeneratorEffect = 1;
            if(UpdateButton(mode, 320 + 4, 180 + 4, GFX.EIcons, EditorCursor.NPC.GeneratorEffect == 2, 0, 32*Icon::up, 32, 32))
                EditorCursor.NPC.GeneratorEffect = 2;

            // delay change
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.phraseDelayIsMs, ((int)EditorCursor.NPC.GeneratorTimeMax) * 100), 3, 50, 230);

            if(EditorCursor.NPC.GeneratorTimeMax > 1.f && UpdateButton(mode, 280 + 4, 220 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
                EditorCursor.NPC.GeneratorTimeMax --;
            if(UpdateButton(mode, 320 + 4, 220 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
                EditorCursor.NPC.GeneratorTimeMax ++;
        }
    }

    // COMMON CONTENTS
    if(m_NPC_page == -1 && m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        SuperPrintR(mode, g_editorStrings.wordCoins, 3, 70, 40);

        bool currently_coins = EditorCursor.Block.Special > 0 && EditorCursor.Block.Special < 1000;
        if(UpdateButton(mode, 60 + 4, 60 + 4, GFXNPC[NPCID_COIN_S3], currently_coins, 0, 0, 32, 32) && !currently_coins)
        {
            EditorCursor.Block.Special = 1;
            currently_coins = true;
        }
        if(currently_coins)
        {
            SuperPrint("x" + std::to_string(EditorCursor.Block.Special), 3, 100, 74);
            if(EditorCursor.Block.Special > 0 && UpdateButton(mode, 180 + 4, 60 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
                EditorCursor.Block.Special --;
            if(EditorCursor.Block.Special < 99 && UpdateButton(mode, 220 + 4, 60 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
                EditorCursor.Block.Special ++;
        }

        SuperPrintR(mode, g_mainMenu.caseNone, 3, 40 + 260, 40);
        if(UpdateButton(mode, 360 + 4, 60 + 4, GFXBlock[2], EditorCursor.Block.Special == 0, 0, 0, 32, 32))
            EditorCursor.Block.Special = 0;

        SuperPrintR(mode, "3", 3, 40 + 10, 140);
        static const int p7_common[] = {NPCID_COIN_S3, NPCID_POWER_S3, NPCID_LIFE_S3, NPCID_FIRE_POWER_S3, NPCID_ICE_POWER_S3, NPCID_LEAF_POWER, NPCID_STATUE_POWER, NPCID_HEAVY_POWER, NPCID_GRN_VINE_TOP_S3, NPCID_RANDOM_POWER};
        UpdateNPCGrid(mode, 40, 160, p7_common, sizeof(p7_common)/sizeof(int), 10);

        SuperPrintR(mode, "4", 3, 40 + 10, 200);
        static const int p7_smw[] = {NPCID_COIN_S4, NPCID_POWER_S4, NPCID_LIFE_S4, NPCID_FIRE_POWER_S4, NPCID_3_LIFE, NPCID_ICE_POWER_S4, NPCID_PET_GREEN, NPCID_KEY, NPCID_GRN_VINE_TOP_S4};
        UpdateNPCGrid(mode, 40, 220, p7_smw, sizeof(p7_smw)/sizeof(int), 10);

        SuperPrintR(mode, "1", 3, 40 + 10, 260);
        static const int p7_smb1[] = {NPCID_COIN_S1, NPCID_POWER_S1, NPCID_LIFE_S1, NPCID_FIRE_POWER_S1, NPCID_POISON};
        UpdateNPCGrid(mode, 40, 280, p7_smb1, sizeof(p7_smb1)/sizeof(int), 5);

        SuperPrintR(mode, "2", 3, 40 + 10, 320);
        static const int p7_smb2[] = {NPCID_COIN_S2, NPCID_POWER_S2, NPCID_BOMB, NPCID_EARTHQUAKE_BLOCK, NPCID_TIMER_S2};
        UpdateNPCGrid(mode, 40, 340, p7_smb2, sizeof(p7_smb2)/sizeof(int), 5);

        SuperPrintR(mode, "*", 3, 40 + 10, 380);
        static const int p7_misc[] = {NPCID_RING, NPCID_POWER_S5, NPCID_FLY_POWER, NPCID_GEM_1, NPCID_GEM_5, NPCID_GEM_20};
        UpdateNPCGrid(mode, 40, 400, p7_misc, sizeof(p7_misc)/sizeof(int), 10);
    }
}

#if 0
void EditorScreen::UpdateMagicBlockScreen(CallMode mode)
{
    SuperPrintR(mode, "MAGIC BLOCK (DEPRECATED EXTRA SETTINGS)", 3, 160, 50);

    if(UpdateButton(mode, e_ScreenW - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
        m_special_page = SPECIAL_PAGE_EDITOR_SETTINGS;

    SuperPrintR(mode, "Enable", 3, 90, 94);
    if(UpdateCheckBox(mode, 40 + 4, 80 + 4, MagicBlock::enabled))
        MagicBlock::enabled = !MagicBlock::enabled;

    SuperPrintR(mode, "Overwrite existing items", 3, 90, 134);
    if(UpdateCheckBox(mode, 40 + 4, 120 + 4, MagicBlock::replace_existing))
        MagicBlock::replace_existing = !MagicBlock::replace_existing;


    if(MagicBlock::enabled)
    {
        SuperPrintR(mode, "Check level edges", 3, 90, 174);
        if(UpdateCheckBox(mode, 40 + 4, 160 + 4, MagicBlock::count_level_edges))
            MagicBlock::count_level_edges = !MagicBlock::count_level_edges;


        SuperPrintR(mode, "ADVANCED", 3, 280, 240);


        SuperPrintR(mode, "Allow inner edges", 3, 90, 274);
        if(UpdateCheckBox(mode, 40 + 4, 260 + 4, MagicBlock::advanced_mode))
            MagicBlock::advanced_mode = !MagicBlock::advanced_mode;


        SuperPrintR(mode, "Check", 3, 160, 320);

        SuperPrintR(mode, "All", 3, 90, 354);
        if(UpdateCheckBox(mode, 40 + 4, 340 + 4, MagicBlock::check_level == MagicBlock::LEVEL_ALL))
            MagicBlock::check_level = MagicBlock::LEVEL_ALL;

        SuperPrintR(mode, "Family", 3, 90, 394);
        if(UpdateCheckBox(mode, 40 + 4, 380 + 4, MagicBlock::check_level == MagicBlock::LEVEL_FAMILY))
            MagicBlock::check_level = MagicBlock::LEVEL_FAMILY;

        SuperPrintR(mode, "Subgroup", 3, 90, 434);
        if(UpdateCheckBox(mode, 40 + 4, 420 + 4, MagicBlock::check_level == MagicBlock::LEVEL_GROUP))
            MagicBlock::check_level = MagicBlock::LEVEL_GROUP;

        SuperPrintR(mode, "Change", 3, 460, 320);

        SuperPrintR(mode, "All", 3, 320 + 90, 354);
        if(UpdateCheckBox(mode, 320 + 40 + 4, 340 + 4, MagicBlock::change_level == MagicBlock::LEVEL_ALL))
            MagicBlock::change_level = MagicBlock::LEVEL_ALL;

        SuperPrintR(mode, "Family", 3, 320 + 90, 394);
        if(UpdateCheckBox(mode, 320 + 40 + 4, 380 + 4, MagicBlock::change_level == MagicBlock::LEVEL_FAMILY))
            MagicBlock::change_level = MagicBlock::LEVEL_FAMILY;

        SuperPrintR(mode, "Subgroup", 3, 320 + 90, 434);
        if(UpdateCheckBox(mode, 320 + 40 + 4, 420 + 4, MagicBlock::change_level == MagicBlock::LEVEL_GROUP))
            MagicBlock::change_level = MagicBlock::LEVEL_GROUP;
    }

    return;
}
#endif

void EditorScreen::UpdateEventsScreen(CallMode mode)
{
    if(m_special_page == SPECIAL_PAGE_EVENT_DELETION)
    {
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.eventsDeletingEvent, Events[m_current_event].Name), 3, 60, 40);
        SuperPrintR(mode, g_editorStrings.phraseAreYouSure, 3, 10, 60);
        SuperPrintR(mode, g_editorStrings.eventsDeletionConfirm, 3, 60, 110);
        if(UpdateButton(mode, 20 + 4, 100 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            DeleteEvent((eventindex_t)m_current_event);
            m_special_page = SPECIAL_PAGE_EVENTS;
            m_current_event = 0;
        }
        SuperPrintR(mode, g_editorStrings.eventsDeletionCancel, 3, 60, 150);
        if(UpdateButton(mode, 20 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_EVENTS;
            m_current_event = 0;
        }
        return;
    }

    // render general GUI
    SuperPrintR(mode, g_editorStrings.eventsHeader, 3, 60, 40);
    int page_max = numEvents / 10;
    SuperPrintR(mode, fmt::format_ne(g_editorStrings.pageBlankOfBlank, m_events_page + 1, page_max + 1), 3, e_ScreenW - 330, 40);
    if(m_events_page > 0 && UpdateButton(mode, e_ScreenW - 120 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        m_events_page --;
    if(m_events_page < page_max && UpdateButton(mode, e_ScreenW - 80 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        m_events_page ++;

    // render event selector
    for(int i = 0; i < 10; i++)
    {
        int e = m_events_page*10 + i;
        auto &eName = Events[e].Name;
        if(!eName.empty())
        {
            if(eName.length() < 20)
                SuperPrintR(mode, eName, 3, 10, 80 + 40*i + 10);
            else
            {
                SuperPrintR(mode, eName.substr(0,19), 3, 10, 80 + 40*i + 2);
                SuperPrintR(mode, eName.substr(19), 3, 10, 80 + 40*i + 20);
            }

            if(UpdateButton(mode, 360 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::page, 32, 32))
            {
                m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
                m_current_event = e;
            }

            // rename, shift up, shift down, delete

            if(e <= 2)
                continue;

            // rename
            if(UpdateButton(mode, 400 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
            {
                DisableCursorNew();
                std::string new_name = TextEntryScreen::Run(g_editorStrings.eventsPromptEventName, Events[e].Name);
                if(!new_name.empty())
                    RenameEvent((eventindex_t)e, new_name);
                MouseMove(SharedCursor.X, SharedCursor.Y);
            }

            // shift up
            if(e > 3 && UpdateButton(mode, 440 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::up, 32, 32))
                SwapEvents(e-1, e);

            // shift down
            if(e < numEvents - 1 && UpdateButton(mode, 480 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::down, 32, 32))
                SwapEvents(e, e+1);

            // delete
            if(e < numEvents && UpdateButton(mode, 520 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
            {
                m_special_page = SPECIAL_PAGE_EVENT_DELETION;
                m_current_event = e;
                return;
            }
        }
        // create a new event!
        else if(e != 0 && !Events[e - 1].Name.empty())
        {
            SuperPrintR(mode, g_editorStrings.eventsItemNewEvent, 3, 54, 80 + 40*i + 10);
            // rename only
            if(UpdateButton(mode, 400 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
            {
                DisableCursorNew();
                std::string new_name = TextEntryScreen::Run(g_editorStrings.eventsPromptEventName, "");
                MouseMove(SharedCursor.X, SharedCursor.Y);
                if(!new_name.empty() && FindEvent(new_name) == EVENT_NONE)
                {
                    InitializeEvent(Events[e]);
                    Events[e].Name = new_name;
                    numEvents ++;
                }
            }
        }
    }
}

void EditorScreen::UpdateEventSettingsScreen(CallMode mode)
{
    if(m_special_page == SPECIAL_PAGE_EVENT_BOUNDS)
    {
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.eventsShouldEvent, Events[m_current_event].Name), 3, 60, 40);

        if(m_special_subpage != 0)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.eventsChangeSectionBoundsToCurrent, m_special_subpage), 3, 10, 60);
        else
            SuperPrintR(mode, g_editorStrings.eventsChangeAllSectionBoundsToCurrent, 3, 10, 60);

        SuperPrintR(mode, g_mainMenu.wordYes, 3, 60, 110);
        if(UpdateButton(mode, 20 + 4, 100 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            if(m_special_subpage == 0)
            {
                for(int s = 0; s <= maxSections; s++)
                    Events[m_current_event].section[s].position = static_cast<SpeedlessLocation_t>(level[s]);
            }
            else
                Events[m_current_event].section[m_special_subpage-1].position = static_cast<SpeedlessLocation_t>(level[m_special_subpage-1]);
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        }

        SuperPrintR(mode, g_mainMenu.wordNo, 3, 60, 150);
        if(UpdateButton(mode, 20 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        }

        return;
    }

    if(m_special_page == SPECIAL_PAGE_EVENT_CONTROLS)
    {
        const int titlePosX = 10;
        const int titlePosY = 40;
        PGE_Size titleSize = FontManager_printTextOptiPxR(mode,
                                                          fmt::format_ne(g_editorStrings.eventsControlsForEvent,
                                                                         Events[m_current_event].Name),
                                                          titlePosX, titlePosY,
                                                          590,
                                                          FontManager::fontIdFromSmbxFont(3));;

        int elementsListBaseY = 80;
        const int elementsLabelOffset = 10;
        const int elementsRowHeight = 40;
        while(titlePosY + titleSize.h() >= elementsListBaseY)
            elementsListBaseY += elementsRowHeight;

        if(UpdateButton(mode, e_ScreenW - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 8 * 32, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;

        using Controls::PlayerControls::Buttons;

        int yOffset = elementsListBaseY;

        if(UpdateCheckBox(mode, 10 + 4, yOffset + 4, Events[m_current_event].Controls.Up))
            Events[m_current_event].Controls.Up = !Events[m_current_event].Controls.Up;
        SuperPrintR(mode, GetButtonName_UI(Buttons::Up), 3, 54, yOffset + elementsLabelOffset);

        if(UpdateCheckBox(mode, e_ScreenW / 2 + 10 + 4, yOffset + 4, Events[m_current_event].Controls.Down))
            Events[m_current_event].Controls.Down = !Events[m_current_event].Controls.Down;
        SuperPrintR(mode, GetButtonName_UI(Buttons::Down), 3, e_ScreenW / 2 + 54, yOffset + elementsLabelOffset);

        yOffset += elementsRowHeight;

        if(UpdateCheckBox(mode, 10 + 4, yOffset + 4, Events[m_current_event].Controls.Left))
            Events[m_current_event].Controls.Left = !Events[m_current_event].Controls.Left;
        SuperPrintR(mode, GetButtonName_UI(Buttons::Left), 3, 54, yOffset + elementsLabelOffset);

        if(UpdateCheckBox(mode, e_ScreenW / 2 + 10 + 4, yOffset + 4, Events[m_current_event].Controls.Right))
            Events[m_current_event].Controls.Right = !Events[m_current_event].Controls.Right;
        SuperPrintR(mode, GetButtonName_UI(Buttons::Right), 3, e_ScreenW / 2 + 54, yOffset + elementsLabelOffset);

        yOffset += elementsRowHeight;

        if(UpdateCheckBox(mode, 10 + 4, yOffset + 4, Events[m_current_event].Controls.Jump))
            Events[m_current_event].Controls.Jump = !Events[m_current_event].Controls.Jump;
        SuperPrintR(mode, GetButtonName_UI(Buttons::Jump), 3, 54, yOffset + elementsLabelOffset);

        if(UpdateCheckBox(mode, e_ScreenW / 2 + 10 + 4, yOffset + 4, Events[m_current_event].Controls.Run))
            Events[m_current_event].Controls.Run = !Events[m_current_event].Controls.Run;
        SuperPrintR(mode, GetButtonName_UI(Buttons::Run), 3, e_ScreenW / 2 + 54, yOffset + elementsLabelOffset);

        yOffset += elementsRowHeight;

        if(UpdateCheckBox(mode, 10 + 4, yOffset + 4, Events[m_current_event].Controls.AltJump))
            Events[m_current_event].Controls.AltJump = !Events[m_current_event].Controls.AltJump;
        SuperPrintR(mode, GetButtonName_UI(Buttons::AltJump), 3, 54, yOffset + elementsLabelOffset);

        if(UpdateCheckBox(mode, e_ScreenW / 2 + 10 + 4, yOffset + 4, Events[m_current_event].Controls.AltRun))
            Events[m_current_event].Controls.AltRun = !Events[m_current_event].Controls.AltRun;
        SuperPrintR(mode, GetButtonName_UI(Buttons::AltRun), 3, e_ScreenW / 2 + 54, yOffset + elementsLabelOffset);

        yOffset += elementsRowHeight;

        if(UpdateCheckBox(mode, 10 + 4, yOffset + 4, Events[m_current_event].Controls.Start))
            Events[m_current_event].Controls.Start = !Events[m_current_event].Controls.Start;
        SuperPrintR(mode, GetButtonName_UI(Buttons::Start), 3, 54, yOffset + elementsLabelOffset);

        if(UpdateCheckBox(mode, e_ScreenW / 2 + 10 + 4, yOffset + 4, Events[m_current_event].Controls.Drop))
            Events[m_current_event].Controls.Drop = !Events[m_current_event].Controls.Drop;
        SuperPrintR(mode, GetButtonName_UI(Buttons::Drop), 3, e_ScreenW / 2 + 54, yOffset + elementsLabelOffset);

        return;
    }

    SuperPrintR(mode, g_editorStrings.eventsSettingsForEvent, 3, 60, 40);
    SuperPrintR(mode, Events[m_current_event].Name, 3, 10, 60);

    // RIGHT PANE: layers
    if(mode == CallMode::Render)
        XRender::renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);
    if(UpdateButton(mode, e_ScreenW - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_EVENTS;
        m_current_event = 0;
    }
    // layers
    int layer_line = 1;
    if(!Events[m_current_event].ShowLayer.empty())
    {
        layer_line ++;
        SuperPrintR(mode, g_editorStrings.eventsHeaderShow, 3, e_ScreenW - 200, 40 + (20 * layer_line));
        layer_line ++;
        SuperPrintR(mode, GetL(Events[m_current_event].ShowLayer[0]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
        layer_line ++;

        if(Events[m_current_event].ShowLayer.size() >= 2)
        {
            SuperPrintR(mode, GetL(Events[m_current_event].ShowLayer[1]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
            layer_line ++;
        }

        if(Events[m_current_event].ShowLayer.size() == 3)
        {
            SuperPrintR(mode, GetL(Events[m_current_event].ShowLayer[2]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
            layer_line ++;
        }
        else if(Events[m_current_event].ShowLayer.size() > 3)
        {
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.phraseCountMore, Events[m_current_event].ShowLayer.size() - 2), 3, e_ScreenW - 220, 40 + (20 * layer_line));
            layer_line ++;
        }
    }
    if(!Events[m_current_event].HideLayer.empty())
    {
        layer_line ++;
        SuperPrintR(mode, g_editorStrings.eventsHeaderHide, 3, e_ScreenW - 200, 40 + (20 * layer_line));
        layer_line ++;
        SuperPrintR(mode, GetL(Events[m_current_event].HideLayer[0]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
        layer_line ++;

        if(Events[m_current_event].HideLayer.size() >= 2)
        {
            SuperPrintR(mode, GetL(Events[m_current_event].HideLayer[1]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
            layer_line ++;
        }

        if(Events[m_current_event].HideLayer.size() == 3)
        {
            SuperPrintR(mode, GetL(Events[m_current_event].HideLayer[2]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
            layer_line ++;
        }
        else if(Events[m_current_event].HideLayer.size() > 3)
        {
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.phraseCountMore, Events[m_current_event].HideLayer.size() - 2), 3, e_ScreenW - 220, 40 + (20 * layer_line));
            layer_line ++;
        }
    }
    if(!Events[m_current_event].ToggleLayer.empty())
    {
        layer_line ++;
        SuperPrintR(mode, g_editorStrings.eventsHeaderToggle, 3, e_ScreenW - 200, 40 + (20 * layer_line));
        layer_line ++;
        SuperPrintR(mode, GetL(Events[m_current_event].ToggleLayer[0]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
        layer_line ++;
        if(Events[m_current_event].ToggleLayer.size() >= 2)
        {
            SuperPrintR(mode, GetL(Events[m_current_event].ToggleLayer[1]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
            layer_line ++;
        }
        if(Events[m_current_event].ToggleLayer.size() == 3)
        {
            SuperPrintR(mode, GetL(Events[m_current_event].ToggleLayer[2]), 3, e_ScreenW - 240, 40 + (20 * layer_line));
            layer_line ++;
        }
        else if(Events[m_current_event].ToggleLayer.size() > 3)
        {
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.phraseCountMore, Events[m_current_event].ToggleLayer.size() - 2), 3, e_ScreenW - 220, 40 + (20 * layer_line));
            layer_line ++;
        }
    }
    // MoveLayer is a layerindex_t, not a vector
    if(Events[m_current_event].MoveLayer != LAYER_NONE)
    {
        layer_line ++;
        SuperPrintR(mode, g_editorStrings.eventsHeaderMove, 3, e_ScreenW - 200, 40 + (20 * layer_line));
        layer_line ++;
        SuperPrintR(mode, GetL(Events[m_current_event].MoveLayer), 3, e_ScreenW - 240, 40 + (20 * layer_line));
        layer_line ++;

        // settings for this...
        int sy = vb6Round(Events[m_current_event].SpeedY*10);
        int sx = vb6Round(Events[m_current_event].SpeedX*10);

        if(sy < 0)
            SuperPrintR(mode, g_editorStrings.letterUp + std::to_string(-sy), 3, e_ScreenW - 240, 40 + (20 * layer_line));
        else if(sy > 0)
            SuperPrintR(mode, g_editorStrings.letterDown + std::to_string(sy), 3, e_ScreenW - 240, 40 + (20 * layer_line));
        else
            SuperPrintR(mode, "-", 3, e_ScreenW - 240, 40 + (20 * layer_line));

        layer_line ++;

        if(sx < 0)
            SuperPrintR(mode, g_editorStrings.letterLeft + std::to_string(-sx), 3, e_ScreenW - 240, 40 + (20 * layer_line));
        else if(sx > 0)
            SuperPrintR(mode, g_editorStrings.letterRight + std::to_string(sx), 3, e_ScreenW - 240, 40 + (20 * layer_line));
        else
            SuperPrintR(mode, "-", 3, e_ScreenW - 240, 40 + (20 * layer_line));

        layer_line --;

        if(UpdateButton(mode, e_ScreenW - 160 + 4, 40 + (20 * layer_line) + 4, GFX.EIcons, false, 0, 32*Icon::down, 32, 32))
            Events[m_current_event].SpeedY = 0.1f*(sy+1);
        if(UpdateButton(mode, e_ScreenW - 120 + 4, 40 + (20 * layer_line) + 4, GFX.EIcons, false, 0, 32*Icon::up, 32, 32))
            Events[m_current_event].SpeedY = 0.1f*(sy-1);
        if(UpdateButton(mode, e_ScreenW - 80 + 4, 40 + (20 * layer_line) + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            Events[m_current_event].SpeedX = 0.1f*(sx-1);
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 40 + (20 * layer_line) + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            Events[m_current_event].SpeedX = 0.1f*(sx+1);
    }

    if(layer_line == 1)
    {
        SuperPrintR(mode, g_editorStrings.layersHeader, 3, e_ScreenW - 192, 40);
        SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 40 + (20 * layer_line));

        Events[m_current_event].LayerSmoke = false;
    }
    else
    {
        SuperPrintR(mode, g_editorStrings.layersHeader, 3, e_ScreenW - 192, 50);

        // layer smoke (bottom of layers pane)
        SuperPrintR(mode, g_editorStrings.eventsPropLayerSmoke, 3, e_ScreenW - 192, 40 + (20 * layer_line) + 30);
        if(UpdateCheckBox(mode, e_ScreenW - 240 + 4, 40 + (20 * layer_line) + 24, !Events[m_current_event].LayerSmoke))
            Events[m_current_event].LayerSmoke = !Events[m_current_event].LayerSmoke;
    }

    if(UpdateButton(mode, e_ScreenW - 240 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_EVENT_LAYERS;

    // BOTTOM PANE: sections - background, music, autoscroll
    if(mode == CallMode::Render)
        XRender::renderRect(0, e_ScreenH - 180, e_ScreenW - 240, 180, 0.6f, 0.6f, 0.8f, 1.0f, true);
    if(m_special_subpage > 0 && UpdateButton(mode, 40 + 4, e_ScreenH - 180 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        m_special_subpage --;
    if(m_special_subpage < maxSections + 1 && UpdateButton(mode, 320 + 4, e_ScreenH - 180 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        m_special_subpage ++;

    // description of possibilities
    SuperPrintCenterR(mode, g_editorStrings.eventsActionKeep, 3, 170, e_ScreenH - 140);
    SuperPrintCenterR(mode, g_editorStrings.eventsActionReset, 3, 270, e_ScreenH - 140);
    SuperPrintCenterR(mode, g_editorStrings.eventsActionSet, 3, 370, e_ScreenH - 140);

    SuperPrintR(mode, g_editorStrings.eventsCaseMusic, 3, 4, e_ScreenH - 110);
    SuperPrintR(mode, g_editorStrings.eventsCaseBackground, 3, 4, e_ScreenH - 70);
    SuperPrintR(mode, g_editorStrings.eventsCaseBounds, 3, 4, e_ScreenH - 30);

    // subpage - 1 is the internal section ID; subpage 0 is all sections.
    if(m_special_subpage == 0)
    {
        SuperPrintCenterR(mode, g_editorStrings.eventsPhraseAllSections, 3, 190, e_ScreenH - 174);

        // music
        bool all_keep = true;
        bool all_reset = true;
        bool all_set = true;
        int set_same = -1;
        for(int s = 0; s <= maxSections; s++)
        {
            if(Events[m_current_event].section[s].music_id != LESet_Nothing)
                all_keep = false;
            if(Events[m_current_event].section[s].music_id != LESet_ResetDefault)
                all_reset = false;
            if(Events[m_current_event].section[s].music_id < 0)
                all_set = false;
            else if(set_same == -1)
                set_same = Events[m_current_event].section[s].music_id;
            else if(set_same != Events[m_current_event].section[s].music_id)
                all_set = false;
        }
        if(UpdateButton(mode, 150 + 4, e_ScreenH - 120 + 4, GFX.EIcons, all_keep, 0, 0, 1, 1) && !all_keep)
        {
            for(int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].music_id = LESet_Nothing;
        }
        if(UpdateButton(mode, 250 + 4, e_ScreenH - 120 + 4, GFX.EIcons, all_reset, 0, 32*Icon::x, 32, 32) && !all_reset)
        {
            for(int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].music_id = LESet_ResetDefault;
        }
        if(UpdateButton(mode, 350 + 4, e_ScreenH - 120 + 4, GFX.EIcons, all_set, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_MUSIC;

        // background
        all_keep = true;
        all_reset = true;
        all_set = true;
        set_same = -1;
        for(int s = 0; s <= maxSections; s++)
        {
            if(Events[m_current_event].section[s].background_id != LESet_Nothing)
                all_keep = false;
            if(Events[m_current_event].section[s].background_id != LESet_ResetDefault)
                all_reset = false;
            if(Events[m_current_event].section[s].background_id < 0)
                all_set = false;
            else if(set_same == -1)
                set_same = Events[m_current_event].section[s].background_id;
            else if(set_same != Events[m_current_event].section[s].background_id)
                all_set = false;
        }
        if(UpdateButton(mode, 150 + 4, e_ScreenH - 80 + 4, GFX.EIcons, all_keep, 0, 0, 1, 1) && !all_keep)
        {
            for(int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].background_id = LESet_Nothing;
        }
        if(UpdateButton(mode, 250 + 4, e_ScreenH - 80 + 4, GFX.EIcons, all_reset, 0, 32*Icon::x, 32, 32) && !all_reset)
        {
            for(int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].background_id = LESet_ResetDefault;
        }
        if(UpdateButton(mode, 350 + 4, e_ScreenH - 80 + 4, GFX.EIcons, all_set, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_BACKGROUND;

        // bounds
        all_keep = true;
        all_reset = true;
        all_set = true;
        for(int s = 0; s <= maxSections; s++)
        {
            if((int)Events[m_current_event].section[s].position.X == LESet_Nothing)
            {
                all_reset = false;
                all_set = false;
            }
            else if((int)Events[m_current_event].section[s].position.X == LESet_ResetDefault)
            {
                all_set = false;
                all_keep = false;
            }
            else
            {
                all_keep = false;
                all_reset = false;
            }
        }
        if(UpdateButton(mode, 150 + 4, e_ScreenH - 40 + 4, GFX.EIcons, all_keep, 0, 0, 1, 1) && !all_keep)
        {
            for(int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].position.X = -1;
        }
        if(UpdateButton(mode, 250 + 4, e_ScreenH - 40 + 4, GFX.EIcons, all_reset, 0, 32*Icon::x, 32, 32) && !all_reset)
        {
            for(int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].position.X = -2;
        }
        if(UpdateButton(mode, 350 + 4, e_ScreenH - 40 + 4, GFX.EIcons, all_set, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_BOUNDS;
    }
    else
    {
        SuperPrintCenterR(mode, fmt::format_ne(g_editorStrings.phraseSectionIndex, m_special_subpage), 3, 190, e_ScreenH - 174);

        // music
        if(UpdateButton(mode, 150 + 4, e_ScreenH - 120 + 4, GFX.EIcons, Events[m_current_event].section[m_special_subpage-1].music_id == LESet_Nothing, 0, 0, 1, 1))
            Events[m_current_event].section[m_special_subpage-1].music_id = LESet_Nothing;

        if(UpdateButton(mode, 250 + 4, e_ScreenH - 120 + 4, GFX.EIcons, Events[m_current_event].section[m_special_subpage-1].music_id == LESet_ResetDefault, 0, 32*Icon::x, 32, 32))
            Events[m_current_event].section[m_special_subpage-1].music_id = LESet_ResetDefault;

        if(UpdateButton(mode, 350 + 4, e_ScreenH - 120 + 4, GFX.EIcons, Events[m_current_event].section[m_special_subpage-1].music_id >= 0, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_MUSIC;

        // background
        if(UpdateButton(mode, 150 + 4, e_ScreenH - 80 + 4, GFX.EIcons, Events[m_current_event].section[m_special_subpage-1].background_id == LESet_Nothing, 0, 0, 1, 1))
            Events[m_current_event].section[m_special_subpage-1].background_id = LESet_Nothing;

        if(UpdateButton(mode, 250 + 4, e_ScreenH - 80 + 4, GFX.EIcons, Events[m_current_event].section[m_special_subpage-1].background_id == LESet_ResetDefault, 0, 32*Icon::x, 32, 32))
            Events[m_current_event].section[m_special_subpage-1].background_id = LESet_ResetDefault;

        if(UpdateButton(mode, 350 + 4, e_ScreenH - 80 + 4, GFX.EIcons, Events[m_current_event].section[m_special_subpage-1].background_id >= 0, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_BACKGROUND;

        // bounds
        if(UpdateButton(mode, 150 + 4, e_ScreenH - 40 + 4, GFX.EIcons, (int)Events[m_current_event].section[m_special_subpage-1].position.X == LESet_Nothing, 0, 0, 1, 1))
            Events[m_current_event].section[m_special_subpage-1].position.X = LESet_Nothing;

        if(UpdateButton(mode, 250 + 4, e_ScreenH - 40 + 4, GFX.EIcons, (int)Events[m_current_event].section[m_special_subpage-1].position.X == LESet_ResetDefault, 0, 32*Icon::x, 32, 32))
            Events[m_current_event].section[m_special_subpage-1].position.X = LESet_ResetDefault;

        if(UpdateButton(mode, 350 + 4, e_ScreenH - 40 + 4, GFX.EIcons, (int)Events[m_current_event].section[m_special_subpage-1].position.X != LESet_Nothing && (int)Events[m_current_event].section[m_special_subpage-1].position.X != LESet_ResetDefault, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_BOUNDS;
    }

    // autostart (top left)
    SuperPrintR(mode, g_editorStrings.eventsPropAutostart, 3, 54, 90);
    if(UpdateCheckBox(mode, 10 + 4, 80 + 4, Events[m_current_event].AutoStart))
        Events[m_current_event].AutoStart = !Events[m_current_event].AutoStart;

    // sound (top right)
    SuperPrintR(mode, g_editorStrings.eventsPropSound, 3, 254, 90);
    if(UpdateButton(mode, 210 + 4, 80 + 4, GFX.EIcons, Events[m_current_event].Sound != 0, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_EVENT_SOUND;

    // end game (mid left)
    SuperPrintR(mode, g_editorStrings.eventsPropEndGame, 3, 54, 130);
    if(UpdateCheckBox(mode, 10 + 4, 120 + 4, Events[m_current_event].EndGame == 1))
        Events[m_current_event].EndGame ^= 1;

    // control lock (mid right)
    bool controls_set = (Events[m_current_event].Controls.AltJump ||
        Events[m_current_event].Controls.AltRun ||
        Events[m_current_event].Controls.Down ||
        Events[m_current_event].Controls.Drop ||
        Events[m_current_event].Controls.Jump ||
        Events[m_current_event].Controls.Left ||
        Events[m_current_event].Controls.Right ||
        Events[m_current_event].Controls.Run ||
        Events[m_current_event].Controls.Start ||
        Events[m_current_event].Controls.Up);
    SuperPrintR(mode, g_editorStrings.eventsPropControls, 3, 254, 130);
    if(UpdateButton(mode, 210 + 4, 120 + 4, GFX.EIcons, controls_set, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_EVENT_CONTROLS;

    // message (bottom left)
    if(!GetS(Events[m_current_event].Text).empty())
    {
        MessageText = GetS(Events[m_current_event].Text);
        BuildUTF8CharMap(MessageText, MessageTextMap);
    }

    SuperPrintR(mode, g_editorStrings.wordText, 3, 54, 170);
    if(UpdateButton(mode, 10 + 4, 160 + 4, GFX.EIcons, !GetS(Events[m_current_event].Text).empty(), 0, 32*Icon::pencil, 32, 32))
    {
        DisableCursorNew();
        SetS(Events[m_current_event].Text, TextEntryScreen::Run(g_editorStrings.eventsPromptEventText, GetS(Events[m_current_event].Text)));
        MouseMove(SharedCursor.X, SharedCursor.Y);
    }

    // trigger event (full width, below all)
    SuperPrintR(mode, g_editorStrings.eventsHeaderTriggerEvent, 3, 54, 220);
    if(UpdateButton(mode, 10 + 4, 220 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_EVENT_TRIGGER;

    if(Events[m_current_event].TriggerEvent != EVENT_NONE)
    {
        SuperPrintR(mode, GetE(Events[m_current_event].TriggerEvent).substr(0,19), 3, 54, 240);

        if(Events[m_current_event].TriggerDelay > 0)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.phraseDelayIsMs, 100 * (int)Events[m_current_event].TriggerDelay), 3, 54, 272);
        else
            SuperPrintR(mode, g_editorStrings.wordInstant, 3, 54, 272);

        if(Events[m_current_event].TriggerDelay > 0 && UpdateButton(mode, 10 + 4, 260 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            Events[m_current_event].TriggerDelay --;

        if(UpdateButton(mode, 290 + 4, 260 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            Events[m_current_event].TriggerDelay ++;
    }
    else
        SuperPrintR(mode, g_mainMenu.caseNone, 3, 54, 240);
}

// updates the bounds for Section 0 in level start event according to its autoscroll
void UpdateStartLevelEventBounds()
{
    Events[0].AutoSection = 0;
    Events[0].section[0].position = static_cast<SpeedlessLocation_t>(level[0]);
    // not sure why 800 is also used for height in the default code, but I will stick with it.
    if(Events[0].AutoX < 0)
        Events[0].section[0].position.X = Events[0].section[0].position.Width - 800;
    else if(Events[0].AutoX > 0)
        Events[0].section[0].position.Width = Events[0].section[0].position.X + 800;
    if(Events[0].AutoY < 0)
        Events[0].section[0].position.Y = Events[0].section[0].position.Height - 800;
    else if(Events[0].AutoY > 0)
        Events[0].section[0].position.Height = Events[0].section[0].position.Y + 800;
}

void EditorScreen::UpdateSectionsScreen(CallMode mode)
{
    // level settings
    if(UpdateButton(mode, 10 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
    {
        DisableCursorNew();
        LevelName = TextEntryScreen::Run(g_editorStrings.levelName, LevelName);
        MouseMove(SharedCursor.X, SharedCursor.Y);
    }
    SuperPrintR(mode, g_editorStrings.levelName, 3, 54, 42);
    if(!LevelName.empty())
        SuperPrintR(mode, LevelName, 3, 54, 60);
    else
        SuperPrintR(mode, g_mainMenu.caseNone, 3, 54, 60);

    SuperPrintR(mode, g_editorStrings.levelStartPos, 3, 10, 110);
    if(UpdateButton(mode, 240, 100, GFXBlock[622], EditorCursor.SubMode == 4, 0, 0, 32, 32))
        EditorCursor.SubMode = 4;
    if(UpdateButton(mode, 280, 100, GFXBlock[623], EditorCursor.SubMode == 5, 0, 0, 32, 32))
        EditorCursor.SubMode = 5;

    // section settings
    SuperPrintCenterR(mode, fmt::format_ne(g_editorStrings.phraseSectionIndex, curSection + 1), 3, 320, 166);
    if(curSection > 0 && UpdateButton(mode, 160 + 4, 160 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        SetSection(curSection - 1);
    if(curSection < maxSections && UpdateButton(mode, 440 + 4, 160 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        SetSection(curSection + 1);

    // music
    if(UpdateButton(mode, 10 + 4, 280 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_SECTION_MUSIC;

    if(mode == CallMode::Render)
    {
        const auto& indices = EditorCustom::music_list.indices;
        const auto& names = EditorCustom::music_list.names;

        size_t i;
        for(i = 0; i < indices.size(); i++)
        {
            if(indices[i] == bgMusic[curSection])
                break;
        }

        if(i == indices.size())
            SuperPrint(g_editorStrings.eventsCaseMusic + ": " + std::to_string(bgMusic[curSection]), 3, 54, 286);
        else
            SuperPrint(g_editorStrings.eventsCaseMusic + ": " + names[i], 3, 54, 286);
    }

    if(bgMusic[curSection] == 24)
    {
        if(CustomMusic[curSection].length() < 15)
        {
            SuperPrintR(mode, CustomMusic[curSection], 3, 374, 292);
        }
        else
        {
            SuperPrintR(mode, CustomMusic[curSection].substr(0,14), 3, 374, 282);
            SuperPrintR(mode, CustomMusic[curSection].substr(14,14), 3, 374, 300);
        }

        if(UpdateButton(mode, 330 + 4, 280 + 4, GFX.EIcons, false, 0, 32*Icon::open, 32, 32))
            StartFileBrowser(&CustomMusic[curSection], FileNamePath, "", c_musicFormats, BROWSER_MODE_OPEN, BROWSER_CALLBACK_CUSTOM_MUSIC);
    }

    // background
    if(UpdateButton(mode, 10 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_SECTION_BACKGROUND;

    if(mode == CallMode::Render)
    {
        const auto& indices = EditorCustom::bg2_list.indices;
        const auto& names = EditorCustom::bg2_list.names;

        size_t i;
        for(i = 0; i < indices.size(); i++)
        {
            if(indices[i] == Background2[curSection])
                break;
        }

        if(i == indices.size())
            SuperPrintR(mode, g_editorStrings.eventsCaseBackground + ": " + std::to_string(Background2[curSection]), 3, 54, 246);
        else
            SuperPrintR(mode, g_editorStrings.eventsCaseBackground + ": " + names[i], 3, 54, 246);
    }

    // set bounds
    SuperPrintR(mode, g_editorStrings.sectionSetBounds, 3, 10, 206);
    if(UpdateButton(mode, 240, 200, GFX.EIcons, EditorCursor.SubMode == 0, 0, 32*Icon::up, 32, 32))
        EditorCursor.SubMode = 0;
    if(UpdateButton(mode, 280, 200, GFX.EIcons, EditorCursor.SubMode == 3, 0, 32*Icon::down, 32, 32))
        EditorCursor.SubMode = 3;
    if(UpdateButton(mode, 320, 200, GFX.EIcons, EditorCursor.SubMode == 1, 0, 32*Icon::left, 32, 32))
        EditorCursor.SubMode = 1;
    if(UpdateButton(mode, 360, 200, GFX.EIcons, EditorCursor.SubMode == 2, 0, 32*Icon::right, 32, 32))
        EditorCursor.SubMode = 2;

    // hwrap - LevelWrap
    if(UpdateCheckBox(mode, 10 + 4, 320 + 4, LevelWrap[curSection]))
        LevelWrap[curSection] = !LevelWrap[curSection];
    SuperPrintR(mode, g_editorStrings.sectionHorizWrap, 3, 54, 326);
    // underwater - UnderWater
    if(UpdateCheckBox(mode, e_ScreenW/2 + 10 + 4, 320 + 4, UnderWater[curSection]))
        UnderWater[curSection] = !UnderWater[curSection];
    SuperPrintR(mode, g_editorStrings.sectionUnderwater, 3, e_ScreenW/2 + 54, 326);
    // no turn back - NoTurnBack
    if(UpdateCheckBox(mode, 10 + 4, 360 + 4, NoTurnBack[curSection]))
        NoTurnBack[curSection] = !NoTurnBack[curSection];
    SuperPrintR(mode, g_editorStrings.sectionNoTurnBack, 3, 54, 366);
    // leave to exit - OffScreenExit
    if(UpdateCheckBox(mode, e_ScreenW/2 + 10 + 4, 360 + 4, OffScreenExit[curSection]))
        OffScreenExit[curSection] = !OffScreenExit[curSection];
    SuperPrintR(mode, g_editorStrings.sectionOffscreenExit, 3, e_ScreenW/2 + 54, 366);

    // moved autoscroll into level settings, and only allow section 0 / event 0 (level start).
    // this is due to an awful bug which couldn't be fixed if people had taken
    // advantage of the ridiculously incorrect original behavior.
    if(curSection == 0)
    {
        SuperPrintR(mode, g_editorStrings.sectionScroll, 3, 10, 430);

        int sy = vb6Round(Events[0].AutoY*10);
        if(sy < 0)
            SuperPrintR(mode, g_editorStrings.letterUp    + std::to_string(-sy), 3, 180, 422);
        else if(sy > 0)
            SuperPrintR(mode, g_editorStrings.letterDown  + std::to_string(sy), 3, 180, 422);
        else
            SuperPrintR(mode, "-", 3, 180, 422);

        int sx = vb6Round(Events[0].AutoX*10);
        if(sx < 0)
            SuperPrintR(mode, g_editorStrings.letterLeft  + std::to_string(-sx), 3, 180, 440);
        else if(sx > 0)
            SuperPrintR(mode, g_editorStrings.letterRight + std::to_string(sx), 3, 180, 440);
        else
            SuperPrintR(mode, "-", 3, 180, 440);

        if(UpdateButton(mode, 240 + 4, 420 + 4, GFX.EIcons, false, 0, 32*Icon::up, 32, 32))
        {
            Events[0].AutoY = 0.1f*(sy-1);
            UpdateStartLevelEventBounds();
        }

        if(UpdateButton(mode, 280 + 4, 420 + 4, GFX.EIcons, false, 0, 32*Icon::down, 32, 32))
        {
            Events[0].AutoY = 0.1f*(sy+1);
            UpdateStartLevelEventBounds();
        }

        if(UpdateButton(mode, 320 + 4, 420 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        {
            Events[0].AutoX = 0.1f*(sx-1);
            UpdateStartLevelEventBounds();
        }

        if(UpdateButton(mode, 360 + 4, 420 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        {
            Events[0].AutoX = 0.1f*(sx+1);
            UpdateStartLevelEventBounds();
        }
    }
}

void EditorScreen::UpdateEditorSettingsScreen(CallMode mode)
{
    // settings screen, now.

#if 0
    // previous magic block settings

    SuperPrintR(mode, g_editorStrings.toggleMagicBlock, 3, 10, 50);


    if(EditorCustom::block_pages.empty() && EditorCustom::bgo_pages.empty() && EditorCustom::tile_pages.empty())
    {
        if(mode == CallMode::Render)
        {
            SuperPrint("Magic Block", 3, 10, 140);
            SuperPrint("needs editor.ini", 3, 10, 160);
            SuperPrint("to work.", 3, 10, 180);
        }
    }
    else
    {
        SuperPrintR(mode, "Normal", 3, 50, 94);
        if(UpdateButton(mode, 4, 80 + 4, GFX.ECursor[2], MagicBlock::enabled && MagicBlock::count_level_edges && !MagicBlock::advanced_mode && MagicBlock::check_level == MagicBlock::LEVEL_FAMILY && MagicBlock::change_level == MagicBlock::LEVEL_FAMILY))
        {
            MagicBlock::enabled = true;
            MagicBlock::count_level_edges = true;
            MagicBlock::advanced_mode = false;
            MagicBlock::check_level = MagicBlock::LEVEL_FAMILY;
            MagicBlock::change_level = MagicBlock::LEVEL_FAMILY;
        }

        SuperPrintR(mode, "Behind", 3, 50, 134);
        if(UpdateButton(mode, 4, 120 + 4, GFX.ECursor[2], MagicBlock::enabled && MagicBlock::count_level_edges && !MagicBlock::advanced_mode && MagicBlock::check_level == MagicBlock::LEVEL_ALL && MagicBlock::change_level == MagicBlock::LEVEL_FAMILY))
        {
            MagicBlock::enabled = true;
            MagicBlock::count_level_edges = true;
            MagicBlock::advanced_mode = false;
            MagicBlock::check_level = MagicBlock::LEVEL_ALL;
            MagicBlock::change_level = MagicBlock::LEVEL_FAMILY;
        }

        SuperPrintR(mode, "Advanced", 3, 50, 174);
        if(UpdateButton(mode, 4, 160 + 4, GFX.EIcons, MagicBlock::enabled && (!MagicBlock::count_level_edges || MagicBlock::advanced_mode || MagicBlock::check_level == MagicBlock::LEVEL_GROUP || MagicBlock::change_level != MagicBlock::LEVEL_FAMILY), 0, 32 * Icon::subscreen, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_MAGICBLOCK;
            return;
        }

        SuperPrintR(mode, "Off", 3, 50, 214);
        if(UpdateButton(mode, 4, 200 + 4, GFX.ECursor[2], !MagicBlock::enabled))
        {
            MagicBlock::enabled = false;
        }

        SuperPrintR(mode, "Overwrite", 3, 50, 294);
        if(UpdateCheckBox(mode, 4, 280 + 4, MagicBlock::replace_existing))
            MagicBlock::replace_existing = !MagicBlock::replace_existing;
    }
#endif

    if(WorldEditor || MagicHand)
        return;

    // level test settings

    if(m_special_subpage < 1)
        m_special_subpage = 1;
    if(m_special_subpage > maxLocalPlayers)
        m_special_subpage = maxLocalPlayers;
    if(this->num_test_players < 1)
        this->num_test_players = 1;
    if(m_special_subpage > this->num_test_players + 1)
        m_special_subpage = this->num_test_players;

    SuperPrintRightR(mode, g_editorStrings.testMagicHand, 3, e_ScreenW - 60, 94);
    if(UpdateCheckBox(mode, e_ScreenW - 50 + 4, 80 + 4, this->test_magic_hand))
        this->test_magic_hand = !this->test_magic_hand;

    if(testStartWarp == 0)
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.warpTo, g_editorStrings.levelStartPos), 3, 46, 94);
    else
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.warpTo, fmt::format_ne(g_editorStrings.phraseWarpIndex, testStartWarp)), 3, 46, 94);

    if(testStartWarp > 0 && UpdateButton(mode, 4, 80 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        testStartWarp--;

    if(testStartWarp < numWarps && UpdateButton(mode, 280 + 4, 80 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        testStartWarp++;

    SuperPrintRightR(mode, Controls::EditorControls::g_button_name_UI[Controls::EditorControls::TestPlay], 3, e_ScreenW - 60, 54);
    if(UpdateButton(mode, e_ScreenW-50 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::play, 32, 32))
    {
        // turn this into a routine...?! (cross-reference editor.cpp handler for EditorControls.TestPlay)
        EditorBackup();
        Backup_FullFileName = FullFileName;
        // how does this interact with cross-level warps?
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
        zTestLevel(this->test_magic_hand);
    }

    SuperPrintCenterR(mode, g_mainMenu.wordPlayer + " " + std::to_string(m_special_subpage), 3, 510, 154);
    if(m_special_subpage > 1 && UpdateButton(mode, 400 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        m_special_subpage --;
    if(m_special_subpage < maxLocalPlayers && m_special_subpage <= this->num_test_players && UpdateButton(mode, 580 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        m_special_subpage ++;

    if(m_special_subpage >= this->num_test_players && m_special_subpage != 1)
    {
        SuperPrintR(mode, g_editorStrings.wordEnabled, 3, e_ScreenW / 2 + 54, 194);

        if(UpdateCheckBox(mode, e_ScreenW / 2 + 10 + 4, 180 + 4, m_special_subpage == this->num_test_players))
        {
            if(m_special_subpage == this->num_test_players)
                this->num_test_players -= 1;
            else
                this->num_test_players += 1;
        }
    }

    if(m_special_subpage > this->num_test_players)
        return;

    SuperPrintR(mode, g_editorStrings.testChar, 3, e_ScreenW / 2 + 10, 234);

    constexpr int block_for_char[] = {622, 623, 624, 625, 631};
    for(int ch = 1; ch <= 5; ch++)
    {
        bool pPctive = testPlayer[m_special_subpage].Character == ch;
        int block = block_for_char[ch - 1];

        if(UpdateButton(mode, e_ScreenW / 2 + 80 + 4 + 40*ch, 220 + 4, GFXBlock[block], pPctive, 0, 0, 32, 32))
            testPlayer[m_special_subpage].Character = ch;
    }

    SuperPrintR(mode, g_editorStrings.testPower, 3, e_ScreenW / 2 + 10, 274);

    if(testPlayer[m_special_subpage].State == 0)
        testPlayer[m_special_subpage].State = 2;

    constexpr int NPC_for_state[] = {0, NPCID_POWER_S3, NPCID_FIRE_POWER_S3, NPCID_LEAF_POWER, NPCID_STATUE_POWER, NPCID_HEAVY_POWER, NPCID_ICE_POWER_S3};
    for(int state = 1; state <= 7; state++)
    {
        bool pActive = testPlayer[m_special_subpage].State == state;
        int sNPC = NPC_for_state[state - 1];

        bool selected;
        if(!sNPC)
            selected = UpdateButton(mode, e_ScreenW / 2 + 120 + 4 + 40 * ((state - 1) % 5), 260 + 4 + ((state - 1) / 5) * 40, GFX.EIcons, pActive, 0, 0, 1, 1);
        else
            selected = UpdateButton(mode, e_ScreenW / 2 + 120 + 4 + 40 * ((state - 1) % 5), 260 + 4 + ((state - 1) / 5) * 40, GFXNPC[sNPC], pActive, 0, 0, 32, 32);

        if(selected)
            testPlayer[m_special_subpage].State = state;
    }

    SuperPrintR(mode, g_editorStrings.testBoot, 3, e_ScreenW / 2 + 10, 354);

    constexpr int NPC_for_boot[] = {NPCID_GRN_BOOT, NPCID_RED_BOOT, NPCID_BLU_BOOT};
    for(int boot = 1; boot <= 3; boot++)
    {
        bool pActive = testPlayer[m_special_subpage].Mount == 1 && testPlayer[m_special_subpage].MountType == boot;
        int pNPC = NPC_for_boot[boot - 1];

        if(UpdateButton(mode, e_ScreenW / 2 + 80 + 4 + 40*boot, 340 + 4, GFXNPC[pNPC], pActive, 0, 0, 32, 32))
        {
            if(pActive)
            {
                testPlayer[m_special_subpage].Mount = 0;
            }
            else
            {
                testPlayer[m_special_subpage].Mount = 1;
                testPlayer[m_special_subpage].MountType = boot;
            }
        }
    }

    SuperPrintR(mode, g_editorStrings.testPet, 3, e_ScreenW / 2 + 10, 394);

    constexpr int NPC_for_yoshi[] = {NPCID_PET_GREEN, NPCID_PET_BLUE, NPCID_PET_YELLOW, NPCID_PET_RED, NPCID_PET_BLACK, NPCID_PET_PURPLE, NPCID_PET_PINK, NPCID_PET_CYAN};
    for(int yoshi = 1; yoshi <= 8; yoshi++)
    {
        bool pActive = testPlayer[m_special_subpage].Mount == 3 && testPlayer[m_special_subpage].MountType == yoshi;
        int pNPC = NPC_for_yoshi[yoshi - 1];

        if(UpdateButton(mode, e_ScreenW / 2 + 120 + 4 + 40 * ((yoshi - 1) % 5), 380 + 4 + ((yoshi - 1) / 5) * 40, GFXNPC[pNPC], pActive, 0, 0, 72, 56))
        {
            if(pActive)
            {
                testPlayer[m_special_subpage].Mount = 0;
            }
            else
            {
                testPlayer[m_special_subpage].Mount = 3;
                testPlayer[m_special_subpage].MountType = yoshi;
            }
        }
    }
}

void EditorScreen::UpdateWorldSettingsScreen(CallMode mode)
{
    // world name
    if(UpdateButton(mode, 10 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
    {
        DisableCursorNew();
        WorldName = TextEntryScreen::Run(g_editorStrings.worldName, WorldName);
        MouseMove((float)SharedCursor.X, (float)SharedCursor.Y);
    }

    SuperPrintR(mode, g_editorStrings.worldName, 3, 54, 42);
    if(!WorldName.empty())
        SuperPrintR(mode, WorldName, 3, 54, 60);
    else
        SuperPrintR(mode, g_mainMenu.caseNone, 3, 54, 60);

    // auto start level
    if(UpdateButton(mode, 10 + 4, 100 + 4, GFX.EIcons, false, 0, 32*Icon::open, 32, 32))
        StartFileBrowser(&StartLevel, FileNamePath, "", {".lvl", ".lvlx"}, BROWSER_MODE_OPEN);

    SuperPrintR(mode, g_editorStrings.worldIntroLevel, 3, 54, 102);

    if(!StartLevel.empty())
        SuperPrintR(mode, StartLevel, 3, 54, 120);
    else
        SuperPrintR(mode, g_mainMenu.caseNone, 3, 54, 120);

    // no world map - NoMap
    if(UpdateCheckBox(mode, 10 + 4, 160 + 4, NoMap))
        NoMap = !NoMap;

    SuperPrintR(mode, g_editorStrings.worldHubWorld, 3, 54, 170);

    // restart after death - RestartLevel
    if(UpdateCheckBox(mode, e_ScreenW/2 + 10 + 4, 160 + 4, RestartLevel))
        RestartLevel = !RestartLevel;

    SuperPrintR(mode, g_editorStrings.worldRetryOnFail, 3, e_ScreenW/2 + 54, 170);

    // world star count
    if(MaxWorldStars > 0 && UpdateButton(mode, 120 + 4, 220 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        MaxWorldStars --;

    SuperPrintCenterR(mode, g_editorStrings.worldTotalStars + std::to_string(MaxWorldStars), 3, 300, 230);

    if(UpdateButton(mode, 440 + 4, 220 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        MaxWorldStars ++;

    // allow chars
    SuperPrintR(mode, g_editorStrings.worldAllowChars, 3, 10, 290);

    if(UpdateButton(mode, 240 + 4, 280 + 4, GFXBlock[622], !blockCharacter[1], 0, 0, 32, 32))
        blockCharacter[1] = !blockCharacter[1];

    if(UpdateButton(mode, 280 + 4, 280 + 4, GFXBlock[623], !blockCharacter[2], 0, 0, 32, 32))
        blockCharacter[2] = !blockCharacter[2];

    if(UpdateButton(mode, 320 + 4, 280 + 4, GFXBlock[624], !blockCharacter[3], 0, 0, 32, 32))
        blockCharacter[3] = !blockCharacter[3];

    if(UpdateButton(mode, 360 + 4, 280 + 4, GFXBlock[625], !blockCharacter[4], 0, 0, 32, 32))
        blockCharacter[4] = !blockCharacter[4];

    if(UpdateButton(mode, 400 + 4, 280 + 4, GFXBlock[631], !blockCharacter[5], 0, 0, 32, 32))
        blockCharacter[5] = !blockCharacter[5];

    // don't allow all characters to be blocked.
    if(blockCharacter[1] && blockCharacter[2] && blockCharacter[3] && blockCharacter[4] && blockCharacter[5])
        blockCharacter[1] = false;


    // World credits...
    if(m_special_subpage > 0 && UpdateButton(mode, 10 + 4, 340 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        m_special_subpage --;

    if(UpdateButton(mode, 50 + 4, 340 + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
    {
        DisableCursorNew();
        WorldCredits[m_special_subpage + 1] = TextEntryScreen::Run(fmt::format_ne(g_editorStrings.worldCreditIndex, m_special_subpage + 1), WorldCredits[m_special_subpage + 1]);
        MouseMove((float)SharedCursor.X, (float)SharedCursor.Y);
        for(int i = SDL_max(numWorldCredits, m_special_subpage + 1); i > 0; --i) // Find the last non-empty line
        {
            if(!WorldCredits[m_special_subpage + 1].empty())
            {
                numWorldCredits = i;
                break;
            }
        }
    }

    if(m_special_subpage < 4 && UpdateButton(mode, 90 + 4, 340 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        m_special_subpage ++;

    SuperPrintR(mode, fmt::format_ne(g_editorStrings.worldCreditIndex, m_special_subpage + 1), 3, 144, 342);
    SuperPrintR(mode, WorldCredits[m_special_subpage + 1], 3, 144, 360);
}

void EditorScreen::UpdateSelectListScreen(CallMode mode)
{
    if(m_special_page != SPECIAL_PAGE_NONE && UpdateButton(mode, e_ScreenW - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
    {
        if(m_special_page == SPECIAL_PAGE_EVENT_SOUND || m_special_page == SPECIAL_PAGE_EVENT_MUSIC || m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        else
            m_special_page = SPECIAL_PAGE_NONE;
    }

    vbint_t* target;
    int* current_page;
    PGE_Size titleSize;
    const int titlePosX = 10;
    const int titlePosY = 40;
    const std::vector<std::string>* source;
    const std::vector<int16_t>* source_indices;

    if(m_special_page == SPECIAL_PAGE_EVENT_SOUND)
    {
        titleSize = FontManager_printTextOptiPxR(mode,
                                                 fmt::format_ne(g_editorStrings.selectSoundForEvent,
                                                                Events[m_current_event].Name),
                                                 titlePosX, titlePosY,
                                                 300,
                                                 FontManager::fontIdFromSmbxFont(3));
        target = &Events[m_current_event].Sound;
        current_page = &m_sounds_page;

        source = &EditorCustom::sound_list.names;
        source_indices = &EditorCustom::sound_list.indices;
    }
    else if(m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
    {
        if(m_special_subpage > 0)
        {
            titleSize = FontManager_printTextOptiPxR(mode,
                                                     fmt::format_ne(g_editorStrings.selectSectBlankPropBlankForEvent,
                                                                    m_special_subpage,
                                                                    g_editorStrings.eventsCaseBackground,
                                                                    Events[m_current_event].Name),
                                                     titlePosX, titlePosY,
                                                     300,
                                                     FontManager::fontIdFromSmbxFont(3));
            target = &Events[m_current_event].section[m_special_subpage-1].background_id;
        }
        else
        {
            titleSize = FontManager_printTextOptiPxR(mode,
                                                     fmt::format_ne(g_editorStrings.selectAllSectPropBlankForEvent,
                                                                    g_editorStrings.eventsCaseBackground,
                                                                    Events[m_current_event].Name),
                                                     titlePosX, titlePosY,
                                                     300,
                                                     FontManager::fontIdFromSmbxFont(3));
            target = nullptr;
        }

        current_page = &m_background_page;

        source = &EditorCustom::bg2_list.names;
        source_indices = &EditorCustom::bg2_list.indices;
    }
    else if(m_special_page == SPECIAL_PAGE_EVENT_MUSIC)
    {
        if(m_special_subpage > 0)
        {
            titleSize = FontManager_printTextOptiPxR(mode,
                                                     fmt::format_ne(g_editorStrings.selectSectBlankPropBlankForEvent,
                                                                    m_special_subpage,
                                                                    g_editorStrings.eventsCaseMusic,
                                                                    Events[m_current_event].Name),
                                                     titlePosX, titlePosY,
                                                     300,
                                                     FontManager::fontIdFromSmbxFont(3));
            target = &Events[m_current_event].section[m_special_subpage-1].music_id;
        }
        else
        {
            titleSize = FontManager_printTextOptiPxR(mode,
                                                     fmt::format_ne(g_editorStrings.selectAllSectPropBlankForEvent,
                                                                    g_editorStrings.eventsCaseMusic,
                                                                    Events[m_current_event].Name),
                                                     titlePosX, titlePosY,
                                                     300,
                                                     FontManager::fontIdFromSmbxFont(3));
            target = nullptr;
        }

        current_page = &m_music_page;

        source = &EditorCustom::music_list.names;
        source_indices = &EditorCustom::music_list.indices;
    }
    else if(m_special_page == SPECIAL_PAGE_SECTION_BACKGROUND)
    {
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.selectSectionBlankPropBlank, curSection + 1, g_editorStrings.eventsCaseBackground), 3, 10, 50);
        target = &Background2[curSection];
        current_page = &m_background_page;

        source = &EditorCustom::bg2_list.names;
        source_indices = &EditorCustom::bg2_list.indices;
    }
    else if(m_special_page == SPECIAL_PAGE_SECTION_MUSIC)
    {
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.selectSectionBlankPropBlank, curSection + 1, g_editorStrings.eventsCaseMusic), 3, 10, 50);
        target = &bgMusic[curSection];
        current_page = &m_music_page;

        source = &EditorCustom::music_list.names;
        source_indices = &EditorCustom::music_list.indices;
    }
    else if(m_special_page == SPECIAL_PAGE_LEVEL_EXIT)
    {
        using Controls::PlayerControls::Buttons;

        if(m_special_subpage == 1)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.selectPathBlankUnlock, GetButtonName_UI(Buttons::Up)), 3, 10, 50);
        else if(m_special_subpage == 2)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.selectPathBlankUnlock, GetButtonName_UI(Buttons::Left)), 3, 10, 50);
        else if(m_special_subpage == 3)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.selectPathBlankUnlock, GetButtonName_UI(Buttons::Down)), 3, 10, 50);
        else if(m_special_subpage == 4)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.selectPathBlankUnlock, GetButtonName_UI(Buttons::Right)), 3, 10, 50);
        else
            return;

        target = &EditorCursor.WorldLevel.LevelExit[m_special_subpage];
        current_page = nullptr;
        source = &EditorCustom::list_level_exit_names;
        source_indices = nullptr;
    }
    else if(m_special_page == SPECIAL_PAGE_WARP_TRANSITION)
    {
        SuperPrintR(mode, g_editorStrings.selectWarpTransitionEffect, 3, 10, 50);

        target = &EditorCursor.Warp.transitEffect;
        current_page = nullptr;
        source = &g_editorStrings.listWarpTransitNames;
        source_indices = nullptr;
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_MUSIC)
    {
        SuperPrintR(mode, g_editorStrings.selectWorldMusic, 3, 10, 50);
        target = &EditorCursor.WorldMusic.Type;
        current_page = &m_music_page;

        source = &EditorCustom::wmusic_list.names;
        source_indices = &EditorCustom::wmusic_list.indices;
    }
    else
    {
        return;
    }


    int elementsOnPage = 20;
    int elementsOnColumn = elementsOnPage / 2;
    int elementsListBaseY = 80;
    const int elementsRowHeight = 40;

    // Reduce the size of the elements list if title has more lines than expected
    while(titlePosY + titleSize.h() >= elementsListBaseY)
    {
        elementsOnPage -= 2;
        elementsOnColumn -= 1;
        elementsListBaseY += elementsRowHeight;
    }

    SDL_assert(elementsOnPage > 0);
    SDL_assert(elementsOnColumn > 0);

    if(current_page != nullptr)
    {
        int page_max = !source->empty() ? ((static_cast<int>(source->size()) - 1) / elementsOnPage) : 0;

        if(!(page_max == 0 && *current_page == 0))
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.pageBlankOfBlank, *current_page + 1, page_max + 1), 3, e_ScreenW - 320, 50);

        if(*current_page > 0 && UpdateButton(mode, e_ScreenW - 120 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            *current_page = *current_page - 1;

        if(*current_page < page_max && UpdateButton(mode, e_ScreenW - 80 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            *current_page = *current_page + 1;

        if(*current_page < 0)
            *current_page = 0;

        if(*current_page > page_max)
            *current_page = page_max;
    }

    for(int i = 0; i < elementsOnPage; i++)
    {
        int x = 10 + (e_ScreenW / 2) * (i / elementsOnColumn);
        int y = elementsListBaseY + elementsRowHeight * (i % elementsOnColumn);

        int j;

        if(current_page != nullptr)
            j = (*current_page * elementsOnPage) + i;
        else
            j = i;

        if(j >= (int)source->size())
            break;

        int index;
        if(source_indices != nullptr)
        {
            if(source_indices->empty())
                return; // Note: this is an invalid: array must not being empty!
            index = (*source_indices)[j];
        }
        else if(m_special_page == SPECIAL_PAGE_LEVEL_EXIT)
            index = j - 1;
        else
            index = j;

        if(index == -1 && m_special_page != SPECIAL_PAGE_LEVEL_EXIT)
            x += 40;

        SuperPrintR(mode, (*source)[j], 3, x + 44, y + 10);
        if(index != -1 || m_special_page == SPECIAL_PAGE_LEVEL_EXIT)
        {
            bool sel;
            if(target != nullptr)
                sel = *target == index;
            else if(m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
            {
                sel = true;
                for(int s = 0; s <= maxSections; s++)
                {
                    if(Events[m_current_event].section[s].background_id != index)
                        sel = false;
                }
            }
            else if(m_special_page == SPECIAL_PAGE_EVENT_MUSIC)
            {
                sel = true;
                for(int s = 0; s <= maxSections; s++)
                {
                    if(Events[m_current_event].section[s].music_id != index)
                        sel = false;
                }
            }
            else // should never happen
            {
                return;
            }

            if(UpdateButton(mode, x + 4, y + 4, GFX.ECursor[2], sel, 0, 0, 32, 32))
            {
                if(!sel)
                {
                    if(target != nullptr)
                        *target = index;
                    else if(m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
                    {
                        for(int s = 0; s <= maxSections; s++)
                            Events[m_current_event].section[s].background_id = index;
                    }
                    else if(m_special_page == SPECIAL_PAGE_EVENT_MUSIC)
                    {
                        for(int s = 0; s <= maxSections; s++)
                            Events[m_current_event].section[s].music_id = index;
                    }

                    // and do whatever necessary to preview it.
                    if(m_special_page == SPECIAL_PAGE_EVENT_SOUND && index != 0)
                        PlaySound(index);
                    else if(m_special_page == SPECIAL_PAGE_SECTION_MUSIC)
                        StartMusic(curSection);
                }
                else
                {
                    if(m_special_page == SPECIAL_PAGE_EVENT_SOUND || m_special_page == SPECIAL_PAGE_EVENT_MUSIC || m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
                        m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
                    else
                        m_special_page = SPECIAL_PAGE_NONE;
                }
            }
        }
    }
}

void EditorScreen::UpdateEventsSubScreen(CallMode mode)
{
    // render shared GUI elements on right
    if(mode == CallMode::Render)
        XRender::renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);

    // back button
    if(UpdateButton(mode, e_ScreenW - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
    {
        if(m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        else
            m_special_page = SPECIAL_PAGE_NONE;

        m_special_subpage = 0;
        return;
    }

    if(EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        // activate event
        SuperPrintR(mode, g_editorStrings.eventsLabelActivate, 3, e_ScreenW - 200, 200 + 2);

        if(EditorCursor.NPC.TriggerActivate != EVENT_NONE)
            SuperPrintR(mode, GetE(EditorCursor.NPC.TriggerActivate), 3, e_ScreenW - 200, 220 + 2);
        else
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 220 + 2);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 200 + 4, GFX.EIcons, m_special_subpage == 1, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 1;

        // death event
        SuperPrintR(mode, g_editorStrings.eventsLabelDeath, 3, e_ScreenW - 200, 240 + 2);

        if(EditorCursor.NPC.TriggerDeath != EVENT_NONE)
            SuperPrintR(mode, GetE(EditorCursor.NPC.TriggerDeath), 3, e_ScreenW - 200, 260 + 2);
        else
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 260 + 2);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 240 + 4, GFX.EIcons, m_special_subpage == 2, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 2;

        // talk event
        SuperPrintR(mode, g_editorStrings.eventsLabelTalk, 3, e_ScreenW - 200, 280 + 2);

        if(EditorCursor.NPC.TriggerTalk != EVENT_NONE)
            SuperPrintR(mode, GetE(EditorCursor.NPC.TriggerTalk), 3, e_ScreenW - 200, 300 + 2);
        else
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 300 + 2);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 280 + 4, GFX.EIcons, m_special_subpage == 3, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 3;

        // layer clear event
        SuperPrintR(mode, g_editorStrings.eventsLabelLayerClear, 3, e_ScreenW - 200, 320 + 2);

        if(EditorCursor.NPC.TriggerLast != EVENT_NONE)
            SuperPrintR(mode, GetE(EditorCursor.NPC.TriggerLast), 3, e_ScreenW - 200, 340 + 2);
        else
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 340 + 2);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 320 + 4, GFX.EIcons, m_special_subpage == 4, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 4;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_BLOCKS)
    {
        // block hit event
        SuperPrintR(mode, g_editorStrings.eventsLabelHit, 3, e_ScreenW - 200, 200 + 2);

        if(EditorCursor.Block.TriggerHit != EVENT_NONE)
            SuperPrintR(mode, GetE(EditorCursor.Block.TriggerHit), 3, e_ScreenW - 200, 220 + 2);
        else
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 220 + 2);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 200 + 4, GFX.EIcons, m_special_subpage == 1, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 1;

        // block destroy event
        SuperPrintR(mode, g_editorStrings.eventsLabelDestroy, 3, e_ScreenW - 200, 240 + 2);

        if(EditorCursor.Block.TriggerDeath != EVENT_NONE)
            SuperPrintR(mode, GetE(EditorCursor.Block.TriggerDeath), 3, e_ScreenW - 200, 260 + 2);
        else
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 260 + 2);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 240 + 4, GFX.EIcons, m_special_subpage == 2, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 2;

        // layer clear event
        SuperPrintR(mode, g_editorStrings.eventsLabelLayerClear, 3, e_ScreenW - 200, 280 + 2);

        if(EditorCursor.Block.TriggerLast != EVENT_NONE)
            SuperPrintR(mode, GetE(EditorCursor.Block.TriggerLast), 3, e_ScreenW - 200, 300 + 2);
        else
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 300 + 2);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 280 + 4, GFX.EIcons, m_special_subpage == 3, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 3;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS)
    {
        m_special_subpage = 1;

        SuperPrintR(mode, g_editorStrings.eventsLabelEnter, 3, e_ScreenW - 200, 200 + 2);
        if(EditorCursor.Warp.eventEnter != EVENT_NONE)
            SuperPrintR(mode, GetE(EditorCursor.Warp.eventEnter), 3, e_ScreenW - 200, 220 + 2);
        else
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 220 + 2);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 200 + 4, GFX.EIcons, m_special_subpage == 1, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 1;
    }
    else if(m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
    {
        // delay selector, instead of page selector
        if(Events[m_current_event].TriggerDelay > 0)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.phraseDelayIsMs, 100 * (int)Events[m_current_event].TriggerDelay), 3, e_ScreenW - 240 + 4, 272);
        else
            SuperPrintR(mode, g_editorStrings.wordInstant, 3, e_ScreenW - 240 + 4, 272);

        if(UpdateButton(mode, e_ScreenW - 240 + 4, 230 + 4, GFX.EIcons, false, 0, 32*Icon::up, 32, 32))
            Events[m_current_event].TriggerDelay ++;

        if(Events[m_current_event].TriggerDelay > 0 && UpdateButton(mode, e_ScreenW - 240 + 4, 290 + 4, GFX.EIcons, false, 0, 32*Icon::down, 32, 32))
            Events[m_current_event].TriggerDelay --;
    }
    else
        return;

    int page_max = numEvents / 10;
    SuperPrintR(mode, fmt::format_ne(g_editorStrings.pageBlankOfBlank, m_events_page + 1, page_max + 1), 3, e_ScreenW - 228, e_ScreenH - 60);
    if(m_events_page > 0 && UpdateButton(mode, e_ScreenW - 160 + 4, e_ScreenH - 40 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        m_events_page --;
    if(m_events_page < page_max && UpdateButton(mode, e_ScreenW - 120 + 4, e_ScreenH - 40 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        m_events_page ++;

    // prepare selector
    std::string event_name;
    std::string event_desc;
    eventindex_t* event_to_set;

    if(m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
    {
        event_name = g_editorStrings.eventsLabelNext;
        event_to_set = &Events[m_current_event].TriggerEvent;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        if(m_special_subpage == 1)
        {
            event_name = g_editorStrings.eventsLabelActivate;
            event_desc = g_editorStrings.eventsDescActivate;
            event_to_set = &EditorCursor.NPC.TriggerActivate;
        }
        else if(m_special_subpage == 2)
        {
            event_name = g_editorStrings.eventsLabelDeath;
            event_desc = g_editorStrings.eventsDescDeath;
            event_to_set = &EditorCursor.NPC.TriggerDeath;
        }
        else if(m_special_subpage == 3)
        {
            event_name = g_editorStrings.eventsLabelTalk;
            event_desc = g_editorStrings.eventsDescTalk;
            event_to_set = &EditorCursor.NPC.TriggerTalk;
        }
        else if(m_special_subpage == 4)
        {
            event_name = g_editorStrings.eventsLabelLayerClear;
            event_desc = g_editorStrings.eventsDescLayerClear;
            event_to_set = &EditorCursor.NPC.TriggerLast;
        }
        else
            return;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_BLOCKS)
    {
        if(m_special_subpage == 1)
        {
            event_name = g_editorStrings.eventsLabelHit;
            event_desc = g_editorStrings.eventsDescHit;
            event_to_set = &EditorCursor.Block.TriggerHit;
        }
        else if(m_special_subpage == 2)
        {
            event_name = g_editorStrings.eventsLabelDestroy;
            event_desc = g_editorStrings.eventsDescDestroy;
            event_to_set = &EditorCursor.Block.TriggerDeath;
        }
        else if(m_special_subpage == 3)
        {
            event_name = g_editorStrings.eventsLabelLayerClear;
            event_desc = g_editorStrings.eventsDescLayerClear;
            event_to_set = &EditorCursor.Block.TriggerLast;
        }
        else
            return;
    }
    else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS)
    {
        if(m_special_subpage == 1)
        {
            event_name = g_editorStrings.eventsLabelEnter;
            event_desc = g_editorStrings.eventsDescEnter;
            event_to_set = &EditorCursor.Warp.eventEnter;
        }
        else
            return;
    }
    else // this should not happen
    {
        return;
    }

    // render description
    if(mode == CallMode::Render)
    {
        // trigger after current event is a special case
        std::string descFormatted = (m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
            ? fmt::format(g_editorStrings.eventsDescPhraseTriggersAfterTemplate, event_name, (int)Events[m_current_event].TriggerDelay * 100, Events[m_current_event].Name)
            : fmt::format(g_editorStrings.eventsDescPhraseTriggersWhenTemplate, event_name, event_desc);

        FontManager::printTextOptiPx(descFormatted,
                                     e_ScreenW - 236, 80,
                                     230,
                                     FontManager::fontIdFromSmbxFont(3));
    }

    // render current event
    SuperPrintR(mode, fmt::format_ne(g_editorStrings.phraseTypeLabelEvent, event_name), 3, 10, 40);
    if(*event_to_set == EVENT_NONE)
        SuperPrintR(mode, g_mainMenu.caseNone, 3, 10, 56);
    else
        SuperPrintR(mode, Events[*event_to_set].Name, 3, 10, 56);

    // render event selector
    for(int i = 0; i < 10; i++)
    {
        eventindex_t e = m_events_page*10 + i - 1;
        if(e == EVENT_NONE)
        {
            SuperPrintR(mode, g_mainMenu.caseNone, 3, 54, 80 + 40*i + 12);
            if(UpdateButton(mode, 10 + 4, 80 + 40*i + 4, GFX.ECursor[2], *event_to_set == EVENT_NONE, 0, 0, 32, 32))
                *event_to_set = EVENT_NONE;
        }
        else if(!Events[e].Name.empty())
        {
            if(Events[e].Name.length() < 20)
                SuperPrintR(mode, Events[e].Name, 3, 54, 80 + 40*i + 10);
            else
            {
                SuperPrintR(mode, Events[e].Name.substr(0,19), 3, 54, 80 + 40*i + 2);
                SuperPrintR(mode, Events[e].Name.substr(19), 3, 54, 80 + 40*i + 20);
            }
            if(UpdateButton(mode, 10 + 4, 80 + 40*i + 4, GFX.ECursor[2], *event_to_set == e, 0, 0, 32, 32))
                *event_to_set = e;
        }
    }
}

void EditorScreen::UpdateLayersScreen(CallMode mode)
{
    if(m_special_page == SPECIAL_PAGE_LAYER_DELETION)
    {
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.layersDeletionHeader, Layer[m_special_subpage].Name), 3, 60, 40);
        SuperPrintR(mode, g_editorStrings.layersDeletionPreserveLayerContents, 3, 10, 60);

        SuperPrintR(mode, g_editorStrings.layersDeletionConfirmPreserve, 3, 60, 110);

        if(UpdateButton(mode, 20 + 4, 100 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            DeleteLayer((layerindex_t)m_special_subpage, false);
            m_special_subpage = 0;
            m_special_page = SPECIAL_PAGE_LAYERS;
            return;
        }


        SuperPrintR(mode, g_editorStrings.layersDeletionConfirmDelete, 3, 60, 150);

        if(UpdateButton(mode, 20 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            DeleteLayer((layerindex_t)m_special_subpage, true);
            m_special_subpage = 0;
            m_special_page = SPECIAL_PAGE_LAYERS;
            return;
        }


        SuperPrintR(mode, g_editorStrings.layersDeletionCancel, 3, 60, 190);

        if(UpdateButton(mode, 20 + 4, 180 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            m_special_subpage = 0;
            m_special_page = SPECIAL_PAGE_LAYERS;
        }

        return;
    }

    // fix just in case wrong subpage is set for obj layer selection
    if(m_special_page == SPECIAL_PAGE_OBJ_LAYER && EditorCursor.Mode != OptCursor_t::LVL_NPCS && m_special_subpage != 0)
        m_special_subpage = 0;

    // render shared GUI elements on right
    if(mode == CallMode::Render && m_special_page != SPECIAL_PAGE_LAYERS && m_special_page != SPECIAL_PAGE_EVENT_LAYERS)
        XRender::renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);

    if(m_special_page != SPECIAL_PAGE_LAYERS && UpdateButton(mode, e_ScreenW - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
    {
        if(m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        else
            m_special_page = SPECIAL_PAGE_NONE;
        m_special_subpage = 0;
    }

    if(m_special_page == SPECIAL_PAGE_OBJ_LAYER && EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        SuperPrintR(mode, g_editorStrings.labelLayer, 3, e_ScreenW - 200, 200 + 2);
        if(EditorCursor.Layer == LAYER_NONE)
            SuperPrintR(mode, g_editorStrings.layersLayerDefault, 3, e_ScreenW - 200, 220 + 2);
        else
            SuperPrintR(mode, GetL(EditorCursor.Layer), 3, e_ScreenW - 200, 220 + 2);
        if(UpdateButton(mode, e_ScreenW - 240 + 4, 200 + 4, GFX.EIcons, m_special_subpage == 0, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 0;

        SuperPrintR(mode, g_editorStrings.layersLabelAttached, 3, e_ScreenW - 200, 260 + 2);
        if(EditorCursor.NPC.AttLayer == LAYER_NONE || EditorCursor.NPC.AttLayer == LAYER_DEFAULT)
            SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 200, 280 + 2);
        else
            SuperPrintR(mode, GetL(EditorCursor.NPC.AttLayer), 3, e_ScreenW - 200, 280 + 2);
        if(UpdateButton(mode, e_ScreenW - 240 + 4, 260 + 4, GFX.EIcons, m_special_subpage == 1, 0, 32*Icon::action, 32, 32))
            m_special_subpage = 1;
    }

    int page_max;
    // extra slot for layer creation when not in the object layer selection
    if(m_special_page == SPECIAL_PAGE_OBJ_LAYER)
        page_max = (numLayers - 1) / 10;
    else
        page_max = numLayers / 10;

    // different location for layers and event layers page because they has so many options
    if(m_special_page == SPECIAL_PAGE_LAYERS || m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
    {
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.pageBlankOfBlank, m_layers_page + 1, page_max + 1), 3, e_ScreenW - 330, 40);
        if(m_layers_page > 0 && UpdateButton(mode, e_ScreenW - 120 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            m_layers_page --;
        if(m_layers_page < page_max && UpdateButton(mode, e_ScreenW - 80 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            m_layers_page ++;
    }
    else
    {
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.pageBlankOfBlank, m_layers_page + 1, page_max + 1), 3, e_ScreenW - 228, e_ScreenH - 60);
        if(m_layers_page > 0 && UpdateButton(mode, e_ScreenW - 160 + 4, e_ScreenH - 40 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            m_layers_page --;
        if(m_layers_page < page_max && UpdateButton(mode, e_ScreenW - 120 + 4, e_ScreenH - 40 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            m_layers_page ++;
    }

    // prepare selector
    const std::string* layer_name;
    layerindex_t* layer_to_set;
    if(m_special_page == SPECIAL_PAGE_OBJ_LAYER && m_special_subpage == 1)
    {
        layer_name = &g_editorStrings.layersLabelAttachedLayer;
        layer_to_set = &EditorCursor.NPC.AttLayer;
    }
    else if(m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
    {
        layer_name = &g_editorStrings.layersLabelMoveLayer;
        layer_to_set = &Events[m_current_event].MoveLayer;
    }
    else
    {
        layer_name = &g_editorStrings.labelLayer;
        layer_to_set = &EditorCursor.Layer;
    }

    // render description
    if(mode == CallMode::Render && m_special_page == SPECIAL_PAGE_OBJ_LAYER && m_special_subpage == 1)
    {
        FontManager::printTextOptiPx(g_editorStrings.layersDescAtt,
                                     e_ScreenW - 236, 80,
                                     230,
                                     FontManager::fontIdFromSmbxFont(3));
//        SuperPrintR(mode, g_editorStrings.layersDescAtt, 3, e_ScreenW - 236, 80);
//        SuperPrintR(mode, g_editorStrings.layersDescAtt2, 3, e_ScreenW - 236, 100);
//        SuperPrintR(mode, g_editorStrings.layersDescAtt3, 3, e_ScreenW - 236, 120);
//        SuperPrintR(mode, g_editorStrings.layersDescAtt4, 3, e_ScreenW - 236, 140);
//        SuperPrintR(mode, g_editorStrings.layersDescAtt5, 3, e_ScreenW - 236, 160);
    }

    // render current layer
    SuperPrintR(mode, *layer_name, 3, 10, 40);
    if(*layer_to_set == LAYER_NONE)
    {
        if(m_special_subpage == 1 || m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
            SuperPrintR(mode, g_mainMenu.caseNone, 3, 10, 56);
        else
            SuperPrintR(mode, g_editorStrings.layersLayerDefault, 3, 10, 56);
    }
    else
        SuperPrintR(mode, GetL(*layer_to_set), 3, 10, 56);

    // render layer selector
    for(int i = 0; i < 10; i++)
    {
        int l;
        // separate Default and None for Event layers
        if(m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
            l = m_layers_page*10 + i - 1;
        else
            l = m_layers_page*10 + i;

        if(l > maxLayers)
            continue;

        // separate None for Event layers
        if(l == -1)
        {
            SuperPrintR(mode, g_mainMenu.caseNone, 3, 54, 80 + 40*i + 12);

            if(m_special_page != SPECIAL_PAGE_EVENT_LAYERS && UpdateButton(mode, 10 + 4, 80 + 40*i + 4, GFX.ECursor[2], *layer_to_set == LAYER_NONE, 0, 0, 32, 32))
                *layer_to_set = LAYER_NONE;
            if(m_special_page == SPECIAL_PAGE_EVENT_LAYERS && UpdateButton(mode, 10 + 4, 80 + 40*i + 4, GFX.EIcons, *layer_to_set == LAYER_NONE, 0, 32*Icon::move, 32, 32))
                *layer_to_set = LAYER_NONE;
        }
        // Default and None are the same for objects layers and for the editor cursor -- set to None if "Default".
        else if(l == 0 && (m_special_page == SPECIAL_PAGE_OBJ_LAYER || m_special_page == SPECIAL_PAGE_LAYERS))
        {
            // AttLayer
            if(m_special_page == SPECIAL_PAGE_OBJ_LAYER && m_special_subpage == 1)
                SuperPrintR(mode, g_mainMenu.caseNone, 3, 54, 80 + 40*i + 12);
            else
                SuperPrintR(mode, g_editorStrings.layersLayerDefault, 3, 54, 80 + 40*i + 12);

            if(UpdateButton(mode, 10 + 4, 80 + 40*i + 4, GFX.ECursor[2], *layer_to_set == LAYER_NONE || (*layer_to_set) == l, 0, 0, 32, 32))
                *layer_to_set = LAYER_NONE;
        }
        else if(!Layer[l].Name.empty())
        {
            if(Layer[l].Name.length() < 20)
                SuperPrintR(mode, Layer[l].Name, 3, 54, 80 + 40*i + 10);
            else
            {
                SuperPrintR(mode, Layer[l].Name.substr(0,19), 3, 54, 80 + 40*i + 2);
                SuperPrintR(mode, Layer[l].Name.substr(19), 3, 54, 80 + 40*i + 20);
            }

            if(m_special_page != SPECIAL_PAGE_EVENT_LAYERS && UpdateButton(mode, 10 + 4, 80 + 40*i + 4, GFX.ECursor[2], (*layer_to_set) == l, 0, 0, 32, 32))
                *layer_to_set = l;

            // extra buttons for layers page
            if(m_special_page == SPECIAL_PAGE_LAYERS)
            {
                // rename, hide/show, shift up, shift down, delete
                // hide/show
                if(UpdateButton(mode, 440 + 4, 80 + 40*i + 4, GFX.EIcons, !Layer[l].Hidden, 0, 32*Icon::show, 32, 32))
                {
                    if(Layer[l].Hidden)
                        ShowLayer(l);
                    else
                        HideLayer(l);
                }

                if(l <= 2)
                    continue;

                // rename
                if(UpdateButton(mode, 400 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
                {
                    DisableCursorNew();
                    std::string new_name = TextEntryScreen::Run(g_editorStrings.layersPromptLayerName, Layer[l].Name);
                    if(!new_name.empty())
                        RenameLayer(l, new_name);
                    MouseMove((float)SharedCursor.X, (float)SharedCursor.Y);
                }

                // shift up
                if(l > 3 && UpdateButton(mode, 480 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::up, 32, 32))
                    SwapLayers(l-1, l);

                // shift down
                if(l < numLayers - 1 && UpdateButton(mode, 520 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::down, 32, 32))
                    SwapLayers(l, l+1);

                // delete
                if(l < numLayers && UpdateButton(mode, 560 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
                {
                    m_special_page = SPECIAL_PAGE_LAYER_DELETION;
                    m_special_subpage = l;
                    return;
                }
            }
            // extra buttons for event layers
            else if(m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
            {
                // nothing, hide, show, toggle
                auto hide_it = std::find(Events[m_current_event].HideLayer.begin(), Events[m_current_event].HideLayer.end(), l);
                auto show_it = std::find(Events[m_current_event].ShowLayer.begin(), Events[m_current_event].ShowLayer.end(), l);
                auto togg_it = std::find(Events[m_current_event].ToggleLayer.begin(), Events[m_current_event].ToggleLayer.end(), l);
                bool cur_hide = hide_it != Events[m_current_event].HideLayer.end();
                bool cur_show = show_it != Events[m_current_event].ShowLayer.end();
                bool cur_togg = togg_it != Events[m_current_event].ToggleLayer.end();

                // no change
                if(UpdateButton(mode, 400 + 4, 80 + 40*i + 4, GFX.EIcons, !cur_hide && !cur_show && !cur_togg, 0, 0, 1, 1))
                {
                    if(cur_hide) Events[m_current_event].HideLayer.erase(hide_it);
                    if(cur_show) Events[m_current_event].ShowLayer.erase(show_it);
                    if(cur_togg) Events[m_current_event].ToggleLayer.erase(togg_it);
                }

                // show layer
                if(UpdateButton(mode, 440 + 4, 80 + 40*i + 4, GFX.EIcons, cur_show, 0, 32*Icon::show, 32, 32))
                {
                    if(cur_hide) Events[m_current_event].HideLayer.erase(hide_it);
                    if(!cur_show) Events[m_current_event].ShowLayer.push_back(l);
                    if(cur_togg) Events[m_current_event].ToggleLayer.erase(togg_it);
                }

                // hide layer
                if(UpdateButton(mode, 480 + 4, 80 + 40*i + 4, GFX.EIcons, cur_hide, 0, 32*Icon::hide, 32, 32))
                {
                    if(!cur_hide) Events[m_current_event].HideLayer.push_back(l);
                    if(cur_show) Events[m_current_event].ShowLayer.erase(show_it);
                    if(cur_togg) Events[m_current_event].ToggleLayer.erase(togg_it);
                }

                // togg layer
                if(UpdateButton(mode, 520 + 4, 80 + 40*i + 4, GFX.EIcons, cur_togg, 0, 32*Icon::toggle, 32, 32))
                {
                    if(cur_hide) Events[m_current_event].HideLayer.erase(hide_it);
                    if(cur_show) Events[m_current_event].ShowLayer.erase(show_it);
                    if(!cur_togg) Events[m_current_event].ToggleLayer.push_back(l);
                }

                if(UpdateButton(mode, 10 + 4, 80 + 40*i + 4, GFX.EIcons, (*layer_to_set) == l, 0, 32*Icon::move, 32, 32))
                    *layer_to_set = l;
            }
        }
        // create a new layer!
        else if(m_special_page == SPECIAL_PAGE_LAYERS && l != 0 && !Layer[l - 1].Name.empty())
        {
            SuperPrintR(mode, g_editorStrings.layersItemNewLayer, 3, 54, 80 + 40*i + 10);
            // rename only
            if(UpdateButton(mode, 400 + 4, 80 + 40*i + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
            {
                DisableCursorNew();
                std::string new_name = TextEntryScreen::Run(g_editorStrings.layersPromptLayerName, "");
                if(!new_name.empty() && FindLayer(new_name) == LAYER_NONE)
                {
                    Layer[l] = Layer_t();
                    Layer[l].Name = new_name;
                    numLayers ++;
                }
                MouseMove((float)SharedCursor.X, (float)SharedCursor.Y);
            }
        }
    }
}

bool EditorScreen::UpdateBlockButton(CallMode mode, int x, int y, int type, bool sel)
{
    int draw_width = 32, draw_height = 32;

    if(!BlockIsSizable[type])
    {
        if(BlockWidth[type] > 0)
            draw_width = BlockWidth[type];
        if(BlockHeight[type] > 0)
            draw_height = BlockHeight[type];
    }

    return UpdateButton(mode, x, y, GFXBlock[type], sel, 0, BlockFrame[type] * 32, draw_width, draw_height) && !sel;
}

void EditorScreen::UpdateBlock(CallMode mode, int x, int y, int type)
{
    if((type < 1) || (type >= maxBlockType))
        return;
    bool sel = EditorCursor.Block.Type == type;
    if(UpdateBlockButton(mode, x, y, type, sel) && !sel)
    {
        // printf("Block %d\n", type);
        SetEditorBlockType(type);
    }
}

void EditorScreen::UpdateBlockGrid(CallMode mode, int x, int y, const int* types, int n_blocks, int n_cols)
{
    for(int i = 0; i < n_blocks; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateBlock(mode, x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateBlockScreen(CallMode mode)
{
    // Block GUI
    if(mode == CallMode::Render)
    {
        XRender::renderRect(e_ScreenW - 160, 40, 160, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);
        XRender::renderRect(0, 40, 40, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);
        XRender::renderRect(38, 40, 2, e_ScreenH - 40, 0.25f, 0.0f, 0.5f, 1.0f, true);
    }

    // Page selector
    int last_category = -1;
    int index = 0;

    for(const EditorCustom::ItemPage_t& page : EditorCustom::block_pages)
    {
        if(page.category != last_category)
        {
            last_category = page.category;

            if(mode == CallMode::Render && index != 0)
                XRender::renderRect(0, 40 + -2 + (40 * index), 40, 4, 0.25f, 0.0f, 0.5f, 1.0f, true);
        }

        index++;

        if(UpdateBlockButton(mode, 4, 4 + (40 * index), page.icon, m_Block_page == index))
            m_Block_page = index;
    }

    // Magic Block toggle
    if(mode == CallMode::Render)
    {
        FontManager::printTextOptiPx(g_editorStrings.toggleMagicBlock,
                                     e_ScreenW - 120, 42,
                                     120,
                                     FontManager::fontIdFromSmbxFont(3));
    }
    if(UpdateButton(mode, e_ScreenW - 160 + 4, 40 + 4, GFXNPC[NPCID_COIN_SWITCH], MagicBlock::enabled, 0, 0, 32, 32))
        MagicBlock::enabled = !MagicBlock::enabled;

    // Resizing
    if(BlockIsSizable[EditorCursor.Block.Type])
    {
        int H = ((int)EditorCursor.Block.Location.Height)/32;
        int W = ((int)EditorCursor.Block.Location.Width)/32;
        SuperPrintR(mode, g_editorStrings.blockLetterWidth  + std::to_string(W), 3, e_ScreenW - 160, 112);
        if(W > 2 && UpdateButton(mode, e_ScreenW - 80 + 4, 100 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            EditorCursor.Block.Location.Width = 32 * (W - 1);
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 100 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            EditorCursor.Block.Location.Width = 32 * (W + 1);
        SuperPrintR(mode, g_editorStrings.blockLetterHeight + std::to_string(H), 3, e_ScreenW - 160, 152);
        if(H > 2 && UpdateButton(mode, e_ScreenW - 80 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            EditorCursor.Block.Location.Height = 32 * (H - 1);
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            EditorCursor.Block.Location.Height = 32 * (H + 1);
    }

    // Legacy for Spin Block
    if(FileFormat == FileFormats::LVL_PGEX && EditorCursor.Block.Type == 90)
    {
        SuperPrintR(mode, g_editorStrings.blockCanBreak, 3, e_ScreenW - 160, 106);
        if(UpdateButton(mode, e_ScreenW - 40 + 4, 120 + 4, GFXBlock[188], EditorCursor.Block.forceSmashable, 0, 0, 32, 32, g_editorStrings.blockTooltipCanBreak.c_str()))
            EditorCursor.Block.forceSmashable = !EditorCursor.Block.forceSmashable;
    }
    else
    {
        EditorCursor.Block.forceSmashable = 0;
    }

    // Slippy ("SLICK") and Invis
    SuperPrintRightR(mode, g_editorStrings.blockSlick, 3, e_ScreenW - 40, 214);
    if(UpdateCheckBox(mode, e_ScreenW - 40 + 4, 200 + 4, EditorCursor.Block.Slippy))
        EditorCursor.Block.Slippy = !EditorCursor.Block.Slippy;
    SuperPrintRightR(mode, g_editorStrings.blockInvis, 3, e_ScreenW - 40, 254);
    if(UpdateCheckBox(mode, e_ScreenW - 40 + 4, 240 + 4, EditorCursor.Block.Invis))
        EditorCursor.Block.Invis = !EditorCursor.Block.Invis;

    // Contents
    SuperPrintRightR(mode, g_editorStrings.blockInside, 3, e_ScreenW - 40, 294);
    int n_type = 0;
    if(EditorCursor.Block.Special > 0 && EditorCursor.Block.Special <= 1000)
    {
        n_type = 10;
        SuperPrintR(mode, "x" + std::to_string(EditorCursor.Block.Special), 3, e_ScreenW-80, 314);
    }
    else if(EditorCursor.Block.Special != 0)
    {
        n_type = EditorCursor.Block.Special - 1000;
    }

    if(((n_type >= 1 && n_type <= maxNPCType) && UpdateNPCButton(mode, e_ScreenW - 40 + 4, 280 + 4, n_type, EditorCursor.Block.Special != 0))
        || ((n_type < 1 || n_type > maxNPCType) && UpdateButton(mode, e_ScreenW - 40 + 4, 280 + 4, GFX.EIcons, EditorCursor.Block.Special != 0, 0, 32 * Icon::subscreen, 32, 32)))
    {
        m_special_page = SPECIAL_PAGE_BLOCK_CONTENTS;
        FocusNPC();
    }

    // Events
    SuperPrintRightR(mode, g_editorStrings.eventsHeader, 3, e_ScreenW - 40, 334);
    SuperPrintR(mode, g_editorStrings.eventsLetterHit + GetE(EditorCursor.Block.TriggerHit), 3, e_ScreenW - 160, 360);
    SuperPrintR(mode, g_editorStrings.eventsLetterDestroy + GetE(EditorCursor.Block.TriggerDeath), 3, e_ScreenW - 160, 380);
    SuperPrintR(mode, g_editorStrings.eventsLetterLayerClear + GetE(EditorCursor.Block.TriggerLast), 3, e_ScreenW - 160, 400);
    if(UpdateButton(mode, e_ScreenW - 40 + 4, 320 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_TRIGGERS;

    // Layers
    SuperPrintRightR(mode, g_editorStrings.labelLayer, 3, e_ScreenW - 40, 434);
    if(EditorCursor.Block.Layer == LAYER_NONE)
        SuperPrintR(mode, g_editorStrings.layersLayerDefault, 3, e_ScreenW - 160, 460);
    else
        SuperPrintR(mode, GetL(EditorCursor.Block.Layer), 3, e_ScreenW - 160, 460);
    if(UpdateButton(mode, e_ScreenW - 40 + 4, 420 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_LAYER;

    // block selector
    if(m_Block_page > 0 && m_Block_page <= (int)EditorCustom::block_pages.size())
    {
        const EditorCustom::ItemPage_t& page = EditorCustom::block_pages[m_Block_page - 1];

        for(auto it = page.begin; it != page.end; ++it)
        {
            const EditorCustom::ItemFamily& family = **it;

            if(mode == CallMode::Render)
            {
                int pix_len = SuperTextPixLen(family.name, 3);

                if(40 + family.X * 40 + pix_len > e_ScreenW - 160)
                    SuperPrint(family.name, 3, e_ScreenW - 160 - 4 - pix_len, 40 + family.Y * 20);
                else
                    SuperPrint(family.name, 3, 40 + family.X * 40 + 4, 40 + family.Y * 20);
            }

            UpdateBlockGrid(mode, 40 + family.X * 40, 60 + family.Y * 20, family.layout_pod.types.data(), family.layout_pod.types.size(), family.layout_pod.cols);
        }
    }
}

bool EditorScreen::UpdateBGOButton(CallMode mode, int x, int y, int type, bool sel)
{
    return UpdateButton(mode, x, y, GFXBackgroundBMP[type], sel, 0, BackgroundFrame[type] * BackgroundHeight[type], GFXBackgroundWidth[type], BackgroundHeight[type]);
}

void EditorScreen::UpdateBGO(CallMode mode, int x, int y, int type)
{
    if((type < 1) || (type >= maxBackgroundType))
        return;
    bool sel = EditorCursor.Background.Type == type;
    if(UpdateBGOButton(mode, x, y, type, sel) && !sel)
    {
        EditorCursor.Background.Type = type;
    }
}

void EditorScreen::UpdateBGOGrid(CallMode mode, int x, int y, const int* types, int n_bgos, int n_cols)
{
    for(int i = 0; i < n_bgos; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateBGO(mode, x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateBGOScreen(CallMode mode)
{
    // BGO GUI
    if(mode == CallMode::Render)
    {
        XRender::renderRect(e_ScreenW - 160, 40, 160, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);
        XRender::renderRect(0, 40, 40, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);
        XRender::renderRect(38, 40, 2, e_ScreenH - 40, 0.25f, 0.0f, 0.5f, 1.0f, true);
    }


    // Page selector
    int last_category = -1;
    int index = 0;

    for(const EditorCustom::ItemPage_t& page : EditorCustom::bgo_pages)
    {
        if(page.category != last_category)
        {
            last_category = page.category;

            if(mode == CallMode::Render && index != 0)
                XRender::renderRect(0, 40 + -2 + (40 * index), 40, 4, 0.25f, 0.0f, 0.5f, 1.0f, true);
        }

        index++;

        if(UpdateBGOButton(mode, 4, 4 + (40 * index), page.icon, m_BGO_page == index))
            m_BGO_page = index;
    }

    // Magic Block toggle
    if(mode == CallMode::Render)
    {
        FontManager::printTextOptiPx(g_editorStrings.toggleMagicBlock,
                                     e_ScreenW - 120, 42,
                                     120,
                                     FontManager::fontIdFromSmbxFont(3));
    }
    if(UpdateButton(mode, e_ScreenW - 160 + 4, 40 + 4, GFXNPC[NPCID_COIN_SWITCH], MagicBlock::enabled, 0, 0, 32, 32))
        MagicBlock::enabled = !MagicBlock::enabled;

    // Layers
    SuperPrintRightR(mode, g_editorStrings.labelLayer, 3, e_ScreenW - 40, 434);
    if(EditorCursor.Background.Layer == LAYER_NONE)
        SuperPrintR(mode, g_editorStrings.layersLayerDefault, 3, e_ScreenW - 160, 460);
    else
        SuperPrintR(mode, GetL(EditorCursor.Background.Layer), 3, e_ScreenW - 160, 460);

    if(UpdateButton(mode, e_ScreenW - 40 + 4, 420 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_LAYER;

    // BGO selector
    if(m_BGO_page > 0 && m_BGO_page <= (int)EditorCustom::bgo_pages.size())
    {
        const EditorCustom::ItemPage_t& page = EditorCustom::bgo_pages[m_BGO_page - 1];

        for(auto it = page.begin; it != page.end; ++it)
        {
            const EditorCustom::ItemFamily& family = **it;

            if(mode == CallMode::Render)
            {
                int pix_len = SuperTextPixLen(family.name, 3);

                if(40 + family.X * 40 + pix_len > e_ScreenW - 160)
                    SuperPrint(family.name, 3, e_ScreenW - 160 - 4 - pix_len, 40 + family.Y * 20);
                else
                    SuperPrint(family.name, 3, 40 + family.X * 40 + 4, 40 + family.Y * 20);
            }

            UpdateBGOGrid(mode, 40 + family.X * 40, 60 + family.Y * 20, family.layout_pod.types.data(), family.layout_pod.types.size(), family.layout_pod.cols);
        }
    }
}

void EditorScreen::UpdateWaterScreen(CallMode mode)
{
    SuperPrintR(mode, g_editorStrings.waterTitle, 3, 200, 50);

    int H = ((int)EditorCursor.Water.Location.Height)/32;
    int W = ((int)EditorCursor.Water.Location.Width)/32;

    // validation
    if(H == 0)
    {
        H = 1;
        EditorCursor.Water.Location.Height = 32;
    }

    if(W == 0)
    {
        W = 1;
        EditorCursor.Water.Location.Width = 32;
    }

    // Width
    if(W >= 10)
        SuperPrintRightR(mode, g_editorStrings.wordWidth + " " + std::to_string(W), 3, 330, 90);
    else
        SuperPrintRightR(mode, g_editorStrings.wordWidth + " " + std::to_string(W), 3, 330, 90);

    if(W > 1 && UpdateButton(mode, 340 + 4, 80 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        EditorCursor.Water.Location.Width = 32 * (W - 1);

    if(UpdateButton(mode, 380 + 4, 80 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        EditorCursor.Water.Location.Width = 32 * (W + 1);

    // Height
    if(H >= 10)
        SuperPrintRightR(mode, g_editorStrings.wordHeight + " " + std::to_string(H), 3, 330, 130);
    else
        SuperPrintRightR(mode, g_editorStrings.wordHeight + " " + std::to_string(H), 3, 330, 130);

    if(H > 1 && UpdateButton(mode, 340 + 4, 120 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        EditorCursor.Water.Location.Height = 32 * (H - 1);

    if(UpdateButton(mode, 380 + 4, 120 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        EditorCursor.Water.Location.Height = 32 * (H + 1);

    // mode
    SuperPrintRightR(mode, g_editorStrings.wordMode, 3, 330, 170);

    if(UpdateButton(mode, 340 + 4, 160 + 4, GFXBackgroundBMP[26], !EditorCursor.Water.Quicksand, 0, 0, 32, 32))
        EditorCursor.Water.Quicksand = false;

    if(UpdateButton(mode, 380 + 4, 160 + 4, GFXBackgroundBMP[188], EditorCursor.Water.Quicksand, 0, 0, 32, 32))
        EditorCursor.Water.Quicksand = true;

    // layers
    SuperPrintRightR(mode, g_editorStrings.labelLayer, 3, 330, 234);
    if(EditorCursor.Layer == LAYER_NONE)
        SuperPrintR(mode, g_editorStrings.layersLayerDefault, 3, 206, 260);
    else
        SuperPrintR(mode, GetL(EditorCursor.Layer), 3, 206, 260);

    if(UpdateButton(mode, 340 + 4, 220 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_LAYER;
}

void EditorScreen::UpdateWarpScreen(CallMode mode)
{
    // Warp GUI
    if(mode == CallMode::Render)
        XRender::renderRect(0, 40, e_ScreenW - 240, 140, 0.7f, 0.7f, 0.9f, 0.75f, true);
    if(mode == CallMode::Render)
        XRender::renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);

    SuperPrintR(mode, g_editorStrings.warpTitle, 3, 200, 50);

    // prep for placement selection
    if(EditorCursor.Warp.level == STRINGINDEX_NONE && !EditorCursor.Warp.LevelEnt && !EditorCursor.Warp.MapWarp)
    {
        if(EditorCursor.SubMode != 1 && EditorCursor.SubMode != 2)
            EditorCursor.SubMode = 1;
    }
    else if(!EditorCursor.Warp.LevelEnt)
    {
        EditorCursor.Warp.twoWay = false;
        EditorCursor.SubMode = 1;
    }
    else
    {
        EditorCursor.Warp.twoWay = false;
        EditorCursor.SubMode = 2;
    }

    // placement selection and directions
    if(!EditorCursor.Warp.LevelEnt)
    {
        SuperPrintR(mode, g_editorStrings.warpPlacing, 3, 40, 82);

        SuperPrintRightR(mode, g_editorStrings.warpIn, 3, 80, 114);
        if(UpdateButton(mode, 80 + 4, 100 + 4, GFX.EIcons, EditorCursor.SubMode == 1, 0, 32*Icon::action, 32, 32))
            EditorCursor.SubMode = 1;

        SuperPrintRightR(mode, g_editorStrings.warpDir, 3, 220, 114);

        if(UpdateButton(mode, 220 + 4, 100 + 4, GFX.EIcons, EditorCursor.Warp.Direction == 1, 0, 32*Icon::up, 32, 32))
            EditorCursor.Warp.Direction = 1;
        if(UpdateButton(mode, 260 + 4, 100 + 4, GFX.EIcons, EditorCursor.Warp.Direction == 3, 0, 32*Icon::down, 32, 32))
            EditorCursor.Warp.Direction = 3;
        if(UpdateButton(mode, 300 + 4, 100 + 4, GFX.EIcons, EditorCursor.Warp.Direction == 2, 0, 32*Icon::left, 32, 32))
            EditorCursor.Warp.Direction = 2;
        if(UpdateButton(mode, 340 + 4, 100 + 4, GFX.EIcons, EditorCursor.Warp.Direction == 4, 0, 32*Icon::right, 32, 32))
            EditorCursor.Warp.Direction = 4;
    }
    else
    {
        SuperPrintR(mode, g_editorStrings.warpPlacing, 3, 40, 122);
    }

    if(EditorCursor.Warp.level == STRINGINDEX_NONE && !EditorCursor.Warp.MapWarp)
    {
        SuperPrintRightR(mode, g_editorStrings.warpOut, 3, 80, 154);
        if(UpdateButton(mode, 80 + 4, 140 + 4, GFX.EIcons, EditorCursor.SubMode == 2, 0, 32*Icon::action, 32, 32))
            EditorCursor.SubMode = 2;

        SuperPrintRightR(mode, g_editorStrings.warpDir, 3, 220, 154);

        if(UpdateButton(mode, 220 + 4, 140 + 4, GFX.EIcons, EditorCursor.Warp.Direction2 == 1, 0, 32*Icon::down, 32, 32))
            EditorCursor.Warp.Direction2 = 1;
        if(UpdateButton(mode, 260 + 4, 140 + 4, GFX.EIcons, EditorCursor.Warp.Direction2 == 3, 0, 32*Icon::up, 32, 32))
            EditorCursor.Warp.Direction2 = 3;
        if(UpdateButton(mode, 300 + 4, 140 + 4, GFX.EIcons, EditorCursor.Warp.Direction2 == 2, 0, 32*Icon::right, 32, 32))
            EditorCursor.Warp.Direction2 = 2;
        if(UpdateButton(mode, 340 + 4, 140 + 4, GFX.EIcons, EditorCursor.Warp.Direction2 == 4, 0, 32*Icon::left, 32, 32))
            EditorCursor.Warp.Direction2 = 4;
    }

    if(EditorCursor.Warp.level == STRINGINDEX_NONE && !EditorCursor.Warp.LevelEnt && !EditorCursor.Warp.MapWarp)
    {
        SuperPrintR(mode, g_editorStrings.warpTwoWay, 3, e_ScreenW - 240 + 40 + 4, 134);
        if(UpdateCheckBox(mode, e_ScreenW - 240 + 4, 120 + 4, EditorCursor.Warp.twoWay))
            EditorCursor.Warp.twoWay = !EditorCursor.Warp.twoWay;

    }

    // Events
    if(FileFormat == FileFormats::LVL_PGEX)
    {
        if(mode == CallMode::Render)
        {
            SuperPrintRightAlign(g_editorStrings.eventsHeader, 3, e_ScreenW - 80, 334);
            SuperPrint(g_editorStrings.eventsLetterEnter + GetE(EditorCursor.Warp.eventEnter), 3, e_ScreenW - 240, 360);
        }
        if(UpdateButton(mode, e_ScreenW - 80 + 4, 320 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
            m_special_page = SPECIAL_PAGE_OBJ_TRIGGERS;
    }

    // Layers
    if(mode == CallMode::Render)
    {
        SuperPrintRightAlign(g_editorStrings.labelLayer, 3, e_ScreenW - 80, 414);
        if(EditorCursor.Warp.Layer == LAYER_NONE)
            SuperPrint(g_editorStrings.layersLayerDefault, 3, e_ScreenW - 240, 440);
        else
            SuperPrint(GetL(EditorCursor.Warp.Layer), 3, e_ScreenW - 240, 440);
    }

    if(UpdateButton(mode, e_ScreenW - 80 + 4, 400 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_LAYER;

    // page selector
    if(UpdateButton(mode, e_ScreenW - 240 + 4, 180 + 4, GFXBlock[294], m_Warp_page == WARP_PAGE_MAIN, 0, 0, 32, 32))
        m_Warp_page = WARP_PAGE_MAIN;
    if(UpdateButton(mode, e_ScreenW - 240 + 4, 220 + 4, GFXNPC[31], m_Warp_page == WARP_PAGE_REQS, 0, 0, 32, 32))
        m_Warp_page = WARP_PAGE_REQS;
    if(UpdateButton(mode, e_ScreenW - 240 + 4, 260 + 4, GFXLevelBMP[2], m_Warp_page == WARP_PAGE_LEVEL, 0, 0, 32, 32))
        m_Warp_page = WARP_PAGE_LEVEL;


    // MAIN WARP PAGE

    if(m_Warp_page == WARP_PAGE_MAIN)
    {
        std::string* style_name;

        // warp effect
        switch(EditorCursor.Warp.Effect)
        {
        default:
            EditorCursor.Warp.Effect = 1;
        //fallthrough
        case 1:
            style_name = &g_editorStrings.warpStylePipe;
            break;
        case 2:
            style_name = &g_editorStrings.warpStyleDoor;
            break;
        case 0:
            style_name = &g_editorStrings.warpStyleBlipInstant;
            break;
        case 3:
            style_name = &g_editorStrings.warpStylePortal;
            break;
        }

        SuperPrintR(mode, g_editorStrings.warpStyle + *style_name, 3, 6, 194);

        if(UpdateButton(mode, 220 + 4, 180 + 4, GFXBlock[294], EditorCursor.Warp.Effect == 1, 0, 0, 32, 32))
            EditorCursor.Warp.Effect = 1;

        if(UpdateButton(mode, 260 + 4, 180 + 4, GFXBackgroundBMP[88], EditorCursor.Warp.Effect == 2, 0, 0, 32, 32))
        {
            EditorCursor.Warp.Effect = 2;
            EditorCursor.Warp.Direction = 1;
            EditorCursor.Warp.Direction2 = 1;
        }

        if(UpdateButton(mode, 300 + 4, 180 + 4, GFXBackgroundBMP[61], EditorCursor.Warp.Effect == 0, 0, 0, 32, 32))
            EditorCursor.Warp.Effect = 0;

        if(FileFormat == FileFormats::LVL_PGEX && UpdateButton(mode, 340 + 4, 180 + 4, GFXNPC[NPCID_HOMING_BALL], EditorCursor.Warp.Effect == 3, 0, 0, 32, 32))
            EditorCursor.Warp.Effect = 3;

        // fade effect
        if(FileFormat == FileFormats::LVL_PGEX)
        {
            if(EditorCursor.Warp.transitEffect < 0 || EditorCursor.Warp.transitEffect >= (int)g_editorStrings.listWarpTransitNames.size())
                EditorCursor.Warp.transitEffect = 0;

            SuperPrintR(mode, g_editorStrings.warpEffect, 3, 6, 234);
            if(mode == CallMode::Render)
                SuperPrintRightAlign(g_editorStrings.listWarpTransitNames[EditorCursor.Warp.transitEffect], 3, 256, 234);

            if(UpdateButton(mode, 260 + 4, 220 + 4, GFX.EIcons, EditorCursor.Warp.transitEffect, 0, 32*Icon::subscreen, 32, 32))
                m_special_page = SPECIAL_PAGE_WARP_TRANSITION;
        }

        // allow / forbid
        SuperPrintR(mode, g_editorStrings.warpAllow, 3, 6, 294);

        SuperPrintRightR(mode, g_editorStrings.warpItem, 3, 220, 294);
        if(UpdateCheckBox(mode, 220 + 4, 280 + 4, EditorCursor.Warp.WarpNPC))
            EditorCursor.Warp.WarpNPC = !EditorCursor.Warp.WarpNPC;

        SuperPrintRightR(mode, g_editorStrings.warpRide, 3, 360, 294);
        if(UpdateCheckBox(mode, 360 + 4, 280 + 4, !EditorCursor.Warp.NoYoshi))
            EditorCursor.Warp.NoYoshi = !EditorCursor.Warp.NoYoshi;

        // cannon exit
        if(FileFormat == FileFormats::LVL_PGEX)
        {
            SuperPrintR(mode, g_editorStrings.warpCannonExit, 3, 6, 354);

            if(UpdateCheckBox(mode, 220 + 4, 340 + 4, EditorCursor.Warp.cannonExit))
            {
                EditorCursor.Warp.cannonExit = !EditorCursor.Warp.cannonExit;
                if(!EditorCursor.Warp.cannonExit)
                    EditorCursor.Warp.cannonExitSpeed = Warp_t().cannonExitSpeed;
            }

            if(EditorCursor.Warp.cannonExit)
            {
                SuperPrintR(mode, g_editorStrings.warpSpeed + std::to_string(vb6Round(EditorCursor.Warp.cannonExitSpeed)), 3, 26, 394);
                if(EditorCursor.Warp.cannonExitSpeed > 1 && UpdateButton(mode, 180 + 4, 380 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
                    EditorCursor.Warp.cannonExitSpeed --;
                if(EditorCursor.Warp.cannonExitSpeed <= 31 && UpdateButton(mode, 220 + 4, 380 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
                    EditorCursor.Warp.cannonExitSpeed ++;
            }
        }
    }

    if(m_Warp_page == WARP_PAGE_REQS)
    {
        // stars required
        if(mode == CallMode::Render)
        {
            std::string need_star_count = fmt::format_ne(g_editorStrings.warpNeedStarCount, EditorCursor.Warp.Stars,
                LanguageFormatNumber(EditorCursor.Warp.Stars, g_gameInfo.wordStarAccusativeSingular, g_gameInfo.wordStarAccusativeDual_Cnt, g_gameInfo.wordStarAccusativePlural),
                g_gameInfo.wordStarAccusativeDual_Cnt);

            SuperPrintR(mode, need_star_count, 3, 6, 194);
        }

        if(EditorCursor.Warp.Stars > 0 && UpdateButton(mode, 260 + 4, 180 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            EditorCursor.Warp.Stars --;
        if(UpdateButton(mode, 300 + 4, 180 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            EditorCursor.Warp.Stars ++;

        // new: StarsMsg
        if(FileFormat == FileFormats::LVL_PGEX)
        {
            SuperPrintR(mode, g_editorStrings.warpStarLockMessage, 3, 44, 234);
            if(UpdateButton(mode, 300 + 4, 220 + 4, GFX.EIcons, EditorCursor.Warp.StarsMsg != STRINGINDEX_NONE, 0, 32*Icon::pencil, 32, 32))
            {
                DisableCursorNew();
                SetS(EditorCursor.Warp.StarsMsg, TextEntryScreen::Run(g_editorStrings.warpStarLockMessage, GetS(EditorCursor.Warp.StarsMsg)));
                MouseMove(SharedCursor.X, SharedCursor.Y);
            }
        }

        SuperPrintR(mode, g_editorStrings.warpNeedKey, 3, 6, 274);
        if(UpdateCheckBox(mode, 220 + 4, 260 + 4, EditorCursor.Warp.Locked))
            EditorCursor.Warp.Locked = !EditorCursor.Warp.Locked;

        // new: must stand to enter
        if(FileFormat == FileFormats::LVL_PGEX)
        {
            SuperPrintR(mode, g_editorStrings.warpNeedFloor, 3, 6, 314);
            if(UpdateCheckBox(mode, 220 + 4, 300 + 4, EditorCursor.Warp.stoodRequired))
                EditorCursor.Warp.stoodRequired = !EditorCursor.Warp.stoodRequired;
        }
    }

    if(m_Warp_page == WARP_PAGE_LEVEL)
    {
        // map/level warps
        SuperPrintR(mode, g_editorStrings.warpToMap, 3, 6, 194);
        if(UpdateCheckBox(mode, 120 + 4, 180 + 4, EditorCursor.Warp.MapWarp))
        {
            EditorCursor.Warp.MapWarp = !EditorCursor.Warp.MapWarp;
            if(EditorCursor.Warp.MapWarp)
            {
                EditorCursor.Warp.level = STRINGINDEX_NONE;
                EditorCursor.Warp.LevelEnt = false;
            }
        }

        SuperPrintR(mode, g_editorStrings.warpLvlWarp, 3, 6, 234);

        SuperPrintRightR(mode, g_editorStrings.warpIn, 3, 240, 234);
        if(UpdateCheckBox(mode, 240 + 4, 220 + 4, EditorCursor.Warp.level != STRINGINDEX_NONE))
        {
            if(EditorCursor.Warp.level == STRINGINDEX_NONE)
            {
                SetS(EditorCursor.Warp.level, "...");
                EditorCursor.Warp.MapWarp = false;
                EditorCursor.Warp.LevelEnt = false;
            }
            else
                EditorCursor.Warp.level = STRINGINDEX_NONE;
        }

        SuperPrintRightR(mode, g_editorStrings.warpOut, 3, 360, 234);
        if(UpdateCheckBox(mode, 360 + 4, 220 + 4, EditorCursor.Warp.LevelEnt))
        {
            EditorCursor.Warp.LevelEnt = !EditorCursor.Warp.LevelEnt;
            if(EditorCursor.Warp.LevelEnt)
            {
                EditorCursor.Warp.level = STRINGINDEX_NONE;
                EditorCursor.Warp.MapWarp = false;
            }
        }

        // special options for lvl warp entrance
        if(EditorCursor.Warp.level != STRINGINDEX_NONE)
        {
            if(GetS(EditorCursor.Warp.level).length() <= 12)
            {
                SuperPrintR(mode, g_editorStrings.warpTarget + GetS(EditorCursor.Warp.level), 3, 6, 314);
            }
            else
            {
                SuperPrintR(mode, g_editorStrings.warpTarget + GetS(EditorCursor.Warp.level).substr(0,12), 3, 6, 302);
                SuperPrintR(mode, GetS(EditorCursor.Warp.level).substr(12), 3, 24, 322);
            }

            if(UpdateButton(mode, 360 + 4, 300 + 4, GFX.EIcons, false, 0, 32*Icon::open, 32, 32))
                StartFileBrowser(PtrS(EditorCursor.Warp.level), FileNamePath, "", {".lvl", ".lvlx"}, BROWSER_MODE_OPEN);

            // display this in a different way if modern options are enabled
            if(EditorCursor.Warp.LevelWarp == 0)
                SuperPrintR(mode, fmt::format_ne(g_editorStrings.warpTo, g_editorStrings.levelStartPos), 3, 46, 354);
            else
                SuperPrintR(mode, fmt::format_ne(g_editorStrings.warpTo, fmt::format_ne(g_editorStrings.phraseWarpIndex, EditorCursor.Warp.LevelWarp)), 3, 46, 354);

            if(EditorCursor.Warp.LevelWarp > 0 && UpdateButton(mode, 4, 340 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
                EditorCursor.Warp.LevelWarp --;

            if(EditorCursor.Warp.LevelWarp < maxWarps && UpdateButton(mode, 280 + 4, 340 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
                EditorCursor.Warp.LevelWarp ++;

            if(FileFormat == FileFormats::LVL_PGEX)
            {
                // display the option to show/hide the level start scene
                SuperPrintRightR(mode, g_editorStrings.warpShowStartScene, 3, 320, 414);
                if(UpdateCheckBox(mode, 320 + 4, 400 + 4, !EditorCursor.Warp.noEntranceScene))
                    EditorCursor.Warp.noEntranceScene = !EditorCursor.Warp.noEntranceScene;
                // display the option to show/hide the level star count
                SuperPrintRightR(mode, g_editorStrings.warpShowStarCount, 3, 320, 454);
                if(UpdateButton(mode, 320 + 4, 440 + 4, GFXNPC[NPCID_STAR_EXIT], !EditorCursor.Warp.noPrintStars, 0, 0, 32, 32))
                    EditorCursor.Warp.noPrintStars = !EditorCursor.Warp.noPrintStars;
            }
        }

        // special options for map warp
        if(EditorCursor.Warp.MapWarp)
        {
            // map warp!
            SuperPrintR(mode, g_editorStrings.mapPos, 3, 44, 302);

            if((int)EditorCursor.Warp.MapX != -1 || (int)EditorCursor.Warp.MapY != -1)
            {
                SuperPrintR(mode, g_editorStrings.letterCoordX + ": " + std::to_string((int)EditorCursor.Warp.MapX), 3, 4, 320);
                SuperPrintR(mode, g_editorStrings.letterCoordY + ": " + std::to_string((int)EditorCursor.Warp.MapY), 3, 4, 340);
            }
            else
            {
                SuperPrintR(mode, g_mainMenu.caseNone, 3, 4, 330);
            }

            if(UpdateButton(mode, 4, 360 + 4, GFX.EIcons, false, 0, 32*Icon::up, 32, 32))
            {
                EditorCursor.Warp.MapY = 32*((int)EditorCursor.Warp.MapY/32 - 1);
                if((int)EditorCursor.Warp.MapX == -1)
                    EditorCursor.Warp.MapX = 0.;
            }

            if(UpdateButton(mode, 40 + 4, 360 + 4, GFX.EIcons, false, 0, 32*Icon::down, 32, 32))
            {
                EditorCursor.Warp.MapY = 32*((int)EditorCursor.Warp.MapY/32 + 1);
                if((int)EditorCursor.Warp.MapX == -1)
                    EditorCursor.Warp.MapX = 0.;
            }

            if(UpdateButton(mode, 80 + 4, 360 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            {
                EditorCursor.Warp.MapX = 32*((int)EditorCursor.Warp.MapX/32 - 1);
                if((int)EditorCursor.Warp.MapY == -1)
                    EditorCursor.Warp.MapY = 0.;
            }

            if(UpdateButton(mode, 120 + 4, 360 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            {
                EditorCursor.Warp.MapX = 32*((int)EditorCursor.Warp.MapX/32 + 1);
                if((int)EditorCursor.Warp.MapY == -1)
                    EditorCursor.Warp.MapY = 0.;
            }
        }
    }
}

bool EditorScreen::UpdateTileButton(CallMode mode, int x, int y, int type, bool sel)
{
    return UpdateButton(mode, x, y, GFXTileBMP[type], sel, 0, TileHeight[type] * TileFrame[type], TileWidth[type], TileHeight[type]);
}

void EditorScreen::UpdateTile(CallMode mode, int x, int y, int type)
{
    if((type < 1) || (type >= maxTileType))
        return;
    bool sel = EditorCursor.Tile.Type == type;
    if(UpdateTileButton(mode, x, y, type, sel) && !sel)
    {
        // printf("Tile %d\n", type);
        EditorCursor.Tile.Type = type;
    }
}

void EditorScreen::UpdateTileGrid(CallMode mode, int x, int y, const int* types, int n_tiles, int n_cols)
{
    for(int i = 0; i < n_tiles; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateTile(mode, x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateTileScreen(CallMode mode)
{
    // Block GUI
    if(mode == CallMode::Render)
    {
        XRender::renderRect(0, 40, 40, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);
        XRender::renderRect(38, 40, 2, e_ScreenH - 40, 0.25f, 0.0f, 0.5f, 1.0f, true);
    }


    // Page selector
    int last_category = -1;
    int index = 0;

    for(const EditorCustom::ItemPage_t& page : EditorCustom::tile_pages)
    {
        if(page.category != last_category)
        {
            last_category = page.category;

            if(mode == CallMode::Render && index != 0)
                XRender::renderRect(0, 40 + -2 + (40 * index), 40, 4, 0.25f, 0.0f, 0.5f, 1.0f, true);
        }

        index++;

        if(UpdateTileButton(mode, 4, 4 + (40 * index), page.icon, m_tile_page == index))
            m_tile_page = index;
    }


    // Tile selector
    if(m_tile_page > 0 && m_tile_page <= (int)EditorCustom::tile_pages.size())
    {
        const EditorCustom::ItemPage_t& page = EditorCustom::tile_pages[m_tile_page - 1];

        for(auto it = page.begin; it != page.end; ++it)
        {
            const EditorCustom::ItemFamily& family = **it;

            if(mode == CallMode::Render)
            {
                int pix_len = SuperTextPixLen(family.name, 3);

                if(40 + family.X * 40 + pix_len > e_ScreenW)
                    SuperPrint(family.name, 3, e_ScreenW - 4 - pix_len, 40 + family.Y * 20);
                else
                    SuperPrint(family.name, 3, 40 + family.X * 40 + 4, 40 + family.Y * 20);
            }

            UpdateTileGrid(mode, 40 + family.X * 40, 60 + family.Y * 20, family.layout_pod.types.data(), family.layout_pod.types.size(), family.layout_pod.cols);
        }
    }
}

void EditorScreen::UpdateScene(CallMode mode, int x, int y, int type)
{
    if((type < 1) || (type >= maxSceneType))
        return;
    bool sel = EditorCursor.Scene.Type == type;
    if(UpdateButton(mode, x, y, GFXSceneBMP[type], sel, 0, SceneHeight[type] * SceneFrame[type], SceneWidth[type], SceneHeight[type]) && !sel)
    {
        // printf("%d\n", type);
        EditorCursor.Scene.Type = type;
    }
}

void EditorScreen::UpdateSceneGrid(CallMode mode, int x, int y, const int* types, int n_scenes, int n_cols)
{
    for(int i = 0; i < n_scenes; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateScene(mode, x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateSceneScreen(CallMode mode)
{
    // Scene GUI (none)

    // SuperPrintR(mode, "SCENERY", 3, 10, 40);
    static const int scenes[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 42, 47, 46, 43, 45, 64, 65, 44, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
    UpdateSceneGrid(mode, 0, 60, scenes, sizeof(scenes)/sizeof(int), 10);
}

void EditorScreen::UpdateLevel(CallMode mode, int x, int y, int type)
{
    if((type < 1) || (type >= maxLevelType))
        return;
    bool sel = EditorCursor.WorldLevel.Type == type;
    int draw_height;
    if(GFXLevelBig[type])
        draw_height = GFXLevelHeight[type];
    else
        draw_height = 32;
    if(UpdateButton(mode, x, y, GFXLevelBMP[type], sel, 0, 32 * LevelFrame[type], GFXLevelWidth[type], draw_height) && !sel)
    {
        // printf("%d\n", type);
        EditorCursor.WorldLevel.Type = type;
    }
}

void EditorScreen::UpdateLevelGrid(CallMode mode, int x, int y, const int* types, int n_levels, int n_cols)
{
    for(int i = 0; i < n_levels; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateLevel(mode, x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateLevelScreen(CallMode mode)
{
    // World Level GUI
    if(mode == CallMode::Render)
        XRender::renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 0.75f, true);

    // SuperPrintR(mode, "LEVEL GRAPHIC", 3, 10, 40);
    static const int levels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
    UpdateLevelGrid(mode, 0, 60, levels, sizeof(levels)/sizeof(int), 8);

    // path bg - Path
    if(UpdateCheckBox(mode, e_ScreenW - 240 + 4, 80+4, EditorCursor.WorldLevel.Path))
    {
        EditorCursor.WorldLevel.Path = !EditorCursor.WorldLevel.Path;
        if(EditorCursor.WorldLevel.Path)
            EditorCursor.WorldLevel.Path2 = false;
    }
    SuperPrintR(mode, g_editorStrings.levelPathBG, 3, e_ScreenW - 240 + 44, 90);

    // big bg - Path2
    if(UpdateCheckBox(mode, e_ScreenW - 240 + 4, 120+4, EditorCursor.WorldLevel.Path2))
    {
        EditorCursor.WorldLevel.Path2 = !EditorCursor.WorldLevel.Path2;
        if(EditorCursor.WorldLevel.Path2)
            EditorCursor.WorldLevel.Path = false;
    }
    SuperPrintR(mode, g_editorStrings.levelBigBG, 3, e_ScreenW - 240 + 44, 130);

    // game start - Start
    if(UpdateCheckBox(mode, e_ScreenW - 240 + 4, 160 + 4, EditorCursor.WorldLevel.Start))
        EditorCursor.WorldLevel.Start = !EditorCursor.WorldLevel.Start;
    SuperPrintR(mode, g_editorStrings.levelGameStart, 3, e_ScreenW - 240 + 44, 170);

    // always visible - Visible
    if(UpdateCheckBox(mode, e_ScreenW - 240 + 4, 200 + 4, EditorCursor.WorldLevel.Visible))
        EditorCursor.WorldLevel.Visible = !EditorCursor.WorldLevel.Visible;
    SuperPrintR(mode, g_editorStrings.levelAlwaysVis, 3, e_ScreenW - 240 + 44, 210);

    // map warp!
    SuperPrintR(mode, g_editorStrings.mapPos, 3, e_ScreenW - 240 + 44, 302);

    if((int)EditorCursor.WorldLevel.WarpX != -1 || (int)EditorCursor.WorldLevel.WarpY != -1)
    {
        SuperPrintR(mode, g_editorStrings.letterCoordX + ": " + std::to_string((int)EditorCursor.WorldLevel.WarpX), 3, e_ScreenW - 240 + 4, 320);
        SuperPrintR(mode, g_editorStrings.letterCoordY + ": " + std::to_string((int)EditorCursor.WorldLevel.WarpY), 3, e_ScreenW - 240 + 4, 340);
        if(UpdateButton(mode, e_ScreenW - 240 + 160 + 4, 320 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
        {
            EditorCursor.WorldLevel.WarpX = -1;
            EditorCursor.WorldLevel.WarpY = -1;
        }
    }
    else
    {
        SuperPrintR(mode, g_mainMenu.caseNone, 3, e_ScreenW - 240 + 4, 330);
    }

    if(UpdateButton(mode, e_ScreenW - 240 + 4, 360 + 4, GFX.EIcons, false, 0, 32*Icon::up, 32, 32))
    {
        EditorCursor.WorldLevel.WarpY = 32*((int)EditorCursor.WorldLevel.WarpY/32 - 1);
        if((int)EditorCursor.WorldLevel.WarpX == -1)
            EditorCursor.WorldLevel.WarpX = 0.;
    }

    if(UpdateButton(mode, e_ScreenW - 240 + 40 + 4, 360 + 4, GFX.EIcons, false, 0, 32*Icon::down, 32, 32))
    {
        EditorCursor.WorldLevel.WarpY = 32*((int)EditorCursor.WorldLevel.WarpY/32 + 1);
        if((int)EditorCursor.WorldLevel.WarpX == -1)
            EditorCursor.WorldLevel.WarpX = 0.;
    }

    if(UpdateButton(mode, e_ScreenW - 240 + 80 + 4, 360 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
    {
        EditorCursor.WorldLevel.WarpX = 32*((int)EditorCursor.WorldLevel.WarpX/32 - 1);
        if((int)EditorCursor.WorldLevel.WarpY == -1)
            EditorCursor.WorldLevel.WarpY = 0.;
    }

    if(UpdateButton(mode, e_ScreenW - 240 + 120 + 4, 360 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
    {
        EditorCursor.WorldLevel.WarpX = 32*((int)EditorCursor.WorldLevel.WarpX/32 + 1);
        if((int)EditorCursor.WorldLevel.WarpY == -1)
            EditorCursor.WorldLevel.WarpY = 0.;
    }

    // bottom pane: level filename, entrance and exits
    if(mode == CallMode::Render)
        XRender::renderRect(0, e_ScreenH - 240, e_ScreenW - 240, 240, 0.6f, 0.6f, 0.8f, 1.0f, true);

    // level name - LevelName
    SuperPrintR(mode, g_editorStrings.levelName, 3, 10 + 44, e_ScreenH - 240 + 2);
    SuperPrintR(mode, EditorCursor.WorldLevel.LevelName.substr(0, 19), 3, 10 + 44, e_ScreenH - 240 + 20);
    if(EditorCursor.WorldLevel.LevelName.length() > 19)
        SuperPrintR(mode, EditorCursor.WorldLevel.LevelName.substr(19), 3, 10 + 44 + 18, e_ScreenH - 240 + 38);
    if(UpdateButton(mode, 10 + 4, e_ScreenH - 240 + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
    {
        DisableCursorNew();
        EditorCursor.WorldLevel.LevelName = TextEntryScreen::Run(g_editorStrings.levelName, EditorCursor.WorldLevel.LevelName);
        MouseMove(SharedCursor.X, SharedCursor.Y);
    }

    // level filename - FileName
    SuperPrintR(mode, g_editorStrings.warpTarget, 3, 10 + 44, e_ScreenH - 180 + 2);
    if(!EditorCursor.WorldLevel.FileName.empty())
        SuperPrintR(mode, EditorCursor.WorldLevel.FileName, 3, 10 + 44, e_ScreenH - 180 + 20);
    else
        SuperPrintR(mode, g_mainMenu.caseNone, 3, 10 + 44, e_ScreenH - 180 + 20);
    if(UpdateButton(mode, 10 + 4, e_ScreenH - 180 + 4, GFX.EIcons, false, 0, 32*Icon::open, 32, 32))
        StartFileBrowser(&EditorCursor.WorldLevel.FileName, FileNamePath, "", {".lvl", ".lvlx"}, BROWSER_MODE_OPEN);

    if(!EditorCursor.WorldLevel.FileName.empty())
    {
        // entrance warp - StartWarp
        if(EditorCursor.WorldLevel.StartWarp > 0 && UpdateButton(mode, 10 + 4, e_ScreenH - 180 + 40 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
            EditorCursor.WorldLevel.StartWarp -= 1;
        if(EditorCursor.WorldLevel.StartWarp < maxWarps && UpdateButton(mode, 280 + 4, e_ScreenH - 180 + 40 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
            EditorCursor.WorldLevel.StartWarp += 1;

        if(EditorCursor.WorldLevel.StartWarp == 0)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.warpTo, g_editorStrings.levelStartPos), 3, 10 + 44, e_ScreenH - 180 + 40 + 12);
        else
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.warpTo, fmt::format_ne(g_editorStrings.phraseWarpIndex, EditorCursor.WorldLevel.StartWarp)), 3, 10 + 44, e_ScreenH - 180 + 40 + 12);

        // exits
        SuperPrintR(mode, g_editorStrings.levelPathUnlocks, 3, 10 + 44, e_ScreenH - 180 + 80 + 2);

        using Controls::PlayerControls::Buttons;

        if(UpdateButton(mode, 10 + 4, e_ScreenH - 180 + 100 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_LEVEL_EXIT;
            m_special_subpage = 1;
        }
        SuperPrintR(mode, GetButtonName_UI(Buttons::Up), 3, 10 + 44, e_ScreenH - 180 + 102);
        SuperPrintR(mode, EditorCustom::list_level_exit_names[EditorCursor.WorldLevel.LevelExit[1]+1], 3, 10 + 44, e_ScreenH - 180 + 120);

        if(UpdateButton(mode, (e_ScreenW - 240)/2 + 10 + 4, e_ScreenH - 180 + 100 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_LEVEL_EXIT;
            m_special_subpage = 3;
        }
        SuperPrintR(mode, GetButtonName_UI(Buttons::Down), 3, (e_ScreenW - 240)/2 + 10 + 44, e_ScreenH - 180 + 102);
        SuperPrintR(mode, EditorCustom::list_level_exit_names[EditorCursor.WorldLevel.LevelExit[3]+1], 3, (e_ScreenW - 240)/2 + 10 + 44, e_ScreenH - 180 + 120);

        if(UpdateButton(mode, 10 + 4, e_ScreenH - 180 + 140 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_LEVEL_EXIT;
            m_special_subpage = 2;
        }
        SuperPrintR(mode, GetButtonName_UI(Buttons::Left), 3, 10 + 44, e_ScreenH - 180 + 142);
        SuperPrintR(mode, EditorCustom::list_level_exit_names[EditorCursor.WorldLevel.LevelExit[2]+1], 3, 10 + 44, e_ScreenH - 180 + 160);

        if(UpdateButton(mode, (e_ScreenW - 240)/2 + 10 + 4, e_ScreenH - 180 + 140 + 4, GFX.EIcons, false, 0, 32*Icon::subscreen, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_LEVEL_EXIT;
            m_special_subpage = 4;
        }
        SuperPrintR(mode, GetButtonName_UI(Buttons::Right), 3, (e_ScreenW - 240)/2 + 10 + 44, e_ScreenH - 180 + 142);
        SuperPrintR(mode, EditorCustom::list_level_exit_names[EditorCursor.WorldLevel.LevelExit[4]+1], 3, (e_ScreenW - 240)/2 + 10 + 44, e_ScreenH - 180 + 160);
    }

}

void EditorScreen::UpdatePath(CallMode mode, int x, int y, int type)
{
    if((type < 1) || (type >= maxPathType))
        return;
    bool sel = EditorCursor.WorldPath.Type == type;
    if(UpdateButton(mode, x, y, GFXPathBMP[type], sel, 0, 0, 32, 32) && !sel)
    {
        // printf("%d\n", type);
        EditorCursor.WorldPath.Type = type;
    }
}

void EditorScreen::UpdatePathGrid(CallMode mode, int x, int y, const int* types, int n_paths, int n_cols)
{
    for(int i = 0; i < n_paths; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdatePath(mode, x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdatePathScreen(CallMode mode)
{
    // Path GUI (none)

    // SuperPrintR(mode, "PATH GRAPHIC", 3, 10, 40);
    static const int paths[] = { 2, 1, 18, 19, 31, 22, 32,
        11, 6, 10, 30, 25, 27, 23,
         9, 5,  7, 21,  3, 20, 26,
        13, 8, 12, 29, 24, 28,  4};
    UpdatePathGrid(mode, 0, 60, paths, sizeof(paths)/sizeof(int), 7);
}

void EditorScreen::UpdateFileScreen(CallMode mode)
{
    if(m_special_page == SPECIAL_PAGE_FILE_CONFIRM)
    {
        if(m_special_subpage == 0)
        {
            m_special_page = SPECIAL_PAGE_FILE;
            m_special_subpage = 0;
            return;
        }

        bool can_save;
        std::string* action;
        if(m_special_subpage == 1)
        {
            can_save = true;
            action = &g_editorStrings.fileActionClearLevel;
        }
        else if(m_special_subpage == 11)
        {
            can_save = true;
            action = &g_editorStrings.fileActionClearWorld;
        }
        else if(m_special_subpage == 2 || m_special_subpage == 12)
        {
            can_save = true;
            action = &g_editorStrings.fileActionOpen;
        }
        else if(m_special_subpage == 3 || m_special_subpage == 13)
        {
            can_save = false;
            action = &g_editorStrings.fileActionRevert;
        }
        else if(m_special_subpage == 4)
        {
            can_save = true;
            action = &g_editorStrings.fileActionExit;
        }
        else // should never happen
        {
            return;
        }

        if(can_save)
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.fileConfirmationSaveBeforeAction, *action), 3, 10, 50);
        else
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.fileConfirmationConfirmAction, *action), 3, 10, 50);

        bool confirmed = false;
        if(can_save)
        {
            SuperPrintR(mode, fmt::format_ne(g_editorStrings.fileOptionYesSaveThenAction, *action), 3, 60, 110);
            if(UpdateButton(mode, 20 + 4, 100 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
            {
                if(!WorldEditor)
                    SaveLevel(FullFileName, FileFormat);
                else
                    SaveWorld(FullFileName, FileFormat);
                confirmed = true;
            }
        }

        SuperPrintR(mode, fmt::format_ne(g_editorStrings.fileOptionActionWithoutSave, *action), 3, 60, 150);
        if(UpdateButton(mode, 20 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            confirmed = true;
        }

        SuperPrintR(mode, fmt::format_ne(g_editorStrings.fileOptionCancelAction, *action), 3, 60, 190);
        if(UpdateButton(mode, 20 + 4, 180 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_FILE;
            m_special_subpage = 0;
            return;
        }

        if(confirmed)
        {
            if(m_special_subpage == 1) // new level
            {
                int cur_FileFormat = FileFormat;
                if(cur_FileFormat < 0 || cur_FileFormat > 2)
                    cur_FileFormat = g_config.editor_preferred_file_format;

                if(cur_FileFormat == FileFormats::LVL_SMBX64 || cur_FileFormat == FileFormats::LVL_SMBX38A)
                    StartFileBrowser(&FullFileName, "", FileNamePath, {".lvl"}, BROWSER_MODE_SAVE_NEW, BROWSER_CALLBACK_NEW_LEVEL);
                else
                    StartFileBrowser(&FullFileName, "", FileNamePath, {".lvlx"}, BROWSER_MODE_SAVE_NEW, BROWSER_CALLBACK_NEW_LEVEL);
            }
            else if(m_special_subpage == 2) // open level
            {
                StartFileBrowser(&FullFileName, "", FileNamePath, {".lvlx", ".lvl"}, BROWSER_MODE_OPEN, BROWSER_CALLBACK_OPEN_LEVEL);
            }
            else if(m_special_subpage == 3) // revert level
            {
                OpenLevel(FullFileName);
                m_special_page = SPECIAL_PAGE_FILE;
                m_special_subpage = 0;
            }
            else if(m_special_subpage == 11) // new world
            {
                int cur_FileFormat = FileFormat;
                if(cur_FileFormat < 0 || cur_FileFormat > 2)
                    cur_FileFormat = g_config.editor_preferred_file_format;

                if(cur_FileFormat == FileFormats::LVL_SMBX64 || cur_FileFormat == FileFormats::LVL_SMBX38A)
                    StartFileBrowser(&FullFileName, "", FileNamePath, {".wld"}, BROWSER_MODE_SAVE_NEW, BROWSER_CALLBACK_NEW_WORLD);
                else
                    StartFileBrowser(&FullFileName, "", FileNamePath, {".wldx"}, BROWSER_MODE_SAVE_NEW, BROWSER_CALLBACK_NEW_WORLD);
            }
            else if(m_special_subpage == 12) // open world
            {
                StartFileBrowser(&FullFileName, "", FileNamePath, {".wldx", ".wld"}, BROWSER_MODE_OPEN, BROWSER_CALLBACK_OPEN_WORLD);
            }
            else if(m_special_subpage == 13) // revert world
            {
                OpenWorld(FullFileName);
                m_special_page = SPECIAL_PAGE_FILE;
                m_special_subpage = 0;
            }
            else if(m_special_subpage == 4) // exit
            {
                if(g_config.EnableInterLevelFade)
                {
                    g_levelScreenFader.setupFader(4, 0, 65, ScreenFader::S_FADE);
                    g_worldScreenFader.setupFader(4, 0, 65, ScreenFader::S_FADE);
                }
                else
                {
                    g_levelScreenFader.setupFader(65, 0, 65, ScreenFader::S_FADE);
                    g_worldScreenFader.setupFader(65, 0, 65, ScreenFader::S_FADE);
                }
                editorWaitForFade();

                ClearLevel();
                ClearWorld();
                GameMenu = true;
                MenuMode = 0;
                MenuCursor = 0;
                LevelEditor = false;
                WorldEditor = false;
                TestLevel = false;
                m_special_page = SPECIAL_PAGE_NONE;
                m_special_subpage = 0;
                testStartWarp = 0;
            }
        }
        return;
    }

    if(m_special_page == SPECIAL_PAGE_FILE_CONVERT)
    {
        // TODO: find a way to warn them if this would overwrite something

        const std::string* format = (m_special_subpage == FileFormats::LVL_PGEX) ? &g_editorStrings.fileFormatModern : &g_editorStrings.fileFormatLegacy;

        SuperPrintR(mode, fmt::format_ne(g_editorStrings.fileConfirmationConvertFormatTo, *format), 3, 10, 50);

        if(mode == CallMode::Render)
        {
            FontManager::printTextOptiPx(g_editorStrings.fileConvertDesc,
                                         20, 90,
                                         606,
                                         FontManager::fontIdFromSmbxFont(4));
        }
        // SuperPrintR(mode, g_editorStrings.fileConvertDesc, 4, 20, 90);

        if(m_saved_message.empty())
            SuperPrintR(mode, g_editorStrings.fileConvertNoIssues, 4, 20, 150);
        else
        {
            SuperPrintR(mode, g_editorStrings.fileConvertFeaturesWillBeLost, 4, 20, 150, 1.0f, 0.5f, 0.5f);
            SuperPrintR(mode, m_saved_message, 4, 4, 180);
        }

        SuperPrintR(mode, g_editorStrings.fileOptionProceedWithConversion, 3, 60, 390);
        if(UpdateButton(mode, 20 + 4, 380 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            if(WorldEditor)
            {
                ConvertWorld(m_special_subpage);
                SaveWorld(FullFileName, FileFormat);
                OpenWorld(FullFileName);
            }
            else
            {
                ConvertLevel(m_special_subpage);
                SaveLevel(FullFileName, FileFormat);
                OpenLevel(FullFileName);
            }

            Integrator::setEditorFile(FileName);

            m_special_page = SPECIAL_PAGE_FILE;
            m_special_subpage = 0;
            m_saved_message.clear();
        }

        SuperPrintR(mode, g_editorStrings.fileOptionCancelConversion, 3, 60, 430);
        if(UpdateButton(mode, 20 + 4, 420 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_FILE;
            m_special_subpage = 0;
            m_saved_message.clear();
        }

        return;
    }

    SuperPrintR(mode, g_editorStrings.fileLabelCurrentFile + FileNameFull, 3, 10, 50);

    SuperPrintR(mode, g_editorStrings.fileLabelFormat, 3, 150, 90);

    if(UpdateButton(mode, 320 + 4, 80 + 4, GFXNPC[NPCID_POWER_S3], FileFormat == FileFormats::LVL_SMBX64, 0, 0, 32, 32, g_editorStrings.fileFormatLegacy.c_str()) && FileFormat != FileFormats::LVL_SMBX64)
    {
        m_special_page = SPECIAL_PAGE_FILE_CONVERT;
        m_special_subpage = FileFormats::LVL_SMBX64;
        CanConvertLevel(FileFormats::LVL_SMBX64, &m_saved_message);
    }

    if(UpdateButton(mode, 360 + 4, 80 + 4, GFX.EIcons, FileFormat == FileFormats::LVL_PGEX, 0, 32*Icon::thextech, 32, 32, g_editorStrings.fileFormatModern.c_str()) && FileFormat != FileFormats::LVL_PGEX)
    {
        m_special_page = SPECIAL_PAGE_FILE_CONVERT;
        m_special_subpage = FileFormats::LVL_PGEX;
        CanConvertLevel(FileFormats::LVL_PGEX, &m_saved_message);
    }

    SuperPrintR(mode, g_editorStrings.fileSectionLevel, 3, 110, 140);

    SuperPrintR(mode, g_editorStrings.fileCommandNew, 3, 54, 170);
    if(UpdateButton(mode, 10 + 4, 160 + 4, GFX.EIcons, false, 0, 32*Icon::newf, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 1;
    }

    SuperPrintR(mode, g_editorStrings.fileCommandOpen, 3, 54, 210);
    if(UpdateButton(mode, 10 + 4, 200 + 4, GFX.EIcons, false, 0, 32*Icon::open, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 2;
    }

    if(!WorldEditor)
    {
        SuperPrintR(mode, g_editorStrings.fileCommandSave, 3, 54, 250);
        if(UpdateButton(mode, 10 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::save, 32, 32))
        {
            SaveLevel(FullFileName, FileFormat);
        }

        SuperPrintR(mode, g_editorStrings.fileCommandSaveAs, 3, 54, 290);
        if(UpdateButton(mode, 10 + 4, 280 + 4, GFX.EIcons, false, 0, 32*Icon::save, 32, 32))
        {
            if(FileFormat == 1 || FileFormat == 2)
                StartFileBrowser(&FullFileName, "", FileNamePath, {".lvl"}, BROWSER_MODE_SAVE, BROWSER_CALLBACK_SAVE_LEVEL);
            else
                StartFileBrowser(&FullFileName, "", FileNamePath, {".lvlx"}, BROWSER_MODE_SAVE, BROWSER_CALLBACK_SAVE_LEVEL);
        }

        SuperPrintR(mode, g_editorStrings.fileActionRevert, 3, 54, 330);
        if(UpdateButton(mode, 10 + 4, 320 + 4, GFX.EIcons, false, 0, 32*Icon::hop, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
            m_special_subpage = 3;
        }
    }

    SuperPrintR(mode, g_editorStrings.fileSectionWorld, 3, e_ScreenW/2 + 110, 140);

    SuperPrintR(mode, g_editorStrings.fileCommandNew, 3, e_ScreenW/2 + 54, 170);
    if(UpdateButton(mode, e_ScreenW/2 + 10 + 4, 160 + 4, GFX.EIcons, false, 0, 32*Icon::newf, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 11;
    }

    SuperPrintR(mode, g_editorStrings.fileCommandOpen, 3, e_ScreenW/2 + 54, 210);
    if(UpdateButton(mode, e_ScreenW/2 + 10 + 4, 200 + 4, GFX.EIcons, false, 0, 32*Icon::open, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 12;
    }

    if(WorldEditor)
    {
        SuperPrintR(mode, g_editorStrings.fileCommandSave, 3, e_ScreenW/2 + 54, 250);
        if(UpdateButton(mode, e_ScreenW/2 + 10 + 4, 240 + 4, GFX.EIcons, false, 0, 32*Icon::save, 32, 32))
        {
            SaveWorld(FullFileName, FileFormat);
        }

        SuperPrintR(mode, g_editorStrings.fileCommandSaveAs, 3, e_ScreenW/2 + 54, 290);
        if(UpdateButton(mode, e_ScreenW/2 + 10 + 4, 280 + 4, GFX.EIcons, false, 0, 32*Icon::save, 32, 32))
        {
            if(FileFormat == 1 || FileFormat == 2)
                StartFileBrowser(&FullFileName, "", FileNamePath, {".wld"}, BROWSER_MODE_SAVE, BROWSER_CALLBACK_SAVE_WORLD);
            else
                StartFileBrowser(&FullFileName, "", FileNamePath, {".wldx"}, BROWSER_MODE_SAVE, BROWSER_CALLBACK_SAVE_WORLD);
        }

        SuperPrintR(mode, g_editorStrings.fileActionRevert, 3, e_ScreenW/2 + 54, 330);
        if(UpdateButton(mode, e_ScreenW/2 + 10 + 4, 320 + 4, GFX.EIcons, false, 0, 32*Icon::hop, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
            m_special_subpage = 13;
        }
    }

    SuperPrintR(mode, g_editorStrings.fileActionExit, 3, 54, 410);
    if(UpdateButton(mode, 10 + 4, 400 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 4;
    }
}

void EditorScreen::StartFileBrowser(std::string* file_target,
                                    const std::string &root_path,
                                    const std::string &current_path,
                                    const std::vector<std::string> &target_exts,
                                    BrowserMode_t browser_mode,
                                    BrowserCallback_t browser_callback)
{
    m_file_target = file_target;
    m_root_path = root_path;
    m_target_exts = target_exts;
    m_browser_mode = browser_mode;
    m_browser_callback = browser_callback;
    m_cur_path = current_path;
    m_path_synced = false;
    m_last_special_page = m_special_page;
    m_special_page = SPECIAL_PAGE_BROWSER;
    m_special_subpage = 0;
}

void EditorScreen::FileBrowserSuccess()
{
    if(m_file_target) *m_file_target = m_cur_file;
    if(m_browser_callback == BROWSER_CALLBACK_OPEN_LEVEL)
    {
        EnsureLevel();
        OpenLevel(FullFileName);
        ResetCursor();
        Integrator::setEditorFile(FileName);
    }
    else if(m_browser_callback == BROWSER_CALLBACK_SAVE_LEVEL)
    {
        SaveLevel(FullFileName, FileFormat);
        // this will resync custom assets
        OpenLevel(FullFileName);
        Integrator::setEditorFile(FileName);
    }
    else if(m_browser_callback == BROWSER_CALLBACK_NEW_LEVEL)
    {
        int cur_FileFormat = FileFormat;
        if(cur_FileFormat < 0 || cur_FileFormat > 2)
            cur_FileFormat = g_config.editor_preferred_file_format;
        EnsureLevel();
        ClearLevel();
        SaveLevel(FullFileName, cur_FileFormat);
        // this will resync custom assets
        OpenLevel(FullFileName);
        ResetCursor();
        Integrator::setEditorFile(FileName);
    }
    else if(m_browser_callback == BROWSER_CALLBACK_OPEN_WORLD)
    {
        EnsureWorld();
        OpenWorld(FullFileName);
        ResetCursor();
        Integrator::setEditorFile(FileName);
    }
    else if(m_browser_callback == BROWSER_CALLBACK_SAVE_WORLD)
    {
        SaveWorld(FullFileName, FileFormat);
        // resync custom assets
        OpenWorld(FullFileName);
        Integrator::setEditorFile(FileName);
    }
    else if(m_browser_callback == BROWSER_CALLBACK_NEW_WORLD)
    {
        int cur_FileFormat = FileFormat;
        if(cur_FileFormat < 0 || cur_FileFormat > 2)
            cur_FileFormat = g_config.editor_preferred_file_format;
        EnsureWorld();
        ClearWorld();
        SaveWorld(FullFileName, cur_FileFormat);
        // resync custom assets
        OpenWorld(FullFileName);
        ResetCursor();
        Integrator::setEditorFile(FileName);
    }
    else if(m_browser_callback == BROWSER_CALLBACK_CUSTOM_MUSIC)
    {
        StartMusic(curSection);
    }
    FileBrowserCleanup();
}

void EditorScreen::FileBrowserFailure()
{
    // if(m_file_target && m_file_target != &FullFileName) m_file_target->clear();
    FileBrowserCleanup();
}

void EditorScreen::FileBrowserCleanup()
{
    m_cur_file.clear();
    m_file_target = nullptr;
    m_special_page = m_last_special_page;
    m_last_special_page = SPECIAL_PAGE_NONE;
    m_special_subpage = 0;
    m_browser_mode = BROWSER_MODE_NONE;
    m_browser_callback = BROWSER_CALLBACK_NONE;
}

void EditorScreen::SyncPath()
{
    if(m_path_synced) return;
    m_dirman.setPath(m_root_path+m_cur_path);
    m_dirman.getListOfFiles(m_cur_path_files, m_target_exts);
    m_dirman.getListOfFolders(m_cur_path_dirs);
    std::sort(m_cur_path_files.begin(), m_cur_path_files.end());
    std::sort(m_cur_path_dirs.begin(), m_cur_path_dirs.end());
    m_path_synced = true;
}

void EditorScreen::GoToSuper()
{
    if(m_cur_path.empty())
    {
        m_cur_path += "../";
    }
    else
    {
        size_t last_slash = m_cur_path.rfind('/', m_cur_path.length()-2);
        size_t last_chunk_start;
        if(last_slash == std::string::npos)
            last_chunk_start = 0;
        else
            last_chunk_start = last_slash + 1;
        if(m_cur_path.substr(last_chunk_start) == "../")
            m_cur_path += "../";
        else
            m_cur_path = m_cur_path.substr(0, last_chunk_start);
    }
    m_path_synced = false;
}

void EditorScreen::ValidateExt(std::string& cur_file)
{
    if(m_target_exts.empty()) return;

    for(const std::string& ext : m_target_exts)
    {
        if(cur_file.size() >= ext.size())
        {
            std::string possible_ext;
            possible_ext.reserve(ext.size());
            for(const char &c : cur_file.substr(cur_file.size() - ext.size()))
                possible_ext.push_back(std::tolower(c));

            if(possible_ext.compare(0, ext.size(), ext) == 0)
                return;
        }
    }

    cur_file += m_target_exts[0];
}

bool EditorScreen::FileExists(const std::string& cur_file)
{
    return (std::find(m_cur_path_files.begin(), m_cur_path_files.end(), cur_file) != m_cur_path_files.end());
}

void EditorScreen::UpdateBrowserScreen(CallMode mode)
{
    constexpr bool IGNORE_DIRS = true;
    // render shared GUI elements on right
    if(UpdateButton(mode, e_ScreenW - 40 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::x, 32, 32))
    {
        FileBrowserFailure();
        return;
    }

    if(m_special_page == SPECIAL_PAGE_BROWSER_CONFIRM)
    {
        SuperPrintR(mode, g_editorStrings.phraseAreYouSure, 3, 60, 40);
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.browserAskOverwriteFile, m_cur_file), 3, 10, 60);

        SuperPrintR(mode, g_mainMenu.wordYes, 3, 60, 110);
        if(UpdateButton(mode, 10 + 4, 100 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            FileBrowserSuccess();
            return;
        }

        SuperPrintR(mode, g_mainMenu.wordNo, 3, 60, 150);
        if(UpdateButton(mode, 10 + 4, 140 + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_BROWSER;
            m_cur_file.clear();
        }

        return;
    }

    if(!m_path_synced)
        SyncPath();

    int dir_length = m_cur_path_dirs.size() + 1; // ".."
    int file_length = m_cur_path_files.size();

    if(m_browser_mode == BROWSER_MODE_SAVE || m_browser_mode == BROWSER_MODE_SAVE_NEW)
    {
        if(m_browser_mode == BROWSER_MODE_SAVE_NEW)
            SuperPrintR(mode, g_editorStrings.browserNewFile, 3, 60, 40);
        else
            SuperPrintR(mode, g_editorStrings.browserSaveFile, 3, 60, 40);

        dir_length ++; // "new", last folder
        file_length ++; // "new", first file
    }
    else
    {
        SuperPrintR(mode, g_editorStrings.browserOpenFile, 3, 60, 40);
    }

    // ignore directories
    if(IGNORE_DIRS)
        dir_length = 0;

    int page_max = (dir_length + file_length - 1) / 20;
    if(!(page_max == 0 && m_special_subpage == 0))
        SuperPrintR(mode, fmt::format_ne(g_editorStrings.pageBlankOfBlank, m_special_subpage + 1, page_max + 1), 3, e_ScreenW - 320, 40);

    if(m_special_subpage > 0 && UpdateButton(mode, e_ScreenW - 120 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::left, 32, 32))
        m_special_subpage --;

    if(m_special_subpage < page_max && UpdateButton(mode, e_ScreenW - 80 + 4, 40 + 4, GFX.EIcons, false, 0, 32*Icon::right, 32, 32))
        m_special_subpage ++;

    if(!m_cur_path.empty())
    {
        const int avail_chars = (e_ScreenW-140)/18;
        if(m_cur_path.size() > avail_chars)
            SuperPrintR(mode, g_editorStrings.npcInContainer + " ..." + m_cur_path.substr(m_cur_path.size() - avail_chars), 3, 10, 60);
        else
            SuperPrintR(mode, g_editorStrings.npcInContainer + " " + m_cur_path, 3, 10, 60);
    }

    // render file selector
    for(int i = 0; i < 20; i++)
    {
        int x = 10 + (e_ScreenW/2)*(i/10);
        int y = 80 + 40*(i%10);

        int l = m_special_subpage*20 + i;
        if(!IGNORE_DIRS && l == 0)
        {
            SuperPrintR(mode, "..", 3, x + 44, y + 12);
            if(UpdateButton(mode, x + 4, y + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
            {
                GoToSuper();
                m_special_subpage = 0;
            }
        }
        else if((m_browser_mode == BROWSER_MODE_SAVE || m_browser_mode == BROWSER_MODE_SAVE_NEW) && l == dir_length - 1)
        {
            SuperPrintR(mode, g_editorStrings.browserItemNewFolder, 3, x + 44, y + 12);
            if(UpdateButton(mode, x + 4, y + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
            {
                DisableCursorNew();
                std::string folder_name = TextEntryScreen::Run(g_editorStrings.browserItemNewFolder, "");
                MouseMove(SharedCursor.X, SharedCursor.Y);

                if(!folder_name.empty() && !m_dirman.exists(folder_name))
                {
                    m_dirman.mkdir(folder_name);
                    m_path_synced = false;
                }
            }
        }
        else if(l < dir_length)
        {
            l -= 1;
            if(m_cur_path_dirs[l].length() < 15)
                SuperPrintR(mode, m_cur_path_dirs[l], 3, x + 44, y + 10);
            else
            {
                SuperPrintR(mode, m_cur_path_dirs[l].substr(0, 14), 3, x + 44, y + 2);
                SuperPrintR(mode, m_cur_path_dirs[l].substr(14, 14), 3, x + 44, y + 20);
            }

            if(UpdateButton(mode, x + 4, y + 4, GFX.EIcons, false, 0, 32*Icon::action, 32, 32))
            {
                m_cur_path += m_cur_path_dirs[l] + "/";
                m_path_synced = false;
                m_special_subpage = 0;
            }
        }
        else if((m_browser_mode == BROWSER_MODE_SAVE || m_browser_mode == BROWSER_MODE_SAVE_NEW) && l == dir_length)
        {
            SuperPrintR(mode, g_editorStrings.browserItemNewFile, 3, x + 44, y + 12);
            if(UpdateButton(mode, x + 4, y + 4, GFX.EIcons, false, 0, 32*Icon::pencil, 32, 32))
            {
                DisableCursorNew();
                std::string file_name = TextEntryScreen::Run(g_editorStrings.fileCommandSaveAs, "");
                MouseMove(SharedCursor.X, SharedCursor.Y);

                if(!file_name.empty())
                {
                    // validate: append the file extension if it doesn't already appear.
                    ValidateExt(file_name);
                    m_cur_file = m_cur_path + file_name;

                    if(FileExists(file_name))
                        m_special_page = SPECIAL_PAGE_BROWSER_CONFIRM;
                    else
                    {
                        FileBrowserSuccess();
                        return;
                    }
                }
            }
        }
        else if(l < dir_length + file_length)
        {
            if(m_browser_mode == BROWSER_MODE_SAVE || m_browser_mode == BROWSER_MODE_SAVE_NEW)
                l -= dir_length + 1;
            else
                l -= dir_length;

            if(m_cur_path_files[l].length() < 15)
                SuperPrintR(mode, m_cur_path_files[l], 3, x + 44, y + 10);
            else
            {
                SuperPrintR(mode, m_cur_path_files[l].substr(0, 14), 3, x + 44, y + 2);
                SuperPrintR(mode, m_cur_path_files[l].substr(14, 14), 3, x + 44, y + 20);
            }

            if(UpdateButton(mode, x + 4, y + 4, GFX.ECursor[2], false, 0, 0, 32, 32))
            {
                m_cur_file = m_cur_path + m_cur_path_files[l];
                if(m_browser_mode == BROWSER_MODE_OPEN)
                {
                    FileBrowserSuccess();
                    return;
                }
                else
                {
                    // overwrite confirmation
                    m_special_page = SPECIAL_PAGE_BROWSER_CONFIRM;
                }
            }
        }
    }
}

inline void swap_screens()
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

void EditorScreen::UpdateSelectorBar(CallMode mode, bool select_bar_only)
{
    // can be drawn either over level screen or at top of editor screen

    // find the selector bar location
    int sx;
    if(select_bar_only)
    {
        e_CursorX = EditorCursor.X;
        e_CursorY = EditorCursor.Y;
        // if(WorldEditor)
        //     e_CursorY += 8;
        sx = (ScreenW - e_ScreenW)/2;
    }
    else
        sx = 0;

    if(mode == CallMode::Render)
    {
        float alpha = 1.0f;
        if(select_bar_only && MagicHand)
            alpha = 0.7f;

        for(int i = 0; i < 5; i++)
        {
            float r = 0.6f + 0.05f*i;
            float g = 0.5f + 0.075f*i;
            XRender::renderRect(sx+0, 38-2*i, e_ScreenW, 2, r, g, 0.8f, alpha, true);
            // XRender::renderRect(sx-10+2*i, 0, 2, 40-10+2*i, r, g, 0.8f, alpha, true);
            // XRender::renderRect(sx+e_ScreenW+8-2*i, 0, 2, 40-10+2*i, r, g, 0.8f, alpha, true);
        }
        XRender::renderRect(sx+0, 0, e_ScreenW, 30, 0.8f, 0.8f, 0.8f, alpha, true);
    }

    bool in_layers = (m_special_page == SPECIAL_PAGE_LAYERS || m_special_page == SPECIAL_PAGE_LAYER_DELETION);
    bool in_events = (m_special_page == SPECIAL_PAGE_EVENTS || m_special_page == SPECIAL_PAGE_EVENT_LAYERS
        || m_special_page == SPECIAL_PAGE_EVENT_TRIGGER || m_special_page == SPECIAL_PAGE_EVENT_SETTINGS
        || m_special_page == SPECIAL_PAGE_EVENT_BOUNDS || m_special_page == SPECIAL_PAGE_EVENT_DELETION
        || m_special_page == SPECIAL_PAGE_EVENT_MUSIC || m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND
        || m_special_page == SPECIAL_PAGE_EVENT_CONTROLS || m_special_page == SPECIAL_PAGE_EVENT_SOUND);
    bool in_file = (m_special_page == SPECIAL_PAGE_FILE || m_special_page == SPECIAL_PAGE_FILE_CONFIRM
        || m_special_page == SPECIAL_PAGE_FILE_CONVERT);
    bool in_world_settings = (m_special_page == SPECIAL_PAGE_WORLD_SETTINGS);
    bool in_leveltest_settings = (m_special_page == SPECIAL_PAGE_EDITOR_SETTINGS || m_special_page == SPECIAL_PAGE_LEVELTEST_HELDNPC || m_special_page == SPECIAL_PAGE_MAGICBLOCK);
    bool in_excl_special = in_layers || in_events || in_world_settings || in_leveltest_settings || in_file;
    bool exit_special = false;

    bool currently_in;

    // select
    currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_SELECT;
    if(UpdateButton(mode, sx+0*40+4, 4, GFX.ECursor[2], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipSelect.c_str()))
    {
        if(editorScreen.active)
            swap_screens();
        EditorCursor.Mode = OptCursor_t::LVL_SELECT;
        if(in_excl_special)
            exit_special = true;
    }

    // erase
    currently_in = !in_excl_special && (EditorCursor.Mode == OptCursor_t::LVL_ERASER0 || EditorCursor.Mode == OptCursor_t::LVL_ERASER);
    if(UpdateButton(mode, sx+1*40+4, 4, GFX.ECursor[3], currently_in && EditorCursor.SubMode != -1, 0, 0, 22, 30, g_editorStrings.tooltipErase.c_str()))
    {
        if(editorScreen.active)
            swap_screens();
        EditorCursor.Mode = OptCursor_t::LVL_ERASER0;
        EditorCursor.SubMode = 0;
        if(in_excl_special)
            exit_special = true;
    }
    if(currently_in && UpdateButton(mode, sx+2*40+4, 4, GFXNPC[NPCID_AXE], currently_in && EditorCursor.SubMode == -1, 0, 0, 32, 32, g_editorStrings.tooltipEraseAll.c_str()))
    {
        if(editorScreen.active)
            swap_screens();
        EditorCursor.Mode = OptCursor_t::LVL_ERASER0;
        EditorCursor.SubMode = -1;
        if(in_excl_special)
            exit_special = true;
    }

    // level editor tabs
    if(!WorldEditor)
    {
        // blocks
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_BLOCKS;
        if(UpdateButton(mode, sx+3*40+4, 4, GFXBlock[1], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipBlocks.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_BLOCKS;
            if(in_excl_special)
                exit_special = true;
        }

        // BGOs
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_BGOS;
        if(UpdateButton(mode, sx+4*40+4, 4, GFXBackgroundBMP[1], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipBGOs.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_BGOS;
            if(in_excl_special)
                exit_special = true;
        }

        // NPCs
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_NPCS;
        if(UpdateButton(mode, sx+5*40+4, 4, GFXNPC[1], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipNPCs.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_NPCS;
            if(in_excl_special)
                exit_special = true;
        }

        // water
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_WATER;
        if(!MagicHand && UpdateButton(mode, sx+6*40+4, 4, GFXBackgroundBMP[26], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipWater.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_WATER;
            if(in_excl_special)
                exit_special = true;
        }

        // warps
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_WARPS;
        if(!MagicHand && UpdateButton(mode, sx+7*40+4, 4, GFXBlock[294], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipWarps.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_WARPS;
            if(in_excl_special)
                exit_special = true;
        }

        // level settings
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_SETTINGS;
        if(!MagicHand && UpdateButton(mode, sx+9*40+4, 4, GFXBlock[60], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipSettings.c_str()))
        {
            if(currently_in || !editorScreen.active)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_SETTINGS;
            if(in_excl_special)
                exit_special = true;
        }

        // layers
        if(!MagicHand && UpdateButton(mode, sx+10*40+4, 4, GFXBlock[447], in_layers, 0, 0, 32, 32, g_editorStrings.tooltipLayers.c_str()))
        {
            if(in_layers || !editorScreen.active)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_SELECT;
            optCursor.current = OptCursor_t::LVL_SELECT;
            m_last_mode = OptCursor_t::LVL_SELECT;
            if(!in_layers)
                m_special_page = SPECIAL_PAGE_LAYERS;
        }

        // events
        if(!MagicHand && UpdateButton(mode, sx+11*40+4, 4, GFXBlock[169], in_events, 0, 0, 32, 32, g_editorStrings.tooltipEvents.c_str()))
        {
            if(in_events || !editorScreen.active)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_SELECT;
            optCursor.current = OptCursor_t::LVL_SELECT;
            m_last_mode = OptCursor_t::LVL_SELECT;
            if(!in_events)
                m_special_page = SPECIAL_PAGE_EVENTS;
        }
    }

    // world editor tabs
    if(WorldEditor)
    {
        // tiles
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_TILES;
        if(UpdateButton(mode, sx+3*40+4, 4, GFXTileBMP[1], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipTiles.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_TILES;
            if(in_excl_special)
                exit_special = true;
        }

        // scenes
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_SCENES;
        if(UpdateButton(mode, sx+4*40+4, 4, GFXSceneBMP[1], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipScenes.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_SCENES;
            if(in_excl_special)
                exit_special = true;
        }

        // levels
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_LEVELS;
        if(UpdateButton(mode, sx+5*40+4, 4, GFXLevelBMP[2], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipLevels.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_LEVELS;
            if(in_excl_special)
                exit_special = true;
        }

        // paths
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_PATHS;
        if(UpdateButton(mode, sx+6*40+4, 4, GFXPathBMP[4], currently_in, 0, 0, 32, 32, g_editorStrings.tooltipPaths.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_PATHS;
            if(in_excl_special)
                exit_special = true;
        }

        // world music
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_MUSIC;
        if(UpdateButton(mode, sx+7*40+4, 4, GFX.EIcons, currently_in, 0, 32*Icon::music, 32, 32, g_editorStrings.tooltipMusic.c_str()))
        {
            if(currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_MUSIC;
            if(in_excl_special)
                exit_special = true;
        }

        // world settings
        if(UpdateButton(mode, sx+9*40+4, 4, GFXLevelBMP[15], in_world_settings, 0, 0, 32, 32, g_editorStrings.tooltipSettings.c_str()))
        {
            if(in_world_settings || !editorScreen.active)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_SELECT;
            optCursor.current = OptCursor_t::LVL_SELECT;
            m_last_mode = OptCursor_t::LVL_SELECT;
            if(!in_world_settings)
                m_special_page = SPECIAL_PAGE_WORLD_SETTINGS;
        }
    }

    // file
    if(!MagicHand && UpdateButton(mode, sx+13*40+4, 4, GFX.EIcons, in_file, 0, 32*Icon::page, 32, 32, g_editorStrings.tooltipFile.c_str()))
    {
        if(in_file || !editorScreen.active)
            swap_screens();
        EditorCursor.Mode = OptCursor_t::LVL_SELECT;
        optCursor.current = OptCursor_t::LVL_SELECT;
        m_last_mode = OptCursor_t::LVL_SELECT;
        if(!in_file)
            m_special_page = SPECIAL_PAGE_FILE;
    }

    // test play settings
    if(!WorldEditor && UpdateButton(mode, sx+14*40 + 4, 4, GFX.EIcons, in_leveltest_settings, 0, 32*Icon::play, 32, 32, g_editorStrings.tooltipSettings.c_str()))
    {
        if(in_leveltest_settings || !editorScreen.active)
            swap_screens();
        EditorCursor.Mode = OptCursor_t::LVL_SELECT;
        optCursor.current = OptCursor_t::LVL_SELECT;
        m_last_mode = OptCursor_t::LVL_SELECT;
        if(!in_leveltest_settings)
            m_special_page = SPECIAL_PAGE_EDITOR_SETTINGS;
    }

    // swap screens
    int switch_screens_icon = 0;
    const char* switch_screens_tooltip;
    if(select_bar_only)
    {
        switch_screens_icon = Icon::down;
        switch_screens_tooltip = g_editorStrings.tooltipShow.c_str();
    }
    else
    {
        switch_screens_icon = Icon::up;
        switch_screens_tooltip = nullptr;
    }
    if(UpdateButton(mode, sx+15*40 + 4, 4, GFX.EIcons, false, 0, 32*switch_screens_icon, 32, 32, switch_screens_tooltip))
        swap_screens();

    // if mode has been updated for any reason, close any special dialogue boxes
    // and sync everything up
    if(exit_special || EditorCursor.Mode != m_last_mode)
    {
        if(m_browser_mode != BROWSER_MODE_NONE)
            FileBrowserCleanup();
        m_special_page = SPECIAL_PAGE_NONE;
        m_special_subpage = 0;
        optCursor.current = EditorCursor.Mode;
        m_last_mode = EditorCursor.Mode;
    }

    if(select_bar_only && mode == CallMode::Render && e_CursorY < 40 && e_CursorX >= sx && e_CursorX < sx+e_ScreenW)
    {
        int X = e_CursorX;
        int Y = e_CursorY;
        if(g_config.editor_edge_scroll && !MagicHand)
        {
            if(X < 36)
                X = 36;
            if(Y < 36)
                Y = 36;
            if(X >= ScreenW - 36)
                X = ScreenW - 36;
            if(Y >= ScreenH - 36)
                Y = ScreenH - 36;
        }

#ifdef __3DS__
        if(select_bar_only)
            XRender::renderTexture(X, Y, GFX.ECursor[2]);
#else
        XRender::renderTexture(X, Y, GFX.ECursor[2]);
#endif

        if(e_tooltip)
        {
            if(e_CursorX + 28 < sx + e_ScreenW / 2)
                SuperPrint(e_tooltip, 3, e_CursorX + 28, e_CursorY + 34, 1.0f, 0.7f, 0.7f);
            else
                SuperPrintRightAlign(e_tooltip, 3, e_CursorX + 10, e_CursorY + 34, 1.0f, 0.7f, 0.7f);
        }
    }
}

void EditorScreen::UpdateEditorScreen(CallMode mode, bool second_screen)
{
    // second screen is like the upper screen of the 3DS or the TV of the Wii U -- any screen without a direct pointing device
    if(second_screen && mode == CallMode::Logic)
        return;

    if(MagicHand && !LevelEditor)
        m_special_page = SPECIAL_PAGE_NONE;

    if(mode == CallMode::Logic)
    {
        if(GamePaused != PauseCode::None)
            return;

        MenuMouseRelease = MouseRelease && !MenuMouseRelease && !SharedCursor.Primary;
    }

    e_tooltip = nullptr;

    bool select_bar_only = ((active && second_screen) || (!active && !second_screen));

    if(select_bar_only)
    {
        UpdateSelectorBar(mode, true);

        if(mode == CallMode::Logic)
            MenuMouseRelease = !SharedCursor.Primary;

        return;
    }

    MessageText.clear();
    MessageTextMap.clear();

#ifdef __3DS__
    if(mode == CallMode::Render && active)
        XRender::setTargetSubScreen();
    else if(mode == CallMode::Render)
    {
        XRender::setTargetMainScreen();
        XRender::setViewport(800/2 - e_ScreenW/2, 0, e_ScreenW, e_ScreenH);
    }
#else
    if(mode == CallMode::Render)
        XRender::setViewport(ScreenW/2-e_ScreenW/2, 0, e_ScreenW, e_ScreenH);
#endif

    e_CursorX = EditorCursor.X;
    e_CursorY = EditorCursor.Y;

#ifdef __3DS__
    if(!editorScreen.active)
        e_CursorX -= ScreenW/2-e_ScreenW/2;
#else
    e_CursorX -= ScreenW/2-e_ScreenW/2;
#endif

    // if(WorldEditor)
    //     e_CursorY += 8;

    if(mode == CallMode::Render)
        XRender::renderRect(0, 0, e_ScreenW, e_ScreenH, 0.4f, 0.4f, 0.8f, 0.75f, true);

    UpdateSelectorBar(mode, false);

    if(m_special_page == SPECIAL_PAGE_BROWSER || m_special_page == SPECIAL_PAGE_BROWSER_CONFIRM)
        UpdateBrowserScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_FILE || m_special_page == SPECIAL_PAGE_FILE_CONFIRM
        || m_special_page == SPECIAL_PAGE_FILE_CONVERT)
        UpdateFileScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_LAYERS || m_special_page == SPECIAL_PAGE_LAYER_DELETION
        || m_special_page == SPECIAL_PAGE_OBJ_LAYER || m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
        UpdateLayersScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_EVENTS || m_special_page == SPECIAL_PAGE_EVENT_DELETION)
        UpdateEventsScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_EVENT_SETTINGS || m_special_page == SPECIAL_PAGE_EVENT_BOUNDS || m_special_page == SPECIAL_PAGE_EVENT_CONTROLS)
        UpdateEventSettingsScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_OBJ_TRIGGERS || m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
        UpdateEventsSubScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS || m_special_page == SPECIAL_PAGE_LEVELTEST_HELDNPC)
        UpdateNPCScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_EDITOR_SETTINGS)
        UpdateEditorSettingsScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_WORLD_SETTINGS)
        UpdateWorldSettingsScreen(mode);
    // else if(m_special_page == SPECIAL_PAGE_MAGICBLOCK)
    //     UpdateMagicBlockScreen(mode);
    else if(m_special_page == SPECIAL_PAGE_EVENT_MUSIC || m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND
        || m_special_page == SPECIAL_PAGE_EVENT_SOUND || m_special_page == SPECIAL_PAGE_SECTION_BACKGROUND
        || m_special_page == SPECIAL_PAGE_SECTION_MUSIC || m_special_page == SPECIAL_PAGE_LEVEL_EXIT
        || EditorCursor.Mode == OptCursor_t::WLD_MUSIC || m_special_page == SPECIAL_PAGE_WARP_TRANSITION)
        UpdateSelectListScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::LVL_BLOCKS)
        UpdateBlockScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::LVL_BGOS)
        UpdateBGOScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::LVL_NPCS)
        UpdateNPCScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::LVL_WATER)
        UpdateWaterScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS)
        UpdateWarpScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::LVL_SETTINGS)
        UpdateSectionsScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::WLD_TILES)
        UpdateTileScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::WLD_SCENES)
        UpdateSceneScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::WLD_LEVELS)
        UpdateLevelScreen(mode);
    else if(EditorCursor.Mode == OptCursor_t::WLD_PATHS)
        UpdatePathScreen(mode);

    if(mode == CallMode::Render && e_CursorX >= 0 && GamePaused == PauseCode::None)
    {
#ifndef __3DS__
        XRender::renderTexture(e_CursorX, e_CursorY, GFX.ECursor[2]);
#endif
        if(e_tooltip)
        {
            if(e_CursorX + 28 < e_ScreenW / 2)
                SuperPrint(e_tooltip, 3, e_CursorX + 28, e_CursorY + 34, 1.0f, 0.7f, 0.7f);
            else
                SuperPrintRightAlign(e_tooltip, 3, e_CursorX + 10, e_CursorY + 34, 1.0f, 0.7f, 0.7f);
        }
    }

    if(mode == CallMode::Logic)
        MenuMouseRelease = !SharedCursor.Primary;
}

EditorScreen editorScreen;
