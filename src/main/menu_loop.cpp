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

void MenuLoop()
{
//    On Error Resume Next
//    Dim A As Integer
    int A;
//    Dim B As Integer
    int B;
//    Dim tempLocation As Location
    Location tempLocation;
//    Dim newJoystick As Boolean
    bool newJoystick;
//    Dim tempBool As Boolean
    bool tempBool;
//    Dim menuLen As Integer
    int menuLen;
//    Dim blankPlayer As Player
    Player blankPlayer;
//    UpdateControls
    UpdateControls();
//    SingleCoop = 0
    SingleCoop = 0;

//    With Player(1).Controls
    {
        Controls &c = player[1].controls;
//    If frmMain.MousePointer <> 99 Then
        if(frmMain.MousePointer != 99)
        {
//        frmMain.MousePointer = 99
            frmMain.MousePointer = 99;
//    End If
        }
//    If .Up = False And .Down = False And .Jump = False And .Run = False And .Start = False Then
        if(!c.Up && !c.Down && !c.Jump && !c.Run && !c.Start)
        {
//        If (GetKeyState(vbKeySpace) And KEY_PRESSED) Or _
//           (GetKeyState(vbKeyReturn) And KEY_PRESSED) Or _
//           (GetKeyState(vbKeyUp) And KEY_PRESSED) Or _
//           (GetKeyState(vbKeyDown) And KEY_PRESSED) Or _
//           (GetKeyState(vbKeyEscape) And KEY_PRESSED) Then
            if(getKeyState(SDL_SCANCODE_SPACE) == KEY_PRESSED ||
               getKeyState(SDL_SCANCODE_RETURN) == KEY_PRESSED ||
               getKeyState(SDL_SCANCODE_UP) == KEY_PRESSED ||
               getKeyState(SDL_SCANCODE_DOWN) == KEY_PRESSED ||
               getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED
            ){
//        Else
            }
            else
            {
//            MenuCursorCanMove = True
            MenuCursorCanMove = true;
//        End If
            }
//    End If
        }
//    'For the menu controls
//        If getNewKeyboard = False And getNewJoystick = False Then
        if(!getNewKeyboard && !getNewJoystick)
        {
//            If .Up = True Or (GetKeyState(vbKeyUp) And KEY_PRESSED) Then
            if(c.Up || (getKeyState(SDL_SCANCODE_UP) == KEY_PRESSED))
            {
//                If MenuCursorCanMove = True Then
                if(MenuCursorCanMove)
                {
//                    MenuCursor = MenuCursor - 1
                    MenuCursor -= 1;
//                    If MenuMode >= 100 Then
                    if(MenuMode >= 100)
                    {
//                        Do While (MenuCursor = PlayerCharacter - 1 And _
//                                 (MenuMode = 300 Or MenuMode = 500)) Or _
//                                  blockCharacter(MenuCursor + 1) = True
                        while((MenuCursor == (PlayerCharacter - 1) &&
                              (MenuMode == 300 || MenuMode == 500)) ||
                               blockCharacter[MenuCursor + 1])
                        {
//                            MenuCursor = MenuCursor - 1
                            MenuCursor -= 1;
//                            If MenuCursor < 0 Then MenuCursor = numCharacters - 1
                            if(MenuCursor < 0)
                                MenuCursor = numCharacters - 1;
//                        Loop
                        }
//                    End If
                    }
//                    PlaySound 26
                    PlaySound(26);
//                End If
                }
//                MenuCursorCanMove = False
                MenuCursorCanMove = false;
//            ElseIf .Down = True Or (GetKeyState(vbKeyDown) And KEY_PRESSED) Then
            } else {
//                If MenuCursorCanMove = True Then
                if(MenuCursorCanMove)
                {
//                    MenuCursor = MenuCursor + 1
                    MenuCursor += 1;
//                    If MenuMode >= 100 Then
                    if(MenuMode >= 100)
                    {
//                        Do While (MenuCursor = PlayerCharacter - 1 And _
//                                 (MenuMode = 300 Or MenuMode = 500)) Or _
//                                  blockCharacter(MenuCursor + 1) = True
                        while((MenuCursor == (PlayerCharacter - 1) &&
                              (MenuMode == 300 || MenuMode == 500)) ||
                               blockCharacter[MenuCursor + 1])
                        {
//                            MenuCursor = MenuCursor + 1
                            MenuCursor += 1;
//                            If MenuCursor >= numCharacters Then MenuCursor = 0
                            if(MenuCursor >= numCharacters)
                                MenuCursor = 0;
//                        Loop
                        }
//                    End If
                    }
//                    PlaySound 26
                    PlaySound(26);
//                End If
                }
//                MenuCursorCanMove = False
                MenuCursorCanMove = false;
//            End If
            }
//        End If
        }

//        If MenuMode = 0 Then ' Main Menu
        // Main Menu
        if(MenuMode == 0)
        {
//            If MenuMouseMove = True Then
            if(MenuMouseMove)
            {
//                For A = 0 To 4
                For(A, 0, 4)
                {
//                    If MenuMouseY >= 350 + A * 30 And MenuMouseY <= 366 + A * 30 Then
                    if(MenuMouseY >= 350 + A * 30 && MenuMouseY <= 366 + A * 30)
                    {
//                        If A = 0 Then
                        if(A == 0)
//                            menuLen = 18 * Len("1 player game") - 2
                            menuLen = 18 * std::strlen("1 player game") - 2;
//                        ElseIf A = 1 Then
                        else if(A == 1)
//                            menuLen = 18 * Len("2 player game") - 2
                            menuLen = 18 * std::strlen("2 player game") - 2;
//                        ElseIf A = 2 Then
                        else if(A == 2)
//                            menuLen = 18 * Len("battle game")
                            menuLen = 18 * std::strlen("battle game");
//                        ElseIf A = 3 Then
                        else if(A == 3)
//                            menuLen = 18 * Len("options")
                            menuLen = 18 * std::strlen("options");
//                        Else
                        else
//                            menuLen = 18 * Len("exit")
                            menuLen = 18 * std::strlen("exit");
//                        End If

//                        If MenuMouseX >= 300 And MenuMouseX <= 300 + menuLen Then
                        if(MenuMouseX >= 300 && MenuMouseX <= 300 + menuLen)
                        {
//                            If MenuMouseRelease = True And MenuMouseDown = True Then MenuMouseClick = True
                            if(MenuMouseRelease && MenuMouseDown) MenuMouseClick = true;
//                            If MenuCursor <> A Then
                            if(MenuCursor != A)
                            {
//                                PlaySound 26
                                PlaySound(26);
//                                MenuCursor = A
                                MenuCursor = A;
//                            End If
                            }
//                        End If
                        }
//                    End If
                    }
//                Next A
                }
//            End If
            }

//            If (GetKeyState(vbKeyEscape) And KEY_PRESSED) And MenuCursorCanMove = True Then
            if(getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED && MenuCursorCanMove)
            {
//                If MenuCursor <> 4 Then
//                    MenuCursor = 4
//                    PlaySound 26
//                End If
//            ElseIf ((.Jump = True Or .Start = True Or (GetKeyState(vbKeySpace) And KEY_PRESSED) Or (GetKeyState(vbKeyReturn) And KEY_PRESSED)) And MenuCursorCanMove = True) Or MenuMouseClick = True Then
            }
            else if(1)
            {
//                MenuCursorCanMove = False
//                PlayerCharacter = 0
//                PlayerCharacter2 = 0
//                If MenuCursor = 0 Then
//                    PlaySound 29
//                    MenuMode = 1
//                    FindWorlds
//                    MenuCursor = 0
//                ElseIf MenuCursor = 1 Then
//                    PlaySound 29
//                    MenuMode = 2
//                    FindWorlds
//                    MenuCursor = 0
//                ElseIf MenuCursor = 2 Then
//                    PlaySound 29
//                    MenuMode = 4
//                    FindLevels
//                    MenuCursor = 0
//                ElseIf MenuCursor = 3 Then
//                    PlaySound 29
//                    MenuMode = 3
//                    MenuCursor = 0
//                ElseIf MenuCursor = 4 Then
//                    PlaySound 29
//                    BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
//                    BitBlt frmMain.hdc, 0, 0, frmMain.ScaleWidth, frmMain.ScaleHeight, 0, 0, 0, vbWhiteness
//                    StopMusic
//                    DoEvents
//                    Sleep 500
//                    KillIt
//                End If
//            End If
            }
//            If MenuCursor > 4 Then MenuCursor = 0
//            If MenuCursor < 0 Then MenuCursor = 4
//        ElseIf MenuMode = 100 Or MenuMode = 200 Or MenuMode = 300 Or MenuMode = 400 Or MenuMode = 500 Then  'Character Select
        }

        // Character Select
        else if(MenuMode == 100 || MenuMode == 200 || MenuMode == 300 || MenuMode == 400 || MenuMode == 500)
        {
//            If MenuMouseMove = True Then
//                B = 0
//                For A = 0 To 4
//                    If blockCharacter(A + 1) = True Then
//                        B = B - 30
//                    Else
//                        If MenuMouseY >= 350 + A * 30 + B And MenuMouseY <= 366 + A * 30 + B Then
//                            If A = 0 Then
//                                menuLen = 18 * Len("mario game") + 2
//                            ElseIf A = 3 Or A = 5 Then
//                                menuLen = 18 * Len("toad game")
//                            Else
//                                menuLen = 18 * Len("luigi game")
//                            End If
//                            If MenuMouseX >= 300 And MenuMouseX <= 300 + menuLen Then
//                                If MenuMouseRelease = True And MenuMouseDown = True Then MenuMouseClick = True
//                                If MenuCursor <> A Then
//                                    If ((MenuMode = 300 Or MenuMode = 500) And PlayerCharacter - 1 = A) Or (blockCharacter(A + 1) = True) And MenuMouseClick = True Then
//                                        MenuMouseClick = False
//                                    Else
//                                        PlaySound 26
//                                        MenuCursor = A
//                                    End If
//                                End If
//                            End If
//                        End If
//                    End If
//                Next A
//            End If
//            If MenuCursorCanMove = True Or MenuMouseClick = True Or MenuMouseBack = True Then
//                If .Run = True Or (GetKeyState(vbKeyEscape) And KEY_PRESSED) Or MenuMouseBack = True Then
//                    If MenuMode = 300 Then
//                        MenuMode = 200
//                        MenuCursor = PlayerCharacter - 1
//                    ElseIf MenuMode = 500 Then
//                        MenuMode = 400
//                        MenuCursor = PlayerCharacter - 1
//                    Else
//                        MenuCursor = selWorld - 1
//                        MenuMode = MenuMode / 100
//                    End If
//                    MenuCursorCanMove = False
//                    PlaySound 26
//                ElseIf .Jump = True Or .Start = True Or (GetKeyState(vbKeySpace) And KEY_PRESSED) Or (GetKeyState(vbKeyReturn) And KEY_PRESSED) Or MenuMouseClick = True Then
//                    PlaySound 29
//                    If MenuMode = 100 Then
//                        PlayerCharacter = MenuCursor + 1
//                        MenuMode = 10
//                        MenuCursor = 0
//                    ElseIf MenuMode = 200 Then
//                        PlayerCharacter = MenuCursor + 1
//                        MenuMode = 300
//                        MenuCursor = PlayerCharacter2
//                    ElseIf MenuMode = 300 Then
//                        PlayerCharacter2 = MenuCursor + 1
//                        MenuMode = 20
//                        MenuCursor = 0
//                    ElseIf MenuMode = 400 Then
//                        PlayerCharacter = MenuCursor + 1
//                        MenuMode = 500
//                        MenuCursor = PlayerCharacter2 - 1
//                        If MenuCursor < 0 Then MenuCursor = 0
//                    ElseIf MenuMode = 500 Then
//                        PlayerCharacter2 = MenuCursor + 1
//                        MenuCursor = 0
//                        StartBattleMode
//                        Exit Sub
//                    End If
//                    MenuCursorCanMove = False
//                End If
//            End If
//            If MenuMode > 0 Then
//                If MenuCursor > numCharacters - 1 Then
//                    MenuCursor = 0
//                    Do While (MenuCursor = PlayerCharacter - 1 And (MenuMode = 300 Or MenuMode = 500)) Or blockCharacter(MenuCursor + 1) = True
//                        MenuCursor = MenuCursor + 1
//                    Loop
//                End If
//                If MenuCursor < 0 Then
//                    MenuCursor = numCharacters - 1
//                    Do While (MenuCursor = PlayerCharacter - 1 And (MenuMode = 300 Or MenuMode = 500)) Or blockCharacter(MenuCursor + 1) = True
//                        MenuCursor = MenuCursor - 1
//                    Loop
//                End If
//            End If
//            Do While ((MenuMode = 300 Or MenuMode = 500) And MenuCursor = PlayerCharacter - 1) Or blockCharacter(MenuCursor + 1) = True
//                MenuCursor = MenuCursor + 1
//            Loop
//            If MenuMode >= 100 Then
//                If MenuCursor >= numCharacters Then
//                    MenuCursor = 0
//                Else
//                    For A = 1 To numPlayers
//                        Player(A).Character = MenuCursor + 1
//                        SizeCheck A
//                    Next A
//                    For A = 1 To numNPCs
//                        If NPC(A).Type = 13 Then NPC(A).Special = MenuCursor + 1
//                    Next A
//                End If
//            End If
//        ElseIf MenuMode = 1 Or MenuMode = 2 Or MenuMode = 4 Then 'World Select
        }

        // World Select
        else if(MenuMode == 1 || MenuMode == 2 || MenuMode == 4)
        {
//            If ScrollDelay > 0 Then
//                MenuMouseMove = True
//                ScrollDelay = ScrollDelay - 1
//            End If
//            If MenuMouseMove = True Then
//                B = 0
//                For A = minShow - 1 To maxShow - 1
//                    If MenuMouseY >= 350 + B * 30 And MenuMouseY <= 366 + B * 30 Then
//                        menuLen = 19 * Len(SelectWorld(A + 1).WorldName)
//                        If MenuMouseX >= 300 And MenuMouseX <= 300 + menuLen Then
//                            If MenuMouseRelease = True And MenuMouseDown = True Then MenuMouseClick = True
//                            If MenuCursor <> A And ScrollDelay = 0 Then
//                                ScrollDelay = 10
//                                PlaySound 26
//                                MenuCursor = A
//                            End If
//                        End If
//                    End If
//                    B = B + 1
//                Next A
//            End If
//            If MenuCursorCanMove = True Or MenuMouseClick = True Or MenuMouseBack = True Then
//                If .Run = True Or (GetKeyState(vbKeyEscape) And KEY_PRESSED) Or MenuMouseBack = True Then
//                    MenuCursor = MenuMode - 1
//                    If MenuMode = 4 Then MenuCursor = 2
//                    MenuMode = 0
//'world select back


//                    PlaySound 26
//                    MenuCursorCanMove = False
//                ElseIf .Jump = True Or .Start = True Or (GetKeyState(vbKeySpace) And KEY_PRESSED) Or (GetKeyState(vbKeyReturn) And KEY_PRESSED) Or MenuMouseClick = True Then
//                    PlaySound 29
//                    selWorld = MenuCursor + 1
//                    FindSaves
//                    For A = 1 To numCharacters
//                        If MenuMode = 4 Then
//                            blockCharacter(A) = False
//                        Else
//                            blockCharacter(A) = SelectWorld(selWorld).blockChar(A)
//                        End If
//                    Next A
//                    MenuMode = MenuMode * 100
//                    MenuCursor = 0
//                    If MenuMode = 400 And PlayerCharacter <> 0 Then MenuCursor = PlayerCharacter - 1
//                    MenuCursorCanMove = False
//                End If
//            End If
//            If MenuMode < 100 Then
//                If MenuCursor >= NumSelectWorld Then MenuCursor = 0
//                If MenuCursor < 0 Then MenuCursor = NumSelectWorld - 1
//            End If
//        ElseIf MenuMode = 10 Or MenuMode = 20 Then 'Save Select
        }

        // Save Select
        else if(MenuMode == 10 || MenuMode == 20)
        {
//            If MenuMouseMove = True Then
//                For A = 0 To 2
//                    If MenuMouseY >= 350 + A * 30 And MenuMouseY <= 366 + A * 30 Then
//                        menuLen = 18 * Len("slot 1 empty") - 2
//                        If SaveSlot(A + 1) >= 0 Then menuLen = 18 * Len("slot ... 100") - 2
//                        If SaveStars(A + 1) > 0 Then menuLen = 288 + Len(SaveStars(A + 1)) * 18
//                        If MenuMouseX >= 300 And MenuMouseX <= 300 + menuLen Then
//                            If MenuMouseRelease = True And MenuMouseDown = True Then MenuMouseClick = True
//                            If MenuCursor <> A Then
//                                PlaySound 26
//                                MenuCursor = A
//                            End If
//                        End If
//                    End If
//                Next A
//            End If
//            If MenuCursorCanMove = True Or MenuMouseClick = True Or MenuMouseBack = True Then
//                If .Run = True Or (GetKeyState(vbKeyEscape) And KEY_PRESSED) Or MenuMouseBack = True Then
//'save select back
//                    If AllCharBlock > 0 Then
//                        MenuMode = MenuMode / 10
//                        MenuCursor = selWorld - 1
//                    Else
//                        If MenuMode = 10 Then
//                            MenuCursor = PlayerCharacter - 1
//                            MenuMode = 100
//                        Else
//                            MenuMode = 300
//                            MenuCursor = PlayerCharacter2 - 1
//                        End If
//                    End If
//                    MenuCursorCanMove = False
//                    PlaySound 29
//                ElseIf .Jump = True Or .Start = True Or (GetKeyState(vbKeySpace) And KEY_PRESSED) Or (GetKeyState(vbKeyReturn) And KEY_PRESSED) Or MenuMouseClick = True Then
//                    PlaySound 29
//                    numPlayers = MenuMode / 10
//                    For A = 1 To numCharacters
//                        SavedChar(A) = blankPlayer
//                        SavedChar(A).Character = A
//                        SavedChar(A).State = 1
//                    Next A
//                    Player(1).State = 1
//                    Player(1).Mount = 0
//                    Player(1).Character = 1
//                    Player(1).HeldBonus = 0
//                    Player(1).CanFly = False
//                    Player(1).CanFly2 = False
//                    Player(1).TailCount = 0
//                    Player(1).YoshiBlue = False
//                    Player(1).YoshiRed = False
//                    Player(1).YoshiYellow = False
//                    Player(1).Hearts = 0
//                    Player(2).State = 1
//                    Player(2).Mount = 0
//                    Player(2).Character = 2
//                    Player(2).HeldBonus = 0
//                    Player(2).CanFly = False
//                    Player(2).CanFly2 = False
//                    Player(2).TailCount = 0
//                    Player(2).YoshiBlue = False
//                    Player(2).YoshiRed = False
//                    Player(2).YoshiYellow = False
//                    Player(2).Hearts = 0
//                    If numPlayers <= 2 And PlayerCharacter > 0 Then
//                        Player(1).Character = PlayerCharacter
//                        PlayerCharacter = 0
//                    End If
//                    If numPlayers = 2 And PlayerCharacter2 > 0 Then
//                        Player(2).Character = PlayerCharacter2
//                        PlayerCharacter2 = 0
//                    End If
//                    selSave = MenuCursor + 1
//                    numStars = 0
//                    Coins = 0
//                    Score = 0
//                    Lives = 3
//                    LevelSelect = True
//                    GameMenu = False
//                    BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
//                    BitBlt frmMain.hdc, 0, 0, frmMain.ScaleWidth, frmMain.ScaleHeight, 0, 0, 0, vbWhiteness
//                    StopMusic
//                    DoEvents
//                    Sleep 500
//                    OpenWorld SelectWorld(selWorld).WorldPath & SelectWorld(selWorld).WorldFile
//                    If SaveSlot(selSave) >= 0 Then
//                        If NoMap = False Then StartLevel = ""
//                        LoadGame
//                    End If
//                    If WorldUnlock = True Then
//                        For A = 1 To numWorldPaths
//                        tempLocation = WorldPath(A).Location
//                        With tempLocation
//                            .X = .X + 4
//                            .Y = .Y + 4
//                            .Width = .Width - 8
//                            .Height = .Height - 8
//                        End With
//                            WorldPath(A).Active = True
//                            For B = 1 To numScenes
//                                If CheckCollision(tempLocation, Scene(B).Location) Then Scene(B).Active = False
//                            Next B
//                        Next A
//                        For A = 1 To numWorldLevels
//                            WorldLevel(A).Active = True
//                        Next A
//                    End If
//                    SetupPlayers
//                    If StartLevel <> "" Then
//                        PlaySound 28
//                        SoundPause(26) = 200
//                        LevelSelect = False

//                        GameThing
//                        ClearLevel

//                        Sleep 1000
//                        OpenLevel SelectWorld(selWorld).WorldPath & StartLevel
//                    End If
//                    Exit Sub
//                End If
//            End If
//            If MenuMode < 100 Then
//                If MenuCursor > 2 Then MenuCursor = 0
//                If MenuCursor < 0 Then MenuCursor = 2
//            End If
//        ElseIf MenuMode = 3 Then 'Options
        }

        // Options
        else if(MenuMode == 3)
        {
//            If MenuMouseMove = True Then
//                For A = 0 To 3
//                    If MenuMouseY >= 350 + A * 30 And MenuMouseY <= 366 + A * 30 Then
//                        If A = 0 Then
//                            menuLen = 18 * Len("player 1 controls") - 4
//                        ElseIf A = 1 Then
//                            menuLen = 18 * Len("player 2 controls") - 4
//                        ElseIf A = 2 Then
//                            If resChanged = True Then
//                                menuLen = 18 * Len("windowed mode")
//                            Else
//                                menuLen = 18 * Len("fullscreen mode")
//                            End If
//                        Else
//                            menuLen = 18 * Len("view credits") - 2
//                        End If
//                        If MenuMouseX >= 300 And MenuMouseX <= 300 + menuLen Then
//                            If MenuMouseRelease = True And MenuMouseDown = True Then MenuMouseClick = True
//                            If MenuCursor <> A Then
//                                PlaySound 26
//                                MenuCursor = A
//                            End If
//                        End If
//                    End If
//                Next A
//            End If
//            If MenuCursorCanMove = True Or MenuMouseClick = True Or MenuMouseBack = True Then
//                If .Run = True Or (GetKeyState(vbKeyEscape) And KEY_PRESSED) Or MenuMouseBack = True Then
//                    MenuMode = 0
//                    MenuCursor = 3
//                    MenuCursorCanMove = False
//                    PlaySound 26
//                ElseIf .Jump = True Or .Start = True Or (GetKeyState(vbKeySpace) And KEY_PRESSED) Or (GetKeyState(vbKeyReturn) And KEY_PRESSED) Or MenuMouseClick = True Then
//                    MenuCursorCanMove = False
//                    If MenuCursor = 0 Then
//                        MenuCursor = 0
//                        MenuMode = 31
//                        PlaySound 26
//                    ElseIf MenuCursor = 1 Then
//                        MenuCursor = 0
//                        MenuMode = 32
//                        PlaySound 26
//                    ElseIf MenuCursor = 2 Then
//                        PlaySound 29
//                        ChangeScreen
//                    ElseIf MenuCursor = 3 Then
//                        PlaySound 29
//                        GameMenu = False
//                        GameOutro = True
//                        CreditChop = 300
//                        EndCredits = 0
//                        SetupCredits
//                    End If
//                End If
//            End If
//            If MenuMode = 3 Then
//                If MenuCursor > 3 Then MenuCursor = 0
//                If MenuCursor < 0 Then MenuCursor = 3
//            End If

//        ElseIf MenuMode = 31 Or MenuMode = 32 Then 'Input Settings
        }

        // Input Settings
        else if(MenuMode == 31 || MenuMode == 32)
        {
//            If MenuMouseMove = True And getNewJoystick = False And getNewKeyboard = False Then
//                If useJoystick(MenuMode - 30) = 0 Then
//                    For A = 0 To 10
//                        If MenuMouseY >= 260 - 44 + A * 30 And MenuMouseY <= 276 - 44 + A * 30 Then
//                            If A = 0 Then
//                                menuLen = 18 * Len("INPUT......KEYBOARD")
//                            ElseIf A = 1 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Up)))
//                            ElseIf A = 2 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Down)))
//                            ElseIf A = 3 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Left)))
//                            ElseIf A = 4 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Right)))
//                            ElseIf A = 5 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Run)))
//                            ElseIf A = 6 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).AltRun)))
//                            ElseIf A = 7 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Jump)))
//                            ElseIf A = 8 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).AltJump)))
//                            ElseIf A = 9 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Drop)))
//                            ElseIf A = 10 Then
//                                menuLen = 18 * Len("UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Start)))
//                            End If
//                            If MenuMouseX >= 300 And MenuMouseX <= 300 + menuLen Then
//                                If MenuMouseRelease = True And MenuMouseDown = True Then MenuMouseClick = True
//                                If MenuCursor <> A Then
//                                    PlaySound 26
//                                    MenuCursor = A
//                                End If
//                            End If
//                        End If
//                    Next A
//                Else
//                    For A = 0 To 6
//                        If MenuMouseY >= 260 - 44 + A * 30 And MenuMouseY <= 276 + A * 30 - 44 Then
//                            If A = 0 Then
//                                menuLen = 18 * Len("INPUT......JOYSTICK 1") - 2
//                            Else
//                                menuLen = 18 * Len("RUN........_")
//                            End If
//                            If MenuMouseX >= 300 And MenuMouseX <= 300 + menuLen Then
//                                If MenuMouseRelease = True And MenuMouseDown = True Then MenuMouseClick = True
//                                If MenuCursor <> A Then
//                                    PlaySound 26
//                                    MenuCursor = A
//                                End If
//                            End If
//                        End If
//                    Next A
//                End If
//            End If
//            If MenuCursorCanMove = True Or ((getNewKeyboard = False And getNewJoystick = False) And (MenuMouseClick = True Or MenuMouseBack = True)) Then
//                If getNewKeyboard = True Then
//                    If inputKey <> 0 Then
//                        getNewKeyboard = False
//                        MenuCursorCanMove = False
//                        PlaySound 29
//                        If MenuCursor = 1 Then
//                            conKeyboard(MenuMode - 30).Up = inputKey
//                        ElseIf MenuCursor = 2 Then conKeyboard(MenuMode - 30).Down = inputKey
//                        ElseIf MenuCursor = 3 Then conKeyboard(MenuMode - 30).Left = inputKey
//                        ElseIf MenuCursor = 4 Then conKeyboard(MenuMode - 30).Right = inputKey
//                        ElseIf MenuCursor = 5 Then conKeyboard(MenuMode - 30).Run = inputKey
//                        ElseIf MenuCursor = 6 Then conKeyboard(MenuMode - 30).AltRun = inputKey
//                        ElseIf MenuCursor = 7 Then conKeyboard(MenuMode - 30).Jump = inputKey
//                        ElseIf MenuCursor = 8 Then conKeyboard(MenuMode - 30).AltJump = inputKey
//                        ElseIf MenuCursor = 9 Then conKeyboard(MenuMode - 30).Drop = inputKey
//                        ElseIf MenuCursor = 10 Then conKeyboard(MenuMode - 30).Start = inputKey
//                        End If
//                    End If
//                ElseIf getNewJoystick = True Then
//                    JoyNum = useJoystick(MenuMode - 30) - 1
//                    PollJoystick
//                    If JoyButtons(oldJumpJoy) = True Then
//                    Else
//                        oldJumpJoy = -1
//                        For A = 0 To 15
//                            If JoyButtons(A) = True Then
//                                PlaySound 29
//                                If MenuCursor = 1 Then
//                                    conJoystick(MenuMode - 30).Run = A
//                                ElseIf MenuCursor = 2 Then conJoystick(MenuMode - 30).AltRun = A
//                                ElseIf MenuCursor = 3 Then conJoystick(MenuMode - 30).Jump = A
//                                ElseIf MenuCursor = 4 Then conJoystick(MenuMode - 30).AltJump = A
//                                ElseIf MenuCursor = 5 Then conJoystick(MenuMode - 30).Drop = A
//                                ElseIf MenuCursor = 6 Then conJoystick(MenuMode - 30).Start = A
//                                End If
//                                getNewJoystick = False
//                                MenuCursorCanMove = False
//                                Exit For
//                            ElseIf (GetKeyState(vbKeyEscape) And KEY_PRESSED) Then
//                                If MenuCursor = 1 Then
//                                    conJoystick(MenuMode - 30).Run = lastJoyButton
//                                ElseIf MenuCursor = 2 Then conJoystick(MenuMode - 30).AltRun = lastJoyButton
//                                ElseIf MenuCursor = 3 Then conJoystick(MenuMode - 30).AltJump = lastJoyButton
//                                ElseIf MenuCursor = 4 Then conJoystick(MenuMode - 30).Jump = lastJoyButton
//                                ElseIf MenuCursor = 5 Then conJoystick(MenuMode - 30).Drop = lastJoyButton
//                                ElseIf MenuCursor = 6 Then conJoystick(MenuMode - 30).Start = lastJoyButton
//                                End If
//                                getNewJoystick = False
//                                MenuCursorCanMove = False
//                            End If
//                        Next A
//                    End If
//                Else
//                    If .Run = True Or (GetKeyState(vbKeyEscape) And KEY_PRESSED) Or MenuMouseBack = True Then
//                        SaveConfig
//                        MenuCursor = MenuMode - 31
//                        MenuMode = 3
//                        MenuCursorCanMove = False
//                        PlaySound 26
//                    ElseIf .Jump = True Or .Start = True Or (GetKeyState(vbKeySpace) And KEY_PRESSED) Or (GetKeyState(vbKeyReturn) And KEY_PRESSED) Or MenuMouseClick = True Then
//                        If MenuCursor = 0 Then
//                            PlaySound 29
//                            useJoystick(MenuMode - 30) = useJoystick(MenuMode - 30) + 1
//                            If useJoystick(MenuMode - 30) > numJoysticks Then useJoystick(MenuMode - 30) = 0
//                        Else
//                            If useJoystick(MenuMode - 30) = 0 Then
//                                getNewKeyboard = True
//                                inputKey = Asc("_")
//                                If MenuCursor = 1 Then
//                                    conKeyboard(MenuMode - 30).Up = inputKey
//                                ElseIf MenuCursor = 2 Then conKeyboard(MenuMode - 30).Down = inputKey
//                                ElseIf MenuCursor = 3 Then conKeyboard(MenuMode - 30).Left = inputKey
//                                ElseIf MenuCursor = 4 Then conKeyboard(MenuMode - 30).Right = inputKey
//                                ElseIf MenuCursor = 5 Then conKeyboard(MenuMode - 30).Run = inputKey
//                                ElseIf MenuCursor = 6 Then conKeyboard(MenuMode - 30).AltRun = inputKey
//                                ElseIf MenuCursor = 7 Then conKeyboard(MenuMode - 30).Jump = inputKey
//                                ElseIf MenuCursor = 8 Then conKeyboard(MenuMode - 30).AltJump = inputKey
//                                ElseIf MenuCursor = 9 Then conKeyboard(MenuMode - 30).Drop = inputKey
//                                ElseIf MenuCursor = 10 Then conKeyboard(MenuMode - 30).Start = inputKey
//                                End If
//                                inputKey = 0
//                            Else
//                                If MenuCursor = 1 Then
//                                    lastJoyButton = conJoystick(MenuMode - 30).Run
//                                    conJoystick(MenuMode - 30).Run = -1
//                                ElseIf MenuCursor = 2 Then
//                                    lastJoyButton = conJoystick(MenuMode - 30).AltRun
//                                    conJoystick(MenuMode - 30).AltRun = -1
//                                ElseIf MenuCursor = 3 Then
//                                    lastJoyButton = conJoystick(MenuMode - 30).Jump
//                                    oldJumpJoy = conJoystick(MenuMode - 30).Jump
//                                    conJoystick(MenuMode - 30).Jump = -1
//                                ElseIf MenuCursor = 4 Then
//                                    lastJoyButton = conJoystick(MenuMode - 30).AltJump
//                                    conJoystick(MenuMode - 30).AltJump = -1
//                                ElseIf MenuCursor = 5 Then
//                                    lastJoyButton = conJoystick(MenuMode - 30).Drop
//                                    conJoystick(MenuMode - 30).Drop = -1
//                                ElseIf MenuCursor = 6 Then
//                                    lastJoyButton = conJoystick(MenuMode - 30).Start
//                                    conJoystick(MenuMode - 30).Start = -1
//                                End If
//                                getNewJoystick = True
//                                MenuCursorCanMove = False
//                            End If
//                        End If
//                        MenuCursorCanMove = False
//                    End If
//                End If
//            End If
//            If MenuMode <> 3 Then
//                If useJoystick(MenuMode - 30) = 0 Then
//                    If MenuCursor > 10 Then MenuCursor = 0
//                    If MenuCursor < 0 Then MenuCursor = 10
//                Else
//                    If MenuCursor > 6 Then MenuCursor = 0
//                    If MenuCursor < 0 Then MenuCursor = 6
//                End If
//            End If
//        End If
        }
//    End With
    }


//'check for all characters blocked
//        If MenuMode = 100 Or MenuMode = 200 Or MenuMode = 300 Then
    if(MenuMode == 100 || MenuMode == 200 || MenuMode == 300)
    {
//            AllCharBlock = 0
//            For A = 1 To numCharacters
//                If blockCharacter(A) = False Then
//                    If AllCharBlock = 0 Then
//                        AllCharBlock = A
//                    Else
//                        AllCharBlock = 0
//                        Exit For
//                    End If
//                End If
//            Next A
//            If AllCharBlock > 0 Then
//                PlayerCharacter = AllCharBlock
//                PlayerCharacter2 = AllCharBlock
//                If MenuMode = 100 Then
//                    MenuMode = 10
//                    MenuCursor = 0
//                ElseIf MenuMode = 200 Then
//                    MenuMode = 300
//                    MenuCursor = PlayerCharacter2
//                Else
//                    MenuMode = 20
//                    MenuCursor = 0
//                End If
//            End If
//        End If
    }



//    If CheckLiving = 0 Then
    if(CheckLiving() == 0)
    {
//        ShowLayer "Destroyed Blocks"
        ShowLayer("Destroyed Blocks");
//        For A = 1 To numNPCs
//            If NPC(A).DefaultType = 0 Then
//                If NPC(A).TimeLeft > 10 Then NPC(A).TimeLeft = 10
//            End If
//        Next A
//    End If
    }
//    For A = 1 To numPlayers
//        With Player(A)
//            If .TimeToLive > 0 Then
//                .TimeToLive = 0
//                .Dead = True
//            End If
//            .Controls.Down = False
//            .Controls.Drop = False
//            .Controls.Right = True
//            .Controls.Left = False
//            .Controls.Run = True
//            .Controls.Up = False
//            .Controls.AltRun = False
//            .Controls.AltJump = False
//            If .Jump = 0 Or .Location.Y < level(0).Y + 200 Then .Controls.Jump = False
//            If .Location.SpeedX < 0.5 Then
//                .Controls.Jump = True
//                If .Slope > 0 Or .StandingOnNPC > 0 Or .Location.SpeedY = 0 Then .CanJump = True
//            End If
//            If .HoldingNPC = 0 Then
//                If (.State = 3 Or .State = 6 Or .State = 7) And Rnd * 100 > 90 Then
//                    If .FireBallCD = 0 And .RunRelease = False Then
//                        .Controls.Run = False
//                    End If
//                End If
//                If (.State = 4 Or .State = 5) And .TailCount = 0 And .RunRelease = False Then
//                    tempLocation.Width = 24
//                    tempLocation.Height = 20
//                    tempLocation.Y = .Location.Y + .Location.Height - 22
//                    tempLocation.X = .Location.X + .Location.Width
//                    For B = 1 To numNPCs
//                        If NPC(B).Active = True And Not NPCIsABonus(NPC(B).Type) And Not NPCWontHurt(NPC(B).Type) And NPC(B).HoldingPlayer = 0 Then
//                            If CheckCollision(tempLocation, NPC(B).Location) Then
//                                .Controls.Run = False
//                            End If
//                        End If
//                    Next B
//                End If
//                If .StandingOnNPC > 0 Then
//                    If NPCGrabFromTop(NPC(.StandingOnNPC).Type) = True Then
//                        .Controls.Down = True
//                        .Controls.Run = True
//                        .RunRelease = True
//                    End If
//                End If
//            End If
//            If .Character = 5 Then
//                If .FireBallCD = 0 And .RunRelease = False Then
//                    tempLocation.Width = 38 + .Location.SpeedX * 0.5
//                    tempLocation.Height = .Location.Height - 8
//                    tempLocation.Y = .Location.Y + 4
//                    tempLocation.X = .Location.X + .Location.Width
//                    For B = 1 To numNPCs
//                        If NPC(B).Active = True And Not NPCIsABonus(NPC(B).Type) And Not NPCWontHurt(NPC(B).Type) And NPC(B).HoldingPlayer = 0 Then
//                            If CheckCollision(tempLocation, NPC(B).Location) Then
//                                .RunRelease = True
//                                If NPC(B).Location.Y > .Location.Y + .Location.Height / 2 Then .Controls.Down = True
//                                Exit For
//                            End If
//                        End If
//                    Next B
//                End If
//                If .Slope = 0 And .StandingOnNPC = 0 Then
//                    If .Location.SpeedY < 0 Then
//                        tempLocation.Width = 200
//                        tempLocation.Height = .Location.Y - level(0).Y + .Location.Height
//                        tempLocation.Y = level(0).Y
//                        tempLocation.X = .Location.X
//                        For B = 1 To numNPCs
//                            If NPC(B).Active = True And Not NPCIsABonus(NPC(B).Type) And Not NPCWontHurt(NPC(B).Type) And NPC(B).HoldingPlayer = 0 Then
//                                If CheckCollision(tempLocation, NPC(B).Location) Then
//                                    .Controls.Up = True
//                                    Exit For
//                                End If
//                            End If
//                        Next B
//                    ElseIf .Location.SpeedY > 0 Then
//                        tempLocation.Width = 200
//                        tempLocation.Height = level(0).Height - .Location.Y
//                        tempLocation.Y = .Location.Y
//                        tempLocation.X = .Location.X
//                        For B = 1 To numNPCs
//                            If NPC(B).Active = True And Not NPCIsABonus(NPC(B).Type) And Not NPCWontHurt(NPC(B).Type) And NPC(B).HoldingPlayer = 0 Then
//                                If CheckCollision(tempLocation, NPC(B).Location) Then
//                                    .Controls.Down = True
//                                    Exit For
//                                End If
//                            End If
//                        Next B
//                    End If
//                End If
//            End If


//            If .Location.X < -vScreenX(1) - .Location.Width And Not -vScreenX(1) <= level(0).X Then .Dead = True
//            If .Location.X > -vScreenX(1) + 1000 Then .Dead = True
//            If .Location.X > -vScreenX(1) + 600 And -vScreenX(1) + 850 < level(0).Width Then .Controls.Run = False
//            If -vScreenX(1) <= level(0).X And (.Dead = True Or .TimeToLive > 0) Then
//                .ForceHold = 65
//                .State = Int(Rnd * 6) + 2
//                .CanFly = False
//                .CanFly2 = False
//                .TailCount = 0
//                .Dead = False
//                .TimeToLive = 0
//                .Character = Int(Rnd * 5) + 1
//                If A >= 1 And A <= 5 Then .Character = A
//                .HeldBonus = 0
//                .Section = 0
//                .Mount = 0
//                .MountType = 0
//                .YoshiBlue = False
//                .YoshiRed = False
//                .YoshiYellow = False
//                .YoshiNPC = 0
//                .Wet = 0
//                .WetFrame = False
//                .YoshiPlayer = 0
//                .Bumped = False
//                .Bumped2 = 0
//                .Direction = 1
//                .Dismount = 0
//                .Effect = 0
//                .Effect2 = 0
//                .FireBallCD = 0
//                .ForceHold = 0
//                .Warp = 0
//                .WarpCD = 0
//                .GroundPound = False
//                .Immune = 0
//                .Frame = 0
//                .Slope = 0
//                .Slide = False
//                .SpinJump = False
//                .FrameCount = 0
//                .TailCount = 0
//                .Duck = False
//                .GroundPound = False
//                .Hearts = 3
//                PlayerFrame A
//                .Location.Height = Physics.PlayerHeight(.Character, .State)
//                .Location.Width = Physics.PlayerWidth(.Character, .State)
//                .Location.X = level(.Section).X - A * 48
//                .Location.SpeedX = Physics.PlayerRunSpeed
//                .Location.Y = level(.Section).Height - .Location.Height - 33
//                Do
//                    tempBool = True
//                    For B = 1 To numBlock
//                        If CheckCollision(.Location, Block(B).Location) = True Then
//                            .Location.Y = Block(B).Location.Y - .Location.Height - 0.1
//                            tempBool = False
//                        End If
//                    Next B
//                Loop While tempBool = False
//                If UnderWater(.Section) = False Then
//                    If Int(Rnd * 25) + 1 = 25 Then
//                        .Mount = 1
//                        .MountType = Int(Rnd * 3) + 1
//                        If .State = 1 Then
//                            .Location.Height = Physics.PlayerHeight(1, 2)
//                            .Location.Y = .Location.Y - Physics.PlayerHeight(1, 2) + Physics.PlayerHeight(.Character, 1)
//                        End If
//                    End If
//                End If
//                If .Mount = 0 And .Character <= 2 Then
//                    If Int(Rnd * 15) + 1 = 15 Then
//                        .Mount = 3
//                        .MountType = Int(Rnd * 7) + 1
//                        .Location.Y = .Location.Y + .Location.Height
//                        .Location.Height = Physics.PlayerHeight(2, 2)
//                        .Location.Y = .Location.Y - .Location.Height - 0.01
//                    End If
//                End If
//                .CanFly = False
//                .CanFly2 = False
//                .RunCount = 0
//                If .Mount = 0 And .Character <> 5 Then
//                    numNPCs = numNPCs + 1
//                    .HoldingNPC = numNPCs
//                    .ForceHold = 120
//                    With NPC(numNPCs)
//                        Do
//                            Do
//                                .Type = Int(Rnd * 286) + 1
//                            Loop While .Type = 11 Or .Type = 16 Or .Type = 18 Or .Type = 15 Or .Type = 21 Or .Type = 12 Or .Type = 13 Or .Type = 30 Or .Type = 17 Or .Type = 31 Or .Type = 32 Or (.Type >= 37 And .Type <= 44) Or .Type = 46 Or .Type = 47 Or .Type = 50 Or (.Type >= 56 And .Type <= 70) Or .Type = 8 Or .Type = 74 Or .Type = 51 Or .Type = 52 Or .Type = 75 Or .Type = 34 Or NPCIsToad(.Type) Or NPCIsAnExit(.Type) Or NPCIsYoshi(.Type) Or (.Type >= 78 And .Type <= 87) Or .Type = 91 Or .Type = 93 Or (.Type >= 104 And .Type <= 108) Or .Type = 125 Or .Type = 133 Or (.Type >= 148 And .Type <= 151) Or .Type = 159 Or .Type = 160 Or .Type = 164 Or .Type = 168 Or (.Type >= 154 And .Type <= 157) Or .Type = 159 Or .Type = 160 Or .Type = 164 Or .Type = 165 Or .Type = 171 Or .Type = 178 Or .Type = 197 Or .Type = 180 Or .Type = 181 Or .Type = 190 Or .Type = 192 Or .Type = 196 Or .Type = 197 Or (UnderWater(0) = True And NPCIsBoot(.Type) = True) Or (.Type >= 198 And .Type <= 228) Or .Type = 234
//                        Loop While .Type = 235 Or .Type = 231 Or .Type = 179 Or .Type = 49 Or .Type = 237 Or .Type = 238 Or .Type = 239 Or .Type = 240 Or .Type = 245 Or .Type = 246 Or .Type = 248 Or .Type = 254 Or .Type = 255 Or .Type = 256 Or .Type = 257 Or .Type = 259 Or .Type = 260 Or .Type = 262 Or .Type = 263 Or .Type = 265 Or .Type = 266 Or (.Type >= 267 And .Type <= 272) Or .Type = 275 Or .Type = 276 Or (.Type >= 280 And .Type <= 284) Or .Type = 241
//                        .Active = True
//                        .HoldingPlayer = A
//                        .Location.Height = NPCHeight(.Type)
//                        .Location.Width = NPCWidth(.Type)
//                        .Location.Y = Player(A).Location.Y  'level(.Section).Height + 1000
//                        .Location.X = Player(A).Location.X 'level(.Section).X + 1000
//                        .TimeLeft = 100
//                        .Section = Player(A).Section
//                    End With
//                End If
//            ElseIf .Location.X > level(.Section).Width + 64 Then
//                .Dead = True
//            End If
//            If .WetFrame = True Then
//                If .Location.SpeedY = 0 Or .Slope > 0 Then .CanJump = True
//                If Rnd * 100 > 98 Or .Location.SpeedY = 0 Or .Slope > 0 Then .Controls.Jump = True
//            End If
//            If Rnd * 100 > 95 And .HoldingNPC = 0 And .Slide = False And .CanAltJump = True And .Mount = 0 Then .Controls.AltJump = True
//            If Rnd * 1000 >= 999 And .CanFly2 = False Then .Controls.Run = False
//            If .Mount = 3 And Rnd * 100 >= 98 And .RunRelease = False Then .Controls.Run = False
//            If NPC(.HoldingNPC).Type = 22 Or NPC(.HoldingNPC).Type = 49 Then .Controls.Run = True
//            If .Slide = True And .CanJump = True Then
//                If .Location.SpeedX > -2 And .Location.SpeedX < 2 Then .Controls.Jump = True
//            End If
//            If .CanFly = False And .CanFly2 = False And (.State = 4 Or .State = 5) And .Slide = False Then
//                .Controls.Jump = True
//            End If
//            If .Quicksand > 0 Then
//                .CanJump = True
//                .Controls.Jump = True
//            End If
//            If .FloatTime > 0 Or (.CanFloat = True And .FloatRelease = True And .Jump = 0 And .Location.SpeedY > 0 And Rnd * 100 > 95) Then
//                .Controls.Jump = True
//            End If
//            If NPC(.HoldingNPC).Type = 13 And Rnd * 100 > 95 Then
//                .Controls.Run = False
//                If Rnd * 2 > 1 Then .Controls.Up = True
//                If Rnd * 2 > 1 Then .Controls.Right = False
//            End If

//            If .Slide = False And (.Slope > 0 Or .StandingOnNPC > 0 Or .Location.SpeedY = 0) Then
//                tempLocation = .Location
//                tempLocation.Width = 95
//                tempLocation.Height = tempLocation.Height - 1
//                For B = 1 To numBlock
//                    If BlockSlope(Block(B).Type) = 0 And BlockIsSizable(Block(B).Type) = False And BlockOnlyHitspot1(Block(B).Type) = False And Block(B).Hidden = False Then
//                        If CheckCollision(Block(B).Location, tempLocation) = True Then
//                            .CanJump = True
//                            .SpinJump = False
//                            .Controls.Jump = True
//                            Exit For
//                        End If
//                    End If
//                Next B
//            End If
//            If .Slope = 0 And .Slide = False And .StandingOnNPC = 0 And (.Slope > 0 Or .Location.SpeedY = 0) Then
//                tempBool = False
//                tempLocation = .Location
//                tempLocation.Width = 16
//                tempLocation.Height = 16
//                tempLocation.X = .Location.X + .Location.Width
//                tempLocation.Y = .Location.Y + .Location.Height
//                For B = 1 To numBlock
//                    If (BlockIsSizable(Block(B).Type) = False Or Block(B).Location.Y > .Location.Y + .Location.Height - 1) And BlockOnlyHitspot1(Block(B).Type) = False And Block(B).Hidden = False Then
//                        If CheckCollision(Block(B).Location, tempLocation) = True Then
//                            tempBool = True
//                            Exit For
//                        End If
//                    End If
//                Next B
//                If tempBool = False Then
//                    .CanJump = True
//                    .SpinJump = False
//                    .Controls.Jump = True
//                End If
//            End If
//            If .Character = 5 And .Controls.Jump = True Then
//                .Controls.AltJump = True
//                '.Controls.Jump = False
//            End If

//        End With
//    Next A
//    If LevelMacro > 0 Then UpdateMacro
//    UpdateLayers
//    UpdateNPCs
//    UpdateBlocks
//    UpdateEffects
//    UpdatePlayer
//    UpdateGraphics
//    UpdateSound
//    UpdateEvents


//    If MenuMouseDown = True Then
//        If Rnd * 100 > 40 Then
//            NewEffect 80, newLoc(MenuMouseX - vScreenX(1), MenuMouseY - vScreenY(1))
//            Effect(numEffects).Location.SpeedX = Rnd * 4 - 2
//            Effect(numEffects).Location.SpeedY = Rnd * 4 - 2
//        End If
//        For A = 1 To numNPCs
//            If NPC(A).Active = True Then
//                If CheckCollision(newLoc(MenuMouseX - vScreenX(1), MenuMouseY - vScreenY(1)), NPC(A).Location) = True Then
//                    If NPCIsACoin(NPC(A).Type) = False Then
//                        NPC(0) = NPC(A)
//                        NPC(0).Location.X = MenuMouseX - vScreenX(1)
//                        NPC(0).Location.Y = MenuMouseY - vScreenY(1)
//                        NPCHit A, 3, 0
//                    Else
//                        NewEffect 78, NPC(A).Location
//                        NPC(A).Killed = 9
//                    End If
//                End If
//            End If
//        Next A
//        For A = 1 To numBlock
//            If Block(A).Hidden = False Then
//                If CheckCollision(newLoc(MenuMouseX - vScreenX(1), MenuMouseY - vScreenY(1)), Block(A).Location) = True Then
//                    BlockHit A
//                    BlockHitHard A
//                End If
//            End If
//        Next A
//    End If

//    MenuMouseMove = False
//    MenuMouseClick = False
//    If MenuMouseDown = True Then
//        MenuMouseRelease = False
//    Else
//        MenuMouseRelease = True
//    End If
//    MenuMouseBack = False
}
