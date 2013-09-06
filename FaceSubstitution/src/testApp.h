#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"
#include "FaceLoader.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);

	ofxFaceTrackerThreaded camTracker;
	ofVideoGrabber cam;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo;

	ofVboMesh camMesh;

	FaceLoader faceLoader;
	int lastFound;
	bool faceChanged;

	ofPixels gray;
};
