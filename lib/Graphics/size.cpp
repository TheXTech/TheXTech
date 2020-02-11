/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2020 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sizef.h"
#include "size.h"

PGE_Size::PGE_Size()
{
    setSize(0, 0);
}

PGE_Size::PGE_Size(const PGE_SizeF &p)
{
    m_w = static_cast<int>(p.m_w);
    m_h = static_cast<int>(p.m_h);
}

PGE_Size::PGE_Size(int w, int h)
{
    m_w = w;
    m_h = h;
}

PGE_Size::~PGE_Size()
{}

void PGE_Size::setSize(int w, int h)
{
    m_w = w;
    m_h = h;
}

void PGE_Size::setWidth(int w)
{
    m_w = w;
}

void PGE_Size::setHeight(int h)
{
    m_h = h;
}

bool PGE_Size::isNull() const
{
    return ((m_w == 0) && (m_h == 0));
}

int PGE_Size::w() const
{
    return m_w;
}

int PGE_Size::h() const
{
    return m_h;
}
