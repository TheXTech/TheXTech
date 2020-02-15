#include <ctime>

#include <AppPath/app_path.h>
#include <Logger/logger.h>

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

#include "pseudo_vb.h"

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
    Player_t blankPlayer;
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

    initAll();

//    Unload frmLoader

    GFX.load(); // Load UI graphics

//    If LevelEditor = False Then
//        frmMain.Show
    frmMain.show();
//        GameMenu = True
    GameMenu = true;
//    Else
//        frmSplash.Show
//        BlocksSorted = True
//    End If

    InitControls(); // init player's controls
    DoEvents();

    if(!noSound)
    {
        InitMixerX();
        PlayInitSound();
    }

    InitSound(); // Setup sound effects
    LevelSelect = true; // world map is to be shown
//    DoEvents
    DoEvents();
//    SetupPhysics 'Setup Physics
    SetupPhysics(); // Setup Physics
//    SetupGraphics 'setup graphics
    SetupGraphics(); // setup graphics
//    Load GFX 'load the graphics form
//    GFX.load(); // load the graphics form // Moved to before sound load
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
                Player[A] = blankPlayer;
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
                Player_t &p = Player[A];
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
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
//                    .Location.Width = Physics.PlayerWidth(.Character, .State)
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
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

                SDL_Delay(1);
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
            WorldPlayer[1].Frame = 0;
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
                Player[A] = blankPlayer;
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
                Player_t &p = Player[A];
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
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
//                    .Location.Width = Physics.PlayerWidth(.Character, .State)
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
//                    .Location.X = level(.Section).X + ((128 + Rnd * 64) * A)
                p.Location.X = level[p.Section].X + ((128 + std::rand() % 64) * A);
//                    .Location.Y = level(.Section).Height - .Location.Height - 65
                p.Location.Y = level[p.Section].Height - p.Location.Height - 65;
