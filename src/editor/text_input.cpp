#include <tinyfiledialogs/tinyfiledialogs.h>
#include "../globals.h"
#include "../sound.h"

const std::string GetTextInput(const std::string& prompt, const std::string& init = "")
{
    SoundPauseAll();
    const char* get;
    if (init.empty())
        get = tinyfd_inputBox(prompt.c_str(), nullptr, " ");
    else
        get = tinyfd_inputBox(prompt.c_str(), nullptr, init.c_str());
    SoundResumeAll();
    if (get == nullptr)
        return "";
    // in case the extra space tripped someone up
    if (get[0] == ' ')
        return get+1;
    return get;
}
