/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <IniProcessor/ini_processing.h>
#include <Logger/logger.h>

#include "core/render.h"

#include "graphics/gfx_frame.h"

void loadFrameInfo(IniProcessing& ini, FrameBorderInfo& info)
{
	info = FrameBorderInfo();

	ini.beginGroup("border-info");
	ini.read("te", info.te, info.te);
	ini.read("ti", info.ti, info.ti);
	ini.read("be", info.be, info.be);
	ini.read("bi", info.bi, info.bi);
	ini.read("le", info.le, info.le);
	ini.read("li", info.li, info.li);
	ini.read("re", info.re, info.re);
	ini.read("ri", info.ri, info.ri);
	ini.endGroup();
}

void DrawTextureTiled(int dst_x, int dst_y, int dst_w, int dst_h, StdPicture& tx, int src_x, int src_y, int src_w, int src_h, int off_x, int off_y, XTColor color)
{
    if(off_x == -1)
        off_x = dst_x;
    if(off_y == -1)
        off_y = dst_y;
    if(src_w == -1)
        src_w = tx.w;
    if(src_h == -1)
        src_h = tx.h;

    int c_off_x = off_x % src_w;
    // want modulus, not remainder
    if(c_off_x < 0)
    	c_off_x += src_w;

    for(int x = dst_x; x < dst_x + dst_w;)
    {
        int render_w = src_w - c_off_x;
        if(x + render_w > dst_x + dst_w)
            render_w = dst_x + dst_w - x;

        int c_off_y = off_y % src_h;
        // want modulus, not remainder
        if(c_off_y < 0)
        	c_off_y += src_h;

        for(int y = dst_y; y < dst_y + dst_h;)
        {
            int render_h = src_h - c_off_y;
            if(y + render_h > dst_y + dst_h)
                render_h = dst_y + dst_h - y;

            XRender::renderTexture(x, y, render_w, render_h, tx, src_x + c_off_x, src_y + c_off_y, color);

            y += src_h - c_off_y;
            c_off_y = 0;
        }

        x += src_w - c_off_x;
        c_off_x = 0;
    }
}