//                    Do
                do
                {
//                        tempBool = True
                    tempBool = true;
//                        For B = 1 To numBlock
                    for(int B = 1; B <= numBlock; ++B)
                    {
//                            If CheckCollision(.Location, Block(B).Location) = True Then
                        if(CheckCollision(p.Location, Block[B].Location))
                        {
//                                .Location.Y = Block(B).Location.Y - .Location.Height - 0.1
                            p.Location.Y = Block[B].Location.Y - p.Location.Height - 0.1;
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
                if(Events[A].AutoStart) ProcEvent(Events[A].Name, true);
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

                SDL_Delay(1);
                if(!GameIsActive) return 0;// Break on quit
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
                if(Player[A].Mount == 0 || Player[A].Mount == 2)
                {
//                    If OwedMount(A) > 0 Then
                    if(OwedMount[A] > 0)
                    {
//                        Player(A).Mount = OwedMount(A)
                        Player[A].Mount = OwedMount[A];
//                        If OwedMountType(A) > 0 Then
                        if(OwedMountType[A] > 0)
//                            Player(A).MountType = OwedMountType(A)
                            Player[A].MountType = OwedMountType[A];
//                        Else
                        else
//                            Player(A).MountType = 1
                            Player[A].MountType = 1;
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
                Player[1].Vine = 0;
//                Player(2).Vine = 0
                Player[2].Vine = 0;
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
                    OpenLevel(SelectWorld[selWorld].WorldPath + StartLevel);
//                Else
                else
                {
//                    OpenLevel SelectWorld(selWorld).WorldPath & GoToLevel
                    OpenLevel(SelectWorld[selWorld].WorldPath + GoToLevel);
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

                    SDL_Delay(1);
                    if(!GameIsActive) return 0;// Break on quit
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
                if(Player[A].Mount == 2)
                    Player[A].Mount = 0; // take players off the clown car
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
                    Player_t &p = Player[A];
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
                    if(Warp[p.Warp].Effect == 1)
                    {
//                            If Warp(.Warp).Direction2 = 1 Then
                        if(Warp[p.Warp].Direction2 == 1)
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.Location.X = Warp[p.Warp].Exit.X + Warp[p.Warp].Exit.Width / 2 - p.Location.Width / 2;
//                                .Location.Y = Warp(.Warp).Exit.Y - .Location.Height - 8
                            p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height - 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 3 Then
                        if(Warp[p.Warp].Direction2 == 3)
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.Location.X = Warp[p.Warp].Exit.X + Warp[p.Warp].Exit.Width / 2 - p.Location.Width / 2;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height + 8
                            p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height + 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 2 Then
                        if(Warp[p.Warp].Direction2 == 2)
                        {
//                                If .Mount = 3 Then .Duck = True
                            if(p.Mount == 3) p.Duck = true;
//                                .Location.X = Warp(.Warp).Exit.X - .Location.Width - 8
                            p.Location.X = Warp[p.Warp].Exit.X + Warp[p.Warp].Exit.Width - 8;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
                            p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height / 2 - p.Location.Height / 2;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 4 Then
                        if(Warp[p.Warp].Direction2 == 4)
                        {
//                                If .Mount = 3 Then .Duck = True
                            if(p.Mount == 3) p.Duck = true;
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width + 8
                            p.Location.X = Warp[p.Warp].Exit.X + Warp[p.Warp].Exit.Width + 8;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
                            p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height / 2 - p.Location.Height / 2;
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
                    else if(Warp[p.Warp].Effect == 2)
                    {
//                            .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                        p.Location.X = Warp[p.Warp].Exit.X + Warp[p.Warp].Exit.Width / 2 - p.Location.Width / 2;
//                            .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height
                        p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height / 2 - p.Location.Height / 2;
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
                if(Events[A].AutoStart)
                    ProcEvent(Events[A].Name, true);
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

                SDL_Delay(1);
                if(!GameIsActive) return 0;// Break on quit
//            Loop While LevelSelect = False And GameMenu = False
            } while(!LevelSelect && !GameMenu);
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
    UnloadGFX();
    ShowCursor(1);
}


void NextLevel()
{
    int A = 0;
    for(A = 1; A <= numPlayers; A++)
        Player[A].HoldingNPC = 0;
    LevelMacro = 0;
    LevelMacroCounter = 0;
    StopMusic();
    ClearLevel();
    frmMain.clearBuffer();
    frmMain.repaint();
    DoEvents();
    if(!TestLevel && GoToLevel == "" && !NoMap)
        SDL_Delay(500);
    if(BattleMode && !LevelEditor)
    {
        EndLevel = false;
        GameMenu = true;
        MenuMode = 4;
        MenuCursor = selWorld - 1;
        PlayerCharacter = Player[1].Character;
        PlayerCharacter2 = Player[2].Character;
    }
    else
    {
        LevelSelect = true;
        EndLevel = false;
    }
}

// macros mainly used for end of level stuffs. takes over the players controls
void UpdateMacro()
{
    int A = 0;
    bool OnScreen = false;
    if(LevelMacro == 1) // SMB3 Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            if(Player[A].Location.X < level[Player[A].Section].Width && Player[A].Dead == false)
            {
                OnScreen = true;
                Player[A].Controls.Down = false;
                Player[A].Controls.Drop = false;
                Player[A].Controls.Jump = false;
                Player[A].Controls.Left = false;
                Player[A].Controls.Right = true;
                Player[A].Controls.Run = false;
                Player[A].Controls.Up = false;
                Player[A].Controls.Start = false;
                Player[A].Controls.AltJump = false;
                Player[A].Controls.AltRun = false;
                if(Player[A].Wet > 0 && Player[A].CanJump == true)
                {
                    if(Player[A].Location.SpeedY > 1)
                        Player[A].Controls.Jump = true;
                }
            }
            else
            {
                Player[A].Location.SpeedY = -Physics.PlayerGravity;
                Player[A].Controls.Down = false;
                Player[A].Controls.Drop = false;
                Player[A].Controls.Jump = false;
                Player[A].Controls.Left = false;
                Player[A].Controls.Right = true;
                Player[A].Controls.Run = false;
                Player[A].Controls.Up = false;
                Player[A].Controls.Start = false;
                Player[A].Controls.AltJump = false;
                Player[A].Controls.AltRun = false;
            }
        }
        if(OnScreen == false)
        {
            LevelMacroCounter = LevelMacroCounter + 1;
            if(LevelMacroCounter >= 100)
            {
                LevelBeatCode = 1;
                LevelMacro = 0;
                LevelMacroCounter = 0;
                EndLevel = true;
            }
        }
    }
    else if(LevelMacro == 2)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Controls.Down = false;
            Player[A].Controls.Drop = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            Player[A].Controls.Run = false;
            Player[A].Controls.Up = false;
            Player[A].Controls.Start = false;
            Player[A].Controls.AltJump = false;
            Player[A].Controls.AltRun = false;
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter >= 460)
        {
            LevelBeatCode = 2;
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            frmMain.clearBuffer();
        }
    }
    else if(LevelMacro == 3)
    {
        float tempTime = 0;
        float gameTime = 0;

        do
        {
            // tempTime = Timer - Int(Timer)
            tempTime = std::floor(float(SDL_GetTicks()) / 1000.0f);
            if(tempTime > (float)(gameTime + 0.01) || tempTime < gameTime)
            {
                gameTime = tempTime;
                DoEvents();
                UpdateGraphics();
                UpdateSound();
                BlockFrames();
                LevelMacroCounter = LevelMacroCounter + 1;
                if(LevelMacroCounter >= 300)
                    break;
            }

            if(!GameIsActive)
                return;

        } while(true);

        LevelBeatCode = 4;
        EndLevel = true;
        LevelMacro = 0;
        LevelMacroCounter = 0;
        frmMain.clearBuffer();
    }
    else if(LevelMacro == 4)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Controls.Down = false;
            Player[A].Controls.Drop = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            Player[A].Controls.Run = false;
            Player[A].Controls.Up = false;
            Player[A].Controls.Start = false;
            Player[A].Controls.AltJump = false;
            Player[A].Controls.AltRun = false;
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter >= 300)
        {
            LevelBeatCode = 5;
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            frmMain.clearBuffer();
        }
    }
    else if(LevelMacro == 5)
    {
        // numNPCs = 0
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Controls.Down = false;
            Player[A].Controls.Drop = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            Player[A].Controls.Run = false;
            Player[A].Controls.Up = false;
            Player[A].Controls.Start = false;
            Player[A].Controls.AltJump = false;
            Player[A].Controls.AltRun = false;
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter == 250)
            PlaySound(45);
        if(LevelMacroCounter >= 800)
        {
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            if(TestLevel == false)
            {
                GameOutro = true;
                BeatTheGame = true;
                SaveGame();
                MenuMode = 0;
                MenuCursor = 0;
            }
            frmMain.clearBuffer();
        }
    }
    else if(LevelMacro == 6) // Star Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Controls.Down = false;
            Player[A].Controls.Drop = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            Player[A].Controls.Run = false;
            Player[A].Controls.Up = false;
            Player[A].Controls.Start = false;
            Player[A].Controls.AltJump = false;
            Player[A].Controls.AltRun = false;
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter >= 300)
        {
            LevelBeatCode = 7;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
    else if(LevelMacro == 7) // SMW Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            if(Player[A].Location.X < level[Player[A].Section].Width && Player[A].Dead == false)
            {
                Player[A].Controls.Down = false;
                Player[A].Controls.Drop = false;
                Player[A].Controls.Jump = false;
                Player[A].Controls.Left = false;
                Player[A].Controls.Right = true;
                Player[A].Controls.Run = false;
                Player[A].Controls.Up = false;
                Player[A].Controls.Start = false;
                Player[A].Controls.AltJump = false;
                Player[A].Controls.AltRun = false;
            }
            else
            {
                Player[A].Location.SpeedY = -Physics.PlayerGravity;
                Player[A].Controls.Down = false;
                Player[A].Controls.Drop = false;
                Player[A].Controls.Jump = false;
                Player[A].Controls.Left = false;
                Player[A].Controls.Right = true;
                Player[A].Controls.Run = false;
                Player[A].Controls.Up = false;
                Player[A].Controls.Start = false;
                Player[A].Controls.AltJump = false;
                Player[A].Controls.AltRun = false;
            }
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter >= 630)
        {
            LevelBeatCode = 8;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
}

void InitControls()
{
    int A = 0;
    int B = 0;
    bool newJoystick = false;

    while(true)
    {
        newJoystick = StartJoystick(A);
        if(newJoystick) {
            A += 1;
        } else {
            break;
        }
    }
    numJoysticks = A;

    /* // Crazy Redigit's solution, useless
//    If numJoysticks = 0 Then
    if(numJoysticks == 0) {
//        useJoystick(1) = 0
        useJoystick[1] = 0;
//        useJoystick(2) = 0
        useJoystick[2] = 0;
//    ElseIf numJoysticks = 1 Then
    } else if(numJoysticks == 1) {
//        useJoystick(1) = 1
        useJoystick[1] = 1;
//        useJoystick(2) = 0
        useJoystick[2] = 0;
//    Else
    } else {
//        useJoystick(1) = 1
        useJoystick[1] = 1;
//        useJoystick(2) = 2
        useJoystick[2] = 2;
//    End If
    }
    */

//    '
    useJoystick[1] = 0;
    useJoystick[2] = 0;
//    '

    For(A, 1, 2)
    {
        {
            auto &j = conJoystick[A];
            j.Run = 2;
            j.AltRun = 3;
            j.Jump = 0;
            j.AltJump = 1;
            j.Drop = 6;
            j.Start = 7;
        }
    }

    conKeyboard[1].Down = vbKeyDown;
    conKeyboard[1].Left = vbKeyLeft;
    conKeyboard[1].Up = vbKeyUp;
    conKeyboard[1].Right = vbKeyRight;
    conKeyboard[1].Jump = vbKeyZ;
    conKeyboard[1].Run = vbKeyX;
    conKeyboard[1].Drop = vbKeyShift;
    conKeyboard[1].Start = vbKeyEscape;
    conKeyboard[1].AltJump = vbKeyA;
    conKeyboard[1].AltRun = vbKeyS;

    conKeyboard[2].Down = vbKeyDown;
    conKeyboard[2].Left = vbKeyLeft;
    conKeyboard[2].Up = vbKeyUp;
    conKeyboard[2].Right = vbKeyRight;
    conKeyboard[2].Jump = vbKeyZ;
    conKeyboard[2].Run = vbKeyX;
    conKeyboard[2].Drop = vbKeyShift;
    conKeyboard[2].Start = vbKeyEscape;
    conKeyboard[2].AltJump = vbKeyA;
    conKeyboard[2].AltRun = vbKeyS;

    OpenConfig();

    if(useJoystick[1] > numJoysticks)
        useJoystick[1] = 0;
    if(useJoystick[2] > numJoysticks)
        useJoystick[2] = 0;
}


// main_config.cpp


void NPCyFix()
{
    int A = 0;
    float XnH = 0;
    float XnHfix = 0;
    for(A = 1; A <= numNPCs; A++)
    {
        XnH = NPC[A].Location.Y + NPC[A].Location.Height;
        if((int(XnH * 100) % 800) / 100 != 0)
        {
            if((int(XnH + std::abs((int(XnH * 100) % 800) / 100)) * 100) % 800 == 0)
                XnHfix = std::abs((int(XnH * 100) % 800) / 100);
            else
                XnHfix = std::abs(8 - ((int(XnH * 100) % 800) / 100));
            NPC[A].Location.Y = NPC[A].Location.Y + XnHfix;
        }
    }
}

void CheckActive()
{
    bool MusicPaused = false;
//    If nPlay.Online = True Then Exit Sub
    // If LevelEditor = False And TestLevel = False Then Exit Sub
    // If LevelEditor = False Then Exit Sub
    while(!frmMain.isWindowActive())
    {
        frmMain.waitEvents();
//        If LevelEditor = True Or MagicHand = True Then frmLevelWindow.vScreen(1).MousePointer = 0
        overTime = 0;
        GoalTime = SDL_GetTicks() + 1000;
        fpsCount = 0;
        fpsTime = 0;
        cycleCount = 0;
        gameTime = 0;
        tempTime = 0;
        keyDownEnter = false;
        keyDownAlt = false;
        if(musicPlaying == true && !MusicPaused)
        {
            pLogDebug("Window Focus lost");
            MusicPaused = true;
            // If noSound = False Then mciSendString "pause all", 0, 0, 0
            if(noSound == false)
                SoundPauseAll();
        }

        if(!GameIsActive)
            break;
    }

    if(MusicPaused)
        pLogDebug("Window Focus got back");

    if(noSound == false)
    {
        if(MusicPaused == true)
        {
            if(GameOutro == true)
            {
                // mciSendString "resume tmusic", 0, 0, 0
                SoundResumeAll();
            }
            else if(LevelSelect == true && GameMenu == false && LevelEditor == false)
            {
                // mciSendString "resume wmusic" & curWorldMusic, 0, 0, 0
                SoundResumeAll();
            }
            else if(curMusic > 0)
            {
                // mciSendString "resume music" & curMusic, 0, 0, 0
                SoundResumeAll();
            }
            else if(curMusic < 0)
            {
                if(PSwitchStop > 0)
                {
                    // mciSendString "resume stmusic", 0, 0, 0
                    SoundResumeAll();
                }
                else
                {
                    // mciSendString "resume smusic", 0, 0, 0
                    SoundResumeAll();
                }
            }
        }
    }
//    If LevelEditor = True Or MagicHand = True Then frmLevelWindow.vScreen(1).MousePointer = 99
}




void AddCredit(std::string newCredit)
{
    numCredits += 1;
    Credit[numCredits].Text = newCredit;
}

Location_t newLoc(double X, double Y, double Width, double Height)
{
    Location_t ret;
    ret.X = X;
    ret.Y = Y;
    ret.Width = Width;
    ret.Height = Height;
    return ret;
}

void MoreScore(int addScore, Location_t Loc, int Multiplier)
{
    //int oldM = 0;
    int A = 0;
    if(GameMenu || GameOutro || BattleMode)
        return;
    A = addScore + Multiplier;
    if(A == 0)
        return;
    Multiplier++;
    if(A > 13)
        A = 13;
    if(A < addScore)
        A = addScore;
    if(Multiplier > 9)
        Multiplier = 8;
    if(A > 13)
        A = 13;
    if(Points[A] <= 5)
    {
        Lives = Lives + Points[A];
        PlaySound(15);
    }
    else
        Score = Score + Points[A];
    NewEffect(79, Loc);
    Effect[numEffects].Frame = A - 1;
}

void SizableBlocks()
{
    BlockIsSizable[568] = true;
    BlockIsSizable[579] = true;
    BlockIsSizable[575] = true;
    BlockIsSizable[25] = true;
    BlockIsSizable[26] = true;
    BlockIsSizable[27] = true;
    BlockIsSizable[28] = true;
    BlockIsSizable[38] = true;
    BlockIsSizable[79] = true;
    BlockIsSizable[108] = true;
    BlockIsSizable[130] = true;
    BlockIsSizable[161] = true;
    BlockIsSizable[240] = true;
    BlockIsSizable[241] = true;
    BlockIsSizable[242] = true;
    BlockIsSizable[243] = true;
    BlockIsSizable[244] = true;
    BlockIsSizable[245] = true;
    BlockIsSizable[259] = true;
    BlockIsSizable[260] = true;
    BlockIsSizable[261] = true;
    BlockIsSizable[287] = true;
    BlockIsSizable[288] = true;
    BlockIsSizable[437] = true;
    BlockIsSizable[441] = true;
    BlockIsSizable[442] = true;
    BlockIsSizable[443] = true;
    BlockIsSizable[444] = true;
    BlockIsSizable[438] = true;
    BlockIsSizable[439] = true;
    BlockIsSizable[440] = true;
    BlockIsSizable[445] = true;
}

void StartBattleMode()
{
    int A = 0;
    Player_t blankPlayer;
    numPlayers = 2;
    for(A = 1; A <= numCharacters; A++)
    {
        SavedChar[A] = blankPlayer;
        SavedChar[A].Character = A;
        SavedChar[A].State = 1;
    }
    Player[1].State = 2;
    Player[1].Mount = 0;
    Player[1].Character = 1;
    Player[1].HeldBonus = 0;
    Player[1].CanFly = false;
    Player[1].CanFly2 = false;
    Player[1].TailCount = 0;
    Player[1].YoshiBlue = false;
    Player[1].YoshiRed = false;
    Player[1].YoshiYellow = false;
    Player[1].Hearts = 2;
    Player[2].State = 2;
    Player[2].Mount = 0;
    Player[2].Character = 2;
    Player[2].HeldBonus = 0;
    Player[2].CanFly = false;
    Player[2].CanFly2 = false;
    Player[2].TailCount = 0;
    Player[2].YoshiBlue = false;
    Player[2].YoshiRed = false;
    Player[2].YoshiYellow = false;
    Player[2].Hearts = 2;
    Player[1].Character = PlayerCharacter;
    Player[2].Character = PlayerCharacter2;
    numStars = 0;
    Coins = 0;
    Score = 0;
    Lives = 99;
    BattleLives[1] = 3;
    BattleLives[2] = 3;
    LevelSelect = false;
    GameMenu = false;
    BattleMode = true;
    frmMain.clearBuffer();
    frmMain.repaint();
    StopMusic();
    DoEvents();
    SDL_Delay(500);
    ClearLevel();
    if(selWorld == 1)
        selWorld = (std::rand() % (NumSelectWorld - 1)) + 2;

    OpenLevel(SelectWorld[selWorld].WorldFile);
    SetupPlayers();
    BattleIntro = 150;
    BattleWinner = 0;
    BattleOutro = 0;
}

std::string FixComma(std::string newStr)
{
    return newStr; // TODO IT
}
