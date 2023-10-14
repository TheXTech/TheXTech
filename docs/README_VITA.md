# About TheXTech & its Vita Port

TheXTech is an open-source rewrite of Super Mario Bros X. in C/++.
The original was written in VB6, targeting Windows only and eating up 2-3Gb of resources easily.

*Almost* every other piece of code compiled with little fighting for the 333 Mhz RISC handheld. Performance is also *pretty good* if I do say so myself. SDL2 is being used for *everything* in this current version. Though, support for a more Vita-specific rendering engine may come at a later date.

## Vita Installation Guide

0. While possible to move all assets over FTP, it’s much faster and more reliable to mount your Vita over USB or move your Vita2SD to your computer and copy that way. You’ve been warned.
1. Download and install `thextech.vpk`.
2. Download one of the game assets packages for TheXTech from here: https://github.com/Wohlstand/TheXTech/wiki/Game-assets-packages
3. Extract the downloaded archive with all it's content
4. Created a folder on your Vita named “TheXTech” (case matters) in ux0:data
        ux0:data/TheXTech
5. Move all extracted folders & files from the computer to ux0:data/TheXTech.
    Upon completion, your folder structure should look like this:
        ux0:data/TheXTech/battle/
        ux0:data/TheXTech/graphics/
        ux0:data/TheXTech/music/
        ux0:data/TheXTech/sound/
        ux0:data/TheXTech/worlds/
        ux0:data/TheXTech/gameinfo.ini
        ux0:data/TheXTech/intro.lvl
        ux0:data/TheXTech/music.ini
        ux0:data/TheXTech/outro.lvl
        ux0:data/TheXTech/sounds.ini

6. Install `thextech.vpk` and enjoy.


### KNOWN ISSUES

- Slowdowns may happen.
- SOME Midi tracks may kill performance (specifcially: using the libOPNMidi backend. this is pretty rare and midi support through libOPN and libADL are all relatively new to SMBX)
- LunaLua/LuaJIT episodes are not supported. (ATWE: A Tiny World Episode immediately comes to mind. The Episode shows in the world list, but upon trying to load the game will gracefully exit.)


## Addendum

I was a huge fan, and VERY involved, in the original Super Mario Bros. X community as **Luigifan2010**.
When Wohlstand first came to me mid 2020 teasing TheXTech, I immediately thought about porting it
to the Vita. Well now, here it is. The stars finally aligned and I've taught myself enough C to figure
this out!

If you want to learn how to code, just do it! There are tons of resources out there.

And most of all, have FUN while playing this port!

- Axiom


# TheXTech Standard Readme
