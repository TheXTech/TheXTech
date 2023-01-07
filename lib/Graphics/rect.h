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

#ifndef PGE_RECT_H
#define PGE_RECT_H

class PGE_RectF;
class PGE_Point;
class PGE_Size;

class PGE_Rect
{
    friend class PGE_RectF;
public:
    PGE_Rect();
    PGE_Rect(int x, int y, int w, int h);
    PGE_Rect(const PGE_Rect &r) = default;
    PGE_Rect(const PGE_RectF &r);
    ~PGE_Rect();
    void setRect(int x, int y, int w, int h);
    void setPos(int x, int y);
    void setSize(int w, int h);

    void setLeft(int l);
    void setRight(int r);
    void setTop(int t);
    void setBottom(int b);

    void setX(int x);
    void setY(int y);
    void setWidth(int w);
    void setHeight(int h);
    void addX(int x);
    void addY(int y);

    void setTopLeft(PGE_Point p);
    void setTopRight(PGE_Point p);
    void setBottomRight(PGE_Point p);
    void setBottomLeft(PGE_Point p);

    void setTopLeft(int l, int t);
    void setTopRight(int r, int t);
    void setBottomRight(int r, int b);
    void setBottomLeft(int l, int b);

    int x() const;
    int y() const;

    int left() const;
    int top() const;
    int bottom() const;
    int right() const;

    int width() const;
    int height() const;

    PGE_Point center() const;
    PGE_Size size() const;
    int centerX() const;
    int centerY() const;

    bool collidePoint(int x, int y) const;
    bool collideRect(int x, int y, int w, int h) const;
    bool collideRect(PGE_Rect &rect) const;
    bool collideRect(PGE_RectF &rect) const;
private:
    int m_x;
    int m_y;
    int m_w;
    int m_h;
    int m_r;
    int m_b;
};

#endif // PGE_RECT_H
