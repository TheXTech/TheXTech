# About TheXTech & its Nintendo Switch Port

TheXTech is an open-source rewrite of Super Mario Bros X. in C/++.
The original was written in VB6, targeting Windows only and eating up 2-3Gb of resources easily.

## Switch Installation Guide

0. You are required to use the hacked Switch.
1. Download and place the `thextech.nro` at the "switch" directory at the root of SD card.
2. Download one of the game assets packages for TheXTech from here: https://github.com/Wohlstand/TheXTech/wiki/Game-assets-packages
3. Extract the downloaded archive with all it's content
4. Created a folder on your Switch named “TheXTech” (case matters) in root of your SD card
        sdmc:/TheXTech
5. Move all extracted folders & files from the computer to sdmc:/TheXTech.
    Upon completion, your folder structure should look like this:
        sdmc:/TheXTech/battle/
        sdmc:/TheXTech/graphics/
        sdmc:/TheXTech/music/
        sdmc:/TheXTech/sound/
        sdmc:/TheXTech/worlds/
        sdmc:/TheXTech/gameinfo.ini
        sdmc:/TheXTech/intro.lvl
        sdmc:/TheXTech/music.ini
        sdmc:/TheXTech/outro.lvl
        sdmc:/TheXTech/sounds.ini

6. Run `thextech.nro` via homebrew and enjoy.


### KNOWN ISSUES

- LunaLua/LuaJIT episodes are not supported. (ATWE: A Tiny World Episode immediately comes to mind. The Episode shows in the world list, but upon trying to load the game will gracefully exit.)


# TheXTech Standard Readme
