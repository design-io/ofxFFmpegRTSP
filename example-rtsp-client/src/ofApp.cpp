#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(120);
    ofSetVerticalSync(false);
    
    params.setName("RTSP_Client_Settings");
    
    params.add(mConnectIPAddress.set("ConnectIpAddress", "127.0.0.1"));
    params.add(mConnectPort.set("ConnectPort", 8554, 1, 10000));
    params.add(mStreamName.set("ConnectStream", "mystream"));
    params.add(mReconnectFreq.set("ReconnectFreq", 10, 0.5, 60));
    
    gui.setup(params);
    
    gui.add(mStatusParam.set("Status", "Disconnected"));
    
    ofAddListener(params.parameterChangedE(), this, &ofApp::onParamChanged);
    
    gui.loadFromFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::update() {
    
    if (!ffmpegRTSP) {
        ffmpegRTSP = make_shared<ofxFFmpegRTSPClient>();
        ofLogNotice("ofApp :: going to connect rtsp");
        if (ffmpegRTSP->connect(getConnectString())) {
            
        }
    }
    
    if (ffmpegRTSP) {
        if (mTimeResetFromParamChange > 0.0f) {
            float delta = ofClamp(ofGetLastFrameTime(), 1.f / 1000.f, 1.f / 5.f);
            mTimeResetFromParamChange -= delta;
            if (mTimeResetFromParamChange <= 0.0f) {
                ofLogNotice("TextureRTSPReceiver :: clearing from param change");
                ffmpegRTSP.reset();
                mTimeResetFromParamChange = -10.;
            }
        }
    }
    
    if(ffmpegRTSP) {
        ffmpegRTSP->update();
        if( ffmpegRTSP->isFrameNew() ) {
            // some fancy code here //
        }
        // haven't received a frame in a while, so assume we are disconnected //
        if (ffmpegRTSP->isConnected() && ffmpegRTSP->getTimeSinceLastFrame() > 15.0f) {
            ffmpegRTSP->clear();
        }
        
        if (!ffmpegRTSP->isConnected() && ffmpegRTSP->getTimeDisconnected() > mReconnectFreq) {
            if (ffmpegRTSP->getTimeSinceLastConnectionAttempt() > mReconnectFreq) {
                ffmpegRTSP->connect(getConnectString());
            }
        }
        
        if (ffmpegRTSP->isConnected()) {
            mStatusParam = "Connected";
        } else {
            mStatusParam = "Disconnected";
            if (ffmpegRTSP->getTimeDisconnected() > mReconnectFreq) {
                float recon = ofClamp(mReconnectFreq - ffmpegRTSP->getTimeSinceLastConnectionAttempt(), 0, mReconnectFreq);
                mStatusParam = "Reconnecting in " + ofToString(recon, 0);
            }
        }
    } else {
        mStatusParam = "Disconnected";
    }

}

//--------------------------------------------------------------
void ofApp::draw() {
    float guir = gui.getPosition().x + gui.getWidth() + 10;
    auto srect = ofRectangle(guir, 10, ofGetWidth() - guir-10, ofGetHeight()-20);
    ofRectangle trect(0, 0, ofGetWidth(), ofGetHeight());

    auto& rtex = ffmpegRTSP->getTexture();
    
    if (rtex.isAllocated()) {
        trect = ofRectangle(0, 0, rtex.getWidth(), rtex.getHeight());
    }
    trect.scaleTo(srect);
    if (rtex.isAllocated()) {
        ofSetColor(255);
        rtex.draw(trect);
    }
    ofNoFill();
    ofDrawRectangle(trect);
    ofFill();


    ofSetColor( ofColor::limeGreen );
    if( !ffmpegRTSP->isConnected() ) {
        ofSetColor( 255 * (sin(ofGetElapsedTimef() * 9.6f) * 0.5f + 0.5f), 0, 0 );
    }
    ofDrawCircle( srect.x+20, srect.y+ 48, 6 );
    string fpsString = "0";
    if(ffmpegRTSP->isConnected() ) {
        fpsString = ofToString(ffmpegRTSP->getIncomingFps(), 0);
    }
    string texDimsString = "0x0";
    if(ffmpegRTSP->isConnected() && ffmpegRTSP->getTexture().isAllocated() ) {
        texDimsString = ofToString(ffmpegRTSP->getTexture().getWidth(), 0) + " x " + ofToString(ffmpegRTSP->getTexture().getHeight(), 0);
    }
    ofDrawBitmapStringHighlight("Incoming Frame fps: "+fpsString+"\n"+
                                "App fps: "+ofToString(ofGetFrameRate(),0)+"\n"+
                                "Video Resolution: " + texDimsString
                                , srect.x+40, srect.y+40 );
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::onParamChanged(ofAbstractParameter& aparam) {
    if (ofIsStringInString(aparam.getName(), "Connect")) {
        mTimeResetFromParamChange = 2.0;
    }
}

//--------------------------------------------------------------
string ofApp::getConnectString() {
    return "rtsp://" + mConnectIPAddress.get() + ":" + ofToString(mConnectPort, 0) + "/" + mStreamName.get();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if( key == 'F' ) {
        ofToggleFullscreen();
    }
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
