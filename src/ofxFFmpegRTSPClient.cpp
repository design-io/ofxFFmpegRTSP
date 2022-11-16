//
//  ofxFFMpegRTSP.cpp
//
//  Created by Nick Hardeman.
//

#include "ofxFFmpegRTSPClient.h"
//#include "ofxFFmpegRTSPUtils.h"

//--------------------------------------------------------------
ofxFFmpegRTSPClient::ofxFFmpegRTSPClient() {
    mStreamSettings.options["rtsp_transport"] = "tcp";
    mStreamSettings.options["max_delay"] = "100";
    mStreamSettings.options["stimeout"] = "5000";
    
#ifdef TARGET_OPENGLES
    mBUsePbo = false;
#endif
}

//--------------------------------------------------------------
ofxFFmpegRTSPClient::~ofxFFmpegRTSPClient() {
    clear();
}

//--------------------------------------------------------------
bool ofxFFmpegRTSPClient::connect( string aAddress ) {
    clear();
    
    mPath = aAddress;
    if( !bInited ) {
        //avformat_network_init();
    }
    bInited = true;
    
    mLastConnectionAttemptTime = ofGetElapsedTimef();
    
//    av_register_all();
    
    mBConnected = false;
    
    int ret = -1;
    
//    if( pFormatCtx != nullptr) {
//        avformat_free_context(pFormatCtx);
//        pFormatCtx = nullptr;
//    }
    
    cout << __FUNCTION__ << " about to init context " << endl;
    
//    pFormatCtx = avformat_alloc_context();
    
    cout << __FUNCTION__ << " just inited the context" << endl;
    
    // https://ffmpeg.org/ffmpeg-protocols.html#rtsp
    AVDictionary *avdic=NULL;
    //char option_key[]="rtsp_transport";
    //char option_value[]="tcp";
    //av_dict_set(&avdic,option_key,option_value,0);
    //
    //char option_key2[]="max_delay";
    //char option_value2[]="100";
    //av_dict_set(&avdic,option_key2,option_value2,0);

    //// -stimeout 3000
    //av_dict_set(&avdic, "stimeout", "5000", 0);
    for (auto& iter : mStreamSettings.options) {
        ofLogNotice("ofxFFMpegRTSPClient :: init : setting ") << iter.first << " : " << iter.second;
        av_dict_set(&avdic, iter.first.c_str(), iter.second.c_str(), 0);
    }
    
//    char option_key3[]="tune";
//    char option_value3[]="zerolatency";
//    av_dict_set(&avdic,option_key3,option_value3,0);
    cout << __FUNCTION__ << " about to open the format " << mPath << endl;
    ret = avformat_open_input(&pFormatCtx, mPath.c_str(), NULL, &avdic);
//    ret = avformat_open_input(&pFormatCtx, mPath.c_str(), NULL, NULL);
    cout << __FUNCTION__ << " just opened the format" << endl;
    if (ret != 0) {
        std::cout << "can't open the file." << std::endl;
        fprintf(stderr, "Error opening the file (%s)\n", av_err2str(ret));
//        mBConnected = false;
//        avformat_free_context(pFormatCtx);
        clear();
        return false;
    }
    
    ret = avformat_find_stream_info(pFormatCtx, &avdic);
    if (ret < 0) {
        std::cout << "can't find stream infomation" << std::endl;
//        avformat_free_context(pFormatCtx);
        clear();
        return false;
    }
    
    videoStream = -1;
    
//    AVCodecContext *codec_ctx = nullptr;
//    if( pCodecCtx != nullptr ) {
//
//    }

    ofLogNotice("ofxFFMpegRTSPClient :: number of streams: ") << pFormatCtx->nb_streams;
    
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
        AVStream *stream = pFormatCtx->streams[i];
        if( stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ) {
            videoStream = i;
        }
//        const AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
//        AVCodecContext *codec_ctx;
//        if (!dec) {
//            av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
//            //return AVERROR_DECODER_NOT_FOUND;
////            return false;
//            continue;
//        }
//        codec_ctx = avcodec_alloc_context3(dec);
//        if (!codec_ctx) {
//            av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
////            return AVERROR(ENOMEM);
////            return false;
//            continue;
//        }
//        int ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
//        if (ret < 0) {
//            av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
//                   "for stream #%u\n", i);
////            return ret;
//            continue;
//        }
//
//        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO ) {
//            videoStream = i;
//        }
        
//        if( pFormatCtx->streams[i]->codecpar ) {
//
//        }
//        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
//            videoStream = i;
//        }
    }
    
    videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO,videoStream, -1, NULL, 0);
    
    if (videoStream == -1) {
        std::cout << "can't find a video stream" << std::endl;
//        mBConnected = false;
//        avformat_free_context(pFormatCtx);
        clear();
        return false;
    }
    
    ofLogNotice("Found a video stream at index: ") << videoStream;
    
    AVStream *stream = pFormatCtx->streams[videoStream];
