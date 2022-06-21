//
//  ofxFFmpegRTSP.h
//
//  Created by Nick Hardeman.
//

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

}


#include <map>



//#define STREAM_FRAME_RATE 25 /* 25 images/s */
//#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
//#define IN_PIX_FMT    AV_PIX_FMT_RGB24 
#define SCALE_FLAGS SWS_BICUBIC

class ofxFFmpegRTSPServer : public ofThread {
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
    
    ofxFFmpegRTSPServer();
    ~ofxFFmpegRTSPServer();

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
