#ifndef CONFIG_H
#define CONFIG_H

#include <unordered_map>
#include <string>

#ifndef FIXED_RES
extern int config_InternalW;
extern int config_InternalH;
#endif

enum ScaleModes
{
    SCALE_DYNAMIC_INTEGER = -2,
    SCALE_DYNAMIC_NEAREST = -1,
    SCALE_DYNAMIC_LINEAR = 0,
    SCALE_FIXED_1X = 1,
    SCALE_FIXED_2X = 2,
};

static const std::unordered_map<int, std::string> ScaleMode_strings = {
	{SCALE_DYNAMIC_INTEGER, "integer"},
    {SCALE_DYNAMIC_NEAREST, "nearest"},
    {SCALE_DYNAMIC_LINEAR, "linear"},
    {SCALE_FIXED_1X, "1x"},
    {SCALE_FIXED_2X, "2x"},
};

extern int config_ScaleMode;

extern bool config_FastMove;

#endif // #ifndef CONFIG_H