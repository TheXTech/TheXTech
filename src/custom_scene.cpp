#include "globals.h"
#include "custom_scene.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <PGE_File_Formats/smbx64.h>
#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

void LoadCustomScenery(int A, std::string cFileName);

void SaveSceneryDefaults()
{
    for(int A = 1; A <= maxSceneType; A++)
    {
        SceneDefaults.SceneWidth[A] = SceneWidth[A];
        SceneDefaults.SceneHeight[A] = SceneHeight[A];
        SceneDefaults.SceneFrameCount[A] = SceneFrameCount[A];
        SceneDefaults.SceneFrameSpeed[A] = SceneFrameSpeed[A];
        SceneDefaults.SceneMovement[A] = SceneMovement[A];
    }
}

void LoadSceneryDefaults()
{
    for(int A = 1; A <= maxSceneType; A++)
    {
        SceneWidth[A] = SceneDefaults.SceneWidth[A];
        SceneHeight[A] = SceneDefaults.SceneHeight[A];
        SceneFrameCount[A] = SceneDefaults.SceneFrameCount[A];
        SceneFrameSpeed[A] = SceneDefaults.SceneFrameSpeed[A];
        SceneMovement[A] = SceneDefaults.SceneMovement[A];
    }
}

void FindCustomScenery(/*std::string cFilePath*/)
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

    for(int A = 1; A < maxSceneType; ++A)
    {

        std::string ScenePathBasegame = AppPath + fmt::format_ne("/config/scene/scene-{0}.txt", A);

        std::string SceneIniPath = FileNamePath + fmt::format_ne("scene-{0}.ini", A);
        std::string SceneIniPathC = FileNamePath + FileName + fmt::format_ne("/scene-{0}.ini", A);
        std::string ScenePath = FileNamePath + fmt::format_ne("scene-{0}.txt", A);
        std::string ScenePathC = FileNamePath + FileName + fmt::format_ne("/scene-{0}.txt", A);

        if(Files::fileExists(ScenePathBasegame))
            LoadCustomScenery(A, ScenePathBasegame);

        if(Files::fileExists(SceneIniPath))
            LoadCustomScenery(A, SceneIniPath);
        if(Files::fileExists(ScenePath))
            LoadCustomScenery(A, ScenePath);

        if(Files::fileExists(SceneIniPathC))
            LoadCustomScenery(A, SceneIniPathC);
        if(Files::fileExists(ScenePathC))
            LoadCustomScenery(A, ScenePathC);
    }
}

void LoadCustomScenery(int A, std::string cFileName)
{
    IniProcessing config(cFileName);
    if(!config.beginGroup("Scene"))
           config.beginGroup("General");


    config.read("width", SceneWidth[A], SceneWidth[A]);
    config.read("height", SceneHeight[A], SceneHeight[A]);

    if(SceneWidth[A] <= 0) // Validate
        SceneWidth[A] = 32;
    if(SceneHeight[A] <= 0)
        SceneHeight[A] = 32;

    config.read("frames", SceneFrameCount[A], SceneFrameCount[A]);
    if(SceneFrameCount[A] <= 0) // Validate
        SceneFrameCount[A] = 1;

    int frameDelay = 62;
    config.read("frame-delay", frameDelay, frameDelay);
    config.read("frame-speed", frameDelay, frameDelay);
    if(frameDelay <= 0) // validate
        frameDelay = 1;

    SceneFrameSpeed[A] = int((double(frameDelay) / 1000.0) * 65.0);
    config.read("framespeed", SceneFrameSpeed[A], SceneFrameSpeed[A]);

    if(SceneFrameSpeed[A] <= 0) // validate
        SceneFrameSpeed[A] = 1;

    config.read("movement", SceneMovement[A], SceneMovement[A]);
    config.read("movementspeed", SceneMovementSpeed[A], SceneMovementSpeed[A]);
    config.read("movement-speed", SceneMovementSpeed[A], SceneMovementSpeed[A]);
    config.endGroup();
}
