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
	srcFbo.allocate(settings);
	camTracker.setup();
	camTracker.setIterations(3);

	faceLoader.setup("faces",FaceLoader::Random);

	clone.strength = 5;
	lastFound = 0;
	faceChanged = false;

	gray.allocate(960,544,1);

	/*ofGstVideoUtils * gst = ((ofGstVideoGrabber*)cam.getGrabber().get())->getGstVideoUtils();
	gst->*/

	ofHideCursor();
}

void testApp::update() {
	cam.update();
	faceLoader.update();
	if(cam.isFrameNew()) {
		int size = cam.getPixelsRef().getWidth()*cam.getPixelsRef().getHeight();
		unsigned char * pixels = cam.getPixels();
		for(int i=0;i<size;i++,pixels+=3){
			gray[i] = *pixels;
		}
		camTracker.update(toCv(cam));
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
			camMesh = camTracker.getImageMesh();
			camMesh.getTexCoords() = faceLoader.getCurrentImagePoints();
			
			srcFbo.begin();
			ofClear(0, 0);
			faceLoader.getCurrentImg().bind();
			camMesh.draw();
			faceLoader.getCurrentImg().unbind();
			srcFbo.end();

			//clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), camMesh);
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
	ofPushMatrix();
	ofTranslate(ofGetWidth(),0);
	
	if(faceLoader.getCurrentImg().getWidth() > 0 && cloneReady) {
		//clone.draw(0, 0, ofGetWidth(), ofGetHeight());
		ofScale(-ofGetWidth()/cam.getWidth(),ofGetWidth()/cam.getWidth(),1);
		clone.draw(srcFbo.getTextureReference(), cam.getTextureReference(), camTracker.getImageMesh());
	} else {
		cam.draw(0, 0, -ofGetWidth(),ofGetHeight());
	}
	ofPopMatrix();
	ofDrawBitmapString(ofToString((int)ofGetFrameRate()),20,20);
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
