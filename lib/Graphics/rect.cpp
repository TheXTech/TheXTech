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

#include "rect.h"
#include "rectf.h"
#include "point.h"
#include "size.h"

PGE_Rect::PGE_Rect()
{
    setRect(0, 0, 0, 0);
}

PGE_Rect::PGE_Rect(int x, int y, int w, int h)
{
    setRect(x, y,  w, h);
}

PGE_Rect::PGE_Rect(const PGE_RectF &r)
{
    m_x = static_cast<int>(r.m_x);
    m_y = static_cast<int>(r.m_y);
    m_w = static_cast<int>(r.m_w);
    m_h = static_cast<int>(r.m_h);
    m_r = static_cast<int>(r.m_r);
    m_b = static_cast<int>(r.m_b);
}

PGE_Rect::~PGE_Rect()
{}

void PGE_Rect::setRect(int x, int y, int w, int h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;
    m_r = m_x + m_w;
    m_b = m_y + m_h;
}

void PGE_Rect::setPos(int x, int y)
{
    m_x = x;
    m_y = y;
    m_r = m_x + m_w;
    m_b = m_y + m_h;
}

void PGE_Rect::setSize(int w, int h)
{
    m_w = w;
    m_h = h;
    m_r = m_x + m_w;
    m_b = m_y + m_h;
}

void PGE_Rect::setLeft(int l)
{
    m_x = l;
    m_w = m_r - m_x;
}

void PGE_Rect::setRight(int r)
{
    m_r = r;
    m_w = m_r - m_x;
}

void PGE_Rect::setTop(int t)
{
    m_y = t;
    m_h = m_b - m_y;
}

void PGE_Rect::setBottom(int b)
{
    m_b = b;
    m_h = m_b - m_y;
}


void PGE_Rect::setX(int x)
{
    m_x = x;
    m_r = m_x + m_w;
}

void PGE_Rect::setY(int y)
{
    m_y = y;
    m_b = m_y + m_h;
}

void PGE_Rect::setWidth(int w)
{
    m_w = w;
    m_r = m_x + m_w;
}

void PGE_Rect::setHeight(int h)
{
    m_h = h;
    m_b = m_y + m_h;
}

void PGE_Rect::addX(int x)
{
    m_x += x;
    m_r = m_x + m_w;
}

void PGE_Rect::addY(int y)
{
    m_y += y;
    m_b = m_y + m_h;
}

void PGE_Rect::setTopLeft(PGE_Point p)
{
    setLeft(p.x());
    setTop(p.y());
}

void PGE_Rect::setTopRight(PGE_Point p)
{
    setTop(p.y());
    setRight(p.x());
}

void PGE_Rect::setBottomRight(PGE_Point p)
{
    setRight(p.x());
    setBottom(p.y());
}

void PGE_Rect::setBottomLeft(PGE_Point p)
{
    setLeft(p.x());
    setBottom(p.y());
}

void PGE_Rect::setTopLeft(int l, int t)
{
    setLeft(l);
    setTop(t);
}

void PGE_Rect::setTopRight(int r, int t)
{
    setTop(t);
    setRight(r);
}

void PGE_Rect::setBottomRight(int r, int b)
{
    setBottom(b);
    setRight(r);
}

void PGE_Rect::setBottomLeft(int l, int b)
{
    setBottom(b);
    setLeft(l);
}



int PGE_Rect::x() const
{
    return m_x;
}

int PGE_Rect::y() const
{
    return m_y;
}

int PGE_Rect::left() const
{
    return m_x;
}

int PGE_Rect::top() const
{
    return m_y;
}

int PGE_Rect::bottom() const
{
    return m_b;
}

int PGE_Rect::right() const
{
    return m_r;
}

int PGE_Rect::width() const
{
    return m_w;
}

int PGE_Rect::height() const
{
    return m_h;
}

PGE_Point PGE_Rect::center() const
{
    return PGE_Point(m_x + (m_w / 2), m_y + (m_h / 2));
}

PGE_Size PGE_Rect::size() const
{
    return PGE_Size(m_w, m_h);
}

int PGE_Rect::centerX() const
{
    return static_cast<int>(m_x + (m_w / 2));
}

int PGE_Rect::centerY() const
{
    return static_cast<int>(m_y + (m_h / 2));
}

bool PGE_Rect::collidePoint(int x, int y) const
{
    if(x < m_x) return false;
    if(y < m_y) return false;
    if(x > m_r) return false;
    if(y > m_b) return false;
    return true;
}

bool PGE_Rect::collideRect(int x, int y, int w, int h) const
{
    if((x + w) < m_x) return false;
    if((y + h) < m_y) return false;
    if(x > m_r) return false;
    if(y > m_b) return false;
    return true;
}

bool PGE_Rect::collideRect(PGE_Rect &rect) const
{
    if(rect.m_r < m_x) return false;
    if(rect.m_b < m_y) return false;
    if(rect.m_x > m_r) return false;
    if(rect.m_y > m_b) return false;
    return true;
}

bool PGE_Rect::collideRect(PGE_RectF &rect) const
{
    if(static_cast<int>(rect.m_r) < m_x) return false;
    if(static_cast<int>(rect.m_b) < m_y) return false;
    if(static_cast<int>(rect.m_x) > m_r) return false;
    if(static_cast<int>(rect.m_y) > m_b) return false;
    return true;
}
