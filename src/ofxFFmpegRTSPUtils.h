//
//  ofxFFmpegRTSP.h
//
//  Created by Nick Hardeman.
//

#pragma once
#include "ofMain.h"

// comment this out to load ffmpeg from system level 
#define OFX_FFMPEG_RTSP_FROM_SOURCE

extern "C"
{
#if defined(OFX_FFMPEG_RTSP_FROM_SOURCE) && !defined(TARGET_LINUX)
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
#else
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
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

class ofxFFmpegRTSPUtils {
public:
    static AVPixelFormat getAvPixelFormat(const ofPixels& apix);
    static ofPixelFormat getOFPixelFormat(const AVPixelFormat& afmt);
};
