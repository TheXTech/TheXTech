#ifndef CONFIG_H
#define CONFIG_H

extern struct Config_t
{
    //! Internal gameplay resolution
    int     InternalW = 800;
    int     InternalH = 600;
    //! Display new editor in menu
    bool    UseNewEditor = false;
    //! Display new editor in menu
    bool    FastMove = false;
} g_config; // main_config.cpp

#endif // #ifndef CONFIG_H