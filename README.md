<p align="center">
<a href="https://github.com/Wohlstand/TheXTech/releases"><img src="https://raw.githubusercontent.com/Wohlstand/TheXTech/master/resources/icon/thextech_512.png" alt="TheXTech"></a>
</p>

# TheXTech

SMBX engine, rewritten into C++ from VisualBasic 6.


# Frequently Asked Questions

## What is this?
This is a direct continuation of the SMBX 1.3 engine. Originally it was written in VB6 for Windows, and later, it got ported/rewritten into C++ and became a cross-platform engine. It completely reproduces the old SMBX 1.3 engine (aside from its Editor), includes many of its logical bugs (critical bugs that lead the game to crash or freeze got fixed), and also adds a lot of new updates and features.


## Why did you make it?
I have several purposes for making it:
- It's a very convenient research model I want to use in developent of the Moondust Engine.
- To provide a fully-compatible replica of the old engine for modern platforms, allowing to play old levels and episodes with the same feeling as if they were played on the original VB6-based SMBX game.
- To make it work without the necessity to use Wine on non-Windows platforms and making it available on non-x86/x64 platforms.
- Optimizing it to use fewer hardware resources than the original VB6-based game.


## You have Moondust Engine, why you have spent over a month to craft this thing?
I need it for Moondust Engine development directly, it's much easier to hack and inspect than an old, inconvenient VB6 environment.


## What's the future of Moondust Engine now that TheXTech exists?
I'll continue developing the Moondust Engine as I have yet to reach the second goal of the project.
Since it's foundation, the Moondust Project had two goals: 1) save SMBX; 2) give a flexible toolkit for new platform games. The opening of SMBX's source-code and introducing TheXTech has solved the first goal: SMBX has been saved and now it's free/opensource cross-platform software. Moondust Engine will be used for the second goal - giving a toolkit for new games. Unlike TheXTech, Moondust Engine gives a high degree of flexibility that allows anyone to build something new from scratch without inheriting an old game base. However, TheXTech is needed for Moondust Engine as a working research model to develop the new engine. It will be similar to GZDoom and Chocolate Doom ports of the Doom game: GZDoom is a powerful and functional engine, the best choice for modders; Chocolate Doom is an accurate port of the original game to a modern platform with the purpose to represent the original game including even bugs. The Moondust Engine intends to be like GZDoom while TheXTech is an analog of Chocolate Doom to represent an original game on modern platforms.


## Can levels and episodes with LunaDLL Autocode work on this?
Yes, can. Since the TheXTech version 1.3.6, there is a built-in implementation of the LunaDLL Autocode language, called LunaScript. This system allows the running of LunaDLL-episodes on any hardware including non-x86 processor architectures.


## Can LunaLua work on this?
No, LunaLua won't work: this project is binary-incompatible with LunaLua. This also means that SMBX2 content is incompatible. The planned lua-based scripting system won't guarantee compatibility. Therefore, after the possible appearance of the Lua scripts support, it will be reasonable to port, or create new from the ground up and target to TheXTech exclusively.


## Why is the code here so bad?
The original author wrote most of the code in the "*src*" folder in VB6. I did a whole conversion of the code with an effort to accurate reproduction. So, a lot of the code is identical to what was written in VB6 originally. The VB6 platform had a lot of challenges and limitations such as:
- All variables are global and accessible from every module and form by default without any includes or imports. The reason why "globals.h" exists: it has a full list of globally available variables.
- Limited and inconvenient support for classes, therefore the code tends to abuse a ton of global variables and arrays (also an initial lack of experience of the original author was an another factor that lead to this mess).
- All functions in all modules are global and can be called from each module directly. Except calls marked as "private". Therefore I had an additional work to provide inclusions into files where these calls are requested.
- Why so much `if-elseif-elseif-elseif-elseif-....?` Yes, here probably will be correct to use `switch()` (in VB6 the `Select Case` analogue) operator. Another factor that shows the original author had a low amount of experience when he coded this project.
- Why the `if() { if {} if { .... } }` lasagna? Two reasons: 1) inexperience of original author, 2) workaround to not check all conditions of expression which may cause a crash. In C++ with multiple conditions splitted by `&&` operator, never executing when one of them gets a false result. In VB6, ALL conditions in expression getting be always executed. This difference caused the next situation: in VB6, an expression `if A < 5 And Array(A) = 1 Then` will cause a crash when A is more than 5. In C++ the same `if(A < 5 && Array[A] == 1)` expression will never crash because a second check gets be never executed if a first check gave a false result.
- Why so long expressions like `if(id == 1 || id == 3 || id == 4 || ... id == N)`? Rather making a ton of conditions like this, it's would be better to use classes with a polymorphism and separate the logic of every object between different classes. Also should be solvable with having to use of function pointers (which aren't possible in VB6 without workaronds, but possible in C++). But, again, the original author's inexperience combined with a bunch of VB6 limits caused these constructions.


## How to use this?
Here are many ways to play games with it:
- There are some ready-to-use packages, just download them, unpack and run them as you did it with SMBX, absolutely same.
- [macOS users, skip this]: Mix the game with existing assets directory: put an executable file into the game root folder with an "thextech.ini" that contains next text:
```ini
[Main]
force-portable = true
```
, music.ini, sounds.ini and additional "graphics/ui" folder. An important note: all default graphics must be converted into PNG, use GIFs2PNG tool from Moondust Project over your "graphics" folder with a "-d" switch. Don't use "-r" switch to keep original GIFs together with new-made PNGs if you plan to continue the use of original VB6-written SMBX.
- Use it for debug mode: in your home directory, create the ".PGE_Project/thextech" folder (on macOS the "`~/TheXTech Games/Debug Assets/`") where you should put a full set of game resources and worlds stuff, this folder will work like a game root in the original game. This mode allows you to run an executable file from any folder location of your computer and use the same location of resources for all builds (except these are marked as portable by an INI file).


