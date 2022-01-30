/*
 * SPC Echo sound effect
 *
 * Copyright (c) 2022-2022 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <tgmath.h>
#include <string.h>
#include "spc_echo.h"
#include "fx_common.hpp"

//#define RESAMPLED_FIR
//#define CUBE_INTERPOLATION


//#define WAVE_DEEP_DEBUG
#ifdef WAVE_DEEP_DEBUG // Dirty debug
#define WAVE_PATH "/home/vitaly/Музыка/spc_echo_dump-"
#include "wave_writer.h"
#endif

#define CLAMP16F( io )\
    {\
        if(io < -1.f)\
            io = -1.f;\
        else if(io > 1.f)\
            io = 1.f;\
    }\

#define ECHO_HIST_SIZE  8
#define SDSP_RATE       32000
#define MAX_CHANNELS    10
#define ECHO_BUFFER_SIZE (32 * 1024 * MAX_CHANNELS)

//// Global registers
//enum
//{
//    r_mvoll = 0x0C, r_mvolr = 0x1C,
//    r_evoll = 0x2C, r_evolr = 0x3C,
//    r_kon   = 0x4C, r_koff  = 0x5C,
//    r_flg   = 0x6C, r_endx  = 0x7C,
//    r_efb   = 0x0D, r_pmon  = 0x2D,
//    r_non   = 0x3D, r_eon   = 0x4D,
//    r_dir   = 0x5D, r_esa   = 0x6D,
//    r_edl   = 0x7D,
//    r_fir   = 0x0F // 8 coefficients at 0x0F, 0x1F ... 0x7F
//};

//static uint8_t_t const initial_regs[register_count] =
//{
///*      0      1     2     3    4     5     6     7     8      9     A    B     C      D     E     F  */
///*00*/ 0x45, 0x8B, 0x5A, 0x9A, 0xE4, 0x82, 0x1B, 0x78, 0x00, 0x00, 0xAA, 0x96, 0x89, 0x0E, 0xE0, 0x80, /*00*/
///*10*/ 0x2A, 0x49, 0x3D, 0xBA, 0x14, 0xA0, 0xAC, 0xC5, 0x00, 0x00, 0x51, 0xBB, 0x9C, 0x4E, 0x7B, 0xFF, /*10*/
///*20*/ 0xF4, 0xFD, 0x57, 0x32, 0x37, 0xD9, 0x42, 0x22, 0x00, 0x00, 0x5B, 0x3C, 0x9F, 0x1B, 0x87, 0x9A, /*20*/
///*30*/ 0x6F, 0x27, 0xAF, 0x7B, 0xE5, 0x68, 0x0A, 0xD9, 0x00, 0x00, 0x9A, 0xC5, 0x9C, 0x4E, 0x7B, 0xFF, /*30*/
///*40*/ 0xEA, 0x21, 0x78, 0x4F, 0xDD, 0xED, 0x24, 0x14, 0x00, 0x00, 0x77, 0xB1, 0xD1, 0x36, 0xC1, 0x67, /*40*/
///*50*/ 0x52, 0x57, 0x46, 0x3D, 0x59, 0xF4, 0x87, 0xA4, 0x00, 0x00, 0x7E, 0x44, 0x9C, 0x4E, 0x7B, 0xFF, /*50*/
///*60*/ 0x75, 0xF5, 0x06, 0x97, 0x10, 0xC3, 0x24, 0xBB, 0x00, 0x00, 0x7B, 0x7A, 0xE0, 0x60, 0x12, 0x0F, /*60*/
///*70*/ 0xF7, 0x74, 0x1C, 0xE5, 0x39, 0x3D, 0x73, 0xC1, 0x00, 0x00, 0x7A, 0xB3, 0xFF, 0x4E, 0x7B, 0xFF  /*70*/
///*      0      1     2     3    4     5     6     7     8      9     A    B     C      D     E     F  */
//};


struct SpcEcho
{
    int is_valid = 0;
    float echo_ram[ECHO_BUFFER_SIZE];

    // Echo history keeps most recent 8 samples (twice the size to simplify wrap handling)
    float echo_hist[ECHO_HIST_SIZE * 2 * MAX_CHANNELS][MAX_CHANNELS];
    float (*echo_hist_pos)[MAX_CHANNELS] = echo_hist; // &echo_hist [0 to 7]

    //! offset from ESA in echo buffer
    int echo_offset = 0;
    //! number of bytes that echo_offset will stop at
    int echo_length = 0;

