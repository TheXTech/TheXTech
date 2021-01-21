#!/bin/bash

# Remove old version if presented
if [[ -d SDL-default ]]; then
    rm -Rf SDL-default
fi

# Unpack recent SDL2 tarball
if [[ ! -d SDL-default ]]; then
    wget https://github.com/WohlSoft/PGE-Project/raw/master/_Libs/_sources/SDL-default.tar.gz
    tar -xf SDL-default.tar.gz
    rm SDL-default.tar.gz
fi

if [[ ! -d thextech/jni/SDL ]]; then
    mkdir thextech/jni/SDL
fi

# Make a link for SDL2 sources
if [[ ! -e thextech/jni/SDL/src ]]; then
    ln -s ../../../SDL-default/src thextech/jni/SDL/src
fi

# Make a link for SDL2 includes
if [[ ! -e thextech/jni/SDL/include ]]; then
    ln -s ../../../SDL-default/src thextech/jni/SDL/include
fi

# Remove old SDL2 Java files
rm -f thextech/src/main/java/org/libsdl/app/*.java

# Copy SDL2 Java files
cp SDL-default/android-project/app/src/main/java/org/libsdl/app/*.java thextech/src/main/java/org/libsdl/app

# Copy Android NDK makefile
cp SDL-default/Android.mk thextech/jni/SDL

echo "Done!"
