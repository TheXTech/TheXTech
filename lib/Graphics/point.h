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

#ifndef PGE_Point_H
#define PGE_Point_H

class PGE_PointF;
class PGE_Point
{
    friend class PGE_PointF;
public:
    PGE_Point();
    PGE_Point(const PGE_Point &p) = default;
    PGE_Point(const PGE_PointF &p);
    PGE_Point(int x, int y);
    ~PGE_Point() = default;
    void setPoint(int x, int y);
    void setX(int x);
    void setY(int y);
    bool isNull() const;
    int x() const;
    int y() const;
    friend bool operator==(const PGE_Point &lhs, const PGE_Point &rhs);
    friend bool operator!=(const PGE_Point &lhs, const PGE_Point &rhs);

private:
    int m_x;
    int m_y;
};

bool operator==(const PGE_Point &lhs, const PGE_Point &rhs);
bool operator!=(const PGE_Point &lhs, const PGE_Point &rhs);

#endif // PGE_Point_H
