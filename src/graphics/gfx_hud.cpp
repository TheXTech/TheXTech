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

#include <fmt_format_ne.h>
#include "sdl_proxy/sdl_stdinc.h"

#include "../globals.h"
#include "../graphics.h"
#include "../core/render.h"
#include "../core/power.h"
#include "../gfx.h"

#include "video.h"
#include "main/speedrunner.h"

#include "config.h"
#include "main/level_medals.h"

void DrawInterface(int Z, int numScreens)
{
    int ScreenTop = 0;
    if(vScreen[Z].Height > 600)
        ScreenTop = vScreen[Z].Height / 2 - 300;
    int CenterX = vScreen[Z].Width / 2;

    int B = 0;
    int C = 0;
    int D = 0;

    std::string scoreStr = std::to_string(Score);
    std::string coinsStr = std::to_string(Coins);
    std::string livesStr = std::to_string(int(Lives));
    std::string numStarsStr = std::to_string(numStars);

    XRender::offsetViewportIgnore(true);

    if(ScreenType == 5 || ScreenType == 6) // 2 Players
    {
        if(static_cast<int>(numScreens) == 1 && ScreenType != 6) // Only 1 screen
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(B == 1)
                    C = -40;
                else
                    C = 40;

                if(Player[B].Character == 3 || Player[B].Character == 4 || Player[B].Character == 5)
                {
                    D = (B == 1) ? -1 : 1;

                    int hGfx;

                    hGfx = Player[B].Hearts > 0 ? 1 : 2;
                    XRender::renderTexture(CenterX - GFX.Heart[hGfx].w / 2 + C - 32 + 17 * D,
                                            ScreenTop + 16,
                                            GFX.Heart[hGfx].w, GFX.Heart[hGfx].h,
                                            GFX.Heart[hGfx], 0, 0);

                    hGfx = Player[B].Hearts > 1 ? 1 : 2;
                    XRender::renderTexture(CenterX - GFX.Heart[hGfx].w / 2 + C + 17 * D,
                                            ScreenTop + 16,
                                            GFX.Heart[hGfx].w, GFX.Heart[hGfx].h,
                                            GFX.Heart[hGfx], 0, 0);

                    hGfx = Player[B].Hearts > 2 ? 1 : 2;
                    XRender::renderTexture(CenterX - GFX.Heart[hGfx].w / 2 + C + 32 + 17 * D,
                                            ScreenTop + 16,
                                            GFX.Heart[hGfx].w, GFX.Heart[hGfx].h,
                                            GFX.Heart[hGfx], 0, 0);
                }
                else
                {
                    // 2 players 1 screen heldbonus
                    XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C, ScreenTop + 16, GFX.Container[1].w, GFX.Container[1].h, GFX.Container[Player[B].Character], 0, 0);

                    if(Player[B].HeldBonus > 0)
                    {
                        XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C + 12,
                                                ScreenTop + 16 + 12,
                                                NPCWidth[Player[B].HeldBonus], NPCHeight[Player[B].HeldBonus],
                                                GFXNPC[Player[B].HeldBonus], 0, 0);
                    }
                }
            }

            for(B = 1; B <= 2; B++)
            {
                C = (B == 1) ? -58 : 56;

                if(Player[B].Character == 5 && Player[B].Bombs > 0)
                {
                    XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 - 34 + C, ScreenTop + 52,
                        GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[8], 0, 0);
                    XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 - 10 + C, ScreenTop + 53,
                        GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(std::to_string(Player[B].Bombs), 1,
                               float(20 + CenterX - GFX.Container[1].w / 2 + 12 + C),
                               ScreenTop + 53);
                }
            }

            C = 40;

            if(!BattleMode)
            {
                // Print coins on the screen
                if(Player[1].HasKey || Player[2].HasKey)
                {
                    XRender::renderTexture(-24 + 40 + 20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[0], 0, 0);
                }

                if(Player[1].Character == 5)
                {
                    XRender::renderTexture(40 + 20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
                }
                else
                {
                    XRender::renderTexture(40 + 20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
                }

                XRender::renderTexture(40 + 20 + CenterX - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, ScreenTop + 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

                SuperPrint(coinsStr, 1,
                           40 + 20 - (int(coinsStr.size()) * 18) +
                           (CenterX) - (GFX.Container[1].w / 2) + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w,
                           ScreenTop + 16 + 11);
                // Print Score
                SuperPrint(scoreStr, 1,
                           40 + 20 - (int(scoreStr.size()) * 18) +
                           (CenterX) - (GFX.Container[1].w / 2) + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w,
                           ScreenTop + 16 + 31);

                // Print lives on the screen
                XRender::renderTexture(-80 + CenterX - GFX.Container[1].w / 2 + C - 122 - 16, ScreenTop + 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                XRender::renderTexture(-80 + CenterX - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, ScreenTop + 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

                SuperPrint(livesStr, 1,
                           float(-80 + (CenterX) - (GFX.Container[1].w / 2) + C - 122 + 12 + 18 + GFX.Interface[5].w),
                           ScreenTop + 16 + 11);
                // Print stars on the screen
                if(numStars > 0)
                {
                    XRender::renderTexture(-80 + CenterX - GFX.Container[1].w / 2 + C - 122, ScreenTop + 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                    XRender::renderTexture(-80 + CenterX - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, ScreenTop + 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(numStarsStr, 1,
                               float(-80 + (CenterX) - (GFX.Container[1].w / 2) + C - 122 + 12 + 18 + GFX.Interface[5].w),
                               ScreenTop + 16 + 31);
                }
            }
            else
            {
                // plr 1 score
                XRender::renderTexture(-80 + CenterX - GFX.Container[1].w / 2 + C - 122 - 16, ScreenTop + 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                XRender::renderTexture(-80 + CenterX - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, ScreenTop + 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(BattleLives[1]), 1,
                           float(-80 + CenterX - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                           ScreenTop + 16 + 11);

                // plr 2 score
                XRender::renderTexture(40 + 20 + CenterX - GFX.Container[1].w / 2 + 96 - 16, ScreenTop + 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[7], 0, 0);
                XRender::renderTexture(40 + 20 + CenterX - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, ScreenTop + 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(BattleLives[2]), 1,
                           float(24 + CenterX - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w),
                           ScreenTop + 16 + 11);
            }
        }
        else // Split screen
        {


// 2 players 2 screen heldbonus

            if(Player[Z].Character == 3 || Player[Z].Character == 4 || Player[Z].Character == 5)
            {
                auto& heart_1_gfx = Player[Z].Hearts > 0 ? GFX.Heart[1] : GFX.Heart[2];
                auto& heart_2_gfx = Player[Z].Hearts > 1 ? GFX.Heart[1] : GFX.Heart[2];
                auto& heart_3_gfx = Player[Z].Hearts > 2 ? GFX.Heart[1] : GFX.Heart[2];

                XRender::renderTexture(CenterX - heart_1_gfx.w / 2 + C - 32, ScreenTop + 16, heart_1_gfx);
                XRender::renderTexture(CenterX - heart_2_gfx.w / 2 + C     , ScreenTop + 16, heart_2_gfx);
                XRender::renderTexture(CenterX - heart_3_gfx.w / 2 + C + 32, ScreenTop + 16, heart_3_gfx);
            }
            else
            {

                XRender::renderTexture(CenterX - GFX.Container[1].w / 2, ScreenTop + 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX.Container[Player[Z].Character], 0, 0);
                if(Player[Z].HeldBonus > 0)
                {
                    XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + 12,
                                            ScreenTop + 16 + 12,
                                            NPCWidth[Player[Z].HeldBonus], NPCHeight[Player[Z].HeldBonus],
                                            GFXNPC[Player[Z].HeldBonus], 0, 0);
                }
            }

            if(Player[Z].Character == 5 && Player[Z].Bombs > 0)
            {
                XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 - 34, ScreenTop + 52, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[8], 0, 0);
                XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 - 10, ScreenTop + 53, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(Player[Z].Bombs), 1, 20 + CenterX - GFX.Container[1].w / 2 + 12, ScreenTop + 53);
            }

            if(!BattleMode)
            {
                // Print coins on the screen
                if(Player[Z].HasKey)
                    XRender::renderTexture(-24 + 20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[0], 0, 0);

                if(Player[Z].Character == 5)
                    XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
                else
                    XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);


                XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, ScreenTop + 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(coinsStr, 1,
                           float(20 - int(coinsStr.size()) * 18 + CenterX - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w),
                           ScreenTop + 16 + 11);

                // Print Score
                SuperPrint(scoreStr,
                           1,
                           float(20 - int(scoreStr.size()) * 18 + CenterX - GFX.Container[1].w / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w),
                           ScreenTop + 16 + 31);

                // Print lives on the screen

                XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122 - 16, ScreenTop + 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, ScreenTop + 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(livesStr, 1,
                           float(CenterX - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                           ScreenTop + 16 + 11);
                // Print stars on the screen
                if(numStars > 0)
                {
                    XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122, ScreenTop + 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                    XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, ScreenTop + 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(numStarsStr, 1,
                               float(CenterX - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                               ScreenTop + 16 + 31);
                }
            }
            else
            {
                // lives in battle mode

                auto& oneup_twoup = (Z == 1) ? GFX.Interface[3] : GFX.Interface[7];

                XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122,
                                      ScreenTop + 16 + 10,
                                      oneup_twoup.w, oneup_twoup.h,
                                      oneup_twoup, 0, 0);

                XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w + 16,
                    ScreenTop + 16 + 11,
                    GFX.Interface[1].w, GFX.Interface[1].h,
                    GFX.Interface[1], 0, 0);

                SuperPrint(std::to_string(BattleLives[Z]), 1,
                           float(CenterX - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w + 16),
                           ScreenTop + 16 + 11);
            }
        }
    }
    else // 1 Player or Multi Mario
    {


//        if(nPlay.Online == false)
        {
            if(Player[1].Character == 3 || Player[1].Character == 4 || Player[1].Character == 5)
            {
//                BitBlt myBackBuffer, CenterX - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, CenterX - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, CenterX - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
                auto& heart_1_gfx = Player[1].Hearts > 0 ? GFX.Heart[1] : GFX.Heart[2];
                auto& heart_2_gfx = Player[1].Hearts > 1 ? GFX.Heart[1] : GFX.Heart[2];
                auto& heart_3_gfx = Player[1].Hearts > 2 ? GFX.Heart[1] : GFX.Heart[2];

                XRender::renderTexture(CenterX - heart_1_gfx.w / 2 + C - 32, ScreenTop + 16, heart_1_gfx);
                XRender::renderTexture(CenterX - heart_2_gfx.w / 2 + C     , ScreenTop + 16, heart_2_gfx);
                XRender::renderTexture(CenterX - heart_3_gfx.w / 2 + C + 32, ScreenTop + 16, heart_3_gfx);
            }
            else
            {
                XRender::renderTexture(CenterX - GFX.Container[1].w / 2, ScreenTop + 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX.Container[0], 0, 0);

                if(Player[1].HeldBonus > 0)
                    XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + 12, ScreenTop + 16 + 12, NPCWidth[Player[1].HeldBonus], NPCHeight[Player[1].HeldBonus], GFXNPC[Player[1].HeldBonus], 0, 0);
            }
        }
//        else
//        {
//            if(Player[nPlay.MySlot + 1].Character == 3 || Player[nPlay.MySlot + 1].Character == 4)
//            {
////                BitBlt myBackBuffer, CenterX - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
////                BitBlt myBackBuffer, CenterX - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
////                BitBlt myBackBuffer, CenterX - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                if(Player[nPlay.MySlot + 1].Hearts > 0)
//                {
//                    XRender::renderTexture(CenterX - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    XRender::renderTexture(CenterX - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//                if(Player[nPlay.MySlot + 1].Hearts > 1)
//                {
//                    XRender::renderTexture(CenterX - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    XRender::renderTexture(CenterX - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//                if(Player[nPlay.MySlot + 1].Hearts > 2)
//                {
//                    XRender::renderTexture(CenterX - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    XRender::renderTexture(CenterX - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//            }
//            else
//            {
//                BitBlt myBackBuffer, CenterX - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX::ContainerMask(0).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, CenterX - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX::Container(0).hdc, 0, 0, vbSrcPaint;
//                if(Player[nPlay.MySlot + 1].HeldBonus > 0)
//                {
//                    XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[nPlay.MySlot + 1].HeldBonus], NPCHeight[Player[nPlay.MySlot + 1].HeldBonus], GFXNPCMask[Player[nPlay.MySlot + 1].HeldBonus], 0, 0);
//                    XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[nPlay.MySlot + 1].HeldBonus], NPCHeight[Player[nPlay.MySlot + 1].HeldBonus], GFXNPC[Player[nPlay.MySlot + 1].HeldBonus], 0, 0);
//                }
//            }
//        }

        if(Player[1].Character == 5 && Player[1].Bombs > 0)
        {
            XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 - 34 + C, ScreenTop + 52, GFX.Interface[8]);
            XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 - 10 + C, ScreenTop + 53, GFX.Interface[1]);
            SuperPrint(std::to_string(Player[1].Bombs), 1,
                       float(20 + CenterX - GFX.Container[1].w / 2 + 12 + C), ScreenTop + 53);
        }

        // Print coins on the screen
        if(Player[1].HasKey)
            XRender::renderTexture(-24 + 20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[0]);

        if(Player[1].Character == 5)
            XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[6]);
        else
            XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 + 96, ScreenTop + 16 + 10, GFX.Interface[2]);

        XRender::renderTexture(20 + CenterX - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, ScreenTop + 16 + 11, GFX.Interface[1]);

        SuperPrint(coinsStr, 1,
                   float(20 - int(coinsStr.size()) * 18 + CenterX - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w),
                   ScreenTop + 16 + 11);

        // Print Score
        SuperPrint(scoreStr, 1,
                   20 - int(scoreStr.size()) * 18 +
                   CenterX - GFX.Container[1].w / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w,
                   ScreenTop + 16 + 31);

        // Print lives on the screen
        XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122 - 16, ScreenTop + 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
        XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, ScreenTop + 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
        SuperPrint(livesStr, 1,
                   float(CenterX - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                   ScreenTop + 16 + 11);

        // Print stars on the screen
        if(numStars > 0)
        {
            XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122, ScreenTop + 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
            XRender::renderTexture(CenterX - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, ScreenTop + 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
            SuperPrint(numStarsStr, 1,
                       float(CenterX - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                       ScreenTop + 16 + 31);
        }
    }

    if(!InHub() && !BattleMode)
    {
        // draw medals at top-right side of HUD
        int MedalsX = SDL_min(static_cast<int>(vScreen[Z].Width) - 16, CenterX + 400 - 16);
        int MedalsY = ScreenTop + 16;

        DrawMedals(MedalsX, MedalsY, false, g_curLevelMedals.max, g_curLevelMedals.prev, 0, g_curLevelMedals.got, g_curLevelMedals.life);
    }

    if(BattleIntro > 0)
    {
        if(BattleIntro > 45 || BattleIntro % 2 == 1)
        {
            XRender::renderTexture(vScreen[Z].Width / 2.0 - GFX.BMVs.w / 2, -96 + vScreen[Z].Height / 2.0 - GFX.BMVs.h / 2, GFX.BMVs.w, GFX.BMVs.h, GFX.BMVs, 0, 0);
            XRender::renderTexture(-50 + vScreen[Z].Width / 2.0 - GFX.CharacterName[Player[1].Character].w, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[1].Character].h / 2, GFX.CharacterName[Player[1].Character].w, GFX.CharacterName[Player[1].Character].h, GFX.CharacterName[Player[1].Character], 0, 0);
            XRender::renderTexture(50 + vScreen[Z].Width / 2.0, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[2].Character].h / 2, GFX.CharacterName[Player[2].Character].w, GFX.CharacterName[Player[2].Character].h, GFX.CharacterName[Player[2].Character], 0, 0);
        }
    }

    if(BattleOutro > 0)
    {
        XRender::renderTexture(10 + vScreen[Z].Width / 2.0, -96 + vScreen[Z].Height / 2.0 - GFX.BMWin.h / 2, GFX.BMWin.w, GFX.BMWin.h, GFX.BMWin, 0, 0);
        XRender::renderTexture(-10 + vScreen[Z].Width / 2.0 - GFX.CharacterName[Player[BattleWinner].Character].w, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[BattleWinner].Character].h / 2, GFX.CharacterName[Player[BattleWinner].Character].w, GFX.CharacterName[Player[BattleWinner].Character].h, GFX.CharacterName[Player[BattleWinner].Character], 0, 0);
    }

    XRender::offsetViewportIgnore(false);
}

