# About TheXTech & its Nintendo DSi Port

TheXTech is an open-source rewrite of Super Mario Bros X. in C++.
The original was written in VB6, targeting Windows only and eating up 2-3 GB of memory easily.

The DSi has 16 MB of system memory and 512 KB of video memory.

## DSi Installation Guide

0. You are required to use a hacked DSi. The game runs best through hbmenu but may work with other launchers. It is confirmed not to work with unlaunch.
1. Download and place the `thextech.nds` at any convenient directory on your SD.
2. Create a folder on your DSi named “/TheXTech/assets/” from the root of your SD card:
    sd:/TheXTech/assets/
3. Download one of the game assets packages for TheXTech from here:
  - Pre-converted packages in the dsi.xta format (optimized for DSi):
    - Super Mario Bros. X: https://www.wohlsoft.ru/projects/TheXTech/_downloads/assets/thextech-smbx13.dsi.xta
    - Adventures of Demo: https://www.wohlsoft.ru/projects/TheXTech/_downloads/assets/thextech-aod.dsi.xta
  - Place the file into the assets folder you created in the previous step:
    For instance, sd:/TheXTech/assets/thextech-aod.dsi.xta
4. Wait for the XTConvert tool to be released to convert your episodes to the dsi.xte format.
5. Place any converted episodes in sd:/TheXTech/worlds/smbx/ or sd:/TheXTech/worlds/aod/.
6. Run `TheXTech` application via Homebrew Launcher and enjoy.

## DSi Usage Guide

- For consistency with other platforms, the game uses B for jump/accept and Y for run/back. The keybindings can be configured in the options menu.
- The DSi screen resolution is 256x192 which yields a significantly smaller playfield than SMBX 1.3's 800x600. The L shoulder button toggles the viewport to match SMBX 1.3's. You can further customize the resolution in the options.

### KNOWN ISSUES

- This is a build straight off of TheXTech's development tree and may contain unexpected bugs
- Most content is playable, but depending on level content, performance may drop as low as 1 FPS
- On launchers other than hbmenu, the system crashes on attempting to exit the game
- Music/sound volume may be unbalanced at times
- Has <1s delays when loading music (can cause in-level stuttering)
- Minor graphical glitches (images sometimes do not line up correctly, leaving gaps)

# TheXTech Standard Readme
