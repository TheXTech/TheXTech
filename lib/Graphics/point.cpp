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

#include "point.h"
#include "pointf.h"

PGE_Point::PGE_Point()
{
    setPoint(0, 0);
}

PGE_Point::PGE_Point(const PGE_PointF &p)
{
    m_x = static_cast<int>(p.m_x);
    m_y = static_cast<int>(p.m_y);
}

PGE_Point::PGE_Point(int x, int y)
{
    m_x = x;
    m_y = y;
}

PGE_Point::~PGE_Point()
{}

void PGE_Point::setPoint(int x, int y)
{
    m_x = x;
    m_y = y;
}

void PGE_Point::setX(int x)
{
    m_x = x;
}

void PGE_Point::setY(int y)
{
    m_y = y;
}

bool PGE_Point::isNull() const
{
    return ((m_x == 0) && (m_y == 0));
}

int PGE_Point::x() const
{
    return m_x;
}

int PGE_Point::y() const
{
    return m_y;
}

bool operator==(const PGE_Point &lhs, const PGE_Point &rhs)
{
    return ((lhs.m_x == rhs.m_x) && (lhs.m_y == rhs.m_y));
}

bool operator!=(const PGE_Point &lhs, const PGE_Point &rhs)
{
    return ((lhs.m_x != rhs.m_x) || (lhs.m_y != rhs.m_y));
}
