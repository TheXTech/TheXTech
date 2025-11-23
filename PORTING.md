TheXTech is a cross-platform engine and is built to be easy to port to new systems. It has very limited dependencies and can be built for most POSIX-like targets.

# System Requirements

The target must have at least **16 MB RAM** (working on Wii with ~48 MB available), a **200 MHz CPU** (working on old Nintendo 3DS with 268 MHz ARM11), and a **C++11-compliant compiler**.

On some of our existing ports, storage or filesystem bandwidth is a pressing consideration.

Full compliance with the SMBX64 standard requires support for the AND and OR bitwise / logical blend modes, but the game includes tools used by our SDL2 ports to approximate these with alpha blending. Some features require the target to support compiling GLSL ES 1.00 and 3.00 shaders.

# Port types and guidelines

Most versions of the game use SDL2 + OpenGL for rendering and SDL Mixer X for software mixing. Your port may make use of these APIs, but is not required to.

## SDL2

If your target supports the SDL2 joystick, window, message box, render, audio, and threading APIs, porting to your target may be as simple as pointing CMake to the appropriate toolchain file. You may need to edit the `CMakeLists.txt` build instructions to enable / disable features for your target as appropriate. Use the Switch, Wii U, or Vita port for reference.

## Custom + SDL2 audio

If your target supports the SDL2 audio and threading APIs but is missing other APIs, you may create custom bindings for many of the subsystems under `src/core/`. Use the 3DS and Wii port for reference.

## Custom

We maintain a "null" port designed to compile on virtually any POSIX-compliant system. This port does **not** depend on SDL2 but uses a shim layer to delegate most SDL2 calls to their POSIX equivalents. This port may be used in a command-line environment with a gameplay recording file to verify gameplay logic before you invest the time in creating a full port for your target.

The null port may be built by passing `-DTHEXTECH_CLI_BUILD=On -DTHEXTECH_NO_SDL_BUILD=On` to CMake in addition to specifying your toolchain.

You may initially extend the null port with a minimal renderer for your target system that (for instance) draws rectangles for all drawn textures. This facilitates early testing and development.

We previously maintained a custom audio library that used the 3DS's DSP for hardware-accelerated mixing. This source code may be checked to understand how to implement a MixerX-like audio library. It may also be possible to replace the audio at a different level of abstraction (for instance, replacing `sound.cpp` entirely).
