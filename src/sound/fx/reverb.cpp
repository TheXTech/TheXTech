/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <vector>
#include <deque>
#include <cmath>
#include <tgmath.h>
#include "reverb.h"


#ifndef INT8_MIN
#define INT8_MIN    (-0x7f - 1)
#endif
#ifndef INT8_MAX
#define INT8_MAX    0x7f
#endif

#ifndef UINT8_MAX
#define UINT8_MAX   0xff
#endif

#ifndef INT16_MIN
#define INT16_MIN   (-0x7fff - 1)
#endif
#ifndef INT16_MAX
#define INT16_MAX   0x7fff
#endif

#ifndef UINT16_MAX
#define UINT16_MAX  0xffff
#endif

#ifndef INT32_MIN
#define INT32_MIN   (-0x7fffffff - 1)
#endif

#ifndef INT32_MAX
#define INT32_MAX   0x7fffffff
#endif

#define MAX_CHANNELS    10


// Float32-LE
static float getFloatLSBSample(uint8_t* raw, int c)
{
    uint32_t r;
    float f;
    void* t;
    raw += (c * sizeof(float));
    r = (((uint32_t)raw[0] <<  0) & 0x000000FF) |
        (((uint32_t)raw[1] <<  8) & 0x0000FF00) |
        (((uint32_t)raw[2] << 16) & 0x00FF0000) |
        (((uint32_t)raw[3] << 24) & 0xFF000000);
    t = &r;
    f = *(float*)t;
    return f;
}
static void setFloatLSBSample(uint8_t** raw, float ov)
{
    void* t = &ov;
    uint32_t r = *(uint32_t*)t;
    *(*raw)++ = (uint8_t)((r >> 0) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 8) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 16) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 24) & 0xFF);
}

// Float32-BE
static float getFloatMSBSample(uint8_t* raw, int c)
{
    uint32_t r;
    float f;
    void* t;
    raw += (c * sizeof(float));
    r = (((uint32_t)raw[3] <<  0) & 0x000000FF) |
        (((uint32_t)raw[2] <<  8) & 0x0000FF00) |
        (((uint32_t)raw[1] << 16) & 0x00FF0000) |
        (((uint32_t)raw[0] << 24) & 0xFF000000);
    t = &r;
    f = *(float*)t;
    return f;
}
static void setFloatMSBSample(uint8_t** raw, float ov)
{
    void* t = &ov;
    uint32_t r = *(uint32_t*)t;
    *(*raw)++ = (uint8_t)((r >> 24) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 16) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 8) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 0) & 0xFF);
}

// int32_t-LE
static float getInt32LSB(uint8_t* raw, int c)
{
    uint32_t r;
    int32_t f;
    raw += (c * sizeof(int32_t));
    r = (((uint32_t)raw[0] <<  0) & 0x000000FF) |
        (((uint32_t)raw[1] <<  8) & 0x0000FF00) |
        (((uint32_t)raw[2] << 16) & 0x00FF0000) |
        (((uint32_t)raw[3] << 24) & 0xFF000000);
    f = *(int32_t*)(&r);
    return (float)((double)f / INT32_MAX);
}
static void setInt32LSB(uint8_t** raw, float ov)
{
    int32_t f = ((int32_t)(double(ov) * INT32_MAX));
    uint32_t r = *(uint32_t*)(&f);
    *(*raw)++ = (uint8_t)((r >> 0) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 8) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 16) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 24) & 0xFF);
}

// int32_t-BE
static float getInt32MSB(uint8_t* raw, int c)
{
    uint32_t r;
    int32_t f;
    raw += (c * sizeof(int32_t));
    r = (((uint32_t)raw[3] <<  0) & 0x000000FF) |
        (((uint32_t)raw[2] <<  8) & 0x0000FF00) |
        (((uint32_t)raw[1] << 16) & 0x00FF0000) |
        (((uint32_t)raw[0] << 24) & 0xFF000000);
    f = *(int32_t*)(&r);
    return (float)((double)f / INT32_MAX);
}
static void setInt32MSB(uint8_t** raw, float ov)
{
    int32_t f = (int32_t(double(ov) * INT32_MAX));
    uint32_t r = *(uint32_t*)(&f);
    *(*raw)++ = (uint8_t)((r >> 24) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 16) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 8) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 0) & 0xFF);
}