## How to add custom episodes for the macOS version?
If you have a bundled build of TheXTech, all default resources are inside your .app: "Content/Resources/assets/". You can modify the content, but it's not recommended! Instead, after the first run of a game, in your home directory will appear the next directory:
```
   ~/TheXTech Games/<game name>/
```
In this directory, you will find an empty "battle" and "worlds" folder to put your custom stuff. At the "settings" sub-directory the game settings and game saves will be stored.
At the "`~/Library/Application Support/PGE Project/thextech/logs/`" path logs will be stored.
If you want to replace default assets with your own, you can modify the content of the app bundle or compile a new build with giving of the necessary CMake arguments which needed to pack your custom assets root and icon into the new bundle or make the assets-less build (if you give no arguments, the assets-less build will result). Therefore, you need to put the full content of the game root into the "`~/TheXTech Games/Debug Assets/`" folder, include default assets (graphics, music, sounds, intro and outro levels, default battle and worlds folders).


## What is different with this thing in comparison to the original VB6 build?
- First off, it's written in C++ while original (as we already know) is written in VB6.
- Doesn't have an Editor. Instead, in has a deep integration with Moondust Editor that allows to use it with the same functionality as in original editor (the "magic hand" functionality was kept to allow real-time editing of the level while testing, it's needed to use IPC communication with Moondust Editor to get the ability to use it better).
- Full support of UTF-8 in filename paths and internal text data (original game had the only 8bit ANSI support).
- For graphics and controlling, it uses an SDL2 library while original game have used WinAPI calls and GDI library.
- It uses PGE-FL library that has better file formats support.
- A support for WLDX world maps are allowing unlimited credits lines and custom music without it being necessary to use a music.ini for music replacements.
- Some LVLX exclusive features now working: vertical section wrap, two-way warps, custom "star needed" message, warp enter event, ability to disable stars printing in HUB episodes for specific doors, ability to disable interscene showing when going to another level through a warp.
- Built-in support for episode and level wide music.ini and sounds.ini to override default music and sounds assets.
- World maps now supports a custom directory to store any specific resources like custom tiles/scenes/paths/levels and not spam the episode root folder with world map resources anymore.
- Default config format is INI, old config.dat format is no longer supported, mainly because of incompatible key code values (SDL_Scancode versus VirtualKeys enum of Windows API).
- Game saves now using the SAVX format instead of a classic SAV. However, if you already have an old gamesave, you still can resume your game by using a new engine now (next gamesave attempt will result a SAVX file, old gamesave in SAV format will be kept untouched).
- Built-in PNG support for custom and default graphics. Masked GIFs are still supported for backward compatibility, however, without making an unexpected auto-conversion like SMBX-38A does.
- Checkpoints now have multi-points! You can use them in your levels multiple times without limits!
- It does use of lazy-decompress algorithm to speed-up the loading of a game and reduce the memory usage.
- For music and SFX, the MixerX library is used to give a support for a wide amount of sound and music formats!
- It doesn't embeds any graphics: there are NO trurely hardcoded graphics, everything is now represented by external graphics!
- Some internal limits have been expanded.
- Built-in GIF recorder by F11 key (F10 on macOS, F11 is reserved by system UI for a "show desktop" action)
- It starts faster: the loading of the game is almost instant (depend on a computer and it's HDD/SSD performance).
- It uses less RAM (80...150 MB instead of 600...800 MB like usually), and it's free from memory leaks given by the MCI interface used by VB6 SMBX originally.
- it doesn't overload CPU (the reason was a bad way to process infinite loops, I did the fix of VB6 build too at my "smbx-experiments" branch)
- it able to work on "toaster" (a weak computer) while VB6-SMBX won't work.
- it's fully cross-platform and doesn't depend on Windows, and it no longer depends on x86 processor: it can work on ARM and MIPS processors too (VB6-SMBX won't work on ARM at all, with x86 emulator it will 20x times slower than usual).


## How to build it?
You can read a guide how to build this project from source here: https://github.com/Wohlstand/TheXTech/wiki/Building-the-game

To build it, you need to have the following things:
- CMake
- Ninja optionally (to speed-up the build process)
- Compatible C/C++ compiler (GCC, Clang, MinGW, MSVC 2017 and 2019, possibly will build also on 2015, but wasn't tested)
- Git (required to pull submodules and clone source of dependent libraries to build them in place)
- Mercurial (required to clone an official SDL2 repository to build it in place here)
- Optionally: system-wide installed dependencies: SDL2, libFreeImageLite (a modded implementation of the FreeImage), MixerX sound library, AudioCodecs collection of libraries. Having them be installed in a system gives a major build speed up. However, it's possible to build all these dependencies in place here with a cost of extra build time being added.

## Localization
Some parts of TheXTech (such as Android launcher) can be localized into many languages, you may help to translate them using WebLate platform: https://hosted.weblate.org/projects/thextech/


## Used software
* GCC, MinGW и Clang - main compilers used during the development of the game. The MSVC is used sometimes which is used for Windows builds for ARM64 processors.
* [Qt Creator](https://www.qt.io/product/development-tools) - IDE from the Qt toolkit, mainly used during the development.
* [JetBrans CLion](https://www.jetbrains.com/ru-ru/clion/) - Intelliji Idea based C/C++ IDE, free for the [Open Source development](https://www.jetbrains.com/ru-ru/community/opensource/).
* [PVS-Studio](https://pvs-studio.com/ru/pvs-studio/?utm_source=github&utm_medium=organic&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code. Is used for periodical checking of the code for possible bugs. It's free for the [Open Source development](https://pvs-studio.com/en/order/open-source-license/).
