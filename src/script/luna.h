#pragma once
#ifndef LUNA_H
#define LUNA_H

#include <string>

extern bool gLunaEnabledGlobally;
extern bool gLunaEnabled;
extern bool gShowDemoCounter;
extern bool gEnableDemoCounter;
extern std::string gDemoCounterTitle;

extern void lunaReset();
extern void lunaLoad();
extern void lunaLoop();
extern void lunaRender();

#endif // LUNA_H
