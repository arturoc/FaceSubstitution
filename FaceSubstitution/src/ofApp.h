#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"
#include "FaceLoader.h"
#include "AutoExposure.h"
#include "Gui.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void onNewFrame(ofPixels & pixels);
    void allocate(int & numRotations);

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

    std::mutex mutex;
    std::condition_variable condition;
	bool refreshOnNewFrameOnly;

    AutoExposure autoExposure;

    ofPixels camRotated, grayPixels;
	ofRectangle autoExposureBB;
	int numCamFrames;

    Gui gui;
};
