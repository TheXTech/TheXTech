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

#include "lunaspriteman.h"
#include "csprite.h"
#include "lunarender.h"
#include "sprite_funcs.h"
#include "lunaplayer.h"
#include "lunamisc.h"
#include "lunaimgbox.h"
#include "game_main.h" // GamePaused

#include "globals.h"


CSpriteManager gSpriteMan;


void CSpriteManager::ResetSpriteManager()
{
    ClearAllSprites();
    m_ComponentList.clear();
}

void CSpriteManager::BasicInit(CSprite *spr, CSpriteRequest *pReq, bool center)
{
    spr->m_Xpos = pReq->x;
    spr->m_Ypos = pReq->y;
    //spr->m_StaticScreenPos = false;
    spr->SetImageResource(pReq->img_resource_code);
    spr->SetImage(pReq->direct_img);
    InitializeDimensions(spr, center);
    if(pReq->time != 0)
        spr->MakeLimitedLifetime(pReq->time);
}

void CSpriteManager::InitializeDimensions(CSprite *spr, bool center_coords)
{
    const LunaImage *box = spr->m_directImg;

    if(!box)
        box = Renderer::Get().GetImageForResourceCode(spr->m_ImgResCode);

    if(box)
    {
        LunaRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = box->getW();
        rect.bottom = box->getH();
        spr->m_GfxRects.clear();
        spr->m_GfxRects.push_back(rect);

        spr->m_Ht = (double)rect.bottom;
        spr->m_Wd = (double)rect.right;
        spr->m_AnimationSet = false;

        spr->m_Hitbox.Left_off = 0;
        spr->m_Hitbox.Top_off = 0;
        spr->m_Hitbox.W = (short)rect.right;
        spr->m_Hitbox.H = (short)rect.bottom;
        spr->m_Hitbox.CollisionType = 0;
        spr->m_Hitbox.pParent = spr;

        if(center_coords) // Fix to generate with x/y as center instead of minimum left/top
        {
            spr->m_Xpos -= spr->m_Wd / 2;
            spr->m_Ypos -= spr->m_Ht / 2;
        }
    }
}

