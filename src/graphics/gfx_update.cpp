/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <SDL2/SDL_timer.h>

#include "../globals.h"
#include "../frame_timer.h"
#include "../graphics.h"
#include "../collision.h"
#include "../editor.h"
#include "../npc.h"
#include "../location.h"
#include "../main/menu_main.h"
#include "../main/speedrunner.h"
#include "../control/joystick.h"
#include "../main/trees.h"
#include "../npc_id.h"
#include "../compat.h"

#ifdef __3DS__
#include "../3ds/n3ds-clock.h"
#include "../3ds/second_screen.h"
#include "../3ds/editor_screen.h"
#endif

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
            frmMain.offsetViewport(0, 0);
            forceX = 0.0;
            forceY = 0.0;
            active = false;
        }
        else
        {
            switch(type)
            {
            default:
            case SHAKE_RANDOM:
                offsetX = forceX > 0 ? (int)round((SDL_fmod(iRand(), forceX) * 4) - forceX * 2) : 0;
                offsetY = forceY > 0 ? (int)round((SDL_fmod(iRand(), forceY) * 4) - forceY * 2) : 0;
                break;
            case SHAKE_SEQUENTIAL:
                offsetX = forceX > 0 ? (int)round(sign * forceX) : 0;
                offsetY = forceY > 0 ? (int)round(sign * forceY) : 0;
                sign *= -1;
                break;
            }
            frmMain.offsetViewport(offsetX, offsetY);
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

const int maxDrawNPCs = 512;

class NPC_Draw_Queue_t
{
public:
    uint16_t BG[maxDrawNPCs];
    size_t BG_n;
    uint16_t Low[maxDrawNPCs];
    size_t Low_n;
    uint16_t Iced[maxDrawNPCs];
    size_t Iced_n;
    uint16_t Normal[maxDrawNPCs];
    size_t Normal_n;
    uint16_t Chat[20];
    size_t Chat_n;
    uint16_t Held[20];
    size_t Held_n;
    uint16_t FG[maxDrawNPCs];
    size_t FG_n;
    uint16_t Dropped[20];
    size_t Dropped_n;
    void reset()
    {
        BG_n = Low_n = Iced_n = Normal_n = Chat_n = Held_n = FG_n = Dropped_n = 0;
    }
    void add(uint16_t A)
    {
        if(NPC[A].Chat)
        {
            if (Chat_n == sizeof(Chat)/sizeof(uint16_t))
                return;
            Chat[Chat_n] = A;
            Chat_n += 1;
        }

        if(NPC[A].Effect == 2)
        {
            if(std::fmod(NPC[A].Effect2, 3) == 0.0)
                return;
            if (Dropped_n == sizeof(Dropped)/sizeof(uint16_t))
                return;
            Dropped[Dropped_n] = A;
            Dropped_n += 1;
        }
        else if(
                (
                  (
                    (NPC[A].HoldingPlayer > 0 && Player[NPC[A].HoldingPlayer].Effect != 3) ||
                    (NPC[A].Type == 50 && NPC[A].standingOnPlayer == 0) ||
                    (NPC[A].Type == 17 && NPC[A].CantHurt > 0)
                  ) || NPC[A].Effect == 5
                ) && NPC[A].Type != 91 && !Player[NPC[A].HoldingPlayer].Dead
            )
        {
            if (Held_n == sizeof(Held)/sizeof(uint16_t))
                return;
            Held[Held_n] = A;
            Held_n += 1;
        }
        else if(NPC[A].Effect == 0 && NPCForeground[NPC[A].Type] && NPC[A].HoldingPlayer == 0 && !NPCIsACoin[NPC[A].Type])
        {
            if (FG_n == sizeof(FG)/sizeof(uint16_t))
                return;
            FG[FG_n] = A;
            FG_n += 1;
        }
        else if(NPC[A].Type == 263 && NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
        {
            if (Iced_n == sizeof(Iced)/sizeof(uint16_t))
                return;
            Iced[Iced_n] = A;
            Iced_n += 1;
        }
        else if(NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0 &&
            (NPC[A].standingOnPlayer > 0 || NPC[A].Type == 56 || NPC[A].Type == 22 ||
                NPC[A].Type == 49 || NPC[A].Type == 91 || NPC[A].Type == 160 ||
                NPC[A].Type == 282 || NPCIsACoin[NPC[A].Type]))
        {
            if (Low_n == sizeof(Low)/sizeof(uint16_t))
                return;
            Low[Low_n] = A;
            Low_n += 1;
        }
        else if(NPC[A].Type == 179 || NPC[A].Type == 270 ||
            ((NPC[A].Effect == 208 || NPCIsAVine[NPC[A].Type] ||
                    NPC[A].Type == 209 || NPC[A].Type == 159 || NPC[A].Type == 245 ||
                    NPC[A].Type == 8 || NPC[A].Type == 93 || NPC[A].Type == 74 ||
                    NPC[A].Type == 256 || NPC[A].Type == 257 || NPC[A].Type == 51 ||
                    NPC[A].Type == 52 || NPC[A].Effect == 1 || NPC[A].Effect == 3 ||
                    NPC[A].Effect == 4 || (NPC[A].Type == 45 && NPC[A].Special == 0.0))
                && NPC[A].standingOnPlayer == 0))
        {
            if (BG_n == sizeof(BG)/sizeof(uint16_t))
                return;
            BG[BG_n] = A;
            BG_n += 1;
        }
        else if(NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
        {
            if (Normal_n == sizeof(Normal)/sizeof(uint16_t))
                return;
            Normal[Normal_n] = A;
            Normal_n += 1;
        }
    }
};

NPC_Draw_Queue_t NPC_Draw_Queue[2] = {NPC_Draw_Queue_t(), NPC_Draw_Queue_t()};

// This draws the graphic to the screen when in a level/game menu/outro/level editor
void UpdateGraphics(bool skipRepaint)
{
//    On Error Resume Next
    float c = ShadowMode ? 0.f : 1.f;
    int A = 0;
    std::string timeStr;
    int Z = 0;
    int numScreens = 0;

    if(!GameIsActive)
        return;

#ifdef __ANDROID__
    if(frmMain.renderBlocked())
        return;
#endif

    // frame skip code
    cycleNextInc();

    frmMain.setTargetTexture();

    frameNextInc();
    frameRenderStart();
    bool Do_FrameSkip = FrameSkip && !TakeScreen && frameSkipNeeded();

    g_stats.reset();

    int B = 0;
//    int B2 = 0;
    int C = 0;
    int D = 0;
    int E = 0;
    double d2 = 0;
//    int e2 = 0;
    int X = 0;
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
        BackgroundFrameCount[36] = BackgroundFrameCount[36] + 1;
        if(BackgroundFrameCount[36] >= 2)
        {
            BackgroundFrame[36] = BackgroundFrame[36] + 1;
            if(BackgroundFrame[36] >= 4)
                BackgroundFrame[36] = 0;
            BackgroundFrameCount[36] = 0;
        }
        BackgroundFrame[68] = BackgroundFrame[36];
        BackgroundFrameCount[65] = BackgroundFrameCount[65] + 1;
        if(BackgroundFrameCount[65] >= 8)
        {
            BackgroundFrame[65] = BackgroundFrame[65] + 1;
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


        BackgroundFrameCount[82] = BackgroundFrameCount[82] + 1;
        if(BackgroundFrameCount[82] >= 10)
        {
            BackgroundFrame[82] = BackgroundFrame[82] + 1;
            if(BackgroundFrame[82] >= 4)
                BackgroundFrame[82] = 0;
            BackgroundFrameCount[82] = 0;
        }

        BackgroundFrameCount[170] = BackgroundFrameCount[170] + 1;
        if(BackgroundFrameCount[170] >= 8)
        {
            BackgroundFrame[170] = BackgroundFrame[170] + 1;
            if(BackgroundFrame[170] >= 4)
                BackgroundFrame[170] = 0;
            BackgroundFrame[171] = BackgroundFrame[170];
            BackgroundFrameCount[170] = 0;
        }

        BackgroundFrameCount[125] = BackgroundFrameCount[125] + 1;
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
        BackgroundFrame[158] = BackgroundFrame[158] + 1;
        BackgroundFrame[159] = BackgroundFrame[159] + 1;
        if(BackgroundFrame[158] >= 4)
            BackgroundFrame[158] = 0;
        if(BackgroundFrame[159] >= 8)
            BackgroundFrame[159] = 0;
    }

    BackgroundFrameCount[168] = BackgroundFrameCount[168] + 1;
    if(BackgroundFrameCount[168] >= 8)
    {
        BackgroundFrame[168] = BackgroundFrame[168] + 1;
        if(BackgroundFrame[168] >= 8)
            BackgroundFrame[168] = 0;
        BackgroundFrameCount[168] = 0;
    }

    BackgroundFrameCount[173] = BackgroundFrameCount[173] + 1;
    if(BackgroundFrameCount[173] >= 8)
    {
        BackgroundFrameCount[173] = 0;
        if(BackgroundFrame[173] == 0)
            BackgroundFrame[173] = 1;
        else
            BackgroundFrame[173] = 0;
    }

    BackgroundFrameCount[187] = BackgroundFrameCount[187] + 1;
    if(BackgroundFrameCount[187] >= 6)
    {
        BackgroundFrame[187] = BackgroundFrame[187] + 1;
        if(BackgroundFrame[187] >= 4)
            BackgroundFrame[187] = 0;
        BackgroundFrame[188] = BackgroundFrame[187];
        BackgroundFrame[189] = BackgroundFrame[187];
        BackgroundFrame[190] = BackgroundFrame[187];
        BackgroundFrameCount[187] = 0;
    }


    // Any and all screen-based logic... Things we don't want to need to do twice.
    SetupScreens();
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
        numScreens = 2;
    CenterScreens();

    For(Z, 1, numScreens)
    {
        if(SingleCoop == 2)
            Z = 2;

        if(LevelEditor)
            S = curSection;
        else
            S = Player[Z].Section;

        if(GameOutro) ScreenType = 7;

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

            if(ScreenType == 2 || ScreenType == 3)
                GetvScreenAverageCanonical(&X, &Y);
            else if(ScreenType == 5 && !vScreen[2].Visible)
                GetvScreenAverageCanonical(&X, &Y);
            else if(ScreenType == 7)
                GetvScreenAverageCanonical(&X, &Y);
            else
                GetvScreenCanonical(Z, &X, &Y);
        }

        // moved to `graphics/gfx_screen.cpp`
        if(!Do_FrameSkip && qScreen)
            Update_qScreen();

        // noturningback
        if(!LevelEditor && NoTurnBack[Player[Z].Section])
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

        // Position swap code?
        if(!GameMenu && !LevelEditor)
        {
            if(numPlayers > 2)
            {
                For(A, 1, numPlayers)
                {
                    Player_t &p = Player[A];
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
                        Player[A].Location.SpeedY = dRand() * 12 - 6; // ?????
                        Player[A].CanJump = true;
                    }
                }
            }
        }
        if(!Do_FrameSkip)
            NPC_Draw_Queue[Z].reset();
        if(!LevelEditor)
        {
            for(A = 1; A <= numNPCs; A++)
            {
                g_stats.checkedNPCs++;

                bool onscreen, cannot_reset, can_activate;
                Location_t loc2;
                bool loc2_exists;
                if(NPCWidthGFX[NPC[A].Type] != 0)
                {
                    loc2_exists = true;
                    loc2 = newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0, NPC[A].Location.Y, static_cast<double>(NPCWidthGFX[NPC[A].Type]), static_cast<double>(NPCHeight[NPC[A].Type]));
                    // not sure why loc2 does not consider NPCHeightGFX...
                }
                else
                    loc2_exists = false;
                if(NPC[A].Hidden)
                {
                    onscreen = cannot_reset = can_activate = false;
                }
                else if (g_compatibility.NPC_activate_mode == NPC_activate_modes::onscreen)
                {
                    onscreen = cannot_reset = can_activate = vScreenCollision(Z, NPC[A].Location) || (loc2_exists && vScreenCollision(Z, loc2));
                }
                else if (g_compatibility.NPC_activate_mode == NPC_activate_modes::smart)
                {
                    onscreen = vScreenCollision(Z, NPC[A].Location) || (loc2_exists && vScreenCollision(Z, loc2));
                    bool onscreen_orig = vScreenCollisionCanonical(X, Y, NPC[A].Location) || (loc2_exists && vScreenCollisionCanonical(X, Y, loc2));
                    cannot_reset = (onscreen || onscreen_orig);
                    if(ForcedControls || qScreen || !NPC[A].TriggerActivate.empty())
                        can_activate = onscreen_orig;
                    // generators, Cheep Cheeps, Thwomps, and more!
                    else if(NPC[A].Generator
                        || NPC[A].Type == NPCID_THWOMP_SMB3 || NPC[A].Type == NPCID_THWOMP_SMW
                        || (NPCIsCheep[NPC[A].Type] && Maths::iRound(NPC[A].Special) == 2))
                    {
                        can_activate = onscreen_orig;
                    }
                    // Think about NPCID_BULLET_SMB3, NPCID_BULLET_SMW, and NPCID_EERIE...
                    else
                        can_activate = onscreen;
                }
                else if (g_compatibility.NPC_activate_mode == NPC_activate_modes::orig)
                {
                    onscreen = vScreenCollision(Z, NPC[A].Location) || (loc2_exists && vScreenCollision(Z, loc2));
                    bool onscreen_orig = vScreenCollisionCanonical(X, Y, NPC[A].Location) || (loc2_exists && vScreenCollisionCanonical(X, Y, loc2));
                    cannot_reset = (onscreen || onscreen_orig);
                    can_activate = onscreen_orig;
                }
                if(NPC[A].Type == 0 || NPC[A].Generator)
                {
                    onscreen = false;
                }

                if(can_activate)
                {
                    if(NPC[A].Generator)
                        NPC[A].GeneratorActive = true;
                    else if(NPC[A].Type == 0) // what is this?
                    {
                        NPC[A].Killed = 9;
                        KillNPC(A, 9);
                    }
                    else if(!NPC[A].Active
                        && ((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Type == NPCID_CONVEYER))
                    {
                        NPC[A].Active = true;
                        NPC[A].JustActivated = Z;
                    }
                }

                // TODO: think through the logical conflict between `cannot_reset` and `can_activate`
                if(cannot_reset)
                {
                    if(NPC[A].Type != 0
                        && ((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Active || NPC[A].Type == NPCID_CONVEYER))
                    {
                        if(NPCIsYoshi[NPC[A].Type] || NPCIsBoot[NPC[A].Type] || NPC[A].Type == NPCID_SHROOM_SMB3
                            || NPC[A].Type == NPCID_FIREFLOWER_SMB3 || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_LIFE_SMB3
                            || NPC[A].Type == 153 || NPC[A].Type == NPCID_TANOOKISUIT || NPC[A].Type == NPCID_HAMMERSUIT || NPC[A].Type == NPCID_FIREFLOWER_SMB
                            || NPC[A].Type == NPCID_FIREFLOWER_SMW || NPC[A].Type == NPCID_SHROOM_SMB || NPC[A].Type == NPCID_SHROOM_SMW
                            || NPC[A].Type == NPCID_LIFE_SMB || NPC[A].Type == NPCID_LIFE_SMW || NPC[A].Type == NPCID_MOON || NPC[A].Type == NPCID_FLIPPEDDISCO
                            || NPC[A].Type == NPCID_PLATFORM_SMB3)
                            NPC[A].TimeLeft = Physics.NPCTimeOffScreen * 20;
                        else
                            NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
                    }

                    if(NPC[A].Active)
                    {
                        NPC[A].Reset[1] = false;
                        NPC[A].Reset[2] = false;
                    }
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

                if(!Do_FrameSkip && onscreen && ((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Active || NPC[A].Type == NPCID_CONVEYER))
                {
                    g_stats.renderedNPCs++;
                    NPC_Draw_Queue[Z].add(A);

                    if(!NPC[A].Active)
                        NPCFrames(A);
                }
            }
        }
        else
        {
            for(A = 1; A <= numNPCs; A++)
            {
                g_stats.checkedNPCs++;
                const Location_t loc2 = newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0, NPC[A].Location.Y, static_cast<double>(NPCWidthGFX[NPC[A].Type]), static_cast<double>(NPCHeight[NPC[A].Type]));
                if(!Do_FrameSkip && !NPC[A].Hidden
                    && (vScreenCollision(Z, NPC[A].Location) || vScreenCollision(Z, loc2)))
                {
                    g_stats.renderedNPCs++;
                    NPC_Draw_Queue[Z].add(A);
                }
            }
        }
    }

    if (Do_FrameSkip) return;

#ifdef __3DS__
    frmMain.initDraw(0);
#endif

    // buffer now cleared every frame because of cases where vScreens change positions
    frmMain.clearBuffer();

    if(SingleCoop == 2)
        numScreens = 2;

    // draw code now separated from logic.
    For(Z, 1, numScreens)
    {
        if(SingleCoop == 2)
            Z = 2;

        if (LevelEditor)
            S = curSection;
        else
            S = Player[Z].Section;

        frmMain.setViewport(vScreen[Z].ScreenLeft, vScreen[Z].ScreenTop, vScreen[Z].Width, vScreen[Z].Height);

#ifdef __3DS__
        frmMain.setLayer(0);
#endif

        DrawBackground(S, Z);

        // don't show background outside of the current section!
        if(LevelEditor)
        {
            if (vScreenX[Z] + level[S].X > 0) {
                frmMain.renderRect(0, 0,
                                   vScreenX[Z] + level[S].X, ScreenH, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if (ScreenW > level[S].Width + vScreenX[Z]) {
                frmMain.renderRect(level[S].Width + vScreenX[Z], 0,
                                   ScreenW - (level[S].Width + vScreenX[Z]), ScreenH, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if (vScreenY[Z] + level[S].Y > 0) {
                frmMain.renderRect(0, 0,
                                   ScreenW, vScreenY[Z] + level[S].Y, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if (ScreenH > level[S].Height + vScreenY[Z]) {
                frmMain.renderRect(0, level[S].Height + vScreenY[Z],
                                   ScreenW, ScreenH - (level[S].Height + vScreenY[Z]), 0.2f, 0.2f, 0.2f, 1.f, true);
            }
        }

#ifdef __3DS__
        frmMain.setLayer(1);
#endif

        // render first BGOs
        // TODO: unify once we have a quadtree that tracks background type among other things
        if (LevelEditor)
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
                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
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
        else{
            For(A, 1, MidBackground - 1)  // First backgrounds
            {
                g_stats.checkedBGOs++;
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
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
                                frmMain.renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                            }
                        }
                    }
                }
            }
        }

        // Mid-level BGOs
        if (LevelEditor)
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
                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
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
        { // NOT AN EDITOR
            for(A = MidBackground; A <= LastBackground; A++) // Second backgrounds
            {
                g_stats.checkedBGOs++;
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
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
                frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
                                      vScreenY[Z] + Background[A].Location.Y,
                                      BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type],
                                      GFXBackgroundBMP[Background[A].Type],
                                      0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
            }
        }

#ifdef __3DS__
        frmMain.setLayer(2);
#endif
//        For A = 1 To numNPCs 'Display NPCs that should be behind blocks
        for(size_t i = 0; i < NPC_Draw_Queue[Z].BG_n; i++)
        {
            A = NPC_Draw_Queue[Z].BG[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                {
                    {
                        {
                            if(NPC[A].Type == 8 || NPC[A].Type == 74 || NPC[A].Type == 93 || NPC[A].Type == 245 || NPC[A].Type == 256 || NPC[A].Type == 270)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type], cn, cn, cn);
                            }
                            else if(NPC[A].Type == 51 || NPC[A].Type == 257)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type],
                                        vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type],
                                        NPC[A].Location.Width, NPC[A].Location.Height,
                                        GFXNPC[NPC[A].Type], 0,
                                        NPC[A].Frame * NPCHeight[NPC[A].Type] + NPCHeight[NPC[A].Type] - NPC[A].Location.Height,
                                        cn, cn, cn);
                            }
                            else if(NPC[A].Type == 52)
                            {
                                if(NPC[A].Direction == -1)
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type]);
                                }
                                else
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], NPCWidth[NPC[A].Type] - NPC[A].Location.Width, NPC[A].Frame * NPCHeight[NPC[A].Type], cn, cn, cn);
                                }
                            }
                            else if(NPCWidthGFX[NPC[A].Type] == 0 || NPC[A].Effect == 1)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type], cn ,cn ,cn);
                            }
                            else
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type] - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                            }
                        }
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
                        if((vScreenCollision(Z, NPC[Player[A].HoldingNPC].Location) | vScreenCollision(Z, newLoc(NPC[Player[A].HoldingNPC].Location.X - (NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] - NPC[Player[A].HoldingNPC].Location.Width) / 2.0, NPC[Player[A].HoldingNPC].Location.Y, static_cast<double>(NPCWidthGFX[NPC[Player[A].HoldingNPC].Type]), static_cast<double>(NPCHeight[NPC[Player[A].HoldingNPC].Type])))) != 0 && NPC[Player[A].HoldingNPC].Hidden == false)
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
                            tempLocation.X = tempLocation.X + NPCFrameOffsetX[NPC[Player[A].HoldingNPC].Type];
                            tempLocation.Y = tempLocation.Y + NPCFrameOffsetY[NPC[Player[A].HoldingNPC].Type];
                            Y2 = 0;
                            X2 = 0;
                            NPCWarpGFX(A, tempLocation, X2, Y2);
                            if(NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] != 0 || NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] != 0)
                            {
                                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], X2, Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeightGFX[NPC[Player[A].HoldingNPC].Type]);
                            }
                            else
                            {
                                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], X2, Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeight[NPC[Player[A].HoldingNPC].Type]);
                            }

                        }
                    }


                    if(Player[A].Mount == 3)
                    {
                        B = Player[A].MountType;
                        // Yoshi's Body
                        tempLocation = Player[A].Location;
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X = tempLocation.X + Player[A].YoshiBX;
                        tempLocation.Y = tempLocation.Y + Player[A].YoshiBY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXYoshiB[B], X2, Y2 + 32 * Player[A].YoshiBFrame, c, c, c);
                        // Yoshi's Head
                        tempLocation = Player[A].Location;
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X = tempLocation.X + Player[A].YoshiTX;
                        tempLocation.Y = tempLocation.Y + Player[A].YoshiTY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXYoshiT[B], X2, Y2 + 32 * Player[A].YoshiTFrame);
                    }

                    if(Player[A].Character == 1)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = Player[A].Location;
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X = tempLocation.X + Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y = tempLocation.Y + Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot[Player[A].MountType], X2, Y2 + 32 * Player[A].MountFrame);
                        }
                        else
                        {
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 99;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                        }
                    }
                    else if(Player[A].Character == 2)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = Player[A].Location;
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X = tempLocation.X + Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y = tempLocation.Y + Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot[Player[A].MountType], X2, Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 99;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                        }
                    }
                    else if(Player[A].Character == 3)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = Player[A].Location;
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXPeach[Player[A].State],
                                                  pfrX(100 + Player[A].Frame * Player[A].Direction) + X2,
                                                  pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X = tempLocation.X + Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y = tempLocation.Y + Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFX.Boot[Player[A].MountType],
                                                  X2,
                                                  Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 99;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXPeach[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                        }
                    }
                    else if(Player[A].Character == 4)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = Player[A].Location;
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 26;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            if(Player[A].State == 1)
                                tempLocation.Y = tempLocation.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + 6;
                            else
                                tempLocation.Y = tempLocation.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 4;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X = tempLocation.X + Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y = tempLocation.Y + Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot[Player[A].MountType], X2, Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 99;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                        }
                    }
                    else if(Player[A].Character == 5)
                    {
                        if(Player[A].Frame > 5)
                            Player[A].Frame = 1;
                        tempLocation = Player[A].Location;
                        tempLocation.Height = 99;
                        tempLocation.Width = 99;
                        tempLocation.X = tempLocation.X + LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                        tempLocation.Y = tempLocation.Y + LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLink[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                    }
                }
                if(Player[A].HoldingNPC > 0 && Player[A].Frame == 15)
                {
                    if((vScreenCollision(Z, NPC[Player[A].HoldingNPC].Location) | vScreenCollision(Z, newLoc(NPC[Player[A].HoldingNPC].Location.X - (NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] - NPC[Player[A].HoldingNPC].Location.Width) / 2.0, NPC[Player[A].HoldingNPC].Location.Y, static_cast<double>(NPCWidthGFX[NPC[Player[A].HoldingNPC].Type]), static_cast<double>(NPCHeight[NPC[Player[A].HoldingNPC].Type])))) != 0 && NPC[Player[A].HoldingNPC].Hidden == false && NPC[Player[A].HoldingNPC].Type != 263)
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
                        tempLocation.X = tempLocation.X + NPCFrameOffsetX[NPC[Player[A].HoldingNPC].Type];
                        tempLocation.Y = tempLocation.Y + NPCFrameOffsetY[NPC[Player[A].HoldingNPC].Type];
                        Y2 = 0;
                        X2 = 0;
                        NPCWarpGFX(A, tempLocation, X2, Y2);
                        if(NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] != 0 || NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] != 0)
                        {
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], X2, Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeightGFX[NPC[Player[A].HoldingNPC].Type]);
                        }
                        else
                        {
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], X2, Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeight[NPC[Player[A].HoldingNPC].Type]);
                        }

                    }
                }
            }
        }


        if (LevelEditor)
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
                    frmMain.renderTexture(vScreenX[Z] + Block[A].Location.X - offX,
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
                    frmMain.renderTexture(vScreenX[Z] + Effect[A].Location.X, vScreenY[Z] + Effect[A].Location.Y, Effect[A].Location.Width, Effect[A].Location.Height, GFXEffect[Effect[A].Type], 0, Effect[A].Frame * EffectHeight[Effect[A].Type], cn, cn, cn);
                }
            }
        }

        // draw NPCs that should be behind other NPCs
        for(size_t i = 0; i < NPC_Draw_Queue[Z].Low_n; i++)
        {
            A = NPC_Draw_Queue[Z].Low[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                {
                    // If Not NPCIsACoin(.Type) Then
                    {
                        {
                            if(NPCWidthGFX[NPC[A].Type] == 0)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                            }
                            else
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                            }
                        }
                    }
                }
            }
        }


        // ice
        for(size_t i = 0; i < NPC_Draw_Queue[Z].Iced_n; i++)
        {
            A = NPC_Draw_Queue[Z].Iced[i];
            {
                {
                    DrawFrozenNPC(Z, A);
                }
            }
        }


