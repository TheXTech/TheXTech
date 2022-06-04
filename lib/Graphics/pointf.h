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

#ifndef PGE_POINTF_H
#define PGE_POINTF_H

class PGE_Point;
class PGE_PointF
{
    friend class PGE_Point;
public:
    inline PGE_PointF()
    {
        setPoint(0, 0);
    }

    inline PGE_PointF(const PGE_PointF &p) = default;

    PGE_PointF(const PGE_Point &p);

    inline PGE_PointF(double x, double y)
    {
        m_x = x;
        m_y = y;
    }

    inline ~PGE_PointF() {}

    inline void setPoint(double x, double y)
    {
        m_x = x;
        m_y = y;
    }

    inline void setX(double x)
    {
        m_x = x;
    }

    inline void setY(double y)
    {
        m_y = y;
    }

    inline bool isNull() const
    {
        return ((m_x == 0.0) && (m_y == 0.0));
    }

    inline double x() const
    {
        return m_x;
    }

    inline double y() const
    {
        return m_y;
    }

    friend bool operator==(const PGE_PointF &lhs, const PGE_PointF &rhs);
    friend bool operator!=(const PGE_PointF &lhs, const PGE_PointF &rhs);

private:
    double m_x;
    double m_y;
};

bool operator==(const PGE_PointF &lhs, const PGE_PointF &rhs);
bool operator!=(const PGE_PointF &lhs, const PGE_PointF &rhs);

#endif // PGE_POINTF_H
