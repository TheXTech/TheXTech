# About TheXTech & its Nintendo Wii U Port

TheXTech is an open-source rewrite of Super Mario Bros X. in C/++.
The original was written in VB6, targeting Windows only and eating up 2-3Gb of resources easily.

## Wii U Installation Guide

0. You are required to use the hacked Wii.
1. Download and place files `thextech.rpx`, `icon.png`, and `meta.xml` at the "wiiu/apps/thextech-wiiu" directory at the root of SD card.
2. Download one of the game assets packages for TheXTech from here: https://github.com/Wohlstand/TheXTech/wiki/Game-assets-packages
3. Extract the downloaded archive with all it's content
4. Created a folder in root of your SD card named “thextech” (case matters)
    sd:/thextech
5. Move all extracted folders & files from the computer to sdmc:/thextech.
    Upon completion, your folder structure should look like this:
        sd:/thextech/battle/
        sd:/thextech/graphics/
        sd:/thextech/music/
        sd:/thextech/sound/
        sd:/thextech/worlds/
        sd:/thextech/gameinfo.ini
        sd:/thextech/intro.lvl
        sd:/thextech/music.ini
        sd:/thextech/outro.lvl
        sd:/thextech/sounds.ini

6. Run `TheXTech` application via Homebrew Launcher and enjoy.

### KNOWN ISSUES

- **The Wii U port is currently experimental and not fully tested.**

# TheXTech Standard Readme
