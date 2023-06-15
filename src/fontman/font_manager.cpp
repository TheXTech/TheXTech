/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_assert.h"
#include "sdl_proxy/sdl_stdinc.h"

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
#include <fmt_format_ne.h>

#include <vector>

BaseFontEngine::~BaseFontEngine()
{}

typedef VPtrList<RasterFont> RasterFontsList;
typedef VPtrList<TtfFont> TtfFontsList;

//! Complete array of available raster fonts
static RasterFontsList g_rasterFonts;
static RasterFontsList g_rasterFontsCustomLevel;
static RasterFontsList g_rasterFontsCustom;
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
static TtfFontsList    g_ttfFonts;
static TtfFontsList    g_ttfFontsCustom;
static TtfFontsList    g_ttfFontsCustomLevel;
#define LOADFONTARG(x) x
#else
#define LOADFONTARG(x) false
#endif

typedef VPtrList<BaseFontEngine*> PtrFontsList;
static PtrFontsList    g_anyFonts;
//! Backup for the case when any custom fonts are been loaded
static PtrFontsList    g_anyFontsBackup;
static PtrFontsList    g_anyFontsBackupWorld;

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
//! Backup of the database of available fonts
static FontsHash        g_fontNameToIdBackup;
static FontsHash        g_fontNameToIdBackupWorld;

static bool             g_double_pixled = false;

static const int c_smbxFontsMapMax = 100;
// Current state
static int s_smbxFontsMap[c_smbxFontsMapMax];
static int s_smbxFontsSizesMap[c_smbxFontsMapMax];

// The initial default state
static int s_smbxFontsMapDefault[c_smbxFontsMapMax];
static int s_smbxFontsSizesMapDefault[c_smbxFontsMapMax];

// The episode-wide preserved state without content of data directory
static int s_smbxFontsMapWorld[c_smbxFontsMapMax];
static int s_smbxFontsSizesMapWorld[c_smbxFontsMapMax];

static const uint32_t c_defaultFontSize = 14;

static bool s_fontMapsDefault = true;
static bool s_fontMapsWorld = true;
static std::string s_lastWorldFontsPath;
static std::string s_lastCustomFontsPath;


static void registerFont(BaseFontEngine* font)
{
    g_anyFonts.push_back(font);

    int newIdx = g_anyFonts.size() - 1;
    auto ef = g_fontNameToId.find(font->getFontName());

    // If overriding an existing font by name, also replace the SMBX code overrides
    if(ef != g_fontNameToId.end())
    {
        int oldIdx = ef->second;
        for(int i = 0; i < c_smbxFontsMapMax; ++i)
        {
            if(s_smbxFontsMap[i] == oldIdx)
                s_smbxFontsMap[i] = newIdx;
        }
        ef->second = newIdx;
    }
    else
        g_fontNameToId.insert({font->getFontName(), newIdx});
}

static void backupDefaultFontMaps()
{
    if(!s_fontMapsDefault)
        return; // Don't preserve customized font maps

    SDL_memcpy(s_smbxFontsMapDefault, s_smbxFontsMap, sizeof(s_smbxFontsMap));
    SDL_memcpy(s_smbxFontsSizesMapDefault, s_smbxFontsSizesMap, sizeof(s_smbxFontsSizesMap));
    g_anyFontsBackup = g_anyFonts;
    g_fontNameToIdBackup = g_fontNameToId;
}

static void restoreDefaultFontMaps()
{
    if(s_fontMapsDefault)
        return; // Don't restore if already default

    SDL_memcpy(s_smbxFontsMap, s_smbxFontsMapDefault, sizeof(s_smbxFontsMap));
    SDL_memcpy(s_smbxFontsSizesMap, s_smbxFontsSizesMapDefault, sizeof(s_smbxFontsSizesMap));
    g_anyFonts = g_anyFontsBackup;
    g_fontNameToId = g_fontNameToIdBackup;
    s_fontMapsDefault = true;
    s_fontMapsWorld = true;
}

static void backupWorldFontMaps()
{
    if(!s_fontMapsWorld)
        return; // Don't preserve customized font maps

    SDL_memcpy(s_smbxFontsMapWorld, s_smbxFontsMap, sizeof(s_smbxFontsMap));
    SDL_memcpy(s_smbxFontsSizesMapWorld, s_smbxFontsSizesMap, sizeof(s_smbxFontsSizesMap));
    g_anyFontsBackupWorld = g_anyFonts;
    g_fontNameToIdBackupWorld = g_fontNameToId;
}

