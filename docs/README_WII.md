# About TheXTech & its Nintendo Wii Port

TheXTech is an open-source rewrite of Super Mario Bros X. in C/++.
The original was written in VB6, targeting Windows only and eating up 2-3Gb of resources easily.

## Wii Installation Guide

0. You are required to use the hacked Wii with installed Homebrew Channel.
1. Download and place files `boot.dol`, `icon.png`, and `meta.xml` at the "apps/thextech-wii" directory at the root of SD card.
2. Download one of the game assets packages for TheXTech from here:
  - Original packages (will work slow without conversion into native Wii format) https://github.com/Wohlstand/TheXTech/wiki/Game-assets-packages
  - Pre-converted packages (Optimized for Wii):
    - Super Mario Bros. X: https://builds.wohlsoft.ru/wii/assets-smbx13-wii.zip
    - Adventures of Demo: https://builds.wohlsoft.ru/wii/assets-aod-wii.zip
3. Extract the downloaded archive with all it's content
4. Created a folder on your Wii named “thextech” (case matters) in root of your SD card
    sdmc:/thextech
5. Move all extracted folders & files from the computer to sdmc:/thextech.
    Upon completion, your folder structure should look like this:
        sdmc:/thextech/battle/
        sdmc:/thextech/graphics/
        sdmc:/thextech/music/
        sdmc:/thextech/sound/
        sdmc:/thextech/worlds/
        sdmc:/thextech/gameinfo.ini
        sdmc:/thextech/intro.lvl
        sdmc:/thextech/music.ini
        sdmc:/thextech/outro.lvl
        sdmc:/thextech/sounds.ini

6. Run `TheXTech` application via Homebrew Channel and enjoy.


### KNOWN ISSUES

- **The Wii port is currently experimental and not fully tested.**
- LunaLua/LuaJIT episodes are not supported. (ATWE: A Tiny World Episode immediately comes to mind. The Episode shows in the world list, but upon trying to load the game will gracefully exit.)
- Background pictures (especially animated) may sometimes fail to load because of insufficient memory.
- Minor graphical glitches (images sometimes do not line up correctly, leaving gaps)
- Has several performance issues related to Wii's slow SD card access
  - Has <1s delays when loading images (will cause in-level stuttering)
  - Has ~1s delays when storing game saves or settings to the SD card

# TheXTech Standard Readme
