/*
 *
 * Copyright (c) 2020-2024 ds-sloth and Vitaly Novichkov <admin@wohlnet.ru>
 *
 * THIS program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * THIS program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with THIS program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 * This file incorporates substantial portions of libavformat's example mux.c
 *
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

extern "C"
{
#include <stdint.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
};

#include <SDL2/SDL_assert.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_audio.h>

#include <Utils/files.h>
#include <pge_delay.h>
#include <Logger/logger.h>

#include "pge_video_rec.h"

#define HAS_CHANNELLAYOUT (LIBAVUTIL_VERSION_MAJOR >= 58)

#undef av_err2str

static char s_errbuf[(AV_ERROR_MAX_STRING_SIZE > 512) ? AV_ERROR_MAX_STRING_SIZE : 512];

inline const char* av_err2str(int errnum)
{
    return av_make_error_string(s_errbuf, AV_ERROR_MAX_STRING_SIZE, errnum);
}

static std::string av_log_buffer;
static SDL_mutex* av_log_mutex = nullptr;

static void av_log_callback_pLog(void* avcl, int level, const char* fmt, va_list vl)
{
    if(av_log_mutex)
        SDL_LockMutex(av_log_mutex);

    // prepare the log buffer with the label
    static const char log_label[] = {'P','G','E','V','i','d','e','o','R','e','c',':',' ','(','f','f','m','p','e','g',')',' '};

    if(av_log_buffer.size() < sizeof(log_label))
    {
        av_log_buffer.resize(sizeof(log_label));
        memcpy(&av_log_buffer[0], log_label, sizeof(log_label));
    }

    // add AV Class name to buffer
    AVClass* avc = avcl ? *(AVClass**)avcl : NULL;
    if(avc)
    {
        s_errbuf[0] = '\0';
        snprintf(s_errbuf, sizeof(s_errbuf), "[%s @ %p] ", avc->item_name(avcl), avcl);

        av_log_buffer += s_errbuf;
    }

    // add message to buffer
    s_errbuf[0] = '\0';
    vsnprintf(s_errbuf, sizeof(s_errbuf), fmt, vl);

    av_log_buffer += s_errbuf;

    // look for newlines
    size_t cutpoint = sizeof(log_label);
    for(size_t checkpoint = 0; checkpoint < av_log_buffer.size(); ++checkpoint)
    {
        if(av_log_buffer[checkpoint] != '\n')
            continue;

        // print the line from cutpoint to checkpoint
        av_log_buffer[checkpoint] = '\0';

        // add "PGEVideoRec: "
        if(cutpoint >= sizeof(log_label))
        {
            memcpy(&av_log_buffer[cutpoint - sizeof(log_label)], log_label, sizeof(log_label));
            cutpoint -= sizeof(log_label);
        }

        switch(level)
        {
        case(AV_LOG_PANIC):
            pLogFatal(&av_log_buffer[cutpoint]);
            break;
        case(AV_LOG_FATAL):
            pLogCritical(&av_log_buffer[cutpoint]);
            break;
        case(AV_LOG_ERROR):
        case(AV_LOG_WARNING):
            pLogWarning(&av_log_buffer[cutpoint]);
            break;
        case(AV_LOG_INFO):
        case(AV_LOG_VERBOSE):
            pLogDebug(&av_log_buffer[cutpoint]);
            break;
        case(AV_LOG_DEBUG):
        default:
            // do nothing
            break;
        }

        cutpoint = checkpoint + 1;
    }

    av_log_buffer.erase(av_log_buffer.begin() + sizeof(log_label), av_log_buffer.begin() + cutpoint);

    if(av_log_mutex)
        SDL_UnlockMutex(av_log_mutex);
}


// a wrapper around a single output AVStream
struct OutputStream {
    AVStream *st = nullptr;
    AVCodecContext *enc = nullptr;

    /* pts of the next frame that will be generated */
    int64_t next_pts = 0;
    int samples_count = 0;

    AVFrame *frame = nullptr;
    AVFrame *tmp_frame = nullptr;

    AVPacket *tmp_pkt = nullptr;

    struct SwsContext *sws_ctx = nullptr;
    struct SwrContext *swr_ctx = nullptr;
};

