#include "globals.h"
#include "custom_tiles.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <PGE_File_Formats/smbx64.h>
#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

void LoadCustomTile(int A, std::string cFileName);

void SaveTileDefaults()
{
    for(int A = 1; A <= maxTileType; A++)
    {
        TileDefaults.TileWidth[A] = TileWidth[A];
        TileDefaults.TileHeight[A] = TileHeight[A];
        TileDefaults.TileFrameCount[A] = TileFrameCount[A];
        TileDefaults.TileFrameSpeed[A] = TileFrameSpeed[A];
        TileDefaults.TileConnecting[A] = TileConnecting[A];
        TileDefaults.TileConnect[A] = TileConnect[A];
    }
}

void LoadTileDefaults()
{
    for(int A = 1; A <= maxTileType; A++)
    {
        TileWidth[A] = TileDefaults.TileWidth[A];
        TileHeight[A] = TileDefaults.TileHeight[A];
        TileFrameCount[A] = TileDefaults.TileFrameCount[A];
        TileFrameSpeed[A] = TileDefaults.TileFrameSpeed[A];
        TileConnecting[A] = TileDefaults.TileConnecting[A];
        TileConnect[A] = TileDefaults.TileConnect[A];
    }
}

void FindCustomTiles(/*std::string cFilePath*/)
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

    for(int A = 1; A < maxTileType; ++A)
    {

        std::string TilePathBasegame = AppPath + fmt::format_ne("/config/tile/tile-{0}.txt", A);

        std::string TileIniPath = FileNamePath + fmt::format_ne("tile-{0}.ini", A);
        std::string TileIniPathC = FileNamePath + FileName + fmt::format_ne("/tile-{0}.ini", A);
        std::string TilePath = FileNamePath + fmt::format_ne("tile-{0}.txt", A);
        std::string TilePathC = FileNamePath + FileName + fmt::format_ne("/tile-{0}.txt", A);

        if(Files::fileExists(TilePathBasegame))
            LoadCustomTile(A, TilePathBasegame);

        if(Files::fileExists(TileIniPath))
            LoadCustomTile(A, TileIniPath);
        if(Files::fileExists(TilePath))
            LoadCustomTile(A, TilePath);

        if(Files::fileExists(TileIniPathC))
            LoadCustomTile(A, TileIniPathC);
        if(Files::fileExists(TilePathC))
            LoadCustomTile(A, TilePathC);
    }
}

void LoadCustomTile(int A, std::string cFileName)
{
    IniProcessing config(cFileName);
    if(!config.beginGroup("tile"))
           config.beginGroup("General");


    config.read("width", TileWidth[A], TileWidth[A]);
    config.read("height", TileHeight[A], TileHeight[A]);

    if(TileWidth[A] <= 0) // Validate
        TileWidth[A] = 32;
    if(TileHeight[A] <= 0)
        TileHeight[A] = 32;

    config.read("frames", TileFrameCount[A], TileFrameCount[A]);
    if(TileFrameCount[A] <= 0) // Validate
        TileFrameCount[A] = 1;

    int frameDelay = 62;
    config.read("frame-delay", frameDelay, frameDelay);
    config.read("frame-speed", frameDelay, frameDelay);
    if(frameDelay <= 0) // validate
        frameDelay = 1;

   TileFrameSpeed[A] = int((double(frameDelay) / 1000.0) * 65.0);
    config.read("framespeed", TileFrameSpeed[A], TileFrameSpeed[A]);

    if(TileFrameSpeed[A] <= 0) // validate
        TileFrameSpeed[A] = 1;

    config.read("connecting", TileConnecting[A], TileConnecting[A]);
    config.read("connect", TileConnect[A], TileConnect[A]);


    config.endGroup();
}
