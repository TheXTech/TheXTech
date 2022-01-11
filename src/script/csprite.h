/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef CSPRITE_HHHHH
#define CSPRITE_HHHHH

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "lunadefs.h"
#include "hitbox.h"
#include "sprite_component.h"

/// CSprite builtin custom variables
#define CVAR_GEN_ANGLE "_GenAngle"

class LunaImage;
class CSprite;
struct CSpriteRequest;
struct SpriteComponent;

typedef void (*pfnSprFunc)(CSprite*, SpriteComponent* obj);
typedef void (*pfnSprDraw)(CSprite*);

// General custom sprite class
class CSprite
{
public:

    /// Functions ///
    CSprite();
    void Init();

    void ClearExpiredComponents();

    void AddBirthComponent(SpriteComponent component);
    void AddBehaviorComponent(SpriteComponent component);
    void AddDrawComponent(pfnSprDraw component);
    void AddDeathComponent(SpriteComponent component);

    void SetImageResource(int _resource_code);
    void SetImage(const std::shared_ptr<LunaImage>& _img);
    void MakeLimitedLifetime(int new_lifetime);

    void Birth();
    void Process();
    void Draw();
    void Die();

    void SetCustomVar(std::string var_name, OPTYPE operation, double val);
    bool CustomVarExists(std::string var_name);
    double GetCustomVar(std::string var_name);

    /// Members///
    int m_ImgResCode;				// Image bank code of image resource the sprite uses
    std::shared_ptr<LunaImage> m_directImg;
    int m_CollisionCode;			// Collision code for collision blueprint bank (-1 == all blocks collide as solid)

    int m_FramesLeft;				// How many frames are left if dying automatically
    int m_DrawPriorityLevel;		// 0 = Low  1 = Mid  2 = High (drawn in front)
    int m_OffscreenCount;			// How many frames this sprite has been offscreen
    int m_FrameCounter;				// Incremented each Process()
    int m_GfxXOffset;
    int m_GfxYOffset;
    bool m_StaticScreenPos;			// Whether or not sprite uses absolute screen coords for drawing
    bool m_Visible;					// Whether or not this sprite should be drawn
    bool m_Birthed;					// Whether or not sprite's birth funcs have been run yet
    bool m_Died;					// Whether or not sprite's death funcs have been run yet
    bool m_Invalidated;				// Whether or not this sprite will be removed during the next cleanup phase
    bool m_LimitedFrameLife;		// Whether or not the sprite dies automatically after a number of frames
    bool m_AnimationSet;			// Whether or not the animation parameters have been set for this sprite yet
    bool m_AlwaysProcess;			// Whether or not this sprite should be processed regardless of player's current section

    double m_Xpos;
    double m_Ypos;
    double m_Ht;					// Of loaded image graphic for backwards compat with native SMBX sprites
    double m_Wd;					// Of loaded image graphic for backwards compat with native SMBX sprites
    double m_Xspd;
    double m_Yspd;

    Hitbox m_Hitbox;				// Hitbox relative to sprite position

    int m_AnimationPhase;			// Time per animation
    int m_AnimationTimer;			// Current timer for animation (animate when reaching 0)
    int m_AnimationFrame;			// The current frame in m_GfxRects to be drawn (0 == first frame)
    std::vector<LunaRect> m_GfxRects;	// Spritesheet areas to draw, indexed by animation frame

    std::list<SpriteComponent> m_BirthComponents;
    std::list<SpriteComponent> m_BehavComponents;	// Currently loaded behavioral components
    std::list<pfnSprDraw> m_DrawFuncs;
    std::list<SpriteComponent> m_DeathComponents;

    std::map<std::string, double> m_CustomVars;	// User-defined vars
};


// Obj for interfacing with sprite factory
struct CSpriteRequest
{
    CSpriteRequest() : type(0), x(0), y(0), time(0), img_resource_code(0), x_speed(0), y_speed(0), spawned(false) {}
    int type;
    int x;
    int y;
    int time;
    int img_resource_code;
    std::shared_ptr<LunaImage> direct_img;
    std::string str;

    // Optional parameters
    double x_speed;
    double y_speed;
    bool spawned;			// "spawned" means the sprite will be deleted after being offscreen for some time
};

#endif // CSPRITE_HHHHH