struct PGE_VideoRecording_VP8 : public PGE_VideoRecording
{
    OutputStream video_st;
    OutputStream audio_st;
    AVFormatContext *oc = nullptr;
#if HAS_CHANNELLAYOUT
    AVChannelLayout src_ch_layout;
#else
    int64_t src_ch_layout;
#endif
    AVSampleFormat src_sample_fmt = AV_SAMPLE_FMT_NONE;

    PGE_VideoFrame current_frame;
    PGE_VideoFrame next_frame;
    PGE_AudioChunk current_chunk;
    decltype(current_chunk.audio_buffer.begin()) current_chunk_offset = current_chunk.audio_buffer.end();

    uint64_t first_timestamp = 0;

    PGE_VideoRecording_VP8();
    virtual ~PGE_VideoRecording_VP8();

    // returns the best file extension for the recording type
    virtual const char* extension() const override;

    // should be called by a main thread, returns false on failure
    virtual bool initialize(const char* filename) override;

    // should be called by an encoding thread, terminates once the empty end frame has been dequeued.
    virtual bool encoding_thread() override;

    // returns true if the sample format is valid
    bool set_src_sample_fmt();
};

bool PGE_VideoRecording_VP8::set_src_sample_fmt()
{
    switch(spec.audio_sample_format)
    {
    case AUDIO_U8:
        src_sample_fmt = AV_SAMPLE_FMT_U8;
        break;
    case AUDIO_S16SYS:
        src_sample_fmt = AV_SAMPLE_FMT_S16;
        break;
    case AUDIO_S32SYS:
        src_sample_fmt = AV_SAMPLE_FMT_S32;
        break;
    case AUDIO_F32SYS:
        src_sample_fmt = AV_SAMPLE_FMT_FLT;
        break;
    default:
        return false;
    }

#if HAS_CHANNELLAYOUT
    av_channel_layout_default(&src_ch_layout, spec.audio_channel_count);
#else
    src_ch_layout = av_get_default_channel_layout(spec.audio_channel_count);
#endif

    return true;
}

static int write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c,
                       AVStream *st, AVFrame *frame, AVPacket *pkt)
{
    int ret;

    // send the frame to the encoder
    ret = avcodec_send_frame(c, frame);
    if (ret < 0) {
        pLogWarning("PGEVideoRec: error sending a frame to the encoder: %s",
                av_err2str(ret));
        return 1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            pLogWarning("PGEVideoRec: error encoding a frame: %s", av_err2str(ret));
            return 1;
        }

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(pkt, c->time_base, st->time_base);
        pkt->stream_index = st->index;

        /* Write the compressed frame to the media file. */
        // log_packet(fmt_ctx, pkt);
        ret = av_interleaved_write_frame(fmt_ctx, pkt);
        /* pkt is now blank (av_interleaved_write_frame() takes ownership of
         * its contents and resets pkt), so that no unreferencing is necessary.
         * THIS would be different if one used av_write_frame(). */
        if (ret < 0) {
            pLogWarning("PGEVideoRec: error while writing output packet: %s", av_err2str(ret));
            return 1;
        }
    }

    return ret == AVERROR_EOF ? 1 : 0;
}

/* Add an output stream. */
static bool init_stream(OutputStream *ost, AVFormatContext *oc,
                       const AVCodec **codec, enum AVCodecID codec_id)
{
    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        pLogWarning("PGEVideoRec: Could not find encoder for '%s'", avcodec_get_name(codec_id));
        return false;
    }

    ost->tmp_pkt = av_packet_alloc();
    if (!ost->tmp_pkt) {
        pLogWarning("PGEVideoRec: Could not allocate AVPacket");
        return false;
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        pLogWarning("PGEVideoRec: Could not allocate stream");
        return false;
    }

    ost->st->id = oc->nb_streams-1;

    ost->enc = avcodec_alloc_context3(*codec);
    if (!ost->enc) {
        pLogWarning("PGEVideoRec: Could not alloc an encoding context");
        return false;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        ost->enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // uncertain whether this is needed
    // ost->enc->codec_id = codec_id;

    return true;
}

/**************************************************************/
/* audio output */

static AVFrame *alloc_audio_frame(AVCodecContext* enc, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        pLogWarning("PGEVideoRec: Error allocating an audio frame");
        return NULL;
    }


    frame->format = enc->sample_fmt;
#if HAS_CHANNELLAYOUT
    av_channel_layout_copy(&frame->ch_layout, &enc->ch_layout);
