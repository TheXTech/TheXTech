/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#include "font_manager.h"
#include "font_manager_private.h"
#include "raster_font.h"
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
#include "ttf_font.h"
#endif

#include <AppPath/app_path.h>
#include <Graphics/graphics_funcs.h>
#include <IniProcessor/ini_processing.h>
#include <Logger/logger.h>
#include "core/render.h"

#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <IniProcessor/ini_processing.h>
#include <fmt/fmt_format.h>

#include <vector>

BaseFontEngine::~BaseFontEngine()
{}

//! Complete array of available raster fonts
static VPtrList<RasterFont> g_rasterFonts;
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
static VPtrList<TtfFont>    g_ttfFonts;
#endif
static VPtrList<BaseFontEngine*>    g_anyFonts;

//! Default raster font to render text
static RasterFont      *g_defaultRasterFont = nullptr;

#ifdef THEXTECH_ENABLE_TTF_SUPPORT
//! Default TTF font to render text
static TtfFont         *g_defaultTtfFont = nullptr;
#endif

//! Is font manager initialized
static bool             g_fontManagerIsInit = false;

typedef std::unordered_map<std::string, int> FontsHash;
//! Database of available fonts
static FontsHash        g_fontNameToId;

static bool             g_double_pixled = false;

static void registerFont(BaseFontEngine* font)
{
    g_anyFonts.push_back(font);
    g_fontNameToId.insert({font->getFontName(), g_anyFonts.size() - 1});
}

static const int c_smbxFontsMapMax = 100;
static int s_smbxFontsMap[c_smbxFontsMapMax];

int FontManager::fontIdFromSmbxFont(int font)
{
    if(font >= c_smbxFontsMapMax || font < 0)
        return -1;
    return s_smbxFontsMap[font];
}


BaseFontEngine *FontManager::getDefaultTtfFont()
{
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    return g_defaultTtfFont;
#else
    return nullptr;
#endif
}

void FontManager::initBasic()
{
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(!g_ft)
        initializeFreeType();
    g_defaultTtfFont = nullptr;
#endif

    g_double_pixled = false;
    g_fontManagerIsInit = true;
}

void FontManager::initFull()
{
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(!g_ft)
        initializeFreeType();
    g_defaultTtfFont = nullptr;
#endif

    g_double_pixled = false; //ConfigManager::setup_fonts.double_pixled;

    std::memset(s_smbxFontsMap, 0, sizeof(s_smbxFontsMap));
    for(int i = 0; i < c_smbxFontsMapMax; ++i)
        s_smbxFontsMap[i] = -1;

    const std::string fonts_root = AppPathManager::assetsRoot() + "fonts";

    /***************Load raster font support****************/
    DirMan fontsDir(fonts_root);

    std::vector<std::string> files;
    fontsDir.getListOfFiles(files, {".font.ini"});
    std::sort(files.begin(), files.end());
    for(std::string &fonFile : files)
    {
        g_rasterFonts.emplace_back();
        RasterFont& rf = g_rasterFonts.back();
        std::string fontPath = fontsDir.absolutePath() + "/" + fonFile;
        pLogDebug("Loading raster font %s...", fontPath.c_str());
        rf.loadFont(fontPath);

        if(!rf.isLoaded())   //Pop broken font from array
        {
            pLogWarning("Failed to load the font %s", fontPath.c_str());
            g_rasterFonts.pop_back();
        }
        else   //Register font name in a table
            registerFont(&rf);
    }

    if(!g_rasterFonts.empty())
        g_defaultRasterFont = &g_rasterFonts.front();

    /***************Load TTF font support****************/
    IniProcessing overrider(fonts_root + "/overrides.ini");

#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(overrider.beginGroup("ttf-fonts"))
    {
        auto k = overrider.allKeys();
        for(auto &key : k)
        {
            std::string fontFile;
            overrider.read(key.c_str(), fontFile, "");

            if(fontFile.empty())
                continue;

            std::string fontPath = fontsDir.absolutePath() + "/" + fontFile;

            g_ttfFonts.emplace_back();
            TtfFont& tf = g_ttfFonts.back();

            pLogDebug("Loading TTF font %s...", fontPath.c_str());
            tf.loadFont(fontPath);
            if(!tf.isLoaded())   //Pop broken font from array
            {
                pLogWarning("Failed to load the font %s", fontPath.c_str());
                g_ttfFonts.pop_back();
            }
            else   //Register font name in a table
            {
                registerFont(&tf);
                // Set the default TTF font (as a fallback)
                if(!g_defaultTtfFont)
                    g_defaultTtfFont = &tf;
            }
        }
    }

    overrider.endGroup();
#endif

    if(overrider.beginGroup("font-overrides"))
    {
        auto k = overrider.allKeys();
        for(auto &key : k)
        {
            std::string name;
            overrider.read(key.c_str(), name, "");
            if(name.empty())
                continue;

            int iKey = std::atoi(key.c_str());
            if(iKey >= 0 && iKey < c_smbxFontsMapMax)            
                s_smbxFontsMap[iKey] = getFontID(name);
        }
    }

    overrider.endGroup();

    g_fontManagerIsInit = true;
}

void FontManager::quit()
{
    g_fontNameToId.clear();
    g_anyFonts.clear();
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    g_ttfFonts.clear();
#endif
    g_rasterFonts.clear();
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    closeFreeType();
#endif
}


