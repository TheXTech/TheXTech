/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <vector>
#include <array>
#include <algorithm>

#include <Logger/logger.h>

#include "globals.h"
#include "collision.h"
#include "main/trees.h"

#include "editor/magic_block.h"
#include "editor/editor_custom.h"
#include "editor.h"

#include "rand.h"

namespace MagicBlock
{

using namespace EditorCustom;


bool enabled = false;
bool replace_existing = false;
bool advanced_mode = false;
bool count_level_edges = true;

CrossEffectLevel check_level = LEVEL_FAMILY;
CrossEffectLevel change_level = LEVEL_FAMILY;



static int s_score_match(const ItemType_t candidate, const ItemType_t target)
{
    // don't consider it if width or height mismatched
    if(candidate.width != target.width)
        return -1;
    if(candidate.height != target.height)
        return -1;

    // don't consider it if group subcategory mismatched
    if(candidate.group != target.group)
        return -1;

    int score = 0;

    // penalize 6 for unmatched UDLR, 2 for unmatched diagonal
    if(candidate.has_4 != target.has_4)
        score += 8;
    if(candidate.has_8 != target.has_8)
        score += 8;
    if(candidate.has_6 != target.has_6)
        score += 8;
    if(candidate.has_2 != target.has_2)
        score += 8;

    if(candidate.has_1 != target.has_1)
        score += 2;
    if(candidate.has_3 != target.has_3)
        score += 2;
    if(candidate.has_7 != target.has_7)
        score += 2;
    if(candidate.has_9 != target.has_9)
        score += 2;

    // care just a tad more about top
    if(candidate.has_8 != target.has_8)
        score += 1;

    // especially penalize missing connections
    if(target.has_4 && !candidate.has_4)
        score += 4;
    if(target.has_8 && !candidate.has_8)
        score += 4;
    if(target.has_6 && !candidate.has_6)
        score += 4;
    if(target.has_2 && !candidate.has_2)
        score += 4;

    if(target.has_1 && !candidate.has_1)
        score += 1;
    if(target.has_3 && !candidate.has_3)
        score += 1;
    if(target.has_7 && !candidate.has_7)
        score += 1;
    if(target.has_9 && !candidate.has_9)
        score += 1;

    // penalize 3 for unmatched slope existence, additional 1 for unmatched slope number
    if((target.slope == ItemType_t::no_slope) && (candidate.slope != ItemType_t::no_slope))
        score += 3;
    if(candidate.slope != target.slope)
        score += 1;

    return score;
}

ItemType_t s_match_type(const ItemFamily& family, const ItemType_t inferred_type)
{
    ItemType_t best_type;
    int best_score = -1;

#if 0
    printf("Has inferred adjacency: ");
    if(inferred_type.has_1) printf("1");
    if(inferred_type.has_2) printf("2");
    if(inferred_type.has_3) printf("3");
    if(inferred_type.has_4) printf("4");
    if(inferred_type.has_6) printf("6");
    if(inferred_type.has_7) printf("7");
    if(inferred_type.has_8) printf("8");
    if(inferred_type.has_9) printf("9");
    printf(" (slope %d)\n", inferred_type.slope);
    printf("Self is %d\n", inferred_type.type);
#endif

    for(const ItemType_t t : family.types)
    {
        if(t.type == inferred_type.type)
        {
            best_type = t;
            best_score = s_score_match(t, inferred_type);
            // printf("Initial self best is %d\n", best_score);
            break;
        }
    }

    int matches = 1;

    for(const ItemType_t t : family.types)
    {
        int score = s_score_match(t, inferred_type);

        if(score == -1)
            continue;

        // printf("%d score is %d\n", t.type, score);

        if(best_score == -1 || score < best_score)
        {
            best_type = t;
            best_score = score;
        }
        else if(score == best_score && best_type.type != inferred_type.type)
        {
            matches++;

            if(t.type == inferred_type.type || iRand(matches) == 0)
                best_type = t;
        }
    }

    return best_type;
}

template<class ItemRef_t>
TreeResult_Sentinel<ItemRef_t> treeQuery(const Location_t& loc, int sort_mode);

template<>
TreeResult_Sentinel<BlockRef_t> treeQuery<BlockRef_t>(const Location_t& loc, int sort_mode)
{
    return treeBlockQuery(loc, sort_mode);
}

template<>
TreeResult_Sentinel<TileRef_t> treeQuery<TileRef_t>(const Location_t& loc, int sort_mode)
{
    return treeWorldTileQuery(loc, sort_mode);
}

template<>
TreeResult_Sentinel<BackgroundRef_t> treeQuery<BackgroundRef_t>(const Location_t& loc, int sort_mode)
{
    return treeBackgroundQuery(loc, sort_mode);
}

template<class ItemRef_t>
bool s_check_hidden(ItemRef_t B)
{
    return B->Hidden;
}

template<>
bool s_check_hidden(TileRef_t B)
{
    UNUSED(B);
    return false;
}

template<class ItemRef_t>
struct MagicInfo {};

template<>
struct MagicInfo<BlockRef_t>
{
    static const int maxType = maxBlockType;
    static constexpr const int& numItem = numBlock;
    static constexpr std::vector<ItemFamily>& families = block_families;
    static constexpr const std::array<uint8_t, maxType>& family_by_type = block_family_by_type;
};

template<>
struct MagicInfo<TileRef_t>
{
    static const int maxType = maxTileType;
    static constexpr const int& numItem = numTiles;
    static constexpr std::vector<ItemFamily>& families = tile_families;
    static constexpr const std::array<uint8_t, maxType>& family_by_type = tile_family_by_type;
};

template<>
struct MagicInfo<BackgroundRef_t>
{
    static const int maxType = maxBackgroundType;
    static constexpr const int& numItem = numBackground;
    static constexpr std::vector<ItemFamily>& families = bgo_families;
    static constexpr const std::array<uint8_t, maxType>& family_by_type = bgo_family_by_type;
};

template<class ItemRef_t>
int s_pick_type(ItemFamily& family, ItemRef_t A)
{
    ItemType_t inferred_type;

    inferred_type.type = A->Type;
    inferred_type.width = std::round(A->Location.Width / 32);
    inferred_type.height = std::round(A->Location.Height / 32);

    for(ItemType_t t : family.types)
    {
        if(t.type == A->Type)
        {
            inferred_type.group = t.group;
            inferred_type.slope = t.slope;
            break;
        }
    }

    Location_t tempLoc = A->Location;

    // 7 (top-left)
    tempLoc.X -= 31;
    tempLoc.Width = 30;
    tempLoc.Y -= 31;
    tempLoc.Height = 30;

    if(count_level_edges && !WorldEditor && (tempLoc.X + tempLoc.Width < level[curSection].X || tempLoc.Y + tempLoc.Height < level[curSection].Y))
        inferred_type.has_7 = true;
    else for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
    {
        if(A != B && !s_check_hidden(B) && CheckCollision(tempLoc, B->Location))
        {
            bool hit = false;
            for(ItemType_t t : family.types)
            {
                if(t.type == B->Type && (check_level != LEVEL_GROUP || t.group == inferred_type.group))
                {
                    if(t.has_3 || !advanced_mode)
                    {
                        hit = true;
                        break;
                    }
                }
            }

            if(hit || check_level == LEVEL_ALL)
            {
                inferred_type.has_7 = true;
                break;
            }
        }
    }

    // 8 (top)
    tempLoc.X = A->Location.X + 1;
    tempLoc.Width = A->Location.Width - 2;

    if(count_level_edges && !WorldEditor && tempLoc.Y + tempLoc.Height < level[curSection].Y)
        inferred_type.has_8 = true;
    else for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
    {
        if(A != B && !s_check_hidden(B) && CheckCollision(tempLoc, B->Location))
        {
            bool hit = false;
            for(ItemType_t t : family.types)
            {
                if(t.type == B->Type && (check_level != LEVEL_GROUP || t.group == inferred_type.group))
                {
                    if(t.has_2 || !advanced_mode)
                    {
                        if((t.has_1 || t.has_3) && !t.has_8 && t.slope > inferred_type.slope)
                            inferred_type.slope = t.slope;

                        hit = true;
                        break;
                    }
                }
            }

            if(hit || check_level == LEVEL_ALL)
            {
                inferred_type.has_8 = true;
                break;
            }
        }
    }

    // 9 (top-right)
    tempLoc.X = A->Location.X + A->Location.Width + 1;
    tempLoc.Width = 30;

    if(count_level_edges && !WorldEditor && (tempLoc.X > level[curSection].Width || tempLoc.Y + tempLoc.Height < level[curSection].Y))
        inferred_type.has_9 = true;
    else for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
    {
        if(A != B && !s_check_hidden(B) && CheckCollision(tempLoc, B->Location))
        {
            bool hit = false;
            for(ItemType_t t : family.types)
            {
                if(t.type == B->Type && (check_level != LEVEL_GROUP || t.group == inferred_type.group))
                {
                    if(t.has_1 || !advanced_mode)
                    {
                        hit = true;
                        break;
                    }
                }
            }

            if(hit || check_level == LEVEL_ALL)
            {
                inferred_type.has_9 = true;
                break;
            }
        }
    }

    // 6 (right)
    tempLoc.Y = A->Location.Y + 1;
    tempLoc.Height = A->Location.Height - 2;

    if(count_level_edges && !WorldEditor && tempLoc.X > level[curSection].Width)
        inferred_type.has_6 = true;
    else for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
    {
        if(A != B && !s_check_hidden(B) && CheckCollision(tempLoc, B->Location))
        {
            bool hit = false;
            for(ItemType_t t : family.types)
            {
                if(t.type == B->Type && (check_level != LEVEL_GROUP || t.group == inferred_type.group))
                {
                    if(t.has_4 || !advanced_mode)
                    {
                        if((t.has_1 || t.has_7) && !t.has_6 && t.slope > inferred_type.slope)
                            inferred_type.slope = t.slope;

                        hit = true;
                        break;
                    }
                }
            }

            if(hit || check_level == LEVEL_ALL)
            {
                inferred_type.has_6 = true;
                break;
            }
        }
    }

    // 3 (bottom-right)
    tempLoc.Y = A->Location.Y + A->Location.Height + 1;
    tempLoc.Height = 30;

    if(count_level_edges && !WorldEditor && (tempLoc.X > level[curSection].Width || tempLoc.Y > level[curSection].Height))
        inferred_type.has_3 = true;
    else for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
    {
        if(A != B && !s_check_hidden(B) && CheckCollision(tempLoc, B->Location))
        {
            bool hit = false;
            for(ItemType_t t : family.types)
            {
                if(t.type == B->Type && (check_level != LEVEL_GROUP || t.group == inferred_type.group))
                {
                    if(t.has_7 || !advanced_mode)
                    {
                        hit = true;
                        break;
                    }
                }
            }

            if(hit || check_level == LEVEL_ALL)
            {
                inferred_type.has_3 = true;
                break;
            }
        }
    }

    // 2 (bottom)
    tempLoc.X = A->Location.X + 1;
    tempLoc.Width = A->Location.Width - 2;

    if(count_level_edges && !WorldEditor && tempLoc.Y > level[curSection].Height)
        inferred_type.has_2 = true;
    else for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
    {
        if(A != B && !s_check_hidden(B) && CheckCollision(tempLoc, B->Location))
        {
            bool hit = false;
            for(ItemType_t t : family.types)
            {
                if(t.type == B->Type && (check_level != LEVEL_GROUP || t.group == inferred_type.group))
                {
                    if(t.has_8 || !advanced_mode)
                    {
                        if((t.has_7 || t.has_9) && !t.has_2 && t.slope > inferred_type.slope)
                            inferred_type.slope = t.slope;

                        hit = true;
                        break;
                    }
                }
            }

            if(hit || check_level == LEVEL_ALL)
            {
                inferred_type.has_2 = true;
                break;
            }
        }
    }

    // 1 (bottom-left)
    tempLoc.X = A->Location.X - 31;
    tempLoc.Width = 30;

    if(count_level_edges && !WorldEditor && (tempLoc.X + tempLoc.Width < level[curSection].X || tempLoc.Y > level[curSection].Height))
        inferred_type.has_1 = true;
    else for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
    {
        if(A != B && !s_check_hidden(B) && CheckCollision(tempLoc, B->Location))
        {
            bool hit = false;
            for(ItemType_t t : family.types)
            {
                if(t.type == B->Type && (check_level != LEVEL_GROUP || t.group == inferred_type.group))
                {
                    if(t.has_9 || !advanced_mode)
                    {
                        hit = true;
                        break;
                    }
                }
            }

            if(hit || check_level == LEVEL_ALL)
            {
                inferred_type.has_1 = true;
                break;
            }
        }
    }

    // 4 (left)
    tempLoc.Y = A->Location.Y + 1;
    tempLoc.Height = A->Location.Height - 2;

    if(count_level_edges && !WorldEditor && tempLoc.X + tempLoc.Width < level[curSection].X)
        inferred_type.has_4 = true;
    else for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
    {
        if(A != B && !s_check_hidden(B) && CheckCollision(tempLoc, B->Location))
        {
            bool hit = false;
            for(ItemType_t t : family.types)
            {
                if(t.type == B->Type && (check_level != LEVEL_GROUP || t.group == inferred_type.group))
                {
                    if(t.has_6 || !advanced_mode)
                    {
                        if((t.has_3 || t.has_9) && !t.has_4 && t.slope > inferred_type.slope)
                            inferred_type.slope = t.slope;

                        hit = true;
                        break;
                    }
                }
            }

            if(hit || check_level == LEVEL_ALL)
            {
                inferred_type.has_4 = true;
                break;
            }
        }
    }

    return s_match_type(family, inferred_type).type;
}


template<class ItemRef_t>
void s_apply_type(ItemRef_t B, int type)
{
    B->Type = type;
}

template<>
void s_apply_type(BlockRef_t B, int type)
{
    if(B->Slippy ==
        (B->Type == 189 || B->Type == 190 || B->Type == 191
            || B->Type == 270 || B->Type == 271 || B->Type == 272
            || B->Type == 620 || B->Type == 621 || B->Type == 633
            || B->Type == 634 || B->Type == 241 || B->Type == 242))
    {
        B->Slippy = (type == 189 || type == 190 || type == 191 || type == 270 || type == 271 || type == 272 || type == 620 || type == 621 || type == 633 || type == 634 || type == 241 || type == 242);
    }

    B->Type = type;
}

template<class ItemRef_t>
void MagicItem(int Type, Location_t loc)
{
    using ItemInfo = MagicInfo<ItemRef_t>;

    if(Type < 1 || Type > ItemInfo::maxType)
        return;

    uint8_t family = ItemInfo::family_by_type[Type - 1];

    if(family == FAMILY_NONE)
        return;

    if(change_level != LEVEL_ALL && ItemInfo::families[family].is_misc)
        return;

    int group = 0;

    for(const ItemType_t& t : ItemInfo::families[family].types)
    {
        if(t.type == Type)
        {
            group = t.group;
            break;
        }
    }

    // first, transform all nearby blocks, then transform the block itself
    Location_t tempLoc = loc;
    tempLoc.X -= 31;
    tempLoc.Y -= 31;
    tempLoc.Width += 62;
    tempLoc.Height += 62;

    for(int i = 0; i < 2; i++)
    {
        for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
        {
            if(B->Type < 1 || B->Type > ItemInfo::maxType)
                continue;

            uint8_t family_b = ItemInfo::family_by_type[B->Type - 1];

            if(family_b == FAMILY_NONE)
                continue;

            if(change_level != LEVEL_ALL && family_b != family)
                continue;

            if(!CheckCollision(tempLoc, B->Location))
                continue;

            if(change_level >= LEVEL_GROUP)
            {
                bool group_match = false;
                for(const ItemType_t& t : ItemInfo::families[family_b].types)
                {
                    if(t.type == B->Type && t.group == group)
                    {
                        group_match = true;
                        break;
                    }
                }

                if(!group_match)
                    continue;
            }

            int new_type = s_pick_type(ItemInfo::families[family_b], B);
            if(new_type != 0)
                s_apply_type(B, new_type);
        }
    }
}

template<class ItemRef_t>
void MagicItem(ItemRef_t A)
{
    using ItemInfo = MagicInfo<ItemRef_t>;

    if((int)A < 1 || (int)A > ItemInfo::numItem)
        return;
    if(A->Type < 1 || A->Type > ItemInfo::maxType)
        return;

    uint8_t family = ItemInfo::family_by_type[A->Type - 1];

    if(family == FAMILY_NONE)
        return;

    if(change_level != LEVEL_ALL && ItemInfo::families[family].is_misc)
        return;

    int group = 0;

    for(const ItemType_t& t : ItemInfo::families[family].types)
    {
        if(t.type == A->Type)
        {
            group = t.group;
            break;
        }
    }

    // first, transform all nearby blocks, then transform the block itself
    Location_t tempLoc = A->Location;
    tempLoc.X -= 31;
    tempLoc.Y -= 31;
    tempLoc.Width += 62;
    tempLoc.Height += 62;

    for(int i = 0; i < 2; i++)
    {
        for(ItemRef_t B : treeQuery<ItemRef_t>(tempLoc, SORTMODE_NONE))
        {
            if(B == A)
                continue;

            if(B->Type < 1 || B->Type > ItemInfo::maxType)
                continue;

            uint8_t family_b = ItemInfo::family_by_type[B->Type - 1];

            if(family_b == FAMILY_NONE)
                continue;

            if(change_level != LEVEL_ALL && family_b != family)
                continue;

            if(!CheckCollision(tempLoc, B->Location))
                continue;

            if(change_level == LEVEL_GROUP)
            {
                bool group_match = false;
                for(const ItemType_t& t : ItemInfo::families[family_b].types)
                {
                    if(t.type == B->Type && t.group == group)
                    {
                        group_match = true;
                        break;
                    }
                }

                if(!group_match)
                    continue;
            }

            int new_type = s_pick_type(ItemInfo::families[family_b], B);
            if(new_type != 0)
                s_apply_type(B, new_type);
        }

        int new_type = s_pick_type(ItemInfo::families[family], A);
        if(new_type != 0)
            s_apply_type(A, new_type);
    }
}

void MagicBlock(int Type, Location_t loc)
{
    if(!enabled)
        return;

    MagicItem<BlockRef_t>(Type, loc);
}

void MagicBlock(BlockRef_t A)
{
    if(!enabled)
        return;

    MagicItem(A);
}

void MagicBackground(int Type, Location_t loc)
{
    if(!enabled)
        return;

    MagicItem<BackgroundRef_t>(Type, loc);
}

void MagicBackground(BackgroundRef_t A)
{
    if(!enabled)
        return;

    MagicItem(A);
}

void MagicTile(int Type, Location_t loc)
{
    if(!enabled)
        return;

    MagicItem<TileRef_t>(Type, loc);
}

void MagicTile(TileRef_t A)
{
    if(!enabled)
        return;

    MagicItem(A);
}

} // namespace MagicBlock
