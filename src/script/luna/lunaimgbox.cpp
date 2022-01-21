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

#include <atomic>

#include "lunaimgbox.h"
#include "core/render.h"


uint64_t LunaImage::getNewUID()
{
    static std::atomic<uint64_t> uidCounter(1);
    return uidCounter.fetch_add(1, std::memory_order_relaxed);
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
