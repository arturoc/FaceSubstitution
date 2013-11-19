#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
	cloneReady = false;
	cam.initGrabber(1280, 720);
	clone.setup(cam.getWidth(), cam.getHeight());
	clone.setStrength(0);
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);
	
	strength = 0;
	targetStrength = 0;

	loadFace("marina.jpg");
	
//	ui.setup();
//	ui.add("rescale", rescale, .5, 1);
//	ui.add("iterations", iterations, 1, 25);
//	ui.add("clamp", clamp, 0, 15);
//	ui.add("tolerance", tolerance, .01, 1);
//	ui.add("attempts", attempts, 1, 4);
}

void testApp::update() {
	strength = ofLerp(strength, targetStrength, .01);
	
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.setRescale(rescale);
		camTracker.setIterations(iterations);
		camTracker.setClamp(clamp);
		camTracker.setTolerance(tolerance);
		camTracker.setAttempts(attempts);
		
		camTracker.update(toCv(cam));
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
			ofMesh camMesh = camTracker.getImageMesh();
			camMesh.clearTexCoords();
			camMesh.addTexCoords(srcPoints);
			
			maskFbo.begin();
			ofClear(0, 255);
			camMesh.draw();
			maskFbo.end();
			
			srcFbo.begin();
			ofClear(0, 255);
			src.bind();
			camMesh.draw();
			src.unbind();
			srcFbo.end();
			
			clone.setStrength(strength);
			clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
		}
	}
}

void testApp::draw() {
	ofSetColor(255);
	clone.draw(0, 0);
//	cam.draw(0, 0);
//	camTracker.draw();
}

void testApp::loadFace(string face){
	src.loadImage(face);
	if(src.getWidth() > 0) {
		srcTracker.update(toCv(src));
		srcPoints = srcTracker.getImagePoints();
	}
}

void testApp::dragEvent(ofDragInfo dragInfo) {
	loadFace(dragInfo.files[0]);
}

void testApp::keyPressed(int key) {
	int level = key - '1';
	level = ofClamp(level, 0, 9);
	targetStrength = ofMap(level, 1, 9, 0, 25);
}