static void restoreWorldFontMaps()
{
    if(s_fontMapsWorld)
        return; // Don't restore if already default

    SDL_memcpy(s_smbxFontsMap, s_smbxFontsMapWorld, sizeof(s_smbxFontsMap));
    SDL_memcpy(s_smbxFontsSizesMap, s_smbxFontsSizesMapWorld, sizeof(s_smbxFontsSizesMap));
    g_anyFonts = g_anyFontsBackupWorld;
    g_fontNameToId = g_fontNameToIdBackupWorld;
    s_fontMapsWorld = true;
}

int FontManager::fontIdFromSmbxFont(int font)
{
    if(font >= c_smbxFontsMapMax || font < 0)
        return -1;
    return s_smbxFontsMap[font];
}

uint32_t FontManager::fontSizeFromSmbxFont(int font)
{
    if(font >= c_smbxFontsMapMax || font < 0)
        return c_defaultFontSize;

    int ret = s_smbxFontsSizesMap[font];
    return (ret <= 0) ? c_defaultFontSize : static_cast<uint32_t>(ret);
}


TtfFont* FontManager::getDefaultTtfFont()
{
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    return g_defaultTtfFont;
#else
    return nullptr;
#endif
}

TtfFont* FontManager::getTtfFontByName(const std::string& fontName)
{
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(fontName.empty())
        return FontManager::getDefaultTtfFont();

    FontsHash::iterator i = g_fontNameToId.find(fontName);
    if(i == g_fontNameToId.end())
        return FontManager::getDefaultTtfFont();

    auto *ret = dynamic_cast<TtfFont*>(g_anyFonts[i->second]);

    return ret ? ret : FontManager::getDefaultTtfFont();
#else
    (void)fontName;
    return nullptr;
#endif
}

#ifdef THEXTECH_ENABLE_TTF_SUPPORT
static bool s_loadFintsFromDir(const std::string &fonts_root,
                               RasterFontsList &outRasterFonts,
                               TtfFontsList &outTtfFonts)
#else
static bool s_loadFintsFromDir(const std::string &fonts_root,
                               RasterFontsList &outRasterFonts,
                               bool)
