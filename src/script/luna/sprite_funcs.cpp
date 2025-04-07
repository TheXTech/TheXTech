/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sprite_funcs.h"
#include "sprite_component.h"
#include "autocode_manager.h"
#include "csprite.h"
#include "globals.h"
#include "player.h"
#include "rand.h"
#include "lunarender.h"
#include "lunaplayer.h"
#include "main/trees.h"
#include "lunamisc.h"
#include "lunaspriteman.h"
#include "renderop_bitmap.h"
#include "mememu.h"

static inline int s_round2int(num_t d)
{
    return std::floor(d + 0.5);
}


// Activate -- Formally trigger a component link in the given sprite (copy all matches into behavior list)
void Activate(int code, CSprite *spr)
{
    if(code > 99)
    {
        std::list<SpriteComponent *> complist;
        gSpriteMan.GetComponents(code, &complist);
        while(!complist.empty())
        {
            spr->AddBehaviorComponent(*complist.front());
            complist.pop_front();
        }
    }
}

// PLAYER COLLECTIBLE -- Call sprite's "Die()" when touched by player
void SpriteFunc::PlayerCollectible(CSprite *me, SpriteComponent *comp)
{
    UNUSED(comp);
    Player_t *demo = PlayerF::Get(1);
    if(demo)
    {
        if(me->m_Hitbox.Test((int)demo->Location.X, (int)demo->Location.Y,
                             (int)demo->Location.Width, (int)demo->Location.Height))
        {
            me->Die();
            gSpriteMan.m_hasInvalid = true;
        }
    }
}

// WAIT FOR PLAYER -- Activate the linked component when a player condition becomes true
void SpriteFunc::WaitForPlayer(CSprite *me, SpriteComponent *obj)
{
    UNUSED(me);
    Player_t *demo = PlayerF::Get(1);
    if(demo)
    {
        auto ftype = (FIELDTYPE)SDL_atoi(obj->data5.c_str());
//        uint8_t *ptr = (uint8_t *)demo;
//        ptr += (int)obj->data1; // offset
        bool triggered = CheckMem(demo, (int)obj->data1, std::floor(obj->data2), (COMPARETYPE)(int)obj->data3, ftype);
        if(triggered)
        {
            //TODO: FINISH IT
            // me->AddBehaviorComponent(
        }
    }
}

// PLAYER HOLDING SPRITE
void SpriteFunc::PlayerHoldingSprite(CSprite *me, SpriteComponent *obj)
{
    Player_t *demo = PlayerF::Get(1);
    if(demo)
    {
        if(PlayerF::IsHoldingSpriteType(demo, (int)obj->data1))
            Activate((int)obj->data4, me);
    }
}

// SET SPRITE VAR
void SpriteFunc::SetSpriteVar(CSprite *me, SpriteComponent *obj)
{
    if(obj->data5.length() > 0)
        me->SetCustomVar(obj->data5, (OPTYPE)(int)obj->data2, obj->data3);
}

// IF SPRITE VAR
void SpriteFunc::IfSpriteVar(CSprite *me, SpriteComponent *obj)
{
    if(obj->data5.length() > 0)
    {
        if(me->CustomVarExists(obj->data5))
        {
            num_t var_val = me->GetCustomVar(obj->data5);
            num_t check_against = obj->data3;
            num_t component_to_activate = obj->data4;

            switch((COMPARETYPE)(int)obj->data2)
            {
            case CMPT_EQUALS:
                if(var_val == check_against)
                    Activate((int)component_to_activate, me);
                break;
            case CMPT_GREATER:
                if(var_val > check_against)
                    Activate((int)component_to_activate, me);
                break;
            case CMPT_LESS:
                if(var_val < check_against)
                    Activate((int)component_to_activate, me);
                break;
            case CMPT_NOTEQ:
                if(var_val != check_against)
                    Activate((int)component_to_activate, me);
                break;
            default:
                break;
            }
        }
    }
}