    double  rate_factor = 1.0;
    int     rate = SDSP_RATE;
    int     channels = 2;
    uint16_t format = AUDIO_F32;

    //! Flags
    uint8_t reg_flg = 0;
    //! $4d rw EON - Echo enable
    uint8_t reg_eon = 0;
    //! $7d rw EDL - Echo delay (ring buffer size)
    uint8_t reg_edl = 0;
    //! $0d rw EFB - Echo feedback volume
    int8_t reg_efb = 0;

    int8_t reg_mvoll = 0;
    int8_t reg_mvolr = 0;
    int8_t reg_evoll = 0;
    int8_t reg_evolr = 0;

#ifdef RESAMPLED_FIR
    //! FIR resampler
    double fir_stream_rateratio = 0;
    double fir_stream_samplecnt = 0;
    float  fir_stream_old_samples[MAX_CHANNELS];
    float  fir_stream_old_samples2[MAX_CHANNELS];
    float  fir_stream_old_samples3[MAX_CHANNELS];
    float  fir_stream_samples[MAX_CHANNELS];
    double fir_stream_rateratio_back = 0;
    double fir_stream_samplecnt_back = 0;
    float  fir_stream_old_samples_back[MAX_CHANNELS];
    float  fir_stream_old_samples_back2[MAX_CHANNELS];
    float  fir_stream_old_samples_back3[MAX_CHANNELS];
    float  fir_stream_samples_back[MAX_CHANNELS];
    float  fir_stream_midbuffer_in[20][MAX_CHANNELS];
    float  fir_stream_midbuffer_out[20][MAX_CHANNELS];
    int    fir_buffer_size = 0;
    int    fir_buffer_read = 0;
#endif


#ifdef WAVE_DEEP_DEBUG
    void *debugIn;
    void *debugInRes;
    void *debugOutRes;
    void *debugOut;
#endif

    inline double cubic(double y1, double y2, double y3, double y4, double x)
    {
        double p[] = {y1, y2, y3, y4};
        double x2 = x * x;
        double x3 = x2 * x;

        return p[1] +
              (-(0.5 * p[0]) + (0.5 * p[2])) * x +
              (p[0] - (2.5 * p[1]) + (2.0 * p[2]) - (0.5 * p[3])) * x2 +
              (-(0.5 * p[0]) + (1.5 * p[1]) - (1.5 * p[2]) + (0.5 * p[3])) * x3;
    }

    //! FIR Defaults: 80 FF 9A FF 67 FF 0F FF
    const uint8_t reg_fir_initial[8] = {0x80, 0xFF, 0x9A, 0xFF, 0x67, 0xFF, 0x0F, 0xFF};
    //! $xf rw FFCx - Echo FIR Filter Coefficient (FFC) X
    int8_t reg_fir[8] = {0, 0, 0, 0, 0, 0, 0, 0};
#if !defined(RESAMPLED_FIR)
    int8_t reg_fir_resampled[8];

    void recomputeFirResampled()
    {
        const double y_factor1 = 1.0;
        const double y_factor2 = rate_factor;
        for(size_t i = 0; i < 8; i++)
        {
            double newFactor = y_factor2 + ((y_factor1 - y_factor2) / (0.0 - 7.0)) * (i - 7.0);
            reg_fir_resampled[i] = (int8_t)(reg_fir[i] * (1.0 + ((newFactor - 1.0) / 100.0)));
        }
        std::fflush(stdout);
    }
#endif

    void setDefaultFir()
    {
        memcpy(reg_fir, reg_fir_initial, 8);
#if !defined(RESAMPLED_FIR)
        recomputeFirResampled();
#endif
    }

    void setDefaultRegs()
    {
        reg_flg = 0;

        reg_mvoll = (int8_t)0x89;
        reg_mvolr = (int8_t)0x9C;
        reg_evoll = (int8_t)0x9F;
        reg_evolr = (int8_t)0x9C;

        setDefaultFir();

        // $0d rw EFB - Echo feedback volume
        reg_efb = (int8_t)0x0E;
        // $4d rw EON - Echo enable
        reg_eon = 1;
        // $7d rw EDL - Echo delay (ring buffer size)
        reg_edl = 3;
    }

    ReadSampleCB    readSample = nullptr;
    WriteSampleCB   writeSample = nullptr;
    int             sample_size = 2;

