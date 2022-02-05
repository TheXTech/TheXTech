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

#include <SDL2/SDL_timer.h>

#include "../globals.h"
#include "../frame_timer.h"
#include "../graphics.h"
#include "../collision.h"
#include "../editor.h"
#include "../npc.h"
#include "../gfx.h"
#include "../layers.h"
#include "../main/menu_main.h"
#include "../main/speedrunner.h"
#include "../main/trees.h"
#include "../main/screen_pause.h"
#include "../main/screen_connect.h"
#include "../main/screen_quickreconnect.h"
#include "../main/screen_textentry.h"
#include "../compat.h"
#include "../game_main.h"
#include "../main/game_globals.h"
#include "../core/render.h"
#include "../script/luna/luna.h"

#include <fmt_format_ne.h>
#include <Utils/maths.h>

struct ScreenShake_t
{
    double forceX = 0;
    double forceY = 0;
    double forceDecay = 1.0;
    int    type = SHAKE_RANDOM;
    double duration = 0;
    double sign = +1.0;

    bool   active = false;

    void update()
    {
        if(!active || GameMenu)
            return;

        int offsetX, offsetY;

        if(duration <= 0)
        {
            if(forceX > 0)
                forceX -= forceDecay;
            if(forceY > 0)
                forceY -= forceDecay;
        }
        else
            duration--;

        if(forceX <= 0 && forceY <= 0)
        {
            forceX = 0.0;
            forceY = 0.0;
            active = false;
        }
        // always perform this section to keep the number of random calls consistent w/legacy sources
        {
            switch(type)
            {
            default:
            case SHAKE_RANDOM:
                offsetX = iRand(forceX * 4) - forceX * 2;
                offsetY = iRand(forceY * 4) - forceY * 2;
                break;
            case SHAKE_SEQUENTIAL:
                offsetX = forceX > 0 ? (int)round(sign * forceX) : 0;
                offsetY = forceY > 0 ? (int)round(sign * forceY) : 0;
                sign *= -1;
                break;
            }

            XRender::offsetViewport(offsetX, offsetY);
        }
    }

    void setup(int i_forceX, int i_forceY, int i_type, int i_duration, double i_decay)
    {
        if(GameMenu)
            return;

        if((forceX <= 0 && forceY <= 0) || (forceDecay < i_decay))
            forceDecay = i_decay;

        // don't override random shake by sequential while random shake is active
        if((forceX <= 0 && forceY <= 0) || (type != SHAKE_RANDOM))
            type = i_type;

        if(forceX < i_forceX)
            forceX = i_forceX;
        if(forceY < i_forceY)
            forceY = i_forceY;
        if(duration < i_duration)
            duration = i_duration;

        active = true;
    }

    void clear()
    {
        forceX = 0.0;
        forceY = 0.0;
        duration = 0.0;
        active = false;
    }
};

static ScreenShake_t s_shakeScreen;

//static double s_shakeScreenX = 0;
//static double s_shakeScreenY = 0;
//static int s_shakeScreenType = SHAKE_RANDOM;
//static double s_shakeScreenDuration = 0;
//static double s_shakeScreenSign = +1.0;

void doShakeScreen(int force, int type)
{
    s_shakeScreen.setup(force, force, type, 0, 1.0);
}

void doShakeScreen(int forceX, int forceY, int type, int duration, double decay)
{
    s_shakeScreen.setup(forceX, forceY, type, duration, decay);
}

void doShakeScreenClear()
{
    s_shakeScreen.clear();
}

void GraphicsLazyPreLoad()
{
    // TODO: check if this is needed at caller
    SetupScreens();

    int numScreens = 1;

    if(ScreenType == 1)
        numScreens = 2;
    if(ScreenType == 4)
        numScreens = 2;
    if(ScreenType == 5)
    {
        DynamicScreen();
        if(vScreen[2].Visible)
            numScreens = 2;
        else
            numScreens = 1;
    }
    if(ScreenType == 8)
        numScreens = 1;


    if(SingleCoop == 2)
        numScreens = 1; // fine to be 1, since it would just be run for Z = 2 twice otherwise;

    For(Z, 1, numScreens)
    {
        if(SingleCoop == 2)
            Z = 2;

        int S = Player[Z].Section;
        int bg = Background2[S];

        switch(bg)
        {
        case 1: // Double-row background
            XRender::lazyPreLoad(GFXBackground2[1]);
            XRender::lazyPreLoad(GFXBackground2[2]);
            break;

        case 2: // Single-row clouds background
            XRender::lazyPreLoad(GFXBackground2[2]);
            break;

        case 3: // Double-row background
            XRender::lazyPreLoad(GFXBackground2[3]);
            XRender::lazyPreLoad(GFXBackground2[2]);
            break;
        case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13:
            // All these backgrounds do use picture with the number less with 1
            XRender::lazyPreLoad(GFXBackground2[bg - 1]);
            break;
        case 22: // Double-row background
            XRender::lazyPreLoad(GFXBackground2[22]);
            XRender::lazyPreLoad(GFXBackground2[2]);
            break;

        default: // Any other normal backgrounds
            if(bg < 1 || bg > maxBackgroundType)
                break; // Don't crash it, stupid!
            XRender::lazyPreLoad(GFXBackground2[bg]);
            break;
        }

        For(A, 1, numPlayers)
        {
            Player_t &p = Player[A];
            int c = p.Character;
            int s = p.State;

            switch(c)
            {
            case 1:
                XRender::lazyPreLoad(GFXMarioBMP[s]);
                break;
            case 2:
                XRender::lazyPreLoad(GFXLuigiBMP[s]);
                break;
            case 3:
                XRender::lazyPreLoad(GFXPeachBMP[s]);
                break;
            case 4:
                XRender::lazyPreLoad(GFXToadBMP[s]);
                break;
            case 5:
                XRender::lazyPreLoad(GFXLinkBMP[s]);
                break;
            default: // Trap
                abort(); // "Please fix me up if you implemented a new playable character, see gfx_update.cpp!"
                return;
            }
        }

        For(A, 1, numBackground)
        {
            auto &b = Background[A];
            if(vScreenCollision(Z, b.Location) && !b.Hidden && IF_INRANGE(b.Type, 1, maxBackgroundType))
                XRender::lazyPreLoad(GFXBackgroundBMP[b.Type]);
        }

        int64_t fBlock = 0;
        int64_t lBlock = 0;
        blockTileGet(-vScreenX[Z], vScreen[Z].Width, fBlock, lBlock);

        For(A, fBlock, lBlock)
        {
            auto &b = Block[A];
            if(vScreenCollision(Z, b.Location) && !b.Hidden && IF_INRANGE(b.Type, 1, maxBlockType))
                XRender::lazyPreLoad(GFXBlock[Block[A].Type]);
        }

        for(int A = 1; A <= numNPCs; A++)
        {
            auto &n = NPC[A];
            if(vScreenCollision(Z, n.Location) && IF_INRANGE(n.Type, 0, maxNPCType))
                XRender::lazyPreLoad(GFXNPC[n.Type]);
        }
    }
}