#else
    frame->channel_layout = enc->channel_layout;
#endif
    frame->sample_rate = enc->sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        if (av_frame_get_buffer(frame, 0) < 0) {
            pLogWarning("PGEVideoRec: Error allocating an audio buffer");
            return NULL;
        }
    }

    return frame;
}

static bool open_audio(const PGE_VideoRecording_VP8* THIS,
                       const AVCodec *codec,
                       OutputStream *ost, AVDictionary *opt_arg)
{
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;

    // fill parameters
    ost->enc->sample_fmt  = codec->sample_fmts ?
        codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    ost->enc->bit_rate    = 64000;
    ost->enc->sample_rate = THIS->spec.audio_sample_rate;

    // restrict sample rate to supported list
    if(codec->supported_samplerates)
    {
        bool sample_rate_supported = false;
        for(int i = 0; codec->supported_samplerates[i]; i++)
        {
            if(codec->supported_samplerates[i] == THIS->spec.audio_sample_rate)
            {
                sample_rate_supported = true;
                break;
            }
        }

        if(!sample_rate_supported)
            ost->enc->sample_rate = codec->supported_samplerates[0];
    }

#if HAS_CHANNELLAYOUT
    const AVChannelLayout& layout = (THIS->spec.audio_channel_count == 1) ? AVChannelLayout AV_CHANNEL_LAYOUT_MONO : AVChannelLayout AV_CHANNEL_LAYOUT_STEREO;
    av_channel_layout_copy(&ost->enc->ch_layout, &layout);
#else
    ost->enc->channel_layout = (THIS->spec.audio_channel_count == 1) ? av_get_default_channel_layout(1) : av_get_default_channel_layout(2);
#endif

    ost->st->time_base = AVRational { 1, ost->enc->sample_rate };

    /* open it */
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(ost->enc, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        pLogWarning("PGEVideoRec: Could not open audio codec: %s", av_err2str(ret));
        exit(1);
    }

    /* init signal generator */

    if (ost->enc->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = ost->enc->frame_size;

    ost->frame     = alloc_audio_frame(ost->enc, nb_samples);

    if(!ost->frame)
        return false;

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, ost->enc);
    if (ret < 0) {
        pLogWarning("PGEVideoRec: Could not copy the stream parameters");
        return false;
    }

    /* create resampler context */
    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx) {
        pLogWarning("PGEVideoRec: Could not allocate resampler context");
        return false;
    }

    /* set options */
#if HAS_CHANNELLAYOUT
    av_opt_set_chlayout  (ost->swr_ctx, "in_chlayout",       &THIS->src_ch_layout,          0);
    av_opt_set_chlayout  (ost->swr_ctx, "out_chlayout",      &ost->enc->ch_layout,          0);
#else
    av_opt_set_channel_layout  (ost->swr_ctx, "in_channel_layout",  THIS->src_ch_layout,          0);
    av_opt_set_channel_layout  (ost->swr_ctx, "out_channel_layout", ost->enc->channel_layout,     0);
