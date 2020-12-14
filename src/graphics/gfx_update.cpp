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
#include "../graphics.h"
#include "../collision.h"
#include "../editor.h"
#include "../npc.h"
#include "../location.h"
#include "../main/menu_main.h"

#include <fmt_format_ne.h>
#include <Utils/maths.h>


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

    // frame skip code
    cycleCount += 1;

    if(FrameSkip && !TakeScreen)
    {
        if(SDL_GetTicks() + floor(1000 * (1 - (cycleCount / 63.0))) > GoalTime) // Don't draw this frame
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
//                                timeStr = timeStr + "2b" + std::to_string(A) + LB;
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
    fpsCount++;

    std::string SuperText;
    std::string tempText;
    int BoxY = 0;
    bool tempBool = false;
    int B = 0;
//    int B2 = 0;
    int C = 0;
    int D = 0;
    int E = 0;
    double d2 = 0;
//    int e2 = 0;
    int X = 0;
    int Y = 0;
    int fBlock = 0;
    int lBlock = 0;
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


//    If LevelEditor = False Then  'Sets up the screens if not in level editor
    { // NOT AN EDITOR
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
        frmMain.clearBuffer();
    }

    if(SingleCoop == 2)
        numScreens = 2;

    For(Z, 1, numScreens)
    {
        if(SingleCoop == 2)
            Z = 2;

//        If LevelEditor = True Then
//            S = curSection
//        ElseIf nPlay.Online = True Then
//            S = Player(nPlay.MySlot + 1).Section
//        Else
        {
            S = Player[Z].Section;
//        End If
        }

//        If GameOutro = True Then ScreenType = 7
        if(GameOutro)
            ScreenType = 7;
//        If LevelEditor = False Then
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
                    frmMain.renderRect(vScreen[Z].Left, vScreen[Z].Top,
                                       vScreen[Z].Width, vScreen[Z].Height, 0.f, 0.f, 0.f, 1.f, true);
                else
                    frmMain.clearBuffer();
            }
        }
