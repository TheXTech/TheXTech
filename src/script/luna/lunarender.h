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

#pragma once
#ifndef LUNARENDER_H
#define LUNARENDER_H

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <list>

#include "lunaimgbox.h"

class RenderOp;
class LunaImage;

struct Renderer
{
    static Renderer &Get();
    static void SetAltThread();
    static void UnsetAltThread();
    static bool IsAltThreadActive();

    Renderer() noexcept;
    ~Renderer() = default;

    bool LoadBitmapResource(const std::string &filename, int resource_code, int transparency_color); // don't give full path
    bool LoadBitmapResource(const std::string &filename, int resource_code);
    void StoreImage(const LunaImage &bmp, int resource_code);
    bool DeleteImage(int resource_code);
    LunaImage *GetImageForResourceCode(int resource_code);

    void AddOp(RenderOp *op);                           // Add a drawing operation to the list
    // void GLCmd(const std::shared_ptr<GLEngineCmd>& cmd, double renderPriority = 1.0);

    void DebugPrint(const std::string &message);                // Print a debug message on the screen
    void DebugPrint(const std::string &message, double val);    // Print a debug message on the screen and display a related value

    void RenderBelowPriority(double maxPriority);

    void ClearAllDebugMessages();

    void ClearAllLoadedImages();

    // Calls from hooks
    void StartCameraRender(int idx);
    void StoreCameraPosition(int idx);
    void StartFrameRender();
    void EndFrameRender();

    void ClearQueue();
private:
    void DrawOp(RenderOp &render_operation);


    // Members //
public:
    class QueueState
    {
    public:
        bool m_InFrameRender;
        int m_curCamIdx; // Camera state

        std::size_t m_renderOpsSortedCount;
        std::size_t m_renderOpsProcessedCount;
        std::vector<RenderOp *> m_currentRenderOps; // render operations to be performed

        std::list<std::string> m_debugMessages;    // Debug message to be printed

    public:
        QueueState() :
            m_InFrameRender(false),
            m_curCamIdx(1),
            m_renderOpsSortedCount(0),
            m_renderOpsProcessedCount(0),
            m_currentRenderOps(),
            m_debugMessages()
        {}
    };

private:
    QueueState m_queueState;
    std::unordered_map<int, LunaImage> m_legacyResourceCodeImages;  // loaded image resources

    // Simple getters //
public:
    int GetCameraIdx() const
    {
        return m_queueState.m_curCamIdx;
    }
    // HDC GetScreenDC() { return (HDC)GM_SCRN_HDC; }

    class QueueStateStacker
    {
    private:
        Renderer &m_renderer;
        QueueState m_savedState;

    public:
        QueueStateStacker() :
            m_renderer(Renderer::Get())
        {
            m_savedState = m_renderer.m_queueState;
            // Don't use m_renderer.ClearQueue() for this because we're effectively moving things and ClearQueue frees some pointers
            m_renderer.m_queueState.m_InFrameRender = false;
            m_renderer.m_queueState.m_curCamIdx = 1;
            m_renderer.m_queueState.m_renderOpsSortedCount = 0;
            m_renderer.m_queueState.m_renderOpsProcessedCount = 0;
            m_renderer.m_queueState.m_currentRenderOps.clear();
            m_renderer.m_queueState.m_debugMessages.clear();
        }

        ~QueueStateStacker()
        {
            m_renderer.ClearQueue();
            m_renderer.m_queueState = m_savedState;
        }
    };
};

namespace Render
{

bool IsOnScreen(double x, double y, double w, double h); // Returns whether or not the digven rectangle is on screen this frame
void CalcCameraPos(double *p_X, double *p_Y);            // Tries to read smbx memory to return the camera coords in the 2 passed args

}


#endif // LUNARENDER_H