// IF LUNA VAR
void SpriteFunc::IfLunaVar(CSprite *me, SpriteComponent *obj)
{
    if(obj->data5.length() > 0)
    {
        if(gAutoMan.VarExists(obj->data5))
        {
            num_t var_val = gAutoMan.GetVar(obj->data5);
            num_t check_against = obj->data3;
            num_t component_to_activate = obj->data4;

            switch((COMPARETYPE)(int)obj->data2)
            {
            case CMPT_EQUALS:
                if(var_val == check_against)
                    Activate((int)component_to_activate, me);
                break;
            case CMPT_GREATER:
                if(var_val > check_against)
                    Activate((int)component_to_activate, me);
                break;
            case CMPT_LESS:
                if(var_val < check_against)
                    Activate((int)component_to_activate, me);
                break;
            case CMPT_NOTEQ:
                if(var_val != check_against)
                    Activate((int)component_to_activate, me);
                break;
            default:
                break;
            }
        }
    }
}

// RANDOM COMPONENT
void SpriteFunc::RandomComponent(CSprite *me, SpriteComponent *obj)
{
    int choice = iRand2(4);

    switch(choice)
    {
    default:
        break;
    case 0:
        Activate((int)obj->data1, me);
        break;
    case 1:
        Activate((int)obj->data2, me);
        break;
    case 2:
        Activate((int)obj->data3, me);
        break;
    case 3:
        Activate((int)obj->data4, me);
        break;
    }
}

// RANDOM COMPONENT RANGE
void SpriteFunc::RandomComponentRange(CSprite *me, SpriteComponent *obj)
{
    int val1 = (int)obj->data1;
    int val2 = (int)obj->data2;
    if(val1 < val2)   // rule out bad values
    {
        int diff = val2 - val1;
        int choice = iRand2(diff);
        Activate(val1 + choice, me);
    }
}

// DIE
void SpriteFunc::Die(CSprite *me, SpriteComponent *obj)
{
    UNUSED(obj);
    me->Die();
    gSpriteMan.m_hasInvalid = true;
}

// DECCELERATE
void SpriteFunc::Deccelerate(CSprite *me, SpriteComponent *obj)
{
    num_t XRate = obj->data1;
    num_t YRate = obj->data2;
    num_t Min  = obj->data3;
    num_t accum = 0;
    if(me->m_Xspd >= 0 && XRate != 0)
    {
        accum = me->m_Xspd - XRate;
        if(accum < Min)
            accum = Min;
        me->m_Xspd = accum;
    }
    else if(XRate != 0)
    {
        accum = me->m_Xspd + XRate;
        if(accum > Min)
            accum = Min;
        me->m_Xspd = accum;
    }

    if(me->m_Yspd >= 0 && YRate != 0)
    {
        accum = me->m_Yspd - YRate;
        if(accum < Min)
            accum = Min;
        me->m_Yspd = accum;
    }
    else if(YRate != 0)
    {
        accum = me->m_Yspd + YRate;
        if(accum > Min)
            accum = Min;
        me->m_Yspd = accum;
    }
}

// ACCELERATE
void SpriteFunc::Accelerate(CSprite *me, SpriteComponent *obj)
{
    UNUSED(me);
    UNUSED(obj);
    //TODO
}

// ACCEL TO PLAYER
void SpriteFunc::AccelToPlayer(CSprite *me, SpriteComponent *obj)
{
    Player_t *demo = PlayerF::Get(1);
    if(demo)
    {
        num_t negmax = obj->data3 * -1;
        if(demo->Location.X < me->m_Xpos)
            me->m_Xspd -= obj->data1;
        else
            me->m_Xspd += obj->data1;
        if(demo->Location.Y < me->m_Ypos)
            me->m_Yspd -= obj->data2;
        else
            me->m_Yspd += obj->data2;
        if(obj->data3 != 0)
        {
            if(me->m_Xspd > obj->data3)
                me->m_Xspd = obj->data3;
            else if(me->m_Xspd < negmax)
                me->m_Xspd = negmax;
            if(me->m_Yspd > obj->data3)
                me->m_Yspd = obj->data3;
            else if(me->m_Yspd < negmax)
                me->m_Yspd = negmax;
        }
    }
}

// APPLY VARIABLE GRAVITY
void SpriteFunc::ApplyVariableGravity(CSprite *me, SpriteComponent *obj)
{
    num_t var = gAutoMan.GetVar(obj->data5);
    if(obj->data2 == 0)  // x
        me->m_Xspd += var;
    else   // y
        me->m_Yspd += var;
}