//        Else
//            If Background2(S) = 0 Then BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
//        End If

        if(qScreen)
        {
            if(vScreenX[1] < qScreenX[1] - 2)
                qScreenX[1] = qScreenX[1] - 2;
            else if(vScreenX[1] > qScreenX[1] + 2)
                qScreenX[1] = qScreenX[1] + 2;
            if(vScreenY[1] < qScreenY[1] - 2)
                qScreenY[1] = qScreenY[1] - 2;
            else if(vScreenY[1] > qScreenY[1] + 2)
                qScreenY[1] = qScreenY[1] + 2;
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
            frmMain.setViewport(vScreen[Z].Left, vScreen[Z].Top, vScreen[Z].Width, vScreen[Z].Height);

        DrawBackground(S, Z);


//            If LevelEditor = True Then
//                If BlockFlash <= 30 Then
//                    With tempLocation 'Black out the level edges
//                        .X = level(curSection).X
//                        .Y = level(curSection).Y
//                        .Width = level(curSection).Width
//                        .Height = level(curSection).Height
//                        BitBlt myBackBuffer, 0, 0, vScreenX(Z) + level(curSection).X, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, 0, 0, vScreenX(Z) + level(curSection).Width, vScreenY(Z) + level(curSection).Y, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        If -vScreenX(Z) < level(curSection).Width Then
//                            BitBlt myBackBuffer, vScreenX(Z) + level(curSection).Width, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        End If
//                        If -vScreenY(Z) < level(curSection).Height Then
//                            BitBlt myBackBuffer, 0, vScreenY(Z) + level(curSection).Height, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        End If
//                    End With
//                End If
//            End If


//        If GameMenu = True Then
        if(GameMenu)
        {
            // (Commented out in original code :thinking:)
            // Curtain
//            frmMain.renderTexture(0, 0, GFX.MenuGFX[1]);
            // Game logo
//            frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70, GFX.MenuGFX[2]);
//        ElseIf LevelEditor = False Then
        } else { // it's NO editor, obviously
//            If numPlayers > 2 And nPlay.Online = False Then
            if(numPlayers > 2)
            {
//                For A = 1 To numPlayers
                For(A, 1, numPlayers)
                {
//                    With Player(A)
                    Player_t &p = Player[A];
//                        If vScreenCollision(Z, .Location) = False And LevelMacro = 0 And .Location.Y < level(.Section).Height And .Location.Y + .Location.Height > level(.Section).Y And .TimeToLive = 0 And .Dead = False Then
                    if(!vScreenCollision(Z, p.Location) && LevelMacro == 0 &&
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

//        If LevelEditor = True Or MagicHand = True Then
//            For A = 1 To numBackground 'First backgrounds
//                With Background(A)
//                    If .Type = 11 Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, 0, vbSrcPaint
//                        End If
//                    ElseIf .Type = 12 Or .Type = 60 Or .Type = 61 Or .Type = 75 Or .Type = 76 Or .Type = 77 Or .Type = 78 Or .Type = 79 Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, 0, vbSrcCopy
//                        End If
//                    End If
//                End With
//            Next A
//        Else
        {// NOT AN EDITOR
//            For A = 1 To MidBackground - 1 'First backgrounds
            For(A, 1, MidBackground - 1)  // First backgrounds
            {
//                if(BackgroundHasNoMask[Background[A].Type] == false) // Useless code
//                {
//                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
//                    {
//                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
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
            if(BlockIsSizable[Block[sBlockArray[A]].Type] && (!Block[sBlockArray[A]].Invis || LevelEditor))
            {
                if(vScreenCollision(Z, Block[sBlockArray[A]].Location) && !Block[sBlockArray[A]].Hidden)
                {
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
                                    frmMain.renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                                }
                                else
                                {
                                    frmMain.renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                                }
#endif
                                frmMain.renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                            }
                        }
                    }
                }
            }
        }

//        If LevelEditor = True Or MagicHand = True Then
//            For A = 1 To numBackground 'Second backgrounds
//                With Background(A)
//                    If Not (.Type = 11 Or .Type = 12 Or .Type = 60 Or .Type = 61 Or .Type = 75 Or .Type = 76 Or .Type = 77 Or .Type = 78 Or .Type = 79) Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            If BackgroundHasNoMask(.Type) = False Then
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                            Else
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                            End If
//                        End If
//                    End If
//                End With
//            Next A
//        Else
        { // NOT AN EDITOR
            for(A = MidBackground; A <= LastBackground; A++) // Second backgrounds
            {
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
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
            if(vScreenCollision(Z, Background[A].Location) &&
                (Background[A].Type == 98 || Background[A].Type == 160) && !Background[A].Hidden)
            {
                frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
                                      vScreenY[Z] + Background[A].Location.Y,
                                      BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type],
                                      GFXBackgroundBMP[Background[A].Type],
                                      0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
            }
        }

//        For A = 1 To numNPCs 'Display NPCs that should be behind blocks
        For(A, 1, numNPCs) // Display NPCs that should be behind blocks
        {
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
//                                timeStr = timeStr + "2b" + std::to_string(A) + LB;
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


//        If LevelEditor = True Then
//            fBlock = 1
//            lBlock = numBlock
//        Else
        { // NOT AN EDITOR!!!
//            fBlock = FirstBlock((-vScreenX(Z) / 32) - 1)
            fBlock = FirstBlock[int(-vScreenX[Z] / 32) - 1];
//            lBlock = LastBlock(((-vScreenX(Z) + vScreen(Z).Width) / 32) + 1)
            lBlock = LastBlock[int((-vScreenX[Z] + vScreen[Z].Width) / 32) + 1];
//        End If
        }


//        For A = fBlock To lBlock 'Non-Sizable Blocks
        For(A, fBlock, lBlock)
        {
            if(!BlockIsSizable[Block[A].Type] && (!Block[A].Invis || (LevelEditor && BlockFlash <= 30)) && Block[A].Type != 0 && !BlockKills[Block[A].Type])
            {
                if(vScreenCollision(Z, Block[A].Location) && !Block[A].Hidden)
                {
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
            if(Effect[A].Type == 112 || Effect[A].Type == 54 || Effect[A].Type == 55 ||
               Effect[A].Type == 59 || Effect[A].Type == 77 || Effect[A].Type == 81 ||
               Effect[A].Type == 82 || Effect[A].Type == 103 || Effect[A].Type == 104 ||
               Effect[A].Type == 114 || Effect[A].Type == 123 || Effect[A].Type == 124)
            {
                if(vScreenCollision(Z, Effect[A].Location))
                {
                    float cn = Effect[A].Shadow ? 0.f : 1.f;
                    frmMain.renderTexture(vScreenX[Z] + Effect[A].Location.X, vScreenY[Z] + Effect[A].Location.Y, Effect[A].Location.Width, Effect[A].Location.Height, GFXEffect[Effect[A].Type], 0, Effect[A].Frame * EffectHeight[Effect[A].Type], cn, cn, cn);
                }
            }
        }


        for(A = 1; A <= numNPCs; A++) // Display NPCs that should be behind other npcs
        {
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(NPC[A].Effect == 0)
            {
                if(NPC[A].HoldingPlayer == 0 && (NPC[A].standingOnPlayer > 0 || NPC[A].Type == 56 || NPC[A].Type == 22 || NPC[A].Type == 49 || NPC[A].Type == 91 || NPC[A].Type == 160 || NPC[A].Type == 282 || NPCIsACoin[NPC[A].Type]) && (NPC[A].Generator == false || LevelEditor == true))
                {
                    // If Not NPCIsACoin(.Type) Then
                    if((vScreenCollision(Z, NPC[A].Location) | vScreenCollision(Z, newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0, NPC[A].Location.Y, static_cast<double>(NPCWidthGFX[NPC[A].Type]), static_cast<double>(NPCHeight[NPC[A].Type])))) != 0 && NPC[A].Hidden == false)
                    {
                        if(NPC[A].Active)
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
//                                    timeStr = timeStr + "2b" + std::to_string(A) + LB;
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
            if(NPC[A].Type == 263 && NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
            {
                if((vScreenCollision(Z, NPC[A].Location) | vScreenCollision(Z, newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0, NPC[A].Location.Y, static_cast<double>(NPCWidthGFX[NPC[A].Type]), static_cast<double>(NPCHeight[NPC[A].Type])))) != 0 && NPC[A].Hidden == false)
                {
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
//                            timeStr = timeStr + "2b" + std::to_string(A) + LB;
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
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(NPC[A].Effect == 0)
            {
                if(!(NPC[A].HoldingPlayer > 0 || NPCIsAVine[NPC[A].Type] || NPC[A].Type == 209 || NPC[A].Type == 282 || NPC[A].Type == 270 || NPC[A].Type == 160 || NPC[A].Type == 159 || NPC[A].Type == 8 || NPC[A].Type == 245 || NPC[A].Type == 93 || NPC[A].Type == 51 || NPC[A].Type == 52 || NPC[A].Type == 74 || NPC[A].Type == 256 || NPC[A].Type == 257 || NPC[A].Type == 56 || NPC[A].Type == 22 || NPC[A].Type == 49 || NPC[A].Type == 91) && !(NPC[A].Type == 45 && NPC[A].Special == 0) && NPC[A].standingOnPlayer == 0 && NPCForeground[NPC[A].Type] == false && (NPC[A].Generator == false || LevelEditor == true) && NPC[A].Type != 179 && NPC[A].Type != 263)
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
                                            B = EditorNPCFrame(static_cast<int>(floor(static_cast<double>(NPC[A].Special))), NPC[A].Direction);
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
                                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(NPC[A].Location.X))) + YoshiBX, vScreenY[Z] + NPC[A].Location.Y + YoshiBY, 32, 32, GFXYoshiB[B], 0, 32 * YoshiBFrame, cn, cn, cn);

                                    // Yoshi's Head
                                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(NPC[A].Location.X))) + YoshiTX, vScreenY[Z] + NPC[A].Location.Y + YoshiTY, 32, 32, GFXYoshiT[B], 0, 32 * YoshiTFrame, cn, cn, cn);
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
//                                    timeStr = timeStr + "2b" + std::to_string(A) + LB;
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
            if(NPC[A].Active && NPC[A].Chat)
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
                if(Player[A].Character == 1)
                {
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 36;
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame, c, c, c);
                }
                else if(Player[A].Character == 2)
                {
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 38;
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame, c, c, c);
                }
                else if(Player[A].Character == 3)
                {
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 30;
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXPeach[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame);
                }
                else if(Player[A].Character == 4)
                {
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 30;
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame, c, c, c);
                }
                else if(Player[A].Character == 5)
                {
                    Y = 30;
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXLink[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame, c, c, c);
                }
            }
        }


        for(A = 1; A <= numNPCs; A++) // Put held NPCs on top
        {
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
//        For A = numPlayers To 1 Step -1 'Players in front of blocks
        for(int A = numPlayers; A >= 1; A--)// Players in front of blocks
        {
//            DrawPlayer A, Z
            DrawPlayer(A, Z);
//        Next A
        }
//'normal player end




//        If LevelEditor = True Or MagicHand = True Then
//            For A = 1 To numBackground 'Foreground objects
//                With Background(A)
//                    If Foreground(.Type) = True Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            If BackgroundHasNoMask(.Type) Then
//                                'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, GFXBackgroundWidth(.Type), GFXBackgroundHeight(.Type), GFXBackground(.Type), 0, 0, vbSrcCopy
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, GFXBackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                            Else
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                            End If
//                        End If
//                    End If
//                End With
//            Next A
//        Else

        { // NOT AN EDITOR
            for(A = LastBackground + 1; A <= numBackground; A++) // Foreground objects
            {
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, GFXBackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackground[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
//        End If
        }

        for(A = 1; A <= numNPCs; A++) // foreground NPCs
        {
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
                                if(NPCWidthGFX[NPC[A].Type] == 0)
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                                }
                                else
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
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
//                                    timeStr = timeStr + "2b" + std::to_string(A) + LB;
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
            if(BlockKills[Block[A].Type])
            {
                if(vScreenCollision(Z, Block[A].Location) && !Block[A].Hidden)
                {
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
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffectMask(.Type), 0, .Frame * EffectHeight(.Type), vbSrcAnd
//                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffect(.Type), 0, .Frame * EffectHeight(.Type), vbSrcPaint
                    float c = e.Shadow ? 0.f : 1.f;
                    frmMain.renderTexture(int(vScreenX[Z] + e.Location.X), int(vScreenY[Z] + e.Location.Y),
                                          int(e.Location.Width), int(e.Location.Height),
                                          GFXEffectBMP[e.Type], 0, e.Frame * EffectHeight[e.Type], c, c, c);
//                    End If
                }
//                End If
            }
//            End With
//        Next A
        }

//        'water
//        If LevelEditor = True Then
//            For B = 1 To numWater
//                With Water(B)
//                    If .Hidden = False Then
//                        For A = 1 To .Location.Height / 32
//                            BitBlt myBackBuffer, .Location.X + vScreenX(Z), .Location.Y + vScreenY(Z) + 32 * A - 32, 2, 32, GFX.Water.hdc, 0, 0, vbSrcCopy
//                            BitBlt myBackBuffer, .Location.X + vScreenX(Z) + .Location.Width - 2, .Location.Y + vScreenY(Z) + 32 * A - 32, 2, 32, GFX.Water.hdc, 0, 0, vbSrcCopy
//                        Next A
//                        For A = 1 To .Location.Width / 32
//                            BitBlt myBackBuffer, .Location.X + vScreenX(Z) + 32 * A - 32, .Location.Y + vScreenY(Z), 32, 2, GFX.Water.hdc, 0, 0, vbSrcCopy
//                            BitBlt myBackBuffer, .Location.X + vScreenX(Z) + 32 * A - 32, .Location.Y + vScreenY(Z) + .Location.Height - 2, 32, 2, GFX.Water.hdc, 0, 0, vbSrcCopy
//                        Next A
//                    End If
//                End With
//            Next B
//        End If


        if(!LevelEditor) // Graphics for the main game.
        {
        // NPC Generators
            for(A = 1; A <= numNPCs; A++)
            {
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
                        frmMain.renderRect(0, 0, vScreen[Z].Width, 1, 0.f, 0.f, 0.f);
                    }
                    else
                    {
                        frmMain.renderRect(0, vScreen[Z].Height - 1, vScreen[Z].Width, 1, 0.f, 0.f, 0.f);
                    }
                }
                else
                {
                    if(vScreen[Z].Left != 0.0)
                    {
                        frmMain.renderRect(0, 0, 1, vScreen[Z].Height, 0.f, 0.f, 0.f);
                    }
                    else
                    {
                        frmMain.renderRect(vScreen[Z].Width - 1, 0, 1, vScreen[Z].Height, 0.f, 0.f, 0.f);
                    }
                }
            }

        // player names
        /* Dropped */



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
                            frmMain.renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z] + 1, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                            frmMain.renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 - 20, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z], GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                            SuperPrint(tempString, 3,
                                       float(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 + 18),
                                       float(Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z]));
                        }
                    }
                }

    //                DrawInterface Z, numScreens
                DrawInterface(Z, numScreens);

                For(A, 1, numNPCs) // Display NPCs that got dropped from the container
                {
                    if(NPC[A].Effect == 2)
                    {
                        if(std::fmod(NPC[A].Effect2, 3) != 0.0)
                        {
                            if(vScreenCollision(Z, NPC[A].Location))
                            {
                                if(NPC[A].Active)
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
                                if(NPC[A].Reset[Z] || NPC[A].Active)
                                {
                                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                                    if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                        timeStr = timeStr + "2b" + std::to_string(A) + LB;
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


    //                If GamePaused = True Then
                if(GamePaused)
                {
                    if(MessageText.empty())
                    {
                        X = 0;
                        Y = 0;
                        if((DScreenType == 1 && Z == 2) || (DScreenType == 2 && Z == 1))
                            X = -400;
                        else if((DScreenType == 6 && Z == 2) || (DScreenType == 4 && Z == 2) || (DScreenType == 3 && Z == 1))
                            Y = -300;
                        frmMain.renderRect(210 + X, 200 + Y, 380, 200, 0.f, 0.f, 0.f);
                        if(TestLevel)
                        {
                            SuperPrint("CONTINUE", 3, 272 + X, 237 + Y);
                            SuperPrint("RESTART LEVEL", 3, 272 + X, 272 + Y);
                            SuperPrint("RESET CHECKPOINTS", 3, 272 + X, 307 + Y);
                            SuperPrint("QUIT TESTING", 3, 272 + X, 342 + Y);
                            frmMain.renderTexture(252 + X, 237 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursor[0], 0, 0);
                        }
                        else if(!Cheater && (LevelSelect || (/*StartLevel == FileName*/IsEpisodeIntro && NoMap)))
                        {
                            SuperPrint("CONTINUE", 3, 272 + X, 257 + Y);
                            SuperPrint("SAVE & CONTINUE", 3, 272 + X, 292 + Y);
                            SuperPrint("SAVE & QUIT", 3, 272 + X, 327 + Y);
                            frmMain.renderTexture(252 + X, 257 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursor[0], 0, 0);
                        }
                        else
                        {
                            SuperPrint("CONTINUE", 3, 272 + 56 + X, 275 + Y);
                            SuperPrint("QUIT", 3, 272 + 56 + X, 310 + Y);
                            frmMain.renderTexture(252 + 56 + X, 275 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursor[0], 0, 0);
                        }
                    }
                    else
                    {
                        X = 0;
                        Y = 0;
                        if((DScreenType == 1 && Z == 2) || (DScreenType == 2 && Z == 1))
                            X = -400;
                        else if((DScreenType == 6 && Z == 2) || (DScreenType == 4 && Z == 2) || (DScreenType == 3 && Z == 1))
                            Y = -300;

                        SuperText = MessageText;
                        BoxY = 150;
                        frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X,
                                              BoxY + Y + Y,
                                              GFX.TextBox.w, 20, GFX.TextBox, 0, 0);
                        BoxY += 10;
                        tempBool = false;
                        do
                        {
                            B = 0;
#if 0 // Old line breaking algorithm
                            for(A = 1; A <= int(SuperText.size()); A++)
                            {
                                if(SuperText[size_t(A) - 1] == ' ' || A == int(SuperText.size()))
                                {
                                    if(A < 28)
                                        B = A;
                                    else
                                        break;
                                }
                            }
#else // Better line breaking algorithm
                            for(A = 1; A <= int(SuperText.size()) && A < 27; A++)
                            {
                                auto c = SuperText[size_t(A) - 1];
                                if(A == int(SuperText.size()))
                                {
                                    if(A < 28)
                                        B = A;
                                }
                                else if(c == ' ')
                                    B = A;
                                else if(c == '\n')
                                {
                                    B = A;
                                    break;
                                }
                            }
#endif

                            if(B == 0)
                                B = A;

                            tempText = SuperText.substr(0, size_t(B));
//                            SuperText = SuperText.substr(size_t(B), SuperText.length());
                            SuperText.erase(0, size_t(B));

                            frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X, BoxY + Y + Y,
                                                  GFX.TextBox.w, 20, GFX.TextBox, 0, 20);
                            if(SuperText.length() == 0 && !tempBool)
                            {
                                SuperPrint(tempText,
                                           4,
                                           162 + X + (27 * 9) - (tempText.length() * 9),
                                           Y + BoxY);
                            }
                            else
                            {
                                SuperPrint(tempText, 4, 162 + X, Y + BoxY);
                            }
                            BoxY += 16;
                            tempBool = true;
                        } while(!SuperText.empty());
                        frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X, BoxY + Y + Y, GFX.TextBox.w, 10, GFX.TextBox, 0, GFX.TextBox.h - 10);
                    }
                }
    //            ElseIf GameOutro = False Then
            }

            else if(!GameOutro)
            {
                if(MenuMode != 1 && MenuMode != 2 && MenuMode != 4)
                    worldCurs = 0;

                int menuFix = -44; // for Input Settings

                frmMain.renderTexture(0, 0, GFX.MenuGFX[1].w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);
                frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70,
                        GFX.MenuGFX[2].w, GFX.MenuGFX[2].h, GFX.MenuGFX[2], 0, 0);

                frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[3].w / 2, 576,
                        GFX.MenuGFX[3].w, GFX.MenuGFX[3].h, GFX.MenuGFX[3], 0, 0);

                if(MenuMode == 0)
                {
                    SuperPrint(g_mainMenu.main1PlayerGame, 3, 300, 350);
                    SuperPrint(g_mainMenu.main2PlayerGame, 3, 300, 380);
                    SuperPrint(g_mainMenu.mainBattleGame, 3, 300, 410);
                    SuperPrint(g_mainMenu.mainOptions, 3, 300, 440);
                    SuperPrint(g_mainMenu.mainExit, 3, 300, 470);
                    frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor[0], 0, 0);
                }
                // Character select
                else if(MenuMode == 100 || MenuMode == 200 || MenuMode == 300 || MenuMode == 400 || MenuMode == 500)
                {
                    A = 0;
                    B = 0;
                    C = 0;

                    // TODO: Make a custom playable character names print here
                    if(!blockCharacter[1])
                        SuperPrint(g_mainMenu.selectPlayer[1], 3, 300, 350);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 1)
                            B = B - 30;
                        if(PlayerCharacter >= 1)
                            C = C - 30;
                    }

                    if(!blockCharacter[2])
                        SuperPrint(g_mainMenu.selectPlayer[2], 3, 300, 380 + A);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 2)
                            B = B - 30;
                        if(PlayerCharacter >= 2)
                            C = C - 30;
                    }

                    if(!blockCharacter[3])
                        SuperPrint(g_mainMenu.selectPlayer[3], 3, 300, 410 + A);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 3)
                            B = B - 30;
                        if(PlayerCharacter >= 3)
                            C = C - 30;
                    }

                    if(!blockCharacter[4])
                        SuperPrint(g_mainMenu.selectPlayer[4], 3, 300, 440 + A);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 4)
                            B = B - 30;
                        if(PlayerCharacter >= 4)
                            C = C - 30;
                    }

                    if(!blockCharacter[5])
                        SuperPrint(g_mainMenu.selectPlayer[5], 3, 300, 470 + A);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 5)
                            B = B - 30;
                        if(PlayerCharacter >= 5)
                            C = C - 30;
                    }

                    if(MenuMode == 300 || MenuMode == 500)
                    {
                        frmMain.renderTexture(300 - 20, B + 350 + (MenuCursor * 30), GFX.MCursor[3]);
                        frmMain.renderTexture(300 - 20, B + 350 + ((PlayerCharacter - 1) * 30), GFX.MCursor[0]);
                    }
                    else
                    {
                        frmMain.renderTexture(300 - 20, B + 350 + (MenuCursor * 30), GFX.MCursor[0]);
                    }

                }
                else if(MenuMode == 1 || MenuMode == 2 || MenuMode == 4)
                {
                    std::string tempStr = "";
                    minShow = 1;
                    maxShow = NumSelectWorld;
                    if(NumSelectWorld > 5)
                    {
                        minShow = worldCurs;
                        maxShow = minShow + 4;

                        if(MenuCursor <= minShow - 1)
                            worldCurs = worldCurs - 1;
                        if(MenuCursor >= maxShow - 1)
                            worldCurs = worldCurs + 1;

                        if(worldCurs < 1)
                            worldCurs = 1;
                        if(worldCurs > NumSelectWorld - 4)
                            worldCurs = NumSelectWorld - 4;

                        if(maxShow >= NumSelectWorld)
                        {
                            maxShow = NumSelectWorld;
                            minShow = NumSelectWorld - 4;
                        }

                        minShow = worldCurs;
                        maxShow = minShow + 4;
                    }

                    for(auto A = minShow; A <= maxShow; A++)
                    {
                        B = A - minShow + 1;
                        tempStr = SelectWorld[A].WorldName;
                        SuperPrint(tempStr, 3, 300, 320 + (B * 30));
                    }

                    if(minShow > 1)
                    {
                        frmMain.renderTexture(400 - 8, 350 - 20, GFX.MCursor[1]);
                    }
                    if(maxShow < NumSelectWorld)
                    {
                        frmMain.renderTexture(400 - 8, 490, GFX.MCursor[2]);
                    }

                    B = MenuCursor - minShow + 1;
                    if(B >= 0 && B < 5)
                    {
                        frmMain.renderTexture(300 - 20, 350 + (B * 30), GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
                    }
                }

                else if(MenuMode == 10 || MenuMode == 20) // Save Select
                {
                    for(auto A = 1; A <= maxSaveSlots; A++)
                    {
                        if(SaveSlot[A] >= 0)
                        {
                            SuperPrint(fmt::format_ne("SLOT {0} ... {1}", A, SaveSlot[A]), 3, 300, 320 + (A * 30));
                            if(SaveStars[A] > 0)
                            {
                                frmMain.renderTexture(560, 320 + (A * 30) + 1,
                                                      GFX.Interface[5].w, GFX.Interface[5].h,
                                                      GFX.Interface[5], 0, 0);
                                frmMain.renderTexture(560 + 24, 320 + (A * 30) + 2,
                                                      GFX.Interface[1].w, GFX.Interface[1].h,
                                                      GFX.Interface[1], 0, 0);
                                SuperPrint(fmt::format_ne(" {0}", SaveStars[A]), 3, 588, 320 + (A * 30));
                            }
                        }
                        else
                        {
                            SuperPrint(fmt::format_ne("SLOT {0} ... EMPTY", A), 3, 300, 320 + (A * 30));
                        }
                    }
                    frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), GFX.MCursor[0]);
                }

                // Options Menu
                else if(MenuMode == 3)
                {
    //                    SuperPrint "PLAYER 1 CONTROLS", 3, 300, 350
                    SuperPrint("PLAYER 1 CONTROLS", 3, 300, 350);
    //                    SuperPrint "PLAYER 2 CONTROLS", 3, 300, 380
                    SuperPrint("PLAYER 2 CONTROLS", 3, 300, 380);
    //                    If resChanged = True Then
                    if(resChanged)
    //                        SuperPrint "WINDOWED MODE", 3, 300, 410
                        SuperPrint("WINDOWED MODE", 3, 300, 410);
    //                    Else
                    else
    //                        SuperPrint "FULLSCREEN MODE", 3, 300, 410
                        SuperPrint("FULLSCREEN MODE", 3, 300, 410);
    //                    End If
    //                    SuperPrint "VIEW CREDITS", 3, 300, 440
                    SuperPrint("VIEW CREDITS", 3, 300, 440);
    //                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
    //                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
                    frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30),
                                          GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    //                ElseIf MenuMode = 31 Or MenuMode = 32 Then
                }
                else if(MenuMode == 31 || MenuMode == 32)
                {
    //                    If useJoystick(MenuMode - 30) = 0 Then
                    if(useJoystick[MenuMode - 30] == 0)
                    {
    //                        SuperPrint "INPUT......KEYBOARD", 3, 300, 260 + menuFix
                        SuperPrint("INPUT......KEYBOARD", 3, 300, 260 + menuFix);
                        SuperPrint(fmt::format_ne("UP.........{0}", getKeyName(conKeyboard[MenuMode - 30].Up)), 3, 300, 290 + menuFix);
                        SuperPrint(fmt::format_ne("DOWN.......{0}", getKeyName(conKeyboard[MenuMode - 30].Down)), 3, 300, 320 + menuFix);
                        SuperPrint(fmt::format_ne("LEFT.......{0}", getKeyName(conKeyboard[MenuMode - 30].Left)), 3, 300, 350 + menuFix);
                        SuperPrint(fmt::format_ne("RIGHT......{0}", getKeyName(conKeyboard[MenuMode - 30].Right)), 3, 300, 380 + menuFix);
                        SuperPrint(fmt::format_ne("RUN........{0}", getKeyName(conKeyboard[MenuMode - 30].Run)), 3, 300, 410 + menuFix);
                        SuperPrint(fmt::format_ne("ALT RUN....{0}", getKeyName(conKeyboard[MenuMode - 30].AltRun)), 3, 300, 440 + menuFix);
                        SuperPrint(fmt::format_ne("JUMP.......{0}", getKeyName(conKeyboard[MenuMode - 30].Jump)), 3, 300, 470 + menuFix);
                        SuperPrint(fmt::format_ne("ALT JUMP...{0}", getKeyName(conKeyboard[MenuMode - 30].AltJump)), 3, 300, 500 + menuFix);
                        SuperPrint(fmt::format_ne("DROP ITEM..{0}", getKeyName(conKeyboard[MenuMode - 30].Drop)), 3, 300, 530 + menuFix);
                        SuperPrint(fmt::format_ne("PAUSE......{0}", getKeyName(conKeyboard[MenuMode - 30].Start)), 3, 300, 560 + menuFix);
    //                    Else
                    }
                    else
                    {
                        SuperPrint("INPUT......JOYSTICK " + std::to_string(useJoystick[MenuMode - 30]), 3, 300, 260 + menuFix);

                        SuperPrint(fmt::format_ne("UP.........{0}", getJoyKeyName(conJoystick[MenuMode - 30].Up)), 3, 300, 290 + menuFix);
                        SuperPrint(fmt::format_ne("DOWN.......{0}", getJoyKeyName(conJoystick[MenuMode - 30].Down)), 3, 300, 320 + menuFix);
                        SuperPrint(fmt::format_ne("LEFT.......{0}", getJoyKeyName(conJoystick[MenuMode - 30].Left)), 3, 300, 350 + menuFix);
                        SuperPrint(fmt::format_ne("RIGHT......{0}", getJoyKeyName(conJoystick[MenuMode - 30].Right)), 3, 300, 380 + menuFix);
                        SuperPrint(fmt::format_ne("RUN........{0}", getJoyKeyName(conJoystick[MenuMode - 30].Run)), 3, 300, 410 + menuFix);
                        SuperPrint(fmt::format_ne("ALT RUN....{0}", getJoyKeyName(conJoystick[MenuMode - 30].AltRun)), 3, 300, 440 + menuFix);
                        SuperPrint(fmt::format_ne("JUMP.......{0}", getJoyKeyName(conJoystick[MenuMode - 30].Jump)), 3, 300, 470 + menuFix);
                        SuperPrint(fmt::format_ne("ALT JUMP...{0}", getJoyKeyName(conJoystick[MenuMode - 30].AltJump)), 3, 300, 500 + menuFix);
                        SuperPrint(fmt::format_ne("DROP ITEM..{0}", getJoyKeyName(conJoystick[MenuMode - 30].Drop)), 3, 300, 530 + menuFix);
                        SuperPrint(fmt::format_ne("PAUSE......{0}", getJoyKeyName(conJoystick[MenuMode - 30].Start)), 3, 300, 560 + menuFix);

//                        if(conJoystick[MenuMode - 30].Run >= 0)
//                            SuperPrint(fmt::format_ne("RUN........{0}", conJoystick[MenuMode - 30].Run), 3, 300, 290 + menuFix);
//                        else
//                            SuperPrint("RUN........_", 3, 300, 290 + menuFix);

//                        if(conJoystick[MenuMode - 30].AltRun >= 0)
//                            SuperPrint(fmt::format_ne("ALT RUN....{0}", conJoystick[MenuMode - 30].AltRun), 3, 300, 320 + menuFix);
//                        else
//                            SuperPrint("ALT RUN...._", 3, 300, 320 + menuFix);

//                        if(conJoystick[MenuMode - 30].Jump >= 0)
//                            SuperPrint(fmt::format_ne("JUMP.......{0}", conJoystick[MenuMode - 30].Jump), 3, 300, 350 + menuFix);
//                        else
//                            SuperPrint("JUMP......._", 3, 300, 350 + menuFix);

//                        if(conJoystick[MenuMode - 30].AltJump >= 0)
//                            SuperPrint(fmt::format_ne("ALT JUMP...{0}", conJoystick[MenuMode - 30].AltJump), 3, 300, 380 + menuFix);
//                        else
//                            SuperPrint("ALT JUMP..._", 3, 300, 380 + menuFix);

//                        if(conJoystick[MenuMode - 30].Drop >= 0)
//                            SuperPrint(fmt::format_ne("DROP ITEM..{0}", conJoystick[MenuMode - 30].Drop), 3, 300, 410 + menuFix);
//                        else
//                            SuperPrint("DROP ITEM.._", 3, 300, 410 + menuFix);

//                        if(conJoystick[MenuMode - 30].Start >= 0)
//                            SuperPrint(fmt::format_ne("PAUSE......{0}", conJoystick[MenuMode - 30].Start), 3, 300, 440 + menuFix);
//                        else
//                            SuperPrint("PAUSE......_", 3, 300, 440 + menuFix);
                    }

                    frmMain.renderTexture(300 - 20, 260 + (MenuCursor * 30) + menuFix,
                                          GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
                }

                frmMain.renderTexture(int(MenuMouseX), int(MenuMouseY), GFX.ECursor[2]);
            }
            if(PrintFPS > 0) {
                SuperPrint(fmt::format_ne("{0}", int(PrintFPS)), 1, 8, 8, 0.f, 1.f, 0.f);
            }
        }

