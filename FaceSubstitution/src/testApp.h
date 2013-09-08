#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"
#include "FaceLoader.h"
#include "ofxFPS.h"
#include "AutoExposure.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void onNewFrame(ofPixels & pixels);
	
	void keyPressed(int key);

	ofxFaceTrackerThreaded camTracker;
	ofVideoGrabber cam;
	ofTexture camTex;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo;

	ofVboMesh camMesh, camMeshWithPicTexCoords;

	FaceLoader faceLoader;
	int lastFound;
	bool faceChanged;

	ofMutex mutex;
	Poco::Condition condition;
	bool refreshOnNewFrameOnly;

	ofxFPS camFPS, camRealFPS;
	AutoExposure autoExposure;

	ofPixels grayPixels;
	ofRectangle autoExposureBB;
	int numCamFrames;
};