// ON PLAYER COLLIDE
void SpriteFunc::OnPlayerCollide(CSprite *me, SpriteComponent *obj)
{
    Player_t *demo = PlayerF::Get(1);

    if(demo)
    {
        if(obj->data2 == 0)   // player normal hitbox
        {
            if(me->m_Hitbox.Test((int)demo->Location.X, (int)demo->Location.Y, (int)demo->Location.Width, (int)demo->Location.Height))
                Activate((int)obj->data4, me);
        }
        else   // special small circle hitbox
        {
            num_t extent = obj->data2 / 2;

            num_t cx = demo->Location.X + (demo->Location.Width / 2);
            num_t cy = demo->Location.Y + (demo->Location.Height / 2);
            if(me->m_Hitbox.Test((int)cx, (int)cy, (int)extent))
                Activate((int)obj->data4, me);
        }
    }
}

// ON PLAYER DISTANCE
void SpriteFunc::OnPlayerDistance(CSprite *me, SpriteComponent *obj)
{
    Player_t *demo = PlayerF::Get(1);
    if(demo)
    {
        double xdist = std::abs(demo->Location.X - me->m_Xpos);
        double ydist = std::abs(demo->Location.Y - me->m_Ypos);

        // Checking farness or nearness?
        if(obj->data2 == 0)
        {
            if(xdist + ydist >= obj->data1)
                Activate((int)obj->data4, me);
        }
        else
        {
            if(xdist + ydist <= obj->data1)
                Activate((int)obj->data4, me);
        }
    }
}

// PHASE MOVE
void SpriteFunc::PhaseMove(CSprite *me, SpriteComponent *obj)
{
    UNUSED(obj);
    me->m_Xpos += me->m_Xspd;
    me->m_Ypos += me->m_Yspd;
}

// BUMP MOVE
void SpriteFunc::BumpMove(CSprite *me, SpriteComponent *obj)
{
    num_t energy_loss_mod = (100 - obj->data2) / 100;

    me->m_Xpos += me->m_Xspd;
    me->m_Ypos += me->m_Yspd;

    // uint32_t COLOR = 0x11111111; // ACTUALLY UNUSED

    bool collided_left = false;
    bool collided_right = false;
    bool collided_top = false;
//    bool collided_bot = false;

    // NOTE: these were previously sorted closest first. If we keep this function, we should replicate that logic here.

    for(BlockRef_t block : treeBlockQuery(newLoc(me->m_Hitbox.CalcLeft(), me->m_Hitbox.CalcRight(), me->m_Hitbox.W, me->m_Hitbox.H), SORTMODE_COMPAT))
    {
        if(!block->Invis && !block->Hidden
           && me->m_Hitbox.Test((int)block->Location.X, (int)block->Location.Y, (int)block->Location.Width, (int)block->Location.Height))
        {
            num_t sprite_bot = me->m_Hitbox.CalcBottom();
            num_t sprite_right = me->m_Hitbox.CalcRight();
            num_t sprite_top = me->m_Hitbox.CalcTop();
            num_t sprite_left = me->m_Hitbox.CalcLeft();

            if(me->m_CollisionCode == -1)   // default solid collision
            {
                num_t block_topcol = std::abs(block->Location.Y - sprite_bot);
                num_t block_botcol = std::abs((block->Location.Y + block->Location.Height) - sprite_top);
                num_t block_leftcol = std::abs(block->Location.X - sprite_right);
                num_t block_rightcol = std::abs((block->Location.X + block->Location.Width) - sprite_left);

                // Determine best direction to free sprite
                // Top collision, push sprite up and out
                if(block_topcol <= block_botcol && block_topcol <= block_leftcol &&
                   block_topcol <= block_rightcol && !collided_top)
                {
                    me->m_Ypos = (block->Location.Y - me->m_Hitbox.H) - 1;
                    me->m_Yspd = -(me->m_Yspd * energy_loss_mod);
                    collided_top = true;
                }

                // Bot collision, push sprite down
                else if(block_botcol <= block_leftcol && block_botcol <= block_rightcol && !collided_right)
                {
                    me->m_Ypos = ((block->Location.Y + block->Location.Height) - me->m_Hitbox.Top_off) + 1;
                    me->m_Yspd = -(me->m_Yspd * energy_loss_mod);
                    // collided_bot = true;
                }

                // Left collision, push sprite left
                else if(block_leftcol <= block_rightcol && !collided_left)
                {
                    me->m_Xpos = (block->Location.X - me->m_Hitbox.W) - 1;
                    me->m_Xspd = -(me->m_Xspd  * energy_loss_mod);
                    collided_left = true;
                }

                // Right collision, push sprite right
                else if(!collided_right)
                {
                    me->m_Xpos = ((block->Location.X + block->Location.Width) - me->m_Hitbox.Left_off) + 1;
                    me->m_Xspd = -(me->m_Xspd  * energy_loss_mod);
                    collided_right = true;
                }
            }
        }
    }
}

