






#include "testApp.h"

#ifdef USE_GST_VIRTUAL_CAMERA
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>
#endif

int w = 1280;
int h = 720;

using namespace ofxCv;

#define FACES_DIR "faces"
#define LOAD_MODE FaceLoader::Sequential


void testApp::setup() {
	ofLog::setAutoSpace(true);
	//ofSetLogLevel(OF_LOG_VERBOSE);
	//ofSetLogLevel("testApp",OF_LOG_VERBOSE);
	//ofSetLogLevel(BlinkDetector::LOG_NAME,OF_LOG_VERBOSE);
	ofSetLogLevel(FaceBlinkRecorder::LOG_NAME,OF_LOG_VERBOSE);
	ofSetLogLevel(VideoFader::LOG_NAME,OF_LOG_VERBOSE);

#ifdef FACES_IN_BUNDLE
	ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);
	cloneReady = false;

	live = true;
	if(live){
		cam.setDeviceID(1);
		cam.initGrabber(w, h);
		video = &cam;
	}else{
		vid.loadMovie("video.mp4");
		video = &vid;
	}

	clone.setup(video->getWidth(), video->getHeight());
	ofFbo::Settings settings;
	settings.width = video->getWidth();
	settings.height = video->getHeight();
	srcFbo.allocate(settings);
	settings.internalformat = GL_LUMINANCE;
	maskFbo.allocate(settings);

	camTracker.setup();
	//camTracker.getTracker()->setRescale(.5);
	camTracker.getTracker()->setIterations(10);

	faceLoader.setup(FACES_DIR,LOAD_MODE);

	if(!live) vid.play();


	millisEyesClosed = 0;
	firstEyesClosedEvent = 0;
	faceChangedOnEyesClosed = false;
	millisToChange = 200;



	leftBD.setup(camTracker.getTracker(),ofxFaceTracker::LEFT_EYE);
	rightBD.setup(camTracker.getTracker(),ofxFaceTracker::RIGHT_EYE);

	ofAddListener(camTracker.threadedUpdateE,this,&testApp::threadedUpdate);

	ofBackground(0);
	numInputRotation90 = 0;
	rotatedInput.allocate(video->getHeight(),video->getWidth(),OF_IMAGE_COLOR);


	clone.setStrength(16);

	autoExposure.setup(0,w,h);

	blinkRecorder.setup(camTracker);
	gui.setup(&faceLoader,&leftBD,&rightBD,&camMesh,&camTracker,&videoFader,&blinkRecorder, &autoExposure, numInputRotation90);

	showVideosChanged(gui.showVideos);
	gui.showVideos.addListener(this,&testApp::showVideosChanged);

	ofEnableAlphaBlending();

	showGui = false;
	ofHideCursor();

	takeSnapshotFrom = 0;
	snapshotSaver.setup("screenshots");
}

void testApp::showVideosChanged(bool & v){
	if(v)
		ofAddListener(blinkRecorder.recordingE,this,&testApp::recording);
	else
		ofRemoveListener(blinkRecorder.recordingE,this,&testApp::recording);
}

void testApp::recording(bool & rec){
	if(rec) videoFader.setup(video);
	isRecording = rec;
}

