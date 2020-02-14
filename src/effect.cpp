#include "globals.h"
#include "effect.h"

// Updates the effects
void UpdateEffects()
{
// please reference the /graphics/effect folder to see what the effects are

//    Dim A As Integer
//    Dim B As Integer
//    Dim DontSpawnExit As Boolean
//    Dim DontResetMusic As Boolean
//    Dim tempBool As Boolean
//    Dim CoinCount As Integer
//    If FreezeNPCs = True Then Exit Sub
    if(FreezeNPCs)
        return;
//    For A = 1 To numEffects
    For(A, 1, numEffects)
    {
//        With Effect(A)
        {
            auto &e = Effect[A];
//            .Life = .Life - 1
            e.Life -= 1;

//            If .Life = 0 Then
//                If .Type = 14 Then
//                    If .NewNpc > 0 Then
//                        numNPCs = numNPCs + 1
//                        With NPC(numNPCs)
//                            .Type = Effect(A).NewNpc
//                            .Location.Height = NPCHeight(.Type)
//                            .Location.Width = NPCWidth(.Type)
//                            .Location.X = Effect(A).Location.X + Effect(A).Location.Width / 2 - .Location.Width / 2
//                            .Location.Y = Effect(A).Location.Y - 1
//                            .Location.SpeedY = -6
//                            .Active = True
//                            .TimeLeft = 100
//                            .Frame = 0
//                            CheckSectionNPC numNPCs
//                            PlaySound 20
//                        End With
//                    End If
//                End If
//            End If

//            .Location.X = .Location.X + .Location.SpeedX
//            .Location.Y = .Location.Y + .Location.SpeedY
            e.Location.X += e.Location.SpeedX;
            e.Location.Y += e.Location.SpeedY;

//            If .Type = 2 Or .Type = 126 Or .Type = 6 Or .Type = 23 Or .Type = 35 Or .Type = 37 Or .Type = 39 Or .Type = 41 Or .Type = 43 Or .Type = 45 Or .Type = 52 Or .Type = 62 Then     'Stomped Goombas
//                .Location.SpeedY = 0
//                .Location.SpeedX = 0
//            ElseIf .Type = 112 Then
//                If .Life Mod 5 = 0 Then
//                    NewEffect 108, newLoc(.Location.X + Rnd * .Location.Width, .Location.Y + Rnd * .Location.Height)
//                End If
//            ElseIf .Type = 111 Then
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 4 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 7 Then .Frame = 0
//                    If .Frame >= 14 Then .Frame = 7
//                End If
//            ElseIf .Type = 108 Then
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 4 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                End If
//                If .Frame >= 7 Then .Life = 0
//            ElseIf .Type = 136 Then 'RotoDisk
//                If .Location.SpeedX <> 0 Or .Location.SpeedY <> 0 Then
//                    .Location.SpeedY = .Location.SpeedY + 0.5
//                End If
//                .Frame = .Frame + 1
//                If .Frame >= 5 Then .Frame = 0
//            ElseIf .Type = 69 Then 'bomb
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 2 Then
//                    .FrameCount = 0
//                    If .Frame = 0 Then
//                        .Frame = 1
//                    Else
//                        .Frame = 0
//                    End If
//                End If
//            ElseIf .Type = 1 Or .Type = 21 Or .Type = 30 Or .Type = 51 Or .Type = 100 Or .Type = 135 Then 'Block break
//                .Location.SpeedY = .Location.SpeedY + 0.6
//                .Location.SpeedX = .Location.SpeedX * 0.99
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 3 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 4 Then .Frame = 0
//                End If
//            ElseIf .Type = 140 Then 'larry shell
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 4 Then
//                    .Frame = .Frame + 1
//                    .FrameCount = 0
//                End If
//                If .Frame > 7 Then .Frame = 0

//                If .Life = 100 Then
//                    .Location.SpeedY = -8
//                    PlaySound 63
//                End If

//            ElseIf .Type = 114 Then 'Splash
//                .FrameCount = .FrameCount + 1
//                If .FrameCount < 8 Then
//                    .Frame = 0
//                ElseIf .FrameCount < 16 Then
//                    .Frame = 1
//                ElseIf .FrameCount < 24 Then
//                    .Frame = 2
//                ElseIf .FrameCount < 32 Then
//                    .Frame = 3
//                ElseIf .FrameCount < 40 Then
//                    .Frame = 4
//                Else
//                    .Life = 0
//                End If
//                If .FrameCount Mod 3 = 0 Then .Frame = 5
//            ElseIf .Type = 113 Then 'Water Bubbles
//                If .NewNpc = 0 Then
//                    tempBool = False
//                    For B = 1 To numWater
//                        If CheckCollision(.Location, Water(B).Location) And Water(B).Hidden = False Then
//                            tempBool = True
//                            Exit For
//                        End If
//                    Next B
//                    If tempBool = False Then .Life = 0
//                End If
//                .FrameCount = .FrameCount + 1
//                If .FrameCount < 4 Then
//                    .Frame = 0
//                ElseIf .FrameCount < 6 Then
//                    .Frame = 1
//                Else
//                    .FrameCount = 0
//                    .Frame = 0
//                End If
//                .Location.Y = .Location.Y - 2
//                .Location.X = .Location.X + Rnd * 2 - 1
//            ElseIf .Type = 57 Then 'egg shells
//                .Location.SpeedY = .Location.SpeedY + 0.6
//                .Location.SpeedX = .Location.SpeedX * 0.99
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//            ElseIf .Type = 3 Or .Type = 5 Or .Type = 129 Or .Type = 130 Or .Type = 134 Then 'Mario & Luigi death
//                .Location.SpeedY = .Location.SpeedY + 0.25
//            ElseIf .Type = 145 Or .Type = 110 Or .Type = 127 Or .Type = 4 Or .Type = 143 Or .Type = 142 Or .Type = 7 Or .Type = 22 Or .Type = 31 Or .Type = 33 Or .Type = 34 Or .Type = 38 Or .Type = 40 Or .Type = 42 Or .Type = 44 Or .Type = 46 Or .Type = 53 Or .Type = 117 Then     'Goomba air ride of dooom
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//                .FrameCount = .FrameCount + 1
//                If .Type = 110 Or .Type = 143 Then .FrameCount = .FrameCount + 1
//                If .FrameCount >= 8 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 2 Then .Frame = 0
//                End If
//            ElseIf .Type = 104 Then  ' Blaarg eyes
//                .Life = .Life + 2
//                If .Life <= 30 Then
//                    .Location.SpeedY = -2.8
//                ElseIf .Life <= 40 Then
//                    .Location.SpeedY = 0.5
//                ElseIf .Life <= 80 Then
//                    .Location.SpeedY = 0
//                ElseIf .Life <= 100 Then
//                    .Location.SpeedY = 2
//                Else
//                    .Life = 0
//                End If
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 16 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 2 Then .Frame = 0
//                End If
//            ElseIf .Type = 61 Then    'Beack Koopa
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 15 Then
//                    .FrameCount = 0
//                    .Frame = .Frame - 1
//                ElseIf .FrameCount = 8 Then
//                    .Frame = .Frame + 1
//                End If
//            ElseIf .Type = 8 Or .Type = 9 Or .Type = 15 Or .Type = 16 Or .Type = 19 Or .Type = 27 Or .Type = 146 Or .Type = 28 Or .Type = 29 Or .Type = 32 Or .Type = 36 Or .Type = 47 Or .Type = 60 Or .Type = 95 Or .Type = 96 Or .Type = 109 Then    'Flying turtle shell / Bullet bill /hard thing
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//            ElseIf .Type = 26 Or .Type = 101 Or .Type = 102 Then ' Goombas shoes
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//                If .Location.SpeedX > 0 Then
//                    .Frame = 0 + SpecialFrame(1)
//                Else
//                    .Frame = 2 + SpecialFrame(1)
//                End If
//            ElseIf .Type = 10 Or .Type = 131 Then   'SMW / SMB3 Puff of smoke
//                .Location.X = .Location.X + .Location.SpeedX
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 3 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 4 Then .Life = 0
//                End If
//            ElseIf .Type = 147 Then   'SMB2 Puff of smoke
//                .Location.X = .Location.X + .Location.SpeedX
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 6 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 4 Then
//                        .Life = 0
//                    End If
//                End If
//            ElseIf .Type = 132 Then 'stomp stars
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 3 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 2 Then
//                        .Life = 0
//                        NewEffect 133, .Location
//                    End If
//                End If
//            ElseIf .Type = 133 Then 'stomp stars
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 1 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 4 Then
//                        .Frame = 0
//                    End If
//                End If
//            ElseIf .Type = 73 Then ' Tail whack
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 2 Then
//                    'If .Frame = 0 Then
//                        '.Frame = 2
//                    'ElseIf .Frame = 2 Then
//                        '.Frame = 1
//                    'ElseIf .Frame = 1 Then
//                        '.Frame = 3
//                    'Else
//                    .Frame = .Frame + 1
//                    If .Frame > 3 Then
//                        .Frame = 0
//                        .Life = 0
//                    End If
//                    .FrameCount = 0
//                End If
//            ElseIf .Type = 75 Then ' Whack
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 4 Then
//                    .Frame = .Frame + 1
//                    If .Frame > 1 Then
//                        .Life = 0
//                    End If
//                    .FrameCount = 0
//                End If
//            ElseIf .Type = 76 Then
//                .Location.X = .Location.X + .Location.SpeedX
//                .Location.Y = .Location.Y + .Location.SpeedY
//            ElseIf .Type = 81 Or .Type = 123 Or .Type = 124 Then ' P Switch
//                If .Life = 1 Then
//                    .Location.X = .Location.X + .Location.Width / 2 - EffectWidth(10) / 2
//                    .Location.Y = .Location.Y + .Location.Height / 2 - EffectHeight(10) / 2
//                    NewEffect 10, .Location
//                End If
//            ElseIf .Type = 74 Then 'Slide Smoke
//                .FrameCount = .FrameCount + 1
//                .Location.Y = .Location.Y - 0.1
//                If .FrameCount >= 4 Then
//                    .Frame = .Frame + 1
//                    .FrameCount = 0
//                    If .Frame > 2 Then .Life = 0
//                End If
//            ElseIf .Type = 63 Then 'Zelda Smoke
//                .Location.X = .Location.X + .Location.SpeedX
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 4 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 4 Then .Life = 0
//                End If
//            ElseIf .Type = 11 Then 'Coin out of block effect
//                If .Life = 1 Then
//                    CoinCount = CoinCount + 1
//                    If CoinCount > 13 Then CoinCount = 10
//                    MoreScore CoinCount, .Location
//                End If
//                If .Life <= 2 Then
//                    .Frame = 6
//                    .Location.SpeedY = .Location.SpeedY = 0
//                ElseIf .Life <= 4 Then
//                    .Frame = 5
//                    .Location.SpeedY = .Location.SpeedY = 0
//                ElseIf .Life <= 6 Then
//                    .Frame = 4
//                    .Location.SpeedY = .Location.SpeedY = 0
//                Else
//                    .Location.SpeedY = .Location.SpeedY + 0.4
//                    .FrameCount = .FrameCount + 1
//                    If .FrameCount >= 3 Then
//                        .FrameCount = 0
//                        .Frame = .Frame + 1
//                        If .Frame >= 4 Then .Frame = 0
//                    End If
//                End If
//            ElseIf .Type = 12 Then ' Big Fireball Tail
//                '.Location.SpeedX = 0
//                '.Location.SpeedY = 0
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 4 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                End If
//            ElseIf .Type = 78 Then ' Coin
//                .Location.SpeedX = 0
//                .Location.SpeedY = 0
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 5 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 3 Then .Life = 0
//                End If
//            ElseIf .Type = 82 Then 'Spinning block
//                .Frame = SpecialFrame(3)
//                If .Life < 10 Then
//                    tempBool = False
//                    For B = 1 To numPlayers
//                        If Player(B).Dead = False And Player(B).TimeToLive = 0 Then
//                            If CheckCollision(.Location, Player(B).Location) Then
//                                tempBool = True
//                                Exit For
//                            End If
//                        End If
//                    Next B
//                    'tempBool = True
//                    If tempBool = False Then
//                        .Life = 0
//                        .Frame = 3
//                        Block(.NewNpc).Hidden = False
//                    Else
//                        .Life = 10
//                    End If
//                End If
//            ElseIf .Type = 80 Then 'Twinkle
            if(e.Type == 80) {
//                .FrameCount = .FrameCount + 1
                e.FrameCount += 1;
//                If .FrameCount >= 8 Then
                if(e.FrameCount >= 8) {
//                    .FrameCount = 0
                    e.FrameCount = 0;
//                    .Frame = .Frame + 1
                    e.Frame += 1;
//                    If .Frame = 3 Then .Life = 0
                    if(e.Frame == 3)
                        e.Life = 0;
//                End If
                }
//            ElseIf .Type = 77 Or .Type = 139 Then ' Small Fireball Tail
            }
//                .Location.X = .Location.X + Rnd * 2 - 1
//                .Location.Y = .Location.Y + Rnd * 2 - 1
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 4 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    If .Frame = 3 Or .Frame = 6 Or .Frame = 9 Or .Frame = 12 Or .Frame = 15 Then .Life = 0
//                End If
//            ElseIf .Type = 13 Then ' Big Fireball Tail
//                .Location.SpeedX = 0
//                .Location.SpeedY = 0
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 6 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    .FrameCount = 0
//                End If
//            ElseIf .Type = 14 Then ' Dead Big Koopa
//                .Location.SpeedX = 0
//                .Location.SpeedY = 0
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 2 Then
//                    .FrameCount = 0
//                    .Frame = .Frame + 1
//                    .FrameCount = 0
//                    If .Frame >= 4 Then .Frame = 0
//                End If
//            ElseIf .Type = 70 Then 'SMB3 Bomb Part 1
//                If .FrameCount = 0 Then
//                    NewEffect 71, .Location, CSng(.Frame)
//                    .Frame = .Frame + 1
//                    If .Frame >= 4 Then .Frame = 0
//                ElseIf .FrameCount >= 6 Then
//                    .FrameCount = -1
//                End If
//                .FrameCount = .FrameCount + 1
//            ElseIf .Type = 71 Or .Type = 148 Then 'SMB3 Bomb Part 2
//                .FrameCount = .FrameCount + 1
//                    If .FrameCount >= 4 Then
//                        .FrameCount = 0
//                        .Frame = .Frame + 1
//                        If .Frame >= 4 Then .Frame = 0
//                    End If
//                If .Type = 148 And Rnd * 10 > 8 Then
//                    NewEffect 77, .Location, 3
//                    Effect(numEffects).Location.SpeedX = Rnd * 3 - 1.5
//                    Effect(numEffects).Location.SpeedY = Rnd * 3 - 1.5
//                End If
//            ElseIf .Type = 125 Then 'POW Block
//                .FrameCount = .FrameCount + 1
//                    If .FrameCount >= 4 Then
//                        .FrameCount = 0
//                        .Frame = .Frame + 1
//                        If .Frame >= 4 Then
//                            .Life = 0
//                            .Frame = 3
//                        End If
//                    End If
//            ElseIf .Type = 54 Or .Type = 55 Or .Type = 59 Or .Type = 103 Then 'door
//                .FrameCount = .FrameCount + 1
//                If .FrameCount > 60 Then
//                    .Life = 0
//                ElseIf .FrameCount > 55 Then
//                    .Frame = 0
//                ElseIf .FrameCount > 50 Then
//                    .Frame = 1
//                ElseIf .FrameCount > 45 Then
//                    .Frame = 2
//                ElseIf .FrameCount > 40 Then
//                    .Frame = 3
//                ElseIf .FrameCount > 20 Then
//                    .Frame = 4
//                ElseIf .FrameCount > 15 Then
//                    .Frame = 3
//                ElseIf .FrameCount > 10 Then
//                    .Frame = 2
//                ElseIf .FrameCount > 5 Then
//                    .Frame = 1
//                End If
//            ElseIf .Type = 15 Or .Type = 16 Or .Type = 25 Or .Type = 48 Or .Type = 49 Or .Type = 50 Or .Type = 68 Or .Type = 72 Or .Type = 89 Or .Type = 90 Or .Type = 91 Or .Type = 92 Or .Type = 93 Or .Type = 94 Or .Type = 98 Or .Type = 99 Or .Type = 105 Or .Type = 138 Or .Type = 106 Or .Type = 141 Then      ' Bullet Bill / Hammer Bro
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//            ElseIf .Type = 128 Then
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//                .FrameCount = .FrameCount + 1
//                .Frame = 5
//                If .FrameCount >= 16 Then
//                    .FrameCount = 0
//                    .Frame = 5
//                ElseIf .FrameCount > 8 Then
//                    .Frame = 4
//                End If
//            ElseIf .Type = 17 Or .Type = 18 Or .Type = 20 Or .Type = 24 Or (.Type >= 64 And .Type <= 67) Or .Type = 83 Then   'Shy guy free falling
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 8 Then
//                    .FrameCount = 0
//                    If .Frame = 4 Then
//                        .Frame = 5
//                    ElseIf .Frame = 5 Then .Frame = 4
//                    ElseIf .Frame = 6 Then .Frame = 7
//                    Else
//                        .Frame = 6
//                    End If
//                End If
//            ElseIf .Type = 85 Or .Type = 86 Or .Type = 87 Or .Type = 88 Or .Type = 97 Or .Type = 115 Or .Type = 122 Or .Type = 116 Or .Type = 118 Or .Type = 119 Or .Type = 120 Or .Type = 121 Or .Type = 137 Then  'Rex / mega mole / smw goomba free falling
//                .Location.SpeedY = .Location.SpeedY + 0.5
//                If .Location.SpeedY >= 10 Then .Location.SpeedY = 10
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 8 Then
//                    .FrameCount = 0
//                    If .Frame = 0 Then
//                        .Frame = 1
//                    ElseIf .Frame = 1 Then .Frame = 0
//                    ElseIf .Frame = 2 Then .Frame = 3
//                    Else
//                        .Frame = 2
//                    End If
//                End If
//            ElseIf .Type = 56 Then 'Egg
//                If .NewNpc = 0 And .FrameCount < 19 Then
//                    .FrameCount = 19
//                End If
//                .FrameCount = .FrameCount + 1
//                If .FrameCount = 10 Then
//                    .Frame = .Frame + 1
//                ElseIf .FrameCount = 20 Then
//                    .Frame = 2
//                    NewEffect 57, .Location
//                ElseIf .FrameCount = 30 Then
//                    .Life = 0
//                    If LevelEditor = False And .NewNpc <> 96 Then
//                        If NPCIsYoshi(.NewNpc) Then
//                            NewEffect 58, .Location, 1, CSng(.NewNpc)
//                        ElseIf .NewNpc > 0 Then
//                            numNPCs = numNPCs + 1
//                            NPC(numNPCs).Location = .Location
//                            With NPC(numNPCs)
//                                .Active = True
//                                .TimeLeft = 100
//                                .Direction = 0
//                                .Type = Effect(A).NewNpc
//                                .Location.Height = NPCHeight(.Type)
//                                .Location.Width = NPCWidth(.Type)
//                                .Location.Y = .Location.Y + 32 - .Location.Height
//                                .Location.X = .Location.X - .Location.Width / 2 + 16
//                                If .Type = 34 Then .Location.SpeedY = -6
//                                CheckSectionNPC (numNPCs)
//                            End With
//                        End If
//                    End If
//                End If
//            ElseIf .Type = 107 Then
//                .FrameCount = .FrameCount + 1
//                If .FrameCount >= 4 Then
//                    .Frame = .Frame + 1
//                    .FrameCount = 0
//                End If
//                If .Frame >= 3 Then .Life = 0
//            ElseIf .Type = 58 Then 'yoshi grow
//                .FrameCount = .FrameCount + 1
//                If .FrameCount < 10 Then
//                    .Frame = 0
//                ElseIf .FrameCount < 20 Then
//                    .Frame = 1
//                ElseIf .FrameCount < 30 Then
//                    .Frame = 0
//                ElseIf .FrameCount < 40 Then
//                    .Frame = 1
//                ElseIf .FrameCount < 50 Then
//                    .Frame = 0
//                ElseIf .FrameCount < 60 Then
//                    .Frame = 1
//                Else
//                    .Frame = 1
//                    .Life = 0
//                    numNPCs = numNPCs + 1
//                    NPC(numNPCs).Location = .Location
//                    With NPC(numNPCs)
//                        .Active = True
//                        .TimeLeft = 100
//                        .Direction = 1
//                        .Type = Effect(A).NewNpc
//                        .Location.Height = NPCHeight(.Type)
//                        .Location.Width = NPCWidth(.Type)
//                        CheckSectionNPC (numNPCs)
//                    End With
//                End If
//                If .NewNpc = 98 Then
//                    .Frame = .Frame + 2
//                ElseIf .NewNpc = 99 Then
//                    .Frame = .Frame + 4
//                ElseIf .NewNpc = 100 Then
//                    .Frame = .Frame + 6
//                ElseIf .NewNpc = 148 Then
//                    .Frame = .Frame + 8
//                ElseIf .NewNpc = 149 Then
//                    .Frame = .Frame + 10
//                ElseIf .NewNpc = 150 Then
//                    .Frame = .Frame + 12
//                ElseIf .NewNpc = 228 Then
//                    .Frame = .Frame + 14
//                End If
//            ElseIf .Type = 79 Then
//                .Location.SpeedY = .Location.SpeedY * 0.97
//            End If
//        End With
        }
//    Next A
    }
//    For A = numEffects To 1 Step -1
    for(int A = numEffects; A >= 1; --A)
    {
//        If Effect(A).Life <= 0 Then KillEffect A
        if(Effect[A].Life <= 0)
            KillEffect(A);
//    Next A
    }
}

