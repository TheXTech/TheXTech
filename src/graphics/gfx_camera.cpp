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

#include "core/render.h"

#include "globals.h"
#include "game_main.h"
#include "graphics.h"
#include "gfx.h"
#include "screen.h"
#include "config.h"
#include "sound.h"

void ResetCameraPanning()
{
    for(int i = 0; i < 2; ++i)
        vScreen[i].small_screen_features = vScreen_t::SmallScreenFeatures_t();
}

void ProcessSmallScreenCam(vScreen_t& vscreen)
{
    const Screen_t& screen = Screens[vscreen.screen_ref];
    const Screen_t& c_screen = screen.canonical_screen();
    const Player_t& p = Player[vscreen.player];

    // slowly disable small screen camera features as player cameras approach each other
    double rate = 1.0;
    if(screen.Type == ScreenTypes::Dynamic && (screen.W < c_screen.W || screen.H < c_screen.H))
    {
        double dx = std::abs(screen.vScreen(1).X - screen.vScreen(2).X);
        double dy = std::abs(screen.vScreen(1).Y - screen.vScreen(2).Y);
        double d = dx + dy;
        const double screen_join = (screen.W + screen.H) / 2;

        rate = (d - screen_join) / screen_join;
        if(rate < 0.0)
            rate = 0.0;
        if(rate > 1.0)
            rate = 1.0;
    }

    if(g_config.small_screen_cam && screen.W < c_screen.W && !NoTurnBack[p.Section])
    {
        int16_t max_offsetX = 360;
        if(max_offsetX > vscreen.Width - p.Location.Width * 4)
            max_offsetX = vscreen.Width - p.Location.Width * 4;

        if(max_offsetX > c_screen.W - screen.W)
            max_offsetX = c_screen.W - screen.W;

        int16_t lookX_target = max_offsetX * p.Location.SpeedX * 1.5 / Physics.PlayerRunSpeed;
        if(lookX_target > max_offsetX)
            lookX_target = max_offsetX;
        if(lookX_target < -max_offsetX)
            lookX_target = -max_offsetX;
        lookX_target &= ~1;

        if(LevelMacro != LEVELMACRO_OFF || ForcedControls)
            lookX_target = 0;

        int16_t rateX = 1;
        // switching directions
        if((vscreen.small_screen_features.offset_x < 0 && lookX_target > 0)
            || (vscreen.small_screen_features.offset_x > 0 && lookX_target < 0))
        {
            rateX = 3;
        }
        // accelerating
        else if((vscreen.small_screen_features.offset_x > 0) == (lookX_target > vscreen.small_screen_features.offset_x))
        {
            rateX = 2;
        }

        if(GamePaused == PauseCode::None && !qScreen && p.Effect == PLREFF_NORMAL)
        {
            if(vscreen.small_screen_features.offset_x < lookX_target)
                vscreen.small_screen_features.offset_x += rateX;
            else if(vscreen.small_screen_features.offset_x > lookX_target)
                vscreen.small_screen_features.offset_x -= rateX;
        }

        vscreen.X -= rate * vscreen.small_screen_features.offset_x / 2;
    }

    if(g_config.small_screen_cam && screen.H < c_screen.H)
    {
        int16_t max_offsetY = 200;
        if(max_offsetY > (c_screen.H - screen.H) + 50)
            max_offsetY = (c_screen.H - screen.H) + 50;

        int16_t lookY_target = max_offsetY;

        bool on_ground = p.Pinched.Bottom1 || p.Slope || p.StandingOnNPC || p.Wet || p.Quicksand;
        // bool duck_jump = !on_ground && p.Duck;
        bool prevent_unlock = vscreen.small_screen_features.offset_y_hold != 0 && (p.Vine || !on_ground || p.GrabTime);

        if(p.Controls.Up == p.Controls.Down || prevent_unlock)
            lookY_target = vscreen.small_screen_features.offset_y_hold;
        else if(p.Controls.Down)
            lookY_target *= -1;

        int16_t rateY = 4;
        if((vscreen.small_screen_features.offset_y < 0 && lookY_target > 0)
            || (vscreen.small_screen_features.offset_y > 0 && lookY_target < 0))
        {
            if(vscreen.small_screen_features.offset_y < 50 && vscreen.small_screen_features.offset_y > -50)
                vscreen.small_screen_features.offset_y *= -1;
        }

        if(GamePaused == PauseCode::None && !qScreen && !ForcedControls && LevelMacro == LEVELMACRO_OFF && p.Effect == PLREFF_NORMAL)
        {
            if(vscreen.small_screen_features.offset_y < lookY_target)
            {
                vscreen.small_screen_features.offset_y += rateY;

                if(vscreen.small_screen_features.offset_y > lookY_target)
                    vscreen.small_screen_features.offset_y = lookY_target;
            }
            else if(vscreen.small_screen_features.offset_y > lookY_target)
            {
                vscreen.small_screen_features.offset_y -= rateY;

                if(vscreen.small_screen_features.offset_y < lookY_target)
                    vscreen.small_screen_features.offset_y = lookY_target;
            }

            if(vscreen.small_screen_features.offset_y_hold == 0 && vscreen.small_screen_features.offset_y < -max_offsetY + 40 && (vscreen.small_screen_features.last_buttons_held & 1) == 0 && p.Controls.Down)
            {
                vscreen.small_screen_features.offset_y_hold = -max_offsetY;
                PlaySoundSpatial(SFX_Camera, p.Location);
            }
            else if(vscreen.small_screen_features.offset_y_hold == 0 && vscreen.small_screen_features.offset_y > max_offsetY - 40 && (vscreen.small_screen_features.last_buttons_held & 2) == 0 && p.Controls.Up)
            {
                vscreen.small_screen_features.offset_y_hold = max_offsetY;
                PlaySoundSpatial(SFX_Camera, p.Location);
            }
            else if(vscreen.small_screen_features.offset_y_hold != 0 && vscreen.small_screen_features.offset_y > -60 && vscreen.small_screen_features.offset_y < 60)
            {
                vscreen.small_screen_features.offset_y_hold = 0;
                PlaySoundSpatial(SFX_Camera, p.Location);
            }

            vscreen.small_screen_features.last_buttons_held = (int8_t)p.Controls.Down | (int8_t)p.Controls.Up << 1;
        }

        int16_t lookY = vscreen.small_screen_features.offset_y;

        if(lookY > -50 && lookY < 50)
            lookY = 0;
        else
        {
            if(lookY > 0)
                lookY -= 50;
            if(lookY < 0)
                lookY += 50;
            lookY /= 2;
        }

        // lower the default offset by 32px
        int fix_default_offset = 32;

        // weaken this effect when very close to the original canonical resolution
        if(fix_default_offset > (c_screen.H - screen.H) / 2)
            fix_default_offset = (c_screen.H - screen.H) / 2;

        // fade this out if approaching the canonical screen ounbds
        fix_default_offset *= (c_screen.H - screen.H) - std::abs(lookY) * 2;
        fix_default_offset /= (c_screen.H - screen.H);

        vscreen.Y += rate * (lookY + fix_default_offset);
    }
}

