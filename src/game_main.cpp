#include <ctime>

#include <AppPath/app_path.h>

#include "globals.h"
#include "game_main.h"

#include "blocks.h"
#include "change_res.h"
#include "collision.h"
#include "custom.h"
#include "effect.h"
#include "graphics.h"
#include "joystick.h"
#include "layers.h"
#include "load_gfx.h"
#include "npc.h"
#include "player.h"
#include "sorting.h"
#include "sound.h"

//Dim ScrollDelay As Integer
static int ScrollDelay = 0;

void CheckActive();
// set up sizable blocks
void SizableBlocks();

bool FileExists(const std::string &fileName)
{
    SDL_RWops*test = SDL_RWFromFile(fileName.c_str(), "rb");
    if(test)
    {
        SDL_FreeRW(test);
        return true;
    }
    return false;
}

// game_main_setupphysics.cpp

int GameMain(int argc, char**argv)
{
    Player blankPlayer;
    int A = 0;
    int B = 0;
    int C = 0;
    bool tempBool = false;
    LB = "\n";
    EoT = "";
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    FrameSkip = false;
//    frmLoader.Show 'show the Splash screen
//    Do
//        DoEvents
//    Loop While StartMenu = False 'wait until the player clicks a button
    StartMenu = true;
    FrameSkip = false;
    noSound = false;

    SDL_memset(&blankPlayer, 0, sizeof(Player));

//    Unload frmLoader


//    If LevelEditor = False Then
//        frmMain.Show
    frmMain.show();
//        GameMenu = True
    GameMenu = true;
//    Else
//        frmSplash.Show
//        BlocksSorted = True
//    End If
//    InitControls 'init player's controls
    InitControls(); // init player's controls
//    DoEvents
    DoEvents();
//    If noSound = False Then
    if(!noSound)
    {
//        InitMixerX
        InitMixerX();
//        ' mciSendString "open " & Chr(34) & App.path & "\sound\do.mp3" & Chr(34) & " alias sound29", 0, 0, 0 'play the nintendo sound
//        ' mciSendString "play sound29 from 10", 0, 0, 0
//        PlayInitSound
        PlayInitSound();
//    End If
    }
//    InitSound 'Setup sound effects
    InitSound();
//    LevelSelect = True 'world map is to be shown
    LevelSelect = true; // world map is to be shown
//    DoEvents
    DoEvents();
//    SetupPhysics 'Setup Physics
    SetupPhysics(); // Setup Physics
//    SetupGraphics 'setup graphics
    SetupGraphics(); // setup graphics
//    Load GFX 'load the graphics form
    gfx.load(); // load the graphics form
//    SizableBlocks
    SizableBlocks();
//    LoadGFX 'load the graphics from file
    LoadGFX(); // load the graphics from file
//    SetupVars 'Setup Variables
    SetupVars(); //Setup Variables
//    frmMain.AutoRedraw = False
//    frmMain.Picture = GFX.Picture
//    frmMain.LoadCoin.Visible = False
//    frmMain.Loader.Visible = False

    GameIsActive = true;
//    Do
    do
    {
//        If GameMenu = True Then
        if(GameMenu)
        {
//            frmMain.MousePointer = 99
            frmMain.MousePointer = 99;
        }
//        ElseIf resChanged = False And TestLevel = False And LevelEditor = False Then
//            frmMain.MousePointer = 0
//        End If

//        If LevelEditor = True Then 'Load the level editor

//        ElseIf GameOutro = True Then 'SMBX Credits
        if(GameOutro) // SMBX Credits
        {
//            ShadowMode = False
            ShadowMode = false;
//            GodMode = False
            GodMode = false;
//            GrabAll = False
            GrabAll= false;
//            CaptainN = False
            CaptainN = false;
//            FlameThrower = False
            FlameThrower = false;
//            FreezeNPCs = False
            FreezeNPCs = false;
//            WalkAnywhere = False
            WalkAnywhere = false;
//            MultiHop = False
            MultiHop = false;
//            SuperSpeed = False
            SuperSpeed = false;
//            FlyForever = False
            FlyForever = false;
//            For A = 1 To maxPlayers
            for(int A = 1; A <= maxPlayers; A++)
            {
//                Player(A) = blankPlayer
                player[A] = blankPlayer;
//            Next A
            }
//            numPlayers = 5
            numPlayers = 5;
//            GameMenu = False
            GameMenu = false;
//            StopMusic
            StopMusic();
//            OpenLevel App.Path & "\outro.lvl"
            OpenLevel(AppPath + "outro.lvl");
//            ScreenType = 7
            ScreenType = 7;
//            SetupScreens
            SetupScreens();
//            ClearBuffer = True
            ClearBuffer = true;
//            For A = 1 To numPlayers
            for(int A = 1; A <= numPlayers; ++A)
            {
//                With Player(A)
                Player &p = player[A];
//                    If A = 1 Then
                if(A == 1)
//                        .State = 4
                    p.State = 4;
//                    ElseIf A = 2 Then
                else if(A == 2)
//                        .State = 7
                    p.State = 7;
//                    ElseIf A = 3 Then
                else if(A == 3)
//                        .State = 5
                    p.State = 5;
//                    ElseIf A = 4 Then
                else if(A == 4)
//                        .State = 3
                    p.State = 3;
//                    Else
                else
//                        .State = 6
                    p.State = 6;
//                    End If

//                    If A = 4 Then
                if(A == 4)
                {
//                        .Mount = 1
                    p.Mount = 1;
//                        .MountType = Int(Rnd * 3) + 1
                    p.MountType = int(std::rand() % 3) + 1;
//                    End If
                }
//                    .Character = A
                p.Character = A;
//                    If A = 2 Then
                if(A == 2)
                {
//                        .Mount = 3
                    p.Mount = 3;
//                        .MountType = Int(Rnd * 8) + 1
                    p.MountType = int(std::rand() % 8) + 1;
//                    End If
                }
//                    .HeldBonus = 0
                p.HeldBonus = 0;
//                    .Section = 0
                p.Section = 0;
//                    .Location.Height = Physics.PlayerHeight(.Character, .State)
                p.location.Height = physics.PlayerHeight[p.Character][p.State];
//                    .Location.Width = Physics.PlayerWidth(.Character, .State)
                p.location.Width = physics.PlayerWidth[p.Character][p.State];
//                End With
//            Next A
            }
//            SetupPlayers
            SetupPlayers();
//            CreditChop = 300 '100
            CreditChop = 300; // 100
//            EndCredits = 0
            EndCredits = 0;
//            SetupCredits
            SetupCredits();
//            overTime = 0
            overTime = 0;
//            GoalTime = GetTickCount + 1000
            GoalTime = SDL_GetTicks() + 1000;
//            fpsCount = 0
            fpsCount = 0;
//            fpsTime = 0
            fpsTime = 0;
//            cycleCount = 0
            cycleCount = 0;
//            gameTime = 0
            gameTime = 0;
//            Do
            do
            {
//                DoEvents
                DoEvents();
//                tempTime = GetTickCount
                tempTime = SDL_GetTicks();
//                ScreenType = 0
                ScreenType = 0;
//                SetupScreens
                SetupScreens();
//                If tempTime >= gameTime + frameRate Or tempTime < gameTime Then
                if(tempTime >= gameTime + frameRate || tempTime < gameTime)
                {
//                    CheckActive
                    CheckActive();
//                    OutroLoop
                    OutroLoop();
//                    If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
                    if(fpsCount >= 32000) fpsCount = 0; // Fixes Overflow bug
//                    If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
                    if(cycleCount >= 32000) cycleCount = 0; // Fixes Overflow bug
//                    overTime = overTime + (tempTime - (gameTime + frameRate))
                    overTime = overTime + (tempTime - (gameTime + frameRate));
//                    If gameTime = 0 Then overTime = 0
                    if(gameTime == 0.0) overTime = 0;
//                    If overTime <= 1 Then
                    if(overTime <= 1)
                    {
//                        overTime = 0
                        overTime = 0;
                    }
//                    ElseIf overTime > 1000 Then
                    else if(overTime > 1000)
                    {
//                        overTime = 1000
                        overTime = 1000;
//                    End If
                    }
//                    gameTime = tempTime - overTime
                    gameTime = tempTime - overTime;
//                    overTime = (overTime - (tempTime - gameTime))
                    overTime = (overTime - (tempTime - gameTime));
//                    DoEvents
                    frmMain.repaint();
                    DoEvents();
//                    If GetTickCount > fpsTime Then
                    if(SDL_GetTicks() > fpsTime)
                    {
//                        If cycleCount >= 65 Then
                        if(cycleCount >= 65)
                        {
//                            overTime = 0
                            overTime = 0;
//                            gameTime = tempTime
                            gameTime = 0;
//                        End If
                        }
//                        cycleCount = 0
                        cycleCount = 0;
//                        fpsTime = GetTickCount + 1000
                        fpsTime = SDL_GetTicks() + 1000;
//                        GoalTime = fpsTime
                        GoalTime = fpsTime;
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                        If ShowFPS = True Then
                        if(ShowFPS)
                        {
//                            PrintFPS = fpsCount
                            PrintFPS = fpsCount;
//                        End If
                        }
//                        fpsCount = 0
                        fpsCount = 0;
//                    End If
                    }
//                End If
                }

                if(!GameIsActive) break;// Break on quit
//            Loop While GameOutro = True
            } while(GameOutro);
        }
//        ElseIf GameMenu = True Then 'The Game Menu
        else if(GameMenu) // The Game Menu
        {
//            BattleIntro = 0
            BattleIntro = 0;
//            BattleOutro = 0
            BattleOutro = 0;
//            AllCharBlock = 0
            AllCharBlock = 0;
//            Cheater = False
            Cheater = false;
//            For A = 1 To maxPlayers
            for(int A = 1; A <= maxPlayers; ++A)
            {
//                OwedMount(A) = 0
                OwedMount[A] = 0;
//                OwedMountType(A) = 0
                OwedMountType[A] = 0;
//            Next A
            }
//            MenuMouseMove = False
            MenuMouseMove = false;
//            MenuMouseRelease = False
            MenuMouseRelease = false;
//            MenuMouseClick = False
            MenuMouseClick = false;
//            MenuMouseBack = False
            MenuMouseBack = false;
//            BattleMode = False
            BattleMode = false;
//            If MenuMode <> 4 Then
            if(MenuMode != 4)
            {
//                PlayerCharacter = 0
                PlayerCharacter = 0;
//                PlayerCharacter2 = 0
                PlayerCharacter2 = 0;
//            End If
            }
//            Checkpoint = ""
            Checkpoint = "";
//            WorldPlayer(1).Frame = 0
            worldPlayer[1].Frame = 0;
//            CheatString = ""
            CheatString = "";
//            LevelBeatCode = 0
            LevelBeatCode = 0;
//            curWorldLevel = 0
            curWorldLevel = 0;
//            ClearWorld
            ClearWorld();
//            ReturnWarp = 0
            ReturnWarp = 0;
//            ShadowMode = False
            ShadowMode = false;
//            GodMode = False
            GodMode = false;
//            GrabAll = False
            GrabAll = false;
//            CaptainN = False
            CaptainN = false;
//            FlameThrower = False
            FlameThrower = false;
//            FreezeNPCs = False
            FreezeNPCs = false;
//            WalkAnywhere = False
            WalkAnywhere = false;
//            MultiHop = False
            MultiHop = false;
//            SuperSpeed = False
            SuperSpeed = false;
//            FlyForever = False
            FlyForever = false;
//            BeatTheGame = False
            BeatTheGame = false;
//            ScreenType = 2
            ScreenType = 2;
//            SetupScreens
            SetupScreens();
//            BattleOutro = 0
            BattleOutro = 0;
//            BattleIntro = 0
            BattleIntro = 0;
//            For A = 1 To maxPlayers
            for(int A = 1; A <= maxPlayers; ++A)
            {
//                Player(A) = blankPlayer
                player[A] = blankPlayer;
//            Next A
            }
//            numPlayers = 6
            numPlayers = 6;
//            OpenLevel App.Path & "\intro.lvl"
            OpenLevel(AppPath + "intro.lvl");
//            vScreenX(1) = -level(0).X
            vScreenX[1] = -level[0].X;
//            StartMusic 0
            StartMusic(0);
//            SetupPlayers
            SetupPlayers();
//            For A = 1 To numPlayers
            For(A, 1, numPlayers)
            {
//                With Player(A)
                Player &p = player[A];
//                    .State = Int(Rnd * 6) + 2
                p.State = int(std::rand() % 6) + 2;
//                    .Character = Int(Rnd * 5) + 1
                p.Character = int(std::rand() % 5) + 1;
//                    If A >= 1 And A <= 5 Then .Character = A
                if(A >= 1 && A <= 5)
                    p.Character = A;
//                    .HeldBonus = 0
                p.HeldBonus = 0;
//                    .Section = 0
                p.Section = 0;
//                    .Location.Height = Physics.PlayerHeight(.Character, .State)
                p.location.Height = physics.PlayerHeight[p.Character][p.State];
//                    .Location.Width = Physics.PlayerWidth(.Character, .State)
                p.location.Width = physics.PlayerWidth[p.Character][p.State];
//                    .Location.X = level(.Section).X + ((128 + Rnd * 64) * A)
                p.location.X = level[p.Section].X + ((128 + std::rand() % 64) * A);
//                    .Location.Y = level(.Section).Height - .Location.Height - 65
                p.location.Y = level[p.Section].Height - p.location.Height - 65;
//                    Do
                do
                {
//                        tempBool = True
                    tempBool = true;
//                        For B = 1 To numBlock
                    for(int B = 1; B <= numBlock; ++B)
                    {
//                            If CheckCollision(.Location, Block(B).Location) = True Then
                        if(CheckCollision(p.location, block[B].location))
                        {
//                                .Location.Y = Block(B).Location.Y - .Location.Height - 0.1
                            p.location.Y = block[B].location.Y - p.location.Height - 0.1;
//                                tempBool = False
                            tempBool = false;
//                            End If
                        }
//                        Next B
                    }
//                    Loop While tempBool = False
                } while(!tempBool);
//                    .Dead = True
                p.Dead = true;
//                End With
//            Next A
            }
//            ProcEvent "Level - Start", True
            ProcEvent("Level - Start", true);
//            For A = 2 To 100
            For(A, 2, 100)
            {
//                If Events(A).AutoStart = True Then ProcEvent Events(A).Name, True
                if(events[A].AutoStart) ProcEvent(events[A].Name, true);
//            Next A
            }
//            overTime = 0
            overTime = 0;
//            GoalTime = GetTickCount + 1000
            GoalTime = SDL_GetTicks() + 1000;
//            fpsCount = 0
            fpsCount = 0;
//            fpsTime = 0
            fpsTime = 0;
//            cycleCount = 0
            cycleCount = 0;
//            gameTime = 0
            gameTime = 0;
//            Do
            do
            {
//                DoEvents
                DoEvents();
//                tempTime = GetTickCount
                tempTime = SDL_GetTicks();
//                If tempTime >= gameTime + frameRate Or tempTime < gameTime Then
                if(tempTime >= gameTime + frameRate || tempTime < gameTime)
                {
//                    CheckActive
                    CheckActive();
//                    MenuLoop    'Run the menu loop
                    MenuLoop();   // Run the menu loop
//                    If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
                    if(fpsCount >= 32000) fpsCount = 0; // Fixes Overflow bug
//                    If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
                    if(cycleCount >= 32000) cycleCount = 0; // Fixes Overflow bug
//                    overTime = overTime + (tempTime - (gameTime + frameRate))
                    overTime = overTime + (tempTime - (gameTime + frameRate));
//                    If gameTime = 0 Then overTime = 0
                    if(gameTime == 0.0) overTime = 0;
//                    If overTime <= 1 Then
                    if(overTime <= 1)
                    {
//                        overTime = 0
                        overTime = 0;
                    }
//                    ElseIf overTime > 1000 Then
                    else if(overTime > 1000)
                    {
//                        overTime = 1000
                        overTime = 1000;
//                    End If
                    }
//                    gameTime = tempTime - overTime
                    gameTime = tempTime - overTime;
//                    overTime = (overTime - (tempTime - gameTime))
                    overTime = (overTime - (tempTime - gameTime));
//                    DoEvents
                    frmMain.repaint();
                    DoEvents();
//                    If GetTickCount > fpsTime Then
                    if(SDL_GetTicks() > fpsTime)
                    {
//                        If cycleCount >= 65 Then
                        if(cycleCount >= 65)
                        {
//                            overTime = 0
                            overTime = 0;
//                            gameTime = tempTime
                            gameTime = 0;
//                        End If
                        }
//                        cycleCount = 0
                        cycleCount = 0;
//                        fpsTime = GetTickCount + 1000
                        fpsTime = SDL_GetTicks() + 1000;
//                        GoalTime = fpsTime
                        GoalTime = fpsTime;
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                        If ShowFPS = True Then
                        if(ShowFPS)
                        {
//                            PrintFPS = fpsCount
                            PrintFPS = fpsCount;
//                        End If
                        }
//                        fpsCount = 0
                        fpsCount = 0;
//                    End If
                    }
//                End If
                }

                if(!GameIsActive) break;// Break on quit
//            Loop While GameMenu = True
            } while(GameMenu);
        }
//        ElseIf LevelSelect = True Then 'World Map
        else if(LevelSelect)
        {
//            CheatString = ""
            CheatString = "";
//            For A = 1 To numPlayers
            For(A, 1, numPlayers)
            {
//                If Player(A).Mount = 0 Or Player(A).Mount = 2 Then
                if(player[A].Mount == 0 || player[A].Mount == 2)
                {
//                    If OwedMount(A) > 0 Then
                    if(OwedMount[A] > 0)
                    {
//                        Player(A).Mount = OwedMount(A)
                        player[A].Mount = OwedMount[A];
//                        If OwedMountType(A) > 0 Then
                        if(OwedMountType[A] > 0)
//                            Player(A).MountType = OwedMountType(A)
                            player[A].MountType = OwedMountType[A];
//                        Else
                        else
//                            Player(A).MountType = 1
                            player[A].MountType = 1;
//                        End If
//                    End If
                    }
//                End If
                }
//                OwedMount(A) = 0
                OwedMount[A] = 0;
//                OwedMountType(A) = 0
                OwedMountType[A] = 0;
//            Next A
            }
//            LoadCustomGFX
            LoadCustomGFX();
//            SetupPlayers
            SetupPlayers();
//            If (StartLevel <> "" And NoMap = True) Or GoToLevel <> "" Then
            if((StartLevel != "" && NoMap) || GoToLevel != "")
            {
//                If NoMap = True Then SaveGame
                if(NoMap) SaveGame();
//                Player(1).Vine = 0
                player[1].Vine = 0;
//                Player(2).Vine = 0
                player[2].Vine = 0;
//                PlaySound 28
                PlaySound(28);
//                SoundPause(26) = 200
                SoundPause[26] = 2000;
//                LevelSelect = False
                LevelSelect = false;

//                GameThing
                GameThing();
//                ClearLevel
                ClearLevel();

//                Sleep 1000
                SDL_Delay(1000);
//                If GoToLevel = "" Then
                if(GoToLevel == "")
//                    OpenLevel SelectWorld(selWorld).WorldPath & StartLevel
                    OpenLevel(selectWorld[selWorld].WorldPath + StartLevel);
//                Else
                else
                {
//                    OpenLevel SelectWorld(selWorld).WorldPath & GoToLevel
                    OpenLevel(selectWorld[selWorld].WorldPath + GoToLevel);
//                    GoToLevel = ""
                    GoToLevel = "";
//                End If
                }
//            Else
            }
            else
            {
//                If curWorldMusic > 0 Then StartMusic curWorldMusic
                if(curWorldMusic > 0)
                    StartMusic(curWorldMusic);
//                overTime = 0
                overTime = 0;
//                GoalTime = GetTickCount + 1000
                GoalTime = SDL_GetTicks() + 1000;
//                fpsCount = 0
                fpsCount = 0;
//                fpsTime = 0
                fpsTime = 0;
//                cycleCount = 0
                cycleCount = 0;
//                gameTime = 0
                gameTime = 0;
//                Do 'level select loop
                do // 'level select loop
                {
//                    FreezeNPCs = False
                    FreezeNPCs = false;
//                    DoEvents
                    DoEvents();
//                    tempTime = GetTickCount
                    tempTime = SDL_GetTicks();
//                    If tempTime >= gameTime + frameRate Or tempTime < gameTime Or MaxFPS = True Then
                    if(tempTime >= gameTime + frameRate || tempTime < gameTime || MaxFPS)
                    {
//                        If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
                        if(fpsCount >= 32000) fpsCount = 0; // Fixes Overflow bug
//                        If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
                        if(cycleCount >= 32000) cycleCount = 0; // Fixes Overflow bug
//                        overTime = overTime + (tempTime - (gameTime + frameRate))
                        overTime = overTime + (tempTime - (gameTime + frameRate));
//                        If gameTime = 0 Then overTime = 0
                        if(gameTime == 0.0) overTime = 0;
//                        If overTime <= 1 Then
                        if(overTime <= 1)
                        {
//                            overTime = 0
                            overTime = 0;
                        }
//                        ElseIf overTime > 1000 Then
                        else if(overTime > 1000)
                        {
//                            overTime = 1000
                            overTime = 1000;
//                        End If
                        }
//                        gameTime = tempTime - overTime
                        gameTime = tempTime - overTime;
//                        overTime = (overTime - (tempTime - gameTime))
                        overTime = (overTime - (tempTime - gameTime));

//                        CheckActive
                        CheckActive();
//                        WorldLoop
                        WorldLoop();
//                        DoEvents
                        frmMain.repaint();
                        DoEvents();
//                        If GetTickCount > fpsTime Then
                        if(SDL_GetTicks() > fpsTime)
                        {
//                            If cycleCount >= 65 Then
                            if(cycleCount >= 65)
                            {
    //                            overTime = 0
                                overTime = 0;
    //                            gameTime = tempTime
                                gameTime = 0;
    //                        End If
                            }
//                            cycleCount = 0
                            cycleCount = 0;
//                            fpsTime = GetTickCount + 1000
                            fpsTime = SDL_GetTicks() + 1000;
//                            GoalTime = fpsTime
                            GoalTime = fpsTime;
//                            If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                            If ShowFPS = True Then
                            if(ShowFPS)
                            {
    //                            PrintFPS = fpsCount
                                PrintFPS = fpsCount;
    //                        End If
                            }
    //                        fpsCount = 0
                            fpsCount = 0;
//                        End If
                        }
//                    End If
                    }

                    if(!GameIsActive) break;// Break on quit
//                Loop While LevelSelect = True
                } while(LevelSelect);
//            End If
            }
//        Else 'MAIN GAME
        }
        else
        {

//            CheatString = "" 'clear the cheat codes
            CheatString = "";
//            EndLevel = False
            EndLevel = false;
//            If numPlayers = 1 Then
            if(numPlayers == 1)
//                ScreenType = 0 'Follow 1 player
                ScreenType = 0; // Follow 1 player
//            ElseIf numPlayers = 2 Then
            else if(numPlayers == 2)
//                ScreenType = 5 'Dynamic screen
                ScreenType = 5; // Dynamic screen
//            Else
            else
            {
//                'ScreenType = 3 'Average, no one leaves the screen
//                ScreenType = 2 'Average
                ScreenType = 2; // Average
//            End If
            }
//            If SingleCoop > 0 Then ScreenType = 6
            if(SingleCoop > 0)
                ScreenType = 6;
//            If nPlay.Online = True Then ScreenType = 8 'Online
//            For A = 1 To numPlayers
            for(int A = 1; A <= numPlayers; ++A)
            {
//                If Player(A).Mount = 2 Then Player(A).Mount = 0 'take players off the clown car
                if(player[A].Mount == 2)
                    player[A].Mount = 0; // take players off the clown car
//            Next A
            }
//            SetupPlayers 'Setup Players for the level
            SetupPlayers(); // Setup Players for the level
//            qScreen = False
            qScreen = false;
//'for warp entrances
//            If (ReturnWarp > 0 And FileName = StartLevel) Or StartWarp > 0 Then
            if((ReturnWarp > 0 && FileName == StartLevel) || (StartWarp > 0))
            {
//                For A = 1 To numPlayers
                for(int A = 1; A <= numPlayers; ++A)
                {
//                    With Player(A)
                    Player &p = player[A];
//                        If StartWarp > 0 Then
                    if(StartWarp > 0)
//                            .Warp = StartWarp
                        p.Warp = StartWarp;
//                        Else
                    else
//                            .Warp = ReturnWarp
                        p.Warp = ReturnWarp;
//                        End If
//                        If Warp(.Warp).Effect = 1 Then
                    if(warp[p.Warp].Effect == 1)
                    {
//                            If Warp(.Warp).Direction2 = 1 Then
                        if(warp[p.Warp].Direction2 == 1)
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.location.X = warp[p.Warp].Exit.X + warp[p.Warp].Exit.Width / 2 - p.location.Width / 2;
//                                .Location.Y = Warp(.Warp).Exit.Y - .Location.Height - 8
                            p.location.Y = warp[p.Warp].Exit.Y + warp[p.Warp].Exit.Height - 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 3 Then
                        if(warp[p.Warp].Direction2 == 3)
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.location.X = warp[p.Warp].Exit.X + warp[p.Warp].Exit.Width / 2 - p.location.Width / 2;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height + 8
                            p.location.Y = warp[p.Warp].Exit.Y + warp[p.Warp].Exit.Height + 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 2 Then
                        if(warp[p.Warp].Direction2 == 2)
                        {
//                                If .Mount = 3 Then .Duck = True
                            if(p.Mount == 3) p.Duck = true;
//                                .Location.X = Warp(.Warp).Exit.X - .Location.Width - 8
                            p.location.X = warp[p.Warp].Exit.X + warp[p.Warp].Exit.Width - 8;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
                            p.location.Y = warp[p.Warp].Exit.Y + warp[p.Warp].Exit.Height / 2 - p.location.Height / 2;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 4 Then
                        if(warp[p.Warp].Direction2 == 4)
                        {
//                                If .Mount = 3 Then .Duck = True
                            if(p.Mount == 3) p.Duck = true;
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width + 8
                            p.location.X = warp[p.Warp].Exit.X + warp[p.Warp].Exit.Width + 8;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
                            p.location.Y = warp[p.Warp].Exit.Y + warp[p.Warp].Exit.Height / 2 - p.location.Height / 2;
//                            End If
                        }
//                            PlayerFrame A
                        PlayerFrame(A);
//                            CheckSection A
                        CheckSection(A);
//                            SoundPause(17) = 0
                        SoundPause[17] = 0;
//                            .Effect = 8
                        p.Effect = 8;
//                            .Effect2 = 950
                        p.Effect2 = 950;
                    }
//                        ElseIf Warp(.Warp).Effect = 2 Then
                    else if(warp[p.Warp].Effect == 2)
                    {
//                            .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                        p.location.X = warp[p.Warp].Exit.X + warp[p.Warp].Exit.Width / 2 - p.location.Width / 2;
//                            .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height
                        p.location.Y = warp[p.Warp].Exit.Y + warp[p.Warp].Exit.Height / 2 - p.location.Height / 2;
//                            CheckSection A
                        CheckSection(A);
//                            .Effect = 8
                        p.Effect = 8;
//                            .Effect2 = 2000
                        p.Effect2 = 2000;
//                        End If
                    }
//                    End With
//                Next A
                }
//                If StartWarp > 0 Then
                if(StartWarp > 0)
//                    StartWarp = 0
                    StartWarp = 0;
//                Else
                else
//                    ReturnWarp = 0
                    ReturnWarp = 0;
//                End If
//            End If
            }
//'--------------------------------------------
//            ProcEvent "Level - Start", True
            ProcEvent("Level - Start", true);
//            For A = 2 To 100
            for(int A = 2; A <= 100; ++A)
            {
//                If Events(A).AutoStart = True Then ProcEvent Events(A).Name, True
                if(events[A].AutoStart)
                    ProcEvent(events[A].Name, true);
//            Next A
            }
//            overTime = 0
            overTime = 0;
//            GoalTime = GetTickCount + 1000
            GoalTime = SDL_GetTicks() + 1000;
//            fpsCount = 0
            fpsCount = 0;
//            fpsTime = 0
            fpsTime = 0;
//            cycleCount = 0
            cycleCount = 0;
//            gameTime = 0
            gameTime = 0;
//            Do 'MAIN GAME LOOP
            do
            {
//                DoEvents
                DoEvents();
//                tempTime = GetTickCount
                tempTime = SDL_GetTicks();
//                If tempTime >= gameTime + frameRate Or tempTime < gameTime Or MaxFPS = True Then
                if(tempTime >= gameTime + frameRate || tempTime < gameTime || MaxFPS)
                {
//                    CheckActive
                    CheckActive();
//                    If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
                    if(fpsCount >= 32000) fpsCount = 0; // Fixes Overflow bug
//                    If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
                    if(cycleCount >= 32000) cycleCount = 0; // Fixes Overflow bug
//                    overTime = overTime + (tempTime - (gameTime + frameRate))
                    overTime = overTime + (tempTime - (gameTime + frameRate));
//                    If gameTime = 0 Then overTime = 0
                    if(gameTime == 0.0) overTime = 0;
//                    If overTime <= 1 Then
                    if(overTime <= 1)
//                        overTime = 0
                        overTime = 0;
//                    ElseIf overTime > 1000 Then
                    else if(overTime > 1000)
//                        overTime = 1000
                        overTime = 1000;
//                    End If
//                    gameTime = tempTime - overTime
                    gameTime = tempTime - overTime;
//                    overTime = (overTime - (tempTime - gameTime))
                    overTime = (overTime - (tempTime - gameTime));
//                    GameLoop    'Run the game loop
                    GameLoop();
//                    DoEvents
                    frmMain.repaint();
                    DoEvents();
//                    If GetTickCount > fpsTime Then
                    if(SDL_GetTicks() > fpsTime)
                    {
//                        If cycleCount >= 65 Then
                        if(cycleCount >= 65)
                        {
//                            overTime = 0
                            overTime = 0;
//                            gameTime = tempTime
                            gameTime = tempTime;
//                        End If
                        }
//                        cycleCount = 0
                        cycleCount = 0;
//                        fpsTime = GetTickCount + 1000
                        fpsTime = SDL_GetTicks() + 1000;
//                        GoalTime = fpsTime
                        GoalTime = fpsTime;
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                        If ShowFPS = True Then
                        if(ShowFPS)
                        {
//                            PrintFPS = fpsCount
                            PrintFPS = fpsCount;
//                        End If
                        }
//                        fpsCount = 0
                        fpsCount = 0;
//                    End If
                    }
//                    If LivingPlayers = False Then
                    if(!LivingPlayers())
                    {
//                        EveryonesDead
                        EveryonesDead();
//                    End If
                    }
//                End If
                }

                if(!GameIsActive) break;// Break on quit
//            Loop While LevelSelect = False And GameMenu = False
            } while(!LevelSelect || !GameMenu);
//            If TestLevel = True Then
//                TestLevel = False
//                LevelEditor = True
//                LevelEditor = true;
//                If nPlay.Online = False Then
//                    OpenLevel FullFileName
//                Else
//                    If nPlay.Mode = 1 Then
//                        Netplay.sendData "H0" & LB
//                        If Len(FullFileName) > 4 Then
//                            If LCase(Right(FullFileName, 4)) = ".lvl" Then
//                                OpenLevel FullFileName
//                            Else
//                                For A = 1 To 15
//                                    If nPlay.ClientCon(A) = True Then Netplay.InitSync A
//                                Next A
//                            End If
//                        Else
//                            For A = 1 To 15
//                                If nPlay.ClientCon(A) = True Then Netplay.InitSync A
//                            Next A
//                        End If
//                    End If
//                End If
//                LevelSelect = False
//            Else
//                ClearLevel
            ClearLevel();
//            End If
//        End If
        }
//    Loop
    } while(GameIsActive);

    return 0;
}

// game_main_setupvars.cpp

// game_loop.cpp

// menu_loop.cpp

void EditorLoop()
{
    // DUMMY
}

void KillIt()
{
    GameIsActive = false;
    frmMain.hide();
    if(resChanged)
        SetOrigRes();
    QuitMixerX();
//    DeleteDC myBackBuffer
//    DeleteObject myBufferBMP
//    UnloadGFX
    UnloadGFX();
    ShowCursor(1);
}

void OpenLevel(std::string FilePath)
{
    // USE PGE-FL here
}

void ClearLevel()
{

}

void NextLevel()
{

}

void UpdateMacro()
{

}

void OpenWorld(std::string FilePath)
{
    // USE PGE-FL here
}

void WorldLoop()
{

}

void LevelPath(int Lvl, int Direction, bool Skp)
{

}

void PathWait()
{

}

void ClearWorld()
{

}

void FindWorlds()
{

}

void FindLevels()
{

}

void FindSaves()
{

}

void SaveGame()
{

}

void LoadGame()
{

}

void PauseGame(int plr)
{

}

void InitControls()
{

}

void OpenConfig()
{

}

void SaveConfig()
{

}

void NPCyFix()
{

}

void CheckActive()
{

}

void CheatCode(std::string NewKey)
{

}

void OutroLoop()
{

}

void SetupCredits()
{

}

void FindStars()
{

}

void AddCredit(std::string newCredit)
{

}

void MoreScore(int addScore, Location Loc, int Multiplier)
{

}

void SetupPlayerFrames()
{

}

void SizableBlocks()
{

}

void StartBattleMode()
{

}

std::string FixComma(std::string newStr)
{

}

