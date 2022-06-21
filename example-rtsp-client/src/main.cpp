#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

	ofGLFWWindowSettings wsettings;
	wsettings.setSize(1600, 800);
	wsettings.setGLVersion(4, 1);
	auto mainWin = ofCreateWindow(wsettings);
	auto app = make_shared<ofApp>();
	ofRunApp(mainWin, app);
	ofRunMainLoop();

}
