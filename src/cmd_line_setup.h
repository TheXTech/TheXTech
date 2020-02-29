#ifndef CMD_LINE_SETUP_H
#define CMD_LINE_SETUP_H

#include <string>

struct CmdLineSetup_t
{
    //! Disable game sound
    bool noSound = false;
    //! Skip frames when laggy rendering is
    bool frameSkip = false;
    //! Don't pause game while window is not active
    bool neverPause = false;
    //! TYpe of a render
    enum RenderType
    {
        RENDER_SW = 0,
        RENDER_HW = 1,
        RENDER_VSYNC = 2
    } renderType;
    //! Is a level testing mode
    bool testLevelMode = false;
    //! Level file to test
    std::string testLevel;
    //! Number of players for level test
    int testNumPlayers = 1;
    //! Run a test in battle mode
    bool testBattleMode = false;
};

#endif // CMD_LINE_SETUP_H