// int16_t-LE
static float getInt16LSB(uint8_t* raw, int c)
{
    uint16_t r;
    int16_t f;
    raw += (c * sizeof(int16_t));
    r = (((uint16_t)raw[0] <<  0) & 0x00FF) |
        (((uint16_t)raw[1] <<  8) & 0xFF00);
    f = *(int16_t*)(&r);
    return (float)f / INT16_MAX;
}
static void setInt16LSB(uint8_t** raw, float ov)
{
    int16_t f = int16_t(ov * INT16_MAX);
    uint16_t r = *(uint16_t*)(&f);
    *(*raw)++ = (uint8_t)((r >> 0) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 8) & 0xFF);
}

// int16_t-BE
static float getInt16MSB(uint8_t* raw, int c)
{
    uint16_t r;
    int16_t f;
    raw += (c * sizeof(int16_t));
    r = (((uint16_t)raw[1] <<  0) & 0x00FF) |
        (((uint16_t)raw[0] <<  8) & 0xFF00);
    f = *(int16_t*)(&r);
    return (float)f / INT16_MIN;
}
static void setInt16MSB(uint8_t** raw, float ov)
{
    int16_t f = int16_t(ov * INT16_MAX);
    uint16_t r = *(uint16_t*)(&f);
    *(*raw)++ = (uint8_t)((r >> 8) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 0) & 0xFF);
}

// uint16_t-LE
static float getuint16_tLSB(uint8_t* raw, int c)
{
    uint16_t r;
    float f;
    raw += (c * sizeof(uint16_t));
    r = (((uint16_t)raw[0] <<  0) & 0x00FF) |
        (((uint16_t)raw[1] <<  8) & 0xFF00);
    f = ((float)r + INT16_MIN) / INT16_MAX;
    return f;
}
static void setuint16_tLSB(uint8_t** raw, float ov)
{
    int16_t f = int16_t((ov * INT16_MAX) - INT16_MIN);
    uint16_t r = *(uint16_t*)(&f);
    *(*raw)++ = (uint8_t)((r >> 0) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 8) & 0xFF);
}

// uint16_t-BE
static float getuint16_tMSB(uint8_t* raw, int c)
{
    uint16_t r;
    float f;
    raw += (c * sizeof(uint16_t));
    r = (((uint16_t)raw[1] <<  0) & 0x00FF) |
        (((uint16_t)raw[0] <<  8) & 0xFF00);
    f = ((float)r + INT16_MIN) / INT16_MAX;
    return f;
}
static void setuint16_tMSB(uint8_t** raw, float ov)
{
    int16_t f = int16_t((ov * INT16_MAX) - INT16_MIN);
    uint16_t r = *(uint16_t*)(&f);
    *(*raw)++ = (uint8_t)((r >> 8) & 0xFF);
    *(*raw)++ = (uint8_t)((r >> 0) & 0xFF);
}

// int8_t
static float getInt8(uint8_t* raw, int c)
{
    float f;
    raw += (c * sizeof(int8_t));
    f = (float)(int8_t)(*raw) / INT8_MAX;
    return f;
}
static void setInt8(uint8_t** raw, float ov)
{
    *(*raw)++ = (uint8_t)(ov * INT8_MAX);
}

// uint8_t
static float getuint8_t(uint8_t* raw, int c)
{
    float f;
    raw += (c * sizeof(int8_t));
    f = (float)((int) * raw + INT8_MIN) / INT8_MAX;
    return f;
}
static void setuint8_t(uint8_t** raw, float ov)
{
    *(*raw)++ = (uint8_t)((ov * INT8_MAX) - INT8_MIN);
}


struct Reverb /* This reverb implementation is based on Freeverb impl. in Sox */
{
    float feedback = 0.f, hf_damping = 0.f, gain = 0.f;

    struct FilterArray
    {
        struct Filter
        {
            std::vector<float> Ptr;
            size_t pos;
            float Store;

            void Create(size_t size)
            {
                Ptr.resize(size);
                pos = 0;
                Store = 0.f;
            }

            float Update(float a, float b)
            {
                Ptr[pos] = a;

                if(!pos)
                    pos = Ptr.size() - 1;
                else
                    --pos;

                return b;
            }

            float ProcessComb(float input, const float feedback, const float hf_damping)
            {
                Store = Ptr[pos] + (Store - Ptr[pos]) * hf_damping;
                return Update(input + feedback * Store, Ptr[pos]);
            }

