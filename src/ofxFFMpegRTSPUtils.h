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
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
#elif defined(TARGET_LINUX)
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
#else
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
#endif


}

class ofxFFMpegRTSPUtils {
public:
    static AVPixelFormat getAvPixelFormat(const ofPixels& apix);
    static ofPixelFormat getOFPixelFormat(const AVPixelFormat& afmt);
    
};