    int init(int i_rate, uint16_t i_format, int i_channels)
    {
        is_valid = 0;
        rate = i_rate;
        format = i_format;
        channels = i_channels;
        rate_factor = (double)i_rate / SDSP_RATE;

#ifndef RESAMPLED_FIR
        if(rate_factor > 50.0)
            return -1; /* Too big scale factor */
#endif

        if(i_rate < 4000)
            return -1; /* Too small sample rate */

#ifdef RESAMPLED_FIR
        fir_stream_rateratio = (double)SDSP_RATE / i_rate;
        fir_stream_rateratio_back = (double)i_rate / SDSP_RATE;
        fir_stream_samplecnt = 0;
        fir_stream_samplecnt_back = 0;
        memset(fir_stream_old_samples, 0, sizeof(fir_stream_old_samples));
        memset(fir_stream_old_samples2, 0, sizeof(fir_stream_old_samples2));
        memset(fir_stream_old_samples3, 0, sizeof(fir_stream_old_samples3));

        memset(fir_stream_samples, 0, sizeof(fir_stream_samples));
        memset(fir_stream_old_samples_back, 0, sizeof(fir_stream_old_samples_back));
        memset(fir_stream_old_samples_back2, 0, sizeof(fir_stream_old_samples_back2));
        memset(fir_stream_old_samples_back3, 0, sizeof(fir_stream_old_samples_back3));
        memset(fir_stream_samples_back, 0, sizeof(fir_stream_samples_back));
#endif

        memset(echo_ram, 0, sizeof(echo_ram));
        memset(echo_hist, 0, sizeof(echo_hist));
#ifndef RESAMPLED_FIR
        memset(reg_fir_resampled, 0, sizeof(reg_fir_resampled));
#endif

#ifdef WAVE_DEEP_DEBUG
        debugIn = ctx_wave_open(channels, rate, sizeof(int16_t), WAVE_FORMAT_PCM, 1, 0, WAVE_PATH "in.wav");
        debugInRes = ctx_wave_open(channels, SDSP_RATE, sizeof(int16_t), WAVE_FORMAT_PCM, 1, 0, WAVE_PATH "in-res.wav");
        debugOutRes = ctx_wave_open(channels, SDSP_RATE, sizeof(int16_t), WAVE_FORMAT_PCM, 1, 0, WAVE_PATH "out-res.wav");
        debugOut = ctx_wave_open(channels, rate, sizeof(int16_t), WAVE_FORMAT_PCM, 1, 0, WAVE_PATH "out.wav");
        ctx_wave_enable_stereo(debugIn);
        ctx_wave_enable_stereo(debugInRes);
        ctx_wave_enable_stereo(debugOutRes);
        ctx_wave_enable_stereo(debugOut);
#endif

        if(!initFormat(readSample, writeSample, sample_size, format))
            return -1;

        setDefaultRegs();

        is_valid = 1;
        return 0;
    }

    void close()
    {
#ifdef WAVE_DEEP_DEBUG
        ctx_wave_close(debugIn);
        ctx_wave_close(debugInRes);
        ctx_wave_close(debugOutRes);
        ctx_wave_close(debugOut);
#endif
    }

#ifdef RESAMPLED_FIR
    void sub_process_echo(float *out, float *echo_out)
    {
        float echo_in[MAX_CHANNELS];
        float *echo_ptr;
        int c, f, e_offset;
        float v;
        float (*echohist_pos)[MAX_CHANNELS];

        e_offset = echo_offset;
        echo_ptr = echo_ram + echo_offset;

        if(!echo_offset)
            echo_length = ((reg_edl & 0x0F) * 0x400 * channels) / 2;
        e_offset += channels;
        if(e_offset >= echo_length)
            e_offset = 0;
        echo_offset = e_offset;

        /* FIR */
        for(c = 0; c < channels; c++)
            echo_in[c] = echo_ptr[c];

        echohist_pos = echo_hist_pos;
        if(++echohist_pos >= &echo_hist[ECHO_HIST_SIZE])
            echohist_pos = echo_hist;
        echo_hist_pos = echohist_pos;

        /* --------------- FIR filter-------------- */
        for(c = 0; c < channels; c++)
            echohist_pos[0][c] = echohist_pos[8][c] = echo_in[c];

        for(c = 0; c < channels; ++c)
            echo_in[c] *= reg_fir[7];

        for(f = 0; f <= 6; ++f)
        {
            for(c = 0; c < channels; ++c)
                echo_in[c] += echohist_pos[f + 1][c] * reg_fir[f];
        }
        /* ---------------------------------------- */

        /* Echo out */
        if(!(reg_flg & 0x20))
        {
            for(c = 0; c < channels; c++)
            {
                // v = (echo_out[c] >> 7) + ((echo_in[c] * reg_efb) >> 14);
                v = (echo_out[c] / 128) + ((echo_in[c] * reg_efb) / 16384.f);
                CLAMP16F(v);
                echo_ptr[c] = v;
            }
        }

        // Do out
        for(c = 0; c < channels; ++c)
            out[c] = echo_in[c];
    }