enum class MedalDrawLevel
{
    Off = 0, Prev, Got, Shiny
};

//! helper function to draw a single medal at a specific top-left position and acquisition level
static inline void s_DrawMedal(int x, int y, int coin_width, int coin_height, MedalDrawLevel level)
{
    if(GFX.Medals.inited)
    {
        XRender::renderTexture(x, y, coin_width, coin_height, GFX.Medals, coin_width * (int)level, 0);
    }
    else
    {
        if(level == MedalDrawLevel::Shiny || level == MedalDrawLevel::Got)
            XRender::renderTexture(x, y, GFX.Interface[2]);
        else if(level == MedalDrawLevel::Prev)
            XRender::renderTexture(x, y, GFX.Interface[2], 0.5f, 0.5f, 0.5f);
        else
            XRender::renderTexture(x, y, GFX.Interface[2], 0.5f, 0.5f, 0.5f, 0.5f);
    }

    // render sparkles for shiny
    if(level == MedalDrawLevel::Shiny)
    {
        int sparkle_1_idx = ((CommonFrame + x * 37) % 1024) / 16; // on frame 3

        for(int i = 0; i < 3; ++i)
        {
            int sparkle_idx = (sparkle_1_idx + i) % 64;

            if(sparkle_idx % 2)
                continue;

            int sparkle_frame = 2 - i;

            int sparkle_X = (9 * sparkle_idx) % (coin_width - 8) + 4;
            int sparkle_Y = (13 * (sparkle_idx % 16) + 29 * (sparkle_idx / 16)) % (coin_height - 8) + 4;

            sparkle_X -= EffectWidth[78] / 2;
            sparkle_Y -= EffectHeight[78] / 2;

            sparkle_X &= ~1;
            sparkle_Y &= ~1;

            XRender::renderTexture(x + sparkle_X, y + sparkle_Y, EffectWidth[78], EffectHeight[78], GFXEffect[78], 0, EffectHeight[78] * sparkle_frame, 1.0f, 1.0f, 1.0f, 0.8f);
        }
    }
}