            float ProcessAllPass(float input)
            {
                return Update(input + Ptr[pos] * .5f, Ptr[pos] - input);
            }

        } comb[8], allpass[4];

        void Create(double rate, double scale, double offset)
        {
            /* Filter delay lengths in samples (44100Hz sample-rate) */
            static const int comb_lengths[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
            static const int allpass_lengths[4] = {225, 341, 441, 556};
            double r = rate * (1 / 44100.0); // Compensate for actual sample-rate
            const int stereo_adjust = 12;

            for(size_t i = 0; i < 8; ++i, offset = -offset)
                comb[i].Create(scale * r * (comb_lengths[i] + stereo_adjust * offset) + 0.5);

            for(size_t i = 0; i < 4; ++i, offset = -offset)
                allpass[i].Create(r * (allpass_lengths[i] + stereo_adjust * offset) + 0.5);
        }

        void Process(size_t length,
                     const std::deque<float>& input, std::vector<float>& output,
                     const float feedback, const float hf_damping, const float gain)
        {
            for(size_t a = 0; a < length; ++a)
            {
                float out = 0, in = input[a];

                for(size_t i = 8; i-- > 0;)
                    out += comb[i].ProcessComb(in, feedback, hf_damping);

                for(size_t i = 4; i-- > 0;)
                    out += allpass[i].ProcessAllPass(out);

                output[a] = out * gain;
            }
        }
    } chan[2];

    std::vector<float> out[2];
    std::deque<float> input_fifo;

    void Create(double sample_rate_Hz,
                double wet_gain_dB,
                double room_scale, double reverberance, double fhf_damping, /* 0..1 */
                double pre_delay_s, double stereo_depth,
                size_t buffer_size)
    {
        size_t delay = static_cast<size_t>(pre_delay_s  * sample_rate_Hz + .5);
        double scale = room_scale * .9 + .1;
        double depth = stereo_depth;
        double a =  -1 /  std::log(1 - /**/.3 /**/);          // Set minimum feedback
        double b = 100 / (std::log(1 - /**/.98/**/) * a + 1); // Set maximum feedback

        feedback = static_cast<float>(1 - std::exp((reverberance * 100.0 - b) / (a * b)));
        hf_damping = static_cast<float>(fhf_damping * .3 + .2);
        gain = static_cast<float>(std::exp(wet_gain_dB * (std::log(10.0) * 0.05)) * .015);
        input_fifo.insert(input_fifo.end(), delay, 0.f);

        for(size_t i = 0; i <= std::ceil(depth); ++i)
        {
            chan[i].Create(sample_rate_Hz, scale, i * depth);
            out[i].resize(buffer_size);
        }
    }

