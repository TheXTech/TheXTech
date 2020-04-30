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
        std::string BlockIniPath = FileNamePath + fmt::format_ne("block-{0}.ini", A);
        std::string BlockIniPathC = FileNamePath + FileName + fmt::format_ne("/block-{0}.ini", A);
        std::string BlockPath = FileNamePath + fmt::format_ne("block-{0}.txt", A);
        std::string BlockPathC = FileNamePath + FileName + fmt::format_ne("/block-{0}.txt", A);

        if(Files::fileExists(BlockIniPath))
            LoadCustomBlock(A, BlockIniPath);
        if(Files::fileExists(BlockPath))
            LoadCustomBlock(A, BlockPath);

        if(Files::fileExists(BlockIniPathC))
            LoadCustomBlock(A, BlockIniPathC);
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

    config.read("player-passthrough", BlockPlayerNoClipping[A], BlockPlayerNoClipping[A]);
    config.read("playerpassthrough", BlockPlayerNoClipping[A], BlockPlayerNoClipping[A]);//Alias

    config.read("npc-passthrough", BlockNPCNoClipping[A], BlockNPCNoClipping[A]);
    config.read("npcpassthrough", BlockNPCNoClipping[A], BlockNPCNoClipping[A]); // alias

    config.read("passthrough", BlockNoClipping[A], BlockNoClipping[A]);

    config.read("floorslope", BlockSlope[A], BlockSlope[A]);
    config.read("cellingslope", BlockSlope2[A], BlockSlope2[A]);

    if(BlockSlope[A] != 0 && BlockSlope2[A] != 0) // Validate input
    {
        BlockSlope[A] = 0;
        BlockSlope2[A] = 0;
    }

    if(BlockSlope[A] > 1) // Validate range
        BlockSlope[A] = 1;
    else if(BlockSlope[A] < -1)
        BlockSlope[A] = -1;

    if(BlockSlope2[A] > 1) // Validate range
        BlockSlope2[A] = 1;
    else if(BlockSlope2[A] < -1)
        BlockSlope2[A] = -1;

    int shapeType;
    config.read("shape", shapeType, 999);
    config.read("shape-type", shapeType, shapeType);//alias

    switch(shapeType)
    {
    case 0:
        BlockSlope[A] = 0;
        BlockSlope2[A] = 0;
        break;
    case -1:
        BlockSlope[A] = -1;
        BlockSlope2[A] = 0;
        break;
    case 1:
        BlockSlope[A] = 1;
        BlockSlope2[A] = 0;
        break;
    case -2:
        BlockSlope[A] = 0;
        BlockSlope2[A] = 1;
        break;
    case 2:
        BlockSlope[A] = 0;
        BlockSlope2[A] = -1;
        break;
    default:
        // Field undefined, do nothing
        break;
    }

    config.read("width", BlockWidth[A], BlockWidth[A]);
    config.read("height", BlockHeight[A], BlockHeight[A]);

    if(BlockWidth[A] <= 0) // Validate
        BlockWidth[A] = 32;
    if(BlockHeight[A] <= 0)
        BlockHeight[A] = 32;

    config.read("semi-solid", BlockOnlyHitspot1[A], BlockOnlyHitspot1[A]);
    config.read("semisolid", BlockOnlyHitspot1[A], BlockOnlyHitspot1[A]); // alias

    config.read("lava", BlockKills[A], BlockKills[A]);
    config.read("kills", BlockKills3[A], BlockKills3[A]);
    config.read("hurts", BlockHurts[A], BlockHurts[A]);

    config.read("p-switch", BlockPSwitch[A], BlockPSwitch[A]);
    config.read("pswitch", BlockPSwitch[A], BlockPSwitch[A]); // alias

    config.read("bounce", BlockBouncy[A], BlockBouncy[A]);

    config.read("bounce-side", BlockBouncyHorizontal[A], BlockBouncyHorizontal[A]);
    config.read("bounceside", BlockBouncyHorizontal[A], BlockBouncyHorizontal[A]);//alias

    config.read("frames", BlockFrameCount[A], BlockFrameCount[A]);
    if(BlockFrameCount[A] <= 0) // Validate
        BlockFrameCount[A] = 1;

    int frameDelay = 62;
    config.read("frame-delay", frameDelay, frameDelay);
    config.read("frame-speed", frameDelay, frameDelay);
    if(frameDelay <= 0) // validate
        frameDelay = 1;

    BlockFrameSpeed[A] = int((double(frameDelay) / 1000.0) * 65.0);
    config.read("framespeed", BlockFrameSpeed[A], BlockFrameSpeed[A]);

    if(BlockFrameSpeed[A] <= 0) // validate
        BlockFrameSpeed[A] = 1;

    config.endGroup();
}
