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

PGE_SizeF::PGE_SizeF()
{
    setSize(0, 0);
}

PGE_SizeF::PGE_SizeF(const PGE_Size &p)
{
    m_w = static_cast<double>(p.m_w);
    m_h = static_cast<double>(p.m_h);
}

PGE_SizeF::PGE_SizeF(double w, double h)
{
    m_w = w;
    m_h = h;
}

PGE_SizeF::~PGE_SizeF()
{}

void PGE_SizeF::setSize(double w, double h)
{
    m_w = w;
    m_h = h;
}

void PGE_SizeF::setWidth(double w)
{
    m_w = w;
}

void PGE_SizeF::setHeight(double h)
{
    m_h = h;
}

bool PGE_SizeF::isNull() const
{
    return ((m_w == 0.0) && (m_h == 0.0));
}

double PGE_SizeF::w() const
{
    return m_w;
}

double PGE_SizeF::h() const
{
    return m_h;
}
