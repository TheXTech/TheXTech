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

#include "../globals.h"
#include "../graphics.h"
#include "../core/render.h"


void DrawInterface(int Z, int numScreens)
{
    int B = 0;
    int C = 0;
    int D = 0;

    std::string scoreStr = std::to_string(Score);
    std::string coinsStr = std::to_string(Coins);
    std::string livesStr = std::to_string(int(Lives));
    std::string numStarsStr = std::to_string(numStars);

    g_render->offsetViewportIgnore(true);

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
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[hGfx].w / 2 + C - 32 + 17 * D, 16,
                                            GFX.Heart[hGfx].w, GFX.Heart[hGfx].h,
                                            GFX.Heart[hGfx], 0, 0);

                    hGfx = Player[B].Hearts > 1 ? 1 : 2;
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[hGfx].w / 2 + C + 17 * D, 16,
                                            GFX.Heart[hGfx].w, GFX.Heart[hGfx].h,
                                            GFX.Heart[hGfx], 0, 0);

                    hGfx = Player[B].Hearts > 2 ? 1 : 2;
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[hGfx].w / 2 + C + 32 + 17 * D, 16,
                                            GFX.Heart[hGfx].w, GFX.Heart[hGfx].h,
                                            GFX.Heart[hGfx], 0, 0);
                }
                else
                {
                    // 2 players 1 screen heldbonus
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C, 16, GFX.Container[1].w, GFX.Container[1].h, GFX.Container[Player[B].Character], 0, 0);
                    if(Player[B].HeldBonus > 0)
                    {
                        g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C + 12, 16 + 12,
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
                    g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 34 + C, 52, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[8], 0, 0);
                    g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 10 + C, 53, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(std::to_string(Player[B].Bombs), 1,
                               float(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12 + C),
                               53);
                }
            }

            C = 40;

            if(!BattleMode)
            {
                // Print coins on the screen
                if(Player[1].HasKey || Player[2].HasKey)
                {
                    g_render->renderTexture(-24 + 40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[0], 0, 0);
                }

                if(Player[1].Character == 5)
                {
                    g_render->renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
                }
                else
                {
                    g_render->renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
                }

                g_render->renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

                SuperPrint(coinsStr, 1,
                           40 + 20 - (int(coinsStr.size()) * 18) +
                           (float(vScreen[Z].Width) / 2.0f) - (GFX.Container[1].w / 2) + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w,
                           16 + 11);
                // Print Score
                SuperPrint(scoreStr, 1,
                           40 + 20 - (int(scoreStr.size()) * 18) +
                           (float(vScreen[Z].Width) / 2.0f) - (GFX.Container[1].w / 2) + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w,
                           16 + 31);

                // Print lives on the screen
                g_render->renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                g_render->renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

                SuperPrint(livesStr, 1,
                           float(-80 + (vScreen[Z].Width / 2.0) - (GFX.Container[1].w / 2) + C - 122 + 12 + 18 + GFX.Interface[5].w),
                           16 + 11);
                // Print stars on the screen
                if(numStars > 0)
                {
                    g_render->renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122, 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                    g_render->renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(numStarsStr, 1,
                               float(-80 + (vScreen[Z].Width / 2.0) - (GFX.Container[1].w / 2) + C - 122 + 12 + 18 + GFX.Interface[5].w),
                               16 + 31);
                }
            }
            else
            {
                // plr 1 score
                g_render->renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                g_render->renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(BattleLives[1]), 1,
                           float(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                           16 + 11);

                // plr 2 score
                g_render->renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[7], 0, 0);
                g_render->renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(BattleLives[2]), 1,
                           float(24 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w),
                           16 + 11);
            }
        }
        else // Split screen
        {


// 2 players 2 screen heldbonus

            if(Player[Z].Character == 3 || Player[Z].Character == 4 || Player[Z].Character == 5)
            {
                if(Player[Z].Hearts > 0)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }

                if(Player[Z].Hearts > 1)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }

                if(Player[Z].Hearts > 2)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
            }
            else
            {

                g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX.Container[Player[Z].Character], 0, 0);
                if(Player[Z].HeldBonus > 0)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 16 + 12,
                                            NPCWidth[Player[Z].HeldBonus], NPCHeight[Player[Z].HeldBonus],
                                            GFXNPC[Player[Z].HeldBonus], 0, 0);
                }
            }

            if(Player[Z].Character == 5 && Player[Z].Bombs > 0)
            {
                g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 34, 52, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[8], 0, 0);
                g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 10, 53, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(Player[Z].Bombs), 1, 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 53);
            }

            if(!BattleMode)
            {
                // Print coins on the screen
                if(Player[Z].HasKey)
                {
                    g_render->renderTexture(-24 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[0], 0, 0);
                }
                if(Player[Z].Character == 5)
                {
                    g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
                }
                else
                {
                    g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
                }


                g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(coinsStr, 1,
                           float(20 - int(coinsStr.size()) * 18 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w),
                           16 + 11);
                // Print Score
                SuperPrint(scoreStr,
                           1,
                           float(20 - int(scoreStr.size()) * 18 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w),
                           16 + 31);
                // Print lives on the screen

                g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(livesStr, 1,
                           float(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                           16 + 11);
                // Print stars on the screen
                if(numStars > 0)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122, 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(numStarsStr, 1,
                               float(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                               16 + 31);
                }
            }
            else
            {
                if(Z == 1)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122,
                                          16 + 10,
                                          GFX.Interface[3].w,
                                          GFX.Interface[3].h,
                                          GFX.Interface[3],
                                          0, 0);
                }
                else
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122,
                                          16 + 10,
                                          GFX.Interface[3].w,
                                          GFX.Interface[3].h,
                                          GFX.Interface[7],
                                          0, 0);
                }

                g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w + 16, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(BattleLives[Z]), 1,
                           float(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w + 16),
                           16 + 11);
            }
        }
    }
    else // 1 Player or Multi Mario
    {


//        if(nPlay.Online == false)
        {
            if(Player[1].Character == 3 || Player[1].Character == 4 || Player[1].Character == 5)
            {
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
                if(Player[1].Hearts > 0)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
                if(Player[1].Hearts > 1)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
                if(Player[1].Hearts > 2)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
            }
            else
            {
                g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX.Container[0], 0, 0);
                if(Player[1].HeldBonus > 0)
                {
                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[1].HeldBonus], NPCHeight[Player[1].HeldBonus], GFXNPC[Player[1].HeldBonus], 0, 0);
                }
            }
        }
