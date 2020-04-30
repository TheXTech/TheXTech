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
        BlockDefaults.BlockKills3[A] = BlockKills3[A];
        BlockDefaults.BlockHurts[A] = BlockHurts[A];
        BlockDefaults.BlockPSwitch[A] = BlockPSwitch[A];
        BlockDefaults.BlockNoClipping[A] = BlockNoClipping[A];
        BlockDefaults.BlockNPCNoClipping[A] = BlockNPCNoClipping[A];
        BlockDefaults.BlockBouncy[A] = BlockBouncy[A];
        BlockDefaults.BlockBouncyHorizontal[A] = BlockBouncyHorizontal[A];
        BlockDefaults.BlockFrameCount[A] = BlockFrameCount[A];
        BlockDefaults.BlockFrameSpeed[A] = BlockFrameSpeed[A];
    }
}

void LoadBlockDefaults()
{
    for(int A = 1; A <= maxBlockType; A++)
    {
        BlockPlayerNoClipping[A] = BlockDefaults.BlockPlayerNoClipping[A];
        BlockSlope[A] = BlockDefaults.BlockSlope[A];
        BlockSlope2[A] = BlockDefaults.BlockSlope2[A];
        BlockWidth[A] = BlockDefaults.BlockWidth[A];
        BlockHeight[A] = BlockDefaults.BlockHeight[A];
        BlockOnlyHitspot1[A] = BlockDefaults.BlockOnlyHitspot1[A];
        BlockKills[A] = BlockDefaults.BlockKills[A];
        BlockKills3[A] = BlockDefaults.BlockKills3[A];
        BlockHurts[A] = BlockDefaults.BlockHurts[A];
        BlockPSwitch[A] = BlockDefaults.BlockPSwitch[A];
        BlockNoClipping[A] = BlockDefaults.BlockNoClipping[A];
        BlockNPCNoClipping[A] = BlockDefaults.BlockNPCNoClipping[A];
        BlockBouncy[A] = BlockDefaults.BlockBouncy[A];
        BlockBouncyHorizontal[A] = BlockDefaults.BlockBouncyHorizontal[A];
        BlockFrameCount[A] = BlockDefaults.BlockFrameCount[A];
        BlockFrameSpeed[A] = BlockDefaults.BlockFrameSpeed[A];
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
        std::string BlockPath = FileNamePath + fmt::format_ne("block-{0}.txt", A);
        std::string BlockPathC = FileNamePath + FileName + fmt::format_ne("/block-{0}.txt", A);
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
    config.read("sizeable", BlockIsSizable[A], BlockIsSizable[A]);
    config.read("player-no-clipping", BlockPlayerNoClipping[A], BlockPlayerNoClipping[A]);
    config.read("npc-no-clipping", BlockNPCNoClipping[A], BlockNPCNoClipping[A]);
    config.read("no-clipping", BlockNoClipping[A], BlockNoClipping[A]);
    config.read("floor-slope", BlockSlope[A], BlockSlope[A]);
    config.read("celling-slope", BlockSlope2[A], BlockSlope2[A]);
    config.read("width", BlockWidth[A], BlockWidth[A]);
    config.read("height", BlockHeight[A], BlockHeight[A]);
    config.read("semi-solid", BlockOnlyHitspot1[A], BlockOnlyHitspot1[A]);
    config.read("lava", BlockKills[A], BlockKills[A]);
    config.read("kills", BlockKills3[A], BlockKills3[A]);
    config.read("hurts", BlockHurts[A], BlockHurts[A]);
    config.read("switch-id", BlockPSwitch[A], BlockPSwitch[A]);
    config.read("bounce", BlockBouncy[A], BlockBouncy[A]);
    config.read("bounce-side", BlockBouncyHorizontal[A], BlockBouncyHorizontal[A]);
    config.read("frames", BlockFrameCount[A], BlockFrameCount[A]);
    config.read("frame-speed", BlockFrameSpeed[A], BlockFrameSpeed[A]);
    config.endGroup();
}