// renders a new-style frame with the included external and internal locations
void RenderFrameBorder(const Location_t& external, const Location_t& internal,
	StdPicture& tile, FrameBorder* border)
{
	// first, verify to what extent the external and internal parts are the same
	bool have_l = external.X != internal.X;
	bool have_t = external.Y != internal.Y;
	bool have_r = external.X + external.Width != internal.X + internal.Width;
	bool have_b = external.Y + external.Height != internal.Y + internal.Height;


	// render a modern background: first the tile, then the border (if it exists)

	// top
	if(have_t)
	{
		DrawTextureTiled(external.X,
			external.Y,
			external.Width,
			internal.Y - external.Y,
			tile);
	}
	// left (excl top)
	if(have_l)
	{
		DrawTextureTiled(external.X,
			internal.Y,
			internal.X - external.X,
			external.Y + external.Height - internal.Y,
			tile);
	}
	// right (excl top)
	if(have_r)
	{
		DrawTextureTiled(internal.X + internal.Width,
			internal.Y,
			(external.X + external.Width) - (internal.X + internal.Width),
			external.Y + external.Height - internal.Y,
			tile);
	}
	// bottom (excl left and right)
	if(have_b)
	{
		DrawTextureTiled(internal.X,
			internal.Y + internal.Height,
			internal.Width,
			(external.Y + external.Height) - (internal.Y + internal.Height),
			tile);
	}

	if(!border || !border->tex.inited)
		return;

	FrameBorder& i = *border;

	// zero: check if the `ini` is invalid
	if(i.le + i.li + i.ri + i.re > i.tex.w)
		return;
	if(i.te + i.ti + i.bi + i.be > i.tex.h)
		return;

	int li = have_l ? i.li : 0;
	int ti = have_t ? i.ti : 0;
	int ri = have_r ? i.ri : 0;
	int bi = have_b ? i.bi : 0;

	// top external-left external
	if(have_l && have_t && i.le && i.te)
	{
		XRender::renderTexture(internal.X - i.le, internal.Y - i.te, i.le, i.te,
		    i.tex,
		    0, 0);
	}
	// top external-left internal
	if(have_l && have_t && i.li && i.te)
	{
		XRender::renderTexture(internal.X, internal.Y - i.te, i.li, i.te,
		    i.tex,
		    i.le, 0);
	}
	// top external-center
	if(have_t && i.te && internal.Width - li - ri > 0)
	{
		DrawTextureTiled(internal.X + li, internal.Y - i.te, internal.Width - li - ri, i.te,
		    i.tex,
		    i.le + i.li, 0,
		    i.tex.w - i.le - i.li - i.ri - i.re, i.te,
		    0, 0);
	}
	// top external-right internal
	if(have_t && have_r && i.ri && i.te)
	{
		XRender::renderTexture(internal.X + internal.Width - i.ri, internal.Y - i.te, i.ri, i.te,
		    i.tex,
		    i.tex.w - i.ri - i.re, 0);
	}
	// top external-right external
	if(have_t && have_r && i.re && i.te)
	{
		XRender::renderTexture(internal.X + internal.Width, internal.Y - i.te, i.re, i.te,
		    i.tex,
		    i.tex.w - i.re, 0);
	}
	// top internal-left external
	if(have_t && have_l && i.le && i.ti)
	{
		XRender::renderTexture(internal.X - i.le, internal.Y, i.le, i.ti,
		    i.tex,
		    0, i.te);
	}
	// center-left external
	if(have_l && i.le && internal.Height - ti - bi > 0)
	{
		DrawTextureTiled(internal.X - i.le, internal.Y + ti, i.le, internal.Height - ti - bi,
		    i.tex,
		    0, i.te + i.ti,
		    i.le,
		    i.tex.h - i.te - i.ti - i.bi - i.be,
		    0, 0);
	}
	// bottom internal-left external
	if(have_b && have_l && i.le && i.bi)
	{
		XRender::renderTexture(internal.X - i.le, internal.Y + internal.Height - i.bi, i.le, i.bi,
		    i.tex,
		    0, i.tex.h - i.bi - i.be);
	}
	// bottom external-left external
	if(have_b && have_l && i.le && i.be)
	{
		XRender::renderTexture(internal.X - i.le, internal.Y + internal.Height, i.le, i.be,
		    i.tex,
		    0, i.tex.h - i.be);
	}
	// bottom external-left internal
	if(have_b && have_l && i.li && i.be)
	{
		XRender::renderTexture(internal.X, internal.Y + internal.Height, i.li, i.be,
		    i.tex,
		    i.le, i.tex.h - i.be);
	}
	// bottom external-center
	if(have_b && i.be && internal.Width - li - ri > 0)
	{
		DrawTextureTiled(internal.X + li, internal.Y + internal.Height, internal.Width - li - ri, i.be,
		    i.tex,
		    i.le + i.li, i.tex.h - i.be, i.tex.w - i.le - i.li - i.ri - i.re, i.be,
		    0, 0);
	}
	// bottom external-right internal
	if(have_b && have_r && i.ri && i.be)
	{
		XRender::renderTexture(internal.X + internal.Width - i.ri, internal.Y + internal.Height, i.ri, i.be,
		    i.tex,
		    i.tex.w - i.ri - i.re, i.tex.h - i.be);
	}
	// bottom external-right external
	if(have_b && have_r && i.re && i.be)
	{
		XRender::renderTexture(internal.X + internal.Width, internal.Y + internal.Height, i.re, i.be,
		    i.tex,
		    i.tex.w - i.re, i.tex.h - i.be);
	}
	// top internal-right external
	if(have_t && have_r && i.re && i.ti)
	{
		XRender::renderTexture(internal.X + internal.Width, internal.Y, i.re, i.ti,
		    i.tex,
		    i.tex.w - i.re, i.te);
	}
	// center-right external
	if(have_r && i.re && internal.Height - ti - bi)
	{
		DrawTextureTiled(internal.X + internal.Width, internal.Y + ti, i.re, internal.Height - ti - bi,
		    i.tex,
		    i.tex.w - i.re, i.te + i.ti,
		    i.re, i.tex.h - i.te - i.ti - i.bi - i.be,
		    0, 0);
	}
	// bottom internal-right external
	if(have_b && have_r && i.bi && i.re)
	{
		XRender::renderTexture(internal.X + internal.Width, internal.Y + internal.Height - i.bi, i.re, i.bi,
		    i.tex,
		    i.tex.w - i.re, i.tex.h - i.bi - i.be);
	}
}