//        else
//        {
//            if(Player[nPlay.MySlot + 1].Character == 3 || Player[nPlay.MySlot + 1].Character == 4)
//            {
////                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
////                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
////                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                if(Player[nPlay.MySlot + 1].Hearts > 0)
//                {
//                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//                if(Player[nPlay.MySlot + 1].Hearts > 1)
//                {
//                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//                if(Player[nPlay.MySlot + 1].Hearts > 2)
//                {
//                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//            }
//            else
//            {
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX::ContainerMask(0).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX::Container(0).hdc, 0, 0, vbSrcPaint;
//                if(Player[nPlay.MySlot + 1].HeldBonus > 0)
//                {
//                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[nPlay.MySlot + 1].HeldBonus], NPCHeight[Player[nPlay.MySlot + 1].HeldBonus], GFXNPCMask[Player[nPlay.MySlot + 1].HeldBonus], 0, 0);
//                    g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[nPlay.MySlot + 1].HeldBonus], NPCHeight[Player[nPlay.MySlot + 1].HeldBonus], GFXNPC[Player[nPlay.MySlot + 1].HeldBonus], 0, 0);
//                }
//            }
//        }

        if(Player[1].Character == 5 && Player[1].Bombs > 0)
        {

            g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 34 + C, 52, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[8], 0, 0);
            g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 10 + C, 53, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
            SuperPrint(std::to_string(Player[1].Bombs), 1,
                       float(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12 + C), 53);
        }

        // Print coins on the screen
        if(Player[1].HasKey)
        {
            g_render->renderTexture(-24 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[0], 0, 0);
        }

        if(Player[1].Character == 5)
        {
            g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
        }
        else
        {
            g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
        }

        g_render->renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
        SuperPrint(coinsStr, 1,
                   float(20 - int(coinsStr.size()) * 18 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w),
                   16 + 11);
        // Print Score
        SuperPrint(scoreStr, 1,
                   20 - int(scoreStr.size()) * 18 +
                   float(vScreen[Z].Width) / 2.0f - GFX.Container[1].w / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w,
                   16 + 31);
        // Print lives on the screen
        g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
        g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
        SuperPrint(livesStr, 1,
                   float(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                   16 + 11);
        // Print stars on the screen
        if(numStars > 0)
        {
            g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122, 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
            g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
            SuperPrint(numStarsStr, 1,
                       float(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w),
                       16 + 31);
        }
    }

    if(BattleIntro > 0)
    {
        if(BattleIntro > 45 || BattleIntro % 2 == 1)
        {
            g_render->renderTexture(vScreen[Z].Width / 2.0 - GFX.BMVs.w / 2, -96 + vScreen[Z].Height / 2.0 - GFX.BMVs.h / 2, GFX.BMVs.w, GFX.BMVs.h, GFX.BMVs, 0, 0);
            g_render->renderTexture(-50 + vScreen[Z].Width / 2.0 - GFX.CharacterName[Player[1].Character].w, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[1].Character].h / 2, GFX.CharacterName[Player[1].Character].w, GFX.CharacterName[Player[1].Character].h, GFX.CharacterName[Player[1].Character], 0, 0);
            g_render->renderTexture(50 + vScreen[Z].Width / 2.0, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[2].Character].h / 2, GFX.CharacterName[Player[2].Character].w, GFX.CharacterName[Player[2].Character].h, GFX.CharacterName[Player[2].Character], 0, 0);
        }
    }

    if(BattleOutro > 0)
    {
        g_render->renderTexture(10 + vScreen[Z].Width / 2.0, -96 + vScreen[Z].Height / 2.0 - GFX.BMWin.h / 2, GFX.BMWin.w, GFX.BMWin.h, GFX.BMWin, 0, 0);
        g_render->renderTexture(-10 + vScreen[Z].Width / 2.0 - GFX.CharacterName[Player[BattleWinner].Character].w, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[BattleWinner].Character].h / 2, GFX.CharacterName[Player[BattleWinner].Character].w, GFX.CharacterName[Player[BattleWinner].Character].h, GFX.CharacterName[Player[BattleWinner].Character], 0, 0);
    }

    g_render->offsetViewportIgnore(false);
}
