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
#include "legacy_font.h"
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
#include "ttf_font.h"
#endif

#include <AppPath/app_path.h>
#include <Graphics/graphics_funcs.h>
#include <IniProcessor/ini_processing.h>
#include <Logger/logger.h>
#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <IniProcessor/ini_processing.h>
#include <fmt_format_ne.h>

#include "global_constants.h"
#include "core/render.h"
#include "global_dirs.h"


#include <vector>
#ifdef LOW_MEM
#   include <map>
#else
#   include <unordered_map>
#endif

BaseFontEngine::~BaseFontEngine()
{}

typedef VPtrList<RasterFont> RasterFontsList;
typedef VPtrList<TtfFont> TtfFontsList;
typedef VPtrList<LegacyFont> LegacyFontsList;

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
#   define LOADFONTARG(x) false
#endif
static LegacyFontsList g_legacyFonts;

typedef VPtrList<BaseFontEngine*> PtrFontsList;
static PtrFontsList    g_anyFonts;
//! Backup for the case when any custom fonts are been loaded
static PtrFontsList    g_anyFontsBackup;
static PtrFontsList    g_anyFontsBackupWorld;

//! Default raster font to render text
static BaseFontEngine  *g_defaultRasterFont = nullptr;

#ifdef THEXTECH_ENABLE_TTF_SUPPORT
//! Default TTF font to render text
static BaseFontEngine  *g_defaultTtfFont = nullptr;
#endif

//! IS font manager initialized?
static bool             g_fontManagerIsInit = false;
//! Does font manager uses legacy fonts fallback?
static bool             g_fontManagerIsLegacy = false;

#ifdef LOW_MEM
typedef std::map<std::string, vbint_t> FontsHash;
#else
typedef std::unordered_map<std::string, vbint_t> FontsHash;
#endif
//! Database of available fonts
static FontsHash        g_fontNameToId;
//! Backup of the database of available fonts
static FontsHash        g_fontNameToIdBackup;
static FontsHash        g_fontNameToIdBackupWorld;

static bool             g_double_pixled = false;

#ifdef LOW_MEM
static const vbint_t    c_smbxFontsMapMax = 10;
#else
static const vbint_t    c_smbxFontsMapMax = 20;
#endif

// Current state
static vbint_t s_smbxFontsMap[c_smbxFontsMapMax];
static vbint_t s_smbxFontsSizesMap[c_smbxFontsMapMax];

// The initial default state
static vbint_t s_smbxFontsMapDefault[c_smbxFontsMapMax];
static vbint_t s_smbxFontsSizesMapDefault[c_smbxFontsMapMax];

// The episode-wide preserved state without content of data directory
static vbint_t s_smbxFontsMapWorld[c_smbxFontsMapMax];
static vbint_t s_smbxFontsSizesMapWorld[c_smbxFontsMapMax];

static const uint32_t c_defaultFontSize = 14;

static bool s_fontMapsDefault = true;
static bool s_fontMapsWorld = true;
static std::string s_lastWorldFontsPath;
static std::string s_lastCustomFontsPath;


