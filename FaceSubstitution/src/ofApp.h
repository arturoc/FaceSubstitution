#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
	void loadFace(string face);
	
	void keyPressed(int key);

	ofxFaceTrackerThreaded camTracker;
	ofVideoGrabber cam;
	
	ofxFaceTracker srcTracker;
	ofImage src;
    vector<glm::vec2> srcPoints;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo, maskFbo;

	ofDirectory faces;
	int currentFace;
};
