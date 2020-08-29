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
#include "../sound.h"
#include "../game_main.h"
#include "../pseudo_vb.h"


void DoCredits()
{
    int A = 0;
    if(GameMenu == true)
        return;

    for(A = 1; A <= numCredits; A++)
    {
        Credit[A].Location.Y -= 0.8;

        // Printing lines of credits
        if(Credit[A].Location.Y <= 600 && Credit[A].Location.Y + Credit[A].Location.Height >= 0)
        {
            SuperPrint(Credit[A].Text, 4, static_cast<float>(Credit[A].Location.X), static_cast<float>(Credit[A].Location.Y));
        }

        // Closing screen
        else if(A == numCredits && Credit[A].Location.Y + Credit[A].Location.Height < -100)
        {
            if(musicPlaying)
            {
                FadeOutMusic(11000);
                musicPlaying = false;
            }

            CreditChop += 0.4f;
            if(CreditChop >= 300)
            {
                CreditChop = 300;
                EndCredits = EndCredits + 1;
                if(EndCredits == 300)
                {
                    SetupCredits();
                    GameOutro = false;
                    GameMenu = true;
                }
            }
            else
                EndCredits = 0;
        }

        // Opening screen
        else if(CreditChop > 100 && Credit[numCredits].Location.Y + Credit[numCredits].Location.Height > 0)
        {
            CreditChop -= 0.02f;
            if(CreditChop < 100)
                CreditChop = 100;

            if(CreditChop < 250 && !musicPlaying)
            {
                if(bgMusic[0] <= 0) // Play default music if no music set in outro level
                {
                    musicName = "tmusic";
                    PlayMusic("tmusic", 2000);
                    musicPlaying = true;
                }
                else // Otherwise, play the music that set by level
                    StartMusic(0, 2000);

            }
        }
    }

    if(CreditChop <= 100 || EndCredits > 0)
    {
        for(A = 1; A <= 2; A++)
        {
            if(((getKeyState(vbKeyEscape) == KEY_PRESSED) | (getKeyState(vbKeySpace) == KEY_PRESSED) | (getKeyState(vbKeyReturn) == KEY_PRESSED)) != 0)
            {
                CreditChop = 300;
                EndCredits = 0;
                SetupCredits();
                GameMenu = true;
                GameOutro = false;
            }
        }
    }
}