//    const AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
    pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
//    AVCodecContext *codec_ctx;
    if (!pCodec) {
        av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", videoStream);
        //return AVERROR_DECODER_NOT_FOUND;
        clear();
        return false;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", videoStream);
//            return AVERROR(ENOMEM);
        clear();
        return false;
    }
    ret = avcodec_parameters_to_context(pCodecCtx, stream->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
               "for stream #%u\n", videoStream);
//            return ret;
        clear();
        return false;
    }
    
//    pCodecCtx = pFormatCtx->streams[videoStream]->codecpar->code;
//    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    
    pCodecCtx->bit_rate = 0;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 10;
    pCodecCtx->frame_number = 1;
//    pCodecCtx->framerate.num = 1;
//    pCodecCtx->framerate.den = 30;
    
    pCodecCtx->codec_id = pCodec->id;
    pCodecCtx->workaround_bugs   = 1;
    
//    cout << "Codec framerate: " << pCodecCtx->framerate.num << endl;
    
    mWidth = pCodecCtx->width;
    mHeight = pCodecCtx->height;

    ofLogNotice("ofxFFMpegRTSPClient :: init : stream width and height ") << mWidth << " x " << mHeight << " half height: " << (mHeight/2);
    
    av_dump_format(pFormatCtx, videoStream, aAddress.c_str(), 0);
    
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        std::cout << "can't open a codec" << std::endl;
        clear();
        return false;
    }
    
    pFrame = av_frame_alloc();
    if (!pFrame) {
        fprintf(stderr, "Could not allocate frame\n");
//        ret = AVERROR(ENOMEM);
        clear();
        return false;
    }
    
    pix_fmt = pCodecCtx->pix_fmt;

    mOutPixFormat = pix_fmt;
    // now lets figure out how to convert to a more readable format //
    if (pix_fmt == AV_PIX_FMT_YUV420P) {
        mOutPixFormat = AV_PIX_FMT_RGB24;
    } else if (pix_fmt == AV_PIX_FMT_BGR24 ) {
        mOutPixFormat = AV_PIX_FMT_RGB24;
    } else if (pix_fmt == AV_PIX_FMT_ARGB) {
        mOutPixFormat = AV_PIX_FMT_RGBA;
    }else{
        ofLogError(__FUNCTION__) << "can't read format " << pix_fmt << " returning ";
        clear();
        return false; 
    }

    if( sws_isSupportedInput(pix_fmt) < 1 ) {
        ofLogError(__FUNCTION__) << " sws does not support input format " << pix_fmt << " returning ";
        clear();
        return false; 
    }

    if(sws_isSupportedOutput(mOutPixFormat) < 1) {
        ofLogError(__FUNCTION__) << " sws does not support output format " << mOutPixFormat << " returning ";
        clear();
        return false; 
    }
    
    // SWS_FAST_BILINEAR SWS_BICUBIC
    imgConvertCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                   pCodecCtx->width, pCodecCtx->height, mOutPixFormat,
                                   SWS_FAST_BILINEAR, NULL, NULL, NULL);    
    if (!imgConvertCtx) {
        fprintf(stderr,
                "Impossible to create scale context for the conversion "
                "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                av_get_pix_fmt_name(pCodecCtx->pix_fmt), pCodecCtx->width, pCodecCtx->height,
                av_get_pix_fmt_name(mOutPixFormat), pCodecCtx->width, pCodecCtx->height );
        ret = AVERROR(EINVAL);
        clear();
        return false;
    }
    
    
