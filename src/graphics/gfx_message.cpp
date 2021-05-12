#include "globals.h"
#include "../graphics.h"

void DrawMessage()
{
    // based on Wohlstand's improved algorithm, but screen-size aware
    static std::string SuperText;
    static std::string tempText;
    SuperText = MessageText;
    int BoxY = 0;
    const int BoxY_Start = ScreenH/2 - 150;
    const int TextBoxW = 500;
    // Draw background all at once:
    // how many lines are there?
    // A is the proposed start of the next line
    int A;
    int B = 0; // start of current line
    int C = 0; // planned start of next line
    int D = 1; // n lines
    bool tempBool;

    for(A = 1; A <= int(SuperText.size()); A++)
    {
        if(SuperText[size_t(A) - 1] == ' ' || A == int(SuperText.size()))
        {
            if(A-B < 28)
                C = A;
            else {
                B = C;
                D ++;
            }
        }
    }
    frmMain.renderRect(ScreenW/2 - TextBoxW / 2 ,
                          BoxY_Start,
                          TextBoxW, D*16 + 20, 0.f, 0.f, 0.f, 1.f);
    frmMain.renderRect(ScreenW/2 - TextBoxW / 2 + 2,
                          BoxY_Start + 2,
                          TextBoxW - 4, D*16 + 20 - 4, 1.f, 1.f, 1.f, 1.f);
    frmMain.renderRect(ScreenW/2 - TextBoxW / 2 + 4,
                          BoxY_Start + 4,
                          TextBoxW - 8, D*16 + 20 - 8, 8.f/255.f, 96.f/255.f, 168.f/255.f, 1.f);
    tempBool = false; // multi-line
    BoxY = BoxY_Start + 10;
    do
    {
        // find last word break
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
        if(SuperText.length() == 0 && !tempBool)
        {
            SuperPrint(tempText,
                       4,
                       ScreenW/2 - TextBoxW / 2 + 12 + (27 * 9) - (tempText.length() * 9),
                       BoxY);
        }
        else
        {
            SuperPrint(tempText, 4, ScreenW/2 - TextBoxW / 2 + 12, BoxY);
        }
        BoxY += 16;
        tempBool = true;
    } while(!SuperText.empty());
}