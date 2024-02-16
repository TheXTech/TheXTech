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

#include "../globals.h"
#include "../game_main.h"
#include "../custom.h"
#include "npc_traits.h"
#include "npc_id.h"

void SetupVars()
{
    //int A;
    SetupPlayerFrames();
    for(int A = 1; A <= maxEffectType; A++)
    {
        EffectWidth[A] = 32;
        EffectHeight[A] = 32;
    }
    // now part of the default NPCTraits_t initializer'
    // for(int A = 1; A <= maxNPCType; ++A)
    //     NPCTraits[A].Speedvar = 1;
    for(int A = 174; A <= 186; ++A)
        BackgroundFence[A] = true;

    GFXLevelBig[21] = true;
    GFXLevelBig[22] = true;
    GFXLevelBig[23] = true;
    GFXLevelBig[24] = true;
    GFXLevelBig[28] = true;

    EffectWidth[145] = 32;
    EffectHeight[145] = 60;

    EffectWidth[71] = 16;
    EffectHeight[71] = 16;
    EffectWidth[148] = 16;
    EffectHeight[148] = 16;


    EffectWidth[111] = 16;
    EffectHeight[111] = 16;
    EffectHeight[121] = 48;

    EffectWidth[144] = 54;
    EffectHeight[144] = 42;


    EffectWidth[143] = 64;
    EffectHeight[143] = 64;

    EffectWidth[120] = 48;
    EffectHeight[121] = 48;


    EffectWidth[138] = 48;
    EffectHeight[138] = 64;


    EffectWidth[125] = 80;
    EffectHeight[125] = 34;

    EffectWidth[134] = 32;
    EffectHeight[134] = 58;

    EffectWidth[130] = 30;
    EffectHeight[130] = 34;

    EffectWidth[132] = 32;
    EffectHeight[132] = 32;
    EffectWidth[133] = 10;
    EffectHeight[133] = 8;


    EffectWidth[1] = 16;
    EffectHeight[1] = 16;

    EffectWidth[135] = 16;
    EffectHeight[135] = 16;

    EffectWidth[104] = 32;
    EffectHeight[104] = 32;
    EffectWidth[105] = 64;
    EffectHeight[105] = 72;

    EffectWidth[129] = 30;
    EffectHeight[129] = 42;

    EffectWidth[106] = 80;
    EffectHeight[106] = 94;

    EffectWidth[108] = 64;
    EffectHeight[108] = 64;

    EffectWidth[112] = 96;
    EffectHeight[112] = 106;

    EffectWidth[4] = 32;
    EffectHeight[4] = 32;
    EffectWidth[8] = 32;
    EffectHeight[8] = 32;
    EffectWidth[9] = 32;
    EffectHeight[9] = 32;
    EffectWidth[10] = 32;
    EffectHeight[10] = 32;
    EffectWidth[11] = 32;
    EffectHeight[11] = 32;
    EffectWidth[12] = 8;
    EffectHeight[12] = 8;
    EffectWidth[13] = 84;
    EffectHeight[13] = 26;
    EffectWidth[14] = 62;
    EffectHeight[14] = 32;
    EffectWidth[15] = 32;
    EffectHeight[15] = 28;
    EffectWidth[16] = 128;
    EffectHeight[16] = 128;
    EffectWidth[17] = 32;
    EffectHeight[17] = 32;
    EffectWidth[18] = 32;
    EffectHeight[18] = 32;
    EffectWidth[19] = 32;
    EffectHeight[19] = 32;
    EffectWidth[20] = 32;
    EffectHeight[20] = 32;
    EffectWidth[21] = 16;
    EffectHeight[21] = 16;
    EffectWidth[22] = 32;
    EffectHeight[22] = 32;
    EffectWidth[23] = 32;
    EffectHeight[23] = 32;
    EffectWidth[24] = 32;
    EffectHeight[24] = 32;
    EffectWidth[25] = 32;
    EffectHeight[25] = 48;
    EffectWidth[26] = 32;
    EffectHeight[26] = 32;
    EffectWidth[29] = 40;
    EffectHeight[29] = 64;
    EffectWidth[30] = 16;
    EffectHeight[30] = 16;
    EffectWidth[31] = 32;
    EffectHeight[31] = 32;
    EffectWidth[32] = 32;
    EffectHeight[32] = 62;
    EffectWidth[33] = 32;
    EffectHeight[33] = 32;
    EffectWidth[34] = 32;
    EffectHeight[34] = 32;
    EffectWidth[35] = 32;
    EffectHeight[35] = 32;
    EffectWidth[36] = 32;
    EffectHeight[36] = 32;
    EffectWidth[37] = 32;
    EffectHeight[37] = 32;
    EffectWidth[38] = 32;
    EffectHeight[38] = 32;
    EffectWidth[45] = 48;
    EffectHeight[45] = 46;
    EffectWidth[46] = 48;
    EffectHeight[46] = 46;
    EffectWidth[47] = 44;
    EffectHeight[47] = 44;
    EffectWidth[48] = 32;
    EffectHeight[48] = 32;
    EffectWidth[50] = 64;
    EffectHeight[50] = 80;
    EffectWidth[51] = 16;
    EffectHeight[51] = 16;
    EffectWidth[54] = 32;
    EffectHeight[54] = 64;
    EffectWidth[55] = 32;
    EffectHeight[55] = 64;
    EffectWidth[56] = 32;
    EffectHeight[56] = 32;
    EffectWidth[57] = 16;
    EffectHeight[57] = 16;
    EffectWidth[58] = 32;
    EffectHeight[58] = 32;
    EffectWidth[59] = 32;
    EffectHeight[59] = 64;
    EffectWidth[60] = 32;
    EffectHeight[60] = 32;
    EffectWidth[61] = 32;
    EffectHeight[61] = 32;
    EffectWidth[62] = 32;
    EffectHeight[62] = 32;
    EffectWidth[63] = 48;
    EffectHeight[63] = 48;
    EffectWidth[64] = 32;
    EffectHeight[64] = 32;
    EffectWidth[65] = 32;
    EffectHeight[65] = 32;
    EffectWidth[66] = 32;
    EffectHeight[66] = 32;
    EffectWidth[67] = 32;
    EffectHeight[67] = 32;
    EffectWidth[68] = 16;
    EffectHeight[68] = 16;
    EffectWidth[69] = 64;
    EffectHeight[69] = 64;
    EffectWidth[70] = 16;
    EffectHeight[70] = 16;
    EffectWidth[71] = 16;
    EffectHeight[71] = 16;
    EffectWidth[72] = 32;
    EffectHeight[72] = 28;
    EffectWidth[73] = 32;
    EffectHeight[73] = 32;
    EffectWidth[74] = 8;
    EffectHeight[74] = 10;
    EffectWidth[75] = 32;
    EffectHeight[75] = 32;
    EffectWidth[76] = 14;
    EffectHeight[76] = 14;
    EffectWidth[77] = 8;
    EffectHeight[77] = 8;
    EffectWidth[139] = 8;
    EffectHeight[139] = 8;
    EffectWidth[78] = 10;
    EffectHeight[78] = 10;
    EffectWidth[79] = 50;
    EffectHeight[79] = 24;
    EffectWidth[80] = 16;
    EffectHeight[80] = 16;
    EffectWidth[81] = 32;
    EffectHeight[81] = 32;
    EffectWidth[82] = 32;
    EffectHeight[82] = 32;
    EffectWidth[83] = 32;
    EffectHeight[83] = 42;
    EffectWidth[84] = 32;
    EffectHeight[84] = 32;
    EffectWidth[85] = 32;
    EffectHeight[85] = 32;
    EffectWidth[86] = 40;
    EffectHeight[86] = 64;
    EffectWidth[87] = 64;
    EffectHeight[87] = 64;
    EffectWidth[88] = 32;
    EffectHeight[88] = 32;
    EffectWidth[89] = 26;
    EffectHeight[89] = 46;
    EffectWidth[90] = 48;
    EffectHeight[90] = 96;
    EffectWidth[91] = 140;
    EffectHeight[91] = 128;
    EffectWidth[92] = 32;
    EffectHeight[92] = 32;
    EffectWidth[93] = 32;
    EffectHeight[93] = 32;
    EffectWidth[94] = 32;
    EffectHeight[94] = 32;
    EffectWidth[97] = 48;
    EffectHeight[97] = 64;
    EffectWidth[98] = 64;
    EffectHeight[98] = 64;
    EffectWidth[99] = 48;
    EffectHeight[99] = 64;
    EffectWidth[100] = 16;
    EffectHeight[100] = 16;

    EffectWidth[113] = 8;
    EffectHeight[113] = 8;

    EffectWidth[140] = 32;
    EffectHeight[140] = 40;


    EffectWidth[114] = 32;
    EffectHeight[114] = 32;

    EffectWidth[103] = 120;
    EffectHeight[103] = 96;

    EffectWidth[123] = 32;
    EffectHeight[123] = 32;
    EffectWidth[124] = 32;
    EffectHeight[124] = 32;
    EffectHeight[95] = 28;
    EffectHeight[96] = 28;

    EffectHeight[46] = 46;
    EffectWidth[46] = 48;
    EffectHeight[47] = 44;
    EffectWidth[47] = 44;

    // NEW: reset all variables to their original values (it will be nice to convert all NPC traits to a single struct)
    NPCTraits.fill(NPCTraits_t());

    // SMBX64 logic:
    // now included as part of default NPCTraits_t initializer
    // for(int A = 1; A <= maxNPCType; ++A)
    // {
    //     NPCTraits[A].Score = 2;
    //     NPCTraits[A].TWidth = 32;
    //     NPCTraits[A].THeight = 32;
    // }

    NPCTraits[291].TWidth = 22;
    NPCTraits[291].THeight = 18;
    NPCTraits[291].WidthGFX = 42;
    NPCTraits[291].HeightGFX = 34;
    NPCTraits[291].WontHurt = true;
    NPCTraits[291].JumpHurt = true;
    NPCTraits[291].NoYoshi = true;



    NPCTraits[256].NoYoshi = true;
    NPCTraits[257].NoYoshi = true;
    NPCTraits[239].NoYoshi = true;


    NPCTraits[208].Score = 0;
    NPCTraits[15].Score = 7;
    NPCTraits[39].Score = 7;
    NPCTraits[86].Score = 9;
    NPCTraits[200].Score = 9;
    NPCTraits[201].Score = 9;
    NPCTraits[209].Score = 9;
    NPCTraits[29].Score = 5;
    NPCTraits[47].Score = 6;
    NPCTraits[284].Score = 6;
    NPCTraits[256].Score = 5;
    NPCTraits[257].Score = 5;
    NPCTraits[262].Score = 8;
    NPCTraits[267].Score = 8;
    NPCTraits[268].Score = 8;
    NPCTraits[280].Score = 8;
    NPCTraits[281].Score = 8;
    NPCTraits[251].FrameOffsetY = 2;
    NPCTraits[252].FrameOffsetY = 2;
    NPCTraits[253].FrameOffsetY = 2;
    NPCTraits[195].FrameOffsetY = 2;
    NPCTraits[77].FrameOffsetY = 2;

    NPCTraits[288].WontHurt = true;
    NPCTraits[288].JumpHurt = true;

    NPCTraits[287].WontHurt = true;
    NPCTraits[195].WontHurt = true;
    NPCTraits[210].Foreground = true;
    NPCTraits[230].Foreground = true;

    NPCTraits[28].IsFish = true;
    NPCTraits[229].IsFish = true;
    NPCTraits[230].IsFish = true;
    NPCTraits[232].IsFish = true;
    NPCTraits[233].IsFish = true;
    NPCTraits[234].IsFish = true;
    NPCTraits[236].IsFish = true;


    NPCTraits[289].NoClipping = true;
    NPCTraits[289].WontHurt = true;
    NPCTraits[289].JumpHurt = true;
    NPCTraits[289].TWidth = 32;
    NPCTraits[289].THeight = 32;
    NPCTraits[289].WidthGFX = 32;
    NPCTraits[289].HeightGFX = 64;


    NPCTraits[283].TWidth = 48;
    NPCTraits[283].THeight = 48;
    NPCTraits[283].WidthGFX = 64;
    NPCTraits[283].HeightGFX = 64;
    NPCTraits[283].FrameOffsetY = 8;
    NPCTraits[283].WontHurt = true;
    NPCTraits[283].JumpHurt = true;

    NPCTraits[290].WidthGFX = 32;
    NPCTraits[290].HeightGFX = 64;
    NPCTraits[290].Foreground = true;
    NPCTraits[290].CanWalkOn = true;
    NPCTraits[290].IsABlock = true;
    NPCTraits[290].WontHurt = true;
    NPCTraits[290].NoClipping = true;


    NPCTraits[263].CanWalkOn = true;
    NPCTraits[263].WontHurt = true;
    NPCTraits[263].MovesPlayer = true;
    NPCTraits[263].IsGrabbable = true;
    NPCTraits[263].GrabFromTop = true;
    NPCTraits[263].IsABlock = true;

    NPCTraits[262].CanWalkOn = true;
    NPCTraits[262].TWidth = 40;
    NPCTraits[262].THeight = 56;
    NPCTraits[262].WidthGFX = 48;
    NPCTraits[262].HeightGFX = 64;



    NPCTraits[254].IsABonus = true;
    NPCTraits[237].IsAShell = true;
    NPCTraits[237].WontHurt = true;
    NPCTraits[237].CanWalkOn = true;
    NPCTraits[13].JumpHurt = true;
    NPCTraits[86].JumpHurt = true;
    NPCTraits[260].JumpHurt = true;



    NPCTraits[263].NoYoshi = true;
    NPCTraits[265].NoYoshi = true;
    NPCTraits[260].NoYoshi = true;
    NPCTraits[255].NoYoshi = true;

    NPCTraits[270].NoClipping = true;

    NPCTraits[246].JumpHurt = true;
    NPCTraits[246].NoClipping = true;
    NPCTraits[246].NoYoshi = true;
    NPCTraits[246].TWidth = 16;
    NPCTraits[246].THeight = 16;

    NPCTraits[251].TWidth = 18; // TLOZ Rupee
    NPCTraits[251].THeight = 32;
    NPCTraits[251].IsABonus = true;
    NPCTraits[251].IsACoin = true;

    NPCTraits[252].TWidth = 18; // TLOZ Rupee
    NPCTraits[252].THeight = 32;
    NPCTraits[252].IsABonus = true;
    NPCTraits[252].IsACoin = true;

    NPCTraits[253].TWidth = 18; // TLOZ Rupee
    NPCTraits[253].THeight = 32;
    NPCTraits[253].IsABonus = true;
    NPCTraits[253].IsACoin = true;

    NPCTraits[250].TWidth = 32; // TLOZ Heart
    NPCTraits[250].THeight = 32;
    NPCTraits[250].IsABonus = true;

    // vines
    for(int A = 213; A <= 224; ++A)
    {
        NPCTraits[A].IsAVine = true;
        NPCTraits[A].WontHurt = true;
        NPCTraits[A].NoClipping = true;
        NPCTraits[A].JumpHurt = true;
        NPCTraits[A].NoYoshi = true;
        NPCTraits[A].TWidth = 16;
        NPCTraits[A].WidthGFX = 32;
        NPCTraits[A].HeightGFX = 32;
    }
    NPCTraits[223].THeight = 24;
    NPCTraits[223].HeightGFX = 24;
    NPCTraits[231].JumpHurt = true;
    NPCTraits[225].WontHurt = true;
    NPCTraits[225].JumpHurt = true;
    NPCTraits[225].NoYoshi = true;
    NPCTraits[226].WontHurt = true;
    NPCTraits[226].JumpHurt = true;
    NPCTraits[226].NoYoshi = true;
    NPCTraits[227].WontHurt = true;
    NPCTraits[227].JumpHurt = true;
    NPCTraits[227].NoYoshi = true;

    NPCTraits[211].NoYoshi = true;

    NPCTraits[195].IsGrabbable = true;
    NPCTraits[195].GrabFromTop = true;

    NPCTraits[240].TWidth = 32;
    NPCTraits[240].THeight = 32;
    NPCTraits[240].IsABonus = true;//32;

    NPCTraits[248].TWidth = 32;
    NPCTraits[248].THeight = 32;
    NPCTraits[248].IsABonus = true;//32;

    NPCTraits[200].TWidth = 64;
    NPCTraits[200].THeight = 72;
    NPCTraits[200].FrameOffsetY = 2;
    NPCTraits[200].JumpHurt = true;
    NPCTraits[210].JumpHurt = true;
    NPCTraits[259].NoClipping = true;
    NPCTraits[259].NoYoshi = true;
    NPCTraits[259].JumpHurt = true;
    NPCTraits[260].TWidth = 16;
    NPCTraits[260].THeight = 16;
    NPCTraits[260].NoClipping = true;

    NPCTraits[153].TWidth = 32; // Poison Mushroom
    NPCTraits[153].THeight = 32;
    NPCTraits[153].FrameOffsetY = 2;

    NPCTraits[242].TWidth = 32; // SML2 Goomba
    NPCTraits[242].THeight = 32;
    NPCTraits[242].FrameOffsetY = 2;
    // NPCDefaultMovement(242) = true;

    NPCTraits[243].TWidth = 32; // SML2 Flying Goomba
    NPCTraits[243].THeight = 32;
    NPCTraits[243].FrameOffsetY = 2;
    NPCTraits[243].WidthGFX = 56;
    NPCTraits[243].HeightGFX = 36;

    NPCTraits[244].TWidth = 32; // SMB3 Flying Goomba
    NPCTraits[244].THeight = 32;
    NPCTraits[244].FrameOffsetY = 2;
    NPCTraits[244].WidthGFX = 40;
    NPCTraits[244].HeightGFX = 48;

    NPCTraits[255].TWidth = 20;
    NPCTraits[255].THeight = 96;
    NPCTraits[255].WontHurt = true;
    NPCTraits[255].IsABlock = true;
    NPCTraits[255].CanWalkOn = true;
    NPCTraits[255].MovesPlayer = true;

    // NPCIsAParaTroopa(244) = true;
    // NPCIsAParaTroopa(243) = true;

    NPCTraits[1].TWidth = 32; // Goomba
    NPCTraits[1].THeight = 32;
    NPCTraits[1].FrameOffsetY = 2;

    NPCTraits[1].TWidth = 32; // Goomba
    NPCTraits[1].THeight = 32;
    NPCTraits[1].FrameOffsetY = 2;
    NPCTraits[2].TWidth = 32; // Red goomba
    NPCTraits[2].THeight = 32;
    NPCTraits[2].FrameOffsetY = 2;
    NPCTraits[3].TWidth = 32; // Flying goomba
    NPCTraits[3].THeight = 32;
    NPCTraits[3].WidthGFX = 40;
    NPCTraits[3].HeightGFX = 48;
    NPCTraits[3].FrameOffsetY = 2;
    NPCTraits[4].TWidth = 32; // Green koopa
    NPCTraits[4].THeight = 32;
    NPCTraits[4].WidthGFX = 32;
    NPCTraits[4].HeightGFX = 54;
    NPCTraits[4].FrameOffsetY = 2;
    NPCTraits[5].TWidth = 32; // Green shell
    NPCTraits[5].THeight = 32;
    NPCTraits[5].FrameOffsetY = 2;
    NPCTraits[6].TWidth = 32; // Red koopa
    NPCTraits[6].THeight = 32;
    NPCTraits[6].WidthGFX = 32;
    NPCTraits[6].HeightGFX = 54;
    NPCTraits[6].FrameOffsetY = 2;
    NPCTraits[7].TWidth = 32; // Red shell
    NPCTraits[7].THeight = 32;
    NPCTraits[7].FrameOffsetY = 2;
    NPCTraits[8].TWidth = 32; // Plant
    NPCTraits[8].THeight = 48;
    NPCTraits[8].WidthGFX = 32;
    NPCTraits[8].HeightGFX = 48;
    NPCTraits[245].TWidth = 32; // SMB 3 Fire Plant
    NPCTraits[245].THeight = 64;
    NPCTraits[245].WidthGFX = 32;
    NPCTraits[245].HeightGFX = 64;
    NPCTraits[8].FrameOffsetY = 1;
    NPCTraits[9].TWidth = 32; // Mushroom
    NPCTraits[9].THeight = 32;
    NPCTraits[9].FrameOffsetY =2;

    // Reversed incompatible powerup score customizability implementation at v1.3.6.1.
    // These scores are now found in loadNpcSetupFixes();

    NPCTraits[273].TWidth = 32; // ? Mushroom
    NPCTraits[273].THeight = 32;
    NPCTraits[273].FrameOffsetY = 2;

    NPCTraits[249].TWidth = 32; // SMB2 Mushroom
    NPCTraits[249].THeight = 32;
    NPCTraits[249].IsABonus = true;

    NPCTraits[274].TWidth = 32; // dragon coin
    NPCTraits[274].THeight = 50;
    NPCTraits[274].IsABonus = true;
    NPCTraits[274].IsACoin = true;
    NPCTraits[274].Score = 6;

    NPCTraits[10].TWidth = 28; // SMB3 Coin
    NPCTraits[10].THeight = 32;
    NPCTraits[11].TWidth = 32; // SMB3 Level exit
    NPCTraits[11].THeight = 32;
    NPCTraits[12].TWidth = 28; // Big Fireball
    NPCTraits[12].THeight = 32;
    NPCTraits[13].TWidth = 16; // Small Fireball
    NPCTraits[13].THeight = 16;
    NPCTraits[265].TWidth = 16; // Ice Bolt
    NPCTraits[265].THeight = 16;
    NPCTraits[265].NoIceBall = true;
    NPCTraits[13].NoYoshi = true;
    NPCTraits[14].TWidth = 32; // Fire Flower
    NPCTraits[14].THeight = 32;
    NPCTraits[14].FrameOffsetY = 2;
    NPCTraits[264].TWidth = 32; // Ice Flower
    NPCTraits[264].THeight = 32;
    NPCTraits[264].FrameOffsetY = 2;
    NPCTraits[277].TWidth = 32; // Ice Flower
    NPCTraits[277].THeight = 32;
    NPCTraits[277].FrameOffsetY = 2;
    NPCTraits[15].TWidth = 60; // Big Koopa
    NPCTraits[15].THeight = 54;
    NPCTraits[15].WidthGFX = 68;
    NPCTraits[15].HeightGFX = 54;
    NPCTraits[15].FrameOffsetY = 2;
    NPCTraits[15].NoYoshi = true;
    NPCTraits[16].TWidth = 32; // Boss Exit
    NPCTraits[16].THeight = 32;
    NPCTraits[17].TWidth = 32; // Bullet Bill
    NPCTraits[17].THeight = 28;
    NPCTraits[18].TWidth = 128; // Giant Bullet Bill
    NPCTraits[18].THeight = 128;
    NPCTraits[18].NoYoshi = true;
    NPCTraits[19].TWidth = 32; // Red Shy guy
    NPCTraits[19].THeight = 32;
    NPCTraits[20].TWidth = 32; // Blue Shy guy
    NPCTraits[20].THeight = 32;
    NPCTraits[247].TWidth = 32; // Cactus Thing
    NPCTraits[247].THeight = 32;
    NPCTraits[21].TWidth = 32; // Bullet Bill Shooter
    NPCTraits[21].THeight = 32;
    NPCTraits[21].NoYoshi = true;
    NPCTraits[22].TWidth = 32; // Bullet Bill Gun
    NPCTraits[22].THeight = 32;
    NPCTraits[22].FrameOffsetY = 2;
    NPCTraits[23].TWidth = 32; // Hard thing
    NPCTraits[23].THeight = 32;
    NPCTraits[23].FrameOffsetY = 2;
    NPCTraits[24].TWidth = 32; // Hard Thing shell
    NPCTraits[24].THeight = 32;
    NPCTraits[24].FrameOffsetY = 2;
    NPCTraits[25].TWidth = 32; // Bouncy Start Thing
    NPCTraits[25].THeight = 32;
    NPCTraits[26].TWidth = 32; // Spring
    NPCTraits[26].THeight = 32;
    NPCTraits[26].WidthGFX = 32;
    NPCTraits[26].HeightGFX = 32;
    NPCTraits[26].FrameOffsetY = 2;
    NPCTraits[27].TWidth = 32; // Grey goomba
    NPCTraits[27].THeight = 32;
    NPCTraits[28].TWidth = 32; // Red Jumping Fish
    NPCTraits[28].THeight = 32;
    NPCTraits[29].TWidth = 32; // Hammer Bro
    NPCTraits[29].THeight = 48;
    NPCTraits[29].FrameOffsetY = 2;
    NPCTraits[30].TWidth = 32; // Hammer
    NPCTraits[30].THeight = 32;
    NPCTraits[30].NoYoshi = true;
    NPCTraits[31].TWidth = 32; // Key
    NPCTraits[31].THeight = 32;
    NPCTraits[31].FrameOffsetY = 1;
    NPCTraits[32].TWidth = 32; // P Switch
    NPCTraits[32].THeight = 32;
    NPCTraits[32].FrameOffsetY = 2;
    NPCTraits[238].TWidth = 32; // P Switch Time
    NPCTraits[238].THeight = 32;
    NPCTraits[238].FrameOffsetY = 2;
    NPCTraits[239].TWidth = 32; // Push down thing
    NPCTraits[239].THeight = 32;
    NPCTraits[239].FrameOffsetY = 2;
    NPCTraits[33].TWidth = 24; // SMW Coin
    NPCTraits[33].THeight = 32;
    NPCTraits[258].TWidth = 24; // SMW Blue Coin
    NPCTraits[258].THeight = 32;
    NPCTraits[34].TWidth = 32; // Leaf
    NPCTraits[34].THeight = 32;
    NPCTraits[35].TWidth = 32; // Goombas Shoe
    NPCTraits[35].THeight = 32;
    NPCTraits[35].FrameOffsetY = 2;
    NPCTraits[191].FrameOffsetY = 2;
    NPCTraits[193].FrameOffsetY = 2;

    NPCTraits[285].TWidth = 32; // Spiney
    NPCTraits[285].THeight = 32;
    NPCTraits[285].FrameOffsetY = 2;
    NPCTraits[286].TWidth = 32; // Falling Spiney
    NPCTraits[286].THeight = 32;
    NPCTraits[286].FrameOffsetY = 2;

    NPCTraits[36].TWidth = 32; // Spiney
    NPCTraits[36].THeight = 32;
    NPCTraits[36].FrameOffsetY = 2;
    NPCTraits[37].TWidth = 48; // Thwomp
    NPCTraits[37].THeight = 64;
    NPCTraits[37].NoYoshi = true;
    NPCTraits[38].TWidth = 32; // Boo
    NPCTraits[38].THeight = 32;
    NPCTraits[38].NoYoshi = true;
    NPCTraits[39].TWidth = 32; // Birdo
    NPCTraits[39].THeight = 60;
    NPCTraits[39].WidthGFX = 40;
    NPCTraits[39].HeightGFX = 72;
    NPCTraits[39].FrameOffsetY = 2;
    NPCTraits[39].FrameOffsetX = 3;
    NPCTraits[39].NoYoshi = true;
    NPCTraits[40].TWidth = 32; // egg
    NPCTraits[40].THeight = 24;
    NPCTraits[41].TWidth = 32; // smb2 exit(birdo)
    NPCTraits[41].THeight = 32;
    NPCTraits[42].TWidth = 32; // ghost 1
    NPCTraits[42].THeight = 32;
    NPCTraits[42].NoYoshi = true;
    NPCTraits[43].TWidth = 32; // ghost 2
    NPCTraits[43].THeight = 32;
    NPCTraits[43].NoYoshi = true;
    NPCTraits[44].TWidth = 128; // big ghost
    NPCTraits[44].THeight = 120;
    NPCTraits[44].WidthGFX = 140;
    NPCTraits[44].HeightGFX = 128;
    NPCTraits[44].NoYoshi = true;
    NPCTraits[45].TWidth = 32; // ice block
    NPCTraits[45].THeight = 32;
    NPCTraits[46].TWidth = 32; // falling block
    NPCTraits[46].THeight = 32;
    NPCTraits[46].NoYoshi = true;
    NPCTraits[212].NoYoshi = true;
    NPCTraits[47].TWidth = 32; // lakitu
    NPCTraits[47].THeight = 48;
    NPCTraits[47].WidthGFX = 32;
    NPCTraits[47].HeightGFX = 64;
    NPCTraits[284].TWidth = 40; // smw lakitu
    NPCTraits[284].THeight = 48;
    NPCTraits[284].FrameOffsetY = 6;
    NPCTraits[284].WidthGFX = 56;
    NPCTraits[284].HeightGFX = 72;

    NPCTraits[48].TWidth = 32; // unripe spiney
    NPCTraits[48].THeight = 32;
    NPCTraits[49].TWidth = 32; // killer pipe
    NPCTraits[49].THeight = 32;
    NPCTraits[49].FrameOffsetY = 2;
    NPCTraits[50].TWidth = 48; // killer plant
    NPCTraits[50].THeight = 32;
    NPCTraits[50].FrameOffsetY = 2;
    NPCTraits[50].NoYoshi = true;
    NPCTraits[51].TWidth = 32; // down piranha plant
    NPCTraits[51].THeight = 64;
    NPCTraits[52].TWidth = 48; // left.right piranha plant
    NPCTraits[52].THeight = 32;
    NPCTraits[53].TWidth = 32; // mr crabs
    NPCTraits[53].THeight = 32;
    NPCTraits[53].FrameOffsetY = 2;
    NPCTraits[54].TWidth = 32; // bee thing
    NPCTraits[54].THeight = 32;
    NPCTraits[54].FrameOffsetY = 2;
    NPCTraits[55].TWidth = 32; // nekkid koopa
    NPCTraits[55].THeight = 32;
    NPCTraits[55].FrameOffsetY = 2;
    NPCTraits[56].TWidth = 128; // koopa clown car
    NPCTraits[56].THeight = 128;
    NPCTraits[56].NoYoshi = true;
    NPCTraits[57].TWidth = 32; // smb3 conveyer belt
    NPCTraits[57].THeight = 32;
    NPCTraits[57].NoYoshi = true;
    NPCTraits[58].TWidth = 32; // smb3 barrel
    NPCTraits[58].THeight = 32;
    NPCTraits[58].NoYoshi = true;
    NPCTraits[59].TWidth = 32; // purple goomba
    NPCTraits[59].THeight = 32;
    NPCTraits[59].FrameOffsetY = 2;
    NPCTraits[60].TWidth = 96; // purple platform
    NPCTraits[60].THeight = 32;
    NPCTraits[60].NoYoshi = true;
    NPCTraits[61].TWidth = 32; // blue goomba
    NPCTraits[61].THeight = 32;
    NPCTraits[61].FrameOffsetY = 2;
    NPCTraits[62].TWidth = 96; // blue platform
    NPCTraits[62].THeight = 32;
    NPCTraits[62].NoYoshi = true;
    NPCTraits[63].TWidth = 32; // green goomba
    NPCTraits[63].THeight = 32;
    NPCTraits[63].FrameOffsetY = 2;
    NPCTraits[64].TWidth = 96; // green platform
    NPCTraits[64].THeight = 32;
    NPCTraits[64].NoYoshi = true;
    NPCTraits[65].TWidth = 32; // red goomba
    NPCTraits[65].THeight = 32;
    NPCTraits[65].FrameOffsetY = 2;
    NPCTraits[66].TWidth = 96; // red platform
    NPCTraits[66].THeight = 32;
    NPCTraits[66].NoYoshi = true;
    NPCTraits[67].TWidth = 128; // grey pipe x
    NPCTraits[67].THeight = 32;
    NPCTraits[67].NoYoshi = true;
    NPCTraits[68].TWidth = 256; // big grey pipe x
    NPCTraits[68].THeight = 32;
    NPCTraits[68].NoYoshi = true;
    NPCTraits[69].TWidth = 32; // grey pipe y
    NPCTraits[69].THeight = 127;//.9; [IDK WY REDIGIT SET THESE!!!]
    NPCTraits[69].NoYoshi = true;
    NPCTraits[70].TWidth = 32; // big grey pipe y
    NPCTraits[70].THeight = 255;//.9;
    NPCTraits[70].NoYoshi = true;
    NPCTraits[71].TWidth = 48; // giant goomba
    NPCTraits[71].THeight = 46;
    NPCTraits[71].FrameOffsetY = 2;
    NPCTraits[72].TWidth = 48; // giant green koopa
    NPCTraits[72].THeight = 48;
    NPCTraits[72].FrameOffsetY = 2;
    NPCTraits[72].WidthGFX = 48;
    NPCTraits[72].HeightGFX = 62;
    NPCTraits[73].TWidth = 44; // giant green shell
    NPCTraits[73].THeight = 44;
    NPCTraits[73].FrameOffsetY = 2;
    NPCTraits[74].TWidth = 48; // giant pirhana plant
    NPCTraits[74].THeight = 64;
    NPCTraits[74].FrameOffsetY = 2;

    NPCTraits[256].TWidth = 48; // gianter pirhana plant
    NPCTraits[256].THeight = 128;
    NPCTraits[256].FrameOffsetY = 2;

    NPCTraits[257].TWidth = 48; // gianter pirhana plant
    NPCTraits[257].THeight = 128;

    NPCTraits[75].TWidth = 38; // toad
    NPCTraits[75].THeight = 54;
    NPCTraits[75].FrameOffsetY = 2;
    NPCTraits[75].WidthGFX = 38;
    NPCTraits[75].HeightGFX = 58;
    // NPCIsToad(75) = true;
    NPCTraits[76].TWidth = 32; // flying green koopa
    NPCTraits[76].THeight = 32;
    NPCTraits[76].FrameOffsetY = 2;
    NPCTraits[76].WidthGFX = 32;
    NPCTraits[76].HeightGFX = 56;
    // NPCIsAParaTroopa(76) = true;
    NPCTraits[161].TWidth = 32; // flying red koopa
    NPCTraits[161].THeight = 32;
    NPCTraits[161].FrameOffsetY = 2;
    NPCTraits[161].WidthGFX = 32;
    NPCTraits[161].HeightGFX = 56;
    // NPCIsAParaTroopa(161) = true;
    NPCTraits[77].TWidth = 32; // black ninja
    NPCTraits[77].THeight = 32;
    NPCTraits[73].FrameOffsetY = 2;
    NPCTraits[78].TWidth = 128; // tank treads
    NPCTraits[78].THeight = 32;
    NPCTraits[78].NoYoshi = true;
    NPCTraits[79].TWidth = 64; // tank parts
    NPCTraits[79].THeight = 32;
    NPCTraits[79].NoYoshi = true;
    NPCTraits[80].TWidth = 128; // tank parts
    NPCTraits[80].THeight = 32;
    NPCTraits[80].NoYoshi = true;
    NPCTraits[81].TWidth = 128; // tank parts
    NPCTraits[81].THeight = 32;
    NPCTraits[81].NoYoshi = true;
    NPCTraits[82].TWidth = 128; // tank parts
    NPCTraits[82].THeight = 32;
    NPCTraits[82].NoYoshi = true;
    NPCTraits[83].TWidth = 256; // tank parts
    NPCTraits[83].THeight = 32;
    NPCTraits[83].NoYoshi = true;
    NPCTraits[84].TWidth = 32; // bowser statue
    NPCTraits[84].THeight = 64;
    NPCTraits[84].NoYoshi = true;
    NPCTraits[85].TWidth = 32; // statue fireball
    NPCTraits[85].THeight = 16;
    NPCTraits[85].NoYoshi = true;
    NPCTraits[86].TWidth = 62; // smb3 bowser
    NPCTraits[86].THeight = 80;
    NPCTraits[86].WidthGFX = 64;
    NPCTraits[86].HeightGFX = 80;
    NPCTraits[86].FrameOffsetY = 2;
    NPCTraits[86].NoYoshi = true;
    NPCTraits[87].TWidth = 48; // smb3 bowser fireball
    NPCTraits[87].THeight = 32;
    NPCTraits[87].NoYoshi = true;
    NPCTraits[88].TWidth = 20; // smb1 coin
    NPCTraits[88].THeight = 32;
    NPCTraits[89].TWidth = 32; // smb1 brown goomba
    NPCTraits[89].THeight = 32;
    NPCTraits[89].FrameOffsetY = 2;
    NPCTraits[90].TWidth = 32; // 1 up
    NPCTraits[90].THeight = 32;
    NPCTraits[90].FrameOffsetY = 2;
    NPCTraits[90].IsAHit1Block = true;
    NPCTraits[91].TWidth = 32; // grab grass
    NPCTraits[91].THeight = 16;
    NPCTraits[91].FrameOffsetY = -16;
    NPCTraits[92].TWidth = 32; // turnip
    NPCTraits[92].THeight = 32;
    // NPCIsVeggie(92) = true;
    NPCTraits[93].TWidth = 32; // SMB1 Plant
    NPCTraits[93].THeight = 48;
    NPCTraits[94].TWidth = 32; // Inert Toad
    NPCTraits[94].THeight = 54;

    NPCTraits[198].TWidth = 32; // Peach
    NPCTraits[198].THeight = 64;
    NPCTraits[198].FrameOffsetY = 2;
    NPCTraits[94].FrameOffsetY = 2;
    // NPCIsToad(94) = true;
    // NPCIsToad(198) = true;
    NPCTraits[95].TWidth = 32; // Green Yoshi
    NPCTraits[95].THeight = 32;
    NPCTraits[95].WidthGFX = 74;
    NPCTraits[95].HeightGFX = 56;
    NPCTraits[95].FrameOffsetY = 2;
    NPCTraits[95].NoYoshi = true;
    NPCTraits[96].TWidth = 32; // Yoshi Egg
    NPCTraits[96].THeight = 32;
    NPCTraits[97].TWidth = 32; // SMB3 Star
    NPCTraits[97].THeight = 32;
    NPCTraits[98].TWidth = 32; // Blue Yoshi
    NPCTraits[98].THeight = 32;
    NPCTraits[98].WidthGFX = 74;
    NPCTraits[98].HeightGFX = 56;
    NPCTraits[98].FrameOffsetY = 2;
    NPCTraits[99].TWidth = 32; // Yellow Yoshi
    NPCTraits[99].THeight = 32;
    NPCTraits[99].WidthGFX = 74;
    NPCTraits[99].HeightGFX = 56;
    NPCTraits[99].FrameOffsetY = 2;
    NPCTraits[100].TWidth = 32; // Red Yoshi
    NPCTraits[100].THeight = 32;
    NPCTraits[100].WidthGFX = 74;
    NPCTraits[100].HeightGFX = 56;
    NPCTraits[100].FrameOffsetY = 2;
    NPCTraits[101].TWidth = 28; // Luigi
    NPCTraits[101].THeight = 62;
    NPCTraits[101].FrameOffsetY = 2;
    // NPCIsToad(101) = true;
    NPCTraits[102].TWidth = 32; // Link
    NPCTraits[102].THeight = 64;
    NPCTraits[102].FrameOffsetY = 2;
    // NPCIsToad(102) = true;
    NPCTraits[103].TWidth = 28; // SMB3 Red Coin
    NPCTraits[103].THeight = 32;
    NPCTraits[104].TWidth = 96; // SMB3 Platform
    NPCTraits[104].THeight = 32;
    NPCTraits[104].NoYoshi = true;
    NPCTraits[105].TWidth = 128; // SMW Falling Platform
    NPCTraits[105].THeight = 22;
    NPCTraits[105].NoYoshi = true;
    NPCTraits[106].TWidth = 128; // SMB Platform
    NPCTraits[106].THeight = 16;
    NPCTraits[106].NoYoshi = true;
    NPCTraits[107].TWidth = 24; // Bob-omb buddy
    NPCTraits[107].THeight = 38;
    NPCTraits[107].WidthGFX = 48;
    NPCTraits[107].HeightGFX = 38;
    NPCTraits[107].FrameOffsetY = 2;
    // NPCIsToad(107) = true;
    NPCTraits[108].TWidth = 32; // Yoshi Fireball
    NPCTraits[108].THeight = 32;
    NPCTraits[108].NoYoshi = true;
    NPCTraits[109].TWidth = 32; // SMW Green Koopa
    NPCTraits[109].THeight = 32;
    NPCTraits[109].WidthGFX = 32;
    NPCTraits[109].HeightGFX = 54;
    NPCTraits[109].FrameOffsetY = 2;
    // NPCDefaultMovement(109) = true;
    NPCTraits[110].TWidth = 32; // SMW Red Koopa
    NPCTraits[110].THeight = 32;
    NPCTraits[110].WidthGFX = 32;
    NPCTraits[110].HeightGFX = 54;
    NPCTraits[110].FrameOffsetY = 2;
    NPCTraits[110].TurnsAtCliffs = true;
    // NPCDefaultMovement(110) = true;
    NPCTraits[111].TWidth = 32; // SMW Blue Koopa
    NPCTraits[111].THeight = 32;
    NPCTraits[111].WidthGFX = 32;
    NPCTraits[111].HeightGFX = 54;
    NPCTraits[111].FrameOffsetY = 2;
    NPCTraits[111].TurnsAtCliffs = true;
    // NPCDefaultMovement(111) = true;
    NPCTraits[112].TWidth = 32; // SMW Yellow Koopa
    NPCTraits[112].THeight = 32;
    NPCTraits[112].WidthGFX = 32;
    NPCTraits[112].HeightGFX = 54;
    NPCTraits[112].FrameOffsetY = 2;
    NPCTraits[112].TurnsAtCliffs = true;
    // NPCDefaultMovement(112) = true;
    NPCTraits[113].TWidth = 32; // SMW Green Shell
    NPCTraits[113].THeight = 32;
    NPCTraits[113].FrameOffsetY = 2;
    NPCTraits[113].IsAShell = true;
    NPCTraits[114].TWidth = 32; // SMW Red Shell
    NPCTraits[114].THeight = 32;
    NPCTraits[114].FrameOffsetY = 2;
    NPCTraits[114].IsAShell = true;
    NPCTraits[115].TWidth = 32; // SMW Blue Shell
    NPCTraits[115].THeight = 32;
    NPCTraits[115].FrameOffsetY = 2;
    NPCTraits[115].IsAShell = true;
    NPCTraits[116].TWidth = 32; // SMW Yellow Shell
    NPCTraits[116].THeight = 32;
    NPCTraits[116].FrameOffsetY = 2;
    NPCTraits[116].IsAShell = true;
    NPCTraits[117].TWidth = 32; // SMW Green Beach Koopa
    NPCTraits[117].THeight = 32;
    NPCTraits[117].FrameOffsetY = 2;
    // NPCDefaultMovement(117) = true;
    NPCTraits[118].TWidth = 32; // SMW Red Beach Koopa
    NPCTraits[118].THeight = 32;
    NPCTraits[118].FrameOffsetY = 2;
    NPCTraits[118].TurnsAtCliffs = true;
    // NPCDefaultMovement(118) = true;
    NPCTraits[119].TWidth = 32; // SMW Blue Beach Koopa
    NPCTraits[119].THeight = 32;
    NPCTraits[119].FrameOffsetY = 2;
    NPCTraits[119].TurnsAtCliffs = true;
    // NPCDefaultMovement(119) = true;
    NPCTraits[120].TWidth = 32; // SMW Yellow Beach Koopa
    NPCTraits[120].THeight = 32;
    NPCTraits[120].FrameOffsetY = 2;
    NPCTraits[120].TurnsAtCliffs = true;
    // NPCDefaultMovement(120) = true;
    NPCTraits[121].TWidth = 32; // SMW Green Para-Koopa
    NPCTraits[121].THeight = 32;
    NPCTraits[121].WidthGFX = 56;
    NPCTraits[121].HeightGFX = 56;
    NPCTraits[121].FrameOffsetY = 2;
    // NPCIsAParaTroopa(121) = true;
    NPCTraits[122].TWidth = 32; // SMW Red Para-Koopa
    NPCTraits[122].THeight = 32;
    NPCTraits[122].WidthGFX = 56;
    NPCTraits[122].HeightGFX = 56;
    NPCTraits[122].FrameOffsetY = 2;
    // NPCIsAParaTroopa(122) = true;
    NPCTraits[123].TWidth = 32; // SMW Blue Para-Koopa
    NPCTraits[123].THeight = 32;
    NPCTraits[123].WidthGFX = 56;
    NPCTraits[123].HeightGFX = 56;
    NPCTraits[123].FrameOffsetY = 2;
    // NPCIsAParaTroopa(123) = true;
    NPCTraits[124].TWidth = 32; // SMW Yellow Para-Koopa
    NPCTraits[124].THeight = 32;
    NPCTraits[124].WidthGFX = 56;
    NPCTraits[124].HeightGFX = 56;
    NPCTraits[124].FrameOffsetY = 2;
    // NPCIsAParaTroopa(124) = true;
    NPCTraits[125].TWidth = 36; // Rat Head
    NPCTraits[125].THeight = 56;
    NPCTraits[125].WidthGFX = 36;
    NPCTraits[125].HeightGFX = 66;
    NPCTraits[125].FrameOffsetY = 2;
    NPCTraits[261].JumpHurt = true;
    NPCTraits[261].NoFireBall = true;
    NPCTraits[17].NoFireBall = true;
    // NPCDefaultMovement(125) = true;
    NPCTraits[125].NoYoshi = true;
    NPCTraits[126].TWidth = 32; // Blue Bot
    NPCTraits[126].THeight = 34;
    NPCTraits[126].WidthGFX = 32;
    NPCTraits[126].HeightGFX = 26;
    // NPCIsABot(126) = true;
    NPCTraits[127].TWidth = 32; // Cyan Bot
    NPCTraits[127].THeight = 34;
    NPCTraits[127].WidthGFX = 32;
    NPCTraits[127].HeightGFX = 26;
    // NPCIsABot(127) = true;
    NPCTraits[128].TWidth = 32; // Red Bot
    NPCTraits[128].THeight = 34;
    NPCTraits[128].WidthGFX = 32;
    NPCTraits[128].HeightGFX = 26;
    NPCTraits[129].TWidth = 32; // SMB2 Jumpy guy
    NPCTraits[129].THeight = 32;
    NPCTraits[129].CanWalkOn = true;
    NPCTraits[129].GrabFromTop = true;
    // NPCDefaultMovement(129) = true;
    NPCTraits[130].TWidth = 32; // Red Sniffit
    NPCTraits[130].THeight = 32;
    NPCTraits[130].CanWalkOn = true;
    NPCTraits[130].GrabFromTop = true;
    // NPCDefaultMovement(130) = true;
    NPCTraits[131].TWidth = 32; // Blue Sniffit
    NPCTraits[131].THeight = 32;
    NPCTraits[131].CanWalkOn = true;
    NPCTraits[131].GrabFromTop = true;
    // NPCDefaultMovement(131) = true;
    NPCTraits[131].TurnsAtCliffs = true;
    NPCTraits[132].TWidth = 32; // Grey Sniffit
    NPCTraits[132].THeight = 32;
    NPCTraits[132].CanWalkOn = true;
    NPCTraits[132].GrabFromTop = true;
    // NPCDefaultMovement(132) = true;
    NPCTraits[133].TWidth = 16; // Bullet
    NPCTraits[133].THeight = 16;
    NPCTraits[133].NoYoshi = true;
    NPCTraits[133].JumpHurt = true;
    NPCTraits[134].TWidth = 24; // SMB2 Bomb
    NPCTraits[134].THeight = 24;
    NPCTraits[134].WidthGFX = 40;
    NPCTraits[134].HeightGFX = 40;
    NPCTraits[134].WontHurt = true;
    NPCTraits[134].IsGrabbable = true;
    NPCTraits[134].GrabFromTop = true;
    NPCTraits[134].CanWalkOn = true;
    NPCTraits[135].TWidth = 32; // SMB2 Bob-om
    NPCTraits[135].THeight = 32;
    NPCTraits[135].GrabFromTop = true;
    NPCTraits[135].CanWalkOn = true;
    // NPCDefaultMovement(135) = true;
    NPCTraits[136].TWidth = 32; // SMB3 Bob-om
    NPCTraits[136].THeight = 32;
    NPCTraits[136].FrameOffsetY = 2;
    // NPCDefaultMovement(136) = true;
    NPCTraits[136].TurnsAtCliffs = true;
    NPCTraits[137].TWidth = 32; // SMB3 Bomb
    NPCTraits[137].THeight = 28;
    NPCTraits[137].FrameOffsetY = 2;
    NPCTraits[137].IsGrabbable = true;
    NPCTraits[138].TWidth = 28; // SMB 2 Coin
    NPCTraits[138].THeight = 32;
    NPCTraits[138].IsABonus = true;
    NPCTraits[138].IsACoin = true;
    NPCTraits[152].TWidth = 32; // Sonic Ring
    NPCTraits[152].THeight = 32;
    NPCTraits[152].IsABonus = true;
    NPCTraits[152].IsACoin = true;
    For(A, 139, 147) // Veggies
    {
        NPCTraits[A].TWidth = GFXNPC[A].w;
        NPCTraits[A].THeight = GFXNPC[A].h;
        NPCTraits[A].CanWalkOn = true;
        NPCTraits[A].GrabFromTop = true;
        NPCTraits[A].WontHurt = true;
        NPCTraits[A].IsGrabbable = true;
        // NPCIsVeggie(A) = true;
    }
    NPCTraits[148].TWidth = 32; // Black Yoshi
    NPCTraits[148].THeight = 32;
    NPCTraits[148].WidthGFX = 74;
    NPCTraits[148].HeightGFX = 56;
    NPCTraits[148].FrameOffsetY = 2;
    NPCTraits[148].NoYoshi = true;
    NPCTraits[149].TWidth = 32; // Purple Yoshi
    NPCTraits[149].THeight = 32;
    NPCTraits[149].WidthGFX = 74;
    NPCTraits[149].HeightGFX = 56;
    NPCTraits[149].FrameOffsetY = 2;
    NPCTraits[149].NoYoshi = true;
    NPCTraits[150].TWidth = 32; // Pink Yoshi
    NPCTraits[150].THeight = 32;
    NPCTraits[150].WidthGFX = 74;
    NPCTraits[150].HeightGFX = 56;
    NPCTraits[150].FrameOffsetY = 2;
    NPCTraits[228].TWidth = 32; // Ice Yoshi
    NPCTraits[228].THeight = 32;
    NPCTraits[228].WidthGFX = 74;
    NPCTraits[228].HeightGFX = 56;
    NPCTraits[228].FrameOffsetY = 2;
    NPCTraits[151].TWidth = 48; // SMW Sign
    NPCTraits[151].THeight = 48;
    NPCTraits[151].NoYoshi = true;
    NPCTraits[151].WontHurt = true;
    NPCTraits[151].NoClipping = true;
    NPCTraits[154].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[154].THeight = 32;
    NPCTraits[155].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[155].THeight = 32;
    NPCTraits[156].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[156].THeight = 32;
    NPCTraits[157].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[157].THeight = 32;
    NPCTraits[158].TWidth = 32; // Mr Saturn
    NPCTraits[158].THeight = 42;
    NPCTraits[158].FrameOffsetY = 2;
    NPCTraits[158].JumpHurt = true;
    NPCTraits[159].TWidth = 32; // Diggable Dirt
    NPCTraits[159].THeight = 32;
    NPCTraits[160].TWidth = 128; // Airship Rocket
    NPCTraits[160].THeight = 32;
    NPCTraits[160].WidthGFX = 316;
    NPCTraits[160].HeightGFX = 32;
    NPCTraits[160].NoYoshi = true;
    NPCTraits[154].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[154].THeight = 32;
    NPCTraits[162].TWidth = 32; // SMW Rex
    NPCTraits[162].THeight = 60;
    NPCTraits[162].WidthGFX = 40;
    NPCTraits[162].HeightGFX = 64;
    NPCTraits[162].FrameOffsetY = 2;
    NPCTraits[163].TWidth = 32; // SMW Rex Smashed
    NPCTraits[163].THeight = 32;
    NPCTraits[163].WidthGFX = 32;
    NPCTraits[163].HeightGFX = 32;
    NPCTraits[163].FrameOffsetY = 2;
    NPCTraits[164].TWidth = 58; // SMW Mega Mole
    NPCTraits[164].THeight = 58;
    NPCTraits[164].WidthGFX = 64;
    NPCTraits[164].HeightGFX = 64;
    NPCTraits[164].FrameOffsetY = 2;
    NPCTraits[165].TWidth = 32; // SMW Goomba
    NPCTraits[165].THeight = 32;
    NPCTraits[165].FrameOffsetY = 2;
    NPCTraits[166].TWidth = 32; // SMW Stomped Goomba Goomba
    NPCTraits[166].THeight = 32;
    NPCTraits[166].FrameOffsetY = 2;
    NPCTraits[167].TWidth = 32; // SMW Para-Goomba
    NPCTraits[167].THeight = 32;
    NPCTraits[167].WidthGFX = 66;
    NPCTraits[167].HeightGFX = 50;
    NPCTraits[167].FrameOffsetY = 2;
    NPCTraits[168].TWidth = 26; // Bully
    NPCTraits[168].THeight = 30;
    NPCTraits[168].WidthGFX = 26;
    NPCTraits[168].HeightGFX = 46;
    NPCTraits[168].FrameOffsetY = 2;
    NPCTraits[169].TWidth = 32; // Tanooki Suit
    NPCTraits[169].THeight = 32;
    NPCTraits[169].FrameOffsetY = 2;
    NPCTraits[169].IsABonus = true;
    NPCTraits[170].TWidth = 32; // Hammer Suit
    NPCTraits[170].THeight = 32;
    NPCTraits[170].FrameOffsetY = 2;
    NPCTraits[170].IsABonus = true;

    NPCTraits[292].WontHurt = true; // Boomerang
    NPCTraits[292].JumpHurt = true;
    NPCTraits[292].NoClipping = true;
    NPCTraits[292].NoYoshi = true;
    NPCTraits[292].Foreground = true;
    NPCTraits[292].NoIceBall = true;

    NPCTraits[247].NoIceBall = true;

    NPCTraits[171].TWidth = 16; // Hammer Suit Hammer
    NPCTraits[171].THeight = 28;
    NPCTraits[171].WontHurt = true;
    NPCTraits[171].NoClipping = true;
    NPCTraits[171].NoYoshi = true;

    NPCTraits[172].THeight = 28; //  Green SMB1 Shell
    NPCTraits[172].IsAShell = true;
    NPCTraits[174].THeight = 28; //  Red SMB1 Shell
    NPCTraits[174].IsAShell = true;
    NPCTraits[195].IsAShell = true;

    NPCTraits[179].JumpHurt = true; // Razor Blade
    NPCTraits[179].WidthGFX = 64;
    NPCTraits[179].HeightGFX = 64;
    NPCTraits[179].TWidth = 48;
    NPCTraits[179].THeight = 48;
    NPCTraits[179].FrameOffsetY = 8;
    NPCTraits[180].JumpHurt = true; // SMW Thwomp
    NPCTraits[180].TWidth = 48;
    NPCTraits[180].THeight = 64;
    NPCTraits[241].TWidth = 32; // POW
    NPCTraits[241].THeight = 32; // POW
    NPCTraits[241].CanWalkOn = true;
    NPCTraits[241].MovesPlayer = true;
    NPCTraits[241].WontHurt = true;
    NPCTraits[241].IsABlock = true;
    NPCTraits[241].GrabFromTop = true;
    NPCTraits[241].IsGrabbable = true;

    NPCTraits[181].WontHurt = true; // SMW Bowser Statue
    NPCTraits[181].IsABlock = true;
    NPCTraits[181].CanWalkOn = true;
    NPCTraits[181].MovesPlayer = true;
    NPCTraits[181].TWidth = 48;
    NPCTraits[181].THeight = 46;
    NPCTraits[181].FrameOffsetY = 2;
    NPCTraits[197].TWidth = 48;
    NPCTraits[197].THeight = 16;
    NPCTraits[197].IsABonus = true;
    NPCTraits[182].IsABonus = true; // SMB1 Flower
    NPCTraits[183].IsABonus = true; // SMW Flower
    NPCTraits[184].IsABonus = true; // SMB1 Mushroom
    NPCTraits[185].IsABonus = true; // SMW Mushroom
    NPCTraits[186].IsABonus = true; // SMB1 1-up
    NPCTraits[187].IsABonus = true; // SMW 1-up
    NPCTraits[188].IsABonus = true; // SMW 3 up
    NPCTraits[188].TWidth = 30;
    NPCTraits[189].TWidth = 32;

    NPCTraits[201].NoYoshi = true;
    NPCTraits[202].NoYoshi = true;
    NPCTraits[202].JumpHurt = true;
    NPCTraits[201].TWidth = 80;
    NPCTraits[201].THeight = 94;

    NPCTraits[199].TWidth = 56;
    NPCTraits[199].THeight = 60;
    NPCTraits[199].WidthGFX = 130;
    NPCTraits[199].HeightGFX = 64;
    NPCTraits[199].JumpHurt = true;
    NPCTraits[199].NoClipping = true;
    NPCTraits[203].NoYoshi = true;
    NPCTraits[204].NoYoshi = true;
    NPCTraits[205].NoYoshi = true;
    NPCTraits[206].NoYoshi = true;

    NPCTraits[203].TWidth = 46;
    NPCTraits[203].THeight = 26;
    NPCTraits[203].NoYoshi = true;
    NPCTraits[203].CanWalkOn = true;
    NPCTraits[204].NoYoshi = true;
    NPCTraits[204].CanWalkOn = true;
    NPCTraits[201].JumpHurt = true;

    NPCTraits[208].NoYoshi = true;
    NPCTraits[209].NoYoshi = true;
    NPCTraits[209].CanWalkOn = true;
    NPCTraits[209].MovesPlayer = true;
    NPCTraits[209].IsABlock = true;
    NPCTraits[209].NoClipping = true;
    NPCTraits[209].TWidth = 96;
    NPCTraits[209].THeight = 106;

    NPCTraits[202].NoYoshi = true;
    NPCTraits[201].NoYoshi = true;
    NPCTraits[200].NoYoshi = true;

    NPCTraits[189].WidthGFX = 48;
    NPCTraits[189].HeightGFX = 64;
    NPCTraits[189].THeight = 64;
    NPCTraits[189].TurnsAtCliffs = true;
    NPCTraits[189].FrameOffsetY = 2;
    // NPCDefaultMovement(189) = true;
    NPCTraits[175].WidthGFX = 32;
    NPCTraits[176].WidthGFX = 32;
    NPCTraits[177].WidthGFX = 32;
    NPCTraits[173].WidthGFX = 32;
    NPCTraits[175].HeightGFX = 48;
    NPCTraits[176].HeightGFX = 48;
    NPCTraits[177].HeightGFX = 48;
    NPCTraits[173].HeightGFX = 48;

    NPCTraits[190].WidthGFX = 32;
    NPCTraits[190].HeightGFX = 32;

    NPCTraits[234].TWidth = 48;

    NPCTraits[235].HeightGFX = 48;
    NPCTraits[235].WidthGFX = 32;
    NPCTraits[235].FrameOffsetY = 16;

    NPCTraits[235].JumpHurt = true;
    NPCTraits[234].JumpHurt = true;

    NPCTraits[190].THeight = 20;
    NPCTraits[190].FrameOffsetY = 12;
    NPCTraits[194].FrameOffsetY = 2;

    NPCTraits[190].MovesPlayer = true;

    NPCTraits[175].TurnsAtCliffs = true;

    NPCTraits[178].NoYoshi = true;
    NPCTraits[179].NoYoshi = true;
    NPCTraits[180].NoYoshi = true;
    NPCTraits[181].NoYoshi = true;
    NPCTraits[189].NoYoshi = true;

    NPCTraits[190].NoYoshi = true;
    NPCTraits[192].NoYoshi = true;
    NPCTraits[192].IsABonus = true;



    // NPCIsAParaTroopa(176) = true;
    // NPCIsAParaTroopa(177) = true;
    NPCTraits[178].IsABonus = true;
    // NPCDefaultMovement(173) = true;
    // NPCDefaultMovement(175) = true;
    // NPCDefaultMovement(162) = true;
    // 'NPCDefaultMovement(163) = true;
    // 'NPCDefaultMovement(164) = true;
    // NPCDefaultMovement(165) = true;
    // NPCDefaultMovement(167) = true;
    NPCTraits[164].CanWalkOn = true;
    NPCTraits[164].IsAHit1Block = true;
    NPCTraits[160].WontHurt = true;
    NPCTraits[166].WontHurt = true;
    NPCTraits[160].CanWalkOn = true;
    NPCTraits[160].MovesPlayer = true;
    NPCTraits[160].IsABlock = true;
    // NPCDefaultMovement(160) = true;
    NPCTraits[159].WontHurt = true;
    NPCTraits[159].CanWalkOn = true;
    NPCTraits[159].IsAHit1Block = true;
    NPCTraits[159].GrabFromTop = true;
    NPCTraits[154].WontHurt = true;
    NPCTraits[154].CanWalkOn = true;
    NPCTraits[154].IsABlock = true;
    NPCTraits[154].MovesPlayer = true;
    NPCTraits[155].WontHurt = true;
    NPCTraits[155].CanWalkOn = true;
    NPCTraits[155].IsABlock = true;
    NPCTraits[155].MovesPlayer = true;
    NPCTraits[156].WontHurt = true;
    NPCTraits[156].CanWalkOn = true;
    NPCTraits[156].IsABlock = true;
    NPCTraits[156].MovesPlayer = true;
    NPCTraits[157].WontHurt = true;
    NPCTraits[157].CanWalkOn = true;
    NPCTraits[157].IsABlock = true;
    NPCTraits[157].MovesPlayer = true;
    NPCTraits[159].NoYoshi = true;


    NPCTraits[197].WontHurt = true;
    NPCTraits[197].NoYoshi = true;


    NPCTraits[154].IsGrabbable = true;
    NPCTraits[166].IsGrabbable = true;
    NPCTraits[154].GrabFromTop = true;
    NPCTraits[155].IsGrabbable = true;
    NPCTraits[155].GrabFromTop = true;
    NPCTraits[156].IsGrabbable = true;
    NPCTraits[156].GrabFromTop = true;
    NPCTraits[157].IsGrabbable = true;
    NPCTraits[157].GrabFromTop = true;
    // NPCIsABot(128) = true;
    // NPCIsYoshi(95) = true;
    // NPCIsYoshi(98) = true;
    // NPCIsYoshi(99) = true;
    // NPCIsYoshi(100) = true;
    // NPCIsYoshi(148) = true;
    // NPCIsYoshi(149) = true;
    // NPCIsYoshi(150) = true;
    // NPCIsYoshi(228) = true;
    // NPCIsBoot(35) = true;
    // NPCIsBoot(191) = true;
    // NPCIsBoot(193) = true;
    NPCTraits[5].IsAShell = true;
    NPCTraits[7].IsAShell = true;
    NPCTraits[24].IsAShell = true;
    NPCTraits[73].IsAShell = true;
    NPCTraits[9].IsABonus = true;
    NPCTraits[273].IsABonus = true;
    NPCTraits[10].IsABonus = true;
    NPCTraits[11].IsABonus = true;
    NPCTraits[14].IsABonus = true;
    NPCTraits[264].IsABonus = true;
    NPCTraits[277].IsABonus = true;
    NPCTraits[16].IsABonus = true;
    NPCTraits[33].IsABonus = true;
    NPCTraits[258].IsABonus = true;
    NPCTraits[34].IsABonus = true;
    NPCTraits[41].IsABonus = true;
    NPCTraits[75].IsABonus = true;
    NPCTraits[88].IsABonus = true;
    NPCTraits[90].IsABonus = true;
    NPCTraits[94].IsABonus = true;
    NPCTraits[198].IsABonus = true;
    NPCTraits[97].IsABonus = true;
    NPCTraits[101].IsABonus = true;
    NPCTraits[102].IsABonus = true;
    NPCTraits[103].IsABonus = true;
    NPCTraits[107].IsABonus = true;
    NPCTraits[153].IsABonus = true;
    NPCTraits[196].IsABonus = true;
    NPCTraits[10].IsACoin = true;
    NPCTraits[33].IsACoin = true;
    NPCTraits[258].IsACoin = true;
    NPCTraits[88].IsACoin = true;
    NPCTraits[103].IsACoin = true;
    // NPCIsAnExit(11) = true;
    // NPCIsAnExit(16) = true;
    // NPCIsAnExit(41) = true;
    // NPCIsAnExit(97) = true;
    // NPCIsAnExit(196) = true;
    NPCTraits[8].JumpHurt = true;
    NPCTraits[245].JumpHurt = true;
    NPCTraits[12].JumpHurt = true;
    NPCTraits[30].JumpHurt = true;
    NPCTraits[36].JumpHurt = true;
    NPCTraits[285].JumpHurt = true;
    NPCTraits[286].JumpHurt = true;
    NPCTraits[37].JumpHurt = true;
    NPCTraits[38].JumpHurt = true;
    NPCTraits[42].JumpHurt = true;
    NPCTraits[43].JumpHurt = true;
    NPCTraits[44].JumpHurt = true;
    NPCTraits[48].JumpHurt = true;
    NPCTraits[51].JumpHurt = true;
    NPCTraits[52].JumpHurt = true;
    NPCTraits[53].JumpHurt = true;
    NPCTraits[54].JumpHurt = true;
    NPCTraits[74].JumpHurt = true;
    NPCTraits[256].JumpHurt = true;
    NPCTraits[257].JumpHurt = true;
    NPCTraits[85].JumpHurt = true;
    NPCTraits[87].JumpHurt = true;
    NPCTraits[93].JumpHurt = true;
    NPCTraits[19].CanWalkOn = true;
    NPCTraits[20].CanWalkOn = true;
    NPCTraits[247].CanWalkOn = true;
    NPCTraits[21].CanWalkOn = true;
    NPCTraits[25].CanWalkOn = true;
    // NPCTraits[28].CanWalkOn = true;
    NPCTraits[31].CanWalkOn = true;
    NPCTraits[39].CanWalkOn = true;
    NPCTraits[40].CanWalkOn = true;
    NPCTraits[45].CanWalkOn = true;
    NPCTraits[46].CanWalkOn = true;
    NPCTraits[212].CanWalkOn = true;
    NPCTraits[56].CanWalkOn = true;
    NPCTraits[57].CanWalkOn = true;
    NPCTraits[58].CanWalkOn = true;
    NPCTraits[60].CanWalkOn = true;
    NPCTraits[62].CanWalkOn = true;
    NPCTraits[64].CanWalkOn = true;
    NPCTraits[66].CanWalkOn = true;
    NPCTraits[67].CanWalkOn = true;
    NPCTraits[68].CanWalkOn = true;
    NPCTraits[69].CanWalkOn = true;
    NPCTraits[70].CanWalkOn = true;
    NPCTraits[78].CanWalkOn = true;
    NPCTraits[79].CanWalkOn = true;
    NPCTraits[80].CanWalkOn = true;
    NPCTraits[81].CanWalkOn = true;
    NPCTraits[82].CanWalkOn = true;
    NPCTraits[83].CanWalkOn = true;
    NPCTraits[84].CanWalkOn = true;
    NPCTraits[91].CanWalkOn = true;
    NPCTraits[92].CanWalkOn = true;
    NPCTraits[104].CanWalkOn = true;
    NPCTraits[105].CanWalkOn = true;
    NPCTraits[106].CanWalkOn = true;
    NPCTraits[19].GrabFromTop = true;
    NPCTraits[20].GrabFromTop = true;
    NPCTraits[247].GrabFromTop = true;
    NPCTraits[25].GrabFromTop = true;
    // NPCTraits[28].GrabFromTop = true;
    NPCTraits[31].GrabFromTop = true;
    NPCTraits[40].GrabFromTop = true;
    NPCTraits[45].GrabFromTop = true;
    NPCTraits[91].GrabFromTop = true;
    NPCTraits[92].GrabFromTop = true;
    NPCTraits[8].NoClipping = true;
    NPCTraits[245].NoClipping = true;
    // NPCTraits[10].NoClipping = true;
    NPCTraits[12].NoClipping = true;
    NPCTraits[17].NoClipping = true;
    NPCTraits[18].NoClipping = true;
    NPCTraits[30].NoClipping = true;
    NPCTraits[34].NoClipping = true;
    NPCTraits[38].NoClipping = true;
    NPCTraits[42].NoClipping = true;
    NPCTraits[43].NoClipping = true;
    NPCTraits[44].NoClipping = true;
    NPCTraits[46].NoClipping = true;
    NPCTraits[212].NoClipping = true;
    NPCTraits[47].NoClipping = true;
    NPCTraits[284].NoClipping = true;
    NPCTraits[50].NoClipping = true;
    NPCTraits[51].NoClipping = true;
    NPCTraits[52].NoClipping = true;
    // NPCTraits[56].NoClipping = true;
    NPCTraits[57].NoClipping = true;
    NPCTraits[60].NoClipping = true;
    NPCTraits[62].NoClipping = true;
    NPCTraits[64].NoClipping = true;
    NPCTraits[66].NoClipping = true;
    NPCTraits[74].NoClipping = true;
    NPCTraits[256].NoClipping = true;
    NPCTraits[257].NoClipping = true;
    NPCTraits[85].NoClipping = true;
    NPCTraits[87].NoClipping = true;
    NPCTraits[91].NoClipping = true;
    NPCTraits[93].NoClipping = true;
    NPCTraits[104].NoClipping = true;
    NPCTraits[105].NoClipping = true;
    NPCTraits[106].NoClipping = true;
    NPCTraits[108].NoClipping = true;
    NPCTraits[6].TurnsAtCliffs = true;
    NPCTraits[20].TurnsAtCliffs = true;
    NPCTraits[247].TurnsAtCliffs = true;
    NPCTraits[55].TurnsAtCliffs = true;
    NPCTraits[59].TurnsAtCliffs = true;
    NPCTraits[61].TurnsAtCliffs = true;
    NPCTraits[63].TurnsAtCliffs = true;
    NPCTraits[65].TurnsAtCliffs = true;
    NPCTraits[107].TurnsAtCliffs = true;
    NPCTraits[158].TurnsAtCliffs = true;
    NPCTraits[21].MovesPlayer = true;
    NPCTraits[31].MovesPlayer = true;
    NPCTraits[32].MovesPlayer = true;
    NPCTraits[238].MovesPlayer = true;
    NPCTraits[239].MovesPlayer = true;
    NPCTraits[45].MovesPlayer = true;
    NPCTraits[46].MovesPlayer = true;
    NPCTraits[212].MovesPlayer = true;
    NPCTraits[57].MovesPlayer = true;
    NPCTraits[58].MovesPlayer = true;
    NPCTraits[67].MovesPlayer = true;
    NPCTraits[68].MovesPlayer = true;
    NPCTraits[69].MovesPlayer = true;
    NPCTraits[70].MovesPlayer = true;
    NPCTraits[78].MovesPlayer = true;
    NPCTraits[79].MovesPlayer = true;
    NPCTraits[80].MovesPlayer = true;
    NPCTraits[81].MovesPlayer = true;
    NPCTraits[82].MovesPlayer = true;
    NPCTraits[83].MovesPlayer = true;
    NPCTraits[84].MovesPlayer = true;
    NPCTraits[21].WontHurt = true;
    NPCTraits[22].WontHurt = true;
    NPCTraits[26].WontHurt = true;
    NPCTraits[31].WontHurt = true;
    NPCTraits[32].WontHurt = true;
    NPCTraits[238].WontHurt = true;
    NPCTraits[239].WontHurt = true;
    NPCTraits[35].WontHurt = true;
    NPCTraits[191].WontHurt = true;
    NPCTraits[193].WontHurt = true;
    NPCTraits[45].WontHurt = true;
    NPCTraits[46].WontHurt = true;
    NPCTraits[212].WontHurt = true;
    NPCTraits[49].WontHurt = true;
    NPCTraits[50].WontHurt = true;
    NPCTraits[56].WontHurt = true;
    NPCTraits[57].WontHurt = true;
    NPCTraits[58].WontHurt = true;
    NPCTraits[60].WontHurt = true;
    NPCTraits[62].WontHurt = true;
    NPCTraits[64].WontHurt = true;
    NPCTraits[66].WontHurt = true;
    NPCTraits[67].WontHurt = true;
    NPCTraits[68].WontHurt = true;
    NPCTraits[69].WontHurt = true;
    NPCTraits[70].WontHurt = true;
    NPCTraits[78].WontHurt = true;
    NPCTraits[79].WontHurt = true;
    NPCTraits[80].WontHurt = true;
    NPCTraits[81].WontHurt = true;
    NPCTraits[82].WontHurt = true;
    NPCTraits[83].WontHurt = true;
    NPCTraits[84].WontHurt = true;
    NPCTraits[91].WontHurt = true;
    NPCTraits[92].WontHurt = true;
    NPCTraits[95].WontHurt = true;
    NPCTraits[96].WontHurt = true;
    NPCTraits[98].WontHurt = true;
    NPCTraits[99].WontHurt = true;
    NPCTraits[100].WontHurt = true;
    NPCTraits[104].WontHurt = true;
    NPCTraits[105].WontHurt = true;
    NPCTraits[106].WontHurt = true;
    NPCTraits[107].WontHurt = true;
    NPCTraits[108].WontHurt = true;
    NPCTraits[148].WontHurt = true;
    NPCTraits[149].WontHurt = true;
    NPCTraits[150].WontHurt = true;
    NPCTraits[228].WontHurt = true;
    NPCTraits[158].WontHurt = true;
    NPCTraits[22].IsGrabbable = true;
    NPCTraits[26].IsGrabbable = true;
    NPCTraits[31].IsGrabbable = true;
    NPCTraits[32].IsGrabbable = true;
    NPCTraits[238].IsGrabbable = true;
    NPCTraits[35].IsGrabbable = true;
    NPCTraits[191].IsGrabbable = true;
    NPCTraits[193].IsGrabbable = true;
    NPCTraits[45].IsGrabbable = true;
    NPCTraits[49].IsGrabbable = true;
    NPCTraits[92].IsGrabbable = true;
    NPCTraits[96].IsGrabbable = true;
    NPCTraits[158].IsGrabbable = true;
    NPCTraits[21].IsABlock = true;
    NPCTraits[31].IsABlock = true;
    NPCTraits[32].IsABlock = true;
    NPCTraits[238].IsABlock = true;
    NPCTraits[239].IsABlock = true;
    NPCTraits[45].IsABlock = true;
    NPCTraits[46].IsABlock = true;
    NPCTraits[212].IsABlock = true;
    NPCTraits[56].IsABlock = true;
    NPCTraits[57].IsABlock = true;
    NPCTraits[58].IsABlock = true;
    NPCTraits[67].IsABlock = true;
    NPCTraits[68].IsABlock = true;
    NPCTraits[69].IsABlock = true;
    NPCTraits[70].IsABlock = true;
    NPCTraits[78].IsABlock = true;
    NPCTraits[79].IsABlock = true;
    NPCTraits[80].IsABlock = true;
    NPCTraits[81].IsABlock = true;
    NPCTraits[82].IsABlock = true;
    NPCTraits[83].IsABlock = true;
    NPCTraits[84].IsABlock = true;
    NPCTraits[190].IsABlock = true;
    NPCTraits[35].IsAHit1Block = true;
    NPCTraits[191].IsAHit1Block = true;
    NPCTraits[193].IsAHit1Block = true;
    NPCTraits[22].IsAHit1Block = true;
    NPCTraits[49].IsAHit1Block = true;
    NPCTraits[26].IsAHit1Block = true;
    NPCTraits[60].IsAHit1Block = true;
    NPCTraits[62].IsAHit1Block = true;
    NPCTraits[64].IsAHit1Block = true;
    NPCTraits[66].IsAHit1Block = true;
    NPCTraits[104].IsAHit1Block = true;
    NPCTraits[105].IsAHit1Block = true;
    NPCTraits[106].IsAHit1Block = true;
    NPCTraits[190].WontHurt = true;
    NPCTraits[190].CanWalkOn = true;
    NPCTraits[22].StandsOnPlayer = true;
    NPCTraits[26].StandsOnPlayer = true;
    NPCTraits[31].StandsOnPlayer = true;
    NPCTraits[32].StandsOnPlayer = true;
    NPCTraits[238].StandsOnPlayer = true;
    NPCTraits[239].StandsOnPlayer = true;
    NPCTraits[35].StandsOnPlayer = true;
    NPCTraits[191].StandsOnPlayer = true;
    NPCTraits[193].StandsOnPlayer = true;
    NPCTraits[49].StandsOnPlayer = true;
    NPCTraits[95].StandsOnPlayer = true;
    NPCTraits[96].StandsOnPlayer = true;
    NPCTraits[98].StandsOnPlayer = true;
    NPCTraits[99].StandsOnPlayer = true;
    NPCTraits[100].StandsOnPlayer = true;
    NPCTraits[148].StandsOnPlayer = true;
    NPCTraits[149].StandsOnPlayer = true;
    NPCTraits[150].StandsOnPlayer = true;
    NPCTraits[228].StandsOnPlayer = true;


    NPCTraits[272].CanWalkOn = true;
    NPCTraits[272].GrabFromTop = true;
    NPCTraits[272].NoFireBall = true;
    NPCTraits[272].NoClipping = true;
    NPCTraits[272].TWidth = 32;
    NPCTraits[272].THeight = 32;

    NPCTraits[208].CanWalkOn = true;
    NPCTraits[208].WontHurt = true;
    NPCTraits[208].MovesPlayer = true;
    NPCTraits[208].TWidth = 128;
    NPCTraits[208].THeight = 128;
    NPCTraits[208].IsABlock = true;

    NPCTraits[208].Foreground = true;
    NPCTraits[207].JumpHurt = true;
    NPCTraits[206].JumpHurt = true;
    NPCTraits[205].JumpHurt = true;

    NPCTraits[204].TWidth = 48;
    NPCTraits[204].THeight = 28;
    NPCTraits[204].WidthGFX = 112;
    NPCTraits[204].HeightGFX = 28;

    NPCTraits[205].WidthGFX = 44;
    NPCTraits[205].HeightGFX = 44;
    NPCTraits[205].FrameOffsetY = 6;


    NPCTraits[270].TWidth = 32;
    NPCTraits[270].THeight = 42;
    NPCTraits[270].JumpHurt = true;
    NPCTraits[270].NoClipping = true;

    NPCTraits[271].TWidth = 32;
    NPCTraits[271].THeight = 32;
    NPCTraits[271].NoClipping = true;


    NPCTraits[210].TWidth = 28;
    NPCTraits[210].NoClipping = true;
    NPCTraits[211].IsABlock = true;
    NPCTraits[211].NoClipping = true;
    NPCTraits[211].MovesPlayer = true;
    NPCTraits[211].CanWalkOn = true;
    NPCTraits[211].WontHurt = true;





    NPCTraits[12].Foreground = true;
    NPCTraits[17].Foreground = true;
    NPCTraits[18].Foreground = true;
    NPCTraits[28].Foreground = true;
    NPCTraits[30].Foreground = true;
    NPCTraits[34].Foreground = true;
    NPCTraits[38].Foreground = true;
    NPCTraits[42].Foreground = true;
    NPCTraits[43].Foreground = true;
    NPCTraits[44].Foreground = true;
    NPCTraits[47].Foreground = true;
    NPCTraits[85].Foreground = true;
    NPCTraits[86].Foreground = true;
    NPCTraits[87].Foreground = true;
    NPCTraits[108].Foreground = true;
    NPCTraits[171].Foreground = true;
    // NPCDefaultMovement(1) = true;
    // NPCDefaultMovement(2) = true;
    // NPCDefaultMovement(3) = true;
    // NPCDefaultMovement(4) = true;
    // NPCDefaultMovement(6) = true;
    // NPCDefaultMovement(19) = true;
    // NPCDefaultMovement(20) = true;
    // NPCDefaultMovement(247) = true;
    // NPCDefaultMovement(23) = true;
    // NPCDefaultMovement(27) = true;
    // NPCDefaultMovement(36) = true;
    // NPCDefaultMovement(285) = true;
    // NPCDefaultMovement(50) = true;
    // NPCDefaultMovement(53) = true;
    // NPCDefaultMovement(55) = true;
    // NPCDefaultMovement(59) = true;
    // NPCDefaultMovement(61) = true;
    // NPCDefaultMovement(63) = true;
    // NPCDefaultMovement(65) = true;
    // NPCDefaultMovement(71) = true;
    // NPCDefaultMovement(72) = true;
    // NPCDefaultMovement(77) = true;
    // NPCDefaultMovement(78) = true;
    // NPCDefaultMovement(89) = true;
    // NPCDefaultMovement(91) = true;
    // NPCDefaultMovement(107) = true;

    NPCTraits[19].NoFireBall = true;
    NPCTraits[20].NoFireBall = true;
    NPCTraits[23].NoFireBall = true;
    NPCTraits[24].NoFireBall = true;
    NPCTraits[137].NoFireBall = true;
    NPCTraits[136].NoFireBall = true;
    NPCTraits[129].NoFireBall = true;
    NPCTraits[130].NoFireBall = true;
    NPCTraits[131].NoFireBall = true;
    NPCTraits[132].NoFireBall = true;
    NPCTraits[12].NoFireBall = true;
    NPCTraits[25].NoFireBall = true;


    // link sword beam
    NPCTraits[266].TWidth = 16;
    NPCTraits[266].THeight = 8;
    NPCTraits[266].WontHurt = true;
    NPCTraits[266].JumpHurt = true;
    NPCTraits[266].NoGravity = true;

    NPCTraits[267].NoYoshi = true;
    NPCTraits[268].NoYoshi = true;
    NPCTraits[269].NoYoshi = true;
    NPCTraits[280].NoYoshi = true;
    NPCTraits[281].NoYoshi = true;
    NPCTraits[282].NoYoshi = true;


    NPCTraits[276].NoClipping = true;
    NPCTraits[276].TWidth = 16;
    NPCTraits[276].THeight = 16;
    NPCTraits[276].NoYoshi = true;
    NPCTraits[276].JumpHurt = true;

    NPCTraits[275].TWidth = 60;
    NPCTraits[275].THeight = 24;
    NPCTraits[275].WidthGFX = 64;
    NPCTraits[275].HeightGFX = 32;
    NPCTraits[275].NoFireBall = true;
    NPCTraits[275].JumpHurt = true;

    NPCTraits[278].TWidth = 32;
    NPCTraits[278].THeight = 44;
    NPCTraits[278].WontHurt = true;
    NPCTraits[278].JumpHurt = true;
    NPCTraits[278].IsGrabbable = true;

    NPCTraits[279].TWidth = 32;
    NPCTraits[279].THeight = 44;
    NPCTraits[279].WidthGFX = 96;
    NPCTraits[279].HeightGFX = 44;
    NPCTraits[279].WontHurt = true;
    NPCTraits[279].JumpHurt = true;
    NPCTraits[279].IsGrabbable = true;


// set ice
    NPCTraits[197].NoIceBall = true;
    NPCTraits[192].NoIceBall = true;

    NPCTraits[267].NoIceBall = true;
    NPCTraits[268].NoIceBall = true;
    NPCTraits[269].NoIceBall = true;
    NPCTraits[275].NoIceBall = true;
    NPCTraits[280].NoIceBall = true;
    NPCTraits[281].NoIceBall = true;
    NPCTraits[282].NoIceBall = true;
    NPCTraits[15].NoIceBall = true;
    NPCTraits[18].NoIceBall = true;
    NPCTraits[26].NoIceBall = true;
    NPCTraits[30].NoIceBall = true;
    NPCTraits[31].NoIceBall = true;
    NPCTraits[32].NoIceBall = true;
    NPCTraits[35].NoIceBall = true;
    NPCTraits[37].NoIceBall = true;
    NPCTraits[38].NoIceBall = true;
    NPCTraits[39].NoIceBall = true;
    NPCTraits[40].NoIceBall = true;
    NPCTraits[42].NoIceBall = true;
    NPCTraits[43].NoIceBall = true;
    NPCTraits[44].NoIceBall = true;
    NPCTraits[45].NoIceBall = true;
    NPCTraits[46].NoIceBall = true;
    NPCTraits[56].NoIceBall = true;
    For(A, 57, 70)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[75].NoIceBall = true;
    For(A, 78, 88)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[91].NoIceBall = true;
    NPCTraits[92].NoIceBall = true;
    NPCTraits[94].NoIceBall = true;
    For(A, 95, 108)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[133].NoIceBall = true;
    NPCTraits[134].NoIceBall = true;
    For(A, 138, 160)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[171].NoIceBall = true;
    NPCTraits[178].NoIceBall = true;
    NPCTraits[179].NoIceBall = true;
    NPCTraits[180].NoIceBall = true;
    NPCTraits[181].NoIceBall = true;
    NPCTraits[190].NoIceBall = true;
    NPCTraits[191].NoIceBall = true;
    NPCTraits[192].NoIceBall = true;
    NPCTraits[193].NoIceBall = true;
    NPCTraits[195].NoIceBall = true;
    NPCTraits[199].NoIceBall = true;
    NPCTraits[200].NoIceBall = true;
    NPCTraits[201].NoIceBall = true;
    NPCTraits[208].NoIceBall = true;
    NPCTraits[209].NoIceBall = true;
    NPCTraits[210].NoIceBall = true;
    For(A, 211, 228)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[237].NoIceBall = true;
    NPCTraits[238].NoIceBall = true;
    NPCTraits[239].NoIceBall = true;
    NPCTraits[241].NoIceBall = true;
    NPCTraits[246].NoIceBall = true;
    NPCTraits[254].NoIceBall = true;
    NPCTraits[255].NoIceBall = true;
    NPCTraits[256].NoIceBall = true;
    NPCTraits[257].NoIceBall = true;
    NPCTraits[259].NoIceBall = true;
    NPCTraits[260].NoIceBall = true;
    NPCTraits[262].NoIceBall = true;
    NPCTraits[251].NoIceBall = true;
    NPCTraits[252].NoIceBall = true;
    NPCTraits[253].NoIceBall = true;
    NPCTraits[258].NoIceBall = true;
    NPCTraits[33].NoIceBall = true;
    NPCTraits[274].NoIceBall = true;
    NPCTraits[278].NoIceBall = true;
    NPCTraits[279].NoIceBall = true;

    for(int A = 282; A <=292; ++A)
    {
        if(A != 285 && A != 286)
            NPCTraits[A].NoIceBall = true;
    }


    NPCTraits[247].THeight = 30;
    NPCTraits[247].HeightGFX = 32;
    NPCTraits[247].WidthGFX = 32;


    // NPCDefaultMovement(158) = true;


    NPCTraits[267].WidthGFX = 84;
    NPCTraits[267].HeightGFX = 62;
    NPCTraits[267].TWidth = 44;
    NPCTraits[267].THeight = 50;

    NPCTraits[268].WidthGFX = 44;
    NPCTraits[268].HeightGFX = 32;
    NPCTraits[268].TWidth = 32;
    NPCTraits[268].THeight = 28;

    NPCTraits[269].WidthGFX = 16;
    NPCTraits[269].HeightGFX = 32;
    NPCTraits[269].TWidth = 16;
    NPCTraits[269].THeight = 32;
    NPCTraits[269].NoClipping = true;
    NPCTraits[269].JumpHurt = true;

    NPCTraits[280].WidthGFX = 64;
    NPCTraits[280].HeightGFX = 64;
    NPCTraits[280].TWidth = 48;
    NPCTraits[280].THeight = 48;

    NPCTraits[281].WidthGFX = 40;
    NPCTraits[281].HeightGFX = 36;
    NPCTraits[281].TWidth = 36;
    NPCTraits[281].THeight = 32;

    NPCTraits[282].WidthGFX = 64;
    NPCTraits[282].HeightGFX = 32;
    NPCTraits[282].TWidth = 64;
    NPCTraits[282].THeight = 26;
    NPCTraits[282].FrameOffsetY = 4;
    NPCTraits[282].NoClipping = true;
    NPCTraits[282].JumpHurt = true;

    // Default NPCs that must use the canonical camera
    NPCTraits[NPCID_STONE_S3].UseDefaultCam = true;
    NPCTraits[NPCID_STONE_S4].UseDefaultCam = true;
    NPCTraits[NPCID_METALBARREL].UseDefaultCam = true;
    NPCTraits[NPCID_CANNONENEMY].UseDefaultCam = true;
    NPCTraits[NPCID_BULLET].UseDefaultCam = true;
    NPCTraits[NPCID_BIG_BULLET].UseDefaultCam = true;
    NPCTraits[NPCID_GHOST_FAST].UseDefaultCam = true;
    NPCTraits[NPCID_STATUE_S3].UseDefaultCam = true;
    NPCTraits[NPCID_STATUE_S4].UseDefaultCam = true;
    NPCTraits[NPCID_HOMING_BALL_GEN].UseDefaultCam = true;
    NPCTraits[NPCID_LAVA_MONSTER].UseDefaultCam = true;
    NPCTraits[NPCID_SPIKY_THROWER].UseDefaultCam = true;
    NPCTraits[NPCID_ITEM_THROWER].UseDefaultCam = true;

    // Default NPCs that render differently when inactive
    for(int A = 1; A <= maxNPCType; A++)
    {
        if(NPCTraits[A].IsABlock || NPCTraits[A].IsAHit1Block || NPCTraits[A].IsACoin || NPCTraits[A].IsAVine || NPCTraits[A].IsABonus)
            NPCTraits[A].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    }

    NPCTraits[NPCID_CHECKPOINT].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_ITEM_BURIED].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_CONVEYOR].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_STONE_S3].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_STONE_S4].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_HOMING_BALL_GEN].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_ITEMGOAL].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_CANNONENEMY].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_STATUE_S3].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_STATUE_S4].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_ITEM_POD].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_SPRING].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_CANNONITEM].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_KEY].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_TIME_SWITCH].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_COIN_SWITCH].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_ICE_BLOCK].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_VEHICLE].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_AXE].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_WALK_PLANT].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_FLY_BLOCK].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_FLY_CANNON].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_MAGIC_DOOR].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_DOOR_MAKER].InactiveRender = NPCTraits_t::SHOW_ALWAYS;

    NPCTraits[NPCID_LAVABUBBLE].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_PLANT_S3].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_BOTTOM_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_SIDE_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_BIG_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_PLANT_S1].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_FIRE_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_LONG_PLANT_UP].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_LONG_PLANT_DOWN].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_JUMP_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_LAVA_MONSTER].InactiveRender = NPCTraits_t::SKIP;

    NPCTraits[NPCID_BULLET].InactiveRender = NPCTraits_t::SMOKE;
    NPCTraits[NPCID_BIG_BULLET].InactiveRender = NPCTraits_t::SMOKE;
    NPCTraits[NPCID_GHOST_FAST].InactiveRender = NPCTraits_t::SMOKE;


    For(A, 1, maxBlockType)
    {
        BlockWidth[A] = 32;
        BlockHeight[A] = 32;
    }

    BlockWidth[571] = 64;
    BlockWidth[572] = 64;
    BlockWidth[615] = 64;

    BlockWidth[634] = 64;
    BlockHeight[634] = 64;

    BlockHeight[595] = 64;
    BlockHeight[596] = 64;
    BlockHeight[597] = 64;

    BlockHeight[569] = 64;
    BlockHeight[570] = 64;
    BlockHeight[571] = 64;
    BlockHeight[572] = 64;
    BlockHeight[575] = 64;

    BlockOnlyHitspot1[572] = true;
    BlockWidth[21] = 64;
    BlockWidth[22] = 64;
    BlockHeight[23] = 64;
    BlockHeight[24] = 64;
    BlockWidth[34] = 64;
    BlockWidth[35] = 64;
    BlockWidth[36] = 64;
    BlockWidth[37] = 64;
    BlockWidth[61] = 128;
    BlockHeight[61] = 128;
    BlockWidth[78] = 64;
    BlockWidth[91] = 64;
    BlockHeight[91] = 64;
    BlockWidth[92] = 128;
    BlockHeight[92] = 128;
    BlockWidth[93] = 128;
    BlockHeight[93] = 128;
    BlockWidth[103] = 64;
    BlockWidth[104] = 64;
    BlockWidth[113] = 64;
    BlockWidth[114] = 64;
    BlockWidth[125] = 64;
    BlockHeight[125] = 64;
    BlockWidth[182] = 96;
    BlockHeight[182] = 96;
    BlockWidth[184] = 64;
    BlockHeight[184] = 64;
    BlockWidth[185] = 128;
    BlockHeight[187] = 128;
    BlockWidth[187] = 128;
    BlockWidth[194] = 64;
    BlockWidth[195] = 64;
    BlockWidth[196] = 64;
    BlockWidth[197] = 64;
    BlockWidth[206] = 64;
    BlockHeight[206] = 64;
    BlockHeight[211] = 64;
    BlockHeight[212] = 64;
    BlockWidth[224] = 64;
    BlockHeight[224] = 64;
    BlockWidth[225] = 64;
    BlockHeight[225] = 64;
    BlockWidth[226] = 64;
    BlockHeight[226] = 64;
    BlockWidth[262] = 128;
    BlockHeight[262] = 128;

    BlockWidth[616] = 64;
    BlockWidth[617] = 64;
    BlockSlope[616] = -1;
    BlockSlope[617] = 1;

    BlockSlope[635] = -1;
    BlockSlope[636] = -1;
    BlockWidth[636] = 64;
    BlockSlope[637] = 1;
    BlockSlope[638] = 1;
    BlockWidth[638] = 64;


    For(A, 137, 146)
    {
        BlockWidth[A] = 64;
    }

    For(A, 147, 158)
    {
        BlockHeight[A] = 64;
    }
    BlockWidth[301] = 128;
    BlockWidth[302] = 128;
    BlockWidth[303] = 128;
    BlockWidth[304] = 128;
    BlockWidth[306] = 64;
    BlockWidth[308] = 64;
    BlockWidth[312] = 64;
    BlockWidth[314] = 64;
    BlockWidth[319] = 128;
    BlockWidth[320] = 128;
    BlockWidth[321] = 128;
    BlockWidth[322] = 128;
    BlockWidth[325] = 64;
    BlockWidth[324] = 64;
    BlockWidth[325] = 64;
    BlockWidth[324] = 64;
    BlockWidth[336] = 64;
    BlockWidth[338] = 64;
    BlockWidth[340] = 64;
    BlockWidth[342] = 64;
    BlockWidth[357] = 64;
    BlockWidth[360] = 64;
    BlockWidth[361] = 64;
    BlockWidth[364] = 64;
    BlockWidth[365] = 64;
    BlockWidth[366] = 64;
    BlockWidth[367] = 64;
    BlockWidth[368] = 64;
    BlockHeight[376] = 64;
    BlockHeight[377] = 64;
    BlockWidth[378] = 64;
    BlockHeight[378] = 64;
    BlockWidth[472] = 64;
    BlockHeight[472] = 32;
    BlockWidth[474] = 64;
    BlockHeight[474] = 32;
    BlockWidth[476] = 64;
    BlockHeight[476] = 32;
    BlockWidth[479] = 64;
    BlockHeight[479] = 32;
    BlockWidth[505] = 64;
    BlockHeight[505] = 32;
    BlockWidth[506] = 64;
    BlockHeight[506] = 64;

    BlockWidth[613] = 64;

    BlockWidth[507] = 64;
    BlockHeight[507] = 32;

    BlockWidth[599] = 64;
    BlockHeight[599] = 64;


    BlockWidth[508] = 64;
    BlockHeight[508] = 32;

    BlockWidth[529] = 32;
    BlockHeight[529] = 64;

    BlockWidth[527] = 64;
    BlockHeight[527] = 96;




    BlockWidth[534] = 48;
    BlockHeight[534] = 128;
    BlockWidth[535] = 48;
    BlockHeight[535] = 128;
    BlockWidth[536] = 128;
    BlockHeight[536] = 48;
    BlockWidth[537] = 128;
    BlockHeight[537] = 48;
    BlockWidth[540] = 64;




    BlockSlope[472] = -1;
    BlockSlope[474] = 1;
    BlockSlope2[476] = 1;
    BlockSlope2[479] = -1;


    BlockSlope2[77] = 1;
    BlockSlope2[78] = 1;
    BlockSlope2[613] = -1;
    BlockSlope2[614] = -1;


    BlockSlope[480] = -1;
    BlockSlope[482] = 1;
    BlockSlope2[486] = 1;
    BlockSlope2[485] = -1;


    BlockOnlyHitspot1[372] = true;
    BlockOnlyHitspot1[373] = true;
    BlockOnlyHitspot1[374] = true;
    BlockOnlyHitspot1[375] = true;
    BlockOnlyHitspot1[379] = true;
    BlockOnlyHitspot1[380] = true;
    BlockOnlyHitspot1[381] = true;
    BlockOnlyHitspot1[382] = true;
    BlockOnlyHitspot1[389] = true;
    BlockOnlyHitspot1[391] = true;
    BlockOnlyHitspot1[392] = true;

    BlockOnlyHitspot1[506] = true;
    BlockOnlyHitspot1[507] = true;
    BlockOnlyHitspot1[508] = true;

    BlockKills[371] = true;
    BlockKills[404] = true;
    BlockKills[406] = true;
    BlockKills[405] = true;
    BlockKills[420] = true;
    BlockHasNoMask[336] = true;
    BlockHasNoMask[337] = true;
    BlockHasNoMask[338] = true;
    BlockHasNoMask[339] = true;
    BlockHasNoMask[303] = true;
    BlockHasNoMask[304] = true;
    BlockHasNoMask[348] = true;
    BlockHasNoMask[353] = true;
    BlockHasNoMask[354] = true;
    BlockHasNoMask[355] = true;
    BlockHasNoMask[356] = true;
    BlockHasNoMask[3] = true;
    BlockHasNoMask[4] = true;
    BlockHasNoMask[13] = true;
    BlockHasNoMask[15] = true;
    BlockHasNoMask[16] = true;
    BlockHasNoMask[17] = true;
    BlockHasNoMask[19] = true;
    BlockHasNoMask[21] = true;
    BlockHasNoMask[34] = true;
    BlockHasNoMask[36] = true;
    BlockHasNoMask[23] = true;
    BlockHasNoMask[29] = true;
    BlockHasNoMask[40] = true;
    BlockHasNoMask[43] = true;
    BlockHasNoMask[46] = true;
    BlockHasNoMask[47] = true;
    BlockHasNoMask[48] = true;
    BlockHasNoMask[49] = true;
    BlockHasNoMask[50] = true;
    BlockHasNoMask[51] = true;
    BlockHasNoMask[52] = true;
    BlockHasNoMask[53] = true;
    BlockHasNoMask[54] = true;
    BlockHasNoMask[59] = true;
    BlockHasNoMask[60] = true;
    BlockHasNoMask[61] = true;
    BlockHasNoMask[63] = true;
    BlockHasNoMask[65] = true;
    BlockHasNoMask[67] = true;
    BlockHasNoMask[70] = true;
    BlockHasNoMask[71] = true;
    BlockHasNoMask[72] = true;
    BlockHasNoMask[73] = true;
    BlockHasNoMask[74] = true;
    BlockHasNoMask[75] = true;
    BlockHasNoMask[76] = true;
    BlockHasNoMask[81] = true;
    BlockHasNoMask[83] = true;
    BlockHasNoMask[84] = true;
    BlockHasNoMask[85] = true;
    BlockHasNoMask[86] = true;
    BlockHasNoMask[87] = true;
    BlockHasNoMask[91] = true;
    BlockHasNoMask[93] = true;
    BlockHasNoMask[94] = true;
    BlockHasNoMask[95] = true;
    BlockHasNoMask[96] = true;
    BlockHasNoMask[97] = true;
    BlockHasNoMask[98] = true;
    BlockHasNoMask[99] = true;
    BlockHasNoMask[100] = true;
    BlockHasNoMask[101] = true;
    BlockHasNoMask[102] = true;
    BlockHasNoMask[103] = true;
    BlockHasNoMask[111] = true;
    BlockHasNoMask[118] = true;
    BlockHasNoMask[119] = true;
    BlockHasNoMask[120] = true;
    BlockHasNoMask[121] = true;
    BlockHasNoMask[122] = true;
    BlockHasNoMask[123] = true;
    BlockHasNoMask[124] = true;
    BlockHasNoMask[125] = true;
    BlockHasNoMask[126] = true;
    BlockHasNoMask[127] = true;
    BlockHasNoMask[131] = true;
    BlockHasNoMask[134] = true;
    BlockHasNoMask[136] = true;
    BlockHasNoMask[159] = true;
    BlockHasNoMask[160] = true;
    BlockHasNoMask[166] = true;
    BlockHasNoMask[183] = true;
    BlockHasNoMask[184] = true;
    BlockHasNoMask[186] = true;
    BlockHasNoMask[187] = true;
    BlockHasNoMask[188] = true;
    BlockHasNoMask[190] = true;
    BlockHasNoMask[198] = true;
    BlockHasNoMask[199] = true;
    BlockHasNoMask[200] = true;
    BlockHasNoMask[201] = true;
    BlockHasNoMask[202] = true;
    BlockHasNoMask[203] = true;
    BlockHasNoMask[204] = true;
    BlockHasNoMask[205] = true;
    BlockHasNoMask[206] = true;
    BlockHasNoMask[216] = true;
    BlockHasNoMask[217] = true;
    BlockHasNoMask[218] = true;
    BlockHasNoMask[223] = true;
    BlockHasNoMask[226] = true;
    BlockHasNoMask[227] = true;
    BlockHasNoMask[228] = true;
    BlockHasNoMask[229] = true;
    BlockHasNoMask[230] = true;
    BlockHasNoMask[231] = true;
    BlockHasNoMask[232] = true;
    BlockHasNoMask[233] = true;
    BlockHasNoMask[234] = true;
    BlockHasNoMask[235] = true;
    BlockHasNoMask[236] = true;
    BlockHasNoMask[237] = true;
    BlockHasNoMask[238] = true;
    BlockHasNoMask[239] = true;
    BlockHasNoMask[250] = true;
    BlockHasNoMask[251] = true;
    BlockHasNoMask[252] = true;
    BlockHasNoMask[253] = true;
    BlockHasNoMask[254] = true;
    BlockHasNoMask[255] = true;
    BlockHasNoMask[256] = true;
    BlockHasNoMask[257] = true;
    BlockHasNoMask[258] = true;
    BlockHasNoMask[261] = true;
    BlockHasNoMask[262] = true;
    BlockHasNoMask[264] = true;
    BlockHasNoMask[263] = true;
    BlockHasNoMask[273] = true;
    BlockHasNoMask[272] = true;
    BlockHasNoMask[276] = true;
    BlockHasNoMask[281] = true;
    BlockHasNoMask[282] = true;
    BlockHasNoMask[283] = true;
    BlockHasNoMask[291] = true;
    BlockHasNoMask[292] = true;
    BlockHasNoMask[320] = true;
    BlockHasNoMask[322] = true;
    BlockHasNoMask[323] = true;
    BlockHasNoMask[330] = true;
    BlockHasNoMask[331] = true;
    BlockHasNoMask[369] = true;
    BlockHasNoMask[370] = true;
    BlockOnlyHitspot1[8] = true;
    BlockOnlyHitspot1[121] = true;
    BlockOnlyHitspot1[122] = true;
    BlockOnlyHitspot1[123] = true;
    BlockOnlyHitspot1[168] = true;
    BlockOnlyHitspot1[289] = true;
    BlockOnlyHitspot1[290] = true;
    BlockOnlyHitspot1[370] = true;

    BlockKills[30] = true;
    BlockHurts[109] = true;
    BlockHurts[598] = true;
    BlockHurts[110] = true;
    BlockHurts[267] = true;
    BlockHurts[268] = true;
    BlockHurts[269] = true;
    BlockHurts[511] = true;
    BlockPSwitch[4] = true;
    BlockPSwitch[60] = true;
    BlockPSwitch[89] = true;
    BlockPSwitch[188] = true;
    BlockPSwitch[280] = true;
    BlockPSwitch[293] = true;
    BlockNoClipping[172] = true;
    BlockNoClipping[175] = true;
    BlockNoClipping[178] = true;
    BlockNoClipping[181] = true;
    BlockSlope[299] = -1;
    BlockSlope[300] = 1;
    BlockSlope[301] = 1;
    BlockSlope[302] = -1;
    BlockSlope[305] = -1;
    BlockSlope[306] = -1;
    BlockSlope[307] = 1;
    BlockSlope[308] = 1;
    BlockSlope[324] = -1;
    BlockSlope[325] = 1;

    BlockSlope2[528] = 1;
    BlockSlope2[523] = -1;

    BlockSlope2[309] = 1;
    BlockSlope2[310] = -1;
    BlockSlope2[311] = 1;
    BlockSlope2[312] = 1;
    BlockSlope2[313] = -1;
    BlockSlope2[314] = -1;
    BlockSlope[315] = 1;
    BlockSlope[316] = -1;
    BlockSlope2[317] = 1;
    BlockSlope2[318] = -1;
    BlockSlope[319] = 1;
    BlockSlope[321] = -1;
    BlockSlope[326] = -1;
    BlockSlope[327] = 1;
    BlockSlope2[328] = -1;
    BlockSlope2[329] = 1;
    BlockSlope[332] = -1;
    BlockSlope[333] = 1;
    BlockSlope2[334] = -1;
    BlockSlope2[335] = 1;
    BlockSlope[340] = -1;
    BlockSlope[341] = -1;
    BlockSlope[342] = 1;
    BlockSlope[343] = 1;
    BlockSlope[357] = -1;
    BlockSlope[358] = -1;
    BlockSlope[359] = 1;
    BlockSlope[360] = 1;
    BlockSlope2[361] = 1;
    BlockSlope2[362] = 1;
    BlockSlope2[363] = -1;
    BlockSlope2[364] = -1;
    BlockSlope[365] = -1;
    BlockSlope[366] = 1;
    BlockSlope2[367] = -1;
    BlockSlope2[368] = 1;

    BlockKills[459] = true;

    BlockKills[460] = true;
    BlockKills[461] = true;
    BlockKills[462] = true;
    BlockKills[463] = true;
    BlockKills[464] = true;
    BlockKills[465] = true;
    BlockKills[466] = true;
    BlockKills[467] = true;
    BlockKills[468] = true;
    BlockKills[469] = true;
    BlockKills[470] = true;
    BlockKills[471] = true;

    BlockKills2[460] = true;
    BlockKills2[461] = true;
    BlockKills2[462] = true;
    BlockKills2[463] = true;
    BlockKills2[464] = true;
    BlockKills2[465] = true;
    BlockKills2[466] = true;
    BlockKills2[467] = true;
    BlockKills[472] = true;
    BlockKills[473] = true;
    BlockKills[474] = true;
    BlockKills[475] = true;
    BlockKills[476] = true;
    BlockKills[478] = true;
    BlockKills[479] = true;
    BlockKills2[472] = true;
    BlockKills2[474] = true;
    BlockKills2[476] = true;
    BlockKills2[479] = true;

    BlockKills[480] = true;
    BlockKills[481] = true;
    BlockKills[482] = true;
    BlockKills[483] = true;
    BlockKills[484] = true;
    BlockKills[485] = true;
    BlockKills[486] = true;
    BlockKills[487] = true;
    BlockKills2[480] = true;
    BlockKills2[482] = true;
    BlockKills2[485] = true;
    BlockKills2[486] = true;



    BlockHurts[407] = true;
    BlockHurts[408] = true;
    BlockHurts[428] = true;
    BlockHurts[429] = true;
    BlockHurts[430] = true;
    BlockHurts[431] = true;
    BlockHurts[511] = true;
    BlockOnlyHitspot1[447] = true;
    BlockOnlyHitspot1[446] = true;
    BlockOnlyHitspot1[448] = true;




    BlockWidth[604] = 64;
    BlockWidth[605] = 64;

    BlockSlope[600] = -1;
    BlockSlope[604] = -1;
    BlockSlope[601] = 1;
    BlockSlope[605] = 1;


    BlockSlope[451] = 1;
    BlockSlope[452] = -1;
    For(A, 459, 487)
    {
        BlockKills[A] = true;
    }
    BackgroundHasNoMask[187] = true;
    BackgroundHasNoMask[188] = true;
    BackgroundHasNoMask[189] = true;
    BackgroundHasNoMask[190] = true;

    BackgroundHasNoMask[172] = true;
    BackgroundHasNoMask[167] = true;
    BackgroundHasNoMask[164] = true;
    BackgroundHasNoMask[165] = true;
    BackgroundHasNoMask[158] = true;
    BackgroundHasNoMask[146] = true;
    BackgroundHasNoMask[12] = true;
    BackgroundHasNoMask[14] = true;
    BackgroundHasNoMask[15] = true;
    BackgroundHasNoMask[22] = true;
    BackgroundHasNoMask[30] = true;
    BackgroundHasNoMask[39] = true;
    BackgroundHasNoMask[40] = true;
    BackgroundHasNoMask[41] = true;
    BackgroundHasNoMask[42] = true;
    BackgroundHasNoMask[43] = true;
    BackgroundHasNoMask[44] = true;
    BackgroundHasNoMask[47] = true;
    BackgroundHasNoMask[52] = true;
    BackgroundHasNoMask[53] = true;
    BackgroundHasNoMask[55] = true;
    BackgroundHasNoMask[56] = true;
    BackgroundHasNoMask[60] = true;
    BackgroundHasNoMask[61] = true;
    BackgroundHasNoMask[64] = true;
    BackgroundHasNoMask[75] = true;
    BackgroundHasNoMask[76] = true;
    BackgroundHasNoMask[77] = true;
    BackgroundHasNoMask[78] = true;
    BackgroundHasNoMask[79] = true;
    BackgroundHasNoMask[83] = true;
    BackgroundHasNoMask[87] = true;
    BackgroundHasNoMask[88] = true;
    BackgroundHasNoMask[91] = true;
    BackgroundHasNoMask[98] = true;
    BackgroundHasNoMask[99] = true;
    BackgroundHasNoMask[107] = true;
    BackgroundHasNoMask[115] = true;
    BackgroundHasNoMask[116] = true;
    BackgroundHasNoMask[117] = true;
    BackgroundHasNoMask[118] = true;
    BackgroundHasNoMask[119] = true;
    BackgroundHasNoMask[122] = true;
    BackgroundHasNoMask[123] = true;
    BackgroundHasNoMask[124] = true;

    BackgroundHasNoMask[139] = true;
    BackgroundHasNoMask[140] = true;
    BackgroundHasNoMask[141] = true;
    BackgroundHasNoMask[144] = true;
    BackgroundHasNoMask[145] = true;
    BlockOnlyHitspot1[69] = true;


    Foreground[187] = true;
    Foreground[188] = true;
    Foreground[143] = true;
    // Foreground[165] = true;
    Foreground[145] = true;
    Foreground[23] = true;
    Foreground[24] = true;
    Foreground[25] = true;
    Foreground[45] = true;
    Foreground[46] = true;
    Foreground[49] = true;
    Foreground[50] = true;
    Foreground[51] = true;
    // Foreground[65] = true;
    Foreground[68] = true;
    Foreground[69] = true;
    Foreground[106] = true;
    Foreground[137] = true;
    Foreground[138] = true;

    Foreground[154] = true;
    Foreground[155] = true;
    Foreground[156] = true;
    Foreground[157] = true;

    BackgroundHeight[158] = 32;
    BackgroundHeight[159] = 32;

    BackgroundHeight[187] = 32;
    BackgroundHeight[188] = 32;
    BackgroundHeight[189] = 32;
    BackgroundHeight[190] = 32;



    BackgroundHeight[170] = 32;
    BackgroundHeight[171] = 32;

    BackgroundHeight[26] = 64;
    BackgroundHeight[18] = 32;
    BackgroundHeight[19] = 32;
    BackgroundHeight[20] = 32;
    BackgroundHeight[36] = 96;
    BackgroundHeight[65] = 96;

    BackgroundHeight[66] = 32;

    BackgroundHeight[68] = 64;
    BackgroundHeight[70] = 32;
    BackgroundHeight[172] = 32;
    BackgroundHeight[82] = 32;
    BackgroundHeight[100] = 32;
    BackgroundHeight[125] = 64;
    BackgroundHeight[134] = 32;
    BackgroundHeight[135] = 32;
    BackgroundHeight[136] = 32;
    BackgroundHeight[137] = 32;
    BackgroundHeight[138] = 32;
    BackgroundWidth[163] = 64;

    BackgroundWidth[173] = 112;
    BackgroundHeight[173] = 120;


    BackgroundWidth[160] = 24;
    BackgroundHeight[160] = 24;

    BackgroundWidth[161] = 64;
    BackgroundHeight[161] = 48;

    BackgroundHeight[168] = 32;
    BackgroundWidth[168] = 64;

    BackgroundHeight[169] = 64;
    BackgroundWidth[169] = 64;
    BackgroundHasNoMask[169] = true;



    For(A, 1, maxSceneType)
    {
        SceneWidth[A] = 32;
        SceneHeight[A] = 32;
        if((A >= 15 && A <= 18) || (A == 21) || (A == 24) || (A == 58) || (A == 59) || (A == 63))
        {
            SceneWidth[A] = 16;
            SceneHeight[A] = 16;
        }
    }
    SceneWidth[20] = 64;
    SceneHeight[20] = 64;
    SceneWidth[27] = 48;
    SceneHeight[27] = 16;
    SceneWidth[28] = 48;
    SceneHeight[28] = 16;
    SceneWidth[29] = 64;
    SceneHeight[29] = 16;
    SceneWidth[30] = 64;
    SceneHeight[30] = 16;
    SceneWidth[33] = 14;
    SceneHeight[33] = 14;
    SceneWidth[34] = 14;
    SceneHeight[34] = 14;
    SceneWidth[44] = 64;
    SceneHeight[50] = 48;
    SceneWidth[50] = 64;
    SceneWidth[54] = 30;
    SceneHeight[54] = 24;
    SceneWidth[55] = 30;
    SceneHeight[55] = 24;
    SceneWidth[57] = 64;
    SceneHeight[57] = 64;
    SceneWidth[60] = 48;
    SceneHeight[60] = 48;
    SceneWidth[61] = 64;
    SceneHeight[61] = 76;

    For(A, 1, maxTileType)
    {
        TileWidth[A] = 32;
        TileHeight[A] = 32;
    }
    TileWidth[8] = 64;
    TileHeight[8] = 64;
    TileWidth[9] = 96;
    TileHeight[9] = 96;
    TileWidth[12] = 64;
    TileHeight[12] = 64;
    TileWidth[13] = 96;
    TileHeight[13] = 96;
    TileWidth[27] = 128;
    TileHeight[27] = 128;
    TileWidth[325] = 64;
    TileHeight[325] = 64;
    Points[1] = 10;
    Points[2] = 100;
    Points[3] = 200;
    Points[4] = 400;
    Points[5] = 800;
    Points[6] = 1000;
    Points[7] = 2000;
    Points[8] = 4000;
    Points[9] = 8000;
    Points[10] = 1;
    Points[11] = 2;
    Points[12] = 3;
    Points[13] = 5;
    For(A, 1, maxNPCType)
    {
        if(NPCTraits[A].IsFish)
            NPCTraits[A].Foreground = true;
    }
    For(A, 1, maxEffectType)
    {
        EffectDefaults.EffectHeight[A] = EffectHeight[A];
        EffectDefaults.EffectWidth[A] = EffectWidth[A];
        if(EffectHeight[A] > 0)
        {
            EffectDefaults.EffectFrames[A] = vb6Round(double(GFXEffectHeight[A]) / EffectHeight[A]);
            if(EffectDefaults.EffectFrames[A] <= 0)
                EffectDefaults.EffectFrames[A] = 1;
        }
        else
            EffectDefaults.EffectFrames[A] = 1;
    }
    SaveNPCDefaults();
    SavePlayerDefaults();
}
