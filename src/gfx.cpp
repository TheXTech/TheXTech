/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "core/msgbox.h"
#include "core/render.h"
#include <fmt_format_ne.h>
#include <Logger/logger.h>


GFX_t GFX;


void GFX_t::loadImage(StdPicture &img, const std::string &path)
{
    pLogDebug("Loading texture %s...", path.c_str());
    img = XRender::LoadPicture(path);

    if(!img.d.hasTexture())
    {
        pLogWarning("Failed to load texture: %s...", path.c_str());
        m_loadErrors++;
    }

    m_loadedImages.push_back(&img);
}

GFX_t::GFX_t() noexcept
{}

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

    if(m_loadErrors > 0)
    {
        std::string msg = fmt::format_ne("Failed to load an UI image assets. Look a log file to get more details:\n{0}"
                                         "\n\n"
                                         "It's possible that you didn't installed the game assets package, or you had installed it at the incorrect directory.",
                                         getLogFilePath());
        XMsgBox::simpleMsgBox(AbstractMsgBox_t::MESSAGEBOX_ERROR, "UI image assets loading error", msg);
        return false;
    }

    return true;
}

void GFX_t::unLoad()
{
    for(StdPicture *p : m_loadedImages)
        XRender::deleteTexture(*p);
    m_loadedImages.clear();
}
