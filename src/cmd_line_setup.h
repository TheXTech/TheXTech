#ifndef CMD_LINE_SETUP_H
#define CMD_LINE_SETUP_H

#include <string>

struct CmdLineSetup_t
{
    //! Disable game video
    bool noVideo = false;
    //! Disable game sound
    bool noSound = false;
    //! Skip frames when laggy rendering is
    bool frameSkip = false;
    //! Don't pause game while window is not active
    bool neverPause = false;
    //! Allow background input for joysticks
    bool allowBgInput = false;
    //! TYpe of a render
    int renderType = -1;
    //! Is a level testing mode
    bool testLevelMode = false;
    //! Level file to test
    std::string testLevel;
    //! Number of players for level test
    int testNumPlayers = 1;
    //! Run a test in battle mode
    bool testBattleMode = false;

    //! Enable interprocessing communication with the Moondust Editor
    bool interprocess = false;

    //! Allow playable character grab everything
    bool testGrabAll = false;
    //! Make playable character be inmortal
    bool testGodMode = false;
    //! Make playable character be inmortal
    bool testShowFPS = false;
    //! Make playable character be inmortal
    bool testMaxFPS = false;
    //! Enable magic-hand functionality
    bool testMagicHand = false;

    //! Force log output into console
    bool verboseLogging = false;

    //! Enforce spefic compatibiltiy level (If Speed-Run mode is enabled, this will be overwritten)
    int  compatibilityLevel = 0;

    //! Enable the speed-run mode
    int  speedRunnerMode = 0;
    //! Draw the speed-run timer semi-transparently
    bool speedRunnerSemiTransparent = false;

    //! Index to replay
    int  recordReplayId = -1;
    //! Record gameplay data
    bool recordRecord = false;
    //! Replay previous gameplay
    bool recordReplay = false;
};

#endif // CMD_LINE_SETUP_H