// This draws the graphic to the screen when in a level/game menu/outro/level editor
void UpdateGraphics(bool skipRepaint)
{
//    On Error Resume Next
    float c = ShadowMode ? 0.f : 1.f;
    int A = 0;
//    std::string timeStr;
    int Z = 0;
    int numScreens = 0;

    if(!GameIsActive)
        return;

#ifdef USE_RENDER_BLOCKING
    // might want to put this after the logic part of UpdateGraphics,
    // once we have merged the multires code that separates logic from
    // rendering
    if(XRender::renderBlocked())
        return;
#endif

    // frame skip code
    cycleNextInc();

    if(FrameSkip && !TakeScreen)
    {
        if(frameSkipNeeded()) // Don't draw this frame
        {
            numScreens = 1;
            if(!LevelEditor)
            {
                if(ScreenType == 1)
                    numScreens = 2;
                if(ScreenType == 4)
                    numScreens = 2;
                if(ScreenType == 5)
                {
                    DynamicScreen();
                    if(vScreen[2].Visible)
                        numScreens = 2;
                    else
                        numScreens = 1;
                }
                if(ScreenType == 8)
                    numScreens = 1;
            }

            for(Z = 1; Z <= numScreens; Z++)
            {
                if(!LevelEditor)
                {
                    if(ScreenType == 2 || ScreenType == 3)
                        GetvScreenAverage();
                    else if(ScreenType == 5 && !vScreen[2].Visible)
                        GetvScreenAverage();
                    else if(ScreenType == 7)
                        GetvScreenCredits();
                    else
                    {
                        GetvScreen(Z);
                    }
                }

                for(A = 1; A <= numNPCs; A++)
                {
                    if(vScreenCollision(Z, NPC[A].Location) && !NPC[A].Hidden)
                    {
                        if(NPC[A].Reset[Z] || NPC[A].Active)
                        {
                            if(!NPC[A].Active)
                            {
                                NPC[A].JustActivated = Z;
//                                if(nPlay.Online == true)
//                                {
//                                    Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                    NPC[A].JustActivated = nPlay.MySlot + 1;
//                                }
                            }
                            NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                            if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                timeStr += "2b" + std::to_string(A) + LB;
                            NPC[A].Active = true;
                        }
                        NPC[A].Reset[1] = false;
                        NPC[A].Reset[2] = false;
                    }
                    else
                    {
                        NPC[A].Reset[Z] = true;
                        if(numScreens == 1)
                            NPC[A].Reset[2] = true;
                        if(SingleCoop == 1)
                            NPC[A].Reset[2] = true;
                        else if(SingleCoop == 2)
                            NPC[A].Reset[1] = true;
                    }
                }
            }
            return;
        }
    }

    XRender::setTargetTexture();

    frameNextInc();
    frameRenderStart();
    lunaRenderStart();

    g_stats.reset();

    std::string SuperText;
    std::string tempText;
    // int BoxY = 0;
    // bool tempBool = false;
    int B = 0;
//    int B2 = 0;
    int C = 0;
    int D = 0;
    int E = 0;
    double d2 = 0;
//    int e2 = 0;
//    int X = 0;
    int Y = 0;
    int64_t fBlock = 0;
    int64_t lBlock = 0;
    Location_t tempLocation;
    int S = 0; // Level section to display

    if(Score > 9999990)
        Score = 9999990;

    if(Lives > 99)
        Lives = 99;

    numScreens = 1;

//    If TakeScreen = True Then // Useless
//        If LevelEditor = True Or MagicHand = True Then
//            frmLevelWindow.vScreen(1).AutoRedraw = True
//        Else
//            frmMain.AutoRedraw = True
//        End If
//    End If

    // Background frames
    if(!FreezeNPCs)
    {
        BackgroundFrameCount[26]++;
        if(BackgroundFrameCount[26] >= 8)
        {
            BackgroundFrame[26]++;
            if(BackgroundFrame[26] >= 8)
                BackgroundFrame[26] = 0;
            BackgroundFrameCount[26] = 0;
        }
        BackgroundFrameCount[18]++;
        if(BackgroundFrameCount[18] >= 12)
        {
            BackgroundFrame[18]++;
            if(BackgroundFrame[18] >= 4)
                BackgroundFrame[18] = 0;
            BackgroundFrame[19] = BackgroundFrame[18];
            BackgroundFrame[20] = BackgroundFrame[18];
            BackgroundFrame[161] = BackgroundFrame[18];
            BackgroundFrameCount[18] = 0;
        }
        BackgroundFrameCount[36] += 1;
        if(BackgroundFrameCount[36] >= 2)
        {
            BackgroundFrame[36] += 1;
            if(BackgroundFrame[36] >= 4)
                BackgroundFrame[36] = 0;
            BackgroundFrameCount[36] = 0;
        }
        BackgroundFrame[68] = BackgroundFrame[36];
        BackgroundFrameCount[65] += 1;
        if(BackgroundFrameCount[65] >= 8)
        {
            BackgroundFrame[65] += 1;
            if(BackgroundFrame[65] >= 4)
                BackgroundFrame[65] = 0;
            BackgroundFrameCount[65] = 0;
        }

        BackgroundFrame[66] = BackgroundFrame[65];

        BackgroundFrame[70] = BackgroundFrame[65];
        BackgroundFrame[100] = BackgroundFrame[65];

        BackgroundFrame[134] = BackgroundFrame[65];
        BackgroundFrame[135] = BackgroundFrame[65];
        BackgroundFrame[136] = BackgroundFrame[65];
        BackgroundFrame[137] = BackgroundFrame[65];
        BackgroundFrame[138] = BackgroundFrame[65];


        BackgroundFrameCount[82] += 1;
        if(BackgroundFrameCount[82] >= 10)
        {
            BackgroundFrame[82] += 1;
            if(BackgroundFrame[82] >= 4)
                BackgroundFrame[82] = 0;
            BackgroundFrameCount[82] = 0;
        }

        BackgroundFrameCount[170] += 1;
        if(BackgroundFrameCount[170] >= 8)
        {
            BackgroundFrame[170] += 1;
            if(BackgroundFrame[170] >= 4)
                BackgroundFrame[170] = 0;
            BackgroundFrame[171] = BackgroundFrame[170];
            BackgroundFrameCount[170] = 0;
        }

        BackgroundFrameCount[125] += 1;
        if(BackgroundFrameCount[125] >= 4)
        {
            if(BackgroundFrame[125] == 0)
                BackgroundFrame[125] = 1;
            else
                BackgroundFrame[125] = 0;
            BackgroundFrameCount[125] = 0;
        }
        SpecialFrames();
    }

    BackgroundFrame[172] = BackgroundFrame[66];
    BackgroundFrameCount[158] += 1;

    if(BackgroundFrameCount[158] >= 6)
    {
        BackgroundFrameCount[158] = 0;
        BackgroundFrame[158] += 1;
        BackgroundFrame[159] += 1;
        if(BackgroundFrame[158] >= 4)
            BackgroundFrame[158] = 0;
        if(BackgroundFrame[159] >= 8)
            BackgroundFrame[159] = 0;
    }

    BackgroundFrameCount[168] += 1;
    if(BackgroundFrameCount[168] >= 8)
    {
        BackgroundFrame[168] += 1;
        if(BackgroundFrame[168] >= 8)
            BackgroundFrame[168] = 0;
        BackgroundFrameCount[168] = 0;
    }

    BackgroundFrameCount[173] += 1;
    if(BackgroundFrameCount[173] >= 8)
    {
        BackgroundFrameCount[173] = 0;
        if(BackgroundFrame[173] == 0)
            BackgroundFrame[173] = 1;
        else
            BackgroundFrame[173] = 0;
    }

    BackgroundFrameCount[187] += 1;
    if(BackgroundFrameCount[187] >= 6)
    {
        BackgroundFrame[187] += 1;
        if(BackgroundFrame[187] >= 4)
            BackgroundFrame[187] = 0;
        BackgroundFrame[188] = BackgroundFrame[187];
        BackgroundFrame[189] = BackgroundFrame[187];
        BackgroundFrame[190] = BackgroundFrame[187];
        BackgroundFrameCount[187] = 0;
    }


//    If LevelEditor = False Then  'Sets up the screens if not in level editor
    if(!LevelEditor)
    {
        if(ScreenType == 1)
            numScreens = 2;
        if(ScreenType == 4)
            numScreens = 2;
        if(ScreenType == 5)
        {
            DynamicScreen();
            if(vScreen[2].Visible)
                numScreens = 2;
            else
                numScreens = 1;
        }
        if(ScreenType == 8)
            numScreens = 1;
    }

    if(ClearBuffer)
    {
        ClearBuffer = false;
        XRender::clearBuffer();
    }

    if(SingleCoop == 2)
        numScreens = 2;

    For(Z, 1, numScreens)
    {
        if(SingleCoop == 2)
            Z = 2;

        if(LevelEditor)
            S = curSection;
        else
            S = Player[Z].Section;

        // not sure why this is getting set over and over again
        if(GameOutro)
            ScreenType = 7;

        if(!LevelEditor)
        {
            if(ScreenType == 2 || ScreenType == 3)
                GetvScreenAverage();
            else if(ScreenType == 5 && !vScreen[2].Visible)
                GetvScreenAverage();
            else if(ScreenType == 7)
                GetvScreenCredits();
            else
                GetvScreen(Z);
            if(Background2[S] == 0)
            {
                if(numScreens > 1)
                    XRender::renderRect(vScreen[Z].Left, vScreen[Z].Top,
                                         vScreen[Z].Width, vScreen[Z].Height, 0.f, 0.f, 0.f, 1.f, true);
                else
                    XRender::clearBuffer();
            }
        }
//        Else
//            If Background2(S) = 0 Then BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
//        End If

        if(qScreen)
        {
            if(vScreenX[1] < qScreenX[1] - 2)
                qScreenX[1] -= 2;
            else if(vScreenX[1] > qScreenX[1] + 2)
                qScreenX[1] += 2;
            if(vScreenY[1] < qScreenY[1] - 2)
                qScreenY[1] -= 2;
            else if(vScreenY[1] > qScreenY[1] + 2)
                qScreenY[1] += 2;
            if(qScreenX[1] < vScreenX[1] + 5 && qScreenX[1] > vScreenX[1] - 5 &&
               qScreenY[1] < vScreenY[1] + 5 && qScreenY[1] > vScreenY[1] - 5)
                qScreen = false;
            vScreenX[1] = qScreenX[1];
            vScreenY[1] = qScreenY[1];
        }

        // noturningback
        if(!LevelEditor)
        {
            if(NoTurnBack[Player[Z].Section])
            {
                A = Z;
                if(numScreens > 1)
                {
                    if(Player[1].Section == Player[2].Section)
                    {
                        if(Z == 1)
                            GetvScreen(2);
                        if(-vScreenX[1] < -vScreenX[2])
                            A = 1;
                        else
                            A = 2;
                    }
                }
                if(-vScreenX[A] > level[S].X)
                {
                    LevelChop[S] += float(-vScreenX[A] - level[S].X);
                    level[S].X = -vScreenX[A];
                }
            }
        }

        if(numScreens > 1) // To separate drawing of screens
            XRender::setViewport(vScreen[Z].Left, vScreen[Z].Top, vScreen[Z].Width, vScreen[Z].Height);

        DrawBackground(S, Z);

        // don't show background outside of the current section!
        if(LevelEditor)
        {
            if(vScreenX[Z] + level[S].X > 0) {
                XRender::renderRect(0, 0,
                                   vScreenX[Z] + level[S].X, ScreenH, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if(ScreenW > level[S].Width + vScreenX[Z]) {
                XRender::renderRect(level[S].Width + vScreenX[Z], 0,
                                   ScreenW - (level[S].Width + vScreenX[Z]), ScreenH, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if(vScreenY[Z] + level[S].Y > 0) {
                XRender::renderRect(0, 0,
                                   ScreenW, vScreenY[Z] + level[S].Y, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if(ScreenH > level[S].Height + vScreenY[Z]) {
                XRender::renderRect(0, level[S].Height + vScreenY[Z],
                                   ScreenW, ScreenH - (level[S].Height + vScreenY[Z]), 0.2f, 0.2f, 0.2f, 1.f, true);
            }
        }


//        If GameMenu = True Then
        // if(GameMenu)
        // {
            // (Commented out in original code :thinking:)
            // Curtain
//            XRender::renderTexture(0, 0, GFX.MenuGFX[1]);
            // Game logo
//            XRender::renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70, GFX.MenuGFX[2]);
//        ElseIf LevelEditor = False Then
        if(!GameMenu && !LevelEditor)
        {
//            If numPlayers > 2 And nPlay.Online = False Then
            if(numPlayers > 2)
            {
//                For A = 1 To numPlayers
                For(A, 1, numPlayers)
                {
//                    With Player(A)
                    Player_t &p = Player[A];
//                        If vScreenCollision(Z, .Location) = False And LevelMacro = 0 And .Location.Y < level(.Section).Height And .Location.Y + .Location.Height > level(.Section).Y And .TimeToLive = 0 And .Dead = False Then
                    if(!vScreenCollision(Z, p.Location) && LevelMacro == LEVELMACRO_OFF &&
                        p.Location.Y < level[p.Section].Height &&
                        p.Location.Y + p.Location.Height > level[p.Section].Y &&
                        p.TimeToLive == 0 && !p.Dead)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Section == Player[A].Section && vScreenCollision(Z, Player[B].Location))
                            {
                                if(C == 0 || std::abs(Player[A].Location.X + Player[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                {
                                    C = std::abs(Player[A].Location.X + Player[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                    D = B;
                                }
                            }
                        }
                        if(C == 0)
                        {
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Section == Player[A].Section)
                                {
                                    if(C == 0 || std::abs(Player[A].Location.X + Player[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(Player[A].Location.X + Player[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                        D = B;
                                    }
                                }
                            }
                        }
                        Player[A].Location.X = Player[D].Location.X + Player[D].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
                        Player[A].Location.Y = Player[D].Location.Y + Player[D].Location.Height - Player[A].Location.Height;
                        Player[A].Section = Player[D].Section;
                        Player[A].Location.SpeedX = Player[D].Location.SpeedX;
                        Player[A].Location.SpeedY = Player[D].Location.SpeedY;
                        Player[A].Location.SpeedY = dRand() * 12 - 6;
                        Player[A].CanJump = true;
                    }
//                    End With
//                Next A
                }
//            End If
            }
//        End If
        }

        if(LevelEditor)
        {
            For(A, 1, numBackground)
            {
                if(Background[A].Type == 11 || Background[A].Type == 12 || Background[A].Type == 60
                    || Background[A].Type == 61 || Background[A].Type == 75 || Background[A].Type == 76
                    || Background[A].Type == 77 || Background[A].Type == 78 || Background[A].Type == 79)
                {
                    g_stats.checkedBGOs++;
                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                    {
                        g_stats.renderedBGOs++;
                        XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                              vScreenY[Z] + Background[A].Location.Y,
                                              GFXBackgroundWidth[Background[A].Type],
                                              BackgroundHeight[Background[A].Type],
                                              GFXBackgroundBMP[Background[A].Type], 0,
                                              BackgroundHeight[Background[A].Type] *
                                              BackgroundFrame[Background[A].Type]);
                    }
                }
            }
        }
        else
        {
//            For A = 1 To MidBackground - 1 'First backgrounds
            For(A, 1, MidBackground - 1)  // First backgrounds
            {
                g_stats.checkedBGOs++;
//                if(BackgroundHasNoMask[Background[A].Type] == false) // Useless code
//                {
//                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
//                    {
//                        XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
//                                              vScreenY[Z] + Background[A].Location.Y,
//                                              BackgroundWidth[Background[A].Type],
//                                              BackgroundHeight[Background[A].Type],
//                                              GFXBackgroundBMP[Background[A].Type], 0,
//                                              BackgroundHeight[Background[A].Type] *
//                                              BackgroundFrame[Background[A].Type]);
//                    }
//                }
//                else
//                {
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                          vScreenY[Z] + Background[A].Location.Y,
                                          GFXBackgroundWidth[Background[A].Type],
                                          BackgroundHeight[Background[A].Type],
                                          GFXBackgroundBMP[Background[A].Type], 0,
                                          BackgroundHeight[Background[A].Type] *
                                          BackgroundFrame[Background[A].Type]);
                }
            }
        }

        tempLocation.Width = 32;
        tempLocation.Height = 32;

        For(A, 1, sBlockNum) // Display sizable blocks
        {
            g_stats.checkedSzBlocks++;
            if(BlockIsSizable[Block[sBlockArray[A]].Type] && (!Block[sBlockArray[A]].Invis || LevelEditor))
            {
                if(vScreenCollision(Z, Block[sBlockArray[A]].Location) && !Block[sBlockArray[A]].Hidden)
                {
                    g_stats.renderedSzBlocks++;
                    int bHeight = Block[sBlockArray[A]].Location.Height / 32.0;
                    for(B = 0; B < bHeight; B++)
                    {
                        int bWidth = Block[sBlockArray[A]].Location.Width / 32.0;
                        for(C = 0; C < bWidth; C++)
                        {
                            tempLocation.X = Block[sBlockArray[A]].Location.X + C * 32;
                            tempLocation.Y = Block[sBlockArray[A]].Location.Y + B * 32;
                            if(vScreenCollision(Z, tempLocation))
                            {
                                D = C;
                                E = B;

                                if(D != 0)
                                {
                                    if(fEqual(D, (Block[sBlockArray[A]].Location.Width / 32.0) - 1))
                                        D = 2;
                                    else
                                    {
                                        D = 1;
                                        d2 = 0.5;
                                        UNUSED(d2);
                                    }
                                }

                                if(E != 0)
                                {
                                    if(fEqual(E, (Block[sBlockArray[A]].Location.Height / 32.0) - 1))
                                        E = 2;
                                    else
                                        E = 1;
                                }
#if 0 // Simplified below
                                if((D == 0 || D == 2) || (E == 0 || E == 2) || (Block[sBlockArray[A]].Type == 130 && (D == 0 || D == 2) && E == 1))
                                {
                                    XRender::renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                                }
                                else
                                {
                                    XRender::renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                                }
#endif
                                XRender::renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                            }
                        }
                    }
                }
            }
        }

        if(LevelEditor)
        {
            For(A, 1, numBackground)
            {
                if(!(Background[A].Type == 11 || Background[A].Type == 12 || Background[A].Type == 60
                    || Background[A].Type == 61 || Background[A].Type == 75 || Background[A].Type == 76
                    || Background[A].Type == 77 || Background[A].Type == 78 || Background[A].Type == 79) && !Foreground[Background[A].Type])
                {
                    g_stats.checkedBGOs++;
                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                    {
                        g_stats.renderedBGOs++;
                        XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                              vScreenY[Z] + Background[A].Location.Y,
                                              GFXBackgroundWidth[Background[A].Type],
                                              BackgroundHeight[Background[A].Type],
                                              GFXBackgroundBMP[Background[A].Type], 0,
                                              BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                    }
                }
            }
        }
        else if(numBackground > 0)
        {
            for(A = MidBackground; A <= LastBackground; A++) // Second backgrounds
            {
                g_stats.checkedBGOs++;
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                          vScreenY[Z] + Background[A].Location.Y,
                                          BackgroundWidth[Background[A].Type],
                                          BackgroundHeight[Background[A].Type],
                                          GFXBackgroundBMP[Background[A].Type],
                                          0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
        }

        For(A, numBackground + 1, numBackground + numLocked) // Locked doors
        {
            g_stats.checkedBGOs++;
            if(vScreenCollision(Z, Background[A].Location) &&
                (Background[A].Type == 98 || Background[A].Type == 160) && !Background[A].Hidden)
            {
                g_stats.renderedBGOs++;
                XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                      vScreenY[Z] + Background[A].Location.Y,
                                      BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type],
                                      GFXBackgroundBMP[Background[A].Type],
                                      0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
            }
        }

//        For A = 1 To numNPCs 'Display NPCs that should be behind blocks
        For(A, 1, numNPCs) // Display NPCs that should be behind blocks
        {
            g_stats.checkedNPCs++;
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(((NPC[A].Effect == 208 || NPCIsAVine[NPC[A].Type] ||
                 NPC[A].Type == 209 || NPC[A].Type == 159 || NPC[A].Type == 245 ||
                 NPC[A].Type == 8 || NPC[A].Type == 93 || NPC[A].Type == 74 ||
                 NPC[A].Type == 256 || NPC[A].Type == 257 || NPC[A].Type == 51 ||
                 NPC[A].Type == 52 || NPC[A].Effect == 1 || NPC[A].Effect == 3 ||
                 NPC[A].Effect == 4 || (NPC[A].Type == 45 && NPC[A].Special == 0.0)) &&
                 (NPC[A].standingOnPlayer == 0 && (!NPC[A].Generator || LevelEditor))) ||
                 NPC[A].Type == 179 || NPC[A].Type == 270)
            {
                if(NPC[A].Effect != 2 && (!NPC[A].Generator || LevelEditor))
                {
                    if(vScreenCollision(Z, NPC[A].Location) && !NPC[A].Hidden)
                    {
                        if(NPC[A].Active)
                        {
                            if(NPC[A].Type == 8 || NPC[A].Type == 74 || NPC[A].Type == 93 || NPC[A].Type == 245 || NPC[A].Type == 256 || NPC[A].Type == 270)
                            {
                                g_stats.renderedNPCs++;
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type], cn, cn, cn);
                            }
                            else if(NPC[A].Type == 51 || NPC[A].Type == 257)
                            {
                                g_stats.renderedNPCs++;
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type],
                                        vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type],
                                        NPC[A].Location.Width, NPC[A].Location.Height,
                                        GFXNPC[NPC[A].Type], 0,
                                        NPC[A].Frame * NPCHeight[NPC[A].Type] + NPCHeight[NPC[A].Type] - NPC[A].Location.Height,
                                        cn, cn, cn);
                            }
                            else if(NPC[A].Type == 52)
                            {
                                g_stats.renderedNPCs++;
                                if(NPC[A].Direction == -1)
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type]);
                                }
                                else
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], NPCWidth[NPC[A].Type] - NPC[A].Location.Width, NPC[A].Frame * NPCHeight[NPC[A].Type], cn, cn, cn);
                                }
                            }
                            else if(NPCWidthGFX[NPC[A].Type] == 0 || NPC[A].Effect == 1)
                            {
                                g_stats.renderedNPCs++;
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type], cn ,cn ,cn);
                            }
                            else
                            {
                                g_stats.renderedNPCs++;
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type] - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                            }
                        }
                        if(NPC[A].Reset[Z] || NPC[A].Active)
                        {
                            if(!NPC[A].Active)
                            {
                                NPC[A].JustActivated = Z;
//                                if(nPlay.Online == true)
//                                {
//                                    Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                    NPC[A].JustActivated = nPlay.MySlot + 1;
//                                }
                            }
                            NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                            if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                timeStr += "2b" + std::to_string(A) + LB;
                            NPC[A].Active = true;
                        }
                        NPC[A].Reset[1] = false;
                        NPC[A].Reset[2] = false;
                    }
                    else
                    {
                        NPC[A].Reset[Z] = true;
                        if(numScreens == 1)
                            NPC[A].Reset[2] = true;
                        if(SingleCoop == 1)
                            NPC[A].Reset[2] = true;
                        else if(SingleCoop == 2)
                            NPC[A].Reset[1] = true;
                    }
                }
            }
        }


