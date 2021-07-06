#include "../globals.h"
#include "../sound.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <Logger/logger.h>
#else
#include <tinyfiledialogs/tinyfiledialogs.h>
#endif

const std::string GetTextInput(const std::string& prompt, const std::string& init = "")
{
    SoundPauseAll();
    const char* get;

#ifndef __EMSCRIPTEN__
    if (init.empty())
        get = tinyfd_inputBox(prompt.c_str(), nullptr, " ");
    else
        get = tinyfd_inputBox(prompt.c_str(), nullptr, init.c_str());
#else
    // FIXME: Implement the Emscripten-side support
    get = nullptr;
    pLogWarning("Calling GetTextInput() with the missing Emscripten implementation");
#endif

    SoundResumeAll();
    if (get == nullptr)
        return "";
    // in case the extra space tripped someone up
    if (get[0] == ' ')
        return get+1;
    return get;
}
