# a2xtech

Vanilla engine, rewritten into C++ from VB6


# Frequently Asking Questions

## What is this?
It's an port of an old VB6 engine, purely written in C++. It reproduces an old engine completely (except an Editor), includes lots of its logical bugs (crashy bugs where they were found got being fixed).

## Why you made it?
Why? I have several purposes for what I made it:
- It's a very convenient life model for researches I want to use in PGE Engine development.
- To make it work without necessary to use Wine on non-Windows platforms and allow to run it on any other than x86 platforms.
- To be able to optimize it to make it use fewer hardware resources than the original VB6-based build of a game.

## You have PGE Engine, why you have spent an over than one month to craft this thing?
I need it for PGE Engine development directly, it's much easier to hack and inspect rather dealing with old and inconvenient VB6 environment.

## What's future of PGE Engine as A2xTech now exist?
I'll continue development of PGE Engine as I still have to pass the second goal of PGE Project.
Since foundation, PGE Project had two goals: 1) save SMBX; 2) give a flexible toolkit for new platform games. Opening of SMBX sources and introducing the A2xTech has been solved a first goal: SMBX has been saved and now it's a free and opensource cross-platform software. PGE Engine will be used to pass the second goal - giving a toolkit for new games. Unlike A2xTech, PGE Engine gives a full flexibility that allows to build something new from scratch without inheriting of an old game base. However, A2xTech is needed for PGE Engine as a working research model to develop a new engine better.

## How it will be useful for the SMBX2/LunaLua project?
Potentially this thing will make any future DLL-injection hacks over old EXE be useless: as we now have this thing, now it's possible to backport a most of LunaLua base into this thing and get in final the standalone game instead of an ugly unstable clunky cludge that we had to deal over than 6 years! Any features can be implemented directly over A2xTech's source code rather crafting a dozen of suspicious ASM hacks are conditionally possible in comparison of direct source code hacking.

## How to use this?
Here are many ways to play games with it:
- there are some ready for use packages, just take and use as you did it with SMBX.
- use by the same way as an original game: put an executible file into the game root folder with an "a2xtech.ini" that contains next text:
```
[Main]
force-portable = true
```
, music.ini, sounds.ini and additional "graphics/ui" folder. An important note: all set of default graphics must be converted into PNG, use GIFs2PNG tool from PGE Project over your "graphics" folder with a "-d" switch. Don't use "-r" switch to keep original GIFs together with new made PNGs if you plan to continue use of original VB6-written SMBX.
- use it for debug mode: in your home directory, create the ".PGE_Project/a2xtech" folder where you should put a full set of game resources and worlds stuff, this folder will work like a game root in original game. This mode allow you to run an executible file from any folder location of your computer and use the same resources location for all builds (except of these are marked as portable by an INI file).

## What differences of this thing in comparison to original VB6 build?
- First off, it's written in C++ while original (as we already know) is written in VB6.
- Doesn't have an Editor. Instead, it will have a deep integration with PGE Editor that will allow to use it with the same functionality as in original editor (the "magic hand" functionality was kept to allow real-time editing of the level while testing, it's need to use IPC communication with PGE Editor to get an ability to use it better).
- Full support of UTF-8 in filename paths and internal text data (original game had the only 8bit ANSI support).
- For graphics and controlling, it uses an SDL2 library while original game have used WinAPI calls and GDI library.
- It uses PGE-FL that has a better file formats support.
- A support for WLDX world maps are allowing unlimited credits lines and custom music without necessary to use a music.ini for music replacements.
- Some LVLX exclusive features now working: vertical section wrap, two-way warps, custom "star needed" message, warp enter event, ability to disable stars printing in HUB episodes for specific door, ability to disable interscene showing when going to another level through a warp.
- Built-in support for episide and level wide music.ini and sounds.ini to override default music and sounds assets.
- World maps now supports a custom directory to store any specific resources like custom tiles/scenes/paths/levels and don't spam the episode root folder with wolrd map resources anymore.
- Default config format is INI, old config.dat format is no more supported, mainly because of incompatible key code values (SDL_Scancode versus VirtualKeys enum of Windows API).
- Game saves now using the SAVX format instead of a classic SAV. However, if you already have an old gamesave, you still can resume your game with using of a new engine now (next gamesave attempt will result a SAVX file, old gamesave in SAV format will be kept untouched).
- Built-in PNG support for custom and default graphics. Masked GIFs are still supported for backward compatibility, however, without making an unexpected auto-conversion like SMBX-38A does.
- Checkpoints now multi-points! You can use them in your levels multiple times without limits!
- It does use of lazy-decompress algorithm to speed-up the loading of a game and reduce the memory usage.
- For music and SFX, the MixerX library is used to give a support for a wide amount of sound and music formats!
- It doesn't embeds any graphics: here is NO any trurely hardcoded graphics, everything now representing as an external graphics!
- Some internal limits was been expanded.
- Built-in GIF recorder by F11 key (F10 on macOS, F11 is reserved by system UI for a "show desktop" action)

## How to build it?
To build it, you need to have next things:
- CMake
- Ninja optionally (speeds-up a build process)
- Compatible C/C++ compiler (GCC, Clang, MSVC didn't tested yet)
- Git (required to pull submodules and clone source of dependent libraries to build them in a place)
- Mercurial (required to clone an official SDL2 repository to build it in a place here)
- Optionally: system-wide installed dependencies: SDL2, libFreeImageLite (a modded implementation of the FreeImage), MixerX sound library, AudioCodecs collection of libraries. Having them be installed in a system giving a major build speed up. However, it's possible to build all these dependencies in a place here with a cost of an extra build time being added.

