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

#include "lunarender.h"
#include "autocode_manager.h"
#include "renderop.h"
#include "globals.h"
#include "lunamisc.h"
#include "renderop_string.h"
#include <fmt_format_ne.h>

#include <algorithm>


static Renderer sLunaRender;

Renderer &Renderer::Get()
{
    return sLunaRender;
}

void Renderer::SetAltThread()
{}

void Renderer::UnsetAltThread()
{}

bool Renderer::IsAltThreadActive()
{
    return false;
}

Renderer::Renderer() noexcept :
    m_queueState(),
    m_legacyResourceCodeImages()
{}

bool Renderer::LoadBitmapResource(const std::string& filename, int resource_code, int transparency_color)
{
    DeleteImage(resource_code);

    auto absPath = AutocodeManager::resolveCustomFileCase(filename);

    if(absPath.empty())
        absPath = AutocodeManager::resolveWorldFileCase(filename);

    if(absPath.empty())
    {
        pLogWarning("LunaRender: image file %s is not found", filename.c_str());
        return false;
    }

    LunaImage img(absPath);
    if(!img.ImageLoaded())
    {
        pLogWarning("LunaRender: Failed to load image: %s", absPath.c_str());
        return false;
    }

    img.m_TransColor = transparency_color;
    StoreImage(img, resource_code);

    return true;
}

bool Renderer::LoadBitmapResource(const std::string& filename, int resource_code)
{
    return LoadBitmapResource(filename, resource_code, DEFAULT_TRANS_COLOR);
}

void Renderer::StoreImage(const LunaImage &bmp, int resource_code)
{
    m_legacyResourceCodeImages[resource_code] = bmp;
}

bool Renderer::DeleteImage(int resource_code)
{
    auto it = m_legacyResourceCodeImages.find(resource_code);
    if(it != m_legacyResourceCodeImages.end())
    {
        it->second.Unload();
        m_legacyResourceCodeImages.erase(it);
        return true;
    }

    return false;
}

LunaImage *Renderer::GetImageForResourceCode(int resource_code)
{
    auto it = m_legacyResourceCodeImages.find(resource_code);
    if(it != m_legacyResourceCodeImages.end())
        return &it->second;

    return nullptr;
}

void Renderer::AddOp(RenderOp *op)
{
    if(op->m_selectedCamera == 0)
    {
        // If the rendering operation was created in the middle of handling a
        // camera's rendering, lock the rendering operation to that camera.
        op->m_selectedCamera = m_queueState.m_curCamIdx;
    }

    this->m_queueState.m_currentRenderOps.push_back(op);
}

void Renderer::DebugPrint(const std::string &message)
{
    this->m_queueState.m_debugMessages.push_back(message);
}

void Renderer::DebugPrint(const std::string &message, double val)
{
    this->m_queueState.m_debugMessages.push_back(fmt::format_ne("{0} {1}", message, val));
}

static bool CompareRenderPriority(const RenderOp *lhs, const RenderOp *rhs)
{
    return lhs->m_renderPriority < rhs->m_renderPriority;
}

void Renderer::RenderBelowPriority(double maxPriority)
{
    if(!m_queueState.m_InFrameRender) return;

    //    if (this == &sLunaRender)
    //    {
    //        // Make sure we kill the loadscreen before main thread rendering
    //        LunaLoadScreenKill();
    //    }

    auto &ops = m_queueState.m_currentRenderOps;
    if(ops.size() <= m_queueState.m_renderOpsProcessedCount) return;

    // Flush pending BltBlt
    //    g_BitBltEmulation.flushPendingBlt();

    // Assume operations already processed were already sorted
    if(m_queueState.m_renderOpsSortedCount == 0)
    {
        std::stable_sort(ops.begin(), ops.end(), CompareRenderPriority);
        m_queueState.m_renderOpsSortedCount = ops.size();
    }
    else if(m_queueState.m_renderOpsSortedCount < ops.size())
    {
        // Sort the new operations
        std::stable_sort(ops.begin() + m_queueState.m_renderOpsSortedCount, ops.end(), CompareRenderPriority);

        // Render things as many of the new items as we should before merging the sorted lists
        double maxPassPriority = maxPriority;
        if(m_queueState.m_renderOpsSortedCount > m_queueState.m_renderOpsProcessedCount)
        {
            double nextPriorityInOldList = ops[m_queueState.m_renderOpsProcessedCount]->m_renderPriority;
            if(nextPriorityInOldList < maxPassPriority)
                maxPassPriority = nextPriorityInOldList;
        }

        for(auto iter = ops.cbegin() + m_queueState.m_renderOpsSortedCount, end = ops.cend(); iter != end; ++iter)
        {
            RenderOp &op = **iter;
            if(op.m_renderPriority >= maxPassPriority)
                break;
            DrawOp(op);
            m_queueState.m_renderOpsProcessedCount++;
        }

        // Merge sorted list sections (note, std::inplace_merge is a stable sort)
        std::inplace_merge(ops.begin(), ops.begin() + m_queueState.m_renderOpsSortedCount, ops.end(), CompareRenderPriority);
        m_queueState.m_renderOpsSortedCount = ops.size();
    }

    // Render other operations
    for(auto iter = ops.cbegin() + m_queueState.m_renderOpsProcessedCount, end = ops.cend(); iter != end; ++iter)
    {
        RenderOp &op = **iter;
        if(op.m_renderPriority >= maxPriority)
            break;
        DrawOp(op);
        m_queueState.m_renderOpsProcessedCount++;
    }

    if(maxPriority >= DBL_MAX)
    {
        // Format debug messages and enter them into renderstring list
        int dbg_x = 325;
        int dbg_y = 160;

        for(auto &dbg : m_queueState.m_debugMessages)
        {
            RenderStringOp(dbg, 4, (float)dbg_x, (float)dbg_y).Draw(this);
            dbg_y += 20;
            if(dbg_y > 560)
            {
                dbg_y = 160;
                dbg_x += 190;
            }
        }

        this->m_queueState.m_debugMessages.clear();
    }
}