//        For A = 1 To numPlayers 'Players behind blocks
        For(A, 1, numPlayers)
        {
            if(!Player[A].Dead && !Player[A].Immune2 && Player[A].TimeToLive == 0 && Player[A].Effect == 3)
            {
                float Y2 = 0;
                float X2 = 0;

                if(vScreenCollision(Z, Player[A].Location))
                {
                    // warp NPCs
                    if(Player[A].HoldingNPC > 0 && Player[A].Frame != 15)
                    {
                        auto warpNpcLoc = newLoc(NPC[Player[A].HoldingNPC].Location.X - (NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] - NPC[Player[A].HoldingNPC].Location.Width) / 2.0,
                                                 NPC[Player[A].HoldingNPC].Location.Y,
                                                 static_cast<double>(NPCWidthGFX[NPC[Player[A].HoldingNPC].Type]),
                                                 static_cast<double>(NPCHeight[NPC[Player[A].HoldingNPC].Type]));

                        if((vScreenCollision(Z, NPC[Player[A].HoldingNPC].Location) || vScreenCollision(Z, warpNpcLoc)) && !NPC[Player[A].HoldingNPC].Hidden)
                        {
                            tempLocation = NPC[Player[A].HoldingNPC].Location;
                            if(NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] != 0 || NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] != 0)
                            {
                                tempLocation.Height = NPCHeightGFX[NPC[Player[A].HoldingNPC].Type];
                                tempLocation.Width = NPCWidthGFX[NPC[Player[A].HoldingNPC].Type];
                                tempLocation.Y = NPC[Player[A].HoldingNPC].Location.Y + NPC[Player[A].HoldingNPC].Location.Height - NPCHeightGFX[NPC[Player[A].HoldingNPC].Type];
                                tempLocation.X = NPC[Player[A].HoldingNPC].Location.X + NPC[Player[A].HoldingNPC].Location.Width / 2.0 - NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] / 2.0;
                            }
                            else
                            {
                                tempLocation.Height = NPCHeight[NPC[Player[A].HoldingNPC].Type];
                                tempLocation.Width = NPCWidth[NPC[Player[A].HoldingNPC].Type];
                            }

                            tempLocation.X += NPCFrameOffsetX[NPC[Player[A].HoldingNPC].Type];
                            tempLocation.Y += NPCFrameOffsetY[NPC[Player[A].HoldingNPC].Type];
                            Y2 = 0;
                            X2 = 0;

                            NPCWarpGFX(A, tempLocation, X2, Y2);

                            if(NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] != 0 || NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] != 0)
                            {
                                XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                      vScreenY[Z] + tempLocation.Y,
                                                      tempLocation.Width,
                                                      tempLocation.Height,
                                                      GFXNPC[NPC[Player[A].HoldingNPC].Type],
                                                      X2,
                                                      Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeightGFX[NPC[Player[A].HoldingNPC].Type]);
                            }
                            else
                            {
                                XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                      vScreenY[Z] + tempLocation.Y,
                                                      tempLocation.Width,
                                                      tempLocation.Height,
                                                      GFXNPC[NPC[Player[A].HoldingNPC].Type],
                                                      X2,
                                                      Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeight[NPC[Player[A].HoldingNPC].Type]);
                            }
                        }
                    }

                    if(Player[A].Mount == 3)
                    {
                        B = Player[A].MountType;
                        // Yoshi's Body
                        tempLocation = roundLoc(Player[A].Location, 2.0);
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X += Player[A].YoshiBX;
                        tempLocation.Y += Player[A].YoshiBY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                              vScreenY[Z] + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              GFXYoshiB[B],
                                              X2,
                                              Y2 + 32 * Player[A].YoshiBFrame,
                                              c, c, c);
                        // Yoshi's Head
                        tempLocation = roundLoc(Player[A].Location, 2.0);
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X += Player[A].YoshiTX;
                        tempLocation.Y += Player[A].YoshiTY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                              vScreenY[Z] + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              GFXYoshiT[B],
                                              X2,
                                              Y2 + 32 * Player[A].YoshiTFrame);
                    }

                    // FIXME: Convert this mess of duplicated code into united with usage of references, pointers, and ternary expressions
                    if(Player[A].Character == 1)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            /*
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                            */
                            tempLocation.Height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - (Player[A].State == 1 ? 30 : 0);
                            tempLocation.Width = 100;
                            tempLocation.X += MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y += MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXMario[Player[A].State],
                                                  pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X += Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y += Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFX.Boot[Player[A].MountType],
                                                  X2, Y2 + 32 * Player[A].MountFrame);
                        }
                        else
                        {
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            tempLocation.Height = 100;
                            tempLocation.Width = 100;
                            tempLocation.X += MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y += MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXMario[Player[A].State],
                                                  pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(Player[A].Frame * Player[A].Direction) + Y2,
                                                  c, c, c);
                        }
                    }
                    else if(Player[A].Character == 2)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                            tempLocation.Width = 100;
                            tempLocation.X += LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y += LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width, tempLocation.Height,
                                                  GFXLuigi[Player[A].State],
                                                  pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X += Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y += Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width, tempLocation.Height,
                                                  GFX.Boot[Player[A].MountType],
                                                  X2, Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            tempLocation.Height = 100;
                            tempLocation.Width = 100;
                            tempLocation.X += LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y += LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width, tempLocation.Height, GFXLuigi[Player[A].State],
                                                  pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(Player[A].Frame * Player[A].Direction) + Y2);
                        }
                    }
                    else if(Player[A].Character == 3)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = roundLoc(Player[A].Location, 2.0);

                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;

                            tempLocation.Width = 100;
                            tempLocation.X += PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y += PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXPeach[Player[A].State],
                                                  pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X += Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y += Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFX.Boot[Player[A].MountType],
                                                  X2,
                                                  Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            tempLocation.Height = 100;
                            tempLocation.Width = 100;
                            tempLocation.X += PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y += PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width, tempLocation.Height,
                                                  GFXPeach[Player[A].State],
                                                  pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                        }
                    }
                    else if(Player[A].Character == 4)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 26;
                            tempLocation.Width = 100;
                            tempLocation.X += ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            if(Player[A].State == 1)
                                tempLocation.Y += ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + 6;
                            else
                                tempLocation.Y += ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 4;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width, tempLocation.Height,
                                                  GFXToad[Player[A].State],
                                                  pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X += Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y += Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width, tempLocation.Height,
                                                  GFX.Boot[Player[A].MountType],
                                                  X2, Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = roundLoc(Player[A].Location, 2.0);
                            tempLocation.Height = 100;
                            tempLocation.Width = 100;
                            tempLocation.X += ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y += ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width, tempLocation.Height,
                                                  GFXToad[Player[A].State],
                                                  pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                        }
                    }
                    else if(Player[A].Character == 5)
                    {
                        if(Player[A].Frame > 5)
                            Player[A].Frame = 1;
                        tempLocation = roundLoc(Player[A].Location, 2.0);
                        tempLocation.Height = 100;
                        tempLocation.Width = 100;
                        tempLocation.X += LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                        tempLocation.Y += LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y,
                                              tempLocation.Width, tempLocation.Height,
                                              GFXLink[Player[A].State],
                                              pfrX(Player[A].Frame * Player[A].Direction) + X2,
                                              pfrY(Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                    }
                }

                if(Player[A].HoldingNPC > 0 && Player[A].Frame == 15)
                {
                    auto &hNpc = NPC[Player[A].HoldingNPC];
                    auto hNpcLoc = newLoc(hNpc.Location.X - (NPCWidthGFX[hNpc.Type] - hNpc.Location.Width) / 2.0,
                                          hNpc.Location.Y,
                                          static_cast<double>(NPCWidthGFX[hNpc.Type]),
                                          static_cast<double>(NPCHeight[hNpc.Type]));

                    if((vScreenCollision(Z, hNpc.Location) || vScreenCollision(Z, hNpcLoc)) && !hNpc.Hidden && hNpc.Type != 263)
                    {
                        tempLocation = hNpc.Location;
                        if(NPCHeightGFX[hNpc.Type] != 0 || NPCWidthGFX[hNpc.Type] != 0)
                        {
                            tempLocation.Height = NPCHeightGFX[hNpc.Type];
                            tempLocation.Width = NPCWidthGFX[hNpc.Type];
                            tempLocation.Y = hNpc.Location.Y + hNpc.Location.Height - NPCHeightGFX[hNpc.Type];
                            tempLocation.X = hNpc.Location.X + hNpc.Location.Width / 2.0 - NPCWidthGFX[hNpc.Type] / 2.0;
                        }
                        else
                        {
                            tempLocation.Height = NPCHeight[hNpc.Type];
                            tempLocation.Width = NPCWidth[hNpc.Type];
                        }

                        tempLocation.X += NPCFrameOffsetX[hNpc.Type];
                        tempLocation.Y += NPCFrameOffsetY[hNpc.Type];
                        Y2 = 0;
                        X2 = 0;

                        NPCWarpGFX(A, tempLocation, X2, Y2);
                        if(NPCHeightGFX[hNpc.Type] != 0 || NPCWidthGFX[hNpc.Type] != 0)
                        {
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXNPC[hNpc.Type],
                                                  X2,
                                                  Y2 + hNpc.Frame * NPCHeightGFX[hNpc.Type]);
                        }
                        else
                        {
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXNPC[hNpc.Type],
                                                  X2,
                                                  Y2 + hNpc.Frame * NPCHeight[hNpc.Type]);
                        }
                    }
                }
            }
        }


        if(LevelEditor)
        {
            fBlock = 1;
            lBlock = numBlock;
        }
        else
        {
            //fBlock = FirstBlock[int(-vScreenX[Z] / 32) - 1];
            //lBlock = LastBlock[int((-vScreenX[Z] + vScreen[Z].Width) / 32) + 1];
            blockTileGet(-vScreenX[Z], vScreen[Z].Width, fBlock, lBlock);
        }


