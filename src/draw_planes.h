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

#pragma once
#ifndef DRAW_PLANES_H
#define DRAW_PLANES_H

enum PLANE
{

    PLANE_INTERNAL_BG    = 0x00, // reserved for engine-internal purposes

    PLANE_GAME_BACKDROP  = 0x08, // backdrop, drawn behind level / world map screens

    PLANE_LVL_3D_NEG2    = 0x00, // start of -2 plane on 3DS (level)
    PLANE_LVL_BG         = 0x10, // level background (background2)
    PLANE_LVL_3D_NEG1    = 0x18, // start of -1 plane on 3DS (level)
    PLANE_LVL_BGO_LOW    = 0x18, // special BGOs drawn below sizables
    PLANE_LVL_SBLOCK     = 0x20, // sizable blocks only
    PLANE_LVL_BGO_NORM   = 0x28, // includes locks at +1
    PLANE_LVL_3D_MAIN    = 0x30, // start of +0 plane on 3DS (level)
    PLANE_LVL_NPC_BG     = 0x30, // NPCs drawn below blocks (includes warping NPCs)
    PLANE_LVL_PLR_WARP   = 0x38, // players warping and their held items
    PLANE_LVL_BLK_NORM   = 0x40, // normal blocks
    PLANE_LVL_EFF_LOW    = 0x48, // low effects
    PLANE_LVL_NPC_LOW    = 0x50, // low NPCs (includes frozen NPCs at +1)
    PLANE_LVL_NPC_NORM   = 0x58, // includes NPC chat indicators
    PLANE_LVL_PLR_NORM   = 0x60, // player graphics (including mounts and held NPCs)
    PLANE_LVL_BGO_FG     = 0x68, // foreground BGOs
    PLANE_LVL_NPC_FG     = 0x70, // foreground NPCs
    PLANE_LVL_BLK_HURTS  = 0x78, // lava and spike blocks
    PLANE_LVL_BGO_TOP    = 0x80, // special BGOs drawn above hurt blocks and FG NPCs
    PLANE_LVL_EFF_NORM   = 0x88, // normal effects
    PLANE_LVL_INFO       = 0x90, // level info, currently includes warp info (star count), editor water rects, and editor info
    PLANE_LVL_SECTION_FG = 0x98, // highest game-space draws, includes Luna draws, Section FX
    PLANE_LVL_3D_POS1    = 0xA0, // start of +1 plane on 3DS (level)
    PLANE_LVL_HUD        = 0xA0, // includes dropped NPCs (at +1)
    PLANE_LVL_META       = 0xA8, // information drawn above the HUD but still in the vScreen (includes level vScreen fader)

    PLANE_WLD_3D_NEG2    = 0x00, // start of -2 plane on 3DS (world)
    PLANE_WLD_BG         = 0x10, // reserved for future background draws on the world map
    PLANE_WLD_3D_NEG1    = 0x18, // start of -1 plane on 3DS (world)
    PLANE_WLD_TIL        = 0x18, // world map tiles
    PLANE_WLD_SCN        = 0x20, // world map scenes
    PLANE_WLD_PTH        = 0x28, // world map paths
    PLANE_WLD_LVL        = 0x30, // world map levels
    PLANE_WLD_PLR        = 0x38, // world map players
    PLANE_WLD_3D_MAIN    = 0x88, // start of +0 plane on 3DS (world)
    PLANE_WLD_EFF        = 0x88, // world map effects (editor-exclusive)
    PLANE_WLD_INFO       = 0x90, // world map info, currently includes level info (star count), editor music boxes, and editor info
    PLANE_WLD_FRAME      = 0x98, // world map frame, drawn below HUD
    PLANE_WLD_3D_POS1    = 0xA0, // start of +1 plane on 3DS (world)
    PLANE_WLD_HUD        = 0xA0, // world map HUD (player preview, level title, etc)
    PLANE_WLD_META       = 0xA8, // information drawn above the HUD but still in the screen (includes world screen fader)

    PLANE_GAME_MENUS     = 0xD0, // game menus drawn above the level / world graphics (including main menu, pause menu, message box, etc)
    PLANE_GAME_META      = 0xD8, // information drawn above the game menus (including game screen fader)
    PLANE_INTERNAL_FG    = 0xF0, // reserved for engine-internal purposes
};


#endif