#endif
    av_opt_set_int       (ost->swr_ctx, "in_sample_rate",     THIS->spec.audio_sample_rate, 0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",      THIS->src_sample_fmt,         0);
    av_opt_set_int       (ost->swr_ctx, "out_sample_rate",    ost->enc->sample_rate,        0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",     ost->enc->sample_fmt,         0);

    /* initialize the resampling context */
    if ((ret = swr_init(ost->swr_ctx)) < 0) {
        pLogWarning("PGEVideoRec: Failed to initialize the resampling context");
        return false;
    }

    return true;
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
static AVFrame *get_audio_frame(PGE_VideoRecording_VP8* THIS, OutputStream *ost)
{
    AVFrame *frame = ost->frame;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    int ret = av_frame_make_writable(ost->frame);
    if(ret < 0)
    {
        pLogWarning("PGEVideoRec: av_frame_make_writable failed");
        return NULL;
    }

    int samples_left = frame->nb_samples;

    int dest_active = 0;

    uint8_t* dest[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for(int i = 0; i < 8 && i < AV_NUM_DATA_POINTERS; i++)
    {
        dest[i] = frame->data[i];
        if(dest[i])
            dest_active++;
    }

#if HAS_CHANNELLAYOUT
    int num_channels = frame->ch_layout.nb_channels;
#else
    int num_channels = av_get_channel_layout_nb_channels(frame->channel_layout);
#endif

    if(dest_active != 1 && dest_active != num_channels)
    {
        pLogWarning("PGEVideoRec: invalid number of channels in dest buffer");
        return NULL;
    }

    ptrdiff_t dest_bytes_per_sample = av_get_bytes_per_sample((AVSampleFormat)frame->format);
    if(dest_active == 1)
        dest_bytes_per_sample *= num_channels;

    ptrdiff_t src_bytes_per_sample = THIS->spec.audio_channel_count * av_get_bytes_per_sample(THIS->src_sample_fmt);
    bool first_loop = true;

    while (samples_left > 0)
    {
        const uint8_t* src[1] = {NULL};
        int src_samples = 0;

        // use remaining samples from buffer if possible
        if(!first_loop || swr_get_out_samples(ost->swr_ctx, 0) < samples_left)
        {
            // poll from game audio if buffer is empty
            while(!THIS->has_audio())
            {
                // no more samples
                if(THIS->exit_requested)
                    return NULL;

                PGE_Delay(1);
                continue;
            }

            THIS->current_chunk = THIS->dequeue_audio();
            THIS->current_chunk_offset = THIS->current_chunk.audio_buffer.begin();

            src[0] = {THIS->current_chunk.audio_buffer.data()};
            src_samples = THIS->current_chunk.audio_buffer.size() / src_bytes_per_sample;
        }

        first_loop = false;

        /* convert to destination format */
        int samples_done = swr_convert(ost->swr_ctx,
                          dest, samples_left,
                          src, src_samples);

        if(samples_done < 0)
        {
            pLogWarning("PGEVideoRec: error during resampling");
            return NULL;
        }

        for(int i = 0; i < 8; i++)
        {
            if(dest[i])
                dest[i] += dest_bytes_per_sample * samples_done;
        }

        samples_left -= samples_done;
    }

    return frame;
}

/*
 * encode one audio frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
static int write_audio_frame(PGE_VideoRecording_VP8* THIS, AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext *c;
    AVFrame *frame;

    c = ost->enc;

    frame = get_audio_frame(THIS, ost);

    if(!frame)
        return 1;

    frame->pts = ost->next_pts;
    ost->samples_count += frame->nb_samples;
    ost->next_pts = av_rescale_q(ost->samples_count, AVRational {1, c->sample_rate}, c->time_base);

    return write_frame(oc, c, ost->st, frame, ost->tmp_pkt);
}

/**************************************************************/
/* video output */

static AVFrame *alloc_frame(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *frame;
    int ret;

    frame = av_frame_alloc();
    if(!frame)
        return NULL;

    frame->format = pix_fmt;
    frame->width  = width;
    frame->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0)
    {
        av_frame_free(&frame);
        frame = NULL;
    }

    return frame;
}

static bool open_video(PGE_VideoRecording_VP8* THIS, const AVCodec *codec,
                       OutputStream *ost, AVDictionary *opt_arg)
{
    // initialize parameters

    /* Resolution must be a multiple of two. */
    if(THIS->spec.downscale_video)
    {
        ost->enc->width    = THIS->spec.frame_w / 4 * 2;
        ost->enc->height   = THIS->spec.frame_h / 4 * 2;
    }
    else
    {
        ost->enc->width    = THIS->spec.frame_w / 2 * 2;
        ost->enc->height   = THIS->spec.frame_h / 2 * 2;
    }

    ost->enc->bit_rate = 8000000;
    /* timebase: THIS is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    ost->st->time_base = AVRational { 1, THIS->spec.frame_rate };
    ost->enc->time_base       = ost->st->time_base;

    // c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    ost->enc->pix_fmt       = AV_PIX_FMT_YUV420P;

    // initialize codec parameters
    int ret;
    AVCodecContext *c = ost->enc;
    AVDictionary *opt = NULL;

    av_dict_copy(&opt, opt_arg, 0);
    av_dict_set( &opt, "deadline", "good", 0 );
    av_dict_set( &opt, "cpu-used", "5", 0 );
    av_dict_set_int( &opt, "crf", THIS->spec.video_quality, 0 );

    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        pLogWarning("PGEVideoRec: could not open video codec: %s", av_err2str(ret));
        return false;
    }

    /* allocate and init a re-usable frame */
    ost->frame = alloc_frame(c->pix_fmt, c->width, c->height);
    if (!ost->frame)
    {
        pLogWarning("PGEVideoRec: could not allocate output video frame");
        return false;
    }

    /* If the output format is not RGB24, then a temporary RGB24
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_RGB24) {
        ost->tmp_frame = alloc_frame(AV_PIX_FMT_RGB24, c->width, c->height);
        if (!ost->tmp_frame) {
            pLogWarning("PGEVideoRec: could not allocate output video frame");
            return false;
        }
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        pLogWarning("PGEVideoRec: could not copy stream parameters");
        return false;
    }

    return true;
}

/* Prepare a dummy image. */
static void fill_rgb_image(AVFrame *pict, uint8_t* src,
                           int width, int height, int src_stride)
{
    int x, y;

    // i = frame_index;

    /* Y */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pict->data[0][y * pict->linesize[0] + x * 3 + 0] = src[y * src_stride + (x) * 4 + 0];
            pict->data[0][y * pict->linesize[0] + x * 3 + 1] = src[y * src_stride + (x) * 4 + 1];
            pict->data[0][y * pict->linesize[0] + x * 3 + 2] = src[y * src_stride + (x) * 4 + 2];
        }
    }
}

/* Prepare a dummy image. */
static void fill_rgb_image_downscale(AVFrame *pict, uint8_t* src,
                           int width, int height, int src_stride)
{
    int x, y;

    // i = frame_index;
    int stride = 2 * src_stride;

    /* Y */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pict->data[0][y * pict->linesize[0] + x * 3 + 0] = src[y * stride + x * 8 + 0];
            pict->data[0][y * pict->linesize[0] + x * 3 + 1] = src[y * stride + x * 8 + 1];
            pict->data[0][y * pict->linesize[0] + x * 3 + 2] = src[y * stride + x * 8 + 2];
        }
    }
}

