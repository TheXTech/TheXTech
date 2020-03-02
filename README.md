# a2xtech

Vanilla engine, rewritten into C++ from VB6


# Frequently Asking Questions

## What is this?
It's an implementation of an old VB6 engine, purely written in C++. It reproduces an old engine completely (except an Editor), includes lots of its logical bugs (crashy bugs where they were found got being fixed).

## Why you made it?
Why? I have several purposes for what I did:
- It's a very convenient working model for researches I want to use in PGE Engine development.
- To make it work without necessary to use Wine on non-Windows platforms and allow to run it on any other than x86 platforms.
- To be able to optimize it to make it use fewer hardware resources than the original VB6-based build of a game.

## You have PGE Engine, why you have spent an over than one month to craft this thing?
I need it for PGE Engine development directly, it's much easier to hack and inspect rather dealing with old and inconvenient VB6 environment. Anyway look: Since a foundation, PGE Engine had two goals: a saving of SMBX; a providing of a clear modern platform for new platform genre games. Since I built an A2xTech, it has completed the first goal of the PGE Engine. So, a development flow of PGE Engine will be just changed: instead of attempts to reverse-engine an old thing, use an experience of an A2xTech thing to complete a second goal: PGE Engine is positioning as a modern and flexible game engine written from scratch and doesn't inherit an old codebase (hardcoded NPC algorithms and game logic) as A2xTech has now. A2xTech will be used as a reference and the working life model that can be used for any future experiments. It will take integration with PGE Editor to allow running of level tests on it without a strong effort.

## How it will be useful for the SMBX2/LunaLua project?
Potentially this thing will make any future DLL-injection hacks over old EXE be useless: as we now have this thing, it's now possible to backport a most of LunaLua base into this thing and finally have a standalone game instead of an ugly unstable and clunky cludge that we had to deal over than 6 years! Any features can be implemented directly over A2xTech's source code rather crafting dozen of suspicious ASM hacks are conditionally possible in comparison of direct source code hacking.

## How to use this?
Here are many ways to play games with it:
- use by the same way as an original game: put an executible file into the game root folder with an "a2xtech.ini" that contains "[Main]\nforce-portable = true" text, music.ini, sounds.ini and additional "graphics/ui" folder.
- use it for debug mode: in your home directory, create the ".PGE_Project/a2xtech" folder where you should put a full set of game resources and worlds stuff, this folder will work like a game root in original game. This mode allow you to run an executible file from any folder location of your computer and use the same resources location for all builds (except of these are marked as portable).

## What differences of this thing in comparison to original VB6 build?
- First off, it's written in C++ while original, obviously, written in VB6
- Doesn't have an Editor. Instead, it will have an integration with PGE Editor that will allow to use it with the same functionality as in original editor (the "magic hand" functionality was kept to allow real-time editing of the level while testing, it's need to use IPC communication with PGE Editor to get an ability to use it better).
- Full support of UTF-8 in filename paths and internal text data (original game had the only 8bit ANSI support).
- For graphics and controlling, it uses SDL2 library while original game have used WinAPI calls and GDI library.
- It uses PGE-FL that has a better file formats support
- A support for WLDX world maps are allowing unlimited credits lines and custom music without necessary to use a music.ini for music replacements.
- Built-in support for episide and level wide music.ini and sounds.ini to override default music and sounds assets.
- World maps now supports a custom directory to store any specific resources like custom tiles/scenes/paths/levels and don't spam the episode root folder with wolrd map resources anymore.
- Default config format is INI, old config.dat format is no more supported, mainly because of incompatible key code values in comparison to SDL_Scancode and VirtualKeys enum of WinAPI.
- Game saves using SAVX format instead of classic SAV. However, if you already have an old gamesave, you can resule the game with using of a new engine now: however, next gamesave will be in SAVX format.
- Built-in PNG support for custom and default graphics. Masked GIFs are still supported for backward compatibility, however, without making an unexpected auto-conversion like SMBX-38A does.
- Some LVLX features are working now: vertical section wrap, two-way warps, custom "star needed" message, warp enter event, ability to disable stars printing in HUB episodes for specific door, ability to disable interscene showing when going to another level through a warp.
- Checkpoints now multi-points! You can use them in your levels multiple times without limits!
- It does use of lazy-decompress algorithm to speed-up the loading of a game and reduce the memory usage.
- For music and SFX, the MixerX library is used to give the support of a wide amount of file formats!
- It doesn't embeds any graphics: here is NO any trurely hardcoded graphics, everything now representing as an external graphics!

## How to build it?
To build it, you need to have next things:
- CMake
- Ninja optionally (speeds-up a build process)
- Compatible C/C++ compiler (GCC, Clang, MSVC didn't tested yet)
- Git (required to pull submodules and clone source of dependent libraries to build them in a place)
- Mercurial (required to clone an official SDL2 repository to build it in a place here)
- Optionally: system-wide installed dependencies: SDL2, libFreeImageLite (a modded implementation of the FreeImage), MixerX sound library, AudioCodecs collection of libraries. Having them be installed in a system giving a major build speed up. However, it's possible to build all these dependencies in a place here with a cost of an extra build time being added.

