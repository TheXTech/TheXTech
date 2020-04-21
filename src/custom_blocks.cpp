#include "globals.h"
#include "custom_blocks.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <PGE_File_Formats/smbx64.h>
#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

void LoadCustomBlock(int A, std::string cFileName);

void SaveBlockDefaults()
{
    for(int A = 1; A <= maxBlockType; A++)
    {
        BlockDefaults.BlockIsSizable[A] = BlockIsSizable[A];
        BlockDefaults.BlockPlayerNoClipping[A] = BlockPlayerNoClipping[A];
        BlockDefaults.BlockSlope[A] = BlockSlope[A];
        BlockDefaults.BlockSlope2[A] = BlockSlope2[A];
        BlockDefaults.BlockWidth[A] = BlockWidth[A];
        BlockDefaults.BlockHeight[A] = BlockHeight[A];
        BlockDefaults.BlockOnlyHitspot1[A] = BlockOnlyHitspot1[A];
        BlockDefaults.BlockKills[A] = BlockKills[A];
        BlockDefaults.BlockHurts[A] = BlockHurts[A];
        BlockDefaults.BlockPSwitch[A] = BlockPSwitch[A];
        BlockDefaults.BlockNoClipping[A] = BlockNoClipping[A];
        BlockDefaults.BlockNPCNoClipping[A] = BlockNPCNoClipping[A];
        BlockFrameSpeed[A] = 8;
    }
}

void LoadBlockDefaults()
{
    for(int A = 1; A <= maxBlockType; A++)
    {
        BlockIsSizable[A] = BlockDefaults.BlockIsSizable[A];
        BlockPlayerNoClipping[A] = BlockDefaults.BlockPlayerNoClipping[A];
        BlockSlope[A] = BlockDefaults.BlockSlope[A];
        BlockSlope2[A] = BlockDefaults.BlockSlope2[A];
        BlockWidth[A] = BlockDefaults.BlockWidth[A];
        BlockHeight[A] = BlockDefaults.BlockHeight[A];
        BlockOnlyHitspot1[A] = BlockDefaults.BlockOnlyHitspot1[A];
        BlockKills[A] = BlockDefaults.BlockKills[A];
        BlockHurts[A] = BlockDefaults.BlockHurts[A];
        BlockPSwitch[A] = BlockDefaults.BlockPSwitch[A];
        BlockNoClipping[A] = BlockDefaults.BlockNoClipping[A];
        BlockNPCNoClipping[A] = BlockDefaults.BlockNPCNoClipping[A];
        BlockFrame[A] = 0;
        BlockFrameSpeed[A] = 8;
    }
}

void FindCustomBlocks(/*std::string cFilePath*/)
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

    for(int A = 1; A < maxBlockType; ++A)
    {
        std::string BlockPath = FileNamePath + fmt::format_ne("Block-{0}.txt", A);
        std::string BlockPathC = FileNamePath + FileName + fmt::format_ne("/Block-{0}.txt", A);
        if(Files::fileExists(BlockPath))
            LoadCustomBlock(A, BlockPath);
        if(Files::fileExists(BlockPathC))
            LoadCustomBlock(A, BlockPathC);
    }
}


void LoadCustomBlock(int A, std::string cFileName)
{
    IniProcessing config(cFileName);
    if(!config.beginGroup("block"))
           config.beginGroup("General");
    config.read("issizeable", BlockIsSizable[A], false);
    config.read("playernoclipping", BlockPlayerNoClipping[A], false);
    config.read("npcnoclipping", BlockNPCNoClipping[A], false);
    config.read("noclipping", BlockNoClipping[A], false);
    config.read("floorslope", BlockSlope[A], 0);
    config.read("cellingslope", BlockSlope2[A], 0);
    config.read("width", BlockWidth[A], 32);
    config.read("height", BlockHeight[A], 32);
    config.read("hitspot1", BlockOnlyHitspot1[A], false);
    config.read("kills", BlockKills[A], false);
    config.read("hurts", BlockHurts[A], false);
    config.read("pswitch", BlockPSwitch[A], false);
    config.read("frames", BlockFrame[A], 0);
    config.read("framespeed", BlockFrameSpeed[A], 8);
    config.endGroup();
}
