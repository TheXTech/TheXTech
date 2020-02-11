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

#ifndef PGE_SizeF_H
#define PGE_SizeF_H

class PGE_Size;
class PGE_SizeF
{
    friend class PGE_Size;
public:
    PGE_SizeF();
    PGE_SizeF(const PGE_SizeF &p) = default;
    PGE_SizeF(const PGE_Size &p);
    PGE_SizeF(double w, double h);
    ~PGE_SizeF();
    void setSize(double w, double h);
    void setWidth(double w);
    void setHeight(double h);
    bool isNull() const;
    double w() const;
    double h() const;
private:
    double m_w;
    double m_h;
};

#endif // PGE_SizeF_H