PGE_Size FontManager::textSize(const std::string& text, int fontID,
                               uint32_t max_line_lenght,
                               bool cut,
                               uint32_t ttfFontSize)
{
    SDL_assert_release(g_fontManagerIsInit);// Font manager is not initialized!
#ifndef THEXTECH_ENABLE_TTF_SUPPORT
    (void)ttfFontSize;
#endif

    (void)cut;

    if(!g_fontManagerIsInit)
        return PGE_Size(27 * 20, static_cast<int>(std::count(text.begin(), text.end(), '\n') + 1) * 20);

    if(text.empty())
        return PGE_Size(0, 0);

    if(max_line_lenght <= 0)
        max_line_lenght = 1000;

//    if(cut)
//    {
//        std::string::size_type i = text.find('\n');
//        if(i != std::string::npos)
//            text.erase(i, text.size() - i);
//    }

    //Use one of loaded fonts
    if((fontID >= 0) && (static_cast<size_t>(fontID) < g_anyFonts.size()))
    {
        if(g_anyFonts[fontID]->isLoaded())
            return g_anyFonts[fontID]->textSize(text, max_line_lenght);
    }

#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(g_defaultTtfFont->isLoaded())
        return g_defaultTtfFont->textSize(text, max_line_lenght, false, ttfFontSize);
#endif

    return PGE_Size(27 * 20, static_cast<int>(std::count(text.begin(), text.end(), '\n') + 1) * 20);
}

int FontManager::getFontID(std::string fontName)
{
    FontsHash::iterator i = g_fontNameToId.find(fontName);
    if(i == g_fontNameToId.end())
        return DefaultRaster;
    else
        return i->second;
}



void FontManager::printText(const std::string &text,
                            int x, int y, int font,
                            float Red, float Green, float Blue, float Alpha,
                            uint32_t ttf_FontSize)
{
    if(!g_fontManagerIsInit)
        return;

    if(text.empty())
        return;

    if((font >= 0) && (static_cast<size_t>(font) < g_anyFonts.size()))
    {
        if(g_anyFonts[font]->isLoaded())
        {
            g_anyFonts[font]->printText(text, x, y, Red, Green, Blue, Alpha);
            return;
        }
    }

    switch(font)
    {
    case DefaultRaster:
        if(g_defaultRasterFont && g_defaultRasterFont->isLoaded())
        {
            g_defaultRasterFont->printText(text, x, y, Red, Green, Blue, Alpha, ttf_FontSize);
            break;
        } /*fallthrough*/
    case DefaultTTF_Font:
    default:
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
        if(g_defaultTtfFont->isLoaded())
            g_defaultTtfFont->printText(text, x, y, Red, Green, Blue, Alpha, ttf_FontSize);
#endif
        break;
    }
}

void FontManager::printText(const char* text, int x, int y, int font, float Red, float Green, float Blue, float Alpha, uint32_t ttf_FontSize)
{
    if(!g_fontManagerIsInit)
        return;

    if(!text)
        return;

    if((font >= 0) && (static_cast<size_t>(font) < g_anyFonts.size()))
    {
        if(g_anyFonts[font]->isLoaded())
        {
            g_anyFonts[font]->printText(text, x, y, Red, Green, Blue, Alpha);
            return;
        }
    }

    switch(font)
    {
    case DefaultRaster:
        if(g_defaultRasterFont && g_defaultRasterFont->isLoaded())
        {
            g_defaultRasterFont->printText(text, x, y, Red, Green, Blue, Alpha, ttf_FontSize);
            break;
        } /*fallthrough*/
    case DefaultTTF_Font:
    default:
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
        if(g_defaultTtfFont->isLoaded())
            g_defaultTtfFont->printText(text, x, y, Red, Green, Blue, Alpha, ttf_FontSize);
#endif
        break;
    }
}

void FontManager::optimizeText(std::string &text, size_t max_line_lenght, int *numLines, int *numCols)
{
    /****************Word wrap*********************/
    size_t  lastspace = 0;
    int     count = 1;
    size_t  maxWidth = 0;

    for(size_t x = 0, i = 0; i < text.size(); i++, x++)
    {
        switch(text[i])
        {
        case '\t':
        case ' ':
            lastspace = i;
            break;

        case '\n':
            lastspace = 0;
            if((maxWidth < x) && (maxWidth < max_line_lenght))
                maxWidth = x;
            x = 0;
            count++;
            break;
        }

        if(x >= max_line_lenght) //If lenght more than allowed
        {
            maxWidth = x;

            if(lastspace > 0)
            {
                text[lastspace] = '\n';
                i = lastspace - 1;
                lastspace = 0;
            }
            else
            {
                text.insert(i, 1, '\n');
                x = 0;
                count++;
            }
        }

        UTF8 uch = static_cast<unsigned char>(text[i]);
        i += static_cast<size_t>(trailingBytesForUTF8[uch]);
    }

    if(count == 1)
        maxWidth = text.length();

    /****************Word wrap*end*****************/

    if(numLines)
        *numLines = count;

    if(numCols)
        *numCols = static_cast<int>(maxWidth);
}

std::string FontManager::cropText(std::string text, size_t max_symbols)
{
    if(max_symbols == 0)
        return text;

    size_t utf8len = utf8_substrlen(text, max_symbols);
    if(text.size() > utf8len)
    {
        text.erase(utf8len, text.size() - utf8len);
        text.append("...");
    }
    return text;
}
