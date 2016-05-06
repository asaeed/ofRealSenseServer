#pragma once

#include "ofMain.h"
#include "ofxRSSDKv2.h"
#include "ofxLibwebsockets.h"
#include "ofxSquash.h"

#define NUM_MESSAGES 30

using namespace ofxRSSDK;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

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

		// websockets
		ofxLibwebsockets::Server server;
		bool bSetup;
		vector<string> messages;
		string toSend;
		void onConnect(ofxLibwebsockets::Event& args);
		void onOpen(ofxLibwebsockets::Event& args);
		void onClose(ofxLibwebsockets::Event& args);
		void onIdle(ofxLibwebsockets::Event& args);
		void onMessage(ofxLibwebsockets::Event& args);
		void onBroadcast(ofxLibwebsockets::Event& args);
		
	private:
		ofTrueTypeFont font;

		// realsense sensor
		RSDevicePtr mRSSDK;
		ofTexture mTexRgb, mTexDepth;
		bool isReady;
		ofPixels pixels;

		// temp video grabber
		//bool bVideoSetup;
		//ofVideoGrabber video;
};