void testApp::update() {
	ofOrientation orientation = ofGetOrientation();
	ofSetOrientation(OF_ORIENTATION_DEFAULT);

	if(loadNextFace){
		faceLoader.loadNext();
		loadNextFace  = false;
	}
	faceLoader.update();
	bool prevFound = cloneReady;
	cloneReady = camTracker.getFound();
	bool frameProcessed = camTracker.isFrameNew();

	if(!isRecording && cloneReady && frameProcessed) {
		camMesh = camTracker.getImageMesh();
		camMesh.getTexCoords() = faceLoader.getCurrentImagePoints();

		if(numInputRotation90!=0){
			for(int i=0;i<camMesh.getNumVertices();i++){
				ofVec3f & v = camMesh.getVertices()[i];
				std::swap(v.x,v.y);
				if(numInputRotation90==1)
					v.y = video->getHeight()-v.y;
				else if(numInputRotation90==3)
					v.x = video->getWidth()-v.x;
			}
		}

		maskFbo.begin();
		ofClear(0, 255);
		camMesh.draw();
		maskFbo.end();


		srcFbo.begin();
		faceLoader.getCurrentImg().bind();
		camMesh.draw();
		faceLoader.getCurrentImg().unbind();
		srcFbo.end();

		clone.update(srcFbo.getTextureReference(), video->getTextureReference(), camMesh, maskFbo.getTextureReference());

		if(takeSnapshotFrom>0 && ofGetElapsedTimef()-takeSnapshotFrom>1.5){
			maskFbo.readToPixels(maskPixels);
			autoExposure.update(video->getPixelsRef(),maskPixels);
			clone.readToPixels(snapshot);
			snapshotSaver.save(snapshot);
			takeSnapshotFrom = 0;
		}
	}

	video->update();
	if(video->isFrameNew()) {
		if(numInputRotation90!=0 && numInputRotation90!=2){
			video->getPixelsRef().rotate90To(rotatedInput,numInputRotation90);
			camTracker.update(toCv(rotatedInput));
		}else{
			camTracker.update(toCv(*video));
		}
		if(gui.showVideos) blinkRecorder.update(video->getPixelsRef());
	}

	if(isRecording){
		videoFader.update();
	}
	ofSetOrientation(orientation);
	gui.update();

}

void testApp::threadedUpdate(ofEventArgs & args){
	if(camTracker.getFound()){
		leftBD.update();
		rightBD.update();

		if(leftBD.isClosed() && rightBD.isClosed()){
			ofLogVerbose("testApp") << "eyesClosed" << millisEyesClosed;
			if(firstEyesClosedEvent==0){
				firstEyesClosedEvent = ofGetElapsedTimeMillis();
			}
			if(!faceChangedOnEyesClosed ){
				millisEyesClosed = ofGetElapsedTimeMillis()-firstEyesClosedEvent;
				if(millisEyesClosed>millisToChange){
					loadNextFace = true;
					faceChangedOnEyesClosed = true;
				}
			}
			if(gui.showVideos) blinkRecorder.setEyesClosed(true);
		}else{
			if(millisEyesClosed>millisToChange){
				takeSnapshotFrom = ofGetElapsedTimef();
			}
			millisEyesClosed = 0;
			firstEyesClosedEvent = 0;
			faceChangedOnEyesClosed = false;
			if(gui.showVideos) blinkRecorder.setEyesClosed(false);
		}
	}else{
		leftBD.reset();
		rightBD.reset();
	}
}

void testApp::draw() {
	ofOrientation orientation = ofGetOrientation();

	ofSetOrientation(OF_ORIENTATION_DEFAULT);
	ofPushView();
	ofSetupScreenPerspective();

	ofSetColor(255);
	
	float ratio = clone.getTextureRef().getHeight()/clone.getTextureRef().getWidth();

	int width = ofGetWidth();
	int height = ofGetWidth()*ratio;
	int x = 0;
	int y = (ofGetHeight()-float(ofGetWidth())*ratio)*.5;


	if(numInputRotation90==0 || numInputRotation90==1){
		x = width;
		width = -width;
	}

	if(numInputRotation90==3){
		y = ofGetHeight() - y;
		height = -height;
	}

	if(isRecording){
		videoFader.draw(x,y,width,height);
	}else{
		if(faceLoader.getCurrentImg().getWidth()> 0 && cloneReady) {
			clone.draw(x,y,width,height);
		} else {
			video->draw(x,y,width,height);
		}
	}
	ofPopView();
	ofSetOrientation(orientation);

	if(showGui){
		gui.draw();
	}
}


void testApp::dragEvent(ofDragInfo dragInfo) {
	//faceLoader.loadFace(dragInfo.files[0]);
}

void testApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		faceLoader.loadNext();
		break;
	case OF_KEY_DOWN:
		faceLoader.loadPrevious();
		break;
	case 'f':
		ofToggleFullscreen();
		return;
	case 'g':
		showGui = !showGui;
		if(showGui){
			ofShowCursor();
		}else{
			ofHideCursor();
		}
		break;
	}
}
