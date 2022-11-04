//
//  ofxFFMpegRTSP.h
//
//  Created by Nick Hardeman.
//

// https://github.com/developer0hye/RTSP-Client-FFMPEG-OpenCV-ON-QT/blob/master/ffmpegdecoder.cpp

// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/demuxing_decoding.c

#pragma once
#include "ofxFFmpegRTSPUtils.h"
#ifndef TARGET_OPENGLES
#include "ofxFFmpegPbo.h"
#endif

extern "C"
{
#if defined(OFX_FFMPEG_RTSP_FROM_SOURCE) && !defined(TARGET_LINUX)
    #include "libavutil/avutil.h"
    #include "libavutil/mathematics.h"
    #include "libavutil/imgutils.h"
    #include "libswscale/swscale.h"
    #include "libavutil/pixdesc.h"
    #include "libavdevice/avdevice.h"
    #include "libavcodec/avcodec.h"
#else
    #include <libavutil/avutil.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixdesc.h>
    #include <libavdevice/avdevice.h>
    #include <libavcodec/avcodec.h>
#endif

}

class ofxFFmpegRTSPClient : public ofThread {
public:
    typedef struct StreamSettings {
        map<string, string> options;
        //AVPixelFormat outPixFormat = AV_PIX_FMT_RGB24;
        //int framerate = 30;
        //string path = "";
        //int64_t bitRate = -1;
    } StreamSettings;
    
    ofxFFmpegRTSPClient();
    ~ofxFFmpegRTSPClient();

    StreamSettings& getStreamSettings() { return mStreamSettings; }
    
    bool connect( string aAddress );
    bool isConnected();
    
    void update();
    bool isFrameNew() { return bNewFrame; }
    
    const ofPixels& getPixels() const { return rPixels; }
    ofPixels& getPixels() { return rPixels; }
    ofTexture& getTexture() { return mTexture; }
    void setTextureTarget( int aTexTarget );
    int getTextureTarget() { return mTextureTarget;}
    void setUseTexture( bool ab ) { mBUseTexture=ab;}
    bool isUsingTextue() {return mBUseTexture;}
    
    float getTimeDisconnected() { return mTimeSinceConnected;}
    float getTimeSinceLastFrame() { return mTimeSinceFrame;}
    float getTimeSinceLastConnectionAttempt() { return ofGetElapsedTimef() - mLastConnectionAttemptTime; }
        
    void clear();
    
    void threadedFunction() override;
    
    void setUsePbo(bool ab) { mBUsePbo = ab; }
    bool isUsingPbo();
    double getIncomingFps() { return mFps.getFps(); }
    
protected:
    void _decode();
    int _decodePacket( AVCodecContext* adec, const AVPacket* apkt);
    
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    const AVCodec *pCodec = nullptr;
    AVFrame *pFrame = nullptr;//, *pFrameRGB = nullptr;
    AVPacket *packet = nullptr;
    
    AVPixelFormat pix_fmt = AV_PIX_FMT_NONE;
    
    uint8_t *outBuffer = nullptr;
    SwsContext *imgConvertCtx = nullptr;
    
    int videoStream = 0;
    
    string mPath = "";
    
    bool mBConnected = false;
    bool bInited = false;
    
    bool mBNewPix = false;
    bool mBUsePbo = true;
    
    int mWidth = 0;
    int mHeight = 0;
    
    //VideoFrameData mVFrameData;
    ofPixels mVideoPixelsThread, mPixels, rPixels;
    
    ofTexture mTexture;
    int mTextureTarget = GL_TEXTURE_RECTANGLE_ARB;
    bool mBUseTexture = true;
    
    bool bNewFrame = false;
    
    //std::mutex video_buffers_mutex;
    
    float mTimeSinceConnected = 0;
    float mTimeSinceFrame = 0;
    float mLastConnectionAttemptTime = 0;

    AVPixelFormat mOutPixFormat = AV_PIX_FMT_RGB24;

    StreamSettings mStreamSettings;

    ofFpsCounter mFps;
#ifndef TARGET_OPENGLES
    shared_ptr<ofxFFmpegPbo> mPbo;
#endif
};
