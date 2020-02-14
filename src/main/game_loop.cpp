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

#include "../pseudo_vb.h"

void CheckActive();//in game_main.cpp

void GameLoop()
{

    UpdateControls();
    if(LevelMacro > 0)
        UpdateMacro();
    if(BattleMode)
    {
        if(BattleOutro > 0)
        {
            BattleOutro++;
            if(BattleOutro == 260)
                EndLevel = true;
        }
    }


    if(EndLevel)
    {
        if(LevelBeatCode > 0)
        {
            if(Checkpoint == FullFileName)
            {
                Checkpoint = "";
            }
        }
        NextLevel();
        UpdateControls();
    }
    else if(qScreen)
    {
        UpdateEffects();
        UpdateGraphics();
    }
    else if(BattleIntro > 0)
    {
        UpdateGraphics();
        BlockFrames();
        UpdateSound();
        For(A, 1, numNPCs)
        {
            NPCFrames(A);
        }
        BattleIntro--;
        if(BattleIntro == 1)
            PlaySound(58);
    }
    else
    {
        UpdateLayers(); // layers before/after npcs
        UpdateNPCs();

        if(LevelMacro == 3)
            return; // stop on key exit

        UpdateBlocks();
        UpdateEffects();
        UpdatePlayer();
        if(LivingPlayers() || BattleMode)
            UpdateGraphics();
        UpdateSound();
        UpdateEvents();
//        If MagicHand = True Then UpdateEditor

        if(Player[1].Controls.Start || (getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED))
        {
            if(LevelMacro == 0 && CheckLiving() > 0)
            {
                if(Player[1].UnStart)
                {
                    if((CaptainN || FreezeNPCs) && PSwitchStop == 0)
                    {
                        if(getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED)
                        {
                            FreezeNPCs = false;
                            PauseGame(1);
                        }
                        else
                        {
                            Player[1].UnStart = false;
                            if(FreezeNPCs)
                            {
                                FreezeNPCs = false;
                                if(PSwitchTime > 0)
                                {
                                    if(!noSound)
                                        SoundResumeAll();
                                }
                            }
                            else
                            {
                                FreezeNPCs = true;
                                if(PSwitchTime > 0)
                                {
                                    if(!noSound)
                                        SoundPauseAll();
                                }
                            }
                            PlaySound(30);
                        }
                    }
                    else
                    {
                        PauseGame(1);
                    }
                }
            }
        }
        else if(numPlayers == 2 && Player[2].Controls.Start)
        {
            if(LevelMacro == 0 && CheckLiving() > 0)
            {
                if(Player[2].UnStart)
                {
                    if(CaptainN || FreezeNPCs)
                    {
                        Player[2].UnStart = false;
                        if(FreezeNPCs)
                        {
                            FreezeNPCs = false;
                        }
                        else
                        {
                            FreezeNPCs = true;
                        }
                        PlaySound(30);
                    }
                }
            }
        }
    }
}

