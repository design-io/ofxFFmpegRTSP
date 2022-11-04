//
//  ofxFFmpegPbo.h
//  Final_Nick
//
//  Created by Nick Hardeman on 7/22/22.
//Based on Arturo Castro's ofxPbo
// https://github.com/arturoc/ofxPBO
//

#pragma once
#ifndef TARGET_OPENGLES
#include "ofMain.h"

class ofxFFmpegPbo : public ofThread {
public:
    ofxFFmpegPbo();
    virtual ~ofxFFmpegPbo();
    
    void allocate(ofTexture & tex, int numPBOs, bool abThreaded);
    void loadData(const ofPixels & pixels );
    void updateTexture();
    void threadedFunction();
    
private:
    ofTexture texture;
    vector<GLuint> pboIds;
    size_t indexUploading = 0, indexToUpdate = 0;
    unsigned int dataSize = 0;
//    Poco::Condition condition;
    std::condition_variable mThreadCondition;
    GLubyte* gpu_ptr = nullptr;
    const unsigned char* cpu_ptr = nullptr;
    bool pboUpdated = false;
    bool lastDataUploaded = false;
    
};
#endif
