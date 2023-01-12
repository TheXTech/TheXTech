/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_assert.h"

#include "globals.h"
#include "gfx.h"
#include "core/msgbox.h"
#include "core/render.h"
#include <fmt_format_ne.h>
#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>

#include "graphics/gfx_frame.h"

GFX_t GFX;


void GFX_t::loadImage(StdPicture &img, const std::string &path)
{
#ifdef X_IMG_EXT
    std::string path_ext = path + X_IMG_EXT;
#else
    std::string path_ext = path + ".png";
#endif

    pLogDebug("Loading texture %s...", path_ext.c_str());
    img = XRender::LoadPicture(path_ext);

#if defined(X_IMG_EXT) && !defined(X_NO_PNG_GIF)
    if(!img.inited)
    {
        path_ext = path + ".png";
        pLogDebug("Could not load, trying %s...", path_ext.c_str());
        img = XRender::LoadPicture(path_ext);
    }
#endif

    if(!img.inited)
    {
        pLogWarning("Failed to load texture: %s...", path_ext.c_str());
        m_loadErrors++;
    }

    m_loadedImages.push_back(&img);
}

GFX_t::GFX_t() noexcept
{}

bool GFX_t::load()
{
    std::string uiPath = AppPath + "graphics/ui/";

    loadImage(BMVs, uiPath + "BMVs");
    loadImage(BMWin, uiPath + "BMWin");
    For(i, 1, 3)
        loadImage(Boot[i], uiPath + fmt::format_ne("Boot{0}", i));

    For(i, 1, 5)
        loadImage(CharacterName[i], uiPath + fmt::format_ne("CharacterName{0}", i));

    loadImage(Chat, uiPath + "Chat");

    For(i, 0, 2)
        loadImage(Container[i], uiPath + fmt::format_ne("Container{0}", i));

    For(i, 1, 3)
        loadImage(ECursor[i], uiPath + fmt::format_ne("ECursor{0}", i));

    For(i, 0, 9)
        loadImage(Font1[i], uiPath + fmt::format_ne("Font1_{0}", i));

    For(i, 1, 3)
        loadImage(Font2[i], uiPath + fmt::format_ne("Font2_{0}", i));

    loadImage(Font2S, uiPath + "Font2S");

    For(i, 1, 2)
        loadImage(Heart[i], uiPath + fmt::format_ne("Heart{0}", i));

    For(i, 0, 8)
        loadImage(Interface[i], uiPath + fmt::format_ne("Interface{0}", i));

    loadImage(LoadCoin, uiPath + "LoadCoin");
    loadImage(Loader, uiPath + "Loader");

    For(i, 0, 3)
        loadImage(MCursor[i], uiPath + fmt::format_ne("MCursor{0}", i));

    For(i, 1, 4)
        loadImage(MenuGFX[i], uiPath + fmt::format_ne("MenuGFX{0}", i));

    loadImage(Mount[2], uiPath + "Mount");

    For(i, 0, 7)
        loadImage(nCursor[i], uiPath + fmt::format_ne("nCursor{0}", i));

    loadImage(TextBox, uiPath + "TextBox");

    For(i, 1, 2)
        loadImage(Tongue[i], uiPath + fmt::format_ne("Tongue{0}", i));

    loadImage(Warp, uiPath + "Warp");

    loadImage(YoshiWings, uiPath + "YoshiWings");

    // Add new required assets here. Also update load_gfx.cpp:loadCustomUIAssets()

    if(m_loadErrors > 0)
    {
        std::string msg = fmt::format_ne("Failed to load an UI image assets. Look a log file to get more details:\n{0}"
                                         "\n\n"
                                         "It's possible that you didn't installed the game assets package, or you had installed it at the incorrect directory.",
                                         getLogFilePath());
        XMsgBox::simpleMsgBox(XMsgBox::MESSAGEBOX_ERROR, "UI image assets loading error", msg);
        return false;
    }

    loadImage(EIcons, uiPath + "EditorIcons");

    if(m_loadErrors > 0)
    {
        pLogWarning("Missing new editor icons.");
        m_loadErrors = 0;
    }

    loadImage(PCursor, uiPath + "PCursor");

    if(m_loadErrors > 0)
    {
        pLogWarning("Missing new player cursor.");
        m_loadErrors = 0;
    }

    loadImage(Backdrop, uiPath + "Backdrop");
    loadImage(Backdrop_Border, uiPath + "Backdrop_Border");

    IniProcessing ini;
    ini.open(uiPath + "Backdrop_Border.ini");
    loadFrameInfo(ini, g_backdropBorderInfo);

    if(Backdrop_Border.inited)
    {
        // warn if invalid
        const FrameBorderInfo& i = g_backdropBorderInfo;
        if(i.le + i.li + i.ri + i.re > Backdrop_Border.w)
            pLogWarning("Invalid border: total internal/external width is %d but Backdrop_Border.png is only %dpx wide.", i.le + i.li + i.ri + i.re, Backdrop_Border.w);
        if(i.te + i.ti + i.bi + i.be > Backdrop_Border.h)
            pLogWarning("Invalid border: total internal/external height is %d but Backdrop_Border.png is only %dpx wide.", i.te + i.ti + i.bi + i.be, Backdrop_Border.h);
    }

    if(m_loadErrors > 0)
    {
        pLogDebug("Missing new backdrop textures.");
        m_loadErrors = 0;
    }

    loadImage(WorldMapFrame_Tile, uiPath + "WorldMapFrame_Tile");
    loadImage(WorldMapFrame_Border, uiPath + "WorldMapFrame_Border");
    loadImage(WorldMapFog, uiPath + "WorldMapFog");

    ini.open(uiPath + "WorldMapFrame_Border.ini");
    loadFrameInfo(ini, g_worldMapFrameBorderInfo);

    if(WorldMapFrame_Border.inited)
    {
        // warn if invalid
        const FrameBorderInfo& i = g_worldMapFrameBorderInfo;
        if(i.le + i.li + i.ri + i.re > WorldMapFrame_Border.w)
            pLogWarning("Invalid border: total internal/external width is %d but WorldMapFrame_Border.png is only %dpx wide.", i.le + i.li + i.ri + i.re, WorldMapFrame_Border.w);
        if(i.te + i.ti + i.bi + i.be > WorldMapFrame_Border.h)
            pLogWarning("Invalid border: total internal/external height is %d but WorldMapFrame_Border.png is only %dpx wide.", i.te + i.ti + i.bi + i.be, WorldMapFrame_Border.h);
    }

    if(m_loadErrors > 0)
    {
        pLogDebug("Missing new world map frame tile/border textures.");
        m_loadErrors = 0;
    }

    loadImage(Camera, uiPath + "Camera");

    if(m_loadErrors > 0)
    {
        pLogDebug("Missing new small-screen look up/down camera texture.");
        m_loadErrors = 0;
    }

    // Add new optional assets above this line. Also update load_gfx.cpp: loadCustomUIAssets(), and gfx.h: GFX_t::m_isCustomVolume.

    SDL_assert_release(m_loadedImages.size() <= m_isCustomVolume);
    SDL_memset(m_isCustom, 0, sizeof(m_loadedImages.size() * sizeof(bool)));

    return true;
}

void GFX_t::unLoad()
{
    for(StdPicture *p : m_loadedImages)
        XRender::deleteTexture(*p);
    m_loadedImages.clear();
    SDL_memset(m_isCustom, 0, sizeof(m_loadedImages.size() * sizeof(bool)));
}

bool& GFX_t::isCustom(size_t i)
{
    SDL_assert_release(i < m_isCustomVolume);
    return m_isCustom[i];
}