void PauseGame(int plr)
{
    bool stopPause = false;
    int A = 0;
    int B = 0;
    bool noButtons = false;
    double fpsTime = 0;
    int fpsCount = 0;

    for(A = numPlayers; A >= 1; A--)
        SavedChar[Player[A].Character] = Player[A];

    if(TestLevel == true && MessageText.empty())
        return;
    if(MessageText.empty())
        PlaySound(30);
    else
    {
        SoundPause[47] = 0;
        PlaySound(47);
    }

    GamePaused = true;
    MenuCursor = 0;
    MenuCursorCanMove = false;

    if(PSwitchTime > 0)
    {
        // If noSound = False Then mciSendString "pause smusic", 0, 0, 0
        if(noSound == false)
            SoundPauseAll();
    }

    overTime = 0;
    GoalTime = SDL_GetTicks() + 1000;
    fpsCount = 0;
    fpsTime = 0;
    cycleCount = 0;
    gameTime = 0;

    do
    {
        tempTime = SDL_GetTicks();
        if(tempTime >= gameTime + frameRate || tempTime < gameTime || MaxFPS == true)
        {
            if(fpsCount >= 32000) // Fixes Overflow bug
                fpsCount = 0;
            if(cycleCount >= 32000) // Fixes Overflow bug
                cycleCount = 0;
            overTime = overTime + (tempTime - (gameTime + frameRate));
            if(gameTime == 0.0)
                overTime = 0;
            if(overTime <= 1)
                overTime = 0;
            else if(overTime > 1000)
                overTime = 1000;
            gameTime = tempTime - overTime;
            overTime = (overTime - (tempTime - gameTime));
            if(SDL_GetTicks() > fpsTime)
            {
                if(cycleCount >= 65)
                {
                    overTime = 0;
                    gameTime = tempTime;
                }
                cycleCount = 0;
                fpsTime = SDL_GetTicks() + 1000;
                GoalTime = fpsTime;
//                if(Debugger == true)
//                    frmLevelDebugger.lblFPS = fpsCount;
                if(ShowFPS == true)
                    PrintFPS = fpsCount;
                fpsCount = 0;
            }


            DoEvents();
            CheckActive();

            if(LevelSelect == true)
                UpdateGraphics2();
            else
                UpdateGraphics();
            UpdateControls();
            UpdateSound();
            BlockFrames();
            UpdateEffects();

            if(SingleCoop > 0 || numPlayers > 2)
            {
                for(A = 1; A <= numPlayers; A++)
                    Player[A].Controls = Player[1].Controls;
            }

            if(MessageText.empty())
            {
                if(noButtons == false)
                {
                    if(!Player[plr].Controls.Down && !Player[plr].Controls.Up &&
                       !Player[plr].Controls.Run && !Player[plr].Controls.Jump &&
                       !Player[plr].Controls.Start)
                    {
                        if(!getKeyState(vbKeyEscape) && !getKeyState(vbKeySpace) &&
                           !getKeyState(vbKeyReturn) && !getKeyState(vbKeyDown) &&
                           !getKeyState(vbKeyUp))
                        {
                            noButtons = true;
                        }
                    }
                }
                else
                {
                    if(getKeyState(vbKeyEscape) == KEY_PRESSED)
                    {
                        if(LevelSelect && !Cheater)
                        {
                            if(MenuCursor != 2)
                                PlaySound(26);
                            MenuCursor = 2;
                        }
                        else
                        {
                            if(MenuCursor != 1)
                                PlaySound(26);
                            MenuCursor = 1;
                        }
                        noButtons = false;
                    }
                    else if(Player[plr].Controls.Start == true)
                        stopPause = true;

                    if(Player[plr].Controls.Up == true || getKeyState(vbKeyUp) == KEY_PRESSED)
                    {
                        PlaySound(26);
                        MenuCursor = MenuCursor - 1;
                        noButtons = false;
                    }
                    else if(Player[plr].Controls.Down == true || getKeyState(vbKeyDown) == KEY_PRESSED)
                    {
                        PlaySound(26);
                        MenuCursor = MenuCursor + 1;
                        noButtons = false;
                    }

                    if(LevelSelect)
                    {
                        if(Player[A].Character == 1 || Player[A].Character == 2)
                            Player[A].Hearts = 0;
                        for(A = 1; A <= numPlayers; A++)
                        {
                            if(!Player[A].RunRelease)
                            {
                                if(!Player[A].Controls.Left && !Player[A].Controls.Right)
                                    Player[A].RunRelease = true;
                            }
                            else if(Player[A].Controls.Left || Player[A].Controls.Right)
                            {
                                AllCharBlock = 0;
                                for(B = 1; B <= numCharacters; B++)
                                {
                                    if(blockCharacter[B] == false)
                                    {
                                        if(AllCharBlock == 0)
                                            AllCharBlock = B;
                                        else
                                        {
                                            AllCharBlock = 0;
                                            break;
                                        }
                                    }
                                }
                                if(AllCharBlock == 0)
                                {
                                    PlaySound(26);
                                    Player[A].RunRelease = false;
                                    if(A == 1)
                                        B = 2;
                                    else
                                        B = 1;
                                    if(numPlayers == 1)
                                        B = 0;
                                    Player[0].Character = 0;
                                    if(Player[A].Controls.Left == true)
                                    {
                                        do
                                        {
                                            Player[A].Character = Player[A].Character - 1;
                                            if(Player[A].Character <= 0)
                                                Player[A].Character = 5;
                                        } while(Player[A].Character == Player[B].Character || blockCharacter[Player[A].Character]);
                                    }
                                    else
                                    {
                                        do
                                        {
                                            Player[A].Character = Player[A].Character + 1;
                                            if(Player[A].Character >= 6)
                                                Player[A].Character = 1;
                                        } while(Player[A].Character == Player[B].Character || blockCharacter[Player[A].Character]);
                                    }
                                    Player[A] = SavedChar[Player[A].Character];
                                    SetupPlayers();
                                }
                            }
                        }
                    }

                    if(Player[plr].Controls.Jump || getKeyState(vbKeySpace) || getKeyState(vbKeyReturn))
                    {
                        if(MenuCursor == 0)
                            stopPause = true;
                        else if(MenuCursor == 1 && (LevelSelect == true ||
                                (StartLevel == FileName && NoMap == true)) && !Cheater)
                        {
                            SaveGame();
                            stopPause = true;
                        }
                        else
                        {
                            if(!Cheater && (LevelSelect ||
                              (StartLevel == FileName && NoMap)))
                            {
                                SaveGame();
                            }
                            stopPause = true;
                            GameMenu = true;
                            MenuMode = 0;
                            MenuCursor = 0;
                            if(LevelSelect == false)
                            {
                                LevelSelect = true;
                                EndLevel = true;
                            }
                            else
                                LevelSelect = false;
                            frmMain.clearBuffer();
                            StopMusic();
                            DoEvents();
                            SDL_Delay(500);
                        }
                    }
                    if(Cheater || !(LevelSelect || (StartLevel == FileName && NoMap)))
                    {
                        if(MenuCursor > 1)
                            MenuCursor = 0;
                        if(MenuCursor < 0)
                            MenuCursor = 1;
                    }
                    else
                    {
                        if(MenuCursor > 2)
                            MenuCursor = 0;
                        if(MenuCursor < 0)
                            MenuCursor = 2;
                    }
                }
            }
            else
            {
                if(noButtons == false)
                {
                    if(!Player[plr].Controls.Down && !Player[plr].Controls.Up &&
                       !Player[plr].Controls.Run && !Player[plr].Controls.Jump &&
                       !Player[plr].Controls.Start)
                    {
                        if(!getKeyState(vbKeyEscape) && !getKeyState(vbKeySpace) &&
                           !getKeyState(vbKeyReturn) && !getKeyState(vbKeyDown) &&
                            !getKeyState(vbKeyUp))
                        {
                            noButtons = true;
                        }
                    }
                }
                else
                {
                    if(getKeyState(vbKeyEscape) || Player[plr].Controls.Jump ||
                       Player[plr].Controls.Run == true || Player[plr].Controls.Start == true ||
                       getKeyState(vbKeySpace) || getKeyState(vbKeyReturn))
                    {
                        stopPause = true;
                    }
                }
            }
        }
        if(qScreen == true)
            stopPause = false;
        SDL_Delay(1);
    } while(!(stopPause == true));
    GamePaused = false;
    Player[plr].UnStart = false;
    Player[plr].CanJump = false;
    if(MessageText.empty())
        PlaySound(30);
    if(PSwitchTime > 0)
    {
        // If noSound = False Then mciSendString "resume smusic", 0, 0, 0
        if(noSound == false)
            SoundResumeAll();
    }
    MessageText.clear();

    overTime = 0;
    GoalTime = SDL_GetTicks() + 1000;
    fpsCount = 0;
    cycleCount = 0;
    gameTime = 0;
    fpsTime = 0;
}

