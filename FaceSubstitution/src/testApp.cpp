#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
#ifdef TARGET_OSX
	ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);

	cloneReady = false;
	cam.setUseTexture(false);
	cam.initGrabber(960,544);
	clone.setup(cam.getWidth(), cam.getHeight());
	camTex.allocate(cam.getWidth(), cam.getHeight(),GL_RGB);
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

	ofSetBackgroundAuto(false);

	ofGstVideoUtils * gst = ((ofGstVideoGrabber*)cam.getGrabber().get())->getGstVideoUtils();
	ofAddListener(gst->bufferEvent,this,&testApp::onNewFrame);

	ofHideCursor();

	autoExposure.setup(0,cam.getWidth(),cam.getHeight());

	refreshOnNewFrameOnly = false;
	mutex.lock();
	numCamFrames = 0;

	gui.setup(autoExposure,clone,cam);
}

void testApp::onNewFrame(ofPixels & pixels){
	if(refreshOnNewFrameOnly) condition.signal();
	gui.newCamFrame();
}

void testApp::update() {
	if(refreshOnNewFrameOnly) condition.wait(mutex);

	cam.update();
	faceLoader.update();
	if(refreshOnNewFrameOnly || cam.isFrameNew()){
		convertColor(cam,grayPixels,CV_RGB2GRAY);
		gui.newCamProcessFrame();
		camTracker.update(toCv(grayPixels));
		cloneReady = camTracker.getFound();
		if(cloneReady) {
			camMesh = camTracker.getImageMesh();
			camMeshWithPicTexCoords = camMesh;
			camMeshWithPicTexCoords.getTexCoords() = faceLoader.getCurrentImagePoints();

			srcFbo.begin();
			ofClear(0, 0);
			faceLoader.getCurrentImg().bind();
			camMeshWithPicTexCoords.draw();
			faceLoader.getCurrentImg().unbind();
			srcFbo.end();

			lastFound = 0;
			faceChanged = false;

			if(numCamFrames%2==0){
				autoExposureBB = camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE).getBoundingBox();
				autoExposureBB.scaleFromCenter(.5);
			}

		}else{
			camTex.loadData(cam.getPixelsRef());
			if(!faceChanged){
				lastFound++;
				if(lastFound>5){
					faceLoader.loadNext();
					faceChanged = true;
					lastFound = 0;
				}
			}

			if(numCamFrames%2==0){
				autoExposureBB.set(0,0,cam.getWidth(),cam.getHeight());
				autoExposureBB.scaleFromCenter(.5);
			}
		}

		if(numCamFrames%2==0){
			autoExposure.update(grayPixels,autoExposureBB);
		}
		numCamFrames++;
	}
	gui.update(autoExposureBB);
}

void testApp::draw() {
	ofSetColor(255);
	ofPushMatrix();
	ofTranslate(ofGetWidth(),0);

	if(faceLoader.getCurrentImg().getWidth() > 0 && cloneReady) {
		//clone.draw(0, 0, ofGetWidth(), ofGetHeight());
		ofScale(-ofGetWidth()/cam.getWidth(),ofGetWidth()/cam.getWidth(),1);
		clone.draw(srcFbo.getTextureReference(), cam.getTextureReference(), camMesh);
	} else {
		camTex.draw(0, 0, -ofGetWidth(),ofGetHeight());
	}
	ofPopMatrix();

	gui.draw();
}


void testApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		faceLoader.loadNext();
		break;
	case OF_KEY_DOWN:
		faceLoader.loadPrevious();
		break;
	case ' ':
		refreshOnNewFrameOnly = !refreshOnNewFrameOnly;
		break;
	case 'h':
		gui.show=!gui.show;
		if(gui.show){
			ofShowCursor();
		}else{
			ofHideCursor();
		}
		break;
	}
}
