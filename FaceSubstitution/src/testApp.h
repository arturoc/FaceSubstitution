#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"

#include "ofxUI.h"
class ofxEasyUI {
public:
	ofxUICanvas* gui;
	
	void setup() {
		gui = new ofxUICanvas();
		gui->setTheme(OFX_UI_THEME_MINBLACK);
	}
	void add(string label, bool& value) {
		gui->addButton(label, &value);
		gui->autoSizeToFitWidgets();
	}
	void add(string label, float& value, float min, float max) {
		gui->addMinimalSlider(label, min, max, &value);
		gui->autoSizeToFitWidgets();
	}
};

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
	void loadFace(string face);
	
	void keyPressed(int key);
	
	float rescale = .5; // 1-.5
	float iterations = 15; // 1-25
	float clamp = 3; // 0-4
	float tolerance = .1; // .01-1
	float attempts = 1; // 1-4
	
	ofxEasyUI ui;

	ofxFaceTracker camTracker;
	ofVideoGrabber cam;
	
	ofxFaceTracker srcTracker;
	ofImage src;
	vector<ofVec2f> srcPoints;
	
	float strength, targetStrength;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo, maskFbo;
};
