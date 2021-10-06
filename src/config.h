#ifndef CONFIG_H
#define CONFIG_H

extern struct Config_t
{
    //! Use SMBX player select screens
    bool    LegacyPlayerSelect = true;
    //! Use strict Drop/Add (lose life on Add after Drop, SwapCharacter)
    bool    StrictDropAdd = false;
} g_config; // main_config.cpp

#endif // CONFIG_H
