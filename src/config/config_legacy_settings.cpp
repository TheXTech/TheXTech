/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config/config_main.h"
#include "globals.h"

#include <IniProcessor/ini_processing.h>

template<>
void Config_t::LoadLegacySettings(IniProcessing* ini, ConfigSetLevel level)
{
#ifndef THEXTECH_NO_SDL_BUILD
    const IniProcessing::StrEnumMap sampleFormats =
    {
        {"s8", AUDIO_S8},
        {"pcm_s8", AUDIO_S8},
        {"u8", AUDIO_U8},
        {"pcm_u8", AUDIO_U8},
        {"s16", AUDIO_S16SYS},
        {"pcm_s16", AUDIO_S16SYS},
        {"s16le", AUDIO_S16LSB},
        {"pcm_s16le", AUDIO_S16LSB},
        {"s16be", AUDIO_S16MSB},
        {"pcm_s16be", AUDIO_S16MSB},
        {"u16", AUDIO_U16SYS},
        {"pcm_u16", AUDIO_U16SYS},
        {"u16le", AUDIO_U16LSB},
        {"pcm_u16le", AUDIO_U16LSB},
        {"u16be", AUDIO_U16MSB},
        {"pcm_u16be", AUDIO_U16MSB},
        {"s32", AUDIO_S32SYS},
        {"pcm_s32", AUDIO_S32SYS},
        {"s32le", AUDIO_S32LSB},
        {"pcm_s32le", AUDIO_S32LSB},
        {"s32be", AUDIO_S32MSB},
        {"pcm_s32be", AUDIO_S32MSB},
        {"float32", AUDIO_F32SYS},
        {"pcm_f32", AUDIO_F32SYS},
        {"float32le", AUDIO_F32LSB},
        {"pcm_f32le", AUDIO_F32LSB},
        {"float32be", AUDIO_F32MSB},
        {"pcm_f32be", AUDIO_F32MSB}
    };

    ini->beginGroup("sound");
    if(ini->hasKey("disable-sound"))
    {
        bool audio_disabled;
        ini->read("disable-sound", audio_disabled, false);
        audio_enable = !audio_disabled;
        audio_enable.m_set = level;
    }
    if(ini->hasKey("sample-rate"))
    {
        int sample_rate;
        ini->read("sample-rate", sample_rate, g_audioDefaults.sampleRate);
        audio_sample_rate = sample_rate;
        audio_sample_rate.m_set = level;
    }
    if(ini->hasKey("channels"))
    {
        int channels;
        ini->read("channels", channels, g_audioDefaults.channels);
        audio_channels = channels;
        audio_channels.m_set = level;
    }
    if(ini->hasKey("buffer-size"))
    {
        int buffer_size;
        ini->read("buffer-size", buffer_size, g_audioDefaults.bufferSize);
        audio_buffer_size = buffer_size;
        audio_buffer_size.m_set = level;
    }
    if(ini->hasKey("format"))
    {
        uint16_t format;
        ini->readEnum("format", format, g_audioDefaults.format, sampleFormats);
        audio_format = format;
        audio_format.m_set = level;
    }
    ini->endGroup();
#endif

#ifndef NO_WINDOW_FOCUS_TRACKING
    if(ini->hasKey("background-controller-input"))
    {
        bool val;
        ini->read("background-controller-input", val, true);
        background_work = val;
        background_work.m_set = level;
    }
#endif
    ini->endGroup();

    ini->beginGroup("gameplay");
    if(ini->hasKey("compatibility-mode"))
    {
        const IniProcessing::StrEnumMap compatMode =
        {
            {"native", 0},
            {"smbx2", 1},
            {"smbx13", 2}
        };

        int val = -1;

        ini->readEnum("compatibility-mode", val, -1, compatMode);
        if(val != -1)
        {
            compatibility_mode = val;
            compatibility_mode.m_set = level;
        }
    }

    // old way of setting death counter
}
