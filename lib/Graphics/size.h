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

#ifndef PGE_Size_H
#define PGE_Size_H

class PGE_SizeF;
class PGE_Size
{
    friend class PGE_SizeF;
public:
    PGE_Size();
    PGE_Size(const PGE_Size &p) = default;
    PGE_Size(const PGE_SizeF &p);
    PGE_Size(int w, int h);
    ~PGE_Size();
    void setSize(int w, int h);
    void setWidth(int w);
    void setHeight(int h);
    bool isNull() const;
    int w() const;
    int h() const;
private:
    int m_w;
    int m_h;
};

#endif // PGE_Size_H
