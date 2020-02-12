TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lSDL2_mixer_ext -lSDL2 -lFreeImageLite

INCLUDEPATH += lib/
INCLUDEPATH += /usr/include/SDL2

SOURCES += \
    lib/AppPath/app_path.cpp \
    src/blocks.cpp \
    src/change_res.cpp \
    src/collision.cpp \
    src/custom.cpp \
    src/effect.cpp \
    src/frm_main.cpp \
    src/game_main.cpp \
    src/main/game_loop.cpp \
    src/main/menu_loop.cpp \
    src/main/setup_physics.cpp \
    src/main/setup_vars.cpp \
    src/gfx.cpp \
    src/globals.cpp \
    src/graphics.cpp \
    src/joystick.cpp \
    src/layers.cpp \
    src/load_gfx.cpp \
    src/main.cpp \
    src/npc.cpp \
    src/player.cpp \
    src/sorting.cpp \
    src/sound.cpp

HEADERS += \
    lib/AppPath/app_path.h \
    src/blocks.h \
    src/change_res.h \
    src/collision.h \
    src/custom.h \
    src/effect.h \
    src/frm_main.h \
    src/game_main.h \
    src/gfx.h \
    src/globals.h \
    src/graphics.h \
    src/joystick.h \
    src/layers.h \
    src/load_gfx.h \
    src/location.h \
    src/npc.h \
    src/player.h \
    src/range_arr.hpp \
    src/sorting.h \
    src/sound.h \
    src/std_picture.h

macx: HEADERS += lib/AppPath/app_path_macosx.h
macx: SOURCES += lib/AppPath/app_path_macosx.m

# PGE File Library
SOURCES += $$files($$PWD/lib/PGE_File_Formats/*.cpp)
SOURCES += $$files($$PWD/lib/PGE_File_Formats/*.c)
HEADERS += $$files($$PWD/lib/PGE_File_Formats/*.h)

# DirManager
SOURCES += $$files($$PWD/lib/DirManager/*.cpp)
HEADERS += $$files($$PWD/lib/DirManager/*.h)

# fmt
SOURCES += $$files($$PWD/lib/fmt/*.cpp)
HEADERS += $$files($$PWD/lib/fmt/*.h)
HEADERS += $$files($$PWD/lib/fmt_*.h)

# Graphics
SOURCES += $$files($$PWD/lib/Graphics/*.cpp)
HEADERS += $$files($$PWD/lib/Graphics/*.h)

# Graphics
SOURCES += $$files($$PWD/lib/FileMapper/*.cpp)
HEADERS += $$files($$PWD/lib/FileMapper/*.h)

# IniProcessor
SOURCES += $$files($$PWD/lib/IniProcessor/*.cpp)
HEADERS += $$files($$PWD/lib/IniProcessor/*.h)

# Utils
SOURCES += $$files($$PWD/lib/Utils/*.cpp)
HEADERS += $$files($$PWD/lib/Utils/*.h)

# Logger
SOURCES += $$files($$PWD/lib/Logger/*.cpp)
HEADERS += $$files($$PWD/lib/Logger/*.h)
