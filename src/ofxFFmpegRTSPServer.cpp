//
//  ofxFFMpegRTSPServer.cpp
//  ofxFFMpegRTSPServer
//
//  Created by Nick Hardeman on 3/30/22.
//

#include "ofxFFmpegRTSPServer.h"
//#include "ofxFFmpegRTSPUtils.h"

//--------------------------------------------------------------
ofxFFmpegRTSPServer::ofxFFmpegRTSPServer() {
    mStreamSettings.options["f"] = "rtsp";
    mStreamSettings.options["rtsp_transport"] ="tcp";
    mStreamSettings.options["tune"] = "zerolatency";
    mStreamSettings.options["stimeout"] = "5000";
    //mStreamSettings.options["codec:v"] = "libvpx-vp9";
    // -qscale:v
    mStreamSettings.options["qscale:v"] = "1";
}

//--------------------------------------------------------------
ofxFFmpegRTSPServer::~ofxFFmpegRTSPServer() {
    clear();
}

//--------------------------------------------------------------
bool ofxFFmpegRTSPServer::configure(const ofPixels& apix) {
    mInputPixFormat = ofxFFmpegRTSPUtils::getAvPixelFormat(apix);
    if (mInputPixFormat == AV_PIX_FMT_NONE) {
        return false;
    }
    mNumInputChannels = apix.getNumChannels();
    if (mNumInputChannels == 4) {
//        mStreamSettings.pixFormat = AV_PIX_FMT_YUVA420P;
//        mStreamSettings.pixFormat = AV_PIX_FMT_ARGB;
        mStreamSettings.pixFormat = AV_PIX_FMT_YUV420P;
    } else if (mNumInputChannels == 3) {
        mStreamSettings.pixFormat = AV_PIX_FMT_YUV420P;
    } else if (mNumInputChannels == 1) {
        mStreamSettings.pixFormat = AV_PIX_FMT_YUV420P;
    }
    return true;
}

//--------------------------------------------------------------
bool ofxFFmpegRTSPServer::init(string apath, int awidth, int aheight ) {
    if( !bInited ) {
        avformat_network_init();
    }
    bInited = true;
    
    //mPath = apath;
    mStreamSettings.path = apath;
    /* allocate the output media context */
    avformat_alloc_output_context2(&pFormatCtx, NULL, "rtsp", mStreamSettings.path.c_str());
    
    mWidth = awidth;
    mHeight = aheight;
    
    if (!pFormatCtx) {
//        printf("Could not deduce output format from file extension: using MPEG.\n");
//        avformat_alloc_output_context2(&pFormatCtx, NULL, "mpeg", mPath.c_str());
    }
    if (!pFormatCtx) {
        clear();
        return false;
    }
    pFmt = pFormatCtx->oformat;
    
    av_dump_format(pFormatCtx, 0, mStreamSettings.path.c_str(), 1);
    
    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    if( !video_st ) {
        video_st = make_shared<OutputStream>();
    }
    bool have_video = false;
    if (pFmt->video_codec != AV_CODEC_ID_NONE) {
        if(add_stream(video_st, pFormatCtx, &pCodec, pFmt->video_codec)) {
//        if(add_stream(video_st, pFormatCtx, &pCodec, AV_CODEC_ID_VP9)) {
//        if(add_stream(video_st, pFormatCtx, &pCodec, AV_CODEC_ID_RAWVIDEO)) {
            have_video = true;
        }
//        have_video = 1;
//        encode_video = 1;
    }
    
    AVDictionary *opt = NULL;
    for (auto& iter : mStreamSettings.options) {
        ofLogNotice("ofxFFMpegRTSPServer :: init : setting " ) << iter.first << " : " << iter.second;
        av_dict_set(&opt, iter.first.c_str(), iter.second.c_str(), 0);
    }
    //av_dict_set(&opt, "f", "rtsp", 0);
    //av_dict_set(&opt, "rtsp_transport", "tcp", 0);
//    av_dict_set(&opt, "framerate", "30", 0);
    //av_dict_set(&opt, "tune", "zerolatency", 0);
//    av_opt_set(dec_ctx->priv_data, "tune", "zerolatency", 0);
//    av_dict_set(&opt, argv[i]+1, argv[i+1], 0);
    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    if (have_video) {
        if(!open_video(pFormatCtx, pCodec, video_st, opt)) {
            clear();
            return false;
        }
    } else {
        clear();
        return false;
    }
    
    av_dump_format(pFormatCtx, 0, mStreamSettings.path.c_str(), 1);
    
    int ret = 0;
    /* open the output file, if needed */
    if (!(pFmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&pFormatCtx->pb, mStreamSettings.path.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open '%s': %s\n", mStreamSettings.path.c_str(), av_err2str(ret));
            clear();
            return false;
        }
    }
    
    /* Write the stream header, if any. */
    ret = avformat_write_header(pFormatCtx, &opt);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
        clear();
        return false;
    }
    
    bSetup = true;
    startThread();
    return true;

}

