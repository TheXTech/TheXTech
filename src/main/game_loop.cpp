#include "../globals.h"
#include "../game_main.h"
#include "../sound.h"
#include "../joystick.h"
#include "../effect.h"
#include "../graphics.h"
#include "../blocks.h"
#include "../npc.h"
#include "../layers.h"
#include "../player.h"

void GameLoop()
{
//    Dim A As Integer
    //int A;
//    Dim tempLocation As Location
    //Location tempLocation;
//    UpdateControls
    UpdateControls();
//    If LevelMacro > 0 Then UpdateMacro
    if(LevelMacro > 0)
        UpdateMacro();
//    If BattleMode = True Then
    if(BattleMode)
    {
//        If BattleOutro > 0 Then
        if(BattleOutro > 0)
        {
//            BattleOutro = BattleOutro + 1
            BattleOutro += 1;
//            If BattleOutro = 260 Then EndLevel = True
            if(BattleOutro == 260)
                EndLevel = true;
//        End If
        }
//    End If
    }


//    If EndLevel = True Then
    if(EndLevel)
    {
//        If LevelBeatCode > 0 Then
        if(LevelBeatCode > 0)
        {
//            If Checkpoint = FullFileName Then
            if(Checkpoint == FullFileName)
            {
//                Checkpoint = ""
                Checkpoint = "";
//            End If
            }
//        End If
        }
//        NextLevel
        NextLevel();
//        UpdateControls
        UpdateControls();
    }
//    ElseIf qScreen = True Then
    else if(qScreen)
    {
//        UpdateEffects
        UpdateEffects();
//        UpdateGraphics
        UpdateGraphics();
    }
//    ElseIf BattleIntro > 0 Then
    else if(BattleIntro > 0)
    {
//        UpdateGraphics
        UpdateGraphics();
//        BlockFrames
        BlockFrames();
//        UpdateSound
        UpdateSound();
//        For A = 1 To numNPCs
        For(A, 1, numNPCs)
        {
//            NPCFrames A
                NPCFrames(A);
//        Next A
        }
//        BattleIntro = BattleIntro - 1
        BattleIntro -= 1;
//        If BattleIntro = 1 Then PlaySound 58
        if(BattleIntro == 1)
            PlaySound(58);
    }
//    Else
    else
    {
//        UpdateLayers 'layers before/after npcs
        UpdateLayers(); // layers before/after npcs
//        UpdateNPCs
        UpdateNPCs();
//        If LevelMacro = 3 Then Exit Sub 'stop on key exit
        if(LevelMacro == 3)
            return; // stop on key exit
//        UpdateBlocks
        UpdateBlocks();
//        UpdateEffects
        UpdateEffects();
//        UpdatePlayer
        UpdatePlayer();
//        If LivingPlayers = True Or BattleMode = True Then UpdateGraphics
        if(LivingPlayers() || BattleMode)
            UpdateGraphics();
//        UpdateSound
        UpdateSound();
//        UpdateEvents
        UpdateEvents();
//        If MagicHand = True Then UpdateEditor

//        If Player(1).Controls.Start = True Or (GetKeyState(vbKeyEscape) And KEY_PRESSED) Then
        if(Player[1].Controls.Start || (getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED))
        {
//            If LevelMacro = 0 And CheckLiving > 0 Then
            if(LevelMacro == 0 && CheckLiving() > 0)
            {
//                If Player(1).UnStart = True Then
                if(Player[1].UnStart)
                {
//                    If (CaptainN = True Or FreezeNPCs = True) And PSwitchStop = 0 Then
                    if((CaptainN || FreezeNPCs) && PSwitchStop == 0)
                    {
//                        If (GetKeyState(vbKeyEscape) And KEY_PRESSED) Then
                        if(getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED)
                        {
//                            FreezeNPCs = False
                            FreezeNPCs = false;
//                            PauseGame 1
                            PauseGame(1);
//                        Else
                        }
                        else
                        {
//                            Player(1).UnStart = False
                            Player[1].UnStart = false;
//                            If FreezeNPCs = True Then
                            if(FreezeNPCs)
                            {
//                                FreezeNPCs = False
                                FreezeNPCs = false;
//                                If PSwitchTime > 0 Then
                                if(PSwitchTime > 0)
                                {
//                                    ' If noSound = False Then mciSendString "resume smusic", 0, 0, 0
//                                    If noSound = False Then SoundResumeAll
                                    if(!noSound)
                                        SoundResumeAll();
//                                End If
                                }
//                            Else
                            }
                            else
                            {
//                                FreezeNPCs = True
                                FreezeNPCs = true;
//                                If PSwitchTime > 0 Then
                                if(PSwitchTime > 0)
                                {
//                                    ' If noSound = False Then mciSendString "pause smusic", 0, 0, 0
//                                    If noSound = False Then SoundPauseAll
                                    if(!noSound)
                                        SoundPauseAll();
//                                End If
                                }
//                            End If
                            }
//                            PlaySound 30
                            PlaySound(30);
//                        End If
                        }
//                    Else
                    }
                    else
                    {
//                        PauseGame 1
                        PauseGame(1);
//                    End If
                    }
//                End If
                }
//            End If
            }
//        ElseIf numPlayers = 2 And Player(2).Controls.Start = True Then
        }
        else if(numPlayers == 2 && Player[2].Controls.Start)
        {
//            If LevelMacro = 0 And CheckLiving > 0 Then
            if(LevelMacro == 0 && CheckLiving() > 0)
            {
//                If Player(2).UnStart = True Then
                if(Player[2].UnStart)
                {
//                    If CaptainN = True Or FreezeNPCs = True Then
                    if(CaptainN || FreezeNPCs)
                    {
//                        Player(2).UnStart = False
                        Player[2].UnStart = false;
//                        If FreezeNPCs = True Then
                        if(FreezeNPCs)
                        {
//                            FreezeNPCs = False
                            FreezeNPCs = false;
//                        Else
                        }
                        else
                        {
//                            FreezeNPCs = True
                            FreezeNPCs = true;
//                        End If
                        }
//                        PlaySound 30
                        PlaySound(30);
//                    End If
                    }
//                End If
                }
//            End If
            }
//        End If
        }
//    End If
    }
}