void Renderer::ClearAllDebugMessages()
{
    this->m_queueState.m_debugMessages.clear();
}

void Renderer::ClearAllLoadedImages()
{
    for(auto &i : m_legacyResourceCodeImages)
        i.second.Unload();
    m_legacyResourceCodeImages.clear();
}

void Renderer::StartCameraRender(int idx)
{
    m_queueState.m_curCamIdx = idx;
    m_queueState.m_renderOpsProcessedCount = 0;
}

void Renderer::StoreCameraPosition(int idx)
{
    UNUSED(idx);
    //    if (g_GLEngine.IsEnabled())
    //    {
    //        std::shared_ptr<GLEngineCmd_SetCamera> cmd = std::make_shared<GLEngineCmd_SetCamera>();
    //        cmd->mX = SMBX_CameraInfo::getCameraX(idx);
    //        cmd->mY = SMBX_CameraInfo::getCameraY(idx);
    //        g_GLEngine.QueueCmd(cmd);
    //    }
}

void Renderer::StartFrameRender()
{
    m_queueState.m_curCamIdx = 0;
    m_queueState.m_InFrameRender = true;
}

void Renderer::EndFrameRender()
{
    if(!m_queueState.m_InFrameRender) return;

    m_queueState.m_curCamIdx = 0;

    // Remove cleared operations
    std::vector<RenderOp *> nonExpiredOps;
    for(auto &m_currentRenderOp : m_queueState.m_currentRenderOps)
    {
        RenderOp *pOp = m_currentRenderOp;
        pOp->m_FramesLeft--;
        if(pOp->m_FramesLeft <= 0)
        {
            m_currentRenderOp = nullptr;
            delete pOp;
        }
        else
            nonExpiredOps.push_back(pOp);
    }

    m_queueState.m_currentRenderOps.swap(nonExpiredOps);
    m_queueState.m_renderOpsProcessedCount = 0;
    m_queueState.m_renderOpsSortedCount = m_queueState.m_currentRenderOps.size();
    m_queueState.m_InFrameRender = false;
}

void Renderer::ClearQueue()
{
    m_queueState.m_curCamIdx = 0;
    for(auto &m_currentRenderOp : m_queueState.m_currentRenderOps)
        delete m_currentRenderOp;
    m_queueState.m_currentRenderOps.clear();
    m_queueState.m_renderOpsProcessedCount = 0;
    m_queueState.m_renderOpsSortedCount = 0;
    m_queueState.m_InFrameRender = false;
}

void Renderer::DrawOp(RenderOp &op)
{
    if((op.m_selectedCamera == 0 || op.m_selectedCamera == m_queueState.m_curCamIdx) && (op.m_FramesLeft >= 1))
        op.Draw(this);
}


bool Render::IsOnScreen(double x, double y, double w, double h)
{
    double cam_x;
    double cam_y;
    CalcCameraPos(&cam_x, &cam_y);

    return FastTestCollision((int)cam_x, (int)cam_y, (int)cam_x + ScreenW, (int)cam_y + ScreenH,
                             (int)x, (int)y, (int)x + (int)w, (int)y + (int)h);
}

void Render::CalcCameraPos(double *ret_x, double *ret_y)
{
    // Old camera func, using "camera" memory
    double val;

    if(ret_x != nullptr)
    {
        val = vScreenX[1];
        *ret_x = val - val - val; // Fix backwards smbx camera
    }

    if(ret_y != nullptr)
    {
        val = vScreenY[1];
        *ret_y = val - val - val; // Fix backwards smbx camera
    }
}