//        For A = fBlock To lBlock 'Non-Sizable Blocks
        For(A, fBlock, lBlock)
        {
            g_stats.checkedBlocks++;
            if(!BlockIsSizable[Block[A].Type] && (!Block[A].Invis || (LevelEditor && BlockFlash <= 30)) && Block[A].Type != 0 && !BlockKills[Block[A].Type])
            {
                if(vScreenCollision(Z, Block[A].Location) && !Block[A].Hidden)
                {
                    g_stats.renderedBlocks++;
                    // Don't show a visual difference of hit-resized block in a comparison to original state
                    double offX = Block[A].wasShrinkResized ? 0.05 : 0.0;
                    double offW = Block[A].wasShrinkResized ? 0.1 : 0.0;
                    XRender::renderTexture(vScreenX[Z] + Block[A].Location.X - offX,
                                          vScreenY[Z] + Block[A].Location.Y + Block[A].ShakeY3,
                                          Block[A].Location.Width + offW,
                                          Block[A].Location.Height,
                                          GFXBlock[Block[A].Type],
                                          0,
                                          BlockFrame[Block[A].Type] * 32);
                }
            }
        }

//'effects in back
        for(A = 1; A <= numEffects; A++)
        {
            g_stats.checkedEffects++;
            if(Effect[A].Type == 112 || Effect[A].Type == 54 || Effect[A].Type == 55 ||
               Effect[A].Type == 59 || Effect[A].Type == 77 || Effect[A].Type == 81 ||
               Effect[A].Type == 82 || Effect[A].Type == 103 || Effect[A].Type == 104 ||
               Effect[A].Type == 114 || Effect[A].Type == 123 || Effect[A].Type == 124)
            {
                g_stats.renderedEffects++;
                if(vScreenCollision(Z, Effect[A].Location))
                {
                    float cn = Effect[A].Shadow ? 0.f : 1.f;
                    XRender::renderTexture(vScreenX[Z] + Effect[A].Location.X, vScreenY[Z] + Effect[A].Location.Y, Effect[A].Location.Width, Effect[A].Location.Height, GFXEffect[Effect[A].Type], 0, Effect[A].Frame * EffectHeight[Effect[A].Type], cn, cn, cn);
                }
            }
        }


        for(A = 1; A <= numNPCs; A++) // Display NPCs that should be behind other npcs
        {
            g_stats.checkedNPCs++;
            float cn = NPC[A].Shadow ? 0.f : 1.f;

            if(NPC[A].Effect == 0)
            {
                if(NPC[A].HoldingPlayer == 0 && (NPC[A].standingOnPlayer > 0 || NPC[A].Type == 56 ||
                   NPC[A].Type == 22 || NPC[A].Type == 49 || NPC[A].Type == 91 || NPC[A].Type == 160 ||
                   NPC[A].Type == 282 || NPCIsACoin[NPC[A].Type]) && (!NPC[A].Generator || LevelEditor))
                {
                    auto npcALoc = newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0,
                                          NPC[A].Location.Y,
                                          static_cast<double>(NPCWidthGFX[NPC[A].Type]),
                                          static_cast<double>(NPCHeight[NPC[A].Type]));

                    // If Not NPCIsACoin(.Type) Then
                    if((vScreenCollision(Z, NPC[A].Location) || vScreenCollision(Z, npcALoc)) && !NPC[A].Hidden)
                    {
                        if(NPC[A].Active)
                        {
                            g_stats.renderedNPCs++;
                            if(NPCWidthGFX[NPC[A].Type] == 0)
                            {
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                            }
                            else
                            {
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                            }
                        }

                        if(NPC[A].Reset[Z] || NPC[A].Active)
                        {
                            if(!NPC[A].Active)
                            {
                                NPC[A].JustActivated = Z;
//                                    if(nPlay.Online == true)
//                                    {
//                                        Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                        NPC[A].JustActivated = nPlay.MySlot + 1;
//                                    }
                            }

                            NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                                if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                    timeStr += "2b" + std::to_string(A) + LB;
                            NPC[A].Active = true;
                        }

                        NPC[A].Reset[1] = false;
                        NPC[A].Reset[2] = false;
                    }
                    else
                    {
                        NPC[A].Reset[Z] = true;
                        if(numScreens == 1)
                            NPC[A].Reset[2] = true;
                        if(SingleCoop == 1)
                            NPC[A].Reset[2] = true;
                        else if(SingleCoop == 2)
                            NPC[A].Reset[1] = true;
                    }
                    // End If
                }
            }
        }


        for(A = 1; A <= numNPCs; A++) // ice
        {
            g_stats.checkedNPCs++;
            if(NPC[A].Type == 263 && NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
            {
                auto npcALoc = newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0,
                                      NPC[A].Location.Y,
                                      static_cast<double>(NPCWidthGFX[NPC[A].Type]),
                                      static_cast<double>(NPCHeight[NPC[A].Type]));

                if((vScreenCollision(Z, NPC[A].Location) || vScreenCollision(Z, npcALoc)) && !NPC[A].Hidden)
                {
                    g_stats.renderedNPCs++;
                    DrawFrozenNPC(Z, A);
                    if(NPC[A].Reset[Z] || NPC[A].Active)
                    {
                        if(!NPC[A].Active)
                        {
                            NPC[A].JustActivated = Z;
//                            if(nPlay.Online == true)
//                            {
//                                Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                NPC[A].JustActivated = nPlay.MySlot + 1;
//                            }
                        }

                        NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                        if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                            timeStr += "2b" + std::to_string(A) + LB;
                        NPC[A].Active = true;
                    }
                    NPC[A].Reset[1] = false;
                    NPC[A].Reset[2] = false;
                }
                else
                {
                    NPC[A].Reset[Z] = true;
                    if(numScreens == 1)
                        NPC[A].Reset[2] = true;
                    if(SingleCoop == 1)
                        NPC[A].Reset[2] = true;
                    else if(SingleCoop == 2)
                        NPC[A].Reset[1] = true;
                }
            }
        }


