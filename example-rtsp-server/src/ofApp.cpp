#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(120);
    mGrabber.setup(1920, 1080);
    
    params.setName("RTSP_Server_Settings");
    
    params.add(mConnectPort.set("ConnectPort", 8554, 1, 10000));
    params.add(mStreamName.set("ConnectStream", "mystream"));
    params.add(mBitRate.set("BitRateKbs", 1000, 1, 10000));
    
    gui.setup(params);
    
    //gui.add(mStatusParam.set("Status", "Disconnected"));
    
    ofAddListener(params.parameterChangedE(), this, &ofApp::onParamChanged);
    
    gui.loadFromFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::update() {
    if( !ffmpegRTSP ) {
        ffmpegRTSP = make_shared<ofxFFmpegRTSPServer>();
        ffmpegRTSP->getStreamSettings().bitRate = mBitRate * (int)1000;
        ffmpegRTSP->init(getConnectString(), mGrabber.getWidth(), mGrabber.getHeight() );
    }
    
    mGrabber.update();
    
    if(mGrabber.isFrameNew() ) {
        if( ffmpegRTSP ) {
            ffmpegRTSP->addFrame(mGrabber.getPixels());
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    
    ofSetColor( 255 );
    auto& tex = mGrabber.getTexture();
    if( tex.isAllocated() ) {
        ofRectangle trect( 0, 0, tex.getWidth(), tex.getHeight() );
        auto srect = ofRectangle(0,0,ofGetWidth(), ofGetHeight());
        trect.scaleTo(srect);
        tex.draw( trect );
    }
    
    if( ffmpegRTSP ) {
        string infoStr = "Streaming: ";
        infoStr += (ffmpegRTSP->isStreaming() ? "yes" : "no");
        ofDrawBitmapStringHighlight( infoStr, 40, ofGetHeight()-64 );
    }
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::onParamChanged(ofAbstractParameter& aparam) {
        //mTimeResetFromParamChange = 2.0;
        if(ffmpegRTSP) {
            ffmpegRTSP.reset();
        }
}

//--------------------------------------------------------------
string ofApp::getConnectString() {
    return "rtsp://127.0.0.1:" + ofToString(mConnectPort, 0) + "/" + mStreamName.get();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
