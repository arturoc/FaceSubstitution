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
	grayPixels.allocate(cam.getWidth(),cam.getHeight(),1);
	grayPixelsRotated.allocate(cam.getHeight(),cam.getWidth(),1);
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
		gui.newCamProcessFrame();
		convertColor(cam,grayPixels,CV_RGB2GRAY);
		if(gui.numRotations>0){
			grayPixels.rotate90To(grayPixelsRotated,gui.numRotations);
			camTracker.update(toCv(grayPixelsRotated));
		}else{
			camTracker.update(toCv(grayPixels));
		}
		cloneReady = camTracker.getFound();
		if(cloneReady) {
			camMesh = camTracker.getImageMesh();
			for(int i=0;i<camMesh.getNumVertices();i++){
				ofVec3f & v = camMesh.getVertices()[i];
				ofVec2f & t = camMesh.getTexCoords()[i];
				std::swap(v.x,v.y);
				std::swap(t.x,t.y);
				if(gui.numRotations==1){
					v.y = cam.getHeight()-v.y;
					t.y = cam.getHeight()-t.y;
				}else if(gui.numRotations==3){
					v.x = cam.getWidth()-v.x;
					t.x = cam.getWidth()-t.x;
				}
			}
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

		//TODO: fix BB when image is rotated
		if(numCamFrames%2==0){
			if(gui.numRotations>0){
				swap(autoExposureBB.x,autoExposureBB.y);
				swap(autoExposureBB.width,autoExposureBB.height);
				if(gui.numRotations==1){
					autoExposureBB.y = cam.getHeight()-autoExposureBB.y;
				}else if(gui.numRotations==3){
					autoExposureBB.x = cam.getHeight()-autoExposureBB.x;
				}
				//autoExposure.update(grayPixelsRotated,autoExposureBB);
			}else{
				autoExposure.update(grayPixels,autoExposureBB);
			}
		}
		numCamFrames++;
	}
	gui.update(autoExposureBB);
}

void testApp::draw() {
	ofSetColor(255);
	ofPushMatrix();

	if(faceLoader.getCurrentImg().getWidth() > 0 && cloneReady) {
		ofTranslate(ofGetWidth(),0);
		ofScale(-ofGetWidth()/cam.getWidth(),ofGetWidth()/cam.getWidth(),1);
		clone.draw(srcFbo.getTextureReference(), cam.getTextureReference(), camMesh);
	} else {
		ofTranslate(ofGetWidth(),0);
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