//--------------------------------------------------------------
bool ofxFFmpegRTSPServer::addFrame( const ofPixels& apix ) {
    if (apix.getWidth() > 0 && apix.getHeight() > 0) {
        if (apix.getWidth() != mWidth || apix.getHeight() != mHeight) {
            if( lock() ) {
                bSendFrame = false;
                bReceivedPixels = false;
                mVideoPixelsThread.clear();
                unlock();
            }
            ofLogError("ofxFFMpegRTSPServer :: addFrame : pixels are different size than width or height ");
            clear();
            return false;
        }
    }


    if( isThreadRunning() ) {
        if( lock() ) {
            if( apix.isAllocated() && !bSendFrame ) {
                mVideoPixelsThread.allocate(apix.getWidth(), apix.getHeight(), apix.getPixelFormat());
                memcpy(mVideoPixelsThread.getData(), apix.getData(), apix.getTotalBytes());
                bReceivedPixels = true;
                bSendFrame = true;
            }
            unlock();
        }
    } else {
        ofLogWarning("Thread is not running");
    }
    return true;
}

//--------------------------------------------------------------
void ofxFFmpegRTSPServer::clear() {
    bSendFrame = false;
    bReceivedPixels = false;
    if( isThreadRunning() ) {
        waitForThread(true, 10000);
    }
    if( pFormatCtx && bSetup ) {
        av_write_trailer(pFormatCtx);
    }
    
    bSetup = false;
    
    if(bInited) {
        avformat_network_deinit();
    }
    bInited = false;
    
    if( video_st ) {
        close_stream(pFormatCtx, video_st );
        video_st.reset();
    }
    
    if (!(pFmt->flags & AVFMT_NOFILE)) {
        /* Close the output file. */
        avio_closep(&pFormatCtx->pb);
    }
    
    /* free the stream */
    if( pFormatCtx != nullptr ) {
        avformat_free_context(pFormatCtx);
        pFormatCtx = nullptr;
    }
}

//--------------------------------------------------------------
void ofxFFmpegRTSPServer::threadedFunction() {
    while( isThreadRunning() ) {
        if(bSetup) {
            if(bReceivedPixels) {
                bool bSendIt = false;
                if( lock() ) {
                    if( bSendFrame ) {
                        bSendIt = true;
                    }
                    unlock();
                }
                int rval = 0;
                if(bSendIt) {
                    rval = write_video_frame(pFormatCtx, video_st);
                }
                
                if( bSendIt ) {
                    if( lock() ) {
                        if( rval < 0 ) {
                            mNumConsecutiveErrors++;
                            if( mNumConsecutiveErrors > 99999 ) {
                                mNumConsecutiveErrors = 99999;
                            }
                        } else {
                            mNumConsecutiveErrors=0;
                        }
                        
                        bSendFrame = false;
                        unlock();
                    }
                }
            }
        } else {
            stopThread();
        }
    }
}

/* Add an output stream. */
//--------------------------------------------------------------
bool ofxFFmpegRTSPServer::add_stream(shared_ptr<OutputStream> ost, AVFormatContext *oc,
                       const AVCodec **codec,
                       enum AVCodecID codec_id) {
    AVCodecContext *c;
    int i;
    
    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    //https://trac.ffmpeg.org/wiki/Encode/MPEG-4
    cout << "ofxFFmpegRTSPServer :: add_stream : " << avcodec_get_name(codec_id) << endl;
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n", avcodec_get_name(codec_id));
        return false;
    }
    
    ost->tmp_pkt = av_packet_alloc();
    if (!ost->tmp_pkt) {
        fprintf(stderr, "Could not allocate AVPacket\n");
        return false;
    }
    
    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        fprintf(stderr, "Could not allocate stream\n");
        return false;
    }
    ost->st->id = oc->nb_streams-1;

    if (mStreamSettings.streamId > -1) {
        ost->st->id = mStreamSettings.streamId;
    }

    c = avcodec_alloc_context3(*codec);
    if (!c) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        return false;
    }
    ost->enc = c;
    //AVChannelLayout templ = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
