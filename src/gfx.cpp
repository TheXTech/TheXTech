#include "globals.h"
#include "gfx.h"
#include "frm_main.h"
#include <AppPath/app_path.h>
#include <fmt_format_ne.h>
#include <Logger/logger.h>

void GFX::loadImage(StdPicture &img, std::string path)
{
    pLogDebug("Loading texture %s...", path.c_str());
    img = frmMain.LoadPicture(path);
    if(!img.texture)
        pLogWarning("Failed to load texture: %s...", path.c_str());
    m_loadedImages.push_back(&img);
}

GFX::GFX()
{}

void GFX::load()
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

    loadImage(Mount, uiPath + "Mount.png");

    For(i, 0, 7)
        loadImage(nCursor[i], uiPath + fmt::format_ne("nCursor{0}.png", i));

    loadImage(TextBox, uiPath + "TextBox.png");

    For(i, 1, 2)
        loadImage(Tongue[i], uiPath + fmt::format_ne("Tongue{0}.png", i));

    loadImage(Warp, uiPath + "Warp.png");

    loadImage(YoshiWings, uiPath + "YoshiWings.png");
}

void GFX::unLoad()
{
    for(StdPicture *p : m_loadedImages)
        frmMain.deleteTexture(*p);
    m_loadedImages.clear();
}
