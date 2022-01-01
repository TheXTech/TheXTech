#ifndef CONFIG_H
#define CONFIG_H

extern struct Config_t
{
    //! Use strict Drop/Add (lose life on Add after Drop, SwapCharacter)
    bool    StrictDropAdd = false;
    //! Record gameplay data
    bool    RecordGameplayData = false;
} g_config; // main_config.cpp

#endif // CONFIG_H