// CRASH MOVE
void SpriteFunc::CrashMove(CSprite *me, SpriteComponent *obj)
{
    UNUSED(obj);
    me->m_Xpos += me->m_Xspd;
    me->m_Ypos += me->m_Yspd;

    for(BlockRef_t block : treeBlockQuery(newLoc(me->m_Hitbox.CalcLeft(), me->m_Hitbox.CalcRight(), me->m_Hitbox.W, me->m_Hitbox.H), SORTMODE_NONE))
    {
        if(!block->Invis && !block->Hidden &&
            me->m_Hitbox.Test((int)block->Location.X,
                              (int)block->Location.Y,
                              (int)block->Location.Width,
                              (int)block->Location.Height))
        {
            me->Die();
            gSpriteMan.m_hasInvalid = true;
            break;
        }
    }
}

// SET X SPEED
void SpriteFunc::SetXSpeed(CSprite *me, SpriteComponent *obj)
{
    me->m_Xspd = obj->data1;
}

// SET Y SPEED
void SpriteFunc::SetYSpeed(CSprite *me, SpriteComponent *obj)
{
    me->m_Yspd = obj->data1;
}

// SET ALWAYS PROCESS
void SpriteFunc::SetAlwaysProcess(CSprite *me, SpriteComponent *obj)
{
    me->m_AlwaysProcess = (bool)obj->data1;
}

// SET VISIBLE
void SpriteFunc::SetVisible(CSprite *me, SpriteComponent *obj)
{
    me->m_Visible = (bool)obj->data1;
}

// SET HITBOX
void SpriteFunc::SetHitbox(CSprite *me, SpriteComponent *obj)
{
    if(obj->data5.find("circle") != std::wstring::npos)
        me->m_Hitbox.CollisionType = 1;
    else
        me->m_Hitbox.CollisionType = 0;
    me->m_Hitbox.Left_off = (short)obj->data1;
    me->m_Hitbox.Top_off = (short)obj->data2;
    me->m_Hitbox.W = (short)obj->data3;
    me->m_Hitbox.H = (short)obj->data4;
}

// TELEPORT NEAR PLAYER
void SpriteFunc::TeleportNearPlayer(CSprite *me, SpriteComponent *obj)
{
    Player_t *demo = PlayerF::Get(1);
    if(demo)
    {
        num_t cx = demo->Location.X;
        num_t cy = demo->Location.Y;
        num_t phase = iRand2(360);
        double xoff = std::sin(phase) * obj->data1;
        double yoff = std::cos(phase) * obj->data1;
        me->m_Xpos = cx + xoff;
        me->m_Ypos = cy + yoff;
    }
}

// TELEPORT TO
void SpriteFunc::TeleportTo(CSprite *me, SpriteComponent *obj)
{
    me->m_Xpos = obj->data1;
    me->m_Ypos = obj->data2;
}

// TRIGGER LUNA EVENT -- Trigger a lunadll script event (such as #1000)
void SpriteFunc::TriggerLunaEvent(CSprite *me, SpriteComponent *obj)
{
    UNUSED(me);
    if(obj->data1 > 21)
        gAutoMan.ActivateCustomEvents(0, (int)obj->data1);
}

// HARM PLAYER
void SpriteFunc::HarmPlayer(CSprite *me, SpriteComponent *obj)
{
    UNUSED(me);
    UNUSED(obj);
    PlayerHurt(1);
}