//        For A = 1 To numNPCs 'Display NPCs that should be in front of blocks
        for(size_t i = 0; i < NPC_Draw_Queue[Z].Normal_n; i++)
        {
            A = NPC_Draw_Queue[Z].Normal[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                {
                    {
                        {
                            {
                                if(!NPCIsYoshi[NPC[A].Type])
                                {
                                    if(NPCWidthGFX[NPC[A].Type] == 0)
                                    {
                                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
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
                                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[A].Special], 0, B * tempLocation.Height);
                                        }

                                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
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
                                            NPC[A].FrameCount = NPC[A].FrameCount + 1;
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
                                            NPC[A].FrameCount = NPC[A].FrameCount + 1;
                                        if(NPC[A].FrameCount > 8)
                                        {
                                            YoshiBFrame = 0;
                                            NPC[A].FrameCount = 0;
                                        }
                                        else if(NPC[A].FrameCount > 6)
                                        {
                                            YoshiBFrame = 1;
                                            YoshiTX = YoshiTX - 1;
                                            YoshiTY = YoshiTY + 2;
                                            YoshiBY = YoshiBY + 1;
                                        }
                                        else if(NPC[A].FrameCount > 4)
                                        {
                                            YoshiBFrame = 2;
                                            YoshiTX = YoshiTX - 2;
                                            YoshiTY = YoshiTY + 4;
                                            YoshiBY = YoshiBY + 2;
                                        }
                                        else if(NPC[A].FrameCount > 2)
                                        {
                                            YoshiBFrame = 1;
                                            YoshiTX = YoshiTX - 1;
                                            YoshiTY = YoshiTY + 2;
                                            YoshiBY = YoshiBY + 1;
                                        }
                                        else
                                            YoshiBFrame = 0;
                                        if(!FreezeNPCs)
                                            NPC[A].Special2 = NPC[A].Special2 + 1;
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
                                        YoshiBY = YoshiBY + 10;
                                        YoshiTY = YoshiTY + 10;
                                    }
                                    if(NPC[A].Direction == 1)
                                    {
                                        YoshiTFrame = YoshiTFrame + 5;
                                        YoshiBFrame = YoshiBFrame + 7;
                                    }
                                    else
                                    {
                                        YoshiBX = -YoshiBX;
                                        YoshiTX = -YoshiTX;
                                    }
                                    // YoshiBX = YoshiBX + 4
                                    // YoshiTX = YoshiTX + 4
                                    // Yoshi's Body
                                    frmMain.renderTexture(vScreenX[Z] + SDL_floor(NPC[A].Location.X) + YoshiBX, vScreenY[Z] + NPC[A].Location.Y + YoshiBY, 32, 32, GFXYoshiB[B], 0, 32 * YoshiBFrame, cn, cn, cn);

                                    // Yoshi's Head
                                    frmMain.renderTexture(vScreenX[Z] + SDL_floor(NPC[A].Location.X) + YoshiTX, vScreenY[Z] + NPC[A].Location.Y + YoshiTY, 32, 32, GFXYoshiT[B], 0, 32 * YoshiTFrame, cn, cn, cn);
                                }
                            }
                        }
                    }
                }
            }
