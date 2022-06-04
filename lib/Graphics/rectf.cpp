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

#include "rectf.h"
#include "rect.h"
#include "pointf.h"
#include "sizef.h"

PGE_RectF::PGE_RectF()
{
    setRect(0.0, 0.0, 0.0, 0.0);
}

PGE_RectF::PGE_RectF(double x, double y, double w, double h)
{
    setRect(x, y,  w, h);
}

PGE_RectF::PGE_RectF(const PGE_Rect &r)
{
    m_x = static_cast<double>(r.m_x);
    m_y = static_cast<double>(r.m_y);
    m_w = static_cast<double>(r.m_w);
    m_h = static_cast<double>(r.m_h);
    m_r = static_cast<double>(r.m_r);
    m_b = static_cast<double>(r.m_b);
}

PGE_RectF::~PGE_RectF()
{}

void PGE_RectF::setRect(double x, double y, double w, double h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;
    m_r = m_x + m_w;
    m_b = m_y + m_h;
}

void PGE_RectF::setPos(double x, double y)
{
    m_x = x;
    m_y = y;
    m_r = m_x + m_w;
    m_b = m_y + m_h;
}

void PGE_RectF::setSize(double w, double h)
{
    m_w = w;
    m_h = h;
    m_r = m_x + m_w;
    m_b = m_y + m_h;
}

void PGE_RectF::setLeft(double l)
{
    m_x = l;
    m_w = m_r - m_x;
}

void PGE_RectF::setRight(double r)
{
    m_r = r;
    m_w = m_r - m_x;
}

void PGE_RectF::setTop(double t)
{
    m_y = t;
    m_h = m_b - m_y;
}

void PGE_RectF::setBottom(double b)
{
    m_b = b;
    m_h = m_b - m_y;
}

PGE_RectF PGE_RectF::withMargin(double margin) const
{
    PGE_RectF rect;
    rect.m_x = m_x - margin;
    rect.m_y = m_y - margin;
    rect.m_b = m_b + margin;
    rect.m_r = m_r + margin;
    rect.m_w = m_w + margin * 2.0;
    rect.m_h = m_h + margin * 2.0;
    return rect;
}

PGE_RectF PGE_RectF::withMargin(double marginX, double marginY) const
{
    PGE_RectF rect;
    rect.m_x = m_x - marginX;
    rect.m_y = m_y - marginY;
    rect.m_b = m_b + marginX;
    rect.m_r = m_r + marginY;
    rect.m_w = m_w + marginX * 2.0;
    rect.m_h = m_h + marginY * 2.0;
    return rect;
}


void PGE_RectF::setX(double x)
{
    m_x = x;
    m_r = m_x + m_w;
}

void PGE_RectF::setY(double y)
{
    m_y = y;
    m_b = m_y + m_h;
}

void PGE_RectF::setWidth(double w)
{
    m_w = w;
    m_r = m_x + m_w;
}

void PGE_RectF::setHeight(double h)
{
    m_h = h;
    m_b = m_y + m_h;
}

void PGE_RectF::addX(double x)
{
    m_x += x;
    m_r = m_x + m_w;
}

void PGE_RectF::addY(double y)
{
    m_y += y;
    m_b = m_y + m_h;
}

void PGE_RectF::setTopLeft(PGE_PointF p)
{
    setLeft(p.x());
    setTop(p.y());
}

void PGE_RectF::setTopRight(PGE_PointF p)
{
    setTop(p.y());
    setRight(p.x());
}

void PGE_RectF::setBottomRight(PGE_PointF p)
{
    setRight(p.x());
    setBottom(p.y());
}

void PGE_RectF::setBottomLeft(PGE_PointF p)
{
    setLeft(p.x());
    setBottom(p.y());
}


void PGE_RectF::setTopLeft(double l, double t)
{
    setLeft(l);
    setTop(t);
}

void PGE_RectF::setTopRight(double r, double t)
{
    setTop(t);
    setRight(r);
}

void PGE_RectF::setBottomRight(double r, double b)
{
    setBottom(b);
    setRight(r);
}

void PGE_RectF::setBottomLeft(double l, double b)
{
    setBottom(b);
    setLeft(l);
}



double PGE_RectF::x() const
{
    return m_x;
}

double PGE_RectF::y() const
{
    return m_y;
}

double PGE_RectF::left() const
{
    return m_x;
}

double PGE_RectF::top() const
{
    return m_y;
}

double PGE_RectF::bottom() const
{
    return m_b;
}

double PGE_RectF::right() const
{
    return m_r;
}

double PGE_RectF::width() const
{
    return m_w;
}

double PGE_RectF::height() const
{
    return m_h;
}

PGE_PointF PGE_RectF::center() const
{
    return PGE_PointF(m_x + m_w / 2.0, m_y + m_h / 2.0);
}

PGE_SizeF PGE_RectF::size() const
{
    return PGE_SizeF(m_w, m_h);
}

double PGE_RectF::centerX() const
{
    return m_x + m_w / 2.0;
}

double PGE_RectF::centerY() const
{
    return m_y + m_h / 2.0;
}

bool PGE_RectF::collidePoint(double x, double y) const
{
    if(x < m_x) return false;
    if(y < m_y) return false;
    if(x > m_r) return false;
    if(y > m_b) return false;
    return true;
}

bool PGE_RectF::collideRect(double x, double y, double w, double h) const
{
    if((x + w) < m_x) return false;
    if((y + h) < m_y) return false;
    if(x > m_r) return false;
    if(y > m_b) return false;
    return true;
}

bool PGE_RectF::collideRect(PGE_RectF &rect) const
{
    if(rect.m_r < m_x) return false;
    if(rect.m_b < m_y) return false;
    if(rect.m_x > m_r) return false;
    if(rect.m_y > m_b) return false;
    return true;
}

bool PGE_RectF::collideRectDeep(PGE_RectF &rect, double depth) const
{
    if(rect.m_r < (m_x + depth)) return false;
    if(rect.m_b < (m_y + depth)) return false;
    if(rect.m_x > (m_r - depth)) return false;
    if(rect.m_y > (m_b - depth)) return false;
    return true;
}

bool PGE_RectF::collideRectDeep(PGE_RectF &rect, double deepX, double deepY) const
{
    if(rect.m_r < (m_x + deepX)) return false;
    if(rect.m_b < (m_y + deepY)) return false;
    if(rect.m_x > (m_r - deepX)) return false;
    if(rect.m_y > (m_b - deepY)) return false;
    return true;
}

bool PGE_RectF::collideRect(PGE_Rect &rect) const
{
    if(rect.m_r < int(m_x)) return false;
    if(rect.m_b < int(m_y)) return false;
    if(rect.m_x > int(m_r)) return false;
    if(rect.m_y > int(m_b)) return false;
    return true;
}