    void Process(size_t length)
    {
        for(int i = 0; i < 2; ++i)
        {
            if(!out[i].empty())
                chan[i].Process(length, input_fifo, out[i], feedback, hf_damping, gain);
        }

        input_fifo.erase(input_fifo.begin(),
                         input_fifo.begin() +
                         static_cast<std::deque<float>::iterator::difference_type>(length));
    }
};

struct FxReverb
{
    bool        wetonly = false;
    unsigned    amplitude_display_counter = 0;
    float       prev_avg_flt[MAX_CHANNELS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    int         channels = 0;
    int         sampleRate = 0;
    uint16_t    format = AUDIO_F32LSB;
    bool        isValid = false;
    int         sample_size = 2;

    Reverb      chan[MAX_CHANNELS];
    std::vector<std::vector<float>> dry;

    float (*readSample)(uint8_t* raw, int channel);
    void (*writeSample)(uint8_t** raw, float sample);

    int init(int i_rate, uint16_t i_format, int i_channels)
    {
        isValid = false;

        if(channels > MAX_CHANNELS)
            return -1;

        format = i_format;
        sampleRate = i_rate;
        channels = i_channels;

        switch(format)
        {
        case AUDIO_U8:
            readSample = getuint8_t;
            writeSample = setuint8_t;
            sample_size = sizeof(uint8_t);
            break;

        case AUDIO_S8:
            readSample = getInt8;
            writeSample = setInt8;
            sample_size = sizeof(int8_t);
            break;

        case AUDIO_S16LSB:
            readSample = getInt16LSB;
            writeSample = setInt16LSB;
            sample_size = sizeof(int16_t);
            break;

        case AUDIO_S16MSB:
            readSample = getInt16MSB;
            writeSample = setInt16MSB;
            sample_size = sizeof(int16_t);
            break;

        case AUDIO_U16LSB:
            readSample = getuint16_tLSB;
            writeSample = setuint16_tLSB;
            sample_size = sizeof(uint16_t);
            break;

        case AUDIO_U16MSB:
            readSample = getuint16_tMSB;
            writeSample = setuint16_tMSB;
            sample_size = sizeof(uint16_t);
            break;

        case AUDIO_S32LSB:
            readSample = getInt32LSB;
            writeSample = setInt32LSB;
            sample_size = sizeof(int32_t);
            break;

        case AUDIO_S32MSB:
            readSample = getInt32MSB;
            writeSample = setInt32MSB;
            sample_size = sizeof(int32_t);
            break;

        case AUDIO_F32LSB:
            readSample = getFloatLSBSample;
            writeSample = setFloatLSBSample;
            sample_size = sizeof(float);
            break;

        case AUDIO_F32MSB:
            readSample = getFloatMSBSample;
            writeSample = setFloatMSBSample;
            sample_size = sizeof(float);
            break;

        default:
            return -1; /* Unsupported format */
        }

        ReverbSetup set;
        setSettings(set);

        dry.resize(channels);

        isValid = true;
        return 0;
    }

    void setSettings(const ReverbSetup& setup)
    {
        for(int i = 0; i < channels; ++i)
        {
            chan[i].Create(sampleRate,
                           setup.gain,  // wet_gain_dB  (-10..10)
                           setup.roomScale,   // room_scale   (0..1)
                           setup.balance,   // reverberance (0..1)
                           setup.hfDamping,   // hf_damping   (0..1)
                           setup.preDelayS, // pre_delay_s  (0.. 0.5)
                           setup.stereoDepth,   // stereo_depth (0..1)
                           16384);
        }
    }

    void close()
    {
        isValid = false;
    }

    void process(uint8_t* stream, int len)
    {
        if(!isValid)
            return; // Do nothing

        int frames = len / (sample_size * channels);
        uint8_t* in_stream = stream;
        uint8_t* out_stream = stream;

        for(int i = 0; i < channels; ++i)
        {
            if(dry[i].size() != size_t(frames))
                dry[i].resize(frames);
        }

        for(int i = 0; i < frames; ++i)
        {
            for(int c = 0; c < channels; ++c)
                dry[c][i] = readSample(in_stream, c);

            in_stream += sample_size * channels;
        }

        // Store reverb buffer
        for(int w = 0; w < channels; ++w)
        {
            // ^  Note: ftree-vectorize causes an error in this loop on g++-4.4.5
            chan[w].input_fifo.insert(chan[w].input_fifo.end(), dry[w].begin(), dry[w].end());
        }

        // Reverbify it
        for(int w = 0; w < channels; ++w)
            chan[w].Process(static_cast<size_t>(frames));

        for(int p = 0; p < frames; ++p)
        {
            for(int w = 0; w < channels; ++w)
            {
                double f = double(dry[w][size_t(p)]);
                float  l = ((size_t)w < chan[0 + (w / 2)].out->size()) ? chan[0 + (w / 2)].out[w][size_t(p)] : 0.0f;
                float  r = ((size_t)w < chan[1 + (w / 2)].out->size()) ? chan[1 + (w / 2)].out[w][size_t(p)] : 0.0f;
                float out = static_cast<float>(static_cast<double>(1 - wetonly) * f + .5 * static_cast<double>(l + r))/* + average_flt[w]*/;
                writeSample(&out_stream, out);
            }
        }
    }
};


FxReverb* reverbEffectInit(int rate, uint16_t format, int channels)
{
    FxReverb* out = new FxReverb();
    out->init(rate, format, channels);
    return out;
}

void reverbEffectFree(FxReverb* context)
{
    if(context)
    {
        context->close();
        delete context;
    }
}

void reverbEffect(int, void* stream, int len, void* context)
{
    FxReverb* out = reinterpret_cast<FxReverb*>(context);

    if(!out)
        return; // Effect doesn't working

    out->process((uint8_t*)stream, len);
}

void reverbUpdateSetup(FxReverb* context, const ReverbSetup& setup)
{
    if(context)
        context->setSettings(setup);
}