//    int av_image_get_buffer_size(enum AVPixelFormat pix_fmt, int width, int height, int align);
    /* buffer is going to be written to rawvideo file, no alignment */
//    if ((ret = av_image_alloc(dst_data, dst_linesize,
//                              pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, 1)) < 0) {
//        fprintf(stderr, "Could not allocate destination image\n");
//        return false;
//    }
    
//    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
//    outBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
//    avpicture_fill((AVPicture *) pFrameRGB, outBuffer, AV_PIX_FMT_BGR24,
//                   pCodecCtx->width, pCodecCtx->height);
    
//    packet = (AVPacket *) malloc(sizeof(AVPacket));
//    av_new_packet(packet, pCodecCtx->width * pCodecCtx->height);
    packet = av_packet_alloc();
    if (!packet) {
        fprintf(stderr, "Could not allocate packet\n");
        int ret = AVERROR(ENOMEM);
        clear();
        return false;
    }
    
    mBConnected = true;
    mTimeSinceFrame = 0;
    mTimeSinceConnected = 0;
    
    //start reading packets from stream and write them to file
//    av_read_play(pFormatCtx);    //play RTSP
    
    startThread();
    return true;
}

//--------------------------------------------------------------
bool ofxFFmpegRTSPClient::isConnected() {
    return mBConnected;
}

//--------------------------------------------------------------
void ofxFFmpegRTSPClient::update() {
    float delta = ofClamp(ofGetLastFrameTime(), 1.f/1000.f, 1.f/5.f);
    bNewFrame = false;
    if( lock() ) {
        if( mBNewPix ) {
            bNewFrame = true;
            rPixels = mPixels;
            if (mWidth < 1 || mHeight < 1 || mWidth != rPixels.getWidth() || mHeight != rPixels.getHeight() || !mBUseTexture) {
                if (mTexture.isAllocated()) {
                    mTexture.clear();
                }
                #ifndef TARGET_OPENGLES
                if( mPbo ) {
                    mPbo.reset();
                }
                #endif
            } else {
                if (mTexture.getWidth() != mWidth || mTexture.getHeight() != mHeight) {
                    mTexture.allocate(rPixels, (mTextureTarget == GL_TEXTURE_RECTANGLE_ARB) );
                    #ifndef TARGET_OPENGLES
                    if( mPbo ) {
                        mPbo.reset();
                    }
                    
                    if( !mPbo && isUsingPbo() ) {
                        mPbo = make_shared<ofxFFmpegPbo>();
                        mPbo->allocate(mTexture, 3, true);
                    }
                    #endif
                }
                
                #ifndef TARGET_OPENGLES
                if( mPbo ) {
                    mPbo->loadData(rPixels);
                } else {
                    mTexture.loadData(rPixels);
                }
                #else
                mTexture.loadData(rPixels);
                #endif                
            }

            //cout << "ofxFFMpegRTSPClient :: update : mHeight: " << mHeight << " mTexture height: " << mTexture.getHeight() << " pix height: " << mPixels.getHeight() << " | " << ofGetFrameNum() << endl;
            mBNewPix = false;
            mFps.newFrame();
        }
        unlock();
    }
    
#ifndef TARGET_OPENGLES
    if( mPbo && mBUseTexture ) {
        mPbo->updateTexture();
    }
#endif
    
    if( bNewFrame ) {
        mTimeSinceFrame = 0;
    } else {
        mTimeSinceFrame += delta;
    }
    if( isConnected() ) {
        mTimeSinceConnected = 0;
    } else {
        mTimeSinceConnected += delta;
    }
    mFps.update();
}