//        For A = 1 To numNPCs 'Display NPCs that should be in front of blocks
        For(A, 1, numNPCs) // Display NPCs that should be in front of blocks
        {
            g_stats.checkedNPCs++;
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(NPC[A].Effect == 0)
            {
                if(!(NPC[A].HoldingPlayer > 0 || NPCIsAVine[NPC[A].Type] || NPC[A].Type == 209 || NPC[A].Type == 282 ||
                     NPC[A].Type == 270 || NPC[A].Type == 160 || NPC[A].Type == 159 || NPC[A].Type == 8 || NPC[A].Type == 245 ||
                     NPC[A].Type == 93 || NPC[A].Type == 51 || NPC[A].Type == 52 || NPC[A].Type == 74 || NPC[A].Type == 256 ||
                     NPC[A].Type == 257 || NPC[A].Type == 56 || NPC[A].Type == 22 || NPC[A].Type == 49 || NPC[A].Type == 91) &&
                   !(NPC[A].Type == 45 && NPC[A].Special == 0) && NPC[A].standingOnPlayer == 0 &&
                   !NPCForeground[NPC[A].Type] && (!NPC[A].Generator || LevelEditor) &&
                   NPC[A].Type != 179 && NPC[A].Type != 263)
                {
                    if(!NPCIsACoin[NPC[A].Type])
                    {
                        if(vScreenCollision(Z, NPC[A].Location) && !NPC[A].Hidden)
                        {
                            if(NPC[A].Type == 0)
                            {
                                NPC[A].Killed = 9;
                                KillNPC(A, 9);
                            }
                            else if(NPC[A].Active)
                            {
                                if(!NPCIsYoshi[NPC[A].Type])
                                {
                                    g_stats.renderedNPCs++;
                                    if(NPCWidthGFX[NPC[A].Type] == 0)
                                    {
                                        XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                                    }
                                    else
                                    {
                                        if(NPC[A].Type == 283 && NPC[A].Special > 0)
                                        {
                                            if(NPCWidthGFX[NPC[A].Special] == 0)
                                            {
                                                tempLocation.Width = NPCWidth[NPC[A].Special];
                                                tempLocation.Height = NPCHeight[NPC[A].Special];
                                            }
                                            else
                                            {
                                                tempLocation.Width = NPCWidthGFX[NPC[A].Special];
                                                tempLocation.Height = NPCHeightGFX[NPC[A].Special];
                                            }
                                            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                                            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - tempLocation.Height / 2.0;
                                            B = EditorNPCFrame((int)SDL_floor(NPC[A].Special), NPC[A].Direction);
                                            XRender::renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[A].Special], 0, B * tempLocation.Height);
                                        }

                                        XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                                    }
                                }
                                else
                                {
                                    if(NPC[A].Type == 95)
                                        B = 1;
                                    else if(NPC[A].Type == 98)
                                        B = 2;
                                    else if(NPC[A].Type == 99)
                                        B = 3;
                                    else if(NPC[A].Type == 100)
                                        B = 4;
                                    else if(NPC[A].Type == 148)
                                        B = 5;
                                    else if(NPC[A].Type == 149)
                                        B = 6;
                                    else if(NPC[A].Type == 150)
                                        B = 7;
                                    else if(NPC[A].Type == 228)
                                        B = 8;
                                    int YoshiBX = 0;
                                    int YoshiBY = 0;
                                    int YoshiTX = 0;
                                    int YoshiTY = 0;
                                    int YoshiTFrame = 0;
                                    int YoshiBFrame = 0;
                                    YoshiBX = 0;
                                    YoshiBY = 0;
                                    YoshiTX = 20;
                                    YoshiTY = -32;
                                    YoshiBFrame = 6;
                                    YoshiTFrame = 0;
                                    if(NPC[A].Special == 0.0)
                                    {
                                        if(!FreezeNPCs)
                                            NPC[A].FrameCount += 1;
                                        if(NPC[A].FrameCount >= 70)
                                        {
                                            if(!FreezeNPCs)
                                                NPC[A].FrameCount = 0;
                                        }
                                        else if(NPC[A].FrameCount >= 50)
                                            YoshiTFrame = 3;
                                    }
                                    else
                                    {
                                        if(!FreezeNPCs)
                                            NPC[A].FrameCount += 1;
                                        if(NPC[A].FrameCount > 8)
                                        {
                                            YoshiBFrame = 0;
                                            NPC[A].FrameCount = 0;
                                        }
                                        else if(NPC[A].FrameCount > 6)
                                        {
                                            YoshiBFrame = 1;
                                            YoshiTX -= 1;
                                            YoshiTY += 2;
                                            YoshiBY += 1;
                                        }
                                        else if(NPC[A].FrameCount > 4)
                                        {
                                            YoshiBFrame = 2;
                                            YoshiTX -= 2;
                                            YoshiTY += 4;
                                            YoshiBY += 2;
                                        }
                                        else if(NPC[A].FrameCount > 2)
                                        {
                                            YoshiBFrame = 1;
                                            YoshiTX -= 1;
                                            YoshiTY += 2;
                                            YoshiBY += 1;
                                        }
                                        else
                                            YoshiBFrame = 0;
                                        if(!FreezeNPCs)
                                            NPC[A].Special2 += 1;
                                        if(NPC[A].Special2 > 30)
                                        {
                                            YoshiTFrame = 0;
                                            if(!FreezeNPCs)
                                                NPC[A].Special2 = 0;
                                        }
                                        else if(NPC[A].Special2 > 10)
                                            YoshiTFrame = 2;

                                    }
                                    if(YoshiBFrame == 6)
                                    {
                                        YoshiBY += 10;
                                        YoshiTY += 10;
                                    }
                                    if(NPC[A].Direction == 1)
                                    {
                                        YoshiTFrame += 5;
                                        YoshiBFrame += 7;
                                    }
                                    else
                                    {
                                        YoshiBX = -YoshiBX;
                                        YoshiTX = -YoshiTX;
                                    }
                                    // YoshiBX += 4
                                    // YoshiTX += 4
                                    g_stats.renderedNPCs++;
                                    // Yoshi's Body
                                    XRender::renderTexture(vScreenX[Z] + SDL_floor(NPC[A].Location.X) + YoshiBX, vScreenY[Z] + NPC[A].Location.Y + YoshiBY, 32, 32, GFXYoshiB[B], 0, 32 * YoshiBFrame, cn, cn, cn);

                                    // Yoshi's Head
                                    XRender::renderTexture(vScreenX[Z] + SDL_floor(NPC[A].Location.X) + YoshiTX, vScreenY[Z] + NPC[A].Location.Y + YoshiTY, 32, 32, GFXYoshiT[B], 0, 32 * YoshiTFrame, cn, cn, cn);
                                }
                            }
                            if((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Active || NPC[A].Type == 57)
                            {
                                if(!NPC[A].Active)
                                {
                                    NPC[A].JustActivated = Z;
//                                    if(nPlay.Online == true)
//                                    {
//                                        Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                        NPC[A].JustActivated = nPlay.MySlot + 1;
//                                    }
                                }
                                NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
                                if(NPCIsYoshi[NPC[A].Type] || NPCIsBoot[NPC[A].Type] || NPC[A].Type == 9 || NPC[A].Type == 14 || NPC[A].Type == 22 || NPC[A].Type == 90 || NPC[A].Type == 153 || NPC[A].Type == 169 || NPC[A].Type == 170 || NPC[A].Type == 182 || NPC[A].Type == 183 || NPC[A].Type == 184 || NPC[A].Type == 185 || NPC[A].Type == 186 || NPC[A].Type == 187 || NPC[A].Type == 188 || NPC[A].Type == 195 || NPC[A].Type == 104)
                                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen * 20;

//                                if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                    timeStr += "2b" + std::to_string(A) + LB;
                                NPC[A].Active = true;
                            }
                            NPC[A].Reset[1] = false;
                            NPC[A].Reset[2] = false;
                        }
                        else
                        {
                            NPC[A].Reset[Z] = true;
                            if(numScreens == 1)
                                NPC[A].Reset[2] = true;
                            if(SingleCoop == 1)
                                NPC[A].Reset[2] = true;
                            else if(SingleCoop == 2)
                                NPC[A].Reset[1] = true;
                        }
                    }
                }
            }
