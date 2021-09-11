#!/bin/bash

gcc exe2ui.c -static -static-libgcc -O3 -no-pie -o exe2ui-linux-x86_64
i686-w64-mingw32-gcc exe2ui.c -static -static-libgcc -O3 -no-pie -o exe2ui.exe