#endif
{
    using namespace FontManager;

    /***************Load raster font support****************/
    DirMan fontsDir(fonts_root);
    if(!fontsDir.exists())
        return false; // Fonts manager is unavailable when directory is not exists

    std::vector<std::string> files;
    fontsDir.getListOfFiles(files, {".font.ini"});
    std::sort(files.begin(), files.end());

    for(std::string &fonFile : files)
    {
        outRasterFonts.emplace_back();
        RasterFont& rf = outRasterFonts.back();
        std::string fontPath = fontsDir.absolutePath() + "/" + fonFile;
        pLogDebug("Loading raster font %s...", fontPath.c_str());
        rf.loadFont(fontPath);

        if(!rf.isLoaded())   //Pop broken font from array
        {
            pLogWarning("Failed to load the font %s", fontPath.c_str());
            outRasterFonts.pop_back();
        }
        else   //Register font name in a table
            registerFont(&rf);
    }

    if(!outRasterFonts.empty())
        g_defaultRasterFont = &outRasterFonts.front();

    /***************Load TTF font support****************/
    IniProcessing overrider(fonts_root + "/overrides.ini");

#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(overrider.beginGroup("ttf-fonts"))
    {
        for(int i = 1; ;++i)
        {
            std::string key = fmt::format_ne("file{0}", i);
            std::string keyAntiAlias = fmt::format_ne("file{0}-antialias", i);
            std::string keyBitmapSize = fmt::format_ne("file{0}-bitmap-size", i);
            std::string keyFontName = fmt::format_ne("file{0}-font-name", i);
            std::string keyDoublePixel = fmt::format_ne("file{0}-double-pixel", i);

            if(!overrider.hasKey(key))
                break; // Stop look up on a first missing key

            std::string fontFile;
            overrider.read(key.c_str(), fontFile, "");

            if(fontFile.empty())
                continue;

            bool antiAlias;
            overrider.read(keyAntiAlias.c_str(), antiAlias, true);

            int bitmapSize;
            overrider.read(keyBitmapSize.c_str(), bitmapSize, 0);

            std::string fontName;
            overrider.read(keyFontName.c_str(), fontName, "");

            bool doublePixel;
            // FIXME: add XRender::RENDER_1X flag, use that instead
#if defined(__WII__) || defined(__3DS__)
            const bool doublePixelDefault = true;
#else
            const bool doublePixelDefault = false;
#endif
            overrider.read(keyDoublePixel.c_str(), doublePixel, doublePixelDefault);

            std::string fontPath = fontsDir.absolutePath() + "/" + fontFile;

            outTtfFonts.emplace_back();
            TtfFont& tf = outTtfFonts.back();

            tf.setAntiAlias(antiAlias);
            tf.setBitmapSize(bitmapSize);
            tf.setDoublePixel(doublePixel);

            pLogDebug("Loading TTF font %s...", fontPath.c_str());
            tf.loadFont(fontPath);
            if(!tf.isLoaded())   //Pop broken font from array
            {
                pLogWarning("Failed to load the font %s", fontPath.c_str());
                outTtfFonts.pop_back();
            }
            else   //Register font name in a table
            {
                if(!fontName.empty()) // Set the custom font name
                    tf.setFontName(fontName);

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

            if(key.find("-size") != std::string::npos)
                continue; // Skil the size key

            overrider.read(key.c_str(), name, "");
            if(name.empty())
                continue;

            int iKey = std::atoi(key.c_str());
            if(iKey >= 0 && iKey < c_smbxFontsMapMax)
                s_smbxFontsMap[iKey] = getFontID(name);

            std::string size_key = fmt::format_ne("{0}-size", iKey);
            if(overrider.hasKey(size_key))
                overrider.read(size_key.c_str(), s_smbxFontsSizesMap[iKey], -1);
        }
    }

    overrider.endGroup();

    return true;
}

void FontManager::initFull()
{
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(!g_ft)
        initializeFreeType();
    g_defaultTtfFont = nullptr;
#endif

    g_double_pixled = false; //ConfigManager::setup_fonts.double_pixled;

    SDL_memset(s_smbxFontsMap, 0, sizeof(s_smbxFontsMap));
    SDL_memset(s_smbxFontsSizesMap, 0, sizeof(s_smbxFontsSizesMap));

    for(int i = 0; i < c_smbxFontsMapMax; ++i)
    {
        s_smbxFontsMap[i] = -1;
        s_smbxFontsSizesMap[i] = -1;
    }

    const std::string fonts_root = AppPathManager::assetsRoot() + "fonts";

    if(!s_loadFintsFromDir(fonts_root, g_rasterFonts, LOADFONTARG(g_ttfFonts)))
    {
        pLogWarning("Can't load any font from the directory %s", fonts_root.c_str());
        return;
    }

    if(g_anyFonts.empty())
    {
        pLogWarning("There is no available fonts at the directory %s", fonts_root.c_str());
        return;
    }

    s_fontMapsDefault = true;
    s_fontMapsWorld = true;
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

void FontManager::loadCustomFonts(const std::string& episodeRoot, const std::string& dataSubDir)
{
    backupDefaultFontMaps();
    bool doLoadWorld = false;
    bool doLoadCustom = false;

    // Loading fonts from the episode directory
    if(s_lastWorldFontsPath != episodeRoot)
    {
        if(!s_lastWorldFontsPath.empty())
            clearAllCustomFonts();
        doLoadWorld = true;
        pLogDebug("Attempt to load custom fonts at %sfonts", episodeRoot.c_str());
    }
    else
        pLogDebug("Fonts at %sfonts already loaded", episodeRoot.c_str());

    if(doLoadWorld && s_loadFintsFromDir(episodeRoot + "fonts",
                                         g_rasterFontsCustom,
                                         LOADFONTARG(g_ttfFontsCustom)))
    {
        s_lastWorldFontsPath = episodeRoot;
        pLogDebug("Loaded custom fonts from the %sfonts", episodeRoot.c_str());
        s_fontMapsDefault = false;
        s_fontMapsWorld = true;
        backupWorldFontMaps();
    }

    // Loading fonts from the data directory
    if(s_lastCustomFontsPath != dataSubDir)
    {
        if(!s_lastCustomFontsPath.empty())
            clearLevelFonts();
        doLoadCustom = true;
        pLogDebug("Attempt to load custom fonts at %sfonts", dataSubDir.c_str());
    }
    else
        pLogDebug("Fonts at %sfonts already loaded", dataSubDir.c_str());

    if(doLoadCustom && s_loadFintsFromDir(dataSubDir + "fonts",
                                          g_rasterFontsCustomLevel,
                                          LOADFONTARG(g_ttfFontsCustomLevel)))
    {
        s_lastCustomFontsPath = dataSubDir;
        pLogDebug("Loaded custom fonts from the %sfonts", dataSubDir.c_str());
        s_fontMapsDefault = false;
        s_fontMapsWorld = false;
    }
}

void FontManager::clearAllCustomFonts()
{
    g_rasterFontsCustomLevel.clear();
    g_rasterFontsCustom.clear();
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    g_ttfFontsCustomLevel.clear();
    g_ttfFontsCustom.clear();
#endif
    s_lastCustomFontsPath.clear();
    s_lastWorldFontsPath.clear();
    restoreDefaultFontMaps();
}

void FontManager::clearLevelFonts()
{
    g_rasterFontsCustomLevel.clear();
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    g_ttfFontsCustomLevel.clear();
#endif

    s_lastCustomFontsPath.clear();
    restoreWorldFontMaps();
}

bool FontManager::isInitied()
{
    return g_fontManagerIsInit;
}


PGE_Size FontManager::textSize(const char* text, size_t text_size, int fontID,
                               uint32_t max_line_lenght,
                               bool cut,
                               uint32_t ttfFontSize)
{
    SDL_assert_release(g_fontManagerIsInit);// Font manager is not initialized!
#ifndef THEXTECH_ENABLE_TTF_SUPPORT
    (void)ttfFontSize;
#endif

    if(!g_fontManagerIsInit)
        return PGE_Size(27 * 20, static_cast<int>(std::count(text, text + text_size, '\n') + 1) * 20);

    if(!text || text_size == 0)
        return PGE_Size(0, 0);

    if(max_line_lenght <= 0)
        max_line_lenght = 1000;

    //Use one of loaded fonts
    if((fontID >= 0) && (static_cast<size_t>(fontID) < g_anyFonts.size()) && g_anyFonts[fontID])
    {
        if(g_anyFonts[fontID]->isLoaded())
            return g_anyFonts[fontID]->textSize(text, text_size, max_line_lenght, cut, ttfFontSize);
    }

#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(g_defaultTtfFont && g_defaultTtfFont->isLoaded())
        return g_defaultTtfFont->textSize(text, text_size, max_line_lenght, cut, ttfFontSize);
#endif

    return PGE_Size(27 * 20, static_cast<int>(std::count(text, text + text_size, '\n') + 1) * 20);
}

int FontManager::getFontID(std::string fontName)
{
    FontsHash::iterator i = g_fontNameToId.find(fontName);
    if(i == g_fontNameToId.end())
        return DefaultRaster;
    else
        return i->second;
}

void FontManager::printText(const char* text, size_t text_size,
                            int x, int y,
                            int font,
                            float Red, float Green, float Blue, float Alpha,
                            uint32_t ttf_FontSize)
{
    if(!g_fontManagerIsInit)
        return;

    if(!text || text_size == 0)
        return;

    if((font >= 0) && (static_cast<size_t>(font) < g_anyFonts.size()) && g_anyFonts[font])
    {
        if(g_anyFonts[font]->isLoaded())
        {
            g_anyFonts[font]->printText(text, text_size, x, y, Red, Green, Blue, Alpha, ttf_FontSize);
            return;
        }
    }

    switch(font)
    {
    case DefaultRaster:
        if(g_defaultRasterFont && g_defaultRasterFont->isLoaded())
        {
            g_defaultRasterFont->printText(text, text_size, x, y, Red, Green, Blue, Alpha, ttf_FontSize);
            break;
        } /*fallthrough*/
    case DefaultTTF_Font:
    default:
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
        if(g_defaultTtfFont && g_defaultTtfFont->isLoaded())
            g_defaultTtfFont->printText(text, text_size, x, y, Red, Green, Blue, Alpha, ttf_FontSize);
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