//    AVChannelLayout templ;
//    templ.order = AV_CHANNEL_ORDER_NATIVE;
//    templ.nb_channels = (2);
//    //templ.u = { templ.u.mask = (AV_CH_LAYOUT_STEREO) };
//    templ.u.mask = AV_CH_LAYOUT_STEREO;

    int audioSampleRate = 44100;
    AVRational trationalAudio = { 1, audioSampleRate };
    AVRational trationalVideo = { 1, mStreamSettings.framerate };
    
    switch ((*codec)->type) {
        case AVMEDIA_TYPE_AUDIO:
            c->sample_fmt  = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
            c->bit_rate    = 64000;
            c->sample_rate = audioSampleRate;
            if ((*codec)->supported_samplerates) {
                c->sample_rate = (*codec)->supported_samplerates[0];
                for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                    if ((*codec)->supported_samplerates[i] == audioSampleRate) {
                        c->sample_rate = audioSampleRate;
                    }
                }
            }
            //av_channel_layout_copy(&c->ch_layout, &templ);
            //ost->st->time_base = (AVRational){ 1, c->sample_rate };
            ost->st->time_base = trationalAudio;
            break;
            
        case AVMEDIA_TYPE_VIDEO:
            c->codec_id = codec_id;
            c->bit_rate = 900000;
            if (mStreamSettings.bitRate > 0) {
                c->bit_rate = mStreamSettings.bitRate;
            }
            /* Resolution must be a multiple of two. */
            c->width    = mWidth;
            c->height   = mHeight;
            /* timebase: This is the fundamental unit of time (in seconds) in terms
             * of which frame timestamps are represented. For fixed-fps content,
             * timebase should be 1/framerate and timestamp increments should be
             * identical to 1. */
            ost->st->time_base = trationalVideo;
            //ost->st->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
            c->time_base       = ost->st->time_base;
            
            c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
            c->pix_fmt       = mStreamSettings.pixFormat;
            if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                /* just for testing, we also add B-frames */
                c->max_b_frames = 2;
            }
            if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
                /* Needed to avoid using macroblocks in which some coeffs overflow.
                 * This does not happen with normal video, it just happens here as
                 * the motion of the chroma plane does not match the luma plane. */
                c->mb_decision = 2;
            }
            if( c->codec_id == AV_CODEC_ID_MPEG4 ) {
                cout << "trying to set the qscale " << endl;
//                av_opt_set(c->priv_data, "preset", "slow", 0);
                av_opt_set(c->priv_data, "q", "1", 0);
            }
            break;
            
        default:
            break;
    }
    
    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    return true;
}

/**************************************************************/
/* video output */

AVFrame* ofxFFmpegRTSPServer::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height) {
    AVFrame *picture;
    int ret;
    
    picture = av_frame_alloc();
    if (!picture)
        return NULL;
    
    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;
    
    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        return NULL;
    }
    
    return picture;
}

//--------------------------------------------------------------
bool ofxFFmpegRTSPServer::open_video(AVFormatContext *oc, const AVCodec *codec,
                       shared_ptr<OutputStream> ost, AVDictionary *opt_arg) {
    int ret;
    AVCodecContext *c = ost->enc;
    AVDictionary *opt = NULL;
    
    av_dict_copy(&opt, opt_arg, 0);
    
    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
        return false;
    }
    
    /* allocate and init a re-usable frame */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return false;
    }
    
    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != mInputPixFormat) {
        ost->tmp_frame = alloc_picture(mInputPixFormat, c->width, c->height);
        
        if (!ost->tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            return false;
        }
    }
    
    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        return false;
    }
    
    return true;
}

//--------------------------------------------------------------
#if !defined(_WIN32) && !defined(TARGET_LINUX)
void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt) {
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
    
    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}
