#include "luna.h"
#include "autocode.h"
#include "autocode_manager.h"
#include "lunainput.h"
#include "lunarender.h"
#include "lunaspriteman.h"
#include "lunacell.h"

#include "globals.h"

bool gLunaEnabled = true;

static void (*levelCodeRun)() = nullptr;


#include "levels/Docopoper-Calleoca.h"

static void InitLevel()
{
    levelCodeRun = nullptr;

    if(FileNameFull == "Docopoper-Calleoca.lvl")
    {
        CalleocaInitCode();
        levelCodeRun = CalleocaCode;
    }
}

void lunaReset()
{
    levelCodeRun = nullptr;

    gFrames = 0;

    gLastDownPress = 0;
    gDownTapped = 0;
    gLastUpPress = 0;
    gUpTapped = 0;
    gLastLeftPress = 0;
    gLeftTapped = 0;
    gLastRightPress = 0;
    gRightTapped = 0;

    gLastJumpPress = 0;
    gJumpTapped = 0;
    gLastRunPress = 0;
    gRunTapped = 0;

    Renderer::Get().ClearAllDebugMessages();
    gSpriteMan.ResetSpriteManager();
    gCellMan.Reset();
//    gSavedVarBank.ClearBank();
    Input::ResetAll();

}

void lunaLoad()
{
    lunaReset();

    if(gLunaEnabled)
    {
        // Load autocode
        gAutoMan.Clear(false);
        gAutoMan.ReadFile(FileNamePath);

        // Try to load world codes
        gAutoMan.ReadWorld(FileNamePath);

        // Do some stuff
        gAutoMan.DoEvents(true); // do with init

        // Init some stuff
        InitLevel();
        gAutoMan.m_Hearts = 2;
    }
}

void lunaLoop()
{
    // Clean up
    gAutoMan.ClearExpired();

    // Update inputs
    Input::CheckSpecialCheats();
    Input::UpdateInputTasks();

    if(gLunaEnabled)
    {
#if COMPILE_PLAYGROUND
        Playground::doPlaygroundStuff();
#endif
//        g_EventHandler.hookLevelLoop();

        // Run autocode
        gAutoMan.DoEvents(false);

        // Update some stuff
        gFrames++;
//        gSavedVarBank.SaveIfNeeded();

        // Run any framecode
//        TestFrameCode();
        if(levelCodeRun)
            levelCodeRun();
    }
}

void lunaRender()
{
    if(!gLunaEnabled)
        return;
    Renderer::Get().StartFrameRender();
    Renderer::Get().RenderBelowPriority(5);
    Renderer::Get().EndFrameRender();
}