// GENERATE IN RADIUS
void SpriteFunc::GenerateInRadius(CSprite *me, SpriteComponent *obj)
{
    num_t rand_x;
    num_t rand_y;
    RandomPointInRadius(&rand_x, &rand_y, me->m_Hitbox.CenterX(), me->m_Hitbox.CenterY(), (int)obj->data3);

    CSpriteRequest req;
    req.type = 0;
    req.img_resource_code = (int)obj->data2;
    req.x = (int)rand_x;
    req.y = (int)rand_y;
    req.time = (int)obj->data4;
    req.str = obj->data5;
    req.spawned = true;

    gSpriteMan.InstantiateSprite(&req, false);
}

// GENERATE AT ANGLE
void SpriteFunc::GenerateAtAngle(CSprite *me, SpriteComponent *obj)
{
    num_t angle = me->GetCustomVar(CVAR_GEN_ANGLE);
    num_t speed = obj->data3;

    double vx = std::cos(angle) * speed;                 // vector x speed
    double vy = std::sin(angle) * speed;                 // vector y speed
    num_t gx = me->m_Hitbox.CenterX() + (vx * 2);  // generation point
    num_t gy = me->m_Hitbox.CenterY() + (vy * 2);  // generation point

    CSpriteRequest req;
    req.type = 0;
    req.img_resource_code = (int)obj->data2;
    req.x = (int)gx;
    req.y = (int)gy;
    req.time = (int)obj->data4;
    req.str = obj->data5;

    req.x_speed = vx;
    req.y_speed = vy;
    req.spawned = true;

    gSpriteMan.InstantiateSprite(&req, false);
}

// SPRITE TIMER
void SpriteFunc::SpriteTimer(CSprite *me, SpriteComponent *obj)
{
    bool repeat = (bool)obj->data3;
    int timer = (int)obj->run_time;
    if(timer == 1 || timer == 0)
    {
        if(timer == 1)
            obj->expired = true;
        Activate((int)obj->data4, me);

        if(repeat)
        {
            obj->expired = false;
            obj->run_time = obj->org_time;
        }
    }
}

// BASIC ANIMATE
void SpriteFunc::BasicAnimate(CSprite *me, SpriteComponent *obj)
{
    int anim_height = (int)obj->data1;
    if(anim_height == 0)
        anim_height = 1;
    int implicit_frames = (int)me->m_Ht / anim_height;

    // Init animation state if necessary
    if(!me->m_AnimationSet)
    {
        me->m_Hitbox.H = static_cast<short>(anim_height);
        me->m_AnimationFrame = 0;
        me->m_AnimationPhase = (int)obj->data2;
        me->m_AnimationTimer = (int)obj->data2;
        me->m_GfxRects.clear();
        for(int i = 0; i < implicit_frames; i++)
        {
            LunaRect temp;
            temp.left = 0;
            temp.right = (int)me->m_Wd;
            temp.top = (int)anim_height * i;
            temp.bottom = (int)anim_height; // <this is a HEIGHT argument
            me->m_GfxRects.push_back(temp);
        }
        me->m_AnimationSet = true;
    }

    // Process animation
    me->m_AnimationTimer--;
    if(me->m_AnimationTimer <= 0)
    {
        me->m_AnimationTimer = me->m_AnimationPhase;
        me->m_AnimationFrame += 1;
        if(me->m_AnimationFrame + 1 > implicit_frames)
            me->m_AnimationFrame = 0;
    }
}

// ANIMATE FLOAT
void SpriteFunc::AnimateFloat(CSprite *me, SpriteComponent *obj)
{
    num_t speed = obj->data1;
    num_t x_mag = obj->data2;
    num_t y_mag = obj->data3;
    if(speed != 0 && (x_mag != 0 || y_mag != 0))
    {
        double frame_val = me->m_FrameCounter / speed;
        if(x_mag != 0)
            me->m_GfxXOffset = (int)(std::cos(frame_val) * x_mag);
        if(y_mag != 0)
            me->m_GfxYOffset = (int)(std::sin(frame_val) * y_mag);
    }
}

// BLINK
void SpriteFunc::Blink(CSprite *me, SpriteComponent *obj)
{
    int mod = (int)obj->run_time % (int)obj->data1;
    if(mod == 0)
        me->m_Visible = (bool)obj->data2;
    else
        me->m_Visible = (bool)(obj->data2 == 0 ? 1 : 0);
}