static AVFrame *get_video_frame(PGE_VideoRecording_VP8* THIS, OutputStream *ost)
{
    AVCodecContext *c = ost->enc;

    /* find a next frame */
    while(THIS->first_timestamp == 0 || av_compare_ts(ost->next_pts, ost->enc->time_base,
                        THIS->next_frame.timestamp - THIS->first_timestamp, AVRational { 1, 1000000 }) >= 0)
    {
        if(THIS->next_frame.end_frame)
            return NULL;

        THIS->current_frame = std::move(THIS->next_frame);

        while(!THIS->has_frame())
        {
            // no more samples
            if(THIS->exit_requested)
                return NULL;

            PGE_Delay(1);
            continue;
        }

        THIS->next_frame = THIS->dequeue_frame();

        if(THIS->first_timestamp == 0)
            THIS->first_timestamp = THIS->next_frame.timestamp;
    }

    /* check if we want to generate more frames */
    // if (ost->next_pts > ost->input->frame_count)
    //     return NULL;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally; make sure we do not overwrite it here */
    if (av_frame_make_writable(ost->frame) < 0)
        exit(1);

    if (c->pix_fmt != AV_PIX_FMT_RGB24) {
        /* as we only generate a RGB24 picture, we must convert it
         * to the codec pixel format if needed */
        if (!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(c->width, c->height,
                                          AV_PIX_FMT_RGB24,
                                          c->width, c->height,
                                          c->pix_fmt,
                                          SWS_BICUBIC, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                fprintf(stderr,
                        "Could not initialize the conversion context");
                exit(1);
            }
        }
        (THIS->spec.downscale_video ? fill_rgb_image_downscale : fill_rgb_image)(ost->tmp_frame, THIS->current_frame.pixels.data(), c->width, c->height, THIS->spec.frame_pitch);
        sws_scale(ost->sws_ctx, (const uint8_t * const *) ost->tmp_frame->data,
                  ost->tmp_frame->linesize, 0, c->height, ost->frame->data,
                  ost->frame->linesize);
    } else {
        (THIS->spec.downscale_video ? fill_rgb_image_downscale : fill_rgb_image)(ost->frame, THIS->current_frame.pixels.data(), c->width, c->height, THIS->spec.frame_pitch);
    }

    ost->frame->pts = ost->next_pts;

    while(av_compare_ts(ost->next_pts, ost->enc->time_base,
                        THIS->next_frame.timestamp - THIS->first_timestamp, AVRational{ 1, 1000000 }) < 0)
    {
        ost->next_pts++;
    }

    return ost->frame;
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
static int write_video_frame(PGE_VideoRecording_VP8* THIS, AVFormatContext *oc, OutputStream *ost)
{
    auto video_frame = get_video_frame(THIS, ost);
    if(!video_frame)
        return 1;

    return write_frame(oc, ost->enc, ost->st, video_frame, ost->tmp_pkt);
}

static void close_stream(OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    av_packet_free(&ost->tmp_pkt);
    sws_freeContext(ost->sws_ctx);
    ost->sws_ctx = NULL;
    swr_free(&ost->swr_ctx);
}


const char* PGE_VideoRecording_VP8::extension() const
{
    return "webm";
}

bool PGE_VideoRecording_VP8::initialize(const char* filename)
{
    pLogInfo("PGEVideoRec: starting VP8/opus recording to [%s]", filename);

    const AVCodec *audio_codec, *video_codec;
    int ret;
    AVDictionary *opt = NULL;

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, "webm", filename);
    if(!oc)
        return false;

    const AVOutputFormat *fmt = oc->oformat;

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    if(fmt->video_codec == AV_CODEC_ID_NONE || !init_stream(&video_st, oc, &video_codec, AV_CODEC_ID_VP8))
        return false;

    if (spec.audio_enabled && fmt->audio_codec != AV_CODEC_ID_NONE && set_src_sample_fmt())
    {
        if(!init_stream(&audio_st, oc, &audio_codec, fmt->audio_codec))
        {
            spec.audio_enabled = false;
            close_stream(&audio_st);
        }
    }
    else
        spec.audio_enabled = false;

    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    if (video_st.st)
    {
        if(!open_video(this, video_codec, &video_st, opt))
            return false;
    }

    if (spec.audio_enabled && audio_st.st)
    {
        if(!open_audio(this, audio_codec, &audio_st, opt))
            spec.audio_enabled = false;
    }

    av_dump_format(oc, 0, filename, 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            pLogWarning("PGEVideoRec: could not open destination [%s]: %s", filename,
                    av_err2str(ret));
            return false;
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(oc, &opt);
    if (ret < 0) {
        pLogWarning("PGEVideoRec: could not write headers: %s",
                av_err2str(ret));
        return false;
    }

    return true;
}

bool PGE_VideoRecording_VP8::encoding_thread()
{
    bool have_video = video_st.st;
    bool have_audio = audio_st.st;

    bool encode_video = have_video;
    bool encode_audio = have_audio;

    while (encode_video || encode_audio) {
        /* select the stream to encode */
        if (encode_video &&
            (!encode_audio || av_compare_ts(video_st.next_pts, video_st.enc->time_base,
                                            audio_st.next_pts, audio_st.enc->time_base) <= 0)) {
            encode_video = !write_video_frame(this, oc, &video_st);
        } else {
            encode_audio = !write_audio_frame(this, oc, &audio_st);
        }
    }

    av_write_trailer(oc);

    /* Close each codec. */
    if (have_video)
        close_stream(&video_st);
    if (have_audio)
        close_stream(&audio_st);

    const AVOutputFormat *fmt = oc->oformat;
    if (!(fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_closep(&oc->pb);

    /* free the stream */
    avformat_free_context(oc);

#if HAS_CHANNELLAYOUT
    av_channel_layout_uninit(&src_ch_layout);
#endif

    return true;
}

PGE_VideoRecording_VP8::PGE_VideoRecording_VP8()
{
    SDL_assert_release(!av_log_mutex); // only one PGE_VideoRecording_VP8 instance may exist at once
    av_log_mutex = SDL_CreateMutex();
}

PGE_VideoRecording_VP8::~PGE_VideoRecording_VP8()
{
    SDL_DestroyMutex(av_log_mutex);
    av_log_mutex = nullptr;
}

std::unique_ptr<PGE_VideoRecording> PGE_new_recording_VP8(const PGE_VideoSpec& spec)
{
    av_log_set_callback(av_log_callback_pLog);
    std::unique_ptr<PGE_VideoRecording> ret(new PGE_VideoRecording_VP8());
    ret->spec = spec;
    // ret->spec.audio_enabled = false;
    return ret;
}