//        Next A
        }

        // npc chat bubble
        for(size_t i = 0; i < NPC_Draw_Queue[Z].Chat_n; i++)
        {
            A = NPC_Draw_Queue[Z].Chat[i];
            {
                B = NPCHeightGFX[NPC[A].Type] - NPC[A].Location.Height;
                if(B < 0)
                    B = 0;
                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPC[A].Location.Width / 2.0 - GFX.Chat.w / 2, vScreenY[Z] + NPC[A].Location.Y - 30 - B, GFX.Chat.w, GFX.Chat.h, GFX.Chat, 0, 0);
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

                frmMain.renderTexture(
                        vScreenX[Z] + SDL_floor(Player[A].Location.X) + frameX - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64,
                        vScreenY[Z] + Player[A].Location.Y + frameY + Player[A].MountOffsetY - Y,
                        99,
                        Player[A].Location.Height - 20 - Player[A].MountOffsetY,
                        *playerGfx,
                        pfrX(100 + Player[A].Frame * Player[A].Direction),
                        pfrY(100 + Player[A].Frame * Player[A].Direction),
                        c, c, c);
                frmMain.renderTexture(
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


        // Put held NPCs on top
        for(size_t i = 0; i < NPC_Draw_Queue[Z].Held_n; i++)
        {
            A = NPC_Draw_Queue[Z].Held[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                if(NPC[A].Type == 263)
                {
                    DrawFrozenNPC(Z, A);
                }
                else if(!NPCIsYoshi[NPC[A].Type] && NPC[A].Type > 0)
                {
                    if(NPCWidthGFX[NPC[A].Type] == 0)
                    {
                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                    }
                }
            }
        }



//'normal player draw code
        for(int A = numPlayers; A >= 1; A--)// Players in front of blocks
        {
            DrawPlayer(A, Z);
        }
//'normal player end




        if (LevelEditor)
        {
            For(A, 1, numBackground)
            {
                if(Foreground[Background[A].Type])
                {
                    g_stats.checkedBGOs++;
                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                    {
                        g_stats.renderedBGOs++;
                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
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
                    frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, GFXBackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackground[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
        }

        // foreground NPCs
        for(size_t i = 0; i < NPC_Draw_Queue[Z].FG_n; i++)
        {
            A = NPC_Draw_Queue[Z].FG[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                {
                    {
                        {
                            {
                                if(NPCWidthGFX[NPC[A].Type] == 0)
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                                }
                                else
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                                }
                            }
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
                    frmMain.renderTexture(vScreenX[Z] + Block[A].Location.X - offX,
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
            auto &e = Effect[A];
            if(e.Type != 112 && e.Type != 54 && e.Type != 55 && e.Type != 59 &&
               e.Type != 77 && e.Type != 81 && e.Type != 82 && e.Type != 103 &&
               e.Type != 104 && e.Type != 114 && e.Type != 123 && e.Type != 124)
            {
                if(vScreenCollision(Z, e.Location))
                {
                    g_stats.renderedEffects++;
                    float c = e.Shadow ? 0.f : 1.f;
                    frmMain.renderTexture(int(vScreenX[Z] + e.Location.X), int(vScreenY[Z] + e.Location.Y),
                                          int(e.Location.Width), int(e.Location.Height),
                                          GFXEffectBMP[e.Type], 0, e.Frame * EffectHeight[e.Type], c, c, c);
                }
            }
        }

        // water
        if(LevelEditor)
        {
            for(B = 1; B <= numWater; B++)
            {
                if(!Water[B].Hidden && vScreenCollision(Z, Water[B].Location))
                {
                    if (Water[B].Quicksand)
                        frmMain.renderRect(vScreenX[Z] + Water[B].Location.X, vScreenY[Z] + Water[B].Location.Y, Water[B].Location.Width, Water[B].Location.Height,
                            1.f, 1.f, 0.f, 1.f, false);
                    else
                        frmMain.renderRect(vScreenX[Z] + Water[B].Location.X, vScreenY[Z] + Water[B].Location.Y, Water[B].Location.Width, Water[B].Location.Height,
                            0.f, 1.f, 1.f, 1.f, false);
                }
            }
        }

#ifdef __3DS
        frmMain.setLayer(3);
#endif


        B = 0;
        C = 0;
        // per-screen interface
        if(!GameMenu && !GameOutro && !LevelEditor)
        {
            For(A, 1, numPlayers)
            {
                if(Player[A].ShowWarp > 0)
                {
                    std::string tempString;
                    auto &w = Warp[Player[A].ShowWarp];
                    if(!w.noPrintStars && w.maxStars > 0 && Player[A].Mount != 2)
                    {
                        tempString = fmt::format_ne("{0}/{1}", w.curStars, w.maxStars);
                        frmMain.renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z] + 1, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                        frmMain.renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 - 20, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z], GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                        SuperPrint(tempString, 3,
                                   float(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 + 18),
                                   float(Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z]));
                    }
                }
            }

            DrawInterface(Z, numScreens);

            // Display NPCs that got dropped from the container
            for(size_t i = 0; i < NPC_Draw_Queue[Z].Dropped_n; i++)
            {
                A = NPC_Draw_Queue[Z].Dropped[i];
                {
                    {
                        {
                            {
                                if(NPCWidthGFX[NPC[A].Type] == 0)
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height);
                                }
                                else
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type] - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type]);
                                }
                            }
                        }
                    }
                }
            }
        }

        if(LevelEditor || (MagicHand && !GamePaused))
        {
            DrawEditorLevel(Z);
        }

    }
    frmMain.setViewport(0, 0, ScreenW, ScreenH);
    // splitscreen divider
    if(vScreen[2].Visible)
    {
        if(DScreenType == 3 || DScreenType == 4 || DScreenType == 6)
            frmMain.renderRect(0, ScreenH/2-2, ScreenW, 4, 0, 0, 0);
        else
            frmMain.renderRect(ScreenW/2-2, 0, 4, ScreenH, 0, 0, 0);
    }

    // pause menu and message interface
    if(!GameMenu && !GameOutro && !LevelEditor && GamePaused)
    {
        if(MessageText.empty())
        {
            frmMain.renderRect(ScreenW/2 - 190, ScreenH/2 - 100, 380, 200, 0, 0, 0);
            if(TestLevel)
            {
                SuperPrint("CONTINUE", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 37);
                SuperPrint("RESTART LEVEL", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 72);
                SuperPrint("RESET CHECKPOINTS", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 107);
                SuperPrint("QUIT TESTING", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 142);
                frmMain.renderTexture(ScreenW/2 - 190 + 42, ScreenH/2 - 100 + 37 + (MenuCursor * 35), 16, 16, GFX.MCursor[0], 0, 0);
            }
            else if(!Cheater && (LevelSelect || (/*StartLevel == FileName*/IsEpisodeIntro && NoMap)))
            {
                SuperPrint("CONTINUE", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 57);
                SuperPrint("SAVE & CONTINUE", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 92);
                SuperPrint("SAVE & QUIT", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 127);
                frmMain.renderTexture(ScreenW/2 - 190 + 42, ScreenH/2 - 100 + 57 + (MenuCursor * 35), 16, 16, GFX.MCursor[0], 0, 0);
            }
            else
            {
                SuperPrint("CONTINUE", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 75);
                SuperPrint("QUIT", 3, ScreenW/2 - 190 + 62, ScreenH/2 - 100 + 110);
                frmMain.renderTexture(ScreenW/2 - 190 + 42, ScreenH/2 - 100 + 75 + (MenuCursor * 35), 16, 16, GFX.MCursor[0], 0, 0);
            }
        }
        else
        {
            DrawMessage();
        }
    }

    if(GameMenu && !GameOutro)
        mainMenuDraw();


    if(PrintFPS > 0)
        SuperPrint(fmt::format_ne("{0}", int(PrintFPS)), 1, 8, 8, 0.f, 1.f, 0.f);
    g_stats.print();

    speedRun_render();

    if(GameOutro)
        DrawCredits();

    s_shakeScreen.update();

    if(!skipRepaint)
        frmMain.repaint();

    frmMain.setTargetScreen();

    if(TakeScreen)
        ScreenShot();

    // Update Coin Frames
    CoinFrame2[1] = CoinFrame2[1] + 1;
    if(CoinFrame2[1] >= 6)
    {
        CoinFrame2[1] = 0;
        CoinFrame[1] = CoinFrame[1] + 1;
        if(CoinFrame[1] >= 4)
            CoinFrame[1] = 0;
    }
    CoinFrame2[2] = CoinFrame2[2] + 1;
    if(CoinFrame2[2] >= 6)
    {
        CoinFrame2[2] = 0;
        CoinFrame[2] = CoinFrame[2] + 1;
        if(CoinFrame[2] >= 7)
            CoinFrame[2] = 0;
    }
    CoinFrame2[3] = CoinFrame2[3] + 1;
    if(CoinFrame2[3] >= 7)
    {
        CoinFrame2[3] = 0;
        CoinFrame[3] = CoinFrame[3] + 1;
        if(CoinFrame[3] >= 4)
            CoinFrame[3] = 0;
    }
//    if(nPlay.Mode == 0)
//    {
//        if(nPlay.NPCWaitCount >= 11)
//            nPlay.NPCWaitCount = 0;
//        nPlay.NPCWaitCount = nPlay.NPCWaitCount + 2;
//        if(timeStr != "")
//            Netplay::sendData timeStr + LB;
//    }

    frameRenderEnd();

//    if(frmMain.lazyLoadedBytes() > 200000) // Reset timer while loading many pictures at the same time
//        resetFrameTimer();
    frmMain.lazyLoadedBytesReset();
}
