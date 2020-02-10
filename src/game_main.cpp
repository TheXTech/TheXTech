#include <ctime>

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

void SetupPhysics()
{
    physics.PlayerJumpVelocity = -5.7f;     // Jump velocity
    physics.PlayerJumpHeight = 20;          // Jump height
    physics.PlayerBlockJumpHeight = 25;     // Jump height off bouncy blocks
    physics.PlayerHeadJumpHeight = 22;      // Jump height off another players head
    physics.PlayerNPCJumpHeight = 22;       // Jump height off a NPC
    physics.PlayerSpringJumpHeight = 55;    // Jump height off a Spring
    physics.PlayerRunSpeed = 6;             // Max run speed
    physics.PlayerWalkSpeed = 3;            // Max walk speed
    physics.PlayerGravity = 0.4f;            // Player// s gravity
    physics.PlayerTerminalVelocity = 12;    // Max falling speed
    physics.PlayerHeight[1][1] = 30;        // Little Mario
    physics.PlayerWidth[1][1] = 24;         // ------------
    physics.PlayerGrabSpotX[1][1] = 18;     // ---------
    physics.PlayerGrabSpotY[1][1] = -2;     // ---------
    physics.PlayerHeight[1][2] = 54;        // Big Mario
    physics.PlayerWidth[1][2] = 24;         // ---------
    physics.PlayerDuckHeight[1][2] = 30;    // ---------
    physics.PlayerGrabSpotX[1][2] = 18;     // ---------
    physics.PlayerGrabSpotY[1][2] = 16;     // ---------
    physics.PlayerHeight[1][3] = 54;        // Fire Mario
    physics.PlayerWidth[1][3] = 24;         // ---------
    physics.PlayerDuckHeight[1][3] = 30;    // ---------
    physics.PlayerGrabSpotX[1][3] = 18;     // ---------
    physics.PlayerGrabSpotY[1][3] = 16;     // ---------
    physics.PlayerHeight[1][7] = 54;        // Ice Mario
    physics.PlayerWidth[1][7] = 24;         // ---------
    physics.PlayerDuckHeight[1][7] = 30;    // ---------
    physics.PlayerGrabSpotX[1][7] = 18;     // ---------
    physics.PlayerGrabSpotY[1][7] = 16;     // ---------
    physics.PlayerHeight[1][4] = 54;        // Racoon Mario
    physics.PlayerWidth[1][4] = 24;         // ---------
    physics.PlayerDuckHeight[1][4] = 30;    // ---------
    physics.PlayerGrabSpotX[1][4] = 18;     // ---------
    physics.PlayerGrabSpotY[1][4] = 16;     // ---------
    physics.PlayerHeight[1][5] = 54;        // Tanooki Mario
    physics.PlayerWidth[1][5] = 24;         // ---------
    physics.PlayerDuckHeight[1][5] = 30;    // ---------
    physics.PlayerGrabSpotX[1][5] = 18;     // ---------
    physics.PlayerGrabSpotY[1][5] = 16;     // ---------
    physics.PlayerHeight[1][6] = 54;        // Hammer Mario
    physics.PlayerWidth[1][6] = 24;         // ---------
    physics.PlayerDuckHeight[1][6] = 30;    // ---------
    physics.PlayerGrabSpotX[1][6] = 18;     // ---------
    physics.PlayerGrabSpotY[1][6] = 16;     // ---------

    physics.PlayerHeight[2][1] = 30;        // Little Luigi
    physics.PlayerWidth[2][1] = 24;         // ------------
    physics.PlayerGrabSpotX[2][1] = 16;     // ---------
    physics.PlayerGrabSpotY[2][1] = -4;     // ---------
    physics.PlayerHeight[2][2] = 60;        // Big Luigi
    physics.PlayerWidth[2][2] = 24;         // ---------
    physics.PlayerDuckHeight[2][2] = 30;    // ---------
    physics.PlayerGrabSpotX[2][2] = 18;     // ---------
    physics.PlayerGrabSpotY[2][2] = 16;     // ---------
    physics.PlayerHeight[2][3] = 60;        // Fire Luigi
    physics.PlayerWidth[2][3] = 24;         // ---------
    physics.PlayerDuckHeight[2][3] = 30;    // ---------
    physics.PlayerGrabSpotX[2][3] = 18;     // ---------
    physics.PlayerGrabSpotY[2][3] = 16;     // ---------
    physics.PlayerHeight[2][4] = 60;        // Racoon Luigi
    physics.PlayerWidth[2][4] = 24;         // ---------
    physics.PlayerDuckHeight[2][4] = 30;    // ---------
    physics.PlayerGrabSpotX[2][4] = 18;     // ---------
    physics.PlayerGrabSpotY[2][4] = 16;     // ---------
    physics.PlayerHeight[2][5] = 60;        // Tanooki Luigi
    physics.PlayerWidth[2][5] = 24;         // ---------
    physics.PlayerDuckHeight[2][5] = 30;    // ---------
    physics.PlayerGrabSpotX[2][5] = 18;     // ---------
    physics.PlayerGrabSpotY[2][5] = 16;     // ---------
    physics.PlayerHeight[2][6] = 60;        // Tanooki Luigi
    physics.PlayerWidth[2][6] = 24;         // ---------
    physics.PlayerDuckHeight[2][6] = 30;    // ---------
    physics.PlayerGrabSpotX[2][6] = 18;     // ---------
    physics.PlayerGrabSpotY[2][6] = 16;     // ---------
    physics.PlayerHeight[2][7] = 60;        // Ice Luigi
    physics.PlayerWidth[2][7] = 24;         // ---------
    physics.PlayerDuckHeight[2][7] = 30;    // ---------
    physics.PlayerGrabSpotX[2][7] = 18;     // ---------
    physics.PlayerGrabSpotY[2][7] = 16;     // ---------

    physics.PlayerHeight[3][1] = 38;        // Little Peach
    physics.PlayerDuckHeight[3][1] = 26;    // ---------
    physics.PlayerWidth[3][1] = 24;         // ------------
    physics.PlayerGrabSpotX[3][1] = 0;      // ---------
    physics.PlayerGrabSpotY[3][1] = 0;      // ---------
    physics.PlayerHeight[3][2] = 60;        // Big Peach
    physics.PlayerWidth[3][2] = 24;         // ---------
    physics.PlayerDuckHeight[3][2] = 30;    // ---------
    physics.PlayerGrabSpotX[3][2] = 0;     // ---------
    physics.PlayerGrabSpotY[3][2] = 0;     // ---------
    physics.PlayerHeight[3][3] = 60;        // Fire Peach
    physics.PlayerWidth[3][3] = 24;         // ---------
    physics.PlayerDuckHeight[3][3] = 30;    // ---------
    physics.PlayerGrabSpotX[3][3] = 18;
    physics.PlayerGrabSpotY[3][3] = 16;

    physics.PlayerHeight[3][4] = 60;        // Racoon Peach
    physics.PlayerWidth[3][4] = 24;         // ---------
    physics.PlayerDuckHeight[3][4] = 30;    // ---------
    physics.PlayerGrabSpotX[3][4] = 18;
    physics.PlayerGrabSpotY[3][4] = 16;

    physics.PlayerHeight[3][5] = 60;        // Tanooki Peach
    physics.PlayerWidth[3][5] = 24;         // ---------
    physics.PlayerDuckHeight[3][5] = 30;    // ---------
    physics.PlayerGrabSpotX[3][5] = 18;
    physics.PlayerGrabSpotY[3][5] = 16;

    physics.PlayerHeight[3][6] = 60;        // Hammer Peach
    physics.PlayerWidth[3][6] = 24;         // ---------
    physics.PlayerDuckHeight[3][6] = 30;    // ---------
    physics.PlayerGrabSpotX[3][6] = 18;
    physics.PlayerGrabSpotY[3][6] = 16;


    physics.PlayerHeight[3][7] = 60;        // Ice Peach
    physics.PlayerWidth[3][7] = 24;         // ---------
    physics.PlayerDuckHeight[3][7] = 30;    // ---------
    physics.PlayerGrabSpotX[3][7] = 18;
    physics.PlayerGrabSpotY[3][7] = 16;

    physics.PlayerHeight[4][1] = 30;        // Little Toad
    physics.PlayerWidth[4][1] = 24;         // ------------
    physics.PlayerDuckHeight[4][1] = 26;    // ---------
    physics.PlayerGrabSpotX[4][1] = 18;     // ---------
    physics.PlayerGrabSpotY[4][1] = -2;     // ---------
    physics.PlayerHeight[4][2] = 50;        // Big Toad
    physics.PlayerWidth[4][2] = 24;         // ---------
    physics.PlayerDuckHeight[4][2] = 30;    // ---------
    physics.PlayerGrabSpotX[4][2] = 18;     // ---------
    physics.PlayerGrabSpotY[4][2] = 16;     // ---------
    physics.PlayerHeight[4][3] = 50;        // Fire Toad
    physics.PlayerWidth[4][3] = 24;         // ---------
    physics.PlayerDuckHeight[4][3] = 30;    // ---------
    physics.PlayerGrabSpotX[4][3] = 18;     // ---------
    physics.PlayerGrabSpotY[4][3] = 16;     // ---------

    physics.PlayerHeight[4][4] = 50;        // Racoon Toad
    physics.PlayerWidth[4][4] = 24;         // ---------
    physics.PlayerDuckHeight[4][4] = 30;    // ---------
    physics.PlayerGrabSpotX[4][4] = 18;     // ---------
    physics.PlayerGrabSpotY[4][4] = 16;     // ---------

    physics.PlayerHeight[4][5] = 50;        // Tanooki Toad
    physics.PlayerWidth[4][5] = 24;         // ---------
    physics.PlayerDuckHeight[4][5] = 30;    // ---------
    physics.PlayerGrabSpotX[4][5] = 18;     // ---------
    physics.PlayerGrabSpotY[4][5] = 16;     // ---------

    physics.PlayerHeight[4][6] = 50;        // Hammer Toad
    physics.PlayerWidth[4][6] = 24;         // ---------
    physics.PlayerDuckHeight[4][6] = 30;    // ---------
    physics.PlayerGrabSpotX[4][6] = 18;     // ---------
    physics.PlayerGrabSpotY[4][6] = 16;     // ---------

    physics.PlayerHeight[4][7] = 50;        // Ice Toad
    physics.PlayerWidth[4][7] = 24;         // ---------
    physics.PlayerDuckHeight[4][7] = 30;    // ---------
    physics.PlayerGrabSpotX[4][7] = 18;     // ---------
    physics.PlayerGrabSpotY[4][7] = 16;     // ---------

    physics.PlayerHeight[5][1] = 54;        // Green Link
    physics.PlayerWidth[5][1] = 22;         // ---------
    physics.PlayerDuckHeight[5][1] = 44;    // ---------
    physics.PlayerGrabSpotX[5][1] = 18;     // ---------
    physics.PlayerGrabSpotY[5][1] = 16;     // ---------

    physics.PlayerHeight[5][2] = 54;        // Green Link
    physics.PlayerWidth[5][2] = 22;         // ---------
    physics.PlayerDuckHeight[5][2] = 44;    // ---------
    physics.PlayerGrabSpotX[5][2] = 18;     // ---------
    physics.PlayerGrabSpotY[5][2] = 16;     // ---------

    physics.PlayerHeight[5][3] = 54;        // Fire Link
    physics.PlayerWidth[5][3] = 22;         // ---------
    physics.PlayerDuckHeight[5][3] = 44;    // ---------
    physics.PlayerGrabSpotX[5][3] = 18;     // ---------
    physics.PlayerGrabSpotY[5][3] = 16;     // ---------

    physics.PlayerHeight[5][4] = 54;        // Blue Link
    physics.PlayerWidth[5][4] = 22;         // ---------
    physics.PlayerDuckHeight[5][4] = 44;    // ---------
    physics.PlayerGrabSpotX[5][4] = 18;     // ---------
    physics.PlayerGrabSpotY[5][4] = 16;     // ---------

    physics.PlayerHeight[5][5] = 54;        // IronKnuckle Link
    physics.PlayerWidth[5][5] = 22;         // ---------
    physics.PlayerDuckHeight[5][5] = 44;    // ---------
    physics.PlayerGrabSpotX[5][5] = 18;     // ---------
    physics.PlayerGrabSpotY[5][5] = 16;     // ---------

    physics.PlayerHeight[5][6] = 54;        // Shadow Link
    physics.PlayerWidth[5][6] = 22;         // ---------
    physics.PlayerDuckHeight[5][6] = 44;    // ---------
    physics.PlayerGrabSpotX[5][6] = 18;     // ---------
    physics.PlayerGrabSpotY[5][6] = 16;     // ---------

    physics.PlayerHeight[5][7] = 54;        // Ice Link
    physics.PlayerWidth[5][7] = 22;         // ---------
    physics.PlayerDuckHeight[5][7] = 44;    // ---------
    physics.PlayerGrabSpotX[5][7] = 18;     // ---------
    physics.PlayerGrabSpotY[5][7] = 16;     // ---------

    physics.NPCTimeOffScreen = 180;         // How long NPCs are active offscreen before being reset
    physics.NPCShellSpeed = 7.1f;            // Speed of kicked shells
    physics.NPCShellSpeedY = 11;            // Vertical Speed of kicked shells
    physics.NPCCanHurtWait = 30;            // How long to wait before NPCs can hurt players
    physics.NPCGravity = 0.26f;              // NPC Gravity
    physics.NPCGravityReal = 0.26f;              // NPC Gravity
    physics.NPCWalkingSpeed = 1.2f;          // NPC Walking Speed
    physics.NPCWalkingOnSpeed = 1;          // NPC that can be walked on walking speed
    physics.NPCMushroomSpeed = 1.8f;         // Mushroom X Speed
    physics.NPCPSwitch = 777;               // P Switch time
}

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
//            If resChanged = True Then
//                ChangeScreen
//            End If
//            BattleMode = False
//            SingleCoop = 0
//            numPlayers = 0
//            frmMain.Hide
//            frmLevelEditor.Show
//            DoEvents
//            SetupEditorGraphics 'Set up the editor graphics
//            MagicHand = False
//            frmLevelEditor.menuFile.Enabled = True
//            frmLevelEditor.MenuTest.Enabled = True
//            frmLevelEditor.mnuOnline.Enabled = True
//            frmLevelEditor.mnuMode.Enabled = True
//            frmLevelEditor.optCursor(5).Enabled = True
//            frmLevelEditor.optCursor(15).Enabled = True
//            frmLevelEditor.optCursor(2).Enabled = True
//            If nPlay.Online = True Then
//                If nPlay.Mode = 0 Then frmLevelEditor.MenuTest.Enabled = False
//            End If
//            For A = 0 To frmLevelSettings.optLevel.Count - 1
//                frmLevelSettings.optLevel(A).Enabled = True
//            Next A
//            For A = 0 To frmLevelSettings.optSection.Count - 1
//                frmLevelSettings.optSection(A).Enabled = True
//            Next A
//            With EditorCursor
//                .Location.Height = 32
//                .Location.Width = 32
//            End With
//            overTime = 0
//            GoalTime = GetTickCount + 1000
//            fpsCount = 0
//            fpsTime = 0
//            cycleCount = 0
//            gameTime = 0
//            Do 'LEVEL EDITOR LOOP
//                tempTime = GetTickCount
//                If tempTime >= gameTime + frameRate Or tempTime < gameTime Then
//                    CheckActive
//                    EditorLoop 'Do the editor loop
//                    If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
//                    If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
//                    overTime = overTime + (tempTime - (gameTime + frameRate))
//                    If gameTime = 0 Then overTime = 0
//                    If overTime <= 1 Then
//                        overTime = 0
//                    ElseIf overTime > 1000 Then
//                        overTime = 1000
//                    End If
//                    gameTime = tempTime - overTime
//                    overTime = (overTime - (tempTime - gameTime))
//                    DoEvents


