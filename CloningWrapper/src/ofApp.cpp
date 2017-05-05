#include "ofApp.h"

void ofApp::setup() {
	ofSetVerticalSync(true);
	w = 640, h = 480;
	src.allocate(640, 480);
	mask.allocate(640, 480);
	dst.initGrabber(w, h);
	clone.setup(w, h);
}

void ofApp::update() {
	dst.update();
	if(dst.isFrameNew()) {
		int x = mouseX, y = mouseY;
		
		ofSetColor(255);
		
		src.begin();
		ofClear(0, 255);
		dst.draw(x - w / 2, y - h / 2);
		src.end();
		
		mask.begin();
		ofClear(0, 255);
		ofDrawCircle(x, y, 32);
		mask.end();
		
		clone.setStrength(32);
		clone.update(src.getTexture(), dst.getTexture(), mask.getTexture());
	}
}

void ofApp::draw() {
	ofBackground(0);
	clone.draw(0, 0);
}
