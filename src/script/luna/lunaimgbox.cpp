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

#ifndef PGE_NO_THREADING
#include <atomic>
#endif

#include "lunaimgbox.h"
#include "core/render.h"
#include <Utils/files.h>

uint64_t LunaImage::p_uidCounter = 1;

uint64_t LunaImage::getNewUID()
{
    return p_uidCounter++;
}

LunaImage::LunaImage()
{
    Init();
}

LunaImage::~LunaImage()
{}

LunaImage::LunaImage(const std::string &filename)
{
    if(filename.empty())
        return;

    Init();

    m_image = XRender::lazyLoadPicture(filename);
    if(Files::hasSuffix(filename, ".jpg") || Files::hasSuffix(filename, ".bmp"))
    {
        m_useTransColor = true;
        XRender::setTransparentColor(m_image, m_TransColor);
    }

    m_uid = getNewUID();

    m_W = m_image.w;
    m_H = m_image.h;
}

LunaImage::LunaImage(const LunaImage &o)
{
    operator=(o);
}

LunaImage &LunaImage::operator=(const LunaImage &o)
{
    m_H = o.m_H;
    m_W = o.m_W;
    m_uid = o.m_uid;
    m_TransColor = o.m_TransColor;
    m_image = o.m_image;
    return *this;
}

void LunaImage::Init()
{
    m_W = 0;
    m_H = 0;
    m_uid = 0;
    m_TransColor = DEFAULT_TRANS_COLOR;
}

void LunaImage::Unload()
{
    XRender::deleteTexture(m_image);
    Init();
}

bool LunaImage::ImageLoaded()
{
    return m_image.inited;
}

void LunaImage::setTransparentColor(uint32_t rgb)
{
    m_TransColor = rgb;
    if(m_useTransColor)
        XRender::setTransparentColor(m_image, rgb);
}
