//
//  ofxFFMpegRTSP.h
//  OpenCVRTSP
//
//  Created by Nick Hardeman on 3/30/22.
//

// https://github.com/developer0hye/RTSP-Client-FFMPEG-OpenCV-ON-QT/blob/master/ffmpegdecoder.cpp

// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/demuxing_decoding.c

#pragma once
#include "ofMain.h"

extern "C"
{

#if defined(_WIN32)
    #include "libavutil/avassert.h"
    #include "libavutil/channel_layout.h"
    #include "libavutil/opt.h"
    #include "libavutil/mathematics.h"
    #include "libavutil/timestamp.h"
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
#elif defined(TARGET_LINUX)
	#include <libavutil/avutil.h>
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/timestamp.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
#else
    #include "libavutil/avassert.h"
    #include "libavutil/channel_layout.h"
    #include "libavutil/opt.h"
    #include "libavutil/mathematics.h"
    #include "libavutil/timestamp.h"
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
#endif

#if defined(_WIN32)
    #ifdef av_err2str
        #undef av_err2str

        #define av_err2str(errnum) \
        av_make_error_string(reinterpret_cast<char*>(_alloca(AV_ERROR_MAX_STRING_SIZE)),\
        AV_ERROR_MAX_STRING_SIZE, errnum)
    #endif
#endif

#if defined(TARGET_LINUX)
    #ifdef av_err2str
        #undef av_err2str
        av_always_inline std::string av_err2string(int errnum) {
    		char str[AV_ERROR_MAX_STRING_SIZE];
    		return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
	}
	#define av_err2str(err) av_err2string(err).c_str()
    #endif
#endif

}


#include <map>



//#define STREAM_FRAME_RATE 25 /* 25 images/s */
//#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
//#define IN_PIX_FMT    AV_PIX_FMT_RGB24 
#define SCALE_FLAGS SWS_BICUBIC

// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/muxing.c
class ofxFFMpegRTSPServer : public ofThread {
public:
    
    // a wrapper around a single output AVStream
    typedef struct OutputStream {
        AVStream *st;
        AVCodecContext *enc;
        
        /* pts of the next frame that will be generated */
        int64_t next_pts;
        int samples_count;
        
        AVFrame *frame;
        AVFrame *tmp_frame;
        
        AVPacket *tmp_pkt;
        
        float t, tincr, tincr2;
        
        struct SwsContext *sws_ctx;
        struct SwrContext *swr_ctx;
    } OutputStream;

    typedef struct StreamSettings {
        map<string, string> options;
        AVPixelFormat pixFormat = AV_PIX_FMT_YUV420P;
        int framerate = 30;
        string path = "";
        int64_t bitRate = -1;
        int streamId = -1;
    } StreamSettings;
    
    ofxFFMpegRTSPServer();
    ~ofxFFMpegRTSPServer();

    StreamSettings& getStreamSettings() { return mStreamSettings; }
    
    bool configure(const ofPixels& apix);
    bool init(string apath, int awidth, int aheight);
    
    bool addFrame( const ofPixels& apix );
        
    void clear();
    
    void threadedFunction() override;

    bool isStreaming() { return bSetup && isThreadRunning(); }

    //AVPixelFormat getAvPixelFormat(const ofPixels& apix);
    // implement these functions and reset if need be 
    //void setFramerate(int arate);
    //void setStreamPixelFormat(AVPixelFormat apixFmt);
    //void setInPixelFormat(AVPixelFormat apixFmt);
    
protected:
    bool add_stream(shared_ptr<OutputStream> ost, AVFormatContext *oc,
                    const AVCodec **codec,
                    enum AVCodecID codec_id);
    AVFrame* alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
    bool open_video(AVFormatContext *oc,
                    const AVCodec *codec,
                    shared_ptr<OutputStream> ost,
                    AVDictionary *opt_arg);
    
    int write_video_frame(AVFormatContext *oc, shared_ptr<OutputStream> ost);
    int write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c, AVStream *st, AVFrame *frame, AVPacket *pkt);
    AVFrame* get_video_frame(shared_ptr<OutputStream> ost);
    void close_stream(AVFormatContext *oc, shared_ptr<OutputStream> ost);
    
    AVFormatContext *pFormatCtx = nullptr;
//    AVCodecContext *pCodecCtx = nullptr;
    const AVCodec *pCodec = nullptr;
//    AVFrame *pFrame = nullptr;//, *pFrameRGB = nullptr;
//    AVPacket *packet = nullptr;
    const AVOutputFormat* pFmt;
    
//    AVPixelFormat pix_fmt;
    
//    uint8_t *outBuffer = nullptr;
//    SwsContext *imgConvertCtx = nullptr;
    
    //string mPath = "";
    bool bInited = false;
    bool bSetup = false;
    
    int mWidth = 0;
    int mHeight = 0;
    
    bool bReceivedPixels = false;
    bool bSendFrame = false;
    
    ofPixels mVideoPixelsThread;
    
    shared_ptr<OutputStream> video_st;

    StreamSettings mStreamSettings;

    //int mTargetStreamFramerate = 25;
    //AVPixelFormat mStreamPixFormat = AV_PIX_FMT_YUV420P;
    AVPixelFormat mInputPixFormat = AV_PIX_FMT_RGB24;
    int mNumInputChannels = 3;
};
