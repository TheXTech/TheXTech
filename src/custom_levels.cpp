#include "globals.h"
#include "custom_levels.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <PGE_File_Formats/smbx64.h>
#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

void LoadCustomLevel(int A, std::string cFileName);

void SaveLevelDefaults()
{
    for(int A = 1; A <= maxLevelType; A++)
    {
        LevelDefaults.LevelWidth[A] = LevelWidth[A];
        LevelDefaults.LevelHeight[A] = LevelHeight[A];
        LevelDefaults.LevelFrameCount[A] = LevelFrameCount[A];
        LevelDefaults.LevelFrameSpeed[A] = LevelFrameSpeed[A];
    }
}

void LoadLevelDefaults()
{
    for(int A = 1; A <= maxLevelType; A++)
    {
        LevelWidth[A] = LevelDefaults.LevelWidth[A];
        LevelHeight[A] = LevelDefaults.LevelHeight[A];
        LevelFrameCount[A] = LevelDefaults.LevelFrameCount[A];
        LevelFrameSpeed[A] = LevelDefaults.LevelFrameSpeed[A];
    }
}

void FindCustomLevels(/*std::string cFilePath*/)
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

    for(int A = 1; A < maxLevelType; ++A)
    {

        std::string LevelPathBasegame = AppPath + fmt::format_ne("config/level/level-{0}.txt", A);

        std::string LevelIniPath = FileNamePath + fmt::format_ne("level-{0}.ini", A);
        std::string LevelIniPathC = FileNamePath + FileName + fmt::format_ne("/level-{0}.ini", A);
        std::string LevelPath = FileNamePath + fmt::format_ne("level-{0}.txt", A);
        std::string LevelPathC = FileNamePath + FileName + fmt::format_ne("/level-{0}.txt", A);

        if(Files::fileExists(LevelPathBasegame))
            LoadCustomLevel(A, LevelPathBasegame);

        if(Files::fileExists(LevelIniPath))
            LoadCustomLevel(A, LevelIniPath);
        if(Files::fileExists(LevelPath))
            LoadCustomLevel(A, LevelPath);

        if(Files::fileExists(LevelIniPathC))
            LoadCustomLevel(A, LevelIniPathC);
        if(Files::fileExists(LevelPathC))
            LoadCustomLevel(A, LevelPathC);
    }
}
void LoadCustomLevel(int A, std::string cFileName)

{
    IniProcessing config(cFileName);
    if(!config.beginGroup("level"))
           config.beginGroup("General");


    config.read("width", LevelWidth[A], LevelWidth[A]);
    config.read("height", LevelHeight[A], LevelHeight[A]);

    if(LevelWidth[A] <= 0) // Validate
        LevelWidth[A] = 32;
    if(LevelHeight[A] <= 0)
        LevelHeight[A] = 32;

    config.read("frames", LevelFrameCount[A], LevelFrameCount[A]);
    if(LevelFrameCount[A] <= 0) // Validate
        LevelFrameCount[A] = 1;

    int frameDelay = 62;
    config.read("frame-delay", frameDelay, frameDelay);
    config.read("frame-speed", frameDelay, frameDelay);
    if(frameDelay <= 0) // validate
        frameDelay = 1;

    LevelFrameSpeed[A] = int((double(frameDelay) / 1000.0) * 65.0);
    config.read("framespeed", LevelFrameSpeed[A], LevelFrameSpeed[A]);

    if(LevelFrameSpeed[A] <= 0) // validate
        LevelFrameSpeed[A] = 1;

    config.endGroup();
}