    void pre_process_echo(float *echo_out)
    {
        int c, q = 0;

        while(fir_stream_samplecnt >= fir_stream_rateratio)
        {
            if(q > 0)
                return;
            for(c = 0; c < channels; c++)
            {
                fir_stream_old_samples3[c] = fir_stream_old_samples2[c];
                fir_stream_old_samples2[c] = fir_stream_old_samples[c];
                fir_stream_old_samples[c] = fir_stream_samples[c];
                fir_stream_samples[c] = echo_out[c];
            }
            fir_stream_samplecnt -= fir_stream_rateratio;
            q++;
        }

        fir_buffer_size = 0;
        while(fir_stream_samplecnt < fir_stream_rateratio)
        {
            for(c = 0; c < channels; c++)
            {
#ifdef CUBE_INTERPOLATION

#define         X   (fir_stream_samplecnt)
#define         D   ((double)fir_stream_samples[c])
#define         C   ((double)fir_stream_old_samples[c])
#define         B   ((double)fir_stream_old_samples2[c])
#define         A   ((double)fir_stream_old_samples3[c])

                fir_stream_midbuffer_in[fir_buffer_size][c] = cubic(A, B, C, D, X);

#undef         X
#undef         A
#undef         B
#undef         C
#undef         D

#else
                fir_stream_midbuffer_in[fir_buffer_size][c] = (((double)fir_stream_old_samples[c] * (fir_stream_rateratio - fir_stream_samplecnt)
                                                              + (double)fir_stream_samples[c] * fir_stream_samplecnt) / fir_stream_rateratio);
#endif
            }

            sub_process_echo(fir_stream_midbuffer_out[fir_buffer_size], fir_stream_midbuffer_in[fir_buffer_size]);
#ifdef WAVE_DEEP_DEBUG
            {
                int16_t outw[MAX_CHANNELS];
                for(c = 0; c < channels; ++c)
                    outw[c] = fir_stream_midbuffer_in[fir_buffer_size][c] / 128;
                ctx_wave_write(debugInRes, (const uint8_t*)outw, channels * sizeof(int16_t));
            }
            {
                int16_t outw[MAX_CHANNELS];
                for(c = 0; c < channels; ++c)
                    outw[c] = fir_stream_midbuffer_out[fir_buffer_size][c] / 128;
                ctx_wave_write(debugOutRes, (const uint8_t*)outw, channels * sizeof(int16_t));
            }
#endif

            fir_stream_samplecnt += 1.0;
            fir_buffer_size++;
        }
    }

    void process_echo(float *out, float *echo_out)
    {
        int c, f;

#ifdef WAVE_DEEP_DEBUG
        {
            int16_t outw[MAX_CHANNELS];
            for(c = 0; c < channels; ++c)
                outw[c] = echo_out[c] / 128;
            ctx_wave_write(debugIn, (const uint8_t*)outw, channels * sizeof(int16_t));
        }
#endif

        // Process directly if no resampling needed
        if(rate == SDSP_RATE)
        {
            sub_process_echo(out, echo_out);

#ifdef WAVE_DEEP_DEBUG
            {
                int16_t outw[MAX_CHANNELS];
                for(c = 0; c < channels; ++c)
                    outw[c] = out[c] / 128;
                ctx_wave_write(debugOut, (const uint8_t*)outw, channels * sizeof(int16_t));
            }
#endif
            return;
        }

        pre_process_echo(echo_out);

        f = 0;
        while(fir_stream_samplecnt_back >= fir_stream_rateratio_back && fir_buffer_size > 0)
        {
            for(c = 0; c < channels; c++)
            {
                fir_stream_old_samples_back3[c] = fir_stream_old_samples_back2[c];
                fir_stream_old_samples_back2[c] = fir_stream_old_samples_back[c];
                fir_stream_old_samples_back[c] = fir_stream_samples_back[c];
                fir_stream_samples_back[c] = fir_stream_midbuffer_out[f][c];
            }
            fir_stream_samplecnt_back -= fir_stream_rateratio_back;
            f++;
        }

        for(c = 0; c < channels; c++)
        {
#ifdef CUBE_INTERPOLATION

#define         X   (fir_stream_samplecnt_back)
#define         D   ((double)fir_stream_samples_back[c])
#define         C   ((double)fir_stream_old_samples_back[c])
#define         B   ((double)fir_stream_old_samples_back2[c])
#define         A   ((double)fir_stream_old_samples_back3[c])

                out[c] = cubic(A, B, C, D, X);

#undef         X
#undef         A
#undef         B
#undef         C
#undef         D

#else
            out[c] = (((double)fir_stream_old_samples_back[c] * (fir_stream_rateratio_back - fir_stream_samplecnt_back)
                     + (double)fir_stream_samples_back[c] * fir_stream_samplecnt_back) / fir_stream_rateratio_back);
#endif
        }

#ifdef WAVE_DEEP_DEBUG
        {
            int16_t outw[MAX_CHANNELS];
            for(c = 0; c < channels; ++c)
                outw[c] = out[c] / 128;
            ctx_wave_write(debugOut, (const uint8_t*)outw, channels * sizeof(int16_t));
        }
#endif

        if(fir_buffer_size > 0)
            fir_stream_samplecnt_back += 1.0;
    }
#endif