//        If LevelEditor = True Or MagicHand = True Then
        if((LevelEditor || MagicHand) && !GamePaused)
        {

#if 0 //.Useless editor-only stuff
//            If LevelEditor = True Then
            if(LevelEditor)
            {

    //            BlockFlash = BlockFlash + 1
                BlockFlash += 1;

    //            If BlockFlash > 45 Then BlockFlash = 0
                if(BlockFlash > 45)
                    BlockFlash = 0;

    //            If BlockFlash <= 30 Then
                if(BlockFlash <= 30)
                {
    //                For A = 1 To numBlock
    //                    If Block(A).Special > 1000 Then
    //                        If Block(A).Hidden = False Then
    //                            If vScreenCollision(Z, Block(A).Location) Then
    //                                If NPCWidthGFX(Block(B).Special - 1000) = 0 Then
    //                                    tempLocation.X = Block(A).Location.X + Block(A).Location.Width / 2 - NPCWidth(Block(A).Special - 1000) / 2
    //                                    tempLocation.Y = Block(A).Location.Y + Block(A).Location.Height / 2 - NPCHeight(Block(A).Special - 1000) / 2
    //                                    tempLocation.Height = NPCHeight(Block(A).Special - 1000)
    //                                    tempLocation.Width = NPCWidth(Block(A).Special - 1000)
    //                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(Block(A).Special - 1000), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(Block(A).Special - 1000), 0, EditorNPCFrame(Block(A).Special - 1000, -1) * tempLocation.Height, vbSrcAnd
    //                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(Block(A).Special - 1000), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(Block(A).Special - 1000), 0, EditorNPCFrame(Block(A).Special - 1000, -1) * tempLocation.Height, vbSrcPaint
    //                                Else
    //                                    tempLocation.X = Block(A).Location.X + Block(A).Location.Width / 2 - NPCWidthGFX(Block(A).Special - 1000) / 2
    //                                    tempLocation.Y = Block(A).Location.Y + Block(A).Location.Height / 2 - NPCHeightGFX(Block(A).Special - 1000) / 2
    //                                    tempLocation.Height = NPCHeightGFX(Block(A).Special - 1000)
    //                                    tempLocation.Width = NPCWidthGFX(Block(A).Special - 1000)
    //                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + (NPCFrameOffsetX(Block(A).Special - 1000)) - NPCWidthGFX(Block(A).Special - 1000) / 2 + tempLocation.Width / 2, vScreenY(Z) + tempLocation.Y + NPCFrameOffsetY(Block(A).Special - 1000) - NPCHeightGFX(Block(A).Special - 1000) + tempLocation.Height, NPCWidthGFX(Block(A).Special - 1000), NPCHeightGFX(Block(A).Special - 1000), GFXNPCMask(Block(A).Special - 1000), 0, EditorNPCFrame(Block(A).Special - 1000, -1) * NPCHeightGFX(Block(A).Special - 1000), vbSrcAnd
    //                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + (NPCFrameOffsetX(Block(A).Special - 1000)) - NPCWidthGFX(Block(A).Special - 1000) / 2 + tempLocation.Width / 2, vScreenY(Z) + tempLocation.Y + NPCFrameOffsetY(Block(A).Special - 1000) - NPCHeightGFX(Block(A).Special - 1000) + tempLocation.Height, NPCWidthGFX(Block(A).Special - 1000), NPCHeightGFX(Block(A).Special - 1000), GFXNPC(Block(A).Special - 1000), 0, EditorNPCFrame(Block(A).Special - 1000, -1) * NPCHeightGFX(Block(A).Special - 1000), vbSrcPaint
    //                                End If
    //                            End If
    //                        End If
    //                    ElseIf Block(A).Special > 0 And Block(A).Hidden = False Then
    //                        tempLocation.X = Block(A).Location.X + Block(A).Location.Width / 2 - NPCWidth(10) / 2
    //                        tempLocation.Y = Block(A).Location.Y + Block(A).Location.Height / 2 - NPCHeight(10) / 2
    //                        tempLocation.Height = NPCHeight(10)
    //                        tempLocation.Width = NPCWidth(10)
    //                        BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(10), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(10), 0, EditorNPCFrame(10, -1) * tempLocation.Height, vbSrcAnd
    //                        BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(10), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(10), 0, EditorNPCFrame(10, -1) * tempLocation.Height, vbSrcPaint
    //                    End If
    //                Next A

    //                For A = 1 To numNPCs
    //                    If NPC(A).Hidden = False And (NPC(A).Type = 91 Or NPC(A).Type = 96) Then
    //                        If NPC(A).Special > 0 Then
    //                            If vScreenCollision(Z, NPC(A).Location) Then
    //                                If NPCWidthGFX(NPC(A).Special) = 0 Then
    //                                    tempLocation.Height = NPCHeight(NPC(A).Special)
    //                                    tempLocation.Width = NPCWidth(NPC(A).Special)
    //                                    If NPC(A).Type = 96 Then
    //                                        tempLocation.Y = NPC(A).Location.Y + NPC(A).Location.Height - tempLocation.Height
    //                                    Else
    //                                        tempLocation.Y = NPC(A).Location.Y
    //                                    End If
    //                                    tempLocation.X = NPC(A).Location.X + NPC(A).Location.Width / 2 - tempLocation.Width / 2
    //                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(NPC(A).Special), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(NPC(A).Special), 0, EditorNPCFrame(Int(NPC(A).Special), -1) * tempLocation.Height, vbSrcAnd
    //                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(NPC(A).Special), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(NPC(A).Special), 0, EditorNPCFrame(Int(NPC(A).Special), -1) * tempLocation.Height, vbSrcPaint
    //                                Else
    //                                    tempLocation.Height = NPCHeightGFX(NPC(A).Special)
    //                                    tempLocation.Width = NPCWidthGFX(NPC(A).Special)
    //                                    If NPC(A).Type = 96 Then
    //                                        tempLocation.Y = NPC(A).Location.Y + NPC(A).Location.Height - tempLocation.Height
    //                                    Else
    //                                        tempLocation.Y = NPC(A).Location.Y
    //                                    End If
    //                                    tempLocation.X = NPC(A).Location.X + NPC(A).Location.Width / 2 - tempLocation.Width / 2
    //                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + (NPCFrameOffsetX(NPC(A).Special)) - NPCWidthGFX(NPC(A).Special) / 2 + tempLocation.Width / 2, vScreenY(Z) + tempLocation.Y + NPCFrameOffsetY(NPC(A).Special) - NPCHeightGFX(NPC(A).Special) + tempLocation.Height, NPCWidthGFX(NPC(A).Special), NPCHeightGFX(NPC(A).Special), GFXNPCMask(NPC(A).Special), 0, EditorNPCFrame(Int(NPC(A).Special), -1) * NPCHeightGFX(NPC(A).Special), vbSrcAnd
    //                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + (NPCFrameOffsetX(NPC(A).Special)) - NPCWidthGFX(NPC(A).Special) / 2 + tempLocation.Width / 2, vScreenY(Z) + tempLocation.Y + NPCFrameOffsetY(NPC(A).Special) - NPCHeightGFX(NPC(A).Special) + tempLocation.Height, NPCWidthGFX(NPC(A).Special), NPCHeightGFX(NPC(A).Special), GFXNPC(NPC(A).Special), 0, EditorNPCFrame(Int(NPC(A).Special), -1) * NPCHeightGFX(NPC(A).Special), vbSrcPaint
    //                                End If
    //                            End If
    //                        End If
    //                    End If
    //                Next A

    //            End If
                }


    //            For A = 1 To 2 'Player start locations
    //                If PlayerStart(A).Width > 0 Then
    //                    C = Physics.PlayerHeight(testPlayer(A).Character, 2) - Physics.PlayerHeight(A, 2)
    //                    If vScreenCollision(Z, PlayerStart(A)) Then
    //                        If testPlayer(A).Character = 1 Then
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + MarioFrameX(201), vScreenY(Z) + PlayerStart(A).Y + MarioFrameY(201) - C, 99, 99, GFXMarioMask(2), 500, 0, vbSrcAnd
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + MarioFrameX(201), vScreenY(Z) + PlayerStart(A).Y + MarioFrameY(201) - C, 99, 99, GFXMario(2), 500, 0, vbSrcPaint
    //                        ElseIf testPlayer(A).Character = 2 Then
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + LuigiFrameX(201), vScreenY(Z) + PlayerStart(A).Y + LuigiFrameY(201) - C, 99, 99, GFXLuigiMask(2), 500, 0, vbSrcAnd
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + LuigiFrameX(201), vScreenY(Z) + PlayerStart(A).Y + LuigiFrameY(201) - C, 99, 99, GFXLuigi(2), 500, 0, vbSrcPaint
    //                        ElseIf testPlayer(A).Character = 3 Then
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + PeachFrameX(201), vScreenY(Z) + PlayerStart(A).Y + PeachFrameY(201) - C, 99, 99, GFXPeachMask(2), 500, 0, vbSrcAnd
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + PeachFrameX(201), vScreenY(Z) + PlayerStart(A).Y + PeachFrameY(201) - C, 99, 99, GFXPeach(2), 500, 0, vbSrcPaint
    //                        ElseIf testPlayer(A).Character = 4 Then
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + ToadFrameX(201), vScreenY(Z) + PlayerStart(A).Y + ToadFrameY(201) - C, 99, 99, GFXToadMask(2), 500, 0, vbSrcAnd
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + ToadFrameX(201), vScreenY(Z) + PlayerStart(A).Y + ToadFrameY(201) - C, 99, 99, GFXToad(2), 500, 0, vbSrcPaint
    //                        ElseIf testPlayer(A).Character = 5 Then
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + LinkFrameX(201), vScreenY(Z) + PlayerStart(A).Y + LinkFrameY(201) - C, 99, 99, GFXLinkMask(2), 500, 0, vbSrcAnd
    //                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + LinkFrameX(201), vScreenY(Z) + PlayerStart(A).Y + LinkFrameY(201) - C, 99, 99, GFXLink(2), 500, 0, vbSrcPaint
    //                        End If
    //                    End If
    //                End If
    //            Next A

    //            For A = 0 To maxSections 'Show sections
    //                If A <> curSection Then
    //                    With tempLocation
    //                        .X = level(A).X
    //                        .Y = level(A).Y
    //                        .Width = level(A).Width - .X
    //                        .Height = level(A).Height - .Y
    //                        If .X < -vScreenX(Z) Then
    //                            .Width = .Width - (-vScreenX(Z) - .X)
    //                            .X = -vScreenX(Z)

    //                        End If
    //                        If .Y < -vScreenY(Z) Then
    //                            .Height = .Height - (-vScreenY(Z) - .Y)
    //                            .Y = -vScreenY(Z)
    //                        End If
    //                        BitBlt myBackBuffer, .X + vScreenX(Z), .Y + vScreenY(Z), .Width, .Height, 0, 0, 0, vbWhiteness
    //                    End With
    //                End If
    //            Next A

    //            For A = 1 To numWarps 'Warps
    //                With Warp(A)
    //                    If .Direction > 0 And .Hidden = False Then
    //                        If .PlacedEnt = True Then
    //                            BitBlt myBackBuffer, vScreenX(Z) + .Entrance.X, vScreenY(Z) + .Entrance.Y, .Entrance.Width, .Entrance.Height, GFX.WarpMask(1).hdc, 0, 0, vbSrcAnd
    //                            BitBlt myBackBuffer, vScreenX(Z) + .Entrance.X, vScreenY(Z) + .Entrance.Y, .Entrance.Width, .Entrance.Height, GFX.Warp(1).hdc, 0, 0, vbSrcPaint
    //                            SuperPrint Str(A), 1, Int(.Entrance.X + 2 + vScreenX(Z)), Int(.Entrance.Y + 2 + vScreenY(Z))
    //                        End If
    //                        If .PlacedExit = True Then
    //                            BitBlt myBackBuffer, vScreenX(Z) + .Exit.X, vScreenY(Z) + .Exit.Y, .Exit.Width, .Exit.Height, GFX.WarpMask(1).hdc, 0, 0, vbSrcAnd
    //                            BitBlt myBackBuffer, vScreenX(Z) + .Exit.X, vScreenY(Z) + .Exit.Y, .Exit.Width, .Exit.Height, GFX.Warp(1).hdc, 0, 0, vbSrcPaint
    //                            SuperPrint Str(A), 1, Int(.Exit.X + .Exit.Width - 16 - 2 + vScreenX(Z)), Int(.Exit.Y + .Exit.Height - 14 - 2 + vScreenY(Z))
    //                        End If
    //                    End If
    //                End With
    //            Next A
//            End If
            }

//            If LevelEditor = True Then
            if(LevelEditor)
            {
//                If BlockFlash > 30 Or BlockFlash = 0 Then
//                    With tempLocation 'Black out the level edges
//                        .X = level(curSection).X
//                        .Y = level(curSection).Y
//                        .Width = level(curSection).Width
//                        .Height = level(curSection).Height
//                        BitBlt myBackBuffer, 0, 0, vScreenX(Z) + level(curSection).X, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, 0, 0, vScreenX(Z) + level(curSection).Width, vScreenY(Z) + level(curSection).Y, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        If -vScreenX(Z) < level(curSection).Width Then
//                            BitBlt myBackBuffer, vScreenX(Z) + level(curSection).Width, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        End If
//                        If -vScreenY(Z) < level(curSection).Height Then
//                            BitBlt myBackBuffer, 0, vScreenY(Z) + level(curSection).Height, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        End If
//                    End With
//                End If
//            End If
            }

#endif

//If nPlay.Online = True Then
//    For A = 0 To 15
//        With nPlay.Player(A)
//            If nPlay.Player(A).Active = True And nPlay.Player(A).IsMe = False Then
//                If nPlay.Player(A).Nick = "Redigit" Then
//                    nPlay.Player(A).Cursor = Int(Rnd * 8)
//                    If Rnd * 100 > 80 Then
//                        NewEffect 80, newLoc(.ECurserX, .ECurserY)
//                        Effect(numEffects).Location.SpeedX = Rnd * 4 - 2
//                        Effect(numEffects).Location.SpeedY = Rnd * 4 - 2
//                    End If
//                End If
//                BitBlt myBackBuffer, vScreenX(Z) + .ECurserX, vScreenY(Z) + .ECurserY, GFX.nCursor(.Cursor).ScaleWidth, GFX.nCursor(.Cursor).ScaleHeight, GFX.nCursorMask(.Cursor).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreenX(Z) + .ECurserX, vScreenY(Z) + .ECurserY, GFX.nCursor(.Cursor).ScaleWidth, GFX.nCursor(.Cursor).ScaleHeight, GFX.nCursor(.Cursor).hdc, 0, 0, vbSrcPaint
//                SuperPrint UCase(.Nick), 3, vScreenX(Z) + .ECurserX + 28, vScreenY(Z) + .ECurserY + 34
//            End If
//        End With
//    Next A
//End If

            if(!MessageText.empty()) // In-Editor message box preview
            {
                X = 0;
                Y = 0;
                if((DScreenType == 1 && Z == 2) || (DScreenType == 2 && Z == 1))
                    X = -400;
                else if((DScreenType == 6 && Z == 2) || (DScreenType == 4 && Z == 2) || (DScreenType == 3 && Z == 1))
                    Y = -300;

                SuperText = MessageText;
                BoxY = 150;
                frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X,
                                      BoxY + Y + Y,
                                      GFX.TextBox.w, 20, GFX.TextBox, 0, 0);
                BoxY += 10;
                tempBool = false;
                do
                {
                    B = 0;
                    for(A = 1; A <= int(SuperText.size()); A++)
                    {
                        if(SuperText[size_t(A) - 1] == ' ' || A == int(SuperText.size()))
                        {
                            if(A < 28)
                                B = A;
                            else
                                break;
                        }
                    }

                    if(B == 0)
                        B = A;

                    tempText = SuperText.substr(0, size_t(B));
                    SuperText = SuperText.substr(size_t(B), SuperText.length());
                    frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X, BoxY + Y + Y,
                                          GFX.TextBox.w, 20, GFX.TextBox, 0, 20);
                    if(SuperText.length() == 0 && !tempBool)
                    {
                        SuperPrint(tempText,
                                   4,
                                   float(162 + X + (27 * 9)) - (tempText.length() * 9),
                                   Y + BoxY);
                    }
                    else
                    {
                        SuperPrint(tempText, 4, 162 + X, Y + BoxY);
                    }
                    BoxY += 16;
                    tempBool = true;
                } while(!SuperText.empty());
                frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X, BoxY + Y + Y, GFX.TextBox.w, 10, GFX.TextBox, 0, GFX.TextBox.h - 10);
            }



            // Display the cursor
            {
                auto &e = EditorCursor;
                int curX = int(double(e.X) - vScreen[Z].Left);
                int curY = int(double(e.Y) - vScreen[Z].Top);

                if(e.Mode == OptCursor_t::LVL_BLOCKS) // Blocks
                {
                    auto &b = e.Block;
                    if(BlockIsSizable[b.Type])
                    {
                        if(vScreenCollision(Z, b.Location))
                        {
                            for(B = 0; B <= (b.Location.Height / 32) - 1; B++)
                            {
                                for(C = 0; C <= (b.Location.Width / 32) - 1; C++)
                                {
                                    D = C;
                                    E = B;

                                    if(D != 0)
                                    {
                                        if(fEqual(D, (b.Location.Width / 32) - 1))
                                            D = 2;
                                        else
                                        {
                                            D = 1;
                                            d2 = 0.5;
                                        }
                                    }

                                    if(E != 0)
                                    {
                                        if(fEqual(E, (b.Location.Height / 32) - 1))
                                            E = 2;
                                        else
                                            E = 1;
                                    }

                                    frmMain.renderTexture(vScreenX[Z] + b.Location.X + C * 32,
                                                          vScreenY[Z] + b.Location.Y + B * 32,
                                                          32, 32, GFXBlock[b.Type], D * 32, E * 32);
//                                    if((D == 0 || D == 2) || (E == 0 || E == 2))
//                                    {
//                                        frmMain.renderTexture(vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlock(.Type), D * 32, E * 32)
//                                    }
//                                    else
//                                        frmMain.renderTexture(vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlock(.Type), D * 32, E * 32)

                                }
                            }
                        }
                    }
                    else
                    {
                        if(vScreenCollision(Z, b.Location))
                        {
                            frmMain.renderTexture(vScreenX[Z] + b.Location.X,
                                                  vScreenY[Z] + b.Location.Y + b.ShakeY3,
                                                  b.Location.Width,
                                                  b.Location.Height,
                                                  GFXBlock[b.Type], 0, BlockFrame[b.Type] * 32);
                        }
                    }
                }

                else if(e.Mode == OptCursor_t::LVL_SETTINGS) // Player start points
                {
//                    If frmLevelSettings.optLevel(4).Value = True Or frmLevelSettings.optLevel(5).Value = True Then
//                        If frmLevelSettings.optLevel(4).Value = True Then
//                            A = 1
//                        Else
//                            A = 2
//                        End If
//                        C = Physics.PlayerHeight(testPlayer(A).Character, 2) - Physics.PlayerHeight(A, 2)
//                        With EditorCursor
//                            If vScreenCollision(Z, .Location) Then
//                                If testPlayer(A).Character = 1 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX(201), vScreenY(Z) + .Location.Y + MarioFrameY(201) - C, 99, 99, GFXMarioMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX(201), vScreenY(Z) + .Location.Y + MarioFrameY(201) - C, 99, 99, GFXMario(2), 500, 0, vbSrcPaint
//                                ElseIf testPlayer(A).Character = 2 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX(201), vScreenY(Z) + .Location.Y + LuigiFrameY(201) - C, 99, 99, GFXLuigiMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX(201), vScreenY(Z) + .Location.Y + LuigiFrameY(201) - C, 99, 99, GFXLuigi(2), 500, 0, vbSrcPaint
//                                ElseIf testPlayer(A).Character = 3 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + PeachFrameX(201), vScreenY(Z) + .Location.Y + PeachFrameY(201) - C, 99, 99, GFXPeachMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + PeachFrameX(201), vScreenY(Z) + .Location.Y + PeachFrameY(201) - C, 99, 99, GFXPeach(2), 500, 0, vbSrcPaint
//                                ElseIf testPlayer(A).Character = 4 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX(201), vScreenY(Z) + .Location.Y + ToadFrameY(201) - C, 99, 99, GFXToadMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX(201), vScreenY(Z) + .Location.Y + ToadFrameY(201) - C, 99, 99, GFXToad(2), 500, 0, vbSrcPaint
//                                ElseIf testPlayer(A).Character = 5 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LinkFrameX(201), vScreenY(Z) + .Location.Y + LinkFrameY(201) - C, 99, 99, GFXLinkMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LinkFrameX(201), vScreenY(Z) + .Location.Y + LinkFrameY(201) - C, 99, 99, GFXLink(2), 500, 0, vbSrcPaint
//                                End If
//                            End If
//                        End With
//                    End If
                }

                else if(e.Mode == OptCursor_t::LVL_BGOS) // BGOs
                {
                    auto &b = e.Background;
                    if(vScreenCollision(Z, b.Location))
                    {
                        frmMain.renderTexture(vScreenX[Z] + b.Location.X,
                                              vScreenY[Z] + b.Location.Y,
                                              BackgroundWidth[b.Type],
                                              BackgroundHeight[b.Type],
                                              GFXBackground[b.Type], 0,
                                              BackgroundHeight[b.Type] * BackgroundFrame[b.Type]);
                    }
                }

                else if(e.Mode == OptCursor_t::LVL_NPCS) // NPCs
                {
                    e.NPC.Frame = NPC[0].Frame;
                    e.NPC.FrameCount = NPC[0].FrameCount;
                    NPC[0] = e.NPC;
                    NPCFrames(0);
                    e.NPC = NPC[0];

                    auto &n = e.NPC;
                    if(NPCWidthGFX[n.Type] == 0)
                    {
                        frmMain.renderTexture(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type],
                                              vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type],
                                              n.Location.Width,
                                              n.Location.Height,
                                              GFXNPC[n.Type], 0, n.Frame * n.Location.Height);
                    }
                    else
                    {
                        if(n.Type == 283 && n.Special > 0)
                        {
                            if(NPCWidthGFX[n.Special] == 0)
                            {
                                tempLocation.Width = NPCWidth[n.Special];
                                tempLocation.Height = NPCHeight[n.Special];
                            }
                            else
                            {
                                tempLocation.Width = NPCWidthGFX[n.Special];
                                tempLocation.Height = NPCHeightGFX[n.Special];
                            }
                            tempLocation.X = n.Location.X + n.Location.Width / 2 - tempLocation.Width / 2;
                            tempLocation.Y = n.Location.Y + n.Location.Height / 2 - tempLocation.Height / 2;
                            B = EditorNPCFrame(int(n.Special), n.Direction);

                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[n.Type],
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width, tempLocation.Height,
                                                  GFXNPC[n.Special], 0, B * tempLocation.Height);
                        }

                        frmMain.renderTexture(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] - NPCWidthGFX[n.Type] / 2 + n.Location.Width / 2,
                                              vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] - NPCHeightGFX[n.Type] + n.Location.Height,
                                              NPCWidthGFX[n.Type], NPCHeightGFX[n.Type], GFXNPC[n.Type],
                                              0, n.Frame * NPCHeightGFX[n.Type]);
                    }
                }

                if(EditorCursor.Mode == 0 || EditorCursor.Mode == 6) // Eraser
                {
                    frmMain.renderTexture(curX - 2, curY, GFX.ECursor[3]);
                }

                else if(EditorCursor.Mode == 13 || EditorCursor.Mode == 14) // Selector
                {
                    frmMain.renderTexture(curX, curY, GFX.ECursor[2]);
                }

                // ElseIf .Mode = 2 And (frmLevelSettings.optLevel(0).Value = True Or frmLevelSettings.optLevel(1).Value = True Or frmLevelSettings.optLevel(2).Value = True Or frmLevelSettings.optLevel(3).Value = True) Then
                else if(EditorCursor.Mode == 2)
                {
                    frmMain.renderTexture(curX, curY, GFX.ECursor[1]);
                }

                else if(EditorCursor.Mode == 15) // Water
                {
//                    For A = 1 To .Location.Height / 32
//                        BitBlt myBackBuffer, .Location.X + vScreenX(Z), .Location.Y + vScreenY(Z) + 32 * A - 32, 2, 32, GFX.Water.hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, .Location.X + vScreenX(Z) + .Location.Width - 2, .Location.Y + vScreenY(Z) + 32 * A - 32, 2, 32, GFX.Water.hdc, 0, 0, vbSrcCopy
//                    Next A
//                    For A = 1 To .Location.Width / 32
//                        BitBlt myBackBuffer, .Location.X + vScreenX(Z) + 32 * A - 32, .Location.Y + vScreenY(Z), 32, 2, GFX.Water.hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, .Location.X + vScreenX(Z) + 32 * A - 32, .Location.Y + vScreenY(Z) + .Location.Height - 2, 32, 2, GFX.Water.hdc, 0, 0, vbSrcCopy
//                    Next A
//                    BitBlt myBackBuffer, .X - vScreen(Z).Left, .Y - vScreen(Z).Top, 32, 32, GFX.ECursorMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, .X - vScreen(Z).Left, .Y - vScreen(Z).Top, 32, 32, GFX.ECursor(2).hdc, 0, 0, vbSrcPaint
                    frmMain.renderTexture(curX, curY, GFX.ECursor[2]);
//                    If LCase(frmLayers.lstLayer.List(frmLayers.lstLayer.ListIndex)) <> "default" Then
                    if(!e.Layer.empty() && SDL_strcasecmp(e.Layer.c_str(), "Default") != 0)
//                        SuperPrint UCase(frmLayers.lstLayer.List(frmLayers.lstLayer.ListIndex)), 3, .X + 28, .Y + 34
                        SuperPrint(e.Layer, 3, X + 28, Y + 34);
//                    End If
                }
