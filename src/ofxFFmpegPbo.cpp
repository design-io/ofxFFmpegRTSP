//
//  ofxFFmpegPbo.cpp
//  Final_Nick
//
//  Created by Nick Hardeman on 7/22/22.
//


#include "ofxFFmpegPbo.h"
#ifndef TARGET_OPENGLES

//--------------------------------------------------------------
ofxFFmpegPbo::ofxFFmpegPbo() {}

//--------------------------------------------------------------
ofxFFmpegPbo::~ofxFFmpegPbo() {
    if( isThreadRunning() ) {
        waitForThread(true, 10000);
    }
    if(!pboIds.empty()){
        glDeleteBuffersARB(pboIds.size(), &pboIds[0]);
    }
    pboIds.clear();
}

//--------------------------------------------------------------
void ofxFFmpegPbo::allocate(ofTexture & tex, int numPBOs, bool abThreaded){
    pboIds.resize(numPBOs);
    glGenBuffersARB(numPBOs, &pboIds[0]);
    int numChannels=1;
    switch(tex.getTextureData().glInternalFormat){
        case GL_LUMINANCE8:
            numChannels = 1;
            break;
        case GL_RGB8:
            numChannels = 3;
            break;
        case GL_RGBA8:
            numChannels = 4;
            break;
    }
    dataSize = tex.getWidth()*tex.getHeight()*numChannels;
    for(int i=0;i<(int)pboIds.size();i++){
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[i]);
        glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, dataSize, 0, GL_STREAM_DRAW_ARB);
    }
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    
    texture = tex;
    lastDataUploaded = true;
    if(abThreaded) startThread();
    else stopThread();
}

//--------------------------------------------------------------
void ofxFFmpegPbo::loadData(const ofPixels & pixels){
    if(pboIds.empty()){
        ofLogError() << "pbo not allocated";
        return;
    }
    
    if(isThreadRunning()) mutex.lock();
    
    if(!lastDataUploaded){
        mutex.unlock();
        return;
    }else{
        glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer
        mutex.unlock();
    }
    
    indexUploading = (indexUploading + 1) % pboIds.size();
    
//    cpu_ptr = pixels.getPixels();
    cpu_ptr = pixels.getData();
    
    // bind PBO to update pixel values
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[indexUploading]);
    
    // map the buffer object into client's memory
    // Note that glMapBufferARB() causes sync issue.
    // If GPU is working with this buffer, glMapBufferARB() will wait(stall)
    // for GPU to finish its job. To avoid waiting (stall), you can call
    // first glBufferDataARB() with NULL pointer before glMapBufferARB().
    // If you do that, the previous data in PBO will be discarded and
    // glMapBufferARB() returns a new allocated pointer immediately
    // even if GPU is still working with the previous data.
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, dataSize, 0, GL_STREAM_DRAW_ARB);
    gpu_ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    if(gpu_ptr) {
        // update data directly on the mapped buffer
//        std::unique_lock<std::mutex> locker(mutex);
        glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
        
        lastDataUploaded = false;
//        condition.signal();
//        mThreadCondition.notify_one();
        
        if( !isThreadRunning() ) {
            mutex.unlock();
            memcpy(gpu_ptr,cpu_ptr,dataSize);
            indexToUpdate = indexUploading;
            pboUpdated = true;
            lastDataUploaded = true;
        }
        
    }
    
}

//--------------------------------------------------------------
void ofxFFmpegPbo::threadedFunction(){
    mutex.lock();
    while(isThreadRunning()){
//        std::unique_lock<std::mutex> locker(mutex);
//        mThreadCondition.wait(mutex);
//        mThreadCondition.wait(locker);
        bool bldupdated = lastDataUploaded;
        
        mutex.unlock();
        if(!bldupdated) {
            memcpy(gpu_ptr,cpu_ptr,dataSize);
            
            mutex.lock();
            indexToUpdate = indexUploading;
            pboUpdated = true;
            lastDataUploaded = true;
        }
    }
}

//--------------------------------------------------------------
void ofxFFmpegPbo::updateTexture(){
    if( isThreadRunning() ) {
        mutex.lock();
    }
    if(pboUpdated){
        mutex.unlock();
        // bind the texture and PBO
        texture.bind();
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[indexToUpdate]);
        glTexSubImage2D(texture.getTextureData().textureTarget, 0, 0, 0, texture.getWidth(), texture.getHeight(), ofGetGLFormatFromInternal(texture.getTextureData().glInternalFormat), GL_UNSIGNED_BYTE, 0);
        texture.unbind();
        // it is good idea to release PBOs with ID 0 after use.
        // Once bound with 0, all pixel operations behave normal ways.
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
        pboUpdated = false;
    }else{
        mutex.unlock();
    }
}
#endif