//--------------------------------------------------------------
void ofxFFmpegRTSPClient::clear() {
    mBConnected = false;

    if( isThreadRunning() ) {
        waitForThread(true, 10000);
    }
    mBNewPix = false;

    if( imgConvertCtx ) {
        sws_freeContext(imgConvertCtx);
        imgConvertCtx = nullptr;
    }
    if( packet != nullptr ) {
        av_packet_free(&packet);
        packet = nullptr;
    }
    
    if( pFrame != nullptr ) {
        av_frame_free(&pFrame);
        pFrame = nullptr;
    }
    
    if( pCodecCtx != nullptr ) {
        avcodec_close(pCodecCtx);
        if(pCodecCtx) {
            delete pCodecCtx;
        }
        pCodecCtx = nullptr;
    }
    
    if( pFormatCtx != nullptr) {
        //avformat_close_input(pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = nullptr;
    }
    
    mWidth = mHeight = 0;
    mOutPixFormat = AV_PIX_FMT_RGB24;
    bNewFrame = false;
    mBNewPix = false;
    
    if(bInited) {
        //avformat_network_deinit();
    }
    bInited = false;
}

//--------------------------------------------------------------
void ofxFFmpegRTSPClient::setTextureTarget( int aTexTarget ) {
    if( mTexture.isAllocated() ) {
        if( mTexture.texData.textureTarget != aTexTarget || !mBUseTexture ) {
            mTexture.clear();
        }
    }
    mTextureTarget = aTexTarget;
}

//--------------------------------------------------------------
void ofxFFmpegRTSPClient::threadedFunction() {
    while( isThreadRunning() ) {
        if( isConnected() ) {
            _decode();
        } else {
            //waitForThread(true, 10000);
            stopThread();
        }
    }
}

//--------------------------------------------------------------
void ofxFFmpegRTSPClient::_decode() {
//    std::chrono::milliseconds duration(5);
    
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if( !mBConnected ) {
            break;
        }
        if (packet->stream_index == videoStream) {
//            int got_picture = 0;
            int ret = -1;
//            int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
//            int ret = -1;
            ret = _decodePacket(pCodecCtx, packet);
//            av_packet_unref(packet);
            
//            ofLogNotice("ofxFFMpegRTSP :: received a new frame: | ") << ofGetFrameNum();
            
            if (ret < 0) {
                std::cout << "decode error.\n" << std::endl;
                break;
            }
        }
        av_packet_unref(packet);
//        av_packet_free(&packet);
        //std::this_thread::sleep_for(duration);
//        sleep(5);
        ofSleepMillis(5);
    }
    /* flush the decoders */
//    if(pCodecCtx != nullptr) {
//        _decodePacket(pCodecCtx, nullptr );
//    }
//    mBConnected = false;
}

