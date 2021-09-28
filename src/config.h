#ifndef CONFIG_H
#define CONFIG_H

extern struct Config_t
{
    //! Internal gameplay resolution
    int     InternalW = 800;
    int     InternalH = 600;
    //! Display new editor in menu
    bool    UseNewEditor = false;
    //! SMB3 world map movement
    bool    FastMove = false;
    //! Dragon coin
    bool    ShowDragonCoins = false;
    //! Show episode title
    int     ShowEpisodeTitle = 0;
    int     ShowEpisodeTitle_SmallScreen = 0;
} g_config; // main_config.cpp

#endif // #ifndef CONFIG_H