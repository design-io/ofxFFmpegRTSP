#pragma once

#include "ofxGui.h"
#include "ofxFFmpegRTSPClient.h"

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void onParamChanged(ofAbstractParameter& aparam);
    string getConnectString();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    shared_ptr<ofxFFmpegRTSPClient> ffmpegRTSP;
    
    ofParameterGroup params;
    ofParameter<string> mConnectIPAddress;
    ofParameter<int> mConnectPort;
    ofParameter<string> mStreamName;
    ofParameter<float> mReconnectFreq;
    ofParameter<string> mStatusParam;
    
    ofxPanel gui;
    
    float mTimeResetFromParamChange = -5;
};