// renders a new-style frame as a fill at the included internal location
// the frame fills the internal rect
void RenderFrameFill(const Location_t& internal, FrameBorder& frame, XTColor color)
{
	FrameBorder& i = frame;

	// zero: check if the `ini` is invalid
	if(i.le + i.li + i.ri + i.re > i.tex.w)
		return;
	if(i.te + i.ti + i.bi + i.be > i.tex.h)
		return;

	// top internal-left internal
	if(i.li && i.ti)
	{
		XRender::renderTexture(internal.X, internal.Y, i.li, i.ti,
		    i.tex,
		    i.le, i.te,
		    color);
	}
	// top internal-center
	if(i.ti && internal.Width - i.li - i.ri > 0)
	{
		DrawTextureTiled(internal.X + i.li, internal.Y, internal.Width - i.li - i.ri, i.ti,
		    i.tex,
		    i.le + i.li, i.te,
		    i.tex.w - i.le - i.li - i.ri - i.re,
		    i.ti,
		    0, 0,
		    color);
	}
	// top internal-right internal
	if(i.ri && i.ti)
	{
		XRender::renderTexture(internal.X + internal.Width - i.ri, internal.Y, i.ri, i.ti,
		    i.tex,
		    i.tex.w - i.ri - i.re, i.te,
		    color);
	}
	// center-left internal
	if(i.li && internal.Height - i.ti - i.bi > 0)
	{
		DrawTextureTiled(internal.X, internal.Y + i.ti, i.li, internal.Height - i.ti - i.bi,
		    i.tex,
		    i.le, i.te + i.ti,
		    i.li,
		    i.tex.h - i.te - i.ti - i.bi - i.be,
		    0, 0,
		    color);
	}
	// bottom internal-left internal
	if(i.li && i.bi)
	{
		XRender::renderTexture(internal.X, internal.Y + internal.Height - i.bi, i.li, i.bi,
		    i.tex,
		    i.le,
		    i.tex.h - i.bi - i.be,
		    color);
	}
	// bottom internal-center
	if(i.bi && internal.Width - i.li - i.ri > 0)
	{
		DrawTextureTiled(internal.X + i.li, internal.Y + internal.Height - i.bi, internal.Width - i.li - i.ri, i.bi,
		    i.tex,
		    i.le + i.li, i.tex.h - i.be - i.bi, i.tex.w - i.le - i.li - i.ri - i.re, i.bi,
		    0, 0,
		    color);
	}
	// bottom internal-right internal
	if(i.ri && i.bi)
	{
		XRender::renderTexture(internal.X + internal.Width - i.ri, internal.Y + internal.Height - i.bi, i.ri, i.bi,
		    i.tex,
		    i.tex.w - i.ri - i.re, i.tex.h - i.bi - i.be,
		    color);
	}
	// center-right internal
	if(i.ri && internal.Height - i.ti - i.bi)
	{
		DrawTextureTiled(internal.X + internal.Width - i.ri, internal.Y + i.ti, i.ri, internal.Height - i.ti - i.bi,
		    i.tex,
		    i.tex.w - i.re - i.ri, i.te + i.ti,
		    i.ri, i.tex.h - i.te - i.ti - i.bi - i.be,
		    0, 0,
		    color);
	}
	// center
	if(internal.Width - i.li - i.ri > 0 && internal.Height - i.ti - i.bi > 0)
	{
		DrawTextureTiled(internal.X + i.li, internal.Y + i.ti, internal.Width - i.li - i.ri, internal.Height - i.ti - i.bi,
		    i.tex,
		    i.le + i.li, i.te + i.ti,
		    i.tex.w - i.le - i.li - i.ri - i.re, i.tex.h - i.te - i.ti - i.bi - i.be,
		    0, 0,
		    color);
	}
}

// renders a simple colored frame, optimized for an opaque frame
void DrawSimpleFrameOpaque(int x, int y, int w, int h, XTColor border_1, XTColor border_2, XTColor fill)
{
	int bg_l = x;
	int bg_r = x + w;
	int bg_t = y;
	int bg_b = y + h;

	// border 1
	XRender::renderRect(bg_l, bg_t, bg_r - bg_l, bg_b - bg_t, border_1);

	// border 2
	XRender::renderRect(bg_l + 2, bg_t + 2, bg_r - bg_l - 4, bg_b - bg_t - 4, border_2);

	// fill
	XRender::renderRect(bg_l + 4, bg_t + 4, bg_r - bg_l - 8, bg_b - bg_t - 8, fill);
}

// renders a simple colored frame
void DrawSimpleFrame(int x, int y, int w, int h, XTColor border_1, XTColor border_2, XTColor fill)
{
	int bg_l = x;
	int bg_r = x + w;
	int bg_t = y;
	int bg_b = y + h;

	// border 1
	XRender::renderRect(bg_l, bg_t, 2, bg_b - bg_t, border_1);
	XRender::renderRect(bg_r - 2, bg_t, 2, bg_b - bg_t, border_1);
	XRender::renderRect(bg_l + 2, bg_t, bg_r - bg_l - 4, 2, border_1);
	XRender::renderRect(bg_l + 2, bg_b - 2, bg_r - bg_l - 4, 2, border_1);

	// border 2
	XRender::renderRect(bg_l + 2, bg_t + 2, 2, bg_b - bg_t - 4, border_2);
	XRender::renderRect(bg_r - 4, bg_t + 2, 2, bg_b - bg_t - 4, border_2);
	XRender::renderRect(bg_l + 4, bg_t + 2, bg_r - bg_l - 8, 2, border_2);
	XRender::renderRect(bg_l + 4, bg_b - 4, bg_r - bg_l - 8, 2, border_2);

	// fill
	XRender::renderRect(bg_l + 4, bg_t + 4, bg_r - bg_l - 8, bg_b - bg_t - 8, fill);
}
