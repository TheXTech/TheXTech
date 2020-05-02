#include "globals.h"
#include "custom_paths.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <PGE_File_Formats/smbx64.h>
#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

void LoadCustomPath(int A, std::string cFileName);

void SavePathDefaults()
{
    for(int A = 1; A <= maxPathType; A++)
    {
        PathDefaults.PathWidth[A] = PathWidth[A];
        PathDefaults.PathHeight[A] = PathHeight[A];
        PathDefaults.PathFrameCount[A] = PathFrameCount[A];
        PathDefaults.PathFrameSpeed[A] = PathFrameSpeed[A];
        PathDefaults.PathPlayerSpeed[A] = PathPlayerSpeed[A];
    }
}

void LoadPathDefaults()
{
    for(int A = 1; A <= maxPathType; A++)
    {
        PathWidth[A] = PathDefaults.PathWidth[A];
        PathHeight[A] = PathDefaults.PathHeight[A];
        PathFrameCount[A] = PathDefaults.PathFrameCount[A];
        PathFrameSpeed[A] = PathDefaults.PathFrameSpeed[A];
        PathPlayerSpeed[A] = PathDefaults.PathPlayerSpeed[A];
    }
}

void FindCustomPaths(/*std::string cFilePath*/)
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

    for(int A = 1; A < maxPathType; ++A)
    {
        std::string PathPathBasegame = AppPath + fmt::format_ne("config/path/path-{0}.txt", A);
        std::string PathIniPath = FileNamePath + fmt::format_ne("path-{0}.ini", A);
        std::string PathIniPathC = FileNamePath + FileName + fmt::format_ne("/path-{0}.ini", A);
        std::string PathPath = FileNamePath + fmt::format_ne("path-{0}.txt", A);
        std::string PathPathC = FileNamePath + FileName + fmt::format_ne("/path-{0}.txt", A);

        if(Files::fileExists(PathPathBasegame))
            LoadCustomPath(A, PathPathBasegame);

        if(Files::fileExists(PathIniPath))
            LoadCustomPath(A, PathIniPath);
        if(Files::fileExists(PathPath))
            LoadCustomPath(A, PathPath);

        if(Files::fileExists(PathIniPathC))
            LoadCustomPath(A, PathIniPathC);
        if(Files::fileExists(PathPathC))
            LoadCustomPath(A, PathPathC);
    }
}

void LoadCustomPath(int A, std::string cFileName)
{
    IniProcessing config(cFileName);
    if(!config.beginGroup("path"))
           config.beginGroup("General");


    config.read("width", PathWidth[A], PathWidth[A]);
    config.read("height", PathHeight[A], PathHeight[A]);
    config.read("playerspeed", PathPlayerSpeed[A], PathPlayerSpeed[A]);
    config.read("player-speed", PathPlayerSpeed[A], PathPlayerSpeed[A]); // alias

    if(PathWidth[A] <= 0) // Validate
        PathWidth[A] = 32;
    if(PathHeight[A] <= 0)
        PathHeight[A] = 32;

    config.read("frames", PathFrameCount[A], PathFrameCount[A]);
    if(PathFrameCount[A] <= 0) // Validate
        PathFrameCount[A] = 1;

    int frameDelay = 62;
    config.read("frame-delay", frameDelay, frameDelay);
    config.read("frame-speed", frameDelay, frameDelay);
    if(frameDelay <= 0) // validate
        frameDelay = 1;

   PathFrameSpeed[A] = int((double(frameDelay) / 1000.0) * 65.0);
    config.read("framespeed", PathFrameSpeed[A], PathFrameSpeed[A]);

    if(PathFrameSpeed[A] <= 0) // validate
        PathFrameSpeed[A] = 1;
    config.endGroup();
}