//        Next A
        }

        // npc chat bubble
        for(A = 1; A <= numNPCs; A++)
        {
            g_stats.checkedNPCs++;
            if(NPC[A].Active && NPC[A].Chat)
            {
                B = NPCHeightGFX[NPC[A].Type] - NPC[A].Location.Height;
                if(B < 0)
                    B = 0;
                g_stats.renderedNPCs++;
                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPC[A].Location.Width / 2.0 - GFX.Chat.w / 2, vScreenY[Z] + NPC[A].Location.Y - 30 - B, GFX.Chat.w, GFX.Chat.h, GFX.Chat, 0, 0);
            }
        }


        For(A, 1, numPlayers) // The clown car
        {
            if(!Player[A].Dead && !Player[A].Immune2 && Player[A].TimeToLive == 0 &&
               !(Player[A].Effect == 3 || Player[A].Effect == 5) && Player[A].Mount == 2)
            {
                int frameX = 0;
                int frameY = 0;
                StdPicture *playerGfx = nullptr;

                switch(Player[A].Character)
                {
                default:
                case 1: // Mario
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 36;
                    frameX = MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    frameY = MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    playerGfx = &GFXMarioBMP[Player[A].State];
                    break;

                case 2: // Luigi
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 38;
                    frameX = LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    frameY = LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    playerGfx = &GFXLuigiBMP[Player[A].State];

                    break;
                case 3: // Peach
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 30;
                    frameX = PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    frameY = PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    playerGfx = &GFXPeachBMP[Player[A].State];
                    break;

                case 4: // Toad
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 30;
                    frameX = ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    frameY = ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    playerGfx = &GFXToadBMP[Player[A].State];
                    break;

                case 5: // Link
                    Y = 30;
                    frameX = LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    frameY = LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    playerGfx = &GFXLinkBMP[Player[A].State];
                    break;
                }

                XRender::renderTexture(
                        vScreenX[Z] + SDL_floor(Player[A].Location.X) + frameX - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64,
                        vScreenY[Z] + Player[A].Location.Y + frameY + Player[A].MountOffsetY - Y,
                        99,
                        Player[A].Location.Height - 20 - Player[A].MountOffsetY,
                        *playerGfx,
                        pfrX(Player[A].Frame * Player[A].Direction),
                        pfrY(Player[A].Frame * Player[A].Direction),
                        c, c, c);
                XRender::renderTexture(
                        vScreenX[Z] + SDL_floor(Player[A].Location.X) + Player[A].Location.Width / 2.0 - 64,
                        vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128,
                        128,
                        128,
                        GFX.Mount[Player[A].Mount],
                        0,
                        128 * Player[A].MountFrame,
                        c, c, c);
            }
        }


        for(A = 1; A <= numNPCs; A++) // Put held NPCs on top
        {
            g_stats.checkedNPCs++;
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(
                (
                  (
                    (NPC[A].HoldingPlayer > 0 && Player[NPC[A].HoldingPlayer].Effect != 3) ||
                    (NPC[A].Type == 50 && NPC[A].standingOnPlayer == 0) ||
                    (NPC[A].Type == 17 && NPC[A].CantHurt > 0)
                  ) || NPC[A].Effect == 5
                ) && NPC[A].Type != 91 && !Player[NPC[A].HoldingPlayer].Dead
            )
            {
                if(NPC[A].Type == 263)
                {
                    g_stats.renderedNPCs++;
                    DrawFrozenNPC(Z, A);
                }
                else if(!NPCIsYoshi[NPC[A].Type] && NPC[A].Type > 0)
                {
                    g_stats.renderedNPCs++;
                    if(NPCWidthGFX[NPC[A].Type] == 0)
                    {
                        XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                    }
                    else
                    {
                        XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                    }
                }
            }
        }