void CSpriteManager::InstantiateSprite(CSpriteRequest *req, bool center_coords)
{
    CSprite *spr = nullptr;

    // For built in sprites
    if(req != nullptr && req->type != (BUILTIN_SPRITE_TYPE)BST_Custom)
    {
        switch(req->type)
        {

        // STATIC SPRITE
        case BST_Static:
        {
            spr = new CSprite;
            BasicInit(spr, req, center_coords);
            spr->m_StaticScreenPos = true;
            spr->AddDrawComponent(&SpriteFunc::StaticDraw);
            break;
        }

        // NORMAL SPRITE
        case BST_Normal:
        {
            spr = new CSprite;
            BasicInit(spr, req, center_coords);
            spr->AddDrawComponent(&SpriteFunc::RelativeDraw);
            break;
        }

        // ITEM SPRITE
        case BST_Item:
        {
            spr = new CSprite;
            BasicInit(spr, req, center_coords);
            spr->AddDrawComponent(&SpriteFunc::RelativeDraw);

            // Add collectible event # if applicable
            int str_arg = std::atoi(req->str.c_str());
            if(str_arg > 21)
            {
                SpriteComponent comp;                       // Add collectible component
                comp.func = SpriteFunc::PlayerCollectible;
                spr->AddBehaviorComponent(comp);

                if(str_arg > 21)                        // Add trigger event on death
                {
                    comp.func = SpriteFunc::TriggerLunaEvent;
                    comp.data1 = str_arg;
                    spr->AddDeathComponent(comp);
                }

            }
            break;
        }

        // PROGRESS BAR
        case BST_Bar:
        {
            //spr = new CSprite;
            //TODO: MAKE IT
            spr = nullptr;
            spr->m_Xpos = req->x;
            spr->m_Ypos = req->y;
            break;
        }

        // PHANTO
        case BST_Phanto:
        {
            // Need to add phanto to blueprints?
            if(m_SpriteBlueprints.find("__DefaultPhanto") == m_SpriteBlueprints.end())
            {
                spr = new CSprite;
                SpriteComponent comp;
                BasicInit(spr, req, center_coords);
                spr->AddDrawComponent(&SpriteFunc::RelativeDraw);

                // Always Decelerate
                comp.Init(0);
                comp.func = SpriteFunc::Deccelerate;
                comp.data1 = 0.04;
                comp.data2 = 0.04;
                comp.data3 = 0.00;
                spr->AddBehaviorComponent(comp);

                // Wait for player to have key -- activate 100
                comp.Init(0);
                comp.func = SpriteFunc::PlayerHoldingSprite;
                comp.data1 = 31; // key ID
                comp.data4 = 100; // activates 100
                spr->AddBehaviorComponent(comp);

                comp.Init(0);
                comp.func = SpriteFunc::PlayerHoldingSprite;
                comp.data1 = 0x0E; // key ID alternate
                comp.data4 = 100; // activates 100
                spr->AddBehaviorComponent(comp);

                comp.Init(0); // Phase Move
                comp.func = SpriteFunc::PhaseMove;
                //comp.lookup_code;
                spr->AddBehaviorComponent(comp);

                // #100 -- Chase player / active mode
                comp.Init(1);
                comp.func = SpriteFunc::AccelToPlayer;
                comp.data1 = 0.11;
                comp.data2 = 0.11;
                comp.data3 = 5.6;
                comp.lookup_code = 100;
                gSpriteMan.m_ComponentList.push_back(comp);

                comp.Init(1); // Check for collision -- activate 101
                comp.func = SpriteFunc::OnPlayerCollide;
                comp.data4 = 101; // harm player
                comp.lookup_code = 100;
                gSpriteMan.m_ComponentList.push_back(comp);

                comp.Init(1); // Check for too much distance from player -- activate 102
                comp.func = SpriteFunc::OnPlayerDistance;
                comp.data1 = 3000;
                comp.data4 = 102; // tele near player
                comp.lookup_code = 100;
                gSpriteMan.m_ComponentList.push_back(comp);

                // #101 -- harm player
                comp.Init(1); // Check for too much distance from player
                comp.func = SpriteFunc::HarmPlayer;
                comp.lookup_code = 101;
                gSpriteMan.m_ComponentList.push_back(comp);

                // #102 -- teleport near player
                comp.Init(1); // Check for too much distance from player
                comp.func = SpriteFunc::TeleportNearPlayer;
                comp.data1 = 1000;
                comp.lookup_code = 102;
                gSpriteMan.m_ComponentList.push_back(comp);
                m_SpriteBlueprints["__DefaultPhanto"] = spr;
            }

            const char *defPhant = "__DefaultPhanto";
            CSprite *from_bp = CopyFromBlueprint(defPhant);
            from_bp->m_Xpos = req->x;
            from_bp->m_Ypos = req->y;
            from_bp->SetImageResource(req->img_resource_code);
            from_bp->SetImage(req->direct_img);
            InitializeDimensions(from_bp, center_coords);
            if(req->time != 0)
                from_bp->MakeLimitedLifetime(req->time);
            spr = from_bp;
            break;
        }

        } //< Switch

        // Add the sprite
        if(spr != nullptr)
            AddSprite(spr);
    }
    // Else, instantiate custom sprite?
    else if(req != nullptr && req->type == BST_Custom)
    {
        CSprite *from_bp = CopyFromBlueprint(const_cast<char *>(req->str.c_str()));

        if(from_bp)
        {
            from_bp->m_Xpos = req->x;
            from_bp->m_Ypos = req->y;
            from_bp->m_Xspd = req->x_speed;
            from_bp->m_Yspd = req->y_speed;
            from_bp->SetImageResource(req->img_resource_code);
            from_bp->SetImage(req->direct_img);
            InitializeDimensions(from_bp, center_coords);
            if(req->time != 0)
                from_bp->MakeLimitedLifetime(req->time);
            from_bp->Birth();
            AddSprite(from_bp);
        }
    }
}

void CSpriteManager::AddBlueprint(const char *blueprint_name, CSprite *spr)
{
    m_SpriteBlueprints[blueprint_name] = spr;
}

CSprite *CSpriteManager::CopyFromBlueprint(const char *blueprint_name)
{
    if(m_SpriteBlueprints.find(blueprint_name) != m_SpriteBlueprints.end())
    {
        auto *newspr = new CSprite;
        *newspr = *m_SpriteBlueprints[blueprint_name];
        return newspr;
    }
    else
        return nullptr;
}

