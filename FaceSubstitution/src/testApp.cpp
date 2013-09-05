#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
#ifdef TARGET_OSX
	ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);
	cloneReady = false;
	cam.initGrabber(960,544);
	clone.setup(cam.getWidth(), cam.getHeight());
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();

	faceLoader.setup("faces",FaceLoader::Random);

	clone.strength = 7;
	lastFound = 0;
	faceChanged = false;
}

void testApp::update() {
	cam.update();
	faceLoader.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
			camMesh = camTracker.getImageMesh();
			camMesh.clearTexCoords();
			camMesh.addTexCoords(faceLoader.getCurrentImagePoints());
			
			maskFbo.begin();
			ofClear(0, 255);
			camMesh.draw();
			maskFbo.end();
			
			srcFbo.begin();
			ofClear(0, 255);
			faceLoader.getCurrentImg().bind();
			camMesh.draw();
			faceLoader.getCurrentImg().unbind();
			srcFbo.end();
			
			clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), camMesh, maskFbo.getTextureReference());
			lastFound = 0;
			faceChanged = false;
		}else{
			if(!faceChanged){
				lastFound++;
				if(lastFound>5){
					faceLoader.loadNext();
					faceChanged = true;
					lastFound = 0;
				}
			}
		}
	}
}

void testApp::draw() {
	ofSetColor(255);
	
	if(faceLoader.getCurrentImg().getWidth() > 0 && cloneReady) {
		clone.draw(0, 0, ofGetWidth(), ofGetHeight());
	} else {
		cam.draw(0, 0, ofGetWidth(),ofGetHeight());
	}
}


void testApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		faceLoader.loadNext();
		break;
	case OF_KEY_DOWN:
		faceLoader.loadPrevious();
		break;
	}
}