//                Else
                else
                {
//                    If .Mode = 5 Then
                    if(EditorCursor.Mode == OptCursor_t::LVL_WARPS)
                    {
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, 2, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .Location.Height - 2, .Location.Width, 2, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, 2, .Location.Height, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + .Location.Width - 2, vScreenY(Z) + .Location.Y, 2, .Location.Height, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                    End If
                    }
//                    BitBlt myBackBuffer, .X - vScreen(Z).Left, .Y - vScreen(Z).Top, 32, 32, GFX.ECursorMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, .X - vScreen(Z).Left, .Y - vScreen(Z).Top, 32, 32, GFX.ECursor(2).hdc, 0, 0, vbSrcPaint
                    frmMain.renderTexture(curX, curY, GFX.ECursor[2]);
//                    If LCase(frmLayers.lstLayer.List(frmLayers.lstLayer.ListIndex)) <> "default" Then
                    if(!e.Layer.empty() && SDL_strcasecmp(e.Layer.c_str(), "Default") != 0)
//                        SuperPrint UCase(frmLayers.lstLayer.List(frmLayers.lstLayer.ListIndex)), 3, .X + 28, .Y + 34
                        SuperPrint(EditorCursor.Layer, 3, curX + 28 , curY + 34);
//                    End If
//                End If
                }

