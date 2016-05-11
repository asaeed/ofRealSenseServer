#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetWindowShape(1280, 480);
	
	mRSSDK = RSDevice::createUniquePtr();

	if (!mRSSDK->init())
	{
		ofLogError("Unable to create ofxRSSDK object");
		exit();
	}
	else
	{
		mRSSDK->initDepth(DepthRes::R200_SD, 30, false);
		mRSSDK->initRgb(RGBRes::SM, 30);
		mTexRgb.allocate(mRSSDK->getRgbWidth(), mRSSDK->getRgbHeight(), GL_RGBA);
		mTexDepth.allocate(mRSSDK->getDepthWidth(), mRSSDK->getDepthHeight(), GL_RGBA);
		mRSSDK->start();
	}

	//////////////////////////////////

	codec = ofxSquash::getCodecList()["density"];

	string text = "\
		I'm going to be squashed! \n\
		I'm going to be squashed! \n\
		I'm going to be squashed! \n\
		I'm going to be squashed! \n\
		I'm going to be squashed! \n\
		I'm going to be squashed! \n\
		";

	//COMPRESS
	string compressedText = codec.compress(text);

	//DECOMPRESS (recover the original)
	string decompressedText;
	auto uncompressedSize = codec.getUncompressedSize(compressedText);
	if (uncompressedSize == 0) {
		// Not all codecs support getting the uncompressed size from the compressed data.

		// In this case, we just have to hope we've allocated enough.
		// (don't worry, the decompression will fail safely if we don't allocate enough).
		decompressedText.resize(1000);

		codec.decompress(decompressedText, compressedText);
	}

	cout << "Original (Uncompressed)" << endl;
	cout << "=======================" << endl;
	cout << text << endl;

	cout << "Compressed" << endl;
	cout << "========" << endl;
	cout << compressedText << endl;

	cout << "Decompressed" << endl;
	cout << "============" << endl;
	cout << decompressedText << endl;

	cout << endl;
	cout << text.size() << "B uncompressed -> " << compressedText.size() << "B compressed" << endl;
	cout << endl;

	//////////////////////////////////

	//video.listDevices();
	////video.setDeviceID(3);
	//bVideoSetup = video.initGrabber(160, 120);

	//////////////////////////////////

	ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
	options.port = 9092;
	options.bUseSSL = false; // you'll have to manually accept this self-signed cert if 'true'!
	bSetup = server.setup(options);
	sendFps = 5.0f;
	sendInterval = 1.0f / sendFps;

	// this adds your app as a listener for the server
	server.addListener(this);

	ofBackground(0);
	ofSetFrameRate(30);
	font.load("myriad.ttf", 20);
}

//--------------------------------------------------------------
void ofApp::update()
{
	mRSSDK->update();

	pixels = mRSSDK->getRgbFrame();

	mTexRgb.loadData(mRSSDK->getRgbFrame(), GL_BGRA);
	mTexDepth.loadData(mRSSDK->getDepth8uFrame());

	//video.update();
	//if (bVideoSetup && video.isFrameNew()) {
		//server.sendBinary(video);
	//}

	// do this for limited fps
	if (ofGetElapsedTimef() > sendInterval) {
		int size = pixels.getWidth() * pixels.getHeight() * pixels.getNumChannels();
		cout << "width: " << pixels.getWidth() << ", height: " << pixels.getHeight() << ", num channels: " << pixels.getNumChannels() << ", size: " << size << endl;


		string sToCompress = string((char *)mRSSDK->getRgbFrame().getData());
		string sToSend = codec.compress(sToCompress);

		server.sendBinary(&sToSend[0u], sToSend.size());

		ofResetElapsedTimeCounter();
	}
	
	

}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofClear(ofColor::black);

	
	mTexRgb.draw(0, 0);
	font.drawString("Raw Color", 10, 20);
	ofPushMatrix();
	ofTranslate(640, 0);
	mTexDepth.draw(0, 0, 640, 480);
	font.drawString("Depth Pixels", 10, 20);
	ofPopMatrix();

	/////////////////////////////////

	//if (bVideoSetup) video.draw(0, 0);

	/////////////////////////////////

	if (bSetup) {
		font.drawString("WebSocket server setup at " + ofToString(server.getPort()) + (server.usingSSL() ? " with SSL" : " without SSL"), 20, 50);

		ofSetColor(150);
		//font.drawString("Click anywhere to open up client example", 20, 40);
	}
	else {
		font.drawString("WebSocket setup failed :(", 20, 20);
	}

	int x = 20;
	int y = 100;

	ofSetColor(0, 150, 0);
	font.drawString("Console", x, 80);

	ofSetColor(255);
	for (int i = messages.size() - 1; i >= 0; i--) {
		font.drawString(messages[i], x, y);
		y += 20;
	}
	if (messages.size() > NUM_MESSAGES) messages.erase(messages.begin());

	ofSetColor(150, 0, 0);
	font.drawString("Type a message, hit [RETURN] to send:", x, ofGetHeight() - 60);
	ofSetColor(255);
	font.drawString(toSend, x, ofGetHeight() - 40);

}


void ofApp::exit()
{
	mRSSDK->stop();
}

//--------------------------------------------------------------
void ofApp::onConnect(ofxLibwebsockets::Event& args) {
	cout << "on connected" << endl;
}

//--------------------------------------------------------------
void ofApp::onOpen(ofxLibwebsockets::Event& args) {
	cout << "new connection open" << endl;
	messages.push_back("New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName());

	//args.conn.send(ofToString(video.getWidth()) + ":" + ofToString(video.getHeight()) + ":" + ofToString(1));
}

//--------------------------------------------------------------
void ofApp::onClose(ofxLibwebsockets::Event& args) {
	cout << "on close" << endl;
	messages.push_back("Connection closed");
}

//--------------------------------------------------------------
void ofApp::onIdle(ofxLibwebsockets::Event& args) {
	//cout << "on idle" << endl;
}

//--------------------------------------------------------------
void ofApp::onMessage(ofxLibwebsockets::Event& args) {
	cout << "got message " << args.message << endl;

	// trace out string messages or JSON messages!
	if (!args.json.isNull()) {
		messages.push_back("New message: " + args.json.toStyledString() + " from " + args.conn.getClientName());
	}
	else {
		messages.push_back("New message: " + args.message + " from " + args.conn.getClientName());
	}

	// echo server = send message right back!
	args.conn.send(args.message);
}

//--------------------------------------------------------------
void ofApp::onBroadcast(ofxLibwebsockets::Event& args) {
	cout << "got broadcast " << args.message << endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	// do some typing!
	if (key != OF_KEY_RETURN) {
		if (key == OF_KEY_BACKSPACE) {
			if (toSend.length() > 0) {
				toSend.erase(toSend.end() - 1);
			}
		}
		else {
			toSend += key;
		}
	}
	else {
		// send to all clients
		server.send(toSend);
		messages.push_back("Sent: '" + toSend + "' to " + ofToString(server.getConnections().size()) + " websockets");
		toSend = "";
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
void ofApp::mousePressed(int x, int y, int button) {
	string url = "http";
	if (server.usingSSL()) {
		url += "s";
	}
	url += "://localhost:" + ofToString(server.getPort());
	//ofLaunchBrowser(url);
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
