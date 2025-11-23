We're glad you're interested in the project! Well-implemented PRs that improve TheXTech's compatibility with SMBX64 or add/improve our ports are always welcome. We are also very interested in PRs that add quality-of-life features to base SMBX64 content.

# Project Roadmap

A broad overview of the goals for the project in the next few versions:

## 1.3.7 (released)

* [Multiple resolution system to play game at non-800x600 resolutions](https://github.com/TheXTech/TheXTech/pull/328)
* [In-game configuration system](https://github.com/TheXTech/TheXTech/pull/734)
* Shared-screen and split-screen >2P multiplayer

## 1.3.7.x

* Compressed archive support for episodes and asset packs (`.xte` / `.xta`)
* Updated file formats system
* NetPlay (aka Online Play) system

## 1.(3.)8

* See [here](https://github.com/TheXTech/TheXTech/issues/952).

## ???

* [Luau scripting system](https://github.com/TheXTech/TheXTech/issues/472) for **Classic Events** and **NPC's**
* GLSL ES shader API
* TAS System
* Rewinding

Please see the [Issues](https://github.com/TheXTech/TheXTech/issues) and [milestones](https://github.com/TheXTech/TheXTech/milestones) that developers have endorsed for a fine-grained list of the development tasks and targets we are currently considering.

## An important note on the scope of TheXTech

TheXTech is based on the [SMBX64](https://wohlsoft.ru/pgewiki/SMBX64) standard, and it also aims for limited compatibility with the other SMBX branches, as well as Low-end device support, NetPlay support, and TAS/Rewind system support. Therefore, any topics (including but not limited to issues, PR's, and discussions) whose purpose is to make the base TheXTech engine with one of the following situations, will be rejected as **out-of-scope**, even with Lua Script System support:

1. Imitating features of non-SMBX games
2. Featuring major gameplay changes
3. Affecting low-end device support
  - We monitor code size, static and dynamic memory usage, and CPU performance
4. Breaks legacy SMBX64 logic
5. Affects past works
6. Spamming new content ideas
7. Causing the engine slowdown
8. Affecting TAS (Tool-assisted Speedrun) Mechanic, Netplay, or rewinding
9. Mentioning unofficial engine features
10. Create lots of forks
  - For anything platform games to be made from scratch, You should use **Moondust Engine** instead, but currently is in progress.

Related comments will also be marked as "off-topic". Related conversations of partial issues and PR's will be locked as "off-topic".

Therefore, one of the project, Nostalgic Paradise, would be out-scoped from this engine. Please don't make feedback here, use another repo instead: https://github.com/Nostalgic-Paradise-Dev-Team/NsPr-Discussion

## A note on "bugfixes"

TheXTech is a faithful reproduction of the SMBX 1.3 engine, so a number of things that may appear to be bugs to a user or contributor may actually be TheXTech reproducing SMBX 1.3 logic intended. Please see [Types of Bugs](https://github.com/TheXTech/TheXTech/wiki/Types-of-bugs) for a categorization of types of bugs.

PRs fixing native bugs and critical vanilla bugs are always welcome (but you may be asked to justify that the bug in question is indeed critical).

Fixes for other vanilla bugs are not a good first contribution to the engine, due to the considerations we need to employ when faced with these. You are welcome to file an issue for any vanilla peculiarities you discover, but we are unlikely to fix them.

# TheXTech content standard
We are in the process of developing a content standard for TheXTech, including new engine features such as multistars, more flexible warps, and world map sections. We are also developing a scripting layer including TheXTech-specific Lua and GLSL ES APIs.

We are being very careful in how we create the TheXTech standard, and we are generally hesitant to accept PRs that add new content to the standard. We intend to maintain indefinite compatibility with all content created for TheXTech 1.3.7+, and this makes it difficult to remove or refactor features once added.

# Ports and limitations

We maintain ports for a wide variety of systems, and you should take care when developing new features to ensure that the appropriate ports of the game will continue to run with our minimum system requirements (16 MB of RAM and 200 MHz CPU).

If you are interested in contributing new ports to any such systems, please see [[Porting]].