//                    If GetTickCount > fpsTime Then
//                        If cycleCount >= 65 Then
//                            overTime = 0
//                            gameTime = tempTime
//                        End If
//                        cycleCount = 0
//                        fpsTime = GetTickCount + 1000
//                        GoalTime = fpsTime
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                        If ShowFPS = True Then
//                            PrintFPS = fpsCount
//                        End If
//                        fpsCount = 0
//                    End If
//                End If
//            Loop While LevelEditor = True



//        ElseIf GameOutro = True Then 'SMBX Credits
//            ShadowMode = False
//            GodMode = False
//            GrabAll = False
//            CaptainN = False
//            FlameThrower = False
//            FreezeNPCs = False
//            WalkAnywhere = False
//            MultiHop = False
//            SuperSpeed = False
//            FlyForever = False
//            For A = 1 To maxPlayers
//                Player(A) = blankPlayer
//            Next A
//            numPlayers = 5
//            GameMenu = False
//            StopMusic
//            OpenLevel App.Path & "\outro.lvl"
//            ScreenType = 7
//            SetupScreens
//            ClearBuffer = True
//            For A = 1 To numPlayers
//                With Player(A)
//                    If A = 1 Then
//                        .State = 4
//                    ElseIf A = 2 Then
//                        .State = 7
//                    ElseIf A = 3 Then
//                        .State = 5
//                    ElseIf A = 4 Then
//                        .State = 3
//                    Else
//                        .State = 6
//                    End If
//                    If A = 4 Then
//                        .Mount = 1
//                        .MountType = Int(Rnd * 3) + 1
//                    End If
//                    .Character = A
//                    If A = 2 Then
//                        .Mount = 3
//                        .MountType = Int(Rnd * 8) + 1
//                    End If
//                    .HeldBonus = 0
//                    .Section = 0
//                    .Location.Height = Physics.PlayerHeight(.Character, .State)
//                    .Location.Width = Physics.PlayerWidth(.Character, .State)
//                End With
//            Next A
//            SetupPlayers
//            CreditChop = 300 '100
//            EndCredits = 0
//            SetupCredits
//            overTime = 0
//            GoalTime = GetTickCount + 1000
//            fpsCount = 0
//            fpsTime = 0
//            cycleCount = 0
//            gameTime = 0
//            Do
//                DoEvents
//                tempTime = GetTickCount
//                ScreenType = 0
//                SetupScreens
//                If tempTime >= gameTime + frameRate Or tempTime < gameTime Then
//                    CheckActive
//                    OutroLoop
//                    If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
//                    If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
//                    overTime = overTime + (tempTime - (gameTime + frameRate))
//                    If gameTime = 0 Then overTime = 0
//                    If overTime <= 1 Then
//                        overTime = 0
//                    ElseIf overTime > 1000 Then
//                        overTime = 1000
//                    End If
//                    gameTime = tempTime - overTime
//                    overTime = (overTime - (tempTime - gameTime))
//                    DoEvents
//                    If GetTickCount > fpsTime Then
//                        If cycleCount >= 65 Then
//                            overTime = 0
//                            gameTime = tempTime
//                        End If
//                        cycleCount = 0
//                        fpsTime = GetTickCount + 1000
//                        GoalTime = fpsTime
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                        If ShowFPS = True Then
//                            PrintFPS = fpsCount
//                        End If
//                        fpsCount = 0
//                    End If
//                End If
//            Loop While GameOutro = True
//        ElseIf GameMenu = True Then 'The Game Menu
//            BattleIntro = 0
//            BattleOutro = 0
//            AllCharBlock = 0
//            Cheater = False
//            For A = 1 To maxPlayers
//                OwedMount(A) = 0
//                OwedMountType(A) = 0
//            Next A
//            MenuMouseMove = False
//            MenuMouseRelease = False
//            MenuMouseClick = False
//            MenuMouseBack = False
//            BattleMode = False
//            If MenuMode <> 4 Then
//                PlayerCharacter = 0
//                PlayerCharacter2 = 0
//            End If
//            Checkpoint = ""
//            WorldPlayer(1).Frame = 0
//            CheatString = ""
//            LevelBeatCode = 0
//            curWorldLevel = 0
//            ClearWorld
//            ReturnWarp = 0
//            ShadowMode = False
//            GodMode = False
//            GrabAll = False
//            CaptainN = False
//            FlameThrower = False
//            FreezeNPCs = False
//            WalkAnywhere = False
//            MultiHop = False
//            SuperSpeed = False
//            FlyForever = False
//            BeatTheGame = False
//            ScreenType = 2
//            SetupScreens
//            BattleOutro = 0
//            BattleIntro = 0
//            For A = 1 To maxPlayers
//                Player(A) = blankPlayer
//            Next A
//            numPlayers = 6
//            OpenLevel App.Path & "\intro.lvl"
//            vScreenX(1) = -level(0).X
//            StartMusic 0
//            SetupPlayers
//            For A = 1 To numPlayers
//                With Player(A)
//                    .State = Int(Rnd * 6) + 2
//                    .Character = Int(Rnd * 5) + 1
//                    If A >= 1 And A <= 5 Then .Character = A
//                    .HeldBonus = 0
//                    .Section = 0
//                    .Location.Height = Physics.PlayerHeight(.Character, .State)
//                    .Location.Width = Physics.PlayerWidth(.Character, .State)
//                    .Location.X = level(.Section).X + ((128 + Rnd * 64) * A)
//                    .Location.Y = level(.Section).Height - .Location.Height - 65
//                    Do
//                        tempBool = True
//                        For B = 1 To numBlock
//                            If CheckCollision(.Location, Block(B).Location) = True Then
//                                .Location.Y = Block(B).Location.Y - .Location.Height - 0.1
//                                tempBool = False
//                            End If
//                        Next B
//                    Loop While tempBool = False
//                    .Dead = True
//                End With
//            Next A
//            ProcEvent "Level - Start", True
//            For A = 2 To 100
//                If Events(A).AutoStart = True Then ProcEvent Events(A).Name, True
//            Next A
//            overTime = 0
//            GoalTime = GetTickCount + 1000
//            fpsCount = 0
//            fpsTime = 0
//            cycleCount = 0
//            gameTime = 0
//            Do
//                DoEvents
//                tempTime = GetTickCount
//                If tempTime >= gameTime + frameRate Or tempTime < gameTime Then
//                    CheckActive
//                    MenuLoop    'Run the menu loop
//                    If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
//                    If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
//                    overTime = overTime + (tempTime - (gameTime + frameRate))
//                    If gameTime = 0 Then overTime = 0
//                    If overTime <= 1 Then
//                        overTime = 0
//                    ElseIf overTime > 1000 Then
//                        overTime = 1000
//                    End If
//                    gameTime = tempTime - overTime
//                    overTime = (overTime - (tempTime - gameTime))
//                    DoEvents
//                    If GetTickCount > fpsTime Then
//                         If cycleCount >= 65 Then
//                            overTime = 0
//                            gameTime = tempTime
//                        End If
//                        cycleCount = 0
//                        fpsTime = GetTickCount + 1000
//                        GoalTime = fpsTime
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                        If ShowFPS = True Then
//                            PrintFPS = fpsCount
//                        End If
//                        fpsCount = 0
//                    End If
//                End If
//            Loop While GameMenu = True
//        ElseIf LevelSelect = True Then 'World Map
//            CheatString = ""
//            For A = 1 To numPlayers
//                If Player(A).Mount = 0 Or Player(A).Mount = 2 Then
//                    If OwedMount(A) > 0 Then
//                        Player(A).Mount = OwedMount(A)
//                        If OwedMountType(A) > 0 Then
//                            Player(A).MountType = OwedMountType(A)
//                        Else
//                            Player(A).MountType = 1
//                        End If
//                    End If
//                End If
//                OwedMount(A) = 0
//                OwedMountType(A) = 0
//            Next A
//            LoadCustomGFX
//            SetupPlayers
//            If (StartLevel <> "" And NoMap = True) Or GoToLevel <> "" Then
//                If NoMap = True Then SaveGame
//                Player(1).Vine = 0
//                Player(2).Vine = 0
//                PlaySound 28
//                SoundPause(26) = 200
//                LevelSelect = False

