/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "core/render.h"

#include "graphics/gfx_frame.h"

void DrawTextureTiled(int dst_x, int dst_y, int dst_w, int dst_h, StdPicture& tx, int src_x, int src_y, int src_w, int src_h, int off_x, int off_y, float alpha)
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

            XRender::renderTexture(x, y, render_w, render_h, tx, src_x + c_off_x, src_y + c_off_y, 1.f, 1.f, 1.f, alpha);

            y += src_h - c_off_y;
            c_off_y = 0;
        }

        x += src_w - c_off_x;
        c_off_x = 0;
    }
}

// renders a new-style frame with the included external and internal locations
void RenderFrame(const Location_t& external, const Location_t& internal,
	StdPicture& tile, StdPicture* border, const FrameBorderInfo* borderinfo)
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

	if(!border || !borderinfo)
		return;

	const FrameBorderInfo& i = *borderinfo;

	int li = have_l ? i.li : 0;
	int ti = have_t ? i.ti : 0;
	int ri = have_r ? i.ri : 0;
	int bi = have_b ? i.bi : 0;

	// top external-left external
	if(have_l && have_t)
	{
		XRender::renderTexture(internal.X - i.le, internal.Y - i.te, i.le, i.te,
		    *border,
		    0, 0);
	}
	// top external-left internal
	if(have_l && have_t)
	{
		XRender::renderTexture(internal.X, internal.Y - i.te, i.li, i.te,
		    *border,
		    i.le, 0);
	}
	// top external-center
	if(have_t)
	{
		DrawTextureTiled(internal.X + li, internal.Y - i.te, internal.Width - li - ri, i.te,
		    *border,
		    i.le + i.li, 0,
		    border->w - i.le - i.li - i.ri - i.re, i.te,
		    0, 0);
	}
	// top external-right internal
	if(have_t && have_r)
	{
		XRender::renderTexture(internal.X + internal.Width - i.ri, internal.Y - i.te, i.ri, i.te,
		    *border,
		    border->w - i.ri - i.re, 0);
	}
	// top external-right external
	if(have_t && have_r)
	{
		XRender::renderTexture(internal.X + internal.Width, internal.Y - i.te, i.re, i.te,
		    *border,
		    border->w - i.re, 0);
	}
	// top internal-left external
	if(have_t && have_l)
	{
		XRender::renderTexture(internal.X - i.le, internal.Y, i.le, i.ti,
		    *border,
		    0, i.te);
	}
	// center-left external
	if(have_l)
	{
		DrawTextureTiled(internal.X - i.le, internal.Y + ti, i.le, internal.Height - ti - bi,
		    *border,
		    0, i.te + i.ti,
		    i.le, border->h - i.te - i.ti - i.bi - i.be,
		    0, 0);
	}
	// bottom internal-left external
	if(have_b && have_l)
	{
		XRender::renderTexture(internal.X - i.le, internal.Y + internal.Height - i.bi, i.le, i.bi,
		    *border,
		    0, border->h - i.bi - i.be);
	}
	// bottom external-left external
	if(have_b && have_l)
	{
		XRender::renderTexture(internal.X - i.le, internal.Y + internal.Height, i.le, i.be,
		    *border,
		    0, border->h - i.be);
	}
	// bottom external-left internal
	if(have_b && have_l)
	{
		XRender::renderTexture(internal.X, internal.Y + internal.Height, i.li, i.be,
		    *border,
		    i.le, border->h - i.be);
	}
	// bottom external-center
	if(have_b)
	{
		DrawTextureTiled(internal.X + li, internal.Y + internal.Height, internal.Width - li - ri, i.be,
		    *border,
		    i.le + i.li, border->h - i.be, border->w - i.le - i.li - i.ri - i.re, i.be,
		    0, 0);
	}
	// bottom external-right internal
	if(have_b && have_r)
	{
		XRender::renderTexture(internal.X + internal.Width - i.ri, internal.Y + internal.Height, i.ri, i.be,
		    *border,
		    border->w - i.ri - i.re, border->h - i.be);
	}
	// bottom external-right external
	if(have_b && have_r)
	{
		XRender::renderTexture(internal.X + internal.Width, internal.Y + internal.Height, i.re, i.be,
		    *border,
		    border->w - i.re, border->h - i.be);
	}
	// top internal-right external
	if(have_t && have_r)
	{
		XRender::renderTexture(internal.X + internal.Width, internal.Y, i.re, i.ti,
		    *border,
		    border->w - i.re, i.te);
	}
	// center-right external
	if(have_r)
	{
		DrawTextureTiled(internal.X + internal.Width, internal.Y + ti, i.re, internal.Height - ti - bi,
		    *border,
		    border->w - i.re, i.te + i.ti,
		    i.re, border->h - i.te - i.ti - i.bi - i.be,
		    0, 0);
	}
	// bottom internal-right external
	if(have_b && have_r)
	{
		XRender::renderTexture(internal.X + internal.Width, internal.Y + internal.Height - i.ti, i.re, i.bi,
		    *border,
		    border->w - i.re, border->h - i.bi - i.be);
	}
}
