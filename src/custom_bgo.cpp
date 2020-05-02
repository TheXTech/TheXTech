#include "globals.h"
#include "custom_bgo.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <PGE_File_Formats/smbx64.h>
#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

void LoadCustomBGO(int A, std::string cFileName);

void SaveBackgroundDefaults()
{
    for(int A = 1; A <= maxBackgroundType; A++)
    {
        BackgroundDefaults.BackgroundWidth[A] = BackgroundWidth[A];
        BackgroundDefaults.BackgroundHeight[A] = BackgroundHeight[A];
        BackgroundDefaults.BackgroundFrameAmount[A] = BackgroundFrameAmount[A];
        BackgroundDefaults.BackgroundFrameSpeed[A] = BackgroundFrameSpeed[A];
		BackgroundDefaults.BackgroundFence[A] = BackgroundFence[A];
        BackgroundDefaults.BackgroundNPCFence[A] = BackgroundNPCFence[A];
        BackgroundDefaults.BackgroundFreeze[A] = BackgroundFreeze[A];
		BackgroundDefaults.Foreground[A] = Foreground[A];
    }
}

void LoadBackgroundDefaults()
{
    for(int A = 1; A <= maxBackgroundType; A++)
    {
        BackgroundWidth[A] = BackgroundDefaults.BackgroundWidth[A];
        BackgroundHeight[A] = BackgroundDefaults.BackgroundHeight[A];
        BackgroundFrameAmount[A] = BackgroundDefaults.BackgroundFrameAmount[A];
        BackgroundFrameSpeed[A] = BackgroundDefaults.BackgroundFrameSpeed[A];
		BackgroundFence[A] = BackgroundDefaults.BackgroundFence[A];
        BackgroundNPCFence[A] = BackgroundDefaults.BackgroundNPCFence[A];
        BackgroundFreeze[A] = BackgroundDefaults.BackgroundFreeze[A];
		Foreground[A] = BackgroundDefaults.Foreground[A];
    }
}

void FindCustomBGOs(/*std::string cFilePath*/)
{
    DirMan searchDir(FileNamePath);
    std::set<std::string> existingFiles;
    std::vector<std::string> files;
    searchDir.getListOfFiles(files, {".txt"});
    for(auto &p : files)
        existingFiles.insert(FileNamePath + p);

    if(DirMan::exists(FileNamePath + FileName))
    {
        DirMan searchDataDir(FileNamePath + FileName);
        searchDataDir.getListOfFiles(files, {".png", ".gif"});
        for(auto &p : files)
            existingFiles.insert(FileNamePath + FileName  + "/"+ p);
    }

    for(int A = 1; A < maxBackgroundType; ++A)
    {

        std::string BGOPathBasegame = AppPath + fmt::format_ne("config/background/background-{0}.txt", A);

        std::string BGOIniPath = FileNamePath + fmt::format_ne("background-{0}.ini", A);
        std::string BGOIniPathC = FileNamePath + FileName + fmt::format_ne("/background-{0}.ini", A);
        std::string BGOPath = FileNamePath + fmt::format_ne("background-{0}.txt", A);
        std::string BGOPathC = FileNamePath + FileName + fmt::format_ne("/background-{0}.txt", A);

        if(Files::fileExists(BGOPathBasegame))
            LoadCustomBGO(A, BGOPathBasegame);

        if(Files::fileExists(BGOIniPath))
            LoadCustomBGO(A, BGOIniPath);
        if(Files::fileExists(BGOPath))
            LoadCustomBGO(A, BGOPath);

        if(Files::fileExists(BGOIniPathC))
            LoadCustomBGO(A, BGOIniPathC);
        if(Files::fileExists(BGOPathC))
            LoadCustomBGO(A, BGOPathC);
    }
}

void LoadCustomBGO(int A, std::string cFileName)
{
    IniProcessing config(cFileName);
    if(!config.beginGroup("BGO"))
           config.beginGroup("General");


    config.read("width", BackgroundWidth[A], BackgroundWidth[A]);
    config.read("height", BackgroundHeight[A], BackgroundHeight[A]);

    if(BackgroundWidth[A] <= 0) // Validate
        BackgroundWidth[A] = 32;
    if(BackgroundHeight[A] <= 0)
        BackgroundHeight[A] = 32;

    config.read("frames", BackgroundFrameAmount[A], BackgroundFrameAmount[A]);
    if(BackgroundFrameAmount[A] <= 0) // Validate
        BackgroundFrameAmount[A] = 1;

    int frameDelay = 62;
    config.read("frame-delay", frameDelay, frameDelay);
    config.read("frame-speed", frameDelay, frameDelay);
    if(frameDelay <= 0) // validate
        frameDelay = 1;

    BackgroundFrameSpeed[A] = int((double(frameDelay) / 1000.0) * 65.0);
    config.read("framespeed", BackgroundFrameSpeed[A], BackgroundFrameSpeed[A]);

    if(BackgroundFrameSpeed[A] <= 0) // validate
        BackgroundFrameSpeed[A] = 1;

    config.read("climbable", BackgroundFence[A], BackgroundFence[A]);
    config.read("npcclimbable", BackgroundNPCFence[A], BackgroundNPCFence[A]);
    config.read("foreground", Foreground[A], Foreground[A]);
    config.read("stoponfreeze", BackgroundFreeze[A], BackgroundFreeze[A]);
    config.read("stop-on-freeze", BackgroundFreeze[A], BackgroundFreeze[A]);
    config.endGroup();
}
