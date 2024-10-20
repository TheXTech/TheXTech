/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "graphics/gfx_frame.h"

#include <IniProcessor/ini_processing.h>
#include <Utils/files_ini.h>
#include <fmt_format_ne.h>
#include <Logger/logger.h>

#if defined(__3DS__) || defined(__SWITCH__) || defined(__WII__) || defined(__WIIU__)
#   include <Utils/files.h>
#   if defined(__SWITCH__)
#       define UI_PLATFORM_EXT "-switch"
#   elif defined(__3DS__)
#       define UI_PLATFORM_EXT "-3ds"
#   elif defined(__WII__)
#       define UI_PLATFORM_EXT "-wii"
#   elif defined(__WIIU__)
#       define UI_PLATFORM_EXT "-wiiu"
#   endif
#endif

#ifdef X_IMG_EXT
#   define  UI_IMG_EXT X_IMG_EXT
#else
#   define  UI_IMG_EXT ".png"
#endif

GFX_t GFX;


void GFX_t::loadImage(StdPicture &img, const std::string &path)
{
    std::string path_ext = path + UI_IMG_EXT;

    pLogDebug("Loading texture %s...", path_ext.c_str());
    XRender::LoadPicture(img, path_ext);

#if defined(X_IMG_EXT) && !defined(X_NO_PNG_GIF)
    if(!img.inited)
    {
        path_ext = path + ".png";
        pLogDebug("Could not load, trying %s...", path_ext.c_str());
        XRender::LoadPicture(img, path_ext);
    }
#endif

    if(!img.inited)
    {
        pLogWarning("Failed to load texture: %s...", path_ext.c_str());
        m_loadErrors++;
    }

    m_loadedImages.push_back(&img);
}

void GFX_t::loadBorder(FrameBorder& border, const std::string& path)
{
    loadImage(border.tex, path);

    if(!border.tex.inited)
        return;

    IniProcessing ini = Files::load_ini(path + ".ini");
    loadFrameInfo(ini, border);

    // warn if invalid
    const FrameBorderInfo& i = border;
    if(i.le + i.li + i.ri + i.re > border.tex.w)
        pLogWarning("Invalid border: total internal/external width is %d but [%s] is only %dpx wide.", i.le + i.li + i.ri + i.re, path.c_str(), border.tex.w);
    if(i.te + i.ti + i.bi + i.be > border.tex.h)
        pLogWarning("Invalid border: total internal/external height is %d but [%s] is only %dpx tall.", i.te + i.ti + i.bi + i.be, path.c_str(), border.tex.h);
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
    {
#if defined(UI_PLATFORM_EXT)
        auto n = uiPath + fmt::format_ne("MenuGFX{0}" UI_PLATFORM_EXT, i);
#   ifdef X_IMG_EXT
        if(Files::fileExists(n + UI_IMG_EXT) || Files::fileExists(n + ".png"))
#   else
        if(Files::fileExists(n + UI_IMG_EXT))
#   endif
        {
            loadImage(MenuGFX[i], n);
            continue;
        }

        pLogWarning("File %s%s doesn't exist, trying to load generic one...", n.c_str(), UI_IMG_EXT);
#endif
        loadImage(MenuGFX[i], uiPath + fmt::format_ne("MenuGFX{0}", i));
    }

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
        // std::string msg = fmt::format_ne("Failed to load UI image assets from {0}. Look a log file to get more details:\n{1}"
        //                                  "\n\n"
        //                                  "It's possible that you didn't install the game assets package, or you had installed it at the incorrect directory.",
        //                                  AppPath, getLogFilePath());
        // XMsgBox::simpleMsgBox(XMsgBox::MESSAGEBOX_ERROR, "UI image assets loading error", msg);
        m_loadErrors = 0;
        return false;
    }

    loadImage(EIcons, uiPath + "EditorIcons");

    if(m_loadErrors > 0)
        m_loadErrors = 0;

    loadImage(PCursor, uiPath + "PCursor");

    if(m_loadErrors > 0)
        m_loadErrors = 0;

    loadImage(Medals, uiPath + "Medals");

    if(m_loadErrors > 0)
        m_loadErrors = 0;

    loadImage(CharSelIcons, uiPath + "CharSelIcons");

    loadBorder(CharSelFrame, uiPath + "CharSelFrame");

    if(m_loadErrors > 0)
        m_loadErrors = 0;

    loadImage(Backdrop, uiPath + "Backdrop");
    loadBorder(Backdrop_Border, uiPath + "Backdrop_Border");

    if(m_loadErrors > 0)
    {
        pLogDebug("Missing new backdrop textures.");
        m_loadErrors = 0;
    }

    loadImage(WorldMapFrame_Tile, uiPath + "WorldMapFrame_Tile");
    loadBorder(WorldMapFrame_Border, uiPath + "WorldMapFrame_Border");

    if(m_loadErrors > 0)
    {
        pLogDebug("Missing new world map frame tile/border textures.");
        m_loadErrors = 0;
    }

    loadImage(Camera, uiPath + "Camera");

    if(m_loadErrors > 0)
        m_loadErrors = 0;

    loadImage(Balance, uiPath + "Balance");

    if(m_loadErrors > 0)
        m_loadErrors = 0;

    // Add new optional assets above this line. Also update load_gfx.cpp: loadCustomUIAssets(), and gfx.h: GFX_t::m_isCustomVolume.

    SDL_assert_release(m_loadedImages.size() <= m_isCustomVolume);
    SDL_memset(m_isCustom, 0, sizeof(m_loadedImages.size() * sizeof(bool)));

    return true;
}

void GFX_t::unLoad()
{
    for(StdPicture *p : m_loadedImages)
        p->reset();

    m_loadedImages.clear();
    SDL_memset(m_isCustom, 0, sizeof(m_loadedImages.size() * sizeof(bool)));
}

bool& GFX_t::isCustom(size_t i)
{
    SDL_assert_release(i < m_isCustomVolume);
    return m_isCustom[i];
}
