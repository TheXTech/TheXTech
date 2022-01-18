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

#pragma once
#ifndef CSPRITEMANAGER_H
#define CSPRITEMANAGER_H

#include <list>
#include <map>
#include <list>
#include <memory>

#include "sprite_component.h"


class LunaImage;

////////////////////////////
/// BUILTIN SPRITE TYPES /// ****************************************************************************************************
////////////////////////////
//
// 1 = Static / HUD sprite      1: Type     2: Img code     3: Xpos     4: Ypos     5: Active lifetime  6:
// 2 = "In level" sprite        1: Type     2: Img code     3: Xpos     4: Ypos     5: Active lifetime  6:
// 3 = Collectible item         1: Type     2: Img code     3: Xpos     4: Ypos     5: Active lifetime  6: Lundll event to trigger
// 4 = Progress bar             1: Type     2: Img code     3: Xpos     4: Ypos     5: Active lifetime  6:
// 5 = Phanto                   1: Type     2: Img code     3: Xpos     4: Ypos     5: Active lifetime  6:
//
//////////////////////////// ****************************************************************************************************

enum BUILTIN_SPRITE_TYPE
{
    BST_Custom = 0,
    BST_Static = 1,
    BST_Normal = 2,
    BST_Item = 3,
    BST_Bar = 4,
    BST_Phanto
};


struct CSpriteRequest;
class CSprite;

// Basic manager class for sprites
struct CSpriteManager
{
    void ResetSpriteManager();                                          // Will be called on level load to re-init everything

    void BasicInit(CSprite *spr, CSpriteRequest *req, bool center);     // Quickly set frequently-used sprite attributes
    void InitializeDimensions(CSprite *spr, bool center_coords);        // Set hitbox and dimensions to size of image
    void InstantiateSprite(CSpriteRequest *req, bool center_coords);    // Officially place sprite in level (m_SpriteList)

    void AddBlueprint(const char *blueprint_name, CSprite *spr);
    CSprite *CopyFromBlueprint(const char *blueprint_name);// Generate new sprite based on blueprint

    void RunSprites();
    int CountSprites() const
    {
        return (int)m_SpriteList.size();
    }

    int CountBlueprints() const
    {
        return (int)m_SpriteBlueprints.size();
    }

    int CountComponents() const
    {
        return (int)m_ComponentList.size();
    }

    void ClearInvalidSprites(); // Don't call while iterating
    void ClearAllSprites();
    void ClearSprites(int imgResourceCode, int xPos, int yPos);
    void ClearSprites(int imgResourceCode);
    void ClearSprites(const std::shared_ptr<LunaImage> &img, int xPos, int yPos);
    void ClearSprites(const std::shared_ptr<LunaImage> &img);

    void AddSprite(CSprite *spr);

    void GetComponents(int code, std::list<SpriteComponent *> *component_list); // Get components with the given code #

    std::list<CSprite *> m_SpriteList;
    std::map<std::string, CSprite *> m_SpriteBlueprints;
    std::list<SpriteComponent> m_ComponentList;     // User components that can be copied (activated) into a sprite's behavior list
};

extern CSpriteManager gSpriteMan;

#endif // CSPRITEMANAGER_H
