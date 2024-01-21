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
    Screen_t& screen = Screens[vscreen.screen_ref];
    Player_t& p = Player[vscreen.player];

    // slowly disable small screen camera features as player cameras approach each other
    double rate = 1.0;
    if(screen.Type == ScreenTypes::Dynamic && (screen.W < 800 || screen.H < 600))
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

    if(g_config.small_screen_camera_features && screen.W < 800 && !NoTurnBack[p.Section])
    {
        int16_t max_offsetX = 360;
        if(max_offsetX > vscreen.Width - p.Location.Width * 4)
            max_offsetX = vscreen.Width - p.Location.Width * 4;

        int16_t lookX_target = max_offsetX * p.Location.SpeedX * 1.5 / Physics.PlayerRunSpeed;
        if(lookX_target > max_offsetX)
            lookX_target = max_offsetX;
        if(lookX_target < -max_offsetX)
            lookX_target = -max_offsetX;
        lookX_target &= ~1;

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

        if(GamePaused == PauseCode::None && !qScreen && !ForcedControls)
        {
            if(vscreen.small_screen_features.offset_x < lookX_target)
                vscreen.small_screen_features.offset_x += rateX;
            else if(vscreen.small_screen_features.offset_x > lookX_target)
                vscreen.small_screen_features.offset_x -= rateX;
        }

        vscreen.X -= rate * vscreen.small_screen_features.offset_x / 2;
    }

    if(g_config.small_screen_camera_features && screen.H < 600)
    {
        int16_t max_offsetY = 200;

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

        if(GamePaused == PauseCode::None && !qScreen && !ForcedControls)
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
                PlaySound(SFX_Camera);
            }
            else if(vscreen.small_screen_features.offset_y_hold == 0 && vscreen.small_screen_features.offset_y > max_offsetY - 40 && (vscreen.small_screen_features.last_buttons_held & 2) == 0 && p.Controls.Up)
            {
                vscreen.small_screen_features.offset_y_hold = max_offsetY;
                PlaySound(SFX_Camera);
            }
            else if(vscreen.small_screen_features.offset_y_hold != 0 && vscreen.small_screen_features.offset_y > -60 && vscreen.small_screen_features.offset_y < 60)
            {
                vscreen.small_screen_features.offset_y_hold = 0;
                PlaySound(SFX_Camera);
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

        vscreen.Y += rate * (lookY + 32);
    }
}

void DrawSmallScreenCam(vScreen_t& vscreen)
{
    const Screen_t& screen = Screens[vscreen.screen_ref];

    int CamX = vscreen.Width - 54;
    int CamY = vscreen.Height - 42;

    // don't overlap the controls display
    if(screen.Type == ScreenTypes::Dynamic && screen.vScreen(2).Visible && vscreen.Left > 0)
        CamY -= 24;

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
    else if(vscreen.small_screen_features.offset_y < -160 || vscreen.small_screen_features.offset_y > 160)
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