void DrawSmallScreenCam(vScreen_t& vscreen)
{
    const Screen_t& screen = Screens[vscreen.screen_ref];
    const Screen_t& c_screen = screen.canonical_screen();

    int CamX = vscreen.Width - 54;
    int CamY = vscreen.Height - 42;

    // don't overlap the controls display
    if(screen.Type == ScreenTypes::Dynamic && screen.vScreen(2).Visible && vscreen.Left > 0)
        CamY -= 24;

    // stay in the safe zone
    if(vscreen.ScreenLeft + vscreen.Width >= XRender::TargetW)
        CamX -= XRender::TargetOverscanX;

    // scale the opacity by the current effectiveness of the camera features
    double rate = 1.0;
    if(screen.Type == ScreenTypes::Dynamic)
    {
        double dx = std::abs(screen.vScreen(1).X - screen.vScreen(2).X);
        double dy = std::abs(screen.vScreen(1).Y - screen.vScreen(2).Y);
        double d = dx + dy;
        const double screen_join = (screen.W + screen.H) / 2;

        rate = (d - screen_join) / screen_join;
        if(rate < 0.0)
            rate = 0.0;
        if(rate > 1.0)
            rate = 1.0;
    }

    int16_t max_offsetY = 200;
    if(max_offsetY > (c_screen.H - screen.H) + 50)
        max_offsetY = (c_screen.H - screen.H) + 50;

    // color for camera
    XTColor color;

    if(vscreen.small_screen_features.offset_y_hold != 0)
    {
        color = XTColorF(1.0f, 0.2f, 0.2f, 1.0f);

        if(vscreen.small_screen_features.offset_y_hold > 0)
            XRender::renderTexture(CamX + 4, CamY - 18, GFX.MCursor[1], XTAlphaF(rate));
        else
            XRender::renderTexture(CamX + 4, CamY + 18, GFX.MCursor[2], XTAlphaF(rate));
    }
    else if(vscreen.small_screen_features.offset_y < -max_offsetY * 4 / 5 || vscreen.small_screen_features.offset_y > max_offsetY * 4 / 5)
    {
        color = XTColorF(0.5f, 1.0f, 0.5f, 0.7f);

        if(vscreen.small_screen_features.offset_y > 0)
            XRender::renderTexture(CamX + 4, CamY - 18, GFX.MCursor[1], XTAlphaF(0.7f * rate));
        else
            XRender::renderTexture(CamX + 4, CamY + 18, GFX.MCursor[2], XTAlphaF(0.7f * rate));
    }
    else if(vscreen.small_screen_features.offset_y <= -48 || vscreen.small_screen_features.offset_y >= 48)
    {
        color = XTAlphaF(0.5f);
    }
    else
    {
        color = XTColorF(0.0f, 0.0f, 0.0f, 0.3f);
    }

    color.a *= rate;

    if(GFX.Camera.inited)
        XRender::renderTexture(CamX, CamY, GFX.Camera, color);
    else
        XRender::renderRect(CamX, CamY, 24, 16, color);
}
