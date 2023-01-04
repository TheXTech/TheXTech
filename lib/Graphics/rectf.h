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

#ifndef PGE_RECTF_H
#define PGE_RECTF_H

class PGE_Rect;
class PGE_PointF;
class PGE_SizeF;

class PGE_RectF
{
    friend class PGE_Rect;
public:
    PGE_RectF();
    PGE_RectF(double x, double y, double w, double h);
    PGE_RectF(const PGE_RectF &r) = default;
    PGE_RectF(const PGE_Rect &r);
    ~PGE_RectF();
    void setRect(double x, double y, double w, double h);
    void setPos(double x, double y);
    void setSize(double w, double h);

    void setLeft(double l);
    void setRight(double r);
    void setTop(double t);
    void setBottom(double b);
    PGE_RectF withMargin(double margin) const;
    PGE_RectF withMargin(double marginX, double marginY) const;

    void setX(double x);
    void setY(double y);
    void setWidth(double w);
    void setHeight(double h);
    void addX(double x);
    void addY(double y);

    void setTopLeft(PGE_PointF p);
    void setTopRight(PGE_PointF p);
    void setBottomRight(PGE_PointF p);
    void setBottomLeft(PGE_PointF p);

    void setTopLeft(double l, double t);
    void setTopRight(double r, double t);
    void setBottomRight(double r, double b);
    void setBottomLeft(double l, double b);

    double x() const;
    double y() const;

    double left() const;
    double top() const;
    double bottom() const;
    double right() const;

    double width() const;
    double height() const;

    PGE_PointF center() const;
    PGE_SizeF size() const;
    double centerX() const;
    double centerY() const;

    bool collidePoint(double x, double y) const;
    bool collideRect(double x, double y, double w, double h) const;
    bool collideRect(PGE_RectF &rect) const;
    bool collideRectDeep(PGE_RectF &rect, double depth) const;
    bool collideRectDeep(PGE_RectF &rect, double deepX, double deepY) const;
    bool collideRect(PGE_Rect &rect) const;

private:
    double m_x;
    double m_y;
    double m_w;
    double m_h;
    double m_r;
    double m_b;
};

#endif // PGE_RECTF_H