//                GameThing
//                ClearLevel

//                Sleep 1000
//                If GoToLevel = "" Then
//                    OpenLevel SelectWorld(selWorld).WorldPath & StartLevel
//                Else
//                    OpenLevel SelectWorld(selWorld).WorldPath & GoToLevel
//                    GoToLevel = ""
//                End If
//            Else
//                If curWorldMusic > 0 Then StartMusic curWorldMusic
//                overTime = 0
//                GoalTime = GetTickCount + 1000
//                fpsCount = 0
//                fpsTime = 0
//                cycleCount = 0
//                gameTime = 0
//                Do 'level select loop
//                    FreezeNPCs = False
//                    DoEvents
//                    tempTime = GetTickCount
//                    If tempTime >= gameTime + frameRate Or tempTime < gameTime Or MaxFPS = True Then

//                        If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
//                        If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
//                        overTime = overTime + (tempTime - (gameTime + frameRate))
//                        If gameTime = 0 Then overTime = 0
//                        If overTime <= 1 Then
//                            overTime = 0
//                        ElseIf overTime > 1000 Then
//                            overTime = 1000
//                        End If
//                        gameTime = tempTime - overTime
//                        overTime = (overTime - (tempTime - gameTime))

//                        CheckActive
//                        WorldLoop
//                        DoEvents
//                        If GetTickCount > fpsTime Then
//                            If cycleCount >= 65 Then
//                                overTime = 0
//                                gameTime = tempTime
//                            End If
//                            cycleCount = 0
//                            fpsTime = GetTickCount + 1000
//                            GoalTime = fpsTime
//                            If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                            If ShowFPS = True Then
//                                PrintFPS = fpsCount
//                            End If
//                            fpsCount = 0
//                        End If
//                    End If
//                Loop While LevelSelect = True
//            End If
//        Else 'MAIN GAME
//            CheatString = "" 'clear the cheat codes
//            EndLevel = False
//            If numPlayers = 1 Then
//                ScreenType = 0 'Follow 1 player
//            ElseIf numPlayers = 2 Then
//                ScreenType = 5 'Dynamic screen
//            Else
//                'ScreenType = 3 'Average, no one leaves the screen
//                ScreenType = 2 'Average
//            End If
//            If SingleCoop > 0 Then ScreenType = 6
//            If nPlay.Online = True Then ScreenType = 8 'Online
//            For A = 1 To numPlayers
//                If Player(A).Mount = 2 Then Player(A).Mount = 0 'take players off the clown car
//            Next A
//            SetupPlayers 'Setup Players for the level
//            qScreen = False
//'for warp entrances
//            If (ReturnWarp > 0 And FileName = StartLevel) Or StartWarp > 0 Then
//                For A = 1 To numPlayers
//                    With Player(A)
//                        If StartWarp > 0 Then
//                            .Warp = StartWarp
//                        Else
//                            .Warp = ReturnWarp
//                        End If
//                        If Warp(.Warp).Effect = 1 Then
//                            If Warp(.Warp).Direction2 = 1 Then
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
//                                .Location.Y = Warp(.Warp).Exit.Y - .Location.Height - 8
//                            ElseIf Warp(.Warp).Direction2 = 3 Then
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height + 8
//                            ElseIf Warp(.Warp).Direction2 = 2 Then
//                                If .Mount = 3 Then .Duck = True
//                                .Location.X = Warp(.Warp).Exit.X - .Location.Width - 8
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
//                            ElseIf Warp(.Warp).Direction2 = 4 Then
//                                If .Mount = 3 Then .Duck = True
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width + 8
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
//                            End If
//                            PlayerFrame A
//                            CheckSection A
//                            SoundPause(17) = 0
//                            .Effect = 8
//                            .Effect2 = 950
//                        ElseIf Warp(.Warp).Effect = 2 Then
//                            .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
//                            .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height
//                            CheckSection A
//                            .Effect = 8
//                            .Effect2 = 2000
//                        End If
//                    End With
//                Next A
//                If StartWarp > 0 Then
//                    StartWarp = 0
//                Else
//                    ReturnWarp = 0
//                End If
//            End If
//'--------------------------------------------
//            ProcEvent "Level - Start", True
//            For A = 2 To 100
//                If Events(A).AutoStart = True Then ProcEvent Events(A).Name, True
//            Next A
//            overTime = 0
//            GoalTime = GetTickCount + 1000
//            fpsCount = 0
//            fpsTime = 0
//            cycleCount = 0
//            gameTime = 0
//            Do 'MAIN GAME LOOP
//                DoEvents
//                tempTime = GetTickCount
//                If tempTime >= gameTime + frameRate Or tempTime < gameTime Or MaxFPS = True Then
//                    CheckActive
//                    If fpsCount >= 32000 Then fpsCount = 0 'Fixes Overflow bug
//                    If cycleCount >= 32000 Then cycleCount = 0 'Fixes Overflow bug
//                    overTime = overTime + (tempTime - (gameTime + frameRate))
//                    If gameTime = 0 Then overTime = 0
//                    If overTime <= 1 Then
//                        overTime = 0
//                    ElseIf overTime > 1000 Then
//                        overTime = 1000
//                    End If
//                    gameTime = tempTime - overTime
//                    overTime = (overTime - (tempTime - gameTime))
//                    GameLoop    'Run the game loop
//                    DoEvents
//                    If GetTickCount > fpsTime Then
//                        If cycleCount >= 65 Then
//                            overTime = 0
//                            gameTime = tempTime
//                        End If
//                        cycleCount = 0
//                        fpsTime = GetTickCount + 1000
//                        GoalTime = fpsTime
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount
//                        If ShowFPS = True Then
//                            PrintFPS = fpsCount
//                        End If
//                        fpsCount = 0
//                    End If
//                    If LivingPlayers = False Then
//                        EveryonesDead
//                    End If
//                End If
//            Loop While LevelSelect = False And GameMenu = False
//            If TestLevel = True Then
//                TestLevel = False
//                LevelEditor = True
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
//            End If
//        End If
//    Loop
    } while(GameIsActive);

    return 0;
}

void SetupVars()
{

}

void GameLoop()
{

}

void MenuLoop()
{

}

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
