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
#include "gfx.h"
#include "frm_main.h"
#include <AppPath/app_path.h>
#include <fmt_format_ne.h>
#include <Logger/logger.h>
#include <SDL2/SDL_messagebox.h>

void GFX_t::loadImage(StdPicture &img, const std::string &path)
{
    pLogDebug("Loading texture %s...", path.c_str());
    img = frmMain.LoadPicture(path);

    if(!img.texture)
    {
        pLogWarning("Failed to load texture: %s...", path.c_str());
        m_loadErrors++;
    }

    m_loadedImages.push_back(&img);
}

GFX_t::GFX_t() noexcept = default;

bool GFX_t::load()
{
    std::string uiPath = AppPath + "graphics/ui/";

    loadImage(BMVs, uiPath + "BMVs.png");
    loadImage(BMWin, uiPath + "BMWin.png");
    For(i, 1, 3)
        loadImage(Boot[i], uiPath + fmt::format_ne("Boot{0}.png", i));

    For(i, 1, 5)
        loadImage(CharacterName[i], uiPath + fmt::format_ne("CharacterName{0}.png", i));

    loadImage(Chat, uiPath + "Chat.png");

    For(i, 0, 2)
        loadImage(Container[i], uiPath + fmt::format_ne("Container{0}.png", i));

    For(i, 1, 3)
        loadImage(ECursor[i], uiPath + fmt::format_ne("ECursor{0}.png", i));

    For(i, 0, 9)
        loadImage(Font1[i], uiPath + fmt::format_ne("Font1_{0}.png", i));

    For(i, 1, 3)
        loadImage(Font2[i], uiPath + fmt::format_ne("Font2_{0}.png", i));

    loadImage(Font2S, uiPath + "Font2S.png");

    For(i, 1, 2)
        loadImage(Heart[i], uiPath + fmt::format_ne("Heart{0}.png", i));

    For(i, 0, 8)
        loadImage(Interface[i], uiPath + fmt::format_ne("Interface{0}.png", i));

    loadImage(LoadCoin, uiPath + "LoadCoin.png");
    loadImage(Loader, uiPath + "Loader.png");

    For(i, 0, 3)
        loadImage(MCursor[i], uiPath + fmt::format_ne("MCursor{0}.png", i));

    For(i, 1, 4)
        loadImage(MenuGFX[i], uiPath + fmt::format_ne("MenuGFX{0}.png", i));

    loadImage(Mount[2], uiPath + "Mount.png");

    For(i, 0, 7)
        loadImage(nCursor[i], uiPath + fmt::format_ne("nCursor{0}.png", i));

    loadImage(TextBox, uiPath + "TextBox.png");

    For(i, 1, 2)
        loadImage(Tongue[i], uiPath + fmt::format_ne("Tongue{0}.png", i));

    loadImage(Warp, uiPath + "Warp.png");

    loadImage(YoshiWings, uiPath + "YoshiWings.png");

#ifdef __ANDROID__
    // Loading a touch-screen buttons from assets
    loadImage(touch[BUTTON_START], "buttons/Start.png");
    loadImage(touch[BUTTON_LEFT], "buttons/Left.png");
    loadImage(touch[BUTTON_LEFT_CHAR], "buttons/Left_char.png");
    loadImage(touch[BUTTON_RIGHT], "buttons/Right.png");
    loadImage(touch[BUTTON_RIGHT_CHAR], "buttons/Right_char.png");
    loadImage(touch[BUTTON_UP], "buttons/Up.png");
    loadImage(touch[BUTTON_DOWN], "buttons/Down.png");
    loadImage(touch[BUTTON_UPLEFT], "buttons/UpLeft.png");
    loadImage(touch[BUTTON_UPRIGHT], "buttons/UpRight.png");
    loadImage(touch[BUTTON_DOWNLEFT], "buttons/DownLeft.png");
    loadImage(touch[BUTTON_DOWNRIGHT], "buttons/DownRight.png");
    loadImage(touch[BUTTON_A], "buttons/A.png");
    loadImage(touch[BUTTON_A_PS], "buttons/A_ps.png");
    loadImage(touch[BUTTON_A_BLANK], "buttons/A_blank.png");
    loadImage(touch[BUTTON_A_DO], "buttons/A_do.png");
    loadImage(touch[BUTTON_A_ENTER], "buttons/A_enter.png");
    loadImage(touch[BUTTON_A_JUMP], "buttons/A_jump.png");
    loadImage(touch[BUTTON_B], "buttons/V.png");
    loadImage(touch[BUTTON_B_PS], "buttons/V_ps.png");
    loadImage(touch[BUTTON_B_BLANK], "buttons/V_blank.png");
    loadImage(touch[BUTTON_B_JUMP], "buttons/V_jump.png");
    loadImage(touch[BUTTON_B_SPINJUMP], "buttons/V_spinjump.png");
    loadImage(touch[BUTTON_X], "buttons/X.png");
    loadImage(touch[BUTTON_X_PS], "buttons/X_ps.png");
    loadImage(touch[BUTTON_X_BACK], "buttons/X_back.png");
    loadImage(touch[BUTTON_X_BLANK], "buttons/X_blank.png");
    loadImage(touch[BUTTON_X_BOMB], "buttons/X_bomb.png");
    loadImage(touch[BUTTON_X_BUMERANG], "buttons/X_bumerang.png");
    loadImage(touch[BUTTON_X_FIRE], "buttons/X_fire.png");
    loadImage(touch[BUTTON_X_HAMMER], "buttons/X_hammer.png");
    loadImage(touch[BUTTON_X_RUN], "buttons/X_run.png");
    loadImage(touch[BUTTON_X_SWORD], "buttons/X_sword.png");
    loadImage(touch[BUTTON_Y], "buttons/Y.png");
    loadImage(touch[BUTTON_Y_PS], "buttons/Y_ps.png");
    loadImage(touch[BUTTON_Y_BLANK], "buttons/Y_blank.png");
    loadImage(touch[BUTTON_Y_BOMB], "buttons/Y_bomb.png");
    loadImage(touch[BUTTON_Y_BUMERANG], "buttons/Y_bumerang.png");
    loadImage(touch[BUTTON_Y_FIRE], "buttons/Y_fire.png");
    loadImage(touch[BUTTON_Y_HAMMER], "buttons/Y_hammer.png");
    loadImage(touch[BUTTON_Y_RUN], "buttons/Y_run.png");
    loadImage(touch[BUTTON_Y_STATUE], "buttons/Y_statue.png");
    loadImage(touch[BUTTON_Y_SWORD], "buttons/Y_sword.png");
    loadImage(touch[BUTTON_DROP], "buttons/Select.png");
    loadImage(touch[BUTTON_HOLD_RUN_OFF], "buttons/RunOff.png");
    loadImage(touch[BUTTON_HOLD_RUN_ON], "buttons/RunOn.png");
    loadImage(touch[BUTTON_VIEW_TOGGLE_OFF], "buttons/KeysShowOff.png");
    loadImage(touch[BUTTON_VIEW_TOGGLE_ON], "buttons/KeysShow.png");
    loadImage(touch[BUTTON_ANALOG_BORDER], "buttons/SBorder.png");
    loadImage(touch[BUTTON_ANALOG_STICK], "buttons/AStick.png");
#endif

    if(m_loadErrors > 0)
    {
        std::string msg = fmt::format_ne("Failed to load an UI image assets. Look a log file to get more details:\n{0}"
                                         "\n\n"
                                         "It's possible that you didn't installed the game assets package, or you had installed it at the incorrect directory.",
                                         getLogFilePath());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "UI image assets loading error", msg.c_str(), nullptr);
        return false;
    }

    return true;
}

void GFX_t::unLoad()
{
    for(StdPicture *p : m_loadedImages)
        frmMain.deleteTexture(*p);
    m_loadedImages.clear();
}
