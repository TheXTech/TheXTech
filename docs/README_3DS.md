# About TheXTech & its Nintendo 3DS Port

TheXTech is an open-source rewrite of Super Mario Bros X. in C/++.
The original was written in VB6, targeting Windows only and eating up 2-3Gb of resources easily.

## 3DS Installation Guide

0. You are required to use the hacked 3DS.
1. Download and place the `thextech.3dsx` at the "3ds" directory at the root of SD card.
2. Download one of the game assets packages for TheXTech from here:
  - Original packages (will work slow without conversion into native 3DS format) https://github.com/Wohlstand/TheXTech/wiki/Game-assets-packages
  - Pre-converted packages as ROMFS (Optimized for 3DS):
    - Super Mario Bros. X: https://builds.wohlsoft.ru/3ds/assets-smbx13-3ds.zip
    - Adventures of Demo: https://builds.wohlsoft.ru/3ds/assets-aod-3ds.zip
3. Extract the downloaded archive with all it's content
4. Created a folder on your 3ds named “3ds/thextech” (case matters) in root of your SD card
    sdmc:/3ds/thextech
5. Move all extracted folders & files from the computer to sdmc:/3ds/thextech.
    Upon completion, if you use the original (not optmizied for 3DS) package, your folder structure should look like this:
        sdmc:/3ds/thextech/battle/
        sdmc:/3ds/thextech/graphics/
        sdmc:/3ds/thextech/music/
        sdmc:/3ds/thextech/sound/
        sdmc:/3ds/thextech/worlds/
        sdmc:/3ds/thextech/gameinfo.ini
        sdmc:/3ds/thextech/intro.lvl
        sdmc:/3ds/thextech/music.ini
        sdmc:/3ds/thextech/outro.lvl
        sdmc:/3ds/thextech/sounds.ini
    Otherwise, if you downloaded a .romfs package, simply place it into `sdmc:/3ds/thextech/` directory and rename it into `sdmc:/3ds/thextech/assets.romfs`.

6. Run `TheXTech` application via Homebrew Launcher and enjoy.


### KNOWN ISSUES

- LunaLua/LuaJIT episodes are not supported. (ATWE: A Tiny World Episode immediately comes to mind. The Episode shows in the world list, but upon trying to load the game will gracefully exit.)
- Has several performance issues on the Old 3DS

# TheXTech Standard Readme