    void process(uint8_t *stream, int len)
    {
        int frames = len / (sample_size * channels);

        float main_out[MAX_CHANNELS];
        float echo_out[MAX_CHANNELS];
        float echo_in[MAX_CHANNELS];

        int c;
        float ov;
#ifndef RESAMPLED_FIR
        int f, e_offset;
        float v;
#endif

        float mvoll[2] = {(float)reg_mvoll, (float)reg_mvolr};
        float evoll[2] = {(float)reg_evoll, (float)reg_evolr};

#ifndef RESAMPLED_FIR
        float (*echohist_pos)[MAX_CHANNELS];
        float *echo_ptr;
#endif

        memset(main_out, 0, sizeof(main_out));
        memset(echo_out, 0, sizeof(echo_out));
        memset(echo_in, 0, sizeof(echo_in));

        if(!is_valid)
            return;

        do
        {
            for(c = 0; c < channels; ++c)
                main_out[c] = readSample(stream, c) * 128;

            if(reg_eon & 1)
            {
                for(c = 0; c < channels; c++)
                    echo_out[c] = main_out[c];
            }

#ifdef RESAMPLED_FIR
            process_echo(echo_in, echo_out);
#else
            e_offset = echo_offset;
            echo_ptr = echo_ram + echo_offset;

            if(!echo_offset)
                echo_length = (int)round((((reg_edl & 0x0F) * 0x400 * channels) / 2) * rate_factor);
            e_offset += channels;
            if(e_offset >= echo_length)
                e_offset = 0;
            echo_offset = e_offset;

            /* FIR */
            for(c = 0; c < channels; c++)
                echo_in[c] = echo_ptr[c];

            echohist_pos = echo_hist_pos;
            if(++echohist_pos >= &echo_hist[ECHO_HIST_SIZE])
                echohist_pos = echo_hist;
            echo_hist_pos = echohist_pos;

            /* --------------- FIR filter-------------- */
#if 0 // Original code
            for(c = 0; c < channels; c++)
                echohist_pos[0][c] = echohist_pos[8][c] = echo_in[c];

            for(c = 0; c < channels; ++c)
                echo_in[c] *= reg_fir[7];

            for(f = 0; f <= 6; ++f)
            {
                for(c = 0; c < channels; ++c)
                    echo_in[c] += echo_hist_pos[f + 1][c] * reg_fir[f];
            }
#else
            for(c = 0; c < channels; c++)
                echohist_pos[0][c] = echohist_pos[8][c] = echo_in[c];

            for(c = 0; c < channels; ++c)
                echo_in[c] *= reg_fir_resampled[7];

            for(f = 0; f <= 6; ++f)
            {
                for(c = 0; c < channels; ++c)
                    echo_in[c] += echo_hist_pos[f + 1][c] * reg_fir_resampled[f];
            }
#endif
            /* ---------------------------------------- */

            /* Echo out */
            if(!(reg_flg & 0x20))
            {
                for(c = 0; c < channels; c++)
                {
                    //v = (echo_out[c] >> 7) + ((echo_in[c] * reg_efb) >> 14);
                    v = (echo_out[c] / 128) + ((echo_in[c] * reg_efb) / 16384.f);
                    CLAMP16F(v);
                    echo_ptr[c] = v;
                }
            }
#endif
            /* Sound out */
            for(c = 0; c < channels; c++)
            {
                ov = (main_out[c] * mvoll[c % 2] + echo_in[c] * evoll[c % 2]) / 16384;
                CLAMP16F(ov);
                if((reg_flg & 0x40))
                    ov = 0;
                writeSample(&stream, ov);
            }
        }
        while(--frames);
    }
};


