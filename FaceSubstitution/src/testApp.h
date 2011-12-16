#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"
#include <gst/app/gstappsrc.h>

#include "BlinkDetector.h"

#include "OldBW.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
	void loadFace(string face);
	
	void threadedUpdate(ofEventArgs & args);

	void allocateGstVirtualCamera();
	void updateGstVirtualCamera();

	void keyPressed(int key);

	ofxFaceTrackerThreaded camTracker;
	ofVideoGrabber cam;
	ofVideoPlayer vid;
	
	ofxFaceTracker srcTracker;
	ofImage src;
	vector<ofVec2f> srcPoints;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo, maskFbo;

	ofDirectory faces;
	int currentFace;

	bool live;

	ofBaseVideoDraws * video;
	ofMesh camMesh;


	ofGstVideoUtils gst;
	GstAppSrc * gstSrc;
	ofPixels pixels;
	ofPixels pixelsAlpha;

	int millisEyesClosed;
	int firstEyesClosedEvent;
	int millisToChange;
	bool faceChangedOnEyesClosed;

	BlinkDetector leftBD,rightBD;

	bool debug;

	bool loadNextFace;

};
