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

#include "pointf.h"
#include "point.h"
#include <Utils/maths.h>

PGE_PointF::PGE_PointF(const PGE_Point &p)
{
    m_x = p.m_x;
    m_y = p.m_y;
}

bool operator==(const PGE_PointF &lhs, const PGE_PointF &rhs)
{
    return (Maths::equals(lhs.m_x, rhs.m_x) && Maths::equals(lhs.m_y, rhs.m_y));
}

bool operator!=(const PGE_PointF &lhs, const PGE_PointF &rhs)
{
    return (!Maths::equals(lhs.m_x, rhs.m_x) || Maths::equals(lhs.m_y, rhs.m_y));
}
