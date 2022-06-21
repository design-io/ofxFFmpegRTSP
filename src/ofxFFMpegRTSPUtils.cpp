//
//  ofxFFmpegRTSPUtils.h
//
//  Created by Nick Hardeman.
//
#include "ofxFFmpegRTSPUtils.h"

//--------------------------------------------------------------
AVPixelFormat ofxFFmpegRTSPUtils::getAvPixelFormat(const ofPixels& apix) {
    auto offmt = apix.getPixelFormat();
    if (offmt == OF_PIXELS_RGB) {
        return AV_PIX_FMT_RGB24;
    } else if (offmt == OF_PIXELS_BGR) {
        return AV_PIX_FMT_BGR24;
    } else if (offmt == OF_PIXELS_GRAY) {
        return AV_PIX_FMT_GRAY8;
    } else if (offmt == OF_PIXELS_RGBA) {
        return AV_PIX_FMT_RGBA;
    }
    return AV_PIX_FMT_NONE;
}

//--------------------------------------------------------------
ofPixelFormat ofxFFmpegRTSPUtils::getOFPixelFormat(const AVPixelFormat& afmt) {
    // now lets figure out how to convert to a more readable format //
    if (afmt == AV_PIX_FMT_YUV420P) {
        return OF_PIXELS_RGB;
    } else if (afmt == AV_PIX_FMT_GRAY8) {
        return OF_PIXELS_GRAY;
    } else if (afmt == AV_PIX_FMT_RGBA) {
        return OF_PIXELS_RGBA;
    } else if (afmt == AV_PIX_FMT_RGB24) {
        return OF_PIXELS_RGB;
    } else if (afmt == AV_PIX_FMT_BGR24) {
        return OF_PIXELS_BGR;
    }
    return OF_PIXELS_UNKNOWN;
}