// SPRITE DEBUG
void SpriteFunc::SpriteDebug(CSprite *me, SpriteComponent *obj)
{
    UNUSED(obj);
    Renderer::Get().DebugPrint("XPOS - ", me->m_Xpos);
    Renderer::Get().DebugPrint("YPOS - ", me->m_Ypos);
    Renderer::Get().DebugPrint("XSPD - ", me->m_Xspd);
    Renderer::Get().DebugPrint("YSPD - ", me->m_Yspd);
    Renderer::Get().DebugPrint("FRAME - ", me->m_AnimationFrame);
    Renderer::Get().DebugPrint("VISIBLE - ", (me->m_Visible ? 1 : 0));
    Renderer::Get().DebugPrint("CVARS - ", (uint32_t)me->m_CustomVars.size());
    Renderer::Get().DebugPrint("BEHAVIORS - ", (uint32_t)me->m_BehavComponents.size());
}





// STATIC DRAW - Simply draw the sprite at its absolute screen coordinates
//               by registering a new bitmap render operation
void SpriteFunc::StaticDraw(CSprite *me)
{
    if(me != nullptr && me->m_Visible)
    {
        if(!me->m_directImg && me->m_GfxRects.empty()) // Workaround
        {
            auto *direct_img = Renderer::Get().GetImageForResourceCode(me->m_ImgResCode);
            if(direct_img)
                gSpriteMan.InitializeDimensions(me, false);
        }

        if(me->m_AnimationFrame < (signed)me->m_GfxRects.size())   // Frame should be less than size of GfxRect container
        {
            auto *op = new RenderBitmapOp();
            op->m_FramesLeft = 1;
            op->x = s_round2int(me->m_Xpos) + me->m_GfxXOffset;
            op->y = s_round2int(me->m_Ypos) + me->m_GfxYOffset;
            auto &r = me->m_GfxRects[me->m_AnimationFrame];
            op->sx = r.left;
            op->sy = r.top;
            op->sw = r.right;
            op->sh = r.bottom;
            if(me->m_directImg)
                op->direct_img = me->m_directImg;
            else
                op->direct_img = Renderer::Get().GetImageForResourceCode(me->m_ImgResCode);

            Renderer::Get().AddOp(op);
        }
    }
}

// RELATIVE DRAW - Calculate sprite position inside level and draw relative
//                 to camera position by registering new bitmap render operation
void SpriteFunc::RelativeDraw(CSprite *me)
{
    if(me != nullptr && me->m_Visible)
    {
        if(!me->m_directImg && me->m_GfxRects.empty()) // Workaround
        {
            auto *direct_img = Renderer::Get().GetImageForResourceCode(me->m_ImgResCode);
            if(direct_img)
                gSpriteMan.InitializeDimensions(me, false);
        }

        if(me->m_AnimationFrame < (signed)me->m_GfxRects.size())
        {
            // double cx = 0;              // camera x (top left of screen)
            // double cy = 0;              // camera y (top left of screen)
            int sx = s_round2int(me->m_Xpos);     // sprite x position (top left of sprite)
            int sy = s_round2int(me->m_Ypos);     // sprite y position (top left of sprite)
            sx +=  me->m_GfxXOffset;
            sy +=  me->m_GfxYOffset;

            // Calc screen draw position based on camera position
            // Render::CalcCameraPos(&cx, &cy);
            // sx = sx - cx;
            // sy = sy - cy;

            // Register drawing operation
            auto *op = new RenderBitmapOp();
            op->m_FramesLeft = 1;
            op->x = sx;
            op->y = sy;

            auto &r = me->m_GfxRects[me->m_AnimationFrame];
            op->sx = r.left;
            op->sy = r.top;
            op->sw = r.right;
            op->sh = r.bottom;

            if(me->m_directImg)
                op->direct_img = me->m_directImg;
            else
                op->direct_img = Renderer::Get().GetImageForResourceCode(me->m_ImgResCode);
            op->sceneCoords = true;
            op->m_renderPriority = RENDEROP_DEFAULT_PRIORITY_RENDEROP_SCENE;

            Renderer::Get().AddOp(op);
            return;
        }
    }
}