SpcEcho *echoEffectInit(int rate, uint16_t format, int channels)
{
    SpcEcho *out = new SpcEcho();
    out->init(rate, format, channels);
    return out;
}

void echoEffectFree(SpcEcho *context)
{
    if(context)
    {
        context->close();
        delete context;
    }
}

void spcEchoEffect(int, void *stream, int len, void *context)
{
    SpcEcho *out = reinterpret_cast<SpcEcho *>(context);
    if(!out)
        return; // Effect doesn't working
    out->process((uint8_t*)stream, len);
}

void echoEffectSetReg(SpcEcho *out, EchoSetup key, int val)
{
    if(!out)
        return;

    switch(key)
    {
    case ECHO_EON:
        out->reg_eon = (uint8_t)val;
        break;
    case ECHO_EDL:
        out->reg_edl = (uint8_t)val;
        break;
    case ECHO_EFB:
        out->reg_efb = (int8_t)val;
        break;
    case ECHO_MVOLL:
        out->reg_mvoll = (int8_t)val;
        break;
    case ECHO_MVOLR:
        out->reg_mvolr = (int8_t)val;
        break;
    case ECHO_EVOLL:
        out->reg_evoll = (int8_t)val;
        break;
    case ECHO_EVOLR:
        out->reg_evolr = (int8_t)val;
        break;

    case ECHO_FIR0:
        out->reg_fir[0]= (int8_t)val;
#ifndef RESAMPLED_FIR
        out->recomputeFirResampled();
#endif
        break;
    case ECHO_FIR1:
        out->reg_fir[1]= (int8_t)val;
#ifndef RESAMPLED_FIR
        out->recomputeFirResampled();
#endif
        break;
    case ECHO_FIR2:
        out->reg_fir[2]= (int8_t)val;
#ifndef RESAMPLED_FIR
        out->recomputeFirResampled();
#endif
        break;
    case ECHO_FIR3:
        out->reg_fir[3]= (int8_t)val;
#ifndef RESAMPLED_FIR
        out->recomputeFirResampled();
#endif
        break;
    case ECHO_FIR4:
        out->reg_fir[4]= (int8_t)val;
#ifndef RESAMPLED_FIR
        out->recomputeFirResampled();
#endif
        break;
    case ECHO_FIR5:
        out->reg_fir[5]= (int8_t)val;
#ifndef RESAMPLED_FIR
        out->recomputeFirResampled();
#endif
        break;
    case ECHO_FIR6:
        out->reg_fir[6]= (int8_t)val;
#ifndef RESAMPLED_FIR
        out->recomputeFirResampled();
#endif
        break;
    case ECHO_FIR7:
        out->reg_fir[7]= (int8_t)val;
#ifndef RESAMPLED_FIR
        out->recomputeFirResampled();
#endif
        break;
    }
}

int echoEffectGetReg(SpcEcho *out, EchoSetup key)
{
    if(!out)
        return 0;

    switch(key)
    {
    case ECHO_EON:
        return out->reg_eon;
    case ECHO_EDL:
        return out->reg_edl;
    case ECHO_EFB:
        return out->reg_efb;
    case ECHO_MVOLL:
        return out->reg_mvoll;
    case ECHO_MVOLR:
        return out->reg_mvolr;
    case ECHO_EVOLL:
        return out->reg_evoll;
    case ECHO_EVOLR:
        return out->reg_evolr;

    case ECHO_FIR0:
        return out->reg_fir[0];
    case ECHO_FIR1:
        return out->reg_fir[1];
    case ECHO_FIR2:
        return out->reg_fir[2];
    case ECHO_FIR3:
        return out->reg_fir[3];
    case ECHO_FIR4:
        return out->reg_fir[4];
    case ECHO_FIR5:
        return out->reg_fir[5];
    case ECHO_FIR6:
        return out->reg_fir[6];
    case ECHO_FIR7:
        return out->reg_fir[7];
    }

    return 0;
}

void echoEffectResetFir(SpcEcho *out)
{
    if(!out)
        return;
    out->setDefaultFir();
}

void echoEffectResetDefaults(SpcEcho *out)
{
    if(!out)
        return;
    out->setDefaultRegs();
}