//'normal player draw code
//        For A = numPlayers To 1 Step -1 'Players in front of blocks
        for(int A = numPlayers; A >= 1; A--)// Players in front of blocks
        {
//            DrawPlayer A, Z
            DrawPlayer(Player[A], Z);
//        Next A
        }
//'normal player end




        if(LevelEditor)
        {
            For(A, 1, numBackground)
            {
                if(Foreground[Background[A].Type])
                {
                    g_stats.checkedBGOs++;
                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                    {
                        g_stats.renderedBGOs++;
                        XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                              vScreenY[Z] + Background[A].Location.Y,
                                              GFXBackgroundWidth[Background[A].Type],
                                              BackgroundHeight[Background[A].Type],
                                              GFXBackgroundBMP[Background[A].Type], 0,
                                              BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                    }
                }
            }
        }
        else
        {
            for(A = LastBackground + 1; A <= numBackground; A++) // Foreground objects
            {
                g_stats.checkedBGOs++;
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, GFXBackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackground[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
//        End If
        }

        for(A = 1; A <= numNPCs; A++) // foreground NPCs
        {
            g_stats.checkedNPCs++;
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(NPC[A].Effect == 0)
            {
                if(NPCForeground[NPC[A].Type] && NPC[A].HoldingPlayer == 0 && (!NPC[A].Generator || LevelEditor))
                {
                    if(!NPCIsACoin[NPC[A].Type])
                    {
                        if(vScreenCollision(Z, NPC[A].Location) && !NPC[A].Hidden)
                        {
                            if(NPC[A].Active)
                            {
                                g_stats.renderedNPCs++;
                                if(NPCWidthGFX[NPC[A].Type] == 0)
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                                }
                                else
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                                }
                            }
                            if((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Active)
                            {
                                if(!NPC[A].Active)
                                {
                                    NPC[A].JustActivated = Z;
//                                    if(nPlay.Online == true)
//                                    {
//                                        Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                        NPC[A].JustActivated = nPlay.MySlot + 1;
//                                    }
                                }
                                NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                                if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                    timeStr += "2b" + std::to_string(A) + LB;
                                NPC[A].Active = true;
                            }
                            NPC[A].Reset[1] = false;
                            NPC[A].Reset[2] = false;
                        }
                        else
                        {
                            NPC[A].Reset[Z] = true;
                            if(numScreens == 1)
                                NPC[A].Reset[2] = true;
                            if(SingleCoop == 1)
                                NPC[A].Reset[2] = true;
                            else if(SingleCoop == 2)
                                NPC[A].Reset[1] = true;
                        }
                    }
                }
            }
        }

        for(A = fBlock; A <= lBlock; A++) // Blocks in Front
        {
            g_stats.checkedBlocks++;
            if(BlockKills[Block[A].Type])
            {
                if(vScreenCollision(Z, Block[A].Location) && !Block[A].Hidden)
                {
                    g_stats.renderedBlocks++;
                    // Don't show a visual difference of hit-resized block in a comparison to original state
                    double offX = Block[A].wasShrinkResized ? 0.05 : 0.0;
                    double offW = Block[A].wasShrinkResized ? 0.1 : 0.0;
                    XRender::renderTexture(vScreenX[Z] + Block[A].Location.X - offX,
                                          vScreenY[Z] + Block[A].Location.Y + Block[A].ShakeY3,
                                          Block[A].Location.Width + offW,
                                          Block[A].Location.Height,
                                          GFXBlock[Block[A].Type],
                                          0,
                                          BlockFrame[Block[A].Type] * 32);
                }
            }
        }

// effects on top
        For(A, 1, numEffects)
        {
            g_stats.checkedEffects++;
//            With Effect(A)
            auto &e = Effect[A];
//                If .Type <> 112 And .Type <> 54 And .Type <> 55 And .Type <> 59 And .Type <> 77 And .Type <> 81 And .Type <> 82 And .Type <> 103 And .Type <> 104 And .Type <> 114 And .Type <> 123 And .Type <> 124 Then
            if(e.Type != 112 && e.Type != 54 && e.Type != 55 && e.Type != 59 &&
               e.Type != 77 && e.Type != 81 && e.Type != 82 && e.Type != 103 &&
               e.Type != 104 && e.Type != 114 && e.Type != 123 && e.Type != 124)
            {
//                    If vScreenCollision(Z, .Location) Then
                if(vScreenCollision(Z, e.Location))
                {
                    g_stats.renderedEffects++;
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffectMask(.Type), 0, .Frame * EffectHeight(.Type), vbSrcAnd
//                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffect(.Type), 0, .Frame * EffectHeight(.Type), vbSrcPaint
                    float c = e.Shadow ? 0.f : 1.f;
                    XRender::renderTexture(int(vScreenX[Z] + e.Location.X), int(vScreenY[Z] + e.Location.Y),
                                          int(e.Location.Width), int(e.Location.Height),
                                          GFXEffectBMP[e.Type], 0, e.Frame * EffectHeight[e.Type], c, c, c);
//                    End If
                }
//                End If
            }
//            End With
//        Next A
        }

        // water
        if(LevelEditor)
        {
            for(B = 1; B <= numWater; B++)
            {
                if(!Water[B].Hidden && vScreenCollision(Z, Water[B].Location))
                {
                    if(Water[B].Quicksand)
                        XRender::renderRect(vScreenX[Z] + Water[B].Location.X, vScreenY[Z] + Water[B].Location.Y, Water[B].Location.Width, Water[B].Location.Height,
                            1.f, 1.f, 0.f, 1.f, false);
                    else
                        XRender::renderRect(vScreenX[Z] + Water[B].Location.X, vScreenY[Z] + Water[B].Location.Y, Water[B].Location.Width, Water[B].Location.Height,
                            0.f, 1.f, 1.f, 1.f, false);
                }
            }
        }

        if(!LevelEditor) // Graphics for the main game.
        {
        // NPC Generators
            for(A = 1; A <= numNPCs; A++)
            {
                g_stats.checkedNPCs++;
                if(NPC[A].Generator)
                {
                    if(vScreenCollision(Z, NPC[A].Location) && !NPC[A].Hidden)
                        NPC[A].GeneratorActive = true;
                }
            }
            if(vScreen[2].Visible)
            {
                if(int(vScreen[Z].Width) == ScreenW)
                {
                    if(vScreen[Z].Top != 0.0)
                    {
                        XRender::renderRect(0, 0, vScreen[Z].Width, 1, 0.f, 0.f, 0.f);
                    }
                    else
                    {
                        XRender::renderRect(0, vScreen[Z].Height - 1, vScreen[Z].Width, 1, 0.f, 0.f, 0.f);
                    }
                }
                else
                {
                    if(vScreen[Z].Left != 0.0)
                    {
                        XRender::renderRect(0, 0, 1, vScreen[Z].Height, 0.f, 0.f, 0.f);
                    }
                    else
                    {
                        XRender::renderRect(vScreen[Z].Width - 1, 0, 1, vScreen[Z].Height, 0.f, 0.f, 0.f);
                    }
                }
            }

        // player names
        /* Dropped */

            lunaRender(Z);

            // Always draw for single-player
            // And don't draw when many players at the same screen
            if(numPlayers == 1 || numScreens != 1)
                g_levelVScreenFader[Z].draw();

    //    'Interface
    //            B = 0
            B = 0;
    //            C = 0
            C = 0;
    //            If GameMenu = False And GameOutro = False Then
            if(!GameMenu && !GameOutro)
            {
    //                For A = 1 To numPlayers
                For(A, 1, numPlayers)
                {
                    if(Player[A].ShowWarp > 0)
                    {
                        std::string tempString;
                        auto &w = Warp[Player[A].ShowWarp];
                        if(!w.noPrintStars && w.maxStars > 0 && Player[A].Mount != 2)
                        {
                            tempString = fmt::format_ne("{0}/{1}", w.curStars, w.maxStars);
                            XRender::renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z] + 1, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                            XRender::renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 - 20, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z], GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                            SuperPrint(tempString, 3,
                                       float(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 + 18),
                                       float(Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z]));
                        }
                    }
                }

                lunaRenderHud();
    //                DrawInterface Z, numScreens
                if(ShowOnScreenHUD && !gSMBXHUDSettings.skip)
                    DrawInterface(Z, numScreens);

                For(A, 1, numNPCs) // Display NPCs that got dropped from the container
                {
                    g_stats.checkedNPCs++;

                    if(NPC[A].Effect == 2)
                    {
                        if(std::fmod(NPC[A].Effect2, 3) != 0.0)
                        {
                            if(vScreenCollision(Z, NPC[A].Location))
                            {
                                if(NPC[A].Active)
                                {
                                    g_stats.renderedNPCs++;
                                    if(NPCWidthGFX[NPC[A].Type] == 0)
                                    {
                                        XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height);
                                    }
                                    else
                                    {
                                        XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type] - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type]);
                                    }
                                }

                                if(NPC[A].Reset[Z] || NPC[A].Active)
                                {
                                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                                    if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                        timeStr += "2b" + std::to_string(A) + LB;
                                    NPC[A].Active = true;
                                }

                                NPC[A].Reset[1] = false;
                                NPC[A].Reset[2] = false;
                            }
                            else
                                NPC[A].Reset[Z] = true;
                        }
                    }
                }
            }

            else if(!GameOutro)
                mainMenuDraw();

            if(PrintFPS > 0)
            {
                XRender::offsetViewportIgnore(true);
                SuperPrint(fmt::format_ne("{0}", int(PrintFPS)), 1, 8, 8, 0.f, 1.f, 0.f);
                XRender::offsetViewportIgnore(false);
            }

            g_stats.print();

        }

