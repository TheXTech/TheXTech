/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "renderop_string.h"

#include "sdl_proxy/sdl_stdinc.h"
#include "core/render.h"

RenderStringOp::RenderStringOp() :
    RenderStringOp(std::string(), 1, 400.f, 400.f)
{
    static_assert(sizeof(RenderStringOp) <= c_rAllocChunkSize,
                  "Size of RenderStringOp class must be smaller than c_rAllocChunkSize");
}

RenderStringOp::RenderStringOp(const std::string &str, int font_type, float X, float Y) :
    RenderOp(RENDEROP_DEFAULT_PRIORITY_TEXT),
    m_FontType(font_type),
    m_X(X),
    m_Y(Y),
    sceneCoords(false)
{
    m_StringSize = str.size();
    m_StringDup = (m_StringSize >= c_rAllocChunkSize);

    if(m_StringDup) // fallback if string is longer than chunk size, that shouldn't happen usually
        m_String = SDL_strdup(str.c_str());
    else
    {
        m_String = (char*)g_rAlloc.Allocate(c_rAllocChunkSize);
        m_StringSize = SDL_strlcpy(m_String, str.c_str(), c_rAllocChunkSize);
    }
}

RenderStringOp::~RenderStringOp()
{
    if(m_StringDup)
        SDL_free(m_String);
    else
        g_rAlloc.Free(m_String);
    m_String = nullptr;
}

void RenderStringOp::Draw(Renderer *renderer)
{
    //        VB6StrPtr text(m_String);
    float x = m_X, y = m_Y;

    if(sceneCoords)
    {
        x -= static_cast<float>(vScreen[renderer->GetCameraIdx()].X);
        y -= static_cast<float>(vScreen[renderer->GetCameraIdx()].Y);
    }

    if(!sceneCoords)
        XRender::offsetViewportIgnore(true);

    SuperPrint(m_StringSize, m_String, m_FontType, x, y);

    if(!sceneCoords)
        XRender::offsetViewportIgnore(false);
}