static void registerFont(BaseFontEngine* font)
{
    g_anyFonts.push_back(font);

    vbint_t newIdx = static_cast<vbint_t>(g_anyFonts.size()) - 1;
    auto ef = g_fontNameToId.find(font->getFontName());

    // If overriding an existing font by name, also replace the SMBX code overrides
    if(ef != g_fontNameToId.end())
    {
        vbint_t oldIdx = ef->second;
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
    return dynamic_cast<TtfFont*>(g_defaultTtfFont);
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
static bool s_loadFontsFromDir(DirListCI &fonts_root,
                               const std::string &subdir,
                               RasterFontsList &outRasterFonts,
                               TtfFontsList &outTtfFonts)
#else
static bool s_loadFontsFromDir(DirListCI &fonts_root,
                               const std::string &subdir,
                               RasterFontsList &outRasterFonts,
                               bool)
#endif
{
    using namespace FontManager;

    /***************Load raster font support****************/
    if(!subdir.empty() && !fonts_root.dirExistsCI(subdir))
        return false; // Fonts manager is unavailable when directory is not exists

    std::vector<std::string> files = fonts_root.getFilesList(subdir, {".font.ini"});
    std::sort(files.begin(), files.end());

    std::string sSubDir = subdir + (subdir.empty() ? "" : "/");

    for(std::string &fonFile : files)
    {
        outRasterFonts.emplace_back();
        RasterFont& rf = outRasterFonts.back();
        std::string fontPath = fonts_root.getCurDir() + sSubDir + fonFile;
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
    IniProcessing overrider(fonts_root.getCurDir() + sSubDir + "overrides.ini");

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

            std::string fontPath = fonts_root.getCurDir() + sSubDir + fontFile;

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
                continue; // Skip the size key

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

static void s_initFontsFallback()
{
    for(int i = 1; i <= 4; ++i)
    {
        g_legacyFonts.emplace_back(i);
        LegacyFont& lf = g_legacyFonts.back();

        if(!lf.isLoaded())   //Pop broken font from array
        {
            pLogWarning("Failed to load the legacy font %d", i);
            g_legacyFonts.pop_back();
        }
        else   //Register font name in a table
        {
            registerFont(&lf);
            s_smbxFontsMap[i] = FontManager::getFontID(lf.getFontName());
        }
    }

    if(!g_legacyFonts.empty())
        g_defaultRasterFont = &g_legacyFonts.front();

    s_fontMapsDefault = true;
    s_fontMapsWorld = true;
    g_fontManagerIsInit = true;
    g_fontManagerIsLegacy = true;
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
    if(!DirMan::exists(fonts_root))
    {
        pLogWarning("Can't load fonts as directory %s does not exists. Built-in fallback fonts will be loaded.", fonts_root.c_str());
        s_initFontsFallback();
        return;
    }

    DirListCI fontsRootCI(fonts_root);

    if(!s_loadFontsFromDir(fontsRootCI, std::string(), g_rasterFonts, LOADFONTARG(g_ttfFonts)))
    {
        pLogWarning("Can't load any font from the directory %s. Built-in fallback fonts will be loaded.", fonts_root.c_str());
        s_initFontsFallback();
        return;
    }

    if(g_anyFonts.empty())
    {
        pLogWarning("There is no available fonts at the directory %s. Fallback fonts will be loaded.", fonts_root.c_str());
        s_initFontsFallback();
        return;
    }

    s_fontMapsDefault = true;
    s_fontMapsWorld = true;
    g_fontManagerIsInit = true;
    g_fontManagerIsLegacy = false;
}

void FontManager::quit()
{
    clearAllCustomFonts();

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

void FontManager::loadCustomFonts()
{
    backupDefaultFontMaps();
    bool doLoadWorld = false;
    bool doLoadCustom = false;

    const std::string& episodeRoot = g_dirEpisode.getCurDir();
    const std::string& dataSubDir = g_dirCustom.getCurDir();

    // Loading fonts from the episode directory
    if(s_lastWorldFontsPath != episodeRoot && episodeRoot != AppPathManager::assetsRoot())
    {
        if(!s_lastWorldFontsPath.empty())
            clearAllCustomFonts();
        doLoadWorld = true;
        pLogDebug("Attempt to load custom fonts at %sfonts", episodeRoot.c_str());
    }
    else
        pLogDebug("Fonts at %sfonts already loaded", episodeRoot.c_str());

    if(doLoadWorld && s_loadFontsFromDir(g_dirEpisode,
                                         "fonts",
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

    if(doLoadCustom && s_loadFontsFromDir(g_dirCustom,
                                         "fonts",
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

bool FontManager::isLegacy()
{
    return g_fontManagerIsLegacy;
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

PGE_Size FontManager::glyphSize(const char* utf8char, uint32_t charNum, int fontId, uint32_t ttf_fontSize)
{
    SDL_assert_release(g_fontManagerIsInit);// Font manager is not initialized!
#ifndef THEXTECH_ENABLE_TTF_SUPPORT
    (void)ttf_fontSize;
#endif

    //Use one of loaded fonts
    if((fontId >= 0) && (static_cast<size_t>(fontId) < g_anyFonts.size()) && g_anyFonts[fontId])
    {
        if(g_anyFonts[fontId]->isLoaded())
            return g_anyFonts[fontId]->glyphSize(utf8char, charNum, ttf_fontSize);
    }

#ifdef THEXTECH_ENABLE_TTF_SUPPORT
    if(g_defaultTtfFont && g_defaultTtfFont->isLoaded())
        return g_defaultTtfFont->glyphSize(utf8char, charNum, ttf_fontSize);
#endif

    return PGE_Size(27 * 20, 1 * 20);
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
                            uint32_t ttf_FontSize, bool outline,
                            float outline_r, float outline_g, float outline_b)
{
    if(!g_fontManagerIsInit)
        return;

    if(!text || text_size == 0)
        return;

    BaseFontEngine* font_engine = nullptr;

    if((font >= 0) && (static_cast<size_t>(font) < g_anyFonts.size()) && g_anyFonts[font])
    {
        if(g_anyFonts[font]->isLoaded())
            font_engine = g_anyFonts[font];
    }

    if(!font_engine)
    {
        switch(font)
        {
        case DefaultRaster:
            if(g_defaultRasterFont && g_defaultRasterFont->isLoaded())
            {
                font_engine = g_defaultRasterFont;
                break;
            } /*fallthrough*/
        case DefaultTTF_Font:
        default:
#ifdef THEXTECH_ENABLE_TTF_SUPPORT
            if(g_defaultTtfFont && g_defaultTtfFont->isLoaded())
                font_engine = g_defaultTtfFont;
#endif
            break;
        }

        if(!font_engine)
        {
            pLogWarning("Attempt to print text [%s] without any font being loaded", text);
            return;
        }
    }

    if(outline)
    {
        // take square of Alpha to match blend of normal text
        float outline_a = Alpha * Alpha;
        font_engine->printText(text, text_size, x - 2, y, outline_r, outline_g, outline_b, outline_a, ttf_FontSize);
        font_engine->printText(text, text_size, x + 2, y, outline_r, outline_g, outline_b, outline_a, ttf_FontSize);
        font_engine->printText(text, text_size, x, y - 2, outline_r, outline_g, outline_b, outline_a, ttf_FontSize);
        font_engine->printText(text, text_size, x, y + 2, outline_r, outline_g, outline_b, outline_a, ttf_FontSize);
    }

    font_engine->printText(text, text_size, x, y, Red, Green, Blue, Alpha, ttf_FontSize);
}

PGE_Size FontManager::optimizeText(std::string &text, size_t max_columns)
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
            if((maxWidth < x) && (maxWidth < max_columns))
                maxWidth = x;
            x = 0;
            count++;
            break;
        }

        if(x >= max_columns) //If lenght more than allowed
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

    return PGE_Size(count, static_cast<int>(maxWidth));
}


PGE_Size FontManager::printTextOptiCol(std::string text,
                                   int x, int y,
                                   size_t max_columns,
                                   int font,
                                   float Red, float Green, float Blue, float Alpha,
                                   uint32_t ttf_FontSize)
{
    PGE_Size ret = FontManager::optimizeText(text, max_columns);
    FontManager::printText(text.c_str(), text.size(),
                           x, y,
                           font,
                           Red, Green, Blue, Alpha,
                           ttf_FontSize);
    return ret;
}

PGE_Size FontManager::printTextOptiPx(std::string text,
                                  int x, int y,
                                  size_t max_pixels_lenght,
                                  int font,
                                  float Red, float Green, float Blue, float Alpha,
                                  uint32_t ttf_FontSize)
{
    PGE_Size ret = FontManager::optimizeTextPx(text, max_pixels_lenght, font, ttf_FontSize);
    FontManager::printText(text.c_str(), text.size(),
                           x, y,
                           font,
                           Red, Green, Blue, Alpha,
                           ttf_FontSize);
    return ret;
}

PGE_Size FontManager::optimizeTextPx(std::string& text,
                                     size_t max_pixels_lenght,
                                     int fontId,
                                     uint32_t ttf_FontSize)
{
    /****************Word wrap*********************/
    size_t  lastspace = 0;
    int     count = 1;
    size_t  height = 0;
    size_t  maxWidth = 0;
    size_t  maxWidthAtSpace = 0;
    size_t  pixelWidth = 0;
    size_t  pixelWidthPrev = 0;
    BaseFontEngine *font = nullptr;
    PGE_Size gs;

    if((fontId < 0) || (static_cast<size_t>(fontId) >= g_anyFonts.size()) || !g_anyFonts[fontId])
        return PGE_Size(0, 0); // Invalid font

    font = g_anyFonts[fontId];

    for(size_t x = 0, i = 0; i < text.size(); i++, x++)
    {
        switch(text[i])
        {
        case '\t':
        case ' ':
            lastspace = i;
            maxWidthAtSpace = pixelWidth;
            gs = font->glyphSize(&text[i], x, ttf_FontSize);
            pixelWidthPrev = pixelWidth;
            pixelWidth += gs.w();
            break;

        case '\n':
            lastspace = 0;
            maxWidthAtSpace = 0;

            if((maxWidth < pixelWidth) && (maxWidth < max_pixels_lenght))
                maxWidth = pixelWidth;

            x = 0;
            pixelWidth = 0;
            pixelWidthPrev = 0;
            height += gs.h();
            count++;
            break;

        default:
            pixelWidthPrev = pixelWidth;
            gs = font->glyphSize(&text[i], x, ttf_FontSize);
            pixelWidth += gs.w();
            break;
        }

        if(pixelWidth >= max_pixels_lenght) //If lenght more than allowed
        {
            if(lastspace > 0)
            {
                if(maxWidth < maxWidthAtSpace)
                    maxWidth = maxWidthAtSpace;
                text[lastspace] = '\n';
                i = lastspace - 1;
                lastspace = 0;
            }
            else
            {
                if(maxWidth < pixelWidthPrev)
                    maxWidth = pixelWidthPrev;
                text.insert(i, 1, '\n');
                x = 0;
                pixelWidth = 0;
                height += gs.h();
                count++;
            }
        }

        UTF8 uch = static_cast<unsigned char>(text[i]);
        i += static_cast<size_t>(trailingBytesForUTF8[uch]);
    }

    if(count == 1)
        maxWidth = pixelWidth;

    /****************Word wrap*end*****************/

    return PGE_Size(maxWidth, height);
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

