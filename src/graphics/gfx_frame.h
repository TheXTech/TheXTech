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

#pragma once
#ifndef GFX_FRAME_H
#define GFX_FRAME_H

#include "location.h"
#include "std_picture.h"

struct FrameBorderInfo
{
	// top, left, bottom, and right, external and internal
	int te = 0;
	int ti = 0;
	int le = 0;
	int li = 0;
	int be = 0;
	int bi = 0;
	int re = 0;
	int ri = 0;
};

struct FrameBorder : public FrameBorderInfo
{
	StdPicture tex;
};

class IniProcessing;

void loadFrameInfo(IniProcessing& ini, FrameBorderInfo& borderinfo);

void DrawTextureTiled(int dst_x, int dst_y, int dst_w, int dst_h, StdPicture& tx, int src_x = 0, int src_y = 0, int src_w = -1, int src_h = -1, int off_x = -1, int off_y = -1, XTColor color = XTColor());

// renders a new-style frame as a border with the included external and internal locations
// the frame fills the space between external and internal
// border is nullable
void RenderFrameBorder(const Location_t& external, const Location_t& internal,
	StdPicture& tile, FrameBorder* border);

// renders a new-style frame as a fill at the included internal location
// the frame fills the internal rect
void RenderFrameFill(const Location_t& internal, FrameBorder& frame, XTColor color = XTColor());

// renders a simple colored frame, optimized for an opaque frame
void DrawSimpleFrameOpaque(int x, int y, int w, int h, XTColor border_1, XTColor border_2, XTColor fill);

// renders a simple colored frame
void DrawSimpleFrame(int x, int y, int w, int h, XTColor border_1, XTColor border_2, XTColor fill);

#endif // GFX_FRAME_H