//        If LevelEditor = True Or MagicHand = True Then
        if((LevelEditor || MagicHand) && GamePaused == PauseCode::None)
        {
            // editor code now located in `gfx_editor.cpp`
            DrawEditorLevel(Z);
        }

        if(numScreens > 1) // for multiple screens
            XRender::resetViewport();

        if(GameOutro)
            DrawCredits();

//        If LevelEditor = True Then
//            StretchBlt frmLevelWindow.vScreen(Z).hdc, 0, 0, frmLevelWindow.vScreen(Z).ScaleWidth, frmLevelWindow.vScreen(Z).ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy
//        Else
        { // NOT AN EDITOR!!!
            s_shakeScreen.update();
        }

        // TODO: VERIFY THIS
        XRender::offsetViewportIgnore(true);
        if(ScreenType == 5 && numScreens == 1)
        {
            speedRun_renderControls(1, -1);
            speedRun_renderControls(2, -1);
        }
        else
            speedRun_renderControls(Z, Z);
        XRender::offsetViewportIgnore(false);

//    Next Z
    } // For(Z, 2, numScreens)

    g_levelScreenFader.draw();

    XRender::offsetViewportIgnore(true);
    speedRun_renderTimer();

    // render special screens
    if(GamePaused == PauseCode::PauseScreen)
        PauseScreen::Render();

    if(GamePaused == PauseCode::Message)
        DrawMessage(MessageText);

    if(QuickReconnectScreen::g_active)
        QuickReconnectScreen::Render();

    if(GamePaused == PauseCode::Reconnect || GamePaused == PauseCode::DropAdd)
    {
        ConnectScreen::Render();
        XRender::renderTexture(int(SharedCursor.X), int(SharedCursor.Y), GFX.ECursor[2]);
    }

    if(GamePaused == PauseCode::TextEntry)
        TextEntryScreen::Render();

    XRender::offsetViewportIgnore(false);

    if(!skipRepaint)
        XRender::repaint();

    XRender::setTargetScreen();

//    If TakeScreen = True Then ScreenShot
    if(TakeScreen)
        ScreenShot();

    // Update Coin Frames
    CoinFrame2[1] += 1;
    if(CoinFrame2[1] >= 6)
    {
        CoinFrame2[1] = 0;
        CoinFrame[1] += 1;
        if(CoinFrame[1] >= 4)
            CoinFrame[1] = 0;
    }
    CoinFrame2[2] += 1;
    if(CoinFrame2[2] >= 6)
    {
        CoinFrame2[2] = 0;
        CoinFrame[2] += 1;
        if(CoinFrame[2] >= 7)
            CoinFrame[2] = 0;
    }
    CoinFrame2[3] += 1;
    if(CoinFrame2[3] >= 7)
    {
        CoinFrame2[3] = 0;
        CoinFrame[3] += 1;
        if(CoinFrame[3] >= 4)
            CoinFrame[3] = 0;
    }
//    if(nPlay.Mode == 0)
//    {
//        if(nPlay.NPCWaitCount >= 11)
//            nPlay.NPCWaitCount = 0;
//        nPlay.NPCWaitCount += 2;
//        if(timeStr != "")
//            Netplay::sendData timeStr + LB;
//    }

    lunaRenderEnd();
    frameRenderEnd();

//    if(XRender::lazyLoadedBytes() > 200000) // Reset timer while loading many pictures at the same time
//        resetFrameTimer();
    XRender::lazyLoadedBytesReset();
}