void CSpriteManager::RunSprites()
{
    ClearInvalidSprites();
    Player_t *demo = PlayerF::Get(1);

    if(demo)
    {
        // Process each
        if(GamePaused == PauseCode::None)
        {
            for(auto &iter : m_SpriteList)
            {
                if(!iter->m_Invalidated)  // Don't process invalids
                {
                    if(ComputeLevelSection((int)iter->m_Xpos, (int)iter->m_Ypos) == demo->Section + 1 ||
                       iter->m_AlwaysProcess || iter->m_StaticScreenPos)   // Valid level section to process in?
                        iter->Process();
                }
                else
                    m_hasInvalid = true;
            }
        }

        // Draw each
        for(auto &iter : m_SpriteList)
        {
            if(!iter->m_Invalidated)
            {
                if(iter->m_StaticScreenPos || Render::IsOnScreen(iter->m_Xpos, iter->m_Ypos, iter->m_Wd, iter->m_Ht))
                    iter->Draw();
            }
            else
                m_hasInvalid = true;
        }
    }
}

void CSpriteManager::ClearInvalidSprites()
{
    if(!m_hasInvalid)
        return;

    auto iter = m_SpriteList.begin();
    // std::list<CSprite*>::iterator end = m_SpriteList.end();

    while(iter != m_SpriteList.end())
    {
        //CSprite* spr = *iter;
        if((*iter)->m_Invalidated)
        {
            delete(*iter);
            iter = m_SpriteList.erase(iter);
        }
        else
            ++iter;
    }

    m_hasInvalid = false;
}

void CSpriteManager::ClearAllSprites()
{
    while(!m_SpriteList.empty())
    {
        delete m_SpriteList.back();
        m_SpriteList.pop_back();
    }
}

void CSpriteManager::ClearSprites(int imgResourceCode, int xPos, int yPos)
{
    auto iter = m_SpriteList.begin();
    //    std::list<CSprite*>::iterator end = m_SpriteList.end();

    while(iter != m_SpriteList.end())
    {
        //CSprite* spr = *iter;
        CSprite *next = *iter;
        if(next->m_ImgResCode == imgResourceCode && (int)next->m_Xpos == xPos && (int)next->m_Ypos == yPos)
        {
            delete(*iter);
            iter = m_SpriteList.erase(iter);
        }
        else
            ++iter;
    }
}

void CSpriteManager::ClearSprites(int imgResourceCode)
{
    auto iter = m_SpriteList.begin();
    //    std::list<CSprite*>::iterator end = m_SpriteList.end();

    while(iter != m_SpriteList.end())
    {
        //CSprite* spr = *iter;
        CSprite *next = *iter;

        if(!next->m_directImg && next->m_ImgResCode == imgResourceCode)
        {
            delete(*iter);
            iter = m_SpriteList.erase(iter);
        }
        else
            ++iter;
    }
}

void CSpriteManager::ClearSprites(LunaImage *img, int xPos, int yPos)
{
    std::list<CSprite *>::iterator iter = m_SpriteList.begin();
    //    std::list<CSprite*>::iterator end = m_SpriteList.end();

    while(iter != m_SpriteList.end())
    {
        //CSprite* spr = *iter;
        CSprite *next = *iter;
        if(next->m_directImg && next->m_directImg->getUID() == img->getUID() &&
           (int)next->m_Xpos == xPos && (int)next->m_Ypos == yPos)
        {
            delete(*iter);
            iter = m_SpriteList.erase(iter);
        }
        else
            ++iter;
    }
}

void CSpriteManager::ClearSprites(LunaImage *img)
{
    std::list<CSprite *>::iterator iter = m_SpriteList.begin();
    // std::list<CSprite*>::iterator end = m_SpriteList.end();

    while(iter != m_SpriteList.end())
    {
        //CSprite* spr = *iter;
        CSprite *next = *iter;
        if(next->m_directImg->getUID() == img->getUID())
        {
            delete(*iter);
            iter = m_SpriteList.erase(iter);
        }
        else
            ++iter;
    }
}

void CSpriteManager::AddSprite(CSprite *spr)
{
    m_SpriteList.push_back(spr);
    if(spr->m_Invalidated)
        m_hasInvalid = true;
}

void CSpriteManager::GetComponents(int code, std::list<SpriteComponent *> *component_list)
{
    for(auto &iter : m_ComponentList)
    {
        // Lookup components that match the code
        if(iter.lookup_code == code)
            component_list->push_back(&iter);
    }
}
