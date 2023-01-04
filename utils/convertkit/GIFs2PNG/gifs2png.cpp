/*
 * GIFs2PNG, a free tool for merge GIF images with his masks and save into PNG
 * This is a part of the Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2017-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <locale>
#include <iostream>
#include <set>
#include <algorithm>
#include <cctype>
#include <stdio.h>

#include <FileMapper/file_mapper.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <Graphics/bitmask2rgba.h>
#include <Utf8Main/utf8main.h>
#include <tclap/CmdLine.h>
#include "version.h"

#include <FreeImageLite.h>

#include "common_features/config_manager.h"

static FIBITMAP *loadImage(const std::string &file, bool convertTo32bit = true)
{
#if  defined(__unix__) || defined(__APPLE__) || defined(_WIN32)
    FileMapper fileMap;
    if(!fileMap.open_file(file.c_str()))
        return nullptr;

    FIMEMORY *imgMEM = FreeImage_OpenMemory(reinterpret_cast<unsigned char *>(fileMap.data()),
                                            (unsigned int)fileMap.size());
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileTypeFromMemory(imgMEM);
    if(formato  == FIF_UNKNOWN)
        return nullptr;
    FIBITMAP *img = FreeImage_LoadFromMemory(formato, imgMEM, 0);
    FreeImage_CloseMemory(imgMEM);
    fileMap.close_file();
    if(!img)
        return nullptr;
#else
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(file.c_str(), 0);
    if(formato  == FIF_UNKNOWN)
        return nullptr;
    FIBITMAP *img = FreeImage_Load(formato, file.c_str());
    if(!img)
        return nullptr;
#endif

    if(convertTo32bit)
    {
        FIBITMAP *temp;
        temp = FreeImage_ConvertTo32Bits(img);
        if(!temp)
            return nullptr;
        FreeImage_Unload(img);
        img = temp;
    }
    return img;
}

static void mergeBitBltToRGBA(FIBITMAP *image, const std::string &pathToMask, FIBITMAP *extMask = nullptr)
{
    if(!image)
        return;

    if(!Files::fileExists(pathToMask) && !extMask)
        return; //Nothing to do

    FIBITMAP *mask = extMask ? extMask : loadImage(pathToMask);

    if(!mask)
        return;//Nothing to do

    bitmask_to_rgba(image, mask);

    if(!extMask)
        FreeImage_Unload(mask);
}


struct GIFs2PNG_Setup
{
    //! Input path (folder)
    std::string pathIn;
    //! Is a list of files mode (otherwise, checking entire folder)
    bool listOfFiles        = false;

    //! Output path (folder where destinition images will be saved)
    std::string pathOut;
    //! Put every image into same folder where original is located
    bool pathOutSame        = false;

    //! Path to configuration package which a source of missing mask files
    std::string configPath;
    //! List of masks (which are located in the episode-folder, are dependent to sub-directories and must be deleted after conversion completion)
    std::set<std::string> deleteLater;

    //! Source images are will be removed after conversion
    bool removeSource       = false;
    //! Scan also all subdirectories, otherwise only current folder content will be converted
    bool walkSubDirs        = false;
    //! Skip background2-*.gif images (which are rendering buggy in LunaLua in PNG format, in GIF there are valid)
    bool skipBackground2    = false;
    //! Count of successfully converted images
    unsigned int count_success  = 0;
    //! Count of failed conversions
    unsigned int count_failed   = 0;
    //! Count of skipped image conversions
    unsigned int count_skipped  = 0;
};

static inline void delEndSlash(std::string &dirPath)
{
    if(!dirPath.empty())
    {
        char last = dirPath[dirPath.size() - 1];
        if((last == '/') || (last == '\\'))
            dirPath.resize(dirPath.size() - 1);
    }
}

static inline void getGifMask(std::string &mask, const std::string &front)
{
    mask = front;
    //Make mask filename
    size_t dotPos = mask.find_last_of('.');
    if(dotPos == std::string::npos)
        mask.push_back('m');
    else
        mask.insert(mask.begin() + std::string::difference_type(dotPos), 'm');
}

void doGifs2PNG(std::string pathIn,  std::string imgFileIn,
                std::string pathOut,
                GIFs2PNG_Setup &setup,
                ConfigPackMiniManager &cnf)
{
    if(Files::hasSuffix(imgFileIn, "m.gif"))
        return; //Skip mask files

    //! Lower-case filename for case-insensitive checks
    std::string imgFileInL = imgFileIn;
    std::transform(imgFileInL.begin(), imgFileInL.end(), imgFileInL.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    std::string imgPathIn = pathIn + "/" + imgFileIn;
    std::string maskPathIn;

    std::cout << imgPathIn;
    std::cout.flush();

    bool isBackground2 = (imgFileInL.compare(0, 11, "background2", 11) == 0);

    if(setup.skipBackground2 && isBackground2)
    {
        setup.count_skipped++;
        std::cout << "...SKIP!\n";
        std::cout.flush();
        return;
    }

    std::string maskFileIn;
    bool maskIsReadOnly = false;
    getGifMask(maskFileIn, imgFileIn);

    maskPathIn = cnf.getFile(maskFileIn, pathIn, &maskIsReadOnly);

    FIBITMAP *image = loadImage(imgPathIn);
    if(!image)
    {
        setup.count_failed++;
        std::cout << "...CAN'T OPEN!\n";
        std::cout.flush();
        return;
    }

    bool isFail = false;
    bool maskIsExists = Files::fileExists(maskPathIn);

    if(maskIsExists) /* When mask exists, use it */
        mergeBitBltToRGBA(image, maskPathIn);
    else if(!isBackground2)/* Try to find the PNG as source of the mask, except of backgrounds */
    {
        maskFileIn = Files::changeSuffix(imgFileIn, ".png");
        maskPathIn = cnf.getFile(maskFileIn);
        std::cout << ".chkPNG.";
        if(Files::fileExists(maskPathIn))
        {
            FIBITMAP *front = loadImage(maskPathIn);
            if(front)
            {
                FIBITMAP *mask = nullptr;
                std::cout << ".PNG-AS-MASK.";
                bitmask_get_mask_from_rgba(front, &mask);
                FreeImage_Unload(front);
                mergeBitBltToRGBA(image, "", mask);
                FreeImage_Unload(mask);
            }
            else
            {
                std::cout << ".NO-MASK.";
            }
        }
    }

    if(image)
    {
        std::string outPath = pathOut + "/" + Files::changeSuffix(imgFileIn, ".png");
        int ret = FreeImage_Save(FIF_PNG, image, outPath.c_str());
        if(!ret)
        {
            std::cout << "...F-WRT FAILED!\n";
            isFail = true;
        }
        FreeImage_Unload(image);
    }

    if(isFail)
    {
        setup.count_failed++;
        std::cout << "...FAILED!\n";
    }
    else
    {
        setup.count_success++;
        if(setup.removeSource)// Detele old files
        {
            if(Files::deleteFile(imgPathIn))
                std::cout << ".F-DEL.";
            //Try to delete or delete-late mask if it is exist and is not read-only
            if(maskIsExists && !maskIsReadOnly)
            {
                /* Delete-Later if mask file is stored in the root of episode directory.
                   Mask file is dependent to images are inside the subfolder */
                if(!setup.listOfFiles && setup.walkSubDirs && (setup.pathIn == Files::dirname(maskPathIn)))
                    setup.deleteLater.insert(maskPathIn);
                else if(Files::deleteFile(maskPathIn)) //Or just delete the mask file
                    std::cout << ".M-DEL.";
            }
        }
        std::cout << "...done\n";
    }

    std::cout.flush();
}


