#ifndef CONFIG_H
#define CONFIG_H

#include <unordered_map>
#include <string>

#ifndef FIXED_RES
extern int config_InternalW;
extern int config_InternalH;
#endif

enum class ScaleMode_t : int
{
    DYNAMIC_INTEGER = -2,
    DYNAMIC_NEAREST = -1,
    DYNAMIC_LINEAR = 0,
    FIXED_1X = 1,
    FIXED_2X = 2,
};

static const std::unordered_map<ScaleMode_t, std::string> ScaleMode_strings = {
	{ScaleMode_t::DYNAMIC_INTEGER, "integer"},
    {ScaleMode_t::DYNAMIC_NEAREST, "nearest"},
    {ScaleMode_t::DYNAMIC_LINEAR, "linear"},
    {ScaleMode_t::FIXED_1X, "1x"},
    {ScaleMode_t::FIXED_2X, "2x"},
};

extern ScaleMode_t config_ScaleMode;

#endif // #ifndef CONFIG_H