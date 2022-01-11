#pragma once
#ifndef LUNARENDER_H
#define LUNARENDER_H

#include <map>
#include <memory>
#include <vector>
#include <list>

class RenderOp;
class LunaImage;

struct Renderer
{
    static Renderer& Get();
    static void SetAltThread();
    static void UnsetAltThread();
    static bool IsAltThreadActive();

    Renderer();
    ~Renderer();

    bool LoadBitmapResource(std::string filename, int resource_code, int transparency_color); // don't give full path
    bool LoadBitmapResource(std::string filename, int resource_code);
    void StoreImage(const std::shared_ptr<LunaImage>& bmp, int resource_code);
    bool DeleteImage(int resource_code);
    std::shared_ptr<LunaImage> GetImageForResourceCode(int resource_code);

    std::vector<std::shared_ptr<LunaImage>> LoadAnimatedBitmapResource(std::string filename, int* frameTime = 0);

    void AddOp(RenderOp* op);							// Add a drawing operation to the list
    // void GLCmd(const std::shared_ptr<GLEngineCmd>& cmd, double renderPriority = 1.0);

    void DebugPrint(const std::string &message);				// Print a debug message on the screen
    void DebugPrint(const std::string &message, double val);	// Print a debug message on the screen and display a related value

    void RenderBelowPriority(double maxPriority);

    void ClearAllDebugMessages();

    // Calls from hooks
    void StartCameraRender(int idx);
    void StoreCameraPosition(int idx);
    void StartFrameRender();
    void EndFrameRender();

    void ClearQueue();
private:
    void DrawOp(RenderOp& render_operation);


    // Members //
public:
    class QueueState
    {
    public:
        bool m_InFrameRender;
        int m_curCamIdx; // Camera state

        std::size_t m_renderOpsSortedCount;
        std::size_t m_renderOpsProcessedCount;
        std::vector<RenderOp*> m_currentRenderOps;  // render operations to be performed

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

    std::map<int, std::shared_ptr<LunaImage>> m_legacyResourceCodeImages;  // loaded image resources

    // Simple getters //
public:
    int GetCameraIdx() { return m_queueState.m_curCamIdx; }
    // HDC GetScreenDC() { return (HDC)GM_SCRN_HDC; }

    class QueueStateStacker {
    private:
        Renderer& m_renderer;
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
void CalcCameraPos(double* p_X, double* p_Y);            // Tries to read smbx memory to return the camera coords in the 2 passed args

}


#endif // LUNARENDER_H