#endif

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//--------------------------------------------------------------
int ofxFFmpegRTSPServer::write_video_frame(AVFormatContext *oc, shared_ptr<OutputStream> ost) {
    return write_frame(oc, ost->enc, ost->st, get_video_frame(ost), ost->tmp_pkt);
}

//--------------------------------------------------------------
int ofxFFmpegRTSPServer::write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c,
                       AVStream *st, AVFrame *frame, AVPacket *pkt) {
    int ret;
    
    // send the frame to the encoder
    ret = avcodec_send_frame(c, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame to the encoder: %s\n", av_err2str(ret));
        return ret;
    }
    
    while (ret >= 0) {
        if( !bReceivedPixels ) {
            return -1;
        }
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            fprintf(stderr, "Error encoding a frame: %s\n", av_err2str(ret));
            break;
        }
        
        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(pkt, c->time_base, st->time_base);
        pkt->stream_index = st->index;
        
        /*cout << "ofxFFMpegRTSPServer :: pos " << pkt->pos << " pts: " << pkt->pts << " | " << ofGetFrameNum() << endl;*/

        /* Write the compressed frame to the media file. */
#if !defined(_WIN32) && !defined(TARGET_LINUX)
        //log_packet(fmt_ctx, pkt);
#endif
        ret = av_interleaved_write_frame(fmt_ctx, pkt);
        /* pkt is now blank (av_interleaved_write_frame() takes ownership of
         * its contents and resets pkt), so that no unreferencing is necessary.
         * This would be different if one used av_write_frame(). */
        if (ret < 0) {
            fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(ret));
            return ret;
        }
    }
    
    return ret == AVERROR_EOF ? 1 : 0;
}

//--------------------------------------------------------------
AVFrame* ofxFFmpegRTSPServer::get_video_frame(shared_ptr<OutputStream> ost) {
    AVCodecContext *c = ost->enc;
    
    /* check if we want to generate more frames */
//    if (av_compare_ts(ost->next_pts, c->time_base, STREAM_DURATION, (AVRational){ 1, 1 }) > 0) {
//        return NULL;
//    }
    
    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally; make sure we do not overwrite it here */
    if (av_frame_make_writable(ost->frame) < 0) {
        return nullptr;
    }
    
//    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
    if( c->pix_fmt != mInputPixFormat) {
        /* as we only generate a YUV420P picture, we must convert it
         * to the codec pixel format if needed */
        if (!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(
                c->width, c->height,
                mInputPixFormat,
                c->width, c->height,
                c->pix_fmt,
                SCALE_FLAGS, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                fprintf(stderr, "Could not initialize the conversion context\n");
                return nullptr;
            }
        }
        //fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);
//        sws_scale(ost->sws_ctx, (const uint8_t * const *) ost->tmp_frame->data,
//                  ost->tmp_frame->linesize, 0, c->height, ost->frame->data,
//                  ost->frame->linesize);
        
        if( lock() ) {
            int linesize[4] = { mNumInputChannels * mWidth,0,0,0};
            uint8_t * video_pixel_data = mVideoPixelsThread.getData();
            uint8_t * dst[4] = {video_pixel_data, 0, 0, 0};
            
            sws_scale(ost->sws_ctx, dst,
                      linesize, 0, c->height,
                      ost->frame->data, ost->frame->linesize);
            
            unlock();
        } else {
            return nullptr;
        }
        
    } else {
        //fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);
        if( lock() ) {
            int linesize[4] = { mNumInputChannels * mWidth,0,0,0};
            uint8_t * video_pixel_data = mVideoPixelsThread.getData();
            uint8_t * dst[4] = {video_pixel_data, 0, 0, 0};
            
            sws_scale(ost->sws_ctx, dst,
                      linesize, 0, c->height,
                      ost->frame->data, ost->frame->linesize);
            
            unlock();
        } else {
            return nullptr;
        }
        
    }
    
    ost->frame->pts = ost->next_pts++;
    
    return ost->frame;
}

//--------------------------------------------------------------
void ofxFFmpegRTSPServer::close_stream(AVFormatContext *oc, shared_ptr<OutputStream> ost) {
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    av_packet_free(&ost->tmp_pkt);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}