//--------------------------------------------------------------
int ofxFFmpegRTSPClient::_decodePacket( AVCodecContext* adec, const AVPacket* apkt) {
    
    if( !mBConnected ) {
        return -1;
    }
    
    int ret = 0;

    //adec->skip_alpha

    //cout << "ofxFFMpegRTSPClient :: pos " << apkt->pos << " pts: " << apkt->pts << " | " << ofGetFrameNum() << endl;
    //cout << "ofxFFMpegRTSPClient :: real time: " << pFormatCtx->start_time_realtime << " start time: " << pFormatCtx->start_time << (pFormatCtx->start_time_realtime == AV_NOPTS_VALUE ? "good":"bad") << " | " << ofGetFrameNum() << endl;
    
    // submit the packet to the decoder
    ret = avcodec_send_packet(adec, apkt);
    if (ret < 0) {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }
    
    //ofLogNotice("ofxFFMpegRTSP :: _decodePacket: | ") << ofGetFrameNum();
    
//    // get all the available frames from the decoder
    while (ret >= 0) {
        if( !mBConnected ) {
            ret = -1;
            return ret;
            //break;
        }
        
        ret = avcodec_receive_frame(adec, pFrame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
                //fprintf(stderr, "_decodePacket :: AVERROR_EOF || AVERROR(EAGAIN) \n");
                return 0;
            }

            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        }

        // write the frame data to output file
        if (adec->codec->type == AVMEDIA_TYPE_VIDEO) {
            //cout << "ofxFFMpegRTSPClient :: " << pFrame->best_effort_timestamp << " | " << ofGetFrameNum() << endl;

            //ret = output_video_frame(pFrame);
            if( imgConvertCtx != nullptr ) {
                if (pFrame->width != mWidth || pFrame->height != mHeight ||
                    pFrame->format != pix_fmt) {
                    
                    /* To handle this change, one could call av_image_alloc again and
                     * decode the following frames into another rawvideo file. */
                    fprintf(stderr, "Error: Width, height and pixel format have to be "
                            "constant in a rawvideo file, but the width, height or "
                            "pixel format of the input video changed:\n"
                            "old: width = %d, height = %d, format = %s\n"
                            "new: width = %d, height = %d, format = %s\n",
                            mWidth, mHeight, av_get_pix_fmt_name(pix_fmt),
                            pFrame->width, pFrame->height,
                            av_get_pix_fmt_name((AVPixelFormat)pFrame->format));
                    return -1;
                    
                }
                
                if( !mVideoPixelsThread.isAllocated() || mVideoPixelsThread.getWidth() != mWidth || mVideoPixelsThread.getHeight() != mHeight ) {
                    auto ofpixfmt = ofxFFmpegRTSPUtils::getOFPixelFormat(mOutPixFormat);
                    if (ofpixfmt != OF_PIXELS_UNKNOWN) {
                        mVideoPixelsThread.allocate(mWidth, mHeight, ofpixfmt);
                    } else {
                        ofLogError("ofxFFMpegRTSPClient :: error finding of format for ") << av_get_pix_fmt_name(mOutPixFormat);
                        return -1;
                    }
                }
                
                int linew = mVideoPixelsThread.getNumChannels()*mWidth;
                int linesize[4] = { linew,0,0,0};
                uint8_t * video_pixel_data = mVideoPixelsThread.getData();
                uint8_t * dst[4] = {video_pixel_data, 0, 0, 0};
                
                ret = sws_scale(imgConvertCtx,
                      (uint8_t const * const *) pFrame->data,
                                pFrame->linesize,
                                0, pCodecCtx->height,
                                dst,
                                linesize
                                );
                if( ret < 0 ) {
                    
                    fprintf(stderr, "imgConvertCtx error (%s)\n", av_err2str(ret));
                    return -1;
                }
                //ofLogNotice("ofxFFMpegRTSP :: received a new frame: | ") << ofGetFrameNum();
                if( lock() ) {
//                    mPixels.copyFrom( mVideoPixelsThread );
                    if(mVideoPixelsThread.isAllocated()) {
                        mPixels.allocate(mVideoPixelsThread.getWidth(), mVideoPixelsThread.getHeight(), mVideoPixelsThread.getPixelFormat());
                        memcpy(mPixels.getData(), mVideoPixelsThread.getData(), mVideoPixelsThread.getTotalBytes());
                    }
                    mBNewPix = true;
                    unlock();
                }
            }
        } else {
            //ret = output_audio_frame(pFrame);
        }

        av_frame_unref(pFrame);
        if (ret < 0) {
            return ret;
        }
    }
    
    return 0;
}

//-----------------------------------------
bool ofxFFmpegRTSPClient::isUsingPbo() {
#ifdef TARGET_OPENGLES
    mBUsePbo = false;
#endif
    
    return mBUsePbo;
}
