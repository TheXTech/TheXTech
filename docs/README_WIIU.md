# About TheXTech & its Nintendo Wii U Port

TheXTech is an open-source rewrite of Super Mario Bros X. in C/++.
The original was written in VB6, targeting Windows only and eating up 2-3Gb of resources easily.

## Wii U Installation Guide

0. You are required to use the hacked Wii U.

### Generic way (resources on SD card as a directory)
This is a simplest way to get the game work on Wii U, and this way even allows you to customise the stuff on the fly as you do on the PC.
HOWEVER, with this way, the game LOADS WERY SLOW on hardware because of the how SD card file system works on Wii U.

1. Download and place files `thextech.rpx`, `icon.png`, and `meta.xml` at the "wiiu/apps/thextech-wiiu" directory at the root of SD card.
2. Download one of the game assets packages for TheXTech from here: https://github.com/Wohlstand/TheXTech/wiki/Game-assets-packages
3. Extract the downloaded archive with all it's content
4. Created a folder "wiiu" in root of your SD card if not exist, and then, inside, make another directory named "thextech" (case matters)
    `sd:/wiiu/thextech`
5. Move all extracted folders & files from the computer to `sd:/wiiu/thextech`.
    Upon completion, your folder structure should look like this:
        sd:/wiiu/thextech/battle/
        sd:/wiiu/thextech/graphics/
        sd:/wiiu/thextech/music/
        sd:/wiiu/thextech/sound/
        sd:/wiiu/thextech/worlds/
        sd:/wiiu/thextech/gameinfo.ini
        sd:/wiiu/thextech/intro.lvl
        sd:/wiiu/thextech/music.ini
        sd:/wiiu/thextech/outro.lvl
        sd:/wiiu/thextech/sounds.ini

6. Run `TheXTech` application via Homebrew Launcher and enjoy.

### Packing into WOHB
To run game via this way, you are required to have the `wohbtool` program (a part of DevkitPro: https://github.com/devkitPro/wut-tools).

**Important note:** Currently CEMU doesn't support WOHB files yet (see details: https://github.com/cemu-project/Cemu/issues/962), so, use this way to play on hardware. In CEMU, the Generic way should work good and load fast.

1. Download the `thextech.rpx` file and place it at any convenient place on your PC.
2. Download one of the game assets packages for TheXTech from here: https://github.com/Wohlstand/TheXTech/wiki/Game-assets-packages
3. Extract the downloaded archive with all its content into new empty directory. If you want to add custom episodes, do that now: place any custom episodes that you want to play into the "worlds" sub-directory at the directory of extracted assets.
4. Run the `wuhbtool` with next arguments:
```
wuhbtool <path-to-RPX-file> <path-to-the-output-WOBH-file> --content=<path-to-assets-directory> --name="<Name-of-the-game>" --short-name="<NoTG>" --icon=<path-to-icon-file> --author="Put-the-name-of-game-creator"
```

**Where:**
- **path-to-RPX-file** - an absolute path to the `thextech.rpx` file that you downloaded recently.
- **path-to-the-output-WOBH-file** - An absolute or relative path to the output filename that will be created. It should end with the ".wohb" suffix.
- **Name-of-the-game** - The understandible name of the game. Give it name from an assets name, for example, "Adventures of Demo", "Lowser's Conquest", etc.
- **NoTG** - The shortened name of the game, can be just an acronym.
- **path-to-icon-file** - The absolute path to the icon file that will be used as game icon: Give an absolute path to assets' "graphics/ui/icon/thextech_128.png" file that is being used as a game icon of the assets package.
- **Put-the-name-of-game-creator** - The name of the author who created the game of this assets pack. For example, "Redigit" (SMBX), "Wohlstand" (AoD), "Talkhaus" ('Analogue Funk' or 'Prelude To The Stupid'), "Sednaiur" (Lowser's Conquest), etc.

After you run the tool with giving proper arguments values, you will get the .wobh file that you can run on your hardware Wii U and have faster loading time.

And now, you can try to launch the package throgh the Aroma or something also.

In addition, there is a note that unlike the generic way, game saves and other stuff appears in different places:
- Settings and game saves will appear at the system-wide gamesave directory. The name of directory is directly depends on the name of the WOHB file, and if you rename it, the gamesave directory will be changed!
- If you plugged an SD card or USB stick, the `wiiu/thextech-user/<title-id>` sub-directory will appear on it: here are screenshots and logs will be saved.
- Inside `wiiu/thextech-user` directory, there are sub-directories with the "Title-ID" value (a hexidecial number) that will contain the different stuff depending on currently running assets package.


### KNOWN ISSUES

- **The Wii U port is currently experimental and not fully tested.**
- Has a very long loading when placing all resources on SD card: the file system of SD card is slow. Suggested to make a WUHB package and run it as a monolythig game package.
- Running the release build on CEMU, you may notice that control gets lost after returning back to the world map. The source of error are bugs in the CEMU's recompiler. To escape this until these bugs gets fixed later, it's need to run the CEMU with the `--force-interpreter` command-line argument.
- Attempt to change the scaling mode will lead a crash (at least on CEMU, not tested on hardware).

# TheXTech Standard Readme