void NewEffect(int A, Location_t Location, float Direction, int NewNpc, bool Shadow)
{
// this sub creates effects
// please reference the /graphics/effect folder to see what the effects are
// A is the effect type

//    Dim B As Integer
    int B;
//    Dim tempBool As Boolean
    bool tempBool;
//    Dim tempDoub As Double
    double tempDoub;

//    If numEffects >= maxEffects - 4 Then Exit Sub
    if(numEffects >= maxEffects - 4)
        return;

//    If A = 1 Or A = 21 Or A = 30 Or A = 51 Or A = 100 Or A = 135 Then  'Block break effect
//        For B = 1 To 4
//            numEffects = numEffects + 1
//            With Effect(numEffects)
//                .Shadow = Shadow
//                .Location.Width = EffectWidth(A)
//                .Location.Height = EffectHeight(A)
//                .Type = A
//                .Location.SpeedX = 3
//                .Life = 200
//                If B = 1 Or B = 3 Then .Location.SpeedX = -.Location.SpeedX
//                If B = 1 Or B = 2 Then
//                    .Location.SpeedY = -11
//                Else
//                    .Location.SpeedY = -7
//                End If
//                .Location.SpeedX = .Location.SpeedX + Rnd * 2 - 1
//                .Location.SpeedY = .Location.SpeedY + Rnd * 4 - 2
//                If B = 1 Then
//                    .Location.X = Location.X
//                    .Location.Y = Location.Y
//                ElseIf B = 2 Then
//                    .Location.X = Location.X + Location.Width / 2
//                    .Location.Y = Location.Y
//                ElseIf B = 3 Then
//                    .Location.X = Location.X
//                    .Location.Y = Location.Y + Location.Height / 2
//                Else
//                    .Location.X = Location.X + Location.Width / 2
//                    .Location.Y = Location.Y + Location.Height / 2
//                End If
//             End With
//        Next B
//    ElseIf A = 140 Then 'larry shell
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .NewNpc = NewNpc
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.SpeedX = 0
//            .Location.SpeedY = 0
//            .Frame = 0
//            .FrameCount = 0
//            .Life = 160
//            .Type = A
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height - .Location.Height
//        End With
//    ElseIf A = 104 Then  'Blaarg eyes
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .NewNpc = NewNpc
//            .Shadow = Shadow
//            If .NewNpc = 96 Then .NewNpc = 0
//            If Direction = -1 Then
//                .Location.X = Location.X + Location.Width / 2 + 16 + 48 * Direction
//            Else
//                .Location.X = Location.X + Location.Width / 2 + 16 '+ 48 * Direction
//            End If

//            .Location.Y = Location.Y
//            .Location.Width = 32
//            .Location.Height = 32
//            .Location.SpeedX = 0
//            .Location.SpeedY = 0
//            .Frame = 0
//            .FrameCount = 0
//            .Life = 10
//            .Type = A
//        End With
//    ElseIf A = 56 Or A = 58 Then 'Egg break / Yoshi grow
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .NewNpc = NewNpc
//            If .NewNpc = 96 Then .NewNpc = 0
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.Width = 32
//            .Location.Height = 32
//            .Location.SpeedX = 0
//            .Location.SpeedY = 0
//            .Frame = 0
//            .FrameCount = 0
//            .Life = 100
//            .Type = A
//            If A = 56 Then
//                If .NewNpc <> 0 And .NewNpc <> 96 Then
//                    PlaySound 51
//                Else
//                    PlaySound 36
//                End If
//            ElseIf A = 58 Then
//                PlaySound 48
//            End If
//        End With
//    ElseIf A = 136 Then 'Roto Disk
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedX = Location.SpeedX
//            .Location.SpeedY = Location.SpeedY
//            .Location.Width = Location.Width
//            .Location.Height = Location.Width
//            .Frame = 0
//            .FrameCount = 0
//            .Life = 10
//            .Type = A
//        End With

//    ElseIf A = 125 Then 'pow
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .NewNpc = NewNpc
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height - .Location.Height
//            .Location.SpeedX = 0
//            .Location.SpeedY = 0
//            .Frame = 0
//            .FrameCount = 0
//            .Life = 100
//            .Type = A
//        End With
//    ElseIf A = 107 Then 'Metroid Block
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .NewNpc = NewNpc
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.Width = 32
//            .Location.Height = 32
//            .Location.SpeedX = 0
//            .Location.SpeedY = 0
//            .Frame = 0
//            .FrameCount = 0
//            .Life = 100
//            .Type = A
//        End With
//    ElseIf A = 57 Then  'Egg shells
//        For B = 1 To 4
//            numEffects = numEffects + 1
//            With Effect(numEffects)
//                .Shadow = Shadow
//                .Location.Width = EffectWidth(A)
//                .Location.Height = EffectHeight(A)
//                .Type = A
//                .Location.SpeedX = 2
//                .Life = 200
//                If B = 1 Or B = 2 Then
//                    .Location.SpeedY = -11
//                Else
//                    .Location.SpeedY = -7
//                    .Location.SpeedX = 1.5
//                End If
//                If B = 1 Or B = 3 Then .Location.SpeedX = -.Location.SpeedX
//                .Location.SpeedX = .Location.SpeedX + (Rnd * 0.5 - 0.25)
//                .Location.SpeedY = .Location.SpeedY + (Rnd * 1 - 0.5)
//                If B = 1 Then
//                    .Frame = 0
//                ElseIf B = 2 Then .Frame = 1
//                ElseIf B = 3 Then .Frame = 3
//                Else
//                    .Frame = 2
//                End If

//                '.Location.SpeedX = .Location.SpeedX + Rnd * 2 - 1
//                '.Location.SpeedY = .Location.SpeedY + Rnd * 4 - 2
//                If B = 1 Then
//                    .Location.X = Location.X
//                    .Location.Y = Location.Y
//                ElseIf B = 2 Then
//                    .Location.X = Location.X + Location.Width / 2
//                    .Location.Y = Location.Y
//                ElseIf B = 3 Then
//                    .Location.X = Location.X
//                    .Location.Y = Location.Y + Location.Height / 2
//                Else
//                    .Location.X = Location.X + Location.Width / 2
//                    .Location.Y = Location.Y + Location.Height / 2
//                End If
//             End With
//        Next B
//    ElseIf A = 2 Or A = 6 Or A = 23 Or A = 35 Or A = 37 Or A = 39 Or A = 41 Or A = 43 Or A = 45 Or A = 52 Or A = 62 Or A = 84 Or A = 126 Then      'Goomba smash effect
//        PlaySound 2 'Stomp sound
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.Width = 32
//            .Location.Height = 34
//            .Location.SpeedX = 0
//            .Location.SpeedY = 0
//            .Frame = 0
//            .Life = 20
//            .Type = A
//            If A = 45 Then
//                .Location.Height = 46
//                .Location.Width = 48
//            End If
//            If A = 84 Then
//                If Direction = 1 Then .Frame = 1
//            End If
//        End With
//    ElseIf A = 81 Or A = 123 Or A = 124 Then  'P Switch
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height - .Location.Height
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 113 Or A = 114 Then  'Water Bubble / Splash
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectWidth(A)
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Shadow = Shadow
//            tempBool = False
//            If A = 114 Then 'Change height for the background
//                For B = 1 To numBackground
//                    If Background(B).Type = 82 Or Background(B).Type = 26 Or Background(B).Type = 65 Or Background(B).Type = 159 Or Background(B).Type = 166 Or Background(B).Type = 168 Then
//                        If CheckCollision(.Location, Background(B).Location) = True Then
//                            If Background(B).Type = 82 Or Background(B).Type = 159 Then .Location.Y = Background(B).Location.Y - .Location.Height + 12
//                            If Background(B).Type = 26 Then .Location.Y = Background(B).Location.Y - .Location.Height + 6
//                            If Background(B).Type = 168 Then .Location.Y = Background(B).Location.Y - .Location.Height + 8
//                            If Background(B).Type = 166 Then .Location.Y = Background(B).Location.Y - .Location.Height + 10
//                            If Background(B).Type = 65 Then .Location.Y = Background(B).Location.Y - .Location.Height + 16
//                            tempBool = True
//                            Exit For
//                        End If
//                    End If
//                Next B
//            End If
//            .Frame = 0
//            .Life = 300
//            .NewNpc = NewNpc
//            .Type = A
//        End With
//        If tempBool = False And A = 114 Then numEffects = numEffects - 1
//    ElseIf A = 109 Then  'Spike Top
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = 32
//            .Location.Height = 32
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = -12
//            .Location.SpeedX = Location.SpeedX
//            .Frame = Direction
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 108 Then  'Metroid
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = 64
//            .Location.Height = 64
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height / 2 - .Location.Height / 2
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 200
//            .Type = A
//        End With
//    ElseIf A = 82 Then  'Block Spin
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .NewNpc = NewNpc
//            .Location.Width = 32
//            .Location.Height = 32
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 300
//            .Type = A
//        End With
//    ElseIf A = 3 Or A = 5 Or A = 129 Or A = 130 Or A = 134 Then  'Mario & Luigi died effect
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X - .Location.Width * 0.5 + Location.Width * 0.5
//            .Location.Y = Location.Y - .Location.Height * 0.5 + Location.Height * 0.5
//            .Location.SpeedY = -11
//            .Location.SpeedX = 0
//            .Frame = 0
//            If A = 134 Then
//                If Direction = 1 Then .Frame = 1
//            End If
//            .Life = 150
//            .Type = A
//        End With
//    ElseIf A = 79 Then 'Score
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Type = A
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(.Type)
//            .Location.Height = EffectHeight(.Type)
//            .Location.X = Location.X - .Location.Width * 0.5 + Location.Width * 0.5
//            .Location.Y = Location.Y - .Location.Height * 0.5 + Location.Height * 0.5
//            .Location.X = .Location.X + Rnd * 32 - 16
//            .Location.Y = .Location.Y + Rnd * 32 - 16
//            .Location.SpeedY = -2
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 60
//        End With
//    ElseIf A = 76 Then 'SMW Smashed
//        For B = 1 To 4
//            If numEffects < maxEffects Then
//            numEffects = numEffects + 1
//                With Effect(numEffects)
//                    .Shadow = Shadow
//                    .Type = A
//                    .Location.Width = EffectWidth(A)
//                    .Location.Height = EffectHeight(A)
//                    .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//                    .Location.Y = Location.Y + Location.Height / 2 - .Location.Height / 2
//                    .Location.SpeedY = -0
//                    .Location.SpeedX = 0
//                    .Life = 15

//                    .Location.SpeedX = 3 * 0.8
//                    .Location.SpeedY = 1.5 * 0.8

//                    If B = 1 Or B = 2 Then .Location.SpeedY = -.Location.SpeedY
//                    If B = 1 Or B = 3 Then .Location.SpeedX = -.Location.SpeedX

//                    .Frame = 0
//                End With
//            End If
//        Next B
//    ElseIf A = 133 Then 'stomp star part 2
//        For B = 1 To 4
//            If numEffects < maxEffects Then
//            numEffects = numEffects + 1
//                With Effect(numEffects)
//                    .Shadow = Shadow
//                    .Type = A
//                    .Location.Width = EffectWidth(A)
//                    .Location.Height = EffectHeight(A)
//                    .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//                    .Location.Y = Location.Y + Location.Height / 2 - .Location.Height / 2
//                    .Life = 8
//                    .Location.SpeedX = 2
//                    .Location.SpeedY = 2
//                    If B = 1 Or B = 2 Then .Location.SpeedY = -.Location.SpeedY
//                    If B = 1 Or B = 3 Then .Location.SpeedX = -.Location.SpeedX
//                    .Location.Y = .Location.Y + .Location.SpeedY * 6
//                    .Location.X = .Location.X + .Location.SpeedX * 6
//                    .Frame = 0
//                End With
//            End If
//        Next B
//    ElseIf A = 70 Then 'SMB3 Bomb Part 1
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = 16
//            .Location.Height = 16
//            .Location.X = Location.X - .Location.Width * 0.5 + Location.Width * 0.5
//            .Location.Y = Location.Y - .Location.Height * 0.5 + Location.Height * 0.5
//            .Location.SpeedY = -0
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 46
//            .Type = A
//        End With

//    ElseIf A = 148 Then 'Heart Bomb
//        For B = 1 To 6
//            If numEffects < maxEffects Then
//            numEffects = numEffects + 1
//                With Effect(numEffects)
//                    .Shadow = Shadow
//                    .Location.Width = EffectWidth(A)
//                    .Location.Height = EffectHeight(A)
//                    .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//                    .Location.Y = Location.Y + Location.Height / 2 - .Location.Height / 2
//                    .Location.SpeedY = -0
//                    .Location.SpeedX = 0
//                    .Life = 10
//                    If B = 1 Or B = 3 Or B = 4 Or B = 6 Then
//                        .Location.SpeedY = 3.5
//                        .Location.SpeedX = 2
//                    Else
//                        .Life = 11
//                        .Location.SpeedY = 0
//                        .Location.SpeedX = 4
//                    End If
//                    If B <= 3 Then .Location.SpeedX = -.Location.SpeedX
//                    If B = 1 Or B = 6 Then .Location.SpeedY = -.Location.SpeedY
//                    If Direction Mod 2 = 0 Then
//                        tempDoub = .Location.SpeedX
//                        .Location.SpeedX = .Location.SpeedY
//                        .Location.SpeedY = tempDoub
//                    End If
//                    .Location.SpeedX = .Location.SpeedX * 0.5
//                    .Location.SpeedY = .Location.SpeedY * 0.5

//                    .Location.X = .Location.X + .Location.SpeedX * 3
//                    .Location.Y = .Location.Y + .Location.SpeedY * 3

//                    .Frame = Int(Rnd * 4)
//                    .Type = A
//                End With
//            End If
//        Next B


//    ElseIf A = 71 Then 'SMB3 Bomb Part 2
//        For B = 1 To 6
//            If numEffects < maxEffects Then
//            numEffects = numEffects + 1
//                With Effect(numEffects)
//                    .Shadow = Shadow
//                    .Location.Width = 16
//                    .Location.Height = 16
//                    .Location.X = Location.X
//                    .Location.Y = Location.Y
//                    .Location.SpeedY = -0
//                    .Location.SpeedX = 0
//                    .Life = 13
//                    If B = 1 Or B = 3 Or B = 4 Or B = 6 Then
//                        .Location.SpeedY = 3
//                        .Location.SpeedX = 2
//                        .Life = 14
//                    Else
//                        .Location.SpeedY = 0
//                        .Location.SpeedX = 4
//                    End If
//                    If B <= 3 Then .Location.SpeedX = -.Location.SpeedX
//                    If B = 1 Or B = 6 Then .Location.SpeedY = -.Location.SpeedY
//                    If Direction Mod 2 = 0 Then
//                        tempDoub = .Location.SpeedX
//                        .Location.SpeedX = .Location.SpeedY
//                        .Location.SpeedY = tempDoub
//                    End If
//                    .Location.SpeedX = .Location.SpeedX * 1.5
//                    .Location.SpeedY = .Location.SpeedY * 1.5
//                    .Frame = Direction
//                    .Type = A
//                End With
//            End If
//        Next B
//    ElseIf A = 54 Or A = 55 Or A = 59 Or A = 103 Then 'Door Effect
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = Location.Width
//            .Location.Height = Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 150
//            .Type = A
//        End With
//    ElseIf A = 4 Or A = 7 Or A = 8 Or A = 9 Or A = 19 Or A = 22 Or A = 26 Or A = 101 Or A = 102 Or A = 27 Or A = 146 Or A = 28 Or A = 29 Or A = 31 Or A = 32 Or A = 145 Or A = 33 Or A = 34 Or A = 36 Or A = 38 Or A = 40 Or A = 42 Or A = 44 Or A = 46 Or A = 47 Or A = 53 Or A = 60 Or A = 9 Or A = 6 Or A = 95 Or A = 96 Or A = 110 Or A = 117 Or A = 121 Or A = 127 Or A = 142 Then  ' Flying goomba / turtle shell / hard thing shell
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height - .Location.Height
//            If Location.SpeedY <> -5.1 Then
//                .Location.SpeedY = -11
//                .Location.SpeedX = Location.SpeedX
//            Else
//                .Location.SpeedY = -5.1
//                .Location.SpeedX = Location.SpeedX * 0.6
//            End If
//            .Frame = 0
//            .Life = 150
//            .Type = A
//            If .Type = 29 And Direction = -1 Then .Frame = 1
//            If (.Type = 27 Or .Type = 146) And Direction = 1 Then .Frame = 2
//            If .Type = 36 And Direction = 1 Then .Frame = 1
//        End With
//    ElseIf A = 78 Then ' Coins
//        For B = 1 To 4
//            numEffects = numEffects + 1
//            With Effect(numEffects)
//                .Shadow = Shadow
//                .Location.Width = EffectWidth(A)
//                .Location.Height = EffectHeight(A)
//                .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//                .Location.Y = Location.Y + Location.Height / 2 - .Location.Height / 2
//                .Location.SpeedY = 0
//                .Location.SpeedX = 0
//                If B = 1 Then .Location.X = .Location.X - 10
//                If B = 3 Then .Location.X = .Location.X + 10
//                If B = 2 Then .Location.Y = .Location.Y + 16
//                If B = 4 Then .Location.Y = .Location.Y - 16
//                .Frame = 0 - B
//                .Life = 20 * B
//                .Type = A
//            End With
//        Next B
//    ElseIf A = 10 Or A = 73 Or A = 74 Or A = 75 Or A = 131 Or A = 132 Or A = 147 Then ' Puff of smoke
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            If A = 132 Then
//                .Location.Y = .Location.Y + Rnd * 16 - 8
//                .Location.X = .Location.X + Rnd * 16 - 8
//            End If
//            .Frame = 0
//            .Life = 12
//            .Type = A
//            If .Type = 147 Then .Life = 24
//            If A = 73 Or A = 75 Then
//                .Location.X = .Location.X + Rnd * 16 - 8
//                .Location.Y = .Location.Y + Rnd * 16 - 8
//            ElseIf A = 74 Then
//                .Location.X = .Location.X + Rnd * 4 - 2
//                .Location.Y = .Location.Y + Rnd * 4 - 2
//            End If
//        End With
//    ElseIf A = 144 Then 'bubble pop
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height / 2 - .Location.Height / 2
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 6
//            .Type = A
//        End With
//    ElseIf A = 63 Then ' Zelda Style Smoke
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = 48
//            .Location.Height = 48
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height / 2 - .Location.Height / 2
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 100
//            .Type = A
//        End With
//    ElseIf A = 11 Then ' Coin hit out of block
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = 32
//            .Location.Height = 32
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y - 32
//            .Location.SpeedY = -8
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 46
//            .Type = A
//        End With
//    ElseIf A = 12 Then ' Big Fireball Tail
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = 8
//            .Location.Height = 8
//            .Location.X = Location.X + 4 + Rnd * 12
//            .Location.Y = Location.Y + 40
//            '.Location.SpeedY = -8
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 12
//            .Type = A
//        End With
//    ElseIf A = 111 Then ' Glass Shatter
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = 16
//            .Location.Height = 16
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = -2 - Rnd * 10
//            .Location.SpeedX = Rnd * 8 - 4
//            .Frame = 0
//            If Int(Rnd * 2) = 1 Then .Frame = 7
//            .Frame = .Frame + Int(Rnd * 7)
//            .Life = 300
//            .Type = A
//        End With
//    ElseIf A = 112 Then ' Mother Brain
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = Location.Width
//            .Location.Height = Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Frame = 0
//            If Direction = 1 Then .Frame = 1
//            .Life = 360
//            .Type = A
//        End With
//    ElseIf A = 77 Or A = 139 Then ' Small Fireball Tail
//            numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - EffectWidth(A) / 2 + Rnd * 4 - 2
//            .Location.Y = Location.Y + Location.Height / 2 - EffectHeight(A) / 2 + Rnd * 4 - 2
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .Frame = 0
//            If Direction = 2 Then .Frame = 3
//            If Direction = 3 Then .Frame = 6
//            If Direction = 4 Then .Frame = 9
//            If Direction = 5 Then .Frame = 12
//            .Life = 60
//            .Type = A
//        End With
//    ElseIf A = 80 Then ' Twinkle
    if(A == 80) // Twinkle
    {
//        numEffects = numEffects + 1
        numEffects += 1;
//        With Effect(numEffects)
        {
            auto &e = Effect[numEffects];
            e.Shadow = Shadow;
            e.Location.Width = 16;
            e.Location.Height = 16;
            e.Location.X = Location.X + Location.Width / 2 - 4 + std::rand() % 4 - 2;
            e.Location.Y = Location.Y + Location.Height / 2 - 4 + std::rand() % 4 - 2;
            e.Location.SpeedY = 0;
            e.Location.SpeedX = 0;
            e.Frame = 0;
            e.Life = 60;
            e.Type = A;
//        End With
        }
//    ElseIf A = 13 Then ' Lava Splash
    }
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(13)
//            .Location.Height = EffectHeight(13)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + 24
//            .Location.SpeedY = -8
//            .Location.SpeedX = 0
//            .Frame = 0
//            .Life = 100
//            .Type = A
//        End With
//    ElseIf A = 14 Then ' Dead Big Koopa
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + 22
//            .Location.SpeedY = 0
//            .Location.SpeedX = 0
//            .NewNpc = NewNpc
//            .Frame = 0
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 15 Or A = 68 Then     ' Dead Bullet Bill
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = Location.Width
//            .Location.Height = Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = Location.SpeedY
//            .Location.SpeedX = -Location.SpeedX
//            If Direction = -1 Then
//                .Frame = 0
//            Else
//                .Frame = 1
//            End If
//            If A = 68 Then .Frame = 0
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 61 Then     ' Flying Beach Koopa
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = Location.Width
//            .Location.Height = Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = -11
//            .Location.SpeedX = -Location.SpeedX
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 48 Then    ' Dead toad
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = Location.Width
//            .Location.Height = Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = Location.SpeedY
//            .Location.SpeedX = Location.SpeedX
//            .Location.X = .Location.X + .Location.Width / 2 - 16
//            .Location.Y = .Location.Y + .Location.Height / 2 - 16
//            .Location.Width = 32
//            .Location.Height = 32
//            .Frame = 0
//            .Location.SpeedY = -8
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 16 Then    ' Dead Giant Bullet Bill
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = Location.Width
//            .Location.Height = Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = Location.SpeedY
//            .Location.SpeedX = Location.SpeedX
//            If Direction = -1 Then
//                .Frame = 0
//            Else
//                .Frame = 1
//            End If
//            If A = 48 Then .Location.SpeedY = -8
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 69 Then 'Bomb
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = 64
//            .Location.Height = 64
//            .Location.X = Location.X + Location.Width / 2 - 32
//            .Location.Y = Location.Y + Location.Height / 2 - 32
//            .Location.SpeedX = 0
//            .Location.SpeedY = 0
//            .Life = 60
//            .Type = A
//        End With
//    ElseIf A = 128 Then  'pokey
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = Location.Width
//            .Location.Height = Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = -11
//            .Location.SpeedX = Location.SpeedX
//            .Life = 120
//            .Type = A
//            .Frame = 5
//        End With

//    ElseIf A = 17 Or A = 18 Or A = 20 Or A = 24 Or (A >= 64 And A <= 67) Or A = 83 Or A = 85 Or A = 86 Or A = 87 Or A = 88 Or A = 97 Or A = 115 Or A = 122 Or A = 116 Or A = 118 Or A = 119 Or A = 120 Or A = 137 Then  'Shy guy / Star Thing /Red Jumping Fish
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height - .Location.Height
//            .Frame = 0
//            If A <> 24 And A <> 115 And A <> 116 Then
//                .Location.SpeedY = -11
//            Else
//                .Location.SpeedY = Location.SpeedY
//            End If
//            .Location.SpeedX = Location.SpeedX
//            If Location.SpeedY = 0.123 Then
//                .Location.SpeedY = 1
//                .Location.SpeedX = 0
//            End If
//            If Direction = -1 Then
//                If A = 85 Or A = 86 Or A = 87 Or A = 88 Or A = 97 Or A = 115 Or A = 116 Or A = 118 Or A = 119 Or A = 120 Or A = 122 Or A = 137 Then
//                    .Frame = 0
//                Else
//                    .Frame = 4
//                End If
//            Else
//                If A = 85 Or A = 86 Or A = 87 Or A = 88 Or A = 97 Or A = 115 Or A = 116 Or A = 118 Or A = 119 Or A = 120 Or A = 122 Or A = 137 Then
//                    .Frame = 2
//                Else
//                    .Frame = 6
//                End If
//            End If
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 90 Or A = 91 Or A = 92 Or A = 93 Or A = 94 Or A = 98 Or A = 99 Then   'Boo / thwomps
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = Location.Width
//            .Location.Height = Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = Location.SpeedY
//            .Location.SpeedX = -Location.SpeedX
//            If A = 91 Then
//                .Location.X = .Location.X + .Location.Width / 2
//                .Location.Y = .Location.Y + .Location.Height / 2
//                .Location.Width = EffectWidth(A)
//                .Location.Height = EffectHeight(A)
//                .Location.X = .Location.X - .Location.Width / 2
//                .Location.Y = .Location.Y - .Location.Height / 2
//            End If
//            If .Location.SpeedX <> 0 And .Location.SpeedX > -2 And .Location.SpeedX < 2 Then
//                .Location.SpeedX = 2 * -Direction
//            End If
//            If Direction = -1 Then
//                .Frame = 0
//            Else
//                .Frame = 2
//            End If
//            If A = 90 Or A = 98 Or A = 99 Then .Frame = 0
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 25 Or A = 49 Or A = 50 Or A = 72 Or A = 89 Or A = 105 Or A = 106 Or A = 138 Or A = 141 Then  'Hammer Bro
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow

//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height - .Location.Height

//            .Location.X = Location.X
//            .Location.Y = Location.Y

//            .Location.SpeedY = Location.SpeedY
//            .Location.SpeedX = -Location.SpeedX
//            If .Location.SpeedX <> 0 And .Location.SpeedX > -2 And .Location.SpeedX < 2 Then
//                .Location.SpeedX = 2 * -Direction
//            End If
//            If Direction = -1 Then
//                .Frame = 0
//            Else
//                .Frame = 1
//            End If
//            .Life = 120
//            .Type = A
//        End With
//    ElseIf A = 143 Then ' ludwig dead
//        numEffects = numEffects + 1
//        With Effect(numEffects)
//            .Shadow = Shadow
//            .Location.Width = EffectWidth(A)
//            .Location.Height = EffectHeight(A)
//            .Location.X = Location.X + Location.Width / 2 - .Location.Width / 2
//            .Location.Y = Location.Y + Location.Height - .Location.Height
//            .Location.X = Location.X
//            .Location.Y = Location.Y
//            .Location.SpeedY = -14
//            .Location.SpeedX = 3 * -Direction
//            .Life = 200
//            .Type = A
//            PlaySound 63
//        End With
//    End If
}

// Remove the effect
void KillEffect(int A)
{
    if(numEffects == 0)
        return;
//    Effect(A) = Effect(numEffects)
    Effect[A] = Effect[numEffects];
//    With Effect(numEffects)
    Effect_t &e = Effect[numEffects];
//        .Frame = 0
    e.Frame = 0;
//        .FrameCount = 0
    e.FrameCount = 0;
//        .Life = 0
    e.Life = 0;
//        .Type = 0
    e.Type = 0;
//    End With
//    numEffects = numEffects - 1
    numEffects -= 1;
}