//            End With
            }
//        End If
        }


        if(numScreens > 1) // for multiple screens
            frmMain.resetViewport();

        if(GameOutro)
        {
            frmMain.renderRect(0, 0, 800, Maths::iRound(CreditChop), 0.f, 0.f, 0.f);
            frmMain.renderRect(0, 600 - Maths::iRound(CreditChop), 800, 600, 0.f, 0.f, 0.f);
            DoCredits();
        }

//        If LevelEditor = True Then
//            StretchBlt frmLevelWindow.vScreen(Z).hdc, 0, 0, frmLevelWindow.vScreen(Z).ScaleWidth, frmLevelWindow.vScreen(Z).ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy
//        Else
        { // NOT AN EDITOR!!!
            if(ScreenShake > 0)
            {
                ScreenShake--;
                if(ScreenShake == 0)
                {
                    frmMain.offsetViewport(0, 0);
                }
                else
                {
                    A = (iRand() % ScreenShake * 4) - ScreenShake * 2;
                    B = (iRand() % ScreenShake * 4) - ScreenShake * 2;
                    frmMain.offsetViewport(A, B);
                }
            }
        }
//    Next Z
    }

    if(!skipRepaint)
        frmMain.repaint();

//    If TakeScreen = True Then ScreenShot
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
}
