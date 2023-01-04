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

#pragma once
#ifndef SpriteFuncs_HHHH
#define SpriteFuncs_HHHH

class CSprite;
struct SpriteComponent;

void Activate(int code, CSprite *spr);  // Formally add the components specified by "code" to sprite behaviors

namespace SpriteFunc
{

/////////////////////
/// Behavior funcs///
/////////////////////


// Collision funcs
void OnPlayerCollide(CSprite *me, SpriteComponent *obj);        // Activate given behavior when colliding with player
void OnPlayerDistance(CSprite *me, SpriteComponent *obj);
void PlayerCollectible(CSprite *me, SpriteComponent *obj);      // Calls Die() if detecting collision with player

// Sprite personal variable funcs
void SetSpriteVar(CSprite *me, SpriteComponent *obj);           // Set sprite custom variable
void IfSpriteVar(CSprite *me, SpriteComponent *obj);            // Activate component based on custom variable comparison
void IfLunaVar(CSprite *me, SpriteComponent *obj);              // Activate component based on lunadll user variable state

// Wait-on-condition funcs
void WaitForPlayer(CSprite *me, SpriteComponent *obj);          // Activate linked component on a player memory condition
void PlayerHoldingSprite(CSprite *me, SpriteComponent *obj);    // Activate linked component when player holding certain sprite

// Random funcs
void RandomComponent(CSprite *me, SpriteComponent *obj);        // Trigger one of 5 random component codes at a 25% chance each
void RandomComponentRange(CSprite *me, SpriteComponent *obj);   // Trigger a component code in the range of two numbers

// Self-action funcs
void Die(CSprite *me, SpriteComponent *obj);

// Speed funcs
void Deccelerate(CSprite *me, SpriteComponent *obj);        // Subtract sprite's speed towards minimum for each active frame
void Accelerate(CSprite *me, SpriteComponent *obj);         // Add to sprite's speed towards maximum for each active frame
void AccelToPlayer(CSprite *me, SpriteComponent *obj);      // Push towards player for each active frame
void ApplyVariableGravity(CSprite *me, SpriteComponent *obj); // Apply gravity specified in a lunadll variable

// Teleport funcs
void TeleportNearPlayer(CSprite *me, SpriteComponent *obj); // Direct set x/y position to somewhere in a circle around the player
void TeleportTo(CSprite *me, SpriteComponent *obj);         // Teleport to direct coordinates

// Move funcs
void PhaseMove(CSprite *me, SpriteComponent *obj);          // Move directly according to speed, ignoring collisions
void BumpMove(CSprite *me, SpriteComponent *obj);           // Move according to speed, bump into and reverse speed when colliding
void CrashMove(CSprite *me, SpriteComponent *obj);          // Calls die() when colliding with any block

// Set funcs
void SetXSpeed(CSprite *me, SpriteComponent *obj);
void SetYSpeed(CSprite *me, SpriteComponent *obj);
void SetAlwaysProcess(CSprite *me, SpriteComponent *obj);
void SetVisible(CSprite *me, SpriteComponent *obj);
void SetHitbox(CSprite *me, SpriteComponent *obj);

// Action funcs
void TriggerLunaEvent(CSprite *me, SpriteComponent *obj);   // Trigger a lunadll eventAT_BumpMove
void HarmPlayer(CSprite *me, SpriteComponent *obj);

// Generator funcs
void GenerateInRadius(CSprite *me, SpriteComponent *obj);   // Generate a sprite in a random radius around this sprite
void GenerateAtAngle(CSprite *me, SpriteComponent *obj);    // Generate a sprite with direction from CVAR_GEN_ANGLE

// Time funcs
void SpriteTimer(CSprite *me, SpriteComponent *obj);        // Counts down to 0 and then activates a component

// Animation funcs
void BasicAnimate(CSprite *me, SpriteComponent *obj);       // Simple animation func (moves drawing rect down the spritesheet)
void Blink(CSprite *me, SpriteComponent *obj);              // Cause sprite to blink by toggling visiblity flag on and off quickly
void AnimateFloat(CSprite *me, SpriteComponent *obj);       // Create "floaty" effect by oscillating graphics offset height

// Debug funcs
void SpriteDebug(CSprite *me, SpriteComponent *obj);        // Prints diagnostic information about the sprite this is attached to


//////////////////
/// Draw funcs ///
//////////////////

void StaticDraw(CSprite *me);       // Draw sprite to absolute position on the screen
void RelativeDraw(CSprite *me);     // Draw sprite inside level, relative to camera position
}

#endif // SpriteFuncs_HHHH
