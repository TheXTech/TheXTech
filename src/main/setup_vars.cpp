/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../globals.h"
#include "../game_main.h"
#include "../custom.h"

void SetupVars()
{
    //int A;
    SetupPlayerFrames();
    for(int A = 1; A <= maxEffectType; A++)
    {
        EffectWidth[A] = 32;
        EffectHeight[A] = 32;
    }
    for(int A = 1; A <= maxNPCType; ++A)
        NPCSpeedvar[A] = 1;
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

    for(int A = 1; A <= maxNPCType; ++A)
    {
        NPCScore[A] = 2;
        NPCWidth[A] = 32;
        NPCHeight[A] = 32;
    }

    NPCWidth[291] = 22;
    NPCHeight[291] = 18;
    NPCWidthGFX[291] = 42;
    NPCHeightGFX[291] = 34;
    NPCWontHurt[291] = true;
    NPCJumpHurt[291] = true;
    NPCNoYoshi[291] = true;



    NPCNoYoshi[256] = true;
    NPCNoYoshi[257] = true;
    NPCNoYoshi[239] = true;


    NPCScore[208] = 0;
    NPCScore[15] = 7;
    NPCScore[39] = 7;
    NPCScore[86] = 9;
    NPCScore[200] = 9;
    NPCScore[201] = 9;
    NPCScore[209] = 9;
    NPCScore[29] = 5;
    NPCScore[47] = 6;
    NPCScore[284] = 6;
    NPCScore[256] = 5;
    NPCScore[257] = 5;
    NPCScore[262] = 8;
    NPCScore[267] = 8;
    NPCScore[268] = 8;
    NPCScore[280] = 8;
    NPCScore[281] = 8;
    NPCFrameOffsetY[251] = 2;
    NPCFrameOffsetY[252] = 2;
    NPCFrameOffsetY[253] = 2;
    NPCFrameOffsetY[195] = 2;
    NPCFrameOffsetY[77] = 2;

    NPCWontHurt[288] = true;
    NPCJumpHurt[288] = true;

    NPCWontHurt[287] = true;
    NPCWontHurt[195] = true;
    NPCForeground[210] = true;
    NPCForeground[230] = true;

    NPCIsCheep[28] = true;
    NPCIsCheep[229] = true;
    NPCIsCheep[230] = true;
    NPCIsCheep[232] = true;
    NPCIsCheep[233] = true;
    NPCIsCheep[234] = true;
    NPCIsCheep[236] = true;


    NPCNoClipping[289] = true;
    NPCWontHurt[289] = true;
    NPCJumpHurt[289] = true;
    NPCWidth[289] = 32;
    NPCHeight[289] = 32;
    NPCWidthGFX[289] = 32;
    NPCHeightGFX[289] = 64;


    NPCWidth[283] = 48;
    NPCHeight[283] = 48;
    NPCWidthGFX[283] = 64;
    NPCHeightGFX[283] = 64;
    NPCFrameOffsetY[283] = 8;
    NPCWontHurt[283] = true;
    NPCJumpHurt[283] = true;

    NPCWidthGFX[290] = 32;
    NPCHeightGFX[290] = 64;
    NPCForeground[290] = true;
    NPCCanWalkOn[290] = true;
    NPCIsABlock[290] = true;
    NPCWontHurt[290] = true;
    NPCNoClipping[290] = true;


    NPCCanWalkOn[263] = true;
    NPCWontHurt[263] = true;
    NPCMovesPlayer[263] = true;
    NPCIsGrabbable[263] = true;
    NPCGrabFromTop[263] = true;
    NPCIsABlock[263] = true;

    NPCCanWalkOn[262] = true;
    NPCWidth[262] = 40;
    NPCHeight[262] = 56;
    NPCWidthGFX[262] = 48;
    NPCHeightGFX[262] = 64;



    NPCIsABonus[254] = true;
    NPCIsAShell[237] = true;
    NPCWontHurt[237] = true;
    NPCCanWalkOn[237] = true;
    NPCJumpHurt[13] = true;
    NPCJumpHurt[86] = true;
    NPCJumpHurt[260] = true;



    NPCNoYoshi[263] = true;
    NPCNoYoshi[265] = true;
    NPCNoYoshi[260] = true;
    NPCNoYoshi[255] = true;

    NPCNoClipping[270] = true;

    NPCJumpHurt[246] = true;
    NPCNoClipping[246] = true;
    NPCNoYoshi[246] = true;
    NPCWidth[246] = 16;
    NPCHeight[246] = 16;

    NPCWidth[251] = 18; // TLOZ Rupee
    NPCHeight[251] = 32;
    NPCIsABonus[251] = true;
    NPCIsACoin[251] = true;

    NPCWidth[252] = 18; // TLOZ Rupee
    NPCHeight[252] = 32;
    NPCIsABonus[252] = true;
    NPCIsACoin[252] = true;

    NPCWidth[253] = 18; // TLOZ Rupee
    NPCHeight[253] = 32;
    NPCIsABonus[253] = true;
    NPCIsACoin[253] = true;

    NPCWidth[250] = 32; // TLOZ Heart
    NPCHeight[250] = 32;
    NPCIsABonus[250] = true;

    // vines
    for(int A = 213; A <= 224; ++A)
    {
        NPCIsAVine[A] = true;
        NPCWontHurt[A] = true;
        NPCNoClipping[A] = true;
        NPCJumpHurt[A] = true;
        NPCNoYoshi[A] = true;
        NPCWidth[A] = 16;
        NPCWidthGFX[A] = 32;
        NPCHeightGFX[A] = 32;
    }
    NPCHeight[223] = 24;
    NPCHeightGFX[223] = 24;
    NPCJumpHurt[231] = true;
    NPCWontHurt[225] = true;
    NPCJumpHurt[225] = true;
    NPCNoYoshi[225] = true;
    NPCWontHurt[226] = true;
    NPCJumpHurt[226] = true;
    NPCNoYoshi[226] = true;
    NPCWontHurt[227] = true;
    NPCJumpHurt[227] = true;
    NPCNoYoshi[227] = true;

    NPCNoYoshi[211] = true;

    NPCIsGrabbable[195] = true;
    NPCGrabFromTop[195] = true;

    NPCWidth[240] = 32;
    NPCHeight[240] = 32;
    NPCIsABonus[240] = 32;

    NPCWidth[248] = 32;
    NPCHeight[248] = 32;
    NPCIsABonus[248] = 32;

    NPCWidth[200] = 64;
    NPCHeight[200] = 72;
    NPCFrameOffsetY[200] = 2;
    NPCJumpHurt[200] = true;
    NPCJumpHurt[210] = true;
    NPCNoClipping[259] = true;
    NPCNoYoshi[259] = true;
    NPCJumpHurt[259] = true;
    NPCWidth[260] = 16;
    NPCHeight[260] = 16;
    NPCNoClipping[260] = true;

    NPCWidth[153] = 32; // Poison Mushroom
    NPCHeight[153] = 32;
    NPCFrameOffsetY[153] = 2;

    NPCWidth[242] = 32; // SML2 Goomba
    NPCHeight[242] = 32;
    NPCFrameOffsetY[242] = 2;
    NPCDefaultMovement[242] = true;

    NPCWidth[243] = 32; // SML2 Flying Goomba
    NPCHeight[243] = 32;
    NPCFrameOffsetY[243] = 2;
    NPCWidthGFX[243] = 56;
    NPCHeightGFX[243] = 36;

    NPCWidth[244] = 32; // SMB3 Flying Goomba
    NPCHeight[244] = 32;
    NPCFrameOffsetY[244] = 2;
    NPCWidthGFX[244] = 40;
    NPCHeightGFX[244] = 48;

    NPCWidth[255] = 20;
    NPCHeight[255] = 96;
    NPCWontHurt[255] = true;
    NPCIsABlock[255] = true;
    NPCCanWalkOn[255] = true;
    NPCMovesPlayer[255] = true;

    NPCIsAParaTroopa[244] = true;
    NPCIsAParaTroopa[243] = true;

    NPCWidth[1] = 32; // Goomba
    NPCHeight[1] = 32;
    NPCFrameOffsetY[1] = 2;

    NPCWidth[1] = 32; // Goomba
    NPCHeight[1] = 32;
    NPCFrameOffsetY[1] = 2;
    NPCWidth[2] = 32; // Red goomba
    NPCHeight[2] = 32;
    NPCFrameOffsetY[2] = 2;
    NPCWidth[3] = 32; // Flying goomba
    NPCHeight[3] = 32;
    NPCWidthGFX[3] = 40;
    NPCHeightGFX[3] = 48;
    NPCFrameOffsetY[3] = 2;
    NPCWidth[4] = 32; // Green koopa
    NPCHeight[4] = 32;
    NPCWidthGFX[4] = 32;
    NPCHeightGFX[4] = 54;
    NPCFrameOffsetY[4] = 2;
    NPCWidth[5] = 32; // Green shell
    NPCHeight[5] = 32;
    NPCFrameOffsetY[5] = 2;
    NPCWidth[6] = 32; // Red koopa
    NPCHeight[6] = 32;
    NPCWidthGFX[6] = 32;
    NPCHeightGFX[6] = 54;
    NPCFrameOffsetY[6] = 2;
    NPCWidth[7] = 32; // Red shell
    NPCHeight[7] = 32;
    NPCFrameOffsetY[7] = 2;
    NPCWidth[8] = 32; // Plant
    NPCHeight[8] = 48;
    NPCWidthGFX[8] = 32;
    NPCHeightGFX[8] = 48;
    NPCWidth[245] = 32; // SMB 3 Fire Plant
    NPCHeight[245] = 64;
    NPCWidthGFX[245] = 32;
    NPCHeightGFX[245] = 64;
    NPCFrameOffsetY[8] = 1;
    NPCWidth[9] = 32; // Mushroom
    NPCHeight[9] = 32;
    NPCFrameOffsetY[9] = 2;

    NPCWidth[273] = 32; // ? Mushroom
    NPCHeight[273] = 32;
    NPCFrameOffsetY[273] = 2;

    NPCWidth[249] = 32; // SMB2 Mushroom
    NPCHeight[249] = 32;
    NPCIsABonus[249] = true;

    NPCWidth[274] = 32; // dragon coin
    NPCHeight[274] = 50;
    NPCIsABonus[274] = true;
    NPCIsACoin[274] = true;
    NPCScore[274] = 6;

    NPCWidth[10] = 28; // SMB3 Coin
    NPCHeight[10] = 32;
    NPCWidth[11] = 32; // SMB3 Level exit
    NPCHeight[11] = 32;
    NPCWidth[12] = 28; // Big Fireball
    NPCHeight[12] = 32;
    NPCWidth[13] = 16; // Small Fireball
    NPCHeight[13] = 16;
    NPCWidth[265] = 16; // Ice Bolt
    NPCHeight[265] = 16;
    NPCNoIceBall[265] = true;
    NPCNoYoshi[13] = true;
    NPCWidth[14] = 32; // Fire Flower
    NPCHeight[14] = 32;
    NPCFrameOffsetY[14] = 2;
    NPCWidth[264] = 32; // Ice Flower
    NPCHeight[264] = 32;
    NPCFrameOffsetY[264] = 2;
    NPCWidth[277] = 32; // Ice Flower
    NPCHeight[277] = 32;
    NPCFrameOffsetY[277] = 2;
    NPCWidth[15] = 60; // Big Koopa
    NPCHeight[15] = 54;
    NPCWidthGFX[15] = 68;
    NPCHeightGFX[15] = 54;
    NPCFrameOffsetY[15] = 2;
    NPCNoYoshi[15] = true;
    NPCWidth[16] = 32; // Boss Exit
    NPCHeight[16] = 32;
    NPCWidth[17] = 32; // Bullet Bill
    NPCHeight[17] = 28;
    NPCWidth[18] = 128; // Giant Bullet Bill
    NPCHeight[18] = 128;
    NPCNoYoshi[18] = true;
    NPCWidth[19] = 32; // Red Shy guy
    NPCHeight[19] = 32;
    NPCWidth[20] = 32; // Blue Shy guy
    NPCHeight[20] = 32;
    NPCWidth[247] = 32; // Cactus Thing
    NPCHeight[247] = 32;
    NPCWidth[21] = 32; // Bullet Bill Shooter
    NPCHeight[21] = 32;
    NPCNoYoshi[21] = true;
    NPCWidth[22] = 32; // Bullet Bill Gun
    NPCHeight[22] = 32;
    NPCFrameOffsetY[22] = 2;
    NPCWidth[23] = 32; // Hard thing
    NPCHeight[23] = 32;
    NPCFrameOffsetY[23] = 2;
    NPCWidth[24] = 32; // Hard Thing shell
    NPCHeight[24] = 32;
    NPCFrameOffsetY[24] = 2;
    NPCWidth[25] = 32; // Bouncy Start Thing
    NPCHeight[25] = 32;
    NPCWidth[26] = 32; // Spring
    NPCHeight[26] = 32;
    NPCWidthGFX[26] = 32;
    NPCHeightGFX[26] = 32;
    NPCFrameOffsetY[26] = 2;
    NPCWidth[27] = 32; // Grey goomba
    NPCHeight[27] = 32;
    NPCWidth[28] = 32; // Red Jumping Fish
    NPCHeight[28] = 32;
    NPCWidth[29] = 32; // Hammer Bro
    NPCHeight[29] = 48;
    NPCFrameOffsetY[29] = 2;
    NPCWidth[30] = 32; // Hammer
    NPCHeight[30] = 32;
    NPCNoYoshi[30] = true;
    NPCWidth[31] = 32; // Key
    NPCHeight[31] = 32;
    NPCFrameOffsetY[31] = 1;
    NPCWidth[32] = 32; // P Switch
    NPCHeight[32] = 32;
    NPCFrameOffsetY[32] = 2;
    NPCWidth[238] = 32; // P Switch Time
    NPCHeight[238] = 32;
    NPCFrameOffsetY[238] = 2;
    NPCWidth[239] = 32; // Push down thing
    NPCHeight[239] = 32;
    NPCFrameOffsetY[239] = 2;
    NPCWidth[33] = 24; // SMW Coin
    NPCHeight[33] = 32;
    NPCWidth[258] = 24; // SMW Blue Coin
    NPCHeight[258] = 32;
    NPCWidth[34] = 32; // Leaf
    NPCHeight[34] = 32;
    NPCWidth[35] = 32; // Goombas Shoe
    NPCHeight[35] = 32;
    NPCFrameOffsetY[35] = 2;
    NPCFrameOffsetY[191] = 2;
    NPCFrameOffsetY[193] = 2;

    NPCWidth[285] = 32; // Spiney
    NPCHeight[285] = 32;
    NPCFrameOffsetY[285] = 2;
    NPCWidth[286] = 32; // Falling Spiney
    NPCHeight[286] = 32;
    NPCFrameOffsetY[286] = 2;

    NPCWidth[36] = 32; // Spiney
    NPCHeight[36] = 32;
    NPCFrameOffsetY[36] = 2;
    NPCWidth[37] = 48; // Thwomp
    NPCHeight[37] = 64;
    NPCNoYoshi[37] = true;
    NPCWidth[38] = 32; // Boo
    NPCHeight[38] = 32;
    NPCNoYoshi[38] = true;
    NPCWidth[39] = 32; // Birdo
    NPCHeight[39] = 60;
    NPCWidthGFX[39] = 40;
    NPCHeightGFX[39] = 72;
    NPCFrameOffsetY[39] = 2;
    NPCFrameOffsetX[39] = 3;
    NPCNoYoshi[39] = true;
    NPCWidth[40] = 32; // egg
    NPCHeight[40] = 24;
    NPCWidth[41] = 32; // smb2 exit(birdo)
    NPCHeight[41] = 32;
    NPCWidth[42] = 32; // ghost 1
    NPCHeight[42] = 32;
    NPCNoYoshi[42] = true;
    NPCWidth[43] = 32; // ghost 2
    NPCHeight[43] = 32;
    NPCNoYoshi[43] = true;
    NPCWidth[44] = 128; // big ghost
    NPCHeight[44] = 120;
    NPCWidthGFX[44] = 140;
    NPCHeightGFX[44] = 128;
    NPCNoYoshi[44] = true;
    NPCWidth[45] = 32; // ice block
    NPCHeight[45] = 32;
    NPCWidth[46] = 32; // falling block
    NPCHeight[46] = 32;
    NPCNoYoshi[46] = true;
    NPCNoYoshi[212] = true;
    NPCWidth[47] = 32; // lakitu
    NPCHeight[47] = 48;
    NPCWidthGFX[47] = 32;
    NPCHeightGFX[47] = 64;
    NPCWidth[284] = 40; // smw lakitu
    NPCHeight[284] = 48;
    NPCFrameOffsetY[284] = 6;
    NPCWidthGFX[284] = 56;
    NPCHeightGFX[284] = 72;

    NPCWidth[48] = 32; // unripe spiney
    NPCHeight[48] = 32;
    NPCWidth[49] = 32; // killer pipe
    NPCHeight[49] = 32;
    NPCFrameOffsetY[49] = 2;
    NPCWidth[50] = 48; // killer plant
    NPCHeight[50] = 32;
    NPCFrameOffsetY[50] = 2;
    NPCNoYoshi[50] = true;
    NPCWidth[51] = 32; // down piranha plant
    NPCHeight[51] = 64;
    NPCWidth[52] = 48; // left.right piranha plant
    NPCHeight[52] = 32;
    NPCWidth[53] = 32; // mr crabs
    NPCHeight[53] = 32;
    NPCFrameOffsetY[53] = 2;
    NPCWidth[54] = 32; // bee thing
    NPCHeight[54] = 32;
    NPCFrameOffsetY[54] = 2;
    NPCWidth[55] = 32; // nekkid koopa
    NPCHeight[55] = 32;
    NPCFrameOffsetY[55] = 2;
    NPCWidth[56] = 128; // koopa clown car
    NPCHeight[56] = 128;
    NPCNoYoshi[56] = true;
    NPCWidth[57] = 32; // smb3 conveyer belt
    NPCHeight[57] = 32;
    NPCNoYoshi[57] = true;
    NPCWidth[58] = 32; // smb3 barrel
    NPCHeight[58] = 32;
    NPCNoYoshi[58] = true;
    NPCWidth[59] = 32; // purple goomba
    NPCHeight[59] = 32;
    NPCFrameOffsetY[59] = 2;
    NPCWidth[60] = 96; // purple platform
    NPCHeight[60] = 32;
    NPCNoYoshi[60] = true;
    NPCWidth[61] = 32; // blue goomba
    NPCHeight[61] = 32;
    NPCFrameOffsetY[61] = 2;
    NPCWidth[62] = 96; // blue platform
    NPCHeight[62] = 32;
    NPCNoYoshi[62] = true;
    NPCWidth[63] = 32; // green goomba
    NPCHeight[63] = 32;
    NPCFrameOffsetY[63] = 2;
    NPCWidth[64] = 96; // green platform
    NPCHeight[64] = 32;
    NPCNoYoshi[64] = true;
    NPCWidth[65] = 32; // red goomba
    NPCHeight[65] = 32;
    NPCFrameOffsetY[65] = 2;
    NPCWidth[66] = 96; // red platform
    NPCHeight[66] = 32;
    NPCNoYoshi[66] = true;
    NPCWidth[67] = 128; // grey pipe x
    NPCHeight[67] = 32;
    NPCNoYoshi[67] = true;
    NPCWidth[68] = 256; // big grey pipe x
    NPCHeight[68] = 32;
    NPCNoYoshi[68] = true;
    NPCWidth[69] = 32; // grey pipe y
    NPCHeight[69] = 127;//.9; [IDK WY REDIGIT SET THESE!!!]
    NPCNoYoshi[69] = true;
    NPCWidth[70] = 32; // big grey pipe y
    NPCHeight[70] = 255;//.9;
    NPCNoYoshi[70] = true;
    NPCWidth[71] = 48; // giant goomba
    NPCHeight[71] = 46;
    NPCFrameOffsetY[71] = 2;
    NPCWidth[72] = 48; // giant green koopa
    NPCHeight[72] = 48;
    NPCFrameOffsetY[72] = 2;
    NPCWidthGFX[72] = 48;
    NPCHeightGFX[72] = 62;
    NPCWidth[73] = 44; // giant green shell
    NPCHeight[73] = 44;
    NPCFrameOffsetY[73] = 2;
    NPCWidth[74] = 48; // giant pirhana plant
    NPCHeight[74] = 64;
    NPCFrameOffsetY[74] = 2;

    NPCWidth[256] = 48; // gianter pirhana plant
    NPCHeight[256] = 128;
    NPCFrameOffsetY[256] = 2;

    NPCWidth[257] = 48; // gianter pirhana plant
    NPCHeight[257] = 128;

    NPCWidth[75] = 38; // toad
    NPCHeight[75] = 54;
    NPCFrameOffsetY[75] = 2;
    NPCWidthGFX[75] = 38;
    NPCHeightGFX[75] = 58;
    NPCIsToad[75] = true;
    NPCWidth[76] = 32; // flying green koopa
    NPCHeight[76] = 32;
    NPCFrameOffsetY[76] = 2;
    NPCWidthGFX[76] = 32;
    NPCHeightGFX[76] = 56;
    NPCIsAParaTroopa[76] = true;
    NPCWidth[161] = 32; // flying red koopa
    NPCHeight[161] = 32;
    NPCFrameOffsetY[161] = 2;
    NPCWidthGFX[161] = 32;
    NPCHeightGFX[161] = 56;
    NPCIsAParaTroopa[161] = true;
    NPCWidth[77] = 32; // black ninja
    NPCHeight[77] = 32;
    NPCFrameOffsetY[73] = 2;
    NPCWidth[78] = 128; // tank treads
    NPCHeight[78] = 32;
    NPCNoYoshi[78] = true;
    NPCWidth[79] = 64; // tank parts
    NPCHeight[79] = 32;
    NPCNoYoshi[79] = true;
    NPCWidth[80] = 128; // tank parts
    NPCHeight[80] = 32;
    NPCNoYoshi[80] = true;
    NPCWidth[81] = 128; // tank parts
    NPCHeight[81] = 32;
    NPCNoYoshi[81] = true;
    NPCWidth[82] = 128; // tank parts
    NPCHeight[82] = 32;
    NPCNoYoshi[82] = true;
    NPCWidth[83] = 256; // tank parts
    NPCHeight[83] = 32;
    NPCNoYoshi[83] = true;
    NPCWidth[84] = 32; // bowser statue
    NPCHeight[84] = 64;
    NPCNoYoshi[84] = true;
    NPCWidth[85] = 32; // statue fireball
    NPCHeight[85] = 16;
    NPCNoYoshi[85] = true;
    NPCWidth[86] = 62; // smb3 bowser
    NPCHeight[86] = 80;
    NPCWidthGFX[86] = 64;
    NPCHeightGFX[86] = 80;
    NPCFrameOffsetY[86] = 2;
    NPCNoYoshi[86] = true;
    NPCWidth[87] = 48; // smb3 bowser fireball
    NPCHeight[87] = 32;
    NPCNoYoshi[87] = true;
    NPCWidth[88] = 20; // smb1 coin
    NPCHeight[88] = 32;
    NPCWidth[89] = 32; // smb1 brown goomba
    NPCHeight[89] = 32;
    NPCFrameOffsetY[89] = 2;
    NPCWidth[90] = 32; // 1 up
    NPCHeight[90] = 32;
    NPCFrameOffsetY[90] = 2;
    NPCIsAHit1Block[90] = true;
    NPCWidth[91] = 32; // grab grass
    NPCHeight[91] = 16;
    NPCFrameOffsetY[91] = -16;
    NPCWidth[92] = 32; // turnip
    NPCHeight[92] = 32;
    NPCIsVeggie[92] = true;
    NPCWidth[93] = 32; // SMB1 Plant
    NPCHeight[93] = 48;
    NPCWidth[94] = 32; // Inert Toad
    NPCHeight[94] = 54;

    NPCWidth[198] = 32; // Peach
    NPCHeight[198] = 64;
    NPCFrameOffsetY[198] = 2;
    NPCFrameOffsetY[94] = 2;
    NPCIsToad[94] = true;
    NPCIsToad[198] = true;
    NPCWidth[95] = 32; // Green Yoshi
    NPCHeight[95] = 32;
    NPCWidthGFX[95] = 74;
    NPCHeightGFX[95] = 56;
    NPCFrameOffsetY[95] = 2;
    NPCNoYoshi[95] = true;
    NPCWidth[96] = 32; // Yoshi Egg
    NPCHeight[96] = 32;
    NPCWidth[97] = 32; // SMB3 Star
    NPCHeight[97] = 32;
    NPCWidth[98] = 32; // Blue Yoshi
    NPCHeight[98] = 32;
    NPCWidthGFX[98] = 74;
    NPCHeightGFX[98] = 56;
    NPCFrameOffsetY[98] = 2;
    NPCWidth[99] = 32; // Yellow Yoshi
    NPCHeight[99] = 32;
    NPCWidthGFX[99] = 74;
    NPCHeightGFX[99] = 56;
    NPCFrameOffsetY[99] = 2;
    NPCWidth[100] = 32; // Red Yoshi
    NPCHeight[100] = 32;
    NPCWidthGFX[100] = 74;
    NPCHeightGFX[100] = 56;
    NPCFrameOffsetY[100] = 2;
    NPCWidth[101] = 28; // Luigi
    NPCHeight[101] = 62;
    NPCFrameOffsetY[101] = 2;
    NPCIsToad[101] = true;
    NPCWidth[102] = 32; // Link
    NPCHeight[102] = 64;
    NPCFrameOffsetY[102] = 2;
    NPCIsToad[102] = true;
    NPCWidth[103] = 28; // SMB3 Red Coin
    NPCHeight[103] = 32;
    NPCWidth[104] = 96; // SMB3 Platform
    NPCHeight[104] = 32;
    NPCNoYoshi[104] = true;
    NPCWidth[105] = 128; // SMW Falling Platform
    NPCHeight[105] = 22;
    NPCNoYoshi[105] = true;
    NPCWidth[106] = 128; // SMB Platform
    NPCHeight[106] = 16;
    NPCNoYoshi[106] = true;
    NPCWidth[107] = 24; // Bob-omb buddy
    NPCHeight[107] = 38;
    NPCWidthGFX[107] = 48;
    NPCHeightGFX[107] = 38;
    NPCFrameOffsetY[107] = 2;
    NPCIsToad[107] = true;
    NPCWidth[108] = 32; // Yoshi Fireball
    NPCHeight[108] = 32;
    NPCNoYoshi[108] = true;
    NPCWidth[109] = 32; // SMW Green Koopa
    NPCHeight[109] = 32;
    NPCWidthGFX[109] = 32;
    NPCHeightGFX[109] = 54;
    NPCFrameOffsetY[109] = 2;
    NPCDefaultMovement[109] = true;
    NPCWidth[110] = 32; // SMW Red Koopa
    NPCHeight[110] = 32;
    NPCWidthGFX[110] = 32;
    NPCHeightGFX[110] = 54;
    NPCFrameOffsetY[110] = 2;
    NPCTurnsAtCliffs[110] = true;
    NPCDefaultMovement[110] = true;
    NPCWidth[111] = 32; // SMW Blue Koopa
    NPCHeight[111] = 32;
    NPCWidthGFX[111] = 32;
    NPCHeightGFX[111] = 54;
    NPCFrameOffsetY[111] = 2;
    NPCTurnsAtCliffs[111] = true;
    NPCDefaultMovement[111] = true;
    NPCWidth[112] = 32; // SMW Yellow Koopa
    NPCHeight[112] = 32;
    NPCWidthGFX[112] = 32;
    NPCHeightGFX[112] = 54;
    NPCFrameOffsetY[112] = 2;
    NPCTurnsAtCliffs[112] = true;
    NPCDefaultMovement[112] = true;
    NPCWidth[113] = 32; // SMW Green Shell
    NPCHeight[113] = 32;
    NPCFrameOffsetY[113] = 2;
    NPCIsAShell[113] = true;
    NPCWidth[114] = 32; // SMW Red Shell
    NPCHeight[114] = 32;
    NPCFrameOffsetY[114] = 2;
    NPCIsAShell[114] = true;
    NPCWidth[115] = 32; // SMW Blue Shell
    NPCHeight[115] = 32;
    NPCFrameOffsetY[115] = 2;
    NPCIsAShell[115] = true;
    NPCWidth[116] = 32; // SMW Yellow Shell
    NPCHeight[116] = 32;
    NPCFrameOffsetY[116] = 2;
    NPCIsAShell[116] = true;
    NPCWidth[117] = 32; // SMW Green Beach Koopa
    NPCHeight[117] = 32;
    NPCFrameOffsetY[117] = 2;
    NPCDefaultMovement[117] = true;
    NPCWidth[118] = 32; // SMW Red Beach Koopa
    NPCHeight[118] = 32;
    NPCFrameOffsetY[118] = 2;
    NPCTurnsAtCliffs[118] = true;
    NPCDefaultMovement[118] = true;
    NPCWidth[119] = 32; // SMW Blue Beach Koopa
    NPCHeight[119] = 32;
    NPCFrameOffsetY[119] = 2;
    NPCTurnsAtCliffs[119] = true;
    NPCDefaultMovement[119] = true;
    NPCWidth[120] = 32; // SMW Yellow Beach Koopa
    NPCHeight[120] = 32;
    NPCFrameOffsetY[120] = 2;
    NPCTurnsAtCliffs[120] = true;
    NPCDefaultMovement[120] = true;
    NPCWidth[121] = 32; // SMW Green Para-Koopa
    NPCHeight[121] = 32;
    NPCWidthGFX[121] = 56;
    NPCHeightGFX[121] = 56;
    NPCFrameOffsetY[121] = 2;
    NPCIsAParaTroopa[121] = true;
    NPCWidth[122] = 32; // SMW Red Para-Koopa
    NPCHeight[122] = 32;
    NPCWidthGFX[122] = 56;
    NPCHeightGFX[122] = 56;
    NPCFrameOffsetY[122] = 2;
    NPCIsAParaTroopa[122] = true;
    NPCWidth[123] = 32; // SMW Blue Para-Koopa
    NPCHeight[123] = 32;
    NPCWidthGFX[123] = 56;
    NPCHeightGFX[123] = 56;
    NPCFrameOffsetY[123] = 2;
    NPCIsAParaTroopa[123] = true;
    NPCWidth[124] = 32; // SMW Yellow Para-Koopa
    NPCHeight[124] = 32;
    NPCWidthGFX[124] = 56;
    NPCHeightGFX[124] = 56;
    NPCFrameOffsetY[124] = 2;
    NPCIsAParaTroopa[124] = true;
    NPCWidth[125] = 36; // Rat Head
    NPCHeight[125] = 56;
    NPCWidthGFX[125] = 36;
    NPCHeightGFX[125] = 66;
    NPCFrameOffsetY[125] = 2;
    NPCJumpHurt[261] = true;
    NPCNoFireBall[261] = true;
    NPCNoFireBall[17] = true;
    NPCDefaultMovement[125] = true;
    NPCNoYoshi[125] = true;
    NPCWidth[126] = 32; // Blue Bot
    NPCHeight[126] = 34;
    NPCWidthGFX[126] = 32;
    NPCHeightGFX[126] = 26;
    NPCIsABot[126] = true;
    NPCWidth[127] = 32; // Cyan Bot
    NPCHeight[127] = 34;
    NPCWidthGFX[127] = 32;
    NPCHeightGFX[127] = 26;
    NPCIsABot[127] = true;
    NPCWidth[128] = 32; // Red Bot
    NPCHeight[128] = 34;
    NPCWidthGFX[128] = 32;
    NPCHeightGFX[128] = 26;
    NPCWidth[129] = 32; // SMB2 Jumpy guy
    NPCHeight[129] = 32;
    NPCCanWalkOn[129] = true;
    NPCGrabFromTop[129] = true;
    NPCDefaultMovement[129] = true;
    NPCWidth[130] = 32; // Red Sniffit
    NPCHeight[130] = 32;
    NPCCanWalkOn[130] = true;
    NPCGrabFromTop[130] = true;
    NPCDefaultMovement[130] = true;
    NPCWidth[131] = 32; // Blue Sniffit
    NPCHeight[131] = 32;
    NPCCanWalkOn[131] = true;
    NPCGrabFromTop[131] = true;
    NPCDefaultMovement[131] = true;
    NPCTurnsAtCliffs[131] = true;
    NPCWidth[132] = 32; // Grey Sniffit
    NPCHeight[132] = 32;
    NPCCanWalkOn[132] = true;
    NPCGrabFromTop[132] = true;
    NPCDefaultMovement[132] = true;
    NPCWidth[133] = 16; // Bullet
    NPCHeight[133] = 16;
    NPCNoYoshi[133] = true;
    NPCJumpHurt[133] = true;
    NPCWidth[134] = 24; // SMB2 Bomb
    NPCHeight[134] = 24;
    NPCWidthGFX[134] = 40;
    NPCHeightGFX[134] = 40;
    NPCWontHurt[134] = true;
    NPCIsGrabbable[134] = true;
    NPCGrabFromTop[134] = true;
    NPCCanWalkOn[134] = true;
    NPCWidth[135] = 32; // SMB2 Bob-om
    NPCHeight[135] = 32;
    NPCGrabFromTop[135] = true;
    NPCCanWalkOn[135] = true;
    NPCDefaultMovement[135] = true;
    NPCWidth[136] = 32; // SMB3 Bob-om
    NPCHeight[136] = 32;
    NPCFrameOffsetY[136] = 2;
    NPCDefaultMovement[136] = true;
    NPCTurnsAtCliffs[136] = true;
    NPCWidth[137] = 32; // SMB3 Bomb
    NPCHeight[137] = 28;
    NPCFrameOffsetY[137] = 2;
    NPCIsGrabbable[137] = true;
    NPCWidth[138] = 28; // SMB 2 Coin
    NPCHeight[138] = 32;
    NPCIsABonus[138] = true;
    NPCIsACoin[138] = true;
    NPCWidth[152] = 32; // Sonic Ring
    NPCHeight[152] = 32;
    NPCIsABonus[152] = true;
    NPCIsACoin[152] = true;
    For(A, 139, 147) // Veggies
    {
        NPCWidth[A] = GFXNPCWidth[A];
        NPCHeight[A] = GFXNPCHeight[A];
        NPCCanWalkOn[A] = true;
        NPCGrabFromTop[A] = true;
        NPCWontHurt[A] = true;
        NPCIsGrabbable[A] = true;
        NPCIsVeggie[A] = true;
    }
    NPCWidth[148] = 32; // Black Yoshi
    NPCHeight[148] = 32;
    NPCWidthGFX[148] = 74;
    NPCHeightGFX[148] = 56;
    NPCFrameOffsetY[148] = 2;
    NPCNoYoshi[148] = true;
    NPCWidth[149] = 32; // Purple Yoshi
    NPCHeight[149] = 32;
    NPCWidthGFX[149] = 74;
    NPCHeightGFX[149] = 56;
    NPCFrameOffsetY[149] = 2;
    NPCNoYoshi[149] = true;
    NPCWidth[150] = 32; // Pink Yoshi
    NPCHeight[150] = 32;
    NPCWidthGFX[150] = 74;
    NPCHeightGFX[150] = 56;
    NPCFrameOffsetY[150] = 2;
    NPCWidth[228] = 32; // Ice Yoshi
    NPCHeight[228] = 32;
    NPCWidthGFX[228] = 74;
    NPCHeightGFX[228] = 56;
    NPCFrameOffsetY[228] = 2;
    NPCWidth[151] = 48; // SMW Sign
    NPCHeight[151] = 48;
    NPCNoYoshi[151] = true;
    NPCWontHurt[151] = true;
    NPCNoClipping[151] = true;
    NPCWidth[154] = 32; // SMB2 Mushroom Block
    NPCHeight[154] = 32;
    NPCWidth[155] = 32; // SMB2 Mushroom Block
    NPCHeight[155] = 32;
    NPCWidth[156] = 32; // SMB2 Mushroom Block
    NPCHeight[156] = 32;
    NPCWidth[157] = 32; // SMB2 Mushroom Block
    NPCHeight[157] = 32;
    NPCWidth[158] = 32; // Mr Saturn
    NPCHeight[158] = 42;
    NPCFrameOffsetY[158] = 2;
    NPCJumpHurt[158] = true;
    NPCWidth[159] = 32; // Diggable Dirt
    NPCHeight[159] = 32;
    NPCWidth[160] = 128; // Airship Rocket
    NPCHeight[160] = 32;
    NPCWidthGFX[160] = 316;
    NPCHeightGFX[160] = 32;
    NPCNoYoshi[160] = true;
    NPCWidth[154] = 32; // SMB2 Mushroom Block
    NPCHeight[154] = 32;
    NPCWidth[162] = 32; // SMW Rex
    NPCHeight[162] = 60;
    NPCWidthGFX[162] = 40;
    NPCHeightGFX[162] = 64;
    NPCFrameOffsetY[162] = 2;
    NPCWidth[163] = 32; // SMW Rex Smashed
    NPCHeight[163] = 32;
    NPCWidthGFX[163] = 32;
    NPCHeightGFX[163] = 32;
    NPCFrameOffsetY[163] = 2;
    NPCWidth[164] = 58; // SMW Mega Mole
    NPCHeight[164] = 58;
    NPCWidthGFX[164] = 64;
    NPCHeightGFX[164] = 64;
    NPCFrameOffsetY[164] = 2;
    NPCWidth[165] = 32; // SMW Goomba
    NPCHeight[165] = 32;
    NPCFrameOffsetY[165] = 2;
    NPCWidth[166] = 32; // SMW Stomped Goomba Goomba
    NPCHeight[166] = 32;
    NPCFrameOffsetY[166] = 2;
    NPCWidth[167] = 32; // SMW Para-Goomba
    NPCHeight[167] = 32;
    NPCWidthGFX[167] = 66;
    NPCHeightGFX[167] = 50;
    NPCFrameOffsetY[167] = 2;
    NPCWidth[168] = 26; // Bully
    NPCHeight[168] = 30;
    NPCWidthGFX[168] = 26;
    NPCHeightGFX[168] = 46;
    NPCFrameOffsetY[168] = 2;
    NPCWidth[169] = 32; // Tanooki Suit
    NPCHeight[169] = 32;
    NPCFrameOffsetY[169] = 2;
    NPCIsABonus[169] = true;
    NPCWidth[170] = 32; // Hammer Suit
    NPCHeight[170] = 32;
    NPCFrameOffsetY[170] = 2;
    NPCIsABonus[170] = true;

    NPCWontHurt[292] = true; // Boomerang
    NPCJumpHurt[292] = true;
    NPCNoClipping[292] = true;
    NPCNoYoshi[292] = true;
    NPCForeground[292] = true;
    NPCNoIceBall[292] = true;

    NPCNoIceBall[247] = true;

    NPCWidth[171] = 16; // Hammer Suit Hammer
    NPCHeight[171] = 28;
    NPCWontHurt[171] = true;
    NPCNoClipping[171] = true;
    NPCNoYoshi[171] = true;

    NPCHeight[172] = 28; //  Green SMB1 Shell
    NPCIsAShell[172] = true;
    NPCHeight[174] = 28; //  Red SMB1 Shell
    NPCIsAShell[174] = true;
    NPCIsAShell[195] = true;

    NPCJumpHurt[179] = true; // Razor Blade
    NPCWidthGFX[179] = 64;
    NPCHeightGFX[179] = 64;
    NPCWidth[179] = 48;
    NPCHeight[179] = 48;
    NPCFrameOffsetY[179] = 8;
    NPCJumpHurt[180] = true; // SMW Thwomp
    NPCWidth[180] = 48;
    NPCHeight[180] = 64;
    NPCWidth[241] = 32; // POW
    NPCHeight[241] = 32; // POW
    NPCCanWalkOn[241] = true;
    NPCMovesPlayer[241] = true;
    NPCWontHurt[241] = true;
    NPCIsABlock[241] = true;
    NPCGrabFromTop[241] = true;
    NPCIsGrabbable[241] = true;

    NPCWontHurt[181] = true; // SMW Bowser Statue
    NPCIsABlock[181] = true;
    NPCCanWalkOn[181] = true;
    NPCMovesPlayer[181] = true;
    NPCWidth[181] = 48;
    NPCHeight[181] = 46;
    NPCFrameOffsetY[181] = 2;
    NPCWidth[197] = 48;
    NPCHeight[197] = 16;
    NPCIsABonus[197] = true;
    NPCIsABonus[182] = true; // SMB1 Flower
    NPCIsABonus[183] = true; // SMW Flower
    NPCIsABonus[184] = true; // SMB1 Mushroom
    NPCIsABonus[185] = true; // SMW Mushroom
    NPCIsABonus[186] = true; // SMB1 1-up
    NPCIsABonus[187] = true; // SMW 1-up
    NPCIsABonus[188] = true; // SMW 3 up
    NPCWidth[188] = 30;
    NPCWidth[189] = 32;

    NPCNoYoshi[201] = true;
    NPCNoYoshi[202] = true;
    NPCJumpHurt[202] = true;
    NPCWidth[201] = 80;
    NPCHeight[201] = 94;

    NPCWidth[199] = 56;
    NPCHeight[199] = 60;
    NPCWidthGFX[199] = 130;
    NPCHeightGFX[199] = 64;
    NPCJumpHurt[199] = true;
    NPCNoClipping[199] = true;
    NPCNoYoshi[203] = true;
    NPCNoYoshi[204] = true;
    NPCNoYoshi[205] = true;
    NPCNoYoshi[206] = true;

    NPCWidth[203] = 46;
    NPCHeight[203] = 26;
    NPCNoYoshi[203] = true;
    NPCCanWalkOn[203] = true;
    NPCNoYoshi[204] = true;
    NPCCanWalkOn[204] = true;
    NPCJumpHurt[201] = true;

    NPCNoYoshi[208] = true;
    NPCNoYoshi[209] = true;
    NPCCanWalkOn[209] = true;
    NPCMovesPlayer[209] = true;
    NPCIsABlock[209] = true;
    NPCNoClipping[209] = true;
    NPCWidth[209] = 96;
    NPCHeight[209] = 106;

    NPCNoYoshi[202] = true;
    NPCNoYoshi[201] = true;
    NPCNoYoshi[200] = true;

    NPCWidthGFX[189] = 48;
    NPCHeightGFX[189] = 64;
    NPCHeight[189] = 64;
    NPCTurnsAtCliffs[189] = true;
    NPCFrameOffsetY[189] = 2;
    NPCDefaultMovement[189] = true;
    NPCWidthGFX[175] = 32;
    NPCWidthGFX[176] = 32;
    NPCWidthGFX[177] = 32;
    NPCWidthGFX[173] = 32;
    NPCHeightGFX[175] = 48;
    NPCHeightGFX[176] = 48;
    NPCHeightGFX[177] = 48;
    NPCHeightGFX[173] = 48;

    NPCWidthGFX[190] = 32;
    NPCHeightGFX[190] = 32;

    NPCWidth[234] = 48;

    NPCHeightGFX[235] = 48;
    NPCWidthGFX[235] = 32;
    NPCFrameOffsetY[235] = 16;

    NPCJumpHurt[235] = true;
    NPCJumpHurt[234] = true;

    NPCHeight[190] = 20;
    NPCFrameOffsetY[190] = 12;
    NPCFrameOffsetY[194] = 2;

    NPCMovesPlayer[190] = true;

    NPCTurnsAtCliffs[175] = true;

    NPCNoYoshi[178] = true;
    NPCNoYoshi[179] = true;
    NPCNoYoshi[180] = true;
    NPCNoYoshi[181] = true;
    NPCNoYoshi[189] = true;

    NPCNoYoshi[190] = true;
    NPCNoYoshi[192] = true;
    NPCIsABonus[192] = true;



    NPCIsAParaTroopa[176] = true;
    NPCIsAParaTroopa[177] = true;
    NPCIsABonus[178] = true;
    NPCDefaultMovement[173] = true;
    NPCDefaultMovement[175] = true;
    NPCDefaultMovement[162] = true;
    // NPCDefaultMovement[163] = true;
    // NPCDefaultMovement[164] = true;
    NPCDefaultMovement[165] = true;
    NPCDefaultMovement[167] = true;
    NPCCanWalkOn[164] = true;
    NPCIsAHit1Block[164] = true;
    NPCWontHurt[160] = true;
    NPCWontHurt[166] = true;
    NPCCanWalkOn[160] = true;
    NPCMovesPlayer[160] = true;
    NPCIsABlock[160] = true;
    NPCDefaultMovement[160] = true;
    NPCWontHurt[159] = true;
    NPCCanWalkOn[159] = true;
    NPCIsAHit1Block[159] = true;
    NPCGrabFromTop[159] = true;
    NPCWontHurt[154] = true;
    NPCCanWalkOn[154] = true;
    NPCIsABlock[154] = true;
    NPCMovesPlayer[154] = true;
    NPCWontHurt[155] = true;
    NPCCanWalkOn[155] = true;
    NPCIsABlock[155] = true;
    NPCMovesPlayer[155] = true;
    NPCWontHurt[156] = true;
    NPCCanWalkOn[156] = true;
    NPCIsABlock[156] = true;
    NPCMovesPlayer[156] = true;
    NPCWontHurt[157] = true;
    NPCCanWalkOn[157] = true;
    NPCIsABlock[157] = true;
    NPCMovesPlayer[157] = true;
    NPCNoYoshi[159] = true;


    NPCWontHurt[197] = true;
    NPCNoYoshi[197] = true;


    NPCIsGrabbable[154] = true;
    NPCIsGrabbable[166] = true;
    NPCGrabFromTop[154] = true;
    NPCIsGrabbable[155] = true;
    NPCGrabFromTop[155] = true;
    NPCIsGrabbable[156] = true;
    NPCGrabFromTop[156] = true;
    NPCIsGrabbable[157] = true;
    NPCGrabFromTop[157] = true;
    NPCIsABot[128] = true;
    NPCIsYoshi[95] = true;
    NPCIsYoshi[98] = true;
    NPCIsYoshi[99] = true;
    NPCIsYoshi[100] = true;
    NPCIsYoshi[148] = true;
    NPCIsYoshi[149] = true;
    NPCIsYoshi[150] = true;
    NPCIsYoshi[228] = true;
    NPCIsBoot[35] = true;
    NPCIsBoot[191] = true;
    NPCIsBoot[193] = true;
    NPCIsAShell[5] = true;
    NPCIsAShell[7] = true;
    NPCIsAShell[24] = true;
    NPCIsAShell[73] = true;
    NPCIsABonus[9] = true;
    NPCIsABonus[273] = true;
    NPCIsABonus[10] = true;
    NPCIsABonus[11] = true;
    NPCIsABonus[14] = true;
    NPCIsABonus[264] = true;
    NPCIsABonus[277] = true;
    NPCIsABonus[16] = true;
    NPCIsABonus[33] = true;
    NPCIsABonus[258] = true;
    NPCIsABonus[34] = true;
    NPCIsABonus[41] = true;
    NPCIsABonus[75] = true;
    NPCIsABonus[88] = true;
    NPCIsABonus[90] = true;
    NPCIsABonus[94] = true;
    NPCIsABonus[198] = true;
    NPCIsABonus[97] = true;
    NPCIsABonus[101] = true;
    NPCIsABonus[102] = true;
    NPCIsABonus[103] = true;
    NPCIsABonus[107] = true;
    NPCIsABonus[153] = true;
    NPCIsABonus[196] = true;
    NPCIsACoin[10] = true;
    NPCIsACoin[33] = true;
    NPCIsACoin[258] = true;
    NPCIsACoin[88] = true;
    NPCIsACoin[103] = true;
    NPCIsAnExit[11] = true;
    NPCIsAnExit[16] = true;
    NPCIsAnExit[41] = true;
    NPCIsAnExit[97] = true;
    NPCIsAnExit[196] = true;
    NPCJumpHurt[8] = true;
    NPCJumpHurt[245] = true;
    NPCJumpHurt[12] = true;
    NPCJumpHurt[30] = true;
    NPCJumpHurt[36] = true;
    NPCJumpHurt[285] = true;
    NPCJumpHurt[286] = true;
    NPCJumpHurt[37] = true;
    NPCJumpHurt[38] = true;
    NPCJumpHurt[42] = true;
    NPCJumpHurt[43] = true;
    NPCJumpHurt[44] = true;
    NPCJumpHurt[48] = true;
    NPCJumpHurt[51] = true;
    NPCJumpHurt[52] = true;
    NPCJumpHurt[53] = true;
    NPCJumpHurt[54] = true;
    NPCJumpHurt[74] = true;
    NPCJumpHurt[256] = true;
    NPCJumpHurt[257] = true;
    NPCJumpHurt[85] = true;
    NPCJumpHurt[87] = true;
    NPCJumpHurt[93] = true;
    NPCCanWalkOn[19] = true;
    NPCCanWalkOn[20] = true;
    NPCCanWalkOn[247] = true;
    NPCCanWalkOn[21] = true;
    NPCCanWalkOn[25] = true;
    // NPCCanWalkOn[28] = true;
    NPCCanWalkOn[31] = true;
    NPCCanWalkOn[39] = true;
    NPCCanWalkOn[40] = true;
    NPCCanWalkOn[45] = true;
    NPCCanWalkOn[46] = true;
    NPCCanWalkOn[212] = true;
    NPCCanWalkOn[56] = true;
    NPCCanWalkOn[57] = true;
    NPCCanWalkOn[58] = true;
    NPCCanWalkOn[60] = true;
    NPCCanWalkOn[62] = true;
    NPCCanWalkOn[64] = true;
    NPCCanWalkOn[66] = true;
    NPCCanWalkOn[67] = true;
    NPCCanWalkOn[68] = true;
    NPCCanWalkOn[69] = true;
    NPCCanWalkOn[70] = true;
    NPCCanWalkOn[78] = true;
    NPCCanWalkOn[79] = true;
    NPCCanWalkOn[80] = true;
    NPCCanWalkOn[81] = true;
    NPCCanWalkOn[82] = true;
    NPCCanWalkOn[83] = true;
    NPCCanWalkOn[84] = true;
    NPCCanWalkOn[91] = true;
    NPCCanWalkOn[92] = true;
    NPCCanWalkOn[104] = true;
    NPCCanWalkOn[105] = true;
    NPCCanWalkOn[106] = true;
    NPCGrabFromTop[19] = true;
    NPCGrabFromTop[20] = true;
    NPCGrabFromTop[247] = true;
    NPCGrabFromTop[25] = true;
    // NPCGrabFromTop[28] = true;
    NPCGrabFromTop[31] = true;
    NPCGrabFromTop[40] = true;
    NPCGrabFromTop[45] = true;
    NPCGrabFromTop[91] = true;
    NPCGrabFromTop[92] = true;
    NPCNoClipping[8] = true;
    NPCNoClipping[245] = true;
    // NPCNoClipping[10] = true;
    NPCNoClipping[12] = true;
    NPCNoClipping[17] = true;
    NPCNoClipping[18] = true;
    NPCNoClipping[30] = true;
    NPCNoClipping[34] = true;
    NPCNoClipping[38] = true;
    NPCNoClipping[42] = true;
    NPCNoClipping[43] = true;
    NPCNoClipping[44] = true;
    NPCNoClipping[46] = true;
    NPCNoClipping[212] = true;
    NPCNoClipping[47] = true;
    NPCNoClipping[284] = true;
    NPCNoClipping[50] = true;
    NPCNoClipping[51] = true;
    NPCNoClipping[52] = true;
    // NPCNoClipping[56] = true;
    NPCNoClipping[57] = true;
    NPCNoClipping[60] = true;
    NPCNoClipping[62] = true;
    NPCNoClipping[64] = true;
    NPCNoClipping[66] = true;
    NPCNoClipping[74] = true;
    NPCNoClipping[256] = true;
    NPCNoClipping[257] = true;
    NPCNoClipping[85] = true;
    NPCNoClipping[87] = true;
    NPCNoClipping[91] = true;
    NPCNoClipping[93] = true;
    NPCNoClipping[104] = true;
    NPCNoClipping[105] = true;
    NPCNoClipping[106] = true;
    NPCNoClipping[108] = true;
    NPCTurnsAtCliffs[6] = true;
    NPCTurnsAtCliffs[20] = true;
    NPCTurnsAtCliffs[247] = true;
    NPCTurnsAtCliffs[55] = true;
    NPCTurnsAtCliffs[59] = true;
    NPCTurnsAtCliffs[61] = true;
    NPCTurnsAtCliffs[63] = true;
    NPCTurnsAtCliffs[65] = true;
    NPCTurnsAtCliffs[107] = true;
    NPCTurnsAtCliffs[158] = true;
    NPCMovesPlayer[21] = true;
    NPCMovesPlayer[31] = true;
    NPCMovesPlayer[32] = true;
    NPCMovesPlayer[238] = true;
    NPCMovesPlayer[239] = true;
    NPCMovesPlayer[45] = true;
    NPCMovesPlayer[46] = true;
    NPCMovesPlayer[212] = true;
    NPCMovesPlayer[57] = true;
    NPCMovesPlayer[58] = true;
    NPCMovesPlayer[67] = true;
    NPCMovesPlayer[68] = true;
    NPCMovesPlayer[69] = true;
    NPCMovesPlayer[70] = true;
    NPCMovesPlayer[78] = true;
    NPCMovesPlayer[79] = true;
    NPCMovesPlayer[80] = true;
    NPCMovesPlayer[81] = true;
    NPCMovesPlayer[82] = true;
    NPCMovesPlayer[83] = true;
    NPCMovesPlayer[84] = true;
    NPCWontHurt[21] = true;
    NPCWontHurt[22] = true;
    NPCWontHurt[26] = true;
    NPCWontHurt[31] = true;
    NPCWontHurt[32] = true;
    NPCWontHurt[238] = true;
    NPCWontHurt[239] = true;
    NPCWontHurt[35] = true;
    NPCWontHurt[191] = true;
    NPCWontHurt[193] = true;
    NPCWontHurt[45] = true;
    NPCWontHurt[46] = true;
    NPCWontHurt[212] = true;
    NPCWontHurt[49] = true;
    NPCWontHurt[50] = true;
    NPCWontHurt[56] = true;
    NPCWontHurt[57] = true;
    NPCWontHurt[58] = true;
    NPCWontHurt[60] = true;
    NPCWontHurt[62] = true;
    NPCWontHurt[64] = true;
    NPCWontHurt[66] = true;
    NPCWontHurt[67] = true;
    NPCWontHurt[68] = true;
    NPCWontHurt[69] = true;
    NPCWontHurt[70] = true;
    NPCWontHurt[78] = true;
    NPCWontHurt[79] = true;
    NPCWontHurt[80] = true;
    NPCWontHurt[81] = true;
    NPCWontHurt[82] = true;
    NPCWontHurt[83] = true;
    NPCWontHurt[84] = true;
    NPCWontHurt[91] = true;
    NPCWontHurt[92] = true;
    NPCWontHurt[95] = true;
    NPCWontHurt[96] = true;
    NPCWontHurt[98] = true;
    NPCWontHurt[99] = true;
    NPCWontHurt[100] = true;
    NPCWontHurt[104] = true;
    NPCWontHurt[105] = true;
    NPCWontHurt[106] = true;
    NPCWontHurt[107] = true;
    NPCWontHurt[108] = true;
    NPCWontHurt[148] = true;
    NPCWontHurt[149] = true;
    NPCWontHurt[150] = true;
    NPCWontHurt[228] = true;
    NPCWontHurt[158] = true;
    NPCIsGrabbable[22] = true;
    NPCIsGrabbable[26] = true;
    NPCIsGrabbable[31] = true;
    NPCIsGrabbable[32] = true;
    NPCIsGrabbable[238] = true;
    NPCIsGrabbable[35] = true;
    NPCIsGrabbable[191] = true;
    NPCIsGrabbable[193] = true;
    NPCIsGrabbable[45] = true;
    NPCIsGrabbable[49] = true;
    NPCIsGrabbable[92] = true;
    NPCIsGrabbable[96] = true;
    NPCIsGrabbable[158] = true;
    NPCIsABlock[21] = true;
    NPCIsABlock[31] = true;
    NPCIsABlock[32] = true;
    NPCIsABlock[238] = true;
    NPCIsABlock[239] = true;
    NPCIsABlock[45] = true;
    NPCIsABlock[46] = true;
    NPCIsABlock[212] = true;
    NPCIsABlock[56] = true;
    NPCIsABlock[57] = true;
    NPCIsABlock[58] = true;
    NPCIsABlock[67] = true;
    NPCIsABlock[68] = true;
    NPCIsABlock[69] = true;
    NPCIsABlock[70] = true;
    NPCIsABlock[78] = true;
    NPCIsABlock[79] = true;
    NPCIsABlock[80] = true;
    NPCIsABlock[81] = true;
    NPCIsABlock[82] = true;
    NPCIsABlock[83] = true;
    NPCIsABlock[84] = true;
    NPCIsABlock[190] = true;
    NPCIsAHit1Block[35] = true;
    NPCIsAHit1Block[191] = true;
    NPCIsAHit1Block[193] = true;
    NPCIsAHit1Block[22] = true;
    NPCIsAHit1Block[49] = true;
    NPCIsAHit1Block[26] = true;
    NPCIsAHit1Block[60] = true;
    NPCIsAHit1Block[62] = true;
    NPCIsAHit1Block[64] = true;
    NPCIsAHit1Block[66] = true;
    NPCIsAHit1Block[104] = true;
    NPCIsAHit1Block[105] = true;
    NPCIsAHit1Block[106] = true;
    NPCWontHurt[190] = true;
    NPCCanWalkOn[190] = true;
    NPCStandsOnPlayer[22] = true;
    NPCStandsOnPlayer[26] = true;
    NPCStandsOnPlayer[31] = true;
    NPCStandsOnPlayer[32] = true;
    NPCStandsOnPlayer[238] = true;
    NPCStandsOnPlayer[239] = true;
    NPCStandsOnPlayer[35] = true;
    NPCStandsOnPlayer[191] = true;
    NPCStandsOnPlayer[193] = true;
    NPCStandsOnPlayer[49] = true;
    NPCStandsOnPlayer[95] = true;
    NPCStandsOnPlayer[96] = true;
    NPCStandsOnPlayer[98] = true;
    NPCStandsOnPlayer[99] = true;
    NPCStandsOnPlayer[100] = true;
    NPCStandsOnPlayer[148] = true;
    NPCStandsOnPlayer[149] = true;
    NPCStandsOnPlayer[150] = true;
    NPCStandsOnPlayer[228] = true;


    NPCCanWalkOn[272] = true;
    NPCGrabFromTop[272] = true;
    NPCNoFireBall[272] = true;
    NPCNoClipping[272] = true;
    NPCWidth[272] = 32;
    NPCHeight[272] = 32;

    NPCCanWalkOn[208] = true;
    NPCWontHurt[208] = true;
    NPCMovesPlayer[208] = true;
    NPCWidth[208] = 128;
    NPCHeight[208] = 128;
    NPCIsABlock[208] = true;

    NPCForeground[208] = true;
    NPCJumpHurt[207] = true;
    NPCJumpHurt[206] = true;
    NPCJumpHurt[205] = true;

    NPCWidth[204] = 48;
    NPCHeight[204] = 28;
    NPCWidthGFX[204] = 112;
    NPCHeightGFX[204] = 28;

    NPCWidthGFX[205] = 44;
    NPCHeightGFX[205] = 44;
    NPCFrameOffsetY[205] = 6;


    NPCWidth[270] = 32;
    NPCHeight[270] = 42;
    NPCJumpHurt[270] = true;
    NPCNoClipping[270] = true;

    NPCWidth[271] = 32;
    NPCHeight[271] = 32;
    NPCNoClipping[271] = true;


    NPCWidth[210] = 28;
    NPCNoClipping[210] = true;
    NPCIsABlock[211] = true;
    NPCNoClipping[211] = true;
    NPCMovesPlayer[211] = true;
    NPCCanWalkOn[211] = true;
    NPCWontHurt[211] = true;





    NPCForeground[12] = true;
    NPCForeground[17] = true;
    NPCForeground[18] = true;
    NPCForeground[28] = true;
    NPCForeground[30] = true;
    NPCForeground[34] = true;
    NPCForeground[38] = true;
    NPCForeground[42] = true;
    NPCForeground[43] = true;
    NPCForeground[44] = true;
    NPCForeground[47] = true;
    NPCForeground[85] = true;
    NPCForeground[86] = true;
    NPCForeground[87] = true;
    NPCForeground[108] = true;
    NPCForeground[171] = true;
    NPCDefaultMovement[1] = true;
    NPCDefaultMovement[2] = true;
    NPCDefaultMovement[3] = true;
    NPCDefaultMovement[4] = true;
    NPCDefaultMovement[6] = true;
    NPCDefaultMovement[19] = true;
    NPCDefaultMovement[20] = true;
    NPCDefaultMovement[247] = true;
    NPCDefaultMovement[23] = true;
    NPCDefaultMovement[27] = true;
    NPCDefaultMovement[36] = true;
    NPCDefaultMovement[285] = true;
    NPCDefaultMovement[50] = true;
    NPCDefaultMovement[53] = true;
    NPCDefaultMovement[55] = true;
    NPCDefaultMovement[59] = true;
    NPCDefaultMovement[61] = true;
    NPCDefaultMovement[63] = true;
    NPCDefaultMovement[65] = true;
    NPCDefaultMovement[71] = true;
    NPCDefaultMovement[72] = true;
    NPCDefaultMovement[77] = true;
    NPCDefaultMovement[78] = true;
    NPCDefaultMovement[89] = true;
    NPCDefaultMovement[91] = true;
    NPCDefaultMovement[107] = true;

    NPCNoFireBall[19] = true;
    NPCNoFireBall[20] = true;
    NPCNoFireBall[23] = true;
    NPCNoFireBall[24] = true;
    NPCNoFireBall[137] = true;
    NPCNoFireBall[136] = true;
    NPCNoFireBall[129] = true;
    NPCNoFireBall[130] = true;
    NPCNoFireBall[131] = true;
    NPCNoFireBall[132] = true;
    NPCNoFireBall[12] = true;
    NPCNoFireBall[25] = true;


    // link sword beam
    NPCWidth[266] = 16;
    NPCHeight[266] = 8;
    NPCWontHurt[266] = true;
    NPCJumpHurt[266] = true;
    NPCNoGravity[266] = true;

    NPCNoYoshi[267] = true;
    NPCNoYoshi[268] = true;
    NPCNoYoshi[269] = true;
    NPCNoYoshi[280] = true;
    NPCNoYoshi[281] = true;
    NPCNoYoshi[282] = true;


    NPCNoClipping[276] = true;
    NPCWidth[276] = 16;
    NPCHeight[276] = 16;
    NPCNoYoshi[276] = true;
    NPCJumpHurt[276] = true;

    NPCWidth[275] = 60;
    NPCHeight[275] = 24;
    NPCWidthGFX[275] = 64;
    NPCHeightGFX[275] = 32;
    NPCNoFireBall[275] = true;
    NPCJumpHurt[275] = true;

    NPCWidth[278] = 32;
    NPCHeight[278] = 44;
    NPCWontHurt[278] = true;
    NPCJumpHurt[278] = true;
    NPCIsGrabbable[278] = true;

    NPCWidth[279] = 32;
    NPCHeight[279] = 44;
    NPCWidthGFX[279] = 96;
    NPCHeightGFX[279] = 44;
    NPCWontHurt[279] = true;
    NPCJumpHurt[279] = true;
    NPCIsGrabbable[279] = true;


// set ice
    NPCNoIceBall[197] = true;
    NPCNoIceBall[192] = true;

    NPCNoIceBall[267] = true;
    NPCNoIceBall[268] = true;
    NPCNoIceBall[269] = true;
    NPCNoIceBall[275] = true;
    NPCNoIceBall[280] = true;
    NPCNoIceBall[281] = true;
    NPCNoIceBall[282] = true;
    NPCNoIceBall[15] = true;
    NPCNoIceBall[18] = true;
    NPCNoIceBall[26] = true;
    NPCNoIceBall[30] = true;
    NPCNoIceBall[31] = true;
    NPCNoIceBall[32] = true;
    NPCNoIceBall[35] = true;
    NPCNoIceBall[37] = true;
    NPCNoIceBall[38] = true;
    NPCNoIceBall[39] = true;
    NPCNoIceBall[40] = true;
    NPCNoIceBall[42] = true;
    NPCNoIceBall[43] = true;
    NPCNoIceBall[44] = true;
    NPCNoIceBall[45] = true;
    NPCNoIceBall[46] = true;
    NPCNoIceBall[56] = true;
    For(A, 57, 70)
    {
        NPCNoIceBall[A] = true;
    }
    NPCNoIceBall[75] = true;
    For(A, 78, 88)
    {
        NPCNoIceBall[A] = true;
    }
    NPCNoIceBall[91] = true;
    NPCNoIceBall[92] = true;
    NPCNoIceBall[94] = true;
    For(A, 95, 108)
    {
        NPCNoIceBall[A] = true;
    }
    NPCNoIceBall[133] = true;
    NPCNoIceBall[134] = true;
    For(A, 138, 160)
    {
        NPCNoIceBall[A] = true;
    }
    NPCNoIceBall[171] = true;
    NPCNoIceBall[178] = true;
    NPCNoIceBall[179] = true;
    NPCNoIceBall[180] = true;
    NPCNoIceBall[181] = true;
    NPCNoIceBall[190] = true;
    NPCNoIceBall[191] = true;
    NPCNoIceBall[192] = true;
    NPCNoIceBall[193] = true;
    NPCNoIceBall[195] = true;
    NPCNoIceBall[199] = true;
    NPCNoIceBall[200] = true;
    NPCNoIceBall[201] = true;
    NPCNoIceBall[208] = true;
    NPCNoIceBall[209] = true;
    NPCNoIceBall[210] = true;
    For(A, 211, 228)
    {
        NPCNoIceBall[A] = true;
    }
    NPCNoIceBall[237] = true;
    NPCNoIceBall[238] = true;
    NPCNoIceBall[239] = true;
    NPCNoIceBall[241] = true;
    NPCNoIceBall[246] = true;
    NPCNoIceBall[254] = true;
    NPCNoIceBall[255] = true;
    NPCNoIceBall[256] = true;
    NPCNoIceBall[257] = true;
    NPCNoIceBall[259] = true;
    NPCNoIceBall[260] = true;
    NPCNoIceBall[262] = true;
    NPCNoIceBall[251] = true;
    NPCNoIceBall[252] = true;
    NPCNoIceBall[253] = true;
    NPCNoIceBall[258] = true;
    NPCNoIceBall[33] = true;
    NPCNoIceBall[274] = true;
    NPCNoIceBall[278] = true;
    NPCNoIceBall[279] = true;

    for(int A = 282; A <=292; ++A)
    {
        if(A != 285 && A != 286)
            NPCNoIceBall[A] = true;
    }


    NPCHeight[247] = 30;
    NPCHeightGFX[247] = 32;
    NPCWidthGFX[247] = 32;


    NPCDefaultMovement[158] = true;


    NPCWidthGFX[267] = 84;
    NPCHeightGFX[267] = 62;
    NPCWidth[267] = 44;
    NPCHeight[267] = 50;

    NPCWidthGFX[268] = 44;
    NPCHeightGFX[268] = 32;
    NPCWidth[268] = 32;
    NPCHeight[268] = 28;

    NPCWidthGFX[269] = 16;
    NPCHeightGFX[269] = 32;
    NPCWidth[269] = 16;
    NPCHeight[269] = 32;
    NPCNoClipping[269] = true;
    NPCJumpHurt[269] = true;

    NPCWidthGFX[280] = 64;
    NPCHeightGFX[280] = 64;
    NPCWidth[280] = 48;
    NPCHeight[280] = 48;

    NPCWidthGFX[281] = 40;
    NPCHeightGFX[281] = 36;
    NPCWidth[281] = 36;
    NPCHeight[281] = 32;

    NPCWidthGFX[282] = 64;
    NPCHeightGFX[282] = 32;
    NPCWidth[282] = 64;
    NPCHeight[282] = 26;
    NPCFrameOffsetY[282] = 4;
    NPCNoClipping[282] = true;
    NPCJumpHurt[282] = true;

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
        if(NPCIsCheep[A])
            NPCForeground[A] = true;
    }
    For(A, 1, maxEffectType)
    {
        EffectDefaults.EffectHeight[A] = EffectHeight[A];
        EffectDefaults.EffectWidth[A] = EffectWidth[A];
        if(EffectHeight[A] > 0)
        {
            EffectDefaults.EffectFrames[A] = GFXEffectHeight[A] / EffectHeight[A];
            if(EffectDefaults.EffectFrames[A] <= 0)
                EffectDefaults.EffectFrames[A] = 1;
        }
        else
            EffectDefaults.EffectFrames[A] = 1;
    }
    SaveNPCDefaults();
}
