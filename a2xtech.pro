TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lSDL2_mixer_ext -lSDL2

SOURCES += \
    src/blocks.cpp \
    src/change_res.cpp \
    src/collision.cpp \
    src/custom.cpp \
    src/effect.cpp \
    src/frm_main.cpp \
    src/game_main.cpp \
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
    src/sorting.h \
    src/sound.h
