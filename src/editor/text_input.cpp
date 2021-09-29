#include "../globals.h"
#include "../sound.h"

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) && !defined(VITA)
#include <tinyfiledialogs/tinyfiledialogs.h>
#else
#include <Logger/logger.h>
#endif

const std::string GetTextInput(const std::string& prompt, const std::string& init = "")
{
    SoundPauseAll();
    const char* get;

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) && !defined(VITA)
    if (init.empty())
        get = tinyfd_inputBox(prompt.c_str(), nullptr, " ");
    else
        get = tinyfd_inputBox(prompt.c_str(), nullptr, init.c_str());
#else
    // FIXME: Implement the Android and Emscripten-sides support
    get = nullptr;
    pLogWarning("Calling GetTextInput() with the missing Android/Emscripten implementation");
#endif

    SoundResumeAll();
    if (get == nullptr)
        return "";
    // in case the extra space tripped someone up
    if (get[0] == ' ')
        return get+1;
    return get;
}