void DrawMedals(int X, int Y, bool warp, uint8_t max, uint8_t prev, uint8_t ckpt, uint8_t got, uint8_t best)
{
    if(g_config.medals_show_policy == Config_t::MEDALS_SHOW_OFF)
        return;

    if(g_config.medals_show_policy == Config_t::MEDALS_SHOW_GOT && got == 0)
        return;

    if(max == 0)
        return;

    int coin_width = GFX.Interface[2].w;
    int coin_height = GFX.Interface[2].h;

    if(GFX.Medals.inited)
    {
        coin_width = GFX.Medals.w / 4;
        coin_height = GFX.Medals.h;
    }

    if(max > c_max_track_medals)
        max = c_max_track_medals;

    // don't spoil the maximum count, make it shiny if all of the discovered medals are shiny
    bool show_max = g_config.medals_show_policy >= Config_t::MEDALS_SHOW_COUNTS;

    // whether to use the shiny effect for medals; for warps, check if best is all 1s (up to bit max); in HUD, always draw shiny medals
    bool show_shiny = (warp && show_max) ? (best == ((1 << max) - 1)) : true;

    // slot-based display
    if(g_config.medals_show_policy == Config_t::MEDALS_SHOW_FULL)
    {
        // position scene
        if(warp)
            X -= ((coin_width * max) / 2) & ~1;
        else
            X -= (coin_width * max);

        // draw coins
        for(int i = 0; i < max; ++i)
        {
            int bit = (1 << i);
            int X_i = X + coin_width * i;

            if((best & bit) && show_shiny)
                s_DrawMedal(X_i, Y, coin_width, coin_height, MedalDrawLevel::Shiny);
            else if(got & bit)
                s_DrawMedal(X_i, Y, coin_width, coin_height, MedalDrawLevel::Got);
            else if(ckpt & bit && (CommonFrame % 64) < 32)
                s_DrawMedal(X_i, Y, coin_width, coin_height, MedalDrawLevel::Got);
            else if(prev & bit)
                s_DrawMedal(X_i, Y, coin_width, coin_height, MedalDrawLevel::Prev);
            else
                s_DrawMedal(X_i, Y, coin_width, coin_height, MedalDrawLevel::Off);
        }

        return;
    }

    // text-based display

    // get counts
    int best_count = 0;
    int got_count = 0;
    int prev_count = 0;

    for(int i = 0; i < max; ++i)
    {
        int bit = (1 << i);

        if(best & bit)
            best_count++;
        if(got & bit)
            got_count++;
        if(prev & bit)
            prev_count++;
    }

    // make labels and construct / position scene
    std::string label;
    int total_len = 0;

    if(g_config.medals_show_policy == Config_t::MEDALS_SHOW_COUNTS)
        label = fmt::format_ne("{0}/{1}", got_count, max);
    else
        label = fmt::format_ne("{0}", got_count);

    total_len += coin_width + 8 + GFX.Interface[1].w + 4;
    total_len += SuperTextPixLen(label, 3);

    if(warp)
        X -= (total_len / 2) & ~1;
    else
        X -= total_len;

    // draw scene
    s_DrawMedal(X, Y, coin_width, coin_height, show_shiny ? MedalDrawLevel::Shiny : MedalDrawLevel::Got);
    X += coin_width + 8;
    XRender::renderTexture(X, Y, GFX.Interface[1]);
    X += GFX.Interface[1].w + 4;
    SuperPrint(label, 3, X, Y);
}

void DrawDeviceBattery()
{
#ifdef RENDER_FULLSCREEN_ALWAYS
    constexpr bool isFullScreen = true;
#else
    const bool isFullScreen = resChanged;
#endif

    if(g_videoSettings.batteryStatus == BATTERY_STATUS_OFF)
        return;

    XPower::StatusInfo status_info = XPower::devicePowerStatus();

    if(status_info.power_status == XPower::StatusInfo::POWER_DISABLED || status_info.power_status == XPower::StatusInfo::POWER_UNKNOWN || status_info.power_status == XPower::StatusInfo::POWER_WIRED)
        return;

    bool isLow = (status_info.power_status <= 0.35f);

    bool showBattery = false;

    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_ALWAYS_ON);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_ANY_WHEN_LOW && isLow);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_WHEN_LOW && isLow && isFullScreen);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_ON && isFullScreen);

    if(showBattery)
    {
        int bw = 40;
        int bh = 22;
        int bx = ScreenW - (bw + 8);
        int by = 24;

        RenderPowerInfo(0, bx, by, bw, bh, 1.0f, &status_info);
    }
}