int main(int argc, char *argv[])
{
    if(argc > 0)
        g_ApplicationPath = Files::dirname(argv[0]);
    g_ApplicationPath = DirMan(g_ApplicationPath).absolutePath();

    DirMan imagesDir;
    std::vector<std::string> fileList;
    FreeImage_Initialise();
    ConfigPackMiniManager config;

    GIFs2PNG_Setup setup;

    try
    {
        // Define the command line object.
        TCLAP::CmdLine  cmd(V_FILE_DESC "\n"
                            "Copyright (c) 2017-2023 Vitaly Novichkov <admin@wohlnet.ru>\n"
                            "This program is distributed under the GNU GPLv3+ license\n", ' ', V_FILE_VERSION V_FILE_RELEASE);

        TCLAP::SwitchArg switchRemove("r", "remove", "Remove source images after a succesful conversion", false);
        TCLAP::SwitchArg switchSkipBG("b", "ingnore-bg", "Skip all \"background2-*.gif\" sprites", false);
        TCLAP::SwitchArg switchDigRecursive("d", "dig-recursive", "Look for images in subdirectories", false);
        TCLAP::SwitchArg switchDigRecursiveDEP("w", "dig-recursive-old", "Look for images in subdirectories [deprecated]", false);

        TCLAP::ValueArg<std::string> outputDirectory("O", "output",
                "path to a directory where the PNG images will be saved",
                false, "", "/path/to/output/directory/");
        TCLAP::ValueArg<std::string> configDirectory("C", "config",
                "Allow usage of default masks from specific PGE config pack "
                "(Useful for cases where the GFX designer didn't make a mask image)",
                false, "", "/path/to/config/pack");
        TCLAP::UnlabeledMultiArg<std::string> inputFileNames("filePath(s)",
                "Input GIF file(s)",
                true,
                "Input file path(s)");

        cmd.add(&switchRemove);
        cmd.add(&switchSkipBG);
        cmd.add(&switchDigRecursive);
        cmd.add(&switchDigRecursiveDEP);
        cmd.add(&outputDirectory);
        cmd.add(&configDirectory);
        cmd.add(&inputFileNames);

        cmd.parse(argc, argv);

        setup.removeSource      = switchRemove.getValue();
        setup.skipBackground2 = switchSkipBG.getValue();
        setup.walkSubDirs     = switchDigRecursive.getValue() | switchDigRecursiveDEP.getValue();
        //nopause         = switchNoPause.getValue();

        setup.pathOut     = outputDirectory.getValue();
        setup.configPath  = configDirectory.getValue();

        for(const std::string &fpath : inputFileNames.getValue())
        {
            if(Files::hasSuffix(fpath, "m.gif"))
                continue;
            else if(DirMan::exists(fpath))
                setup.pathIn = fpath;
            else
            {
                fileList.push_back(fpath);
                setup.listOfFiles = true;
            }
        }

        if((argc <= 1) || (setup.pathIn.empty() && !setup.listOfFiles))
        {
            fprintf(stderr, "\n"
                    "ERROR: Missing input files!\n"
                    "Type \"%s --help\" to display usage.\n\n", argv[0]);
            return 2;
        }
    }
    catch(TCLAP::ArgException &e)   // catch any exceptions
    {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 2;
    }

    fprintf(stderr, "============================================================================\n"
            "Pair of GIFs to PNG converter tool by Wohlstand. Version " V_FILE_VERSION V_FILE_RELEASE "\n"
            "============================================================================\n"
            "This program is distributed under the GNU GPLv3 license \n"
            "============================================================================\n");
    fflush(stderr);

    if(!setup.listOfFiles)
    {
        if(setup.pathIn.empty())
            goto WrongInputPath;
        if(!DirMan::exists(setup.pathIn))
            goto WrongInputPath;

        imagesDir.setPath(setup.pathIn);
        setup.pathIn = imagesDir.absolutePath();
    }

    if(!setup.pathOut.empty())
    {
        if(!DirMan::exists(setup.pathOut) && !DirMan::mkAbsPath(setup.pathOut))
            goto WrongOutputPath;

        setup.pathOut = DirMan(setup.pathOut).absolutePath();
        setup.pathOutSame   = false;
    }
    else
    {
        setup.pathOut       = setup.pathIn;
        setup.pathOutSame   = true;
    }

    delEndSlash(setup.pathIn);
    delEndSlash(setup.pathOut);

    printf("============================================================================\n"
           "Converting images...\n"
           "============================================================================\n");
    fflush(stdout);

    if(!setup.listOfFiles)
        std::cout << ("Input path:  " + setup.pathIn + "\n");

    std::cout << ("Output path: " + setup.pathOut + "\n");

    std::cout << "============================================================================\n";
    std::cout.flush();

    config.setConfigDir(setup.configPath);

    if(config.isUsing())
    {
        std::cout << "============================================================================\n";
        std::cout << ("Used config pack: " + setup.configPath + "\n");
        if(!setup.listOfFiles)
        {
            config.appendDir(setup.pathIn);
            std::cout << ("With episode directory: " + setup.pathIn + "\n");
        }
        std::cout << "============================================================================\n";
        std::cout.flush();
    }

    if(setup.listOfFiles)// Process a list of flies
    {
        for(std::string &file : fileList)
        {
            std::string fname   = Files::basename(file);
            setup.pathIn = DirMan(Files::dirname(file)).absolutePath();
            if(setup.pathOutSame)
                setup.pathOut = DirMan(Files::dirname(file)).absolutePath();
            doGifs2PNG(setup.pathIn, fname , setup.pathOut, setup, config);
        }
    }
    else // Process directories with a source files
    {
        imagesDir.getListOfFiles(fileList, {".gif"});
        if(!setup.walkSubDirs) //By directories
        {
            for(std::string &fname : fileList)
                doGifs2PNG(setup.pathIn, fname, setup.pathOut, setup, config);
        }
        else
        {
            imagesDir.beginWalking({".gif"});
            std::string curPath;
            while(imagesDir.fetchListFromWalker(curPath, fileList))
            {
                if(Files::hasSuffix(curPath, "/_backup"))
                    continue; //Skip LazyFix's backup directories
                for(std::string &file : fileList)
                {
                    if(setup.pathOutSame)
                        setup.pathOut = curPath;
                    doGifs2PNG(curPath, file, setup.pathOut, setup, config);
                }
            }
        }
    }

    if(!setup.deleteLater.empty())
    {
        printf("======================Deleting old files...=================================\n");
        fflush(stdout);
        for(const std::string &s : setup.deleteLater)
        {
            std::cout << s << "...";
            std::cout.flush();
            if(Files::deleteFile(s))
                std::cout << ".DEL.";
            std::cout << "\n";
            std::cout.flush();
        }
    }

    printf("============================================================================\n"
           "                      Conversion has been completed!\n"
           "============================================================================\n"
           "Successfully merged:        %d\n"
           "Conversion failed:          %d\n"
           "Skipped files (bg2-*):      %d\n"
           "\n",
           setup.count_success,
           setup.count_failed,
           setup.count_skipped);
    fflush(stdout);
    return (setup.count_failed == 0) ? 0 : 1;

WrongInputPath:
    std::cout.flush();
    std::cerr.flush();
    printf("============================================================================\n"
           "                           Wrong input path!\n"
           "============================================================================\n");
    fflush(stdout);
    return 2;

WrongOutputPath:
    std::cout.flush();
    std::cerr.flush();
    printf("============================================================================\n"
           "                          Wrong output path!\n"
           "============================================================================\n");
    fflush(stdout);
    return 2;
}
