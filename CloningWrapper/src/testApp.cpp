#include "testApp.h"

void testApp::setup() {
	ofSetVerticalSync(true);
	w = 640, h = 480;
	src.allocate(640, 480);
	mask.allocate(640, 480);
	dst.initGrabber(w, h);
	clone.setup(w, h);
}

void testApp::update() {
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
		ofCircle(x, y, 32);
		mask.end();
		
		clone.setStrength(32);
		clone.update(src.getTextureReference(), dst.getTextureReference(), mask.getTextureReference());
	}
}

void testApp::draw() {
	ofBackground(0);
	clone.draw(0, 0);
}