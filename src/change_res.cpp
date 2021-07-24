/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "globals.h"
#include "change_res.h"
#include "load_gfx.h"

#ifdef __3DS__
void SetOrigRes() {}

void ChangeRes(int, int, int, int) {}

#else

void SetOrigRes()
{
    frmMain.setFullScreen(false);
    resChanged = false;
    SDL_SetWindowSize(frmMain.getWindow(), ScreenW, ScreenH);
#ifdef __EMSCRIPTEN__
    frmMain.eventResize();
#endif
    if(LoadingInProcess)
        UpdateLoad();
    if(!GameMenu && !MagicHand && !LevelEditor && !WorldEditor)
        showCursor(1);
}

void ChangeRes(int, int, int, int)
{
    frmMain.setFullScreen(true);
    if(LoadingInProcess)
        UpdateLoad();
}

#endif

//void SaveIt(int ScX, int ScY, int ScC, int ScF, std::string ScreenChanged)
//{

//}
