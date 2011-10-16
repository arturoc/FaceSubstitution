#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
	ofSetVerticalSync(true);
	cloneReady = false;
	cam.initGrabber(640, 480);
	clone.setup(cam.getWidth(), cam.getHeight());
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
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
			
			clone.setStrength(16);
			clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
		}
	}
}

void testApp::draw() {
	ofSetColor(255);
	
	if(src.getWidth() > 0 && cloneReady) {
		clone.draw(0, 0);
	} else {
		cam.draw(0, 0);
	}
	
	if(!camTracker.getFound()) {
		drawHighlightString("camera face not found", 10, 10);
	}
	if(src.getWidth() == 0) {
		drawHighlightString("drag an image here", 10, 30);
	} else if(!srcTracker.getFound()) {
		drawHighlightString("image face not found", 10, 30);
	}
}

void testApp::dragEvent(ofDragInfo dragInfo) {
	src.loadImage(dragInfo.files[0]);
	if(src.getWidth() > 0) {
		srcTracker.update(toCv(src));
		srcPoints = srcTracker.getImagePoints();
	}
}