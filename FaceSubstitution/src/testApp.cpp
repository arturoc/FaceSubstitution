#include "testApp.h"

int w = 1280;
int h = 720;

using namespace ofxCv;

#define FACES_DIR "faces"
#define LOAD_MODE FaceLoader::Sequential


void testApp::setup() {
	// logging
	ofLog::setAutoSpace(true);
	//ofSetLogLevel(OF_LOG_VERBOSE);
	//ofSetLogLevel("testApp",OF_LOG_VERBOSE);
	//ofSetLogLevel(BlinkDetector::LOG_NAME,OF_LOG_VERBOSE);
	ofSetLogLevel(FaceBlinkRecorder::LOG_NAME,OF_LOG_VERBOSE);
	ofSetLogLevel(VideoFader::LOG_NAME,OF_LOG_VERBOSE);
	ofSetLogLevel(ofxV4L2Settings::LOG_NAME,OF_LOG_VERBOSE);

#ifdef FACES_IN_BUNDLE
	ofSetDataPathRoot("../data/");
#endif


	// initialize state variables
	cloneReady = false;
	takeSnapshotFrom = 0;
	loadNextFace = false;
	numInputRotation90 = 0;


	// general graphics options
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	ofBackground(0);


	// video
	live = true;
	if(live){
		//cam.setDeviceID(1);
		cam.initGrabber(w, h);
		video = &cam;
	}else{
		vid.loadMovie("video.mp4");
		video = &vid;
		vid.play();
	}


	// init clone and related buffers
	clone.setup(video->getWidth(), video->getHeight());
	clone.setStrength(16);

	ofFbo::Settings settings;
	settings.width = video->getWidth();
	settings.height = video->getHeight();
	srcFbo.allocate(settings);

	settings.internalformat = GL_LUMINANCE;
	maskFbo.allocate(settings);

	rotatedInput.allocate(video->getHeight(),video->getWidth(),OF_IMAGE_COLOR);


	// init trackers
	camTracker.setup();
	//camTracker.getTracker()->setRescale(.5);
	camTracker.getTracker()->setIterations(10);

	faceLoader.setup(FACES_DIR,LOAD_MODE);


	// init blink triggers
	blinkTrigger.setup(camTracker);
	ofAddListener(blinkTrigger.blinkE,this,&testApp::blinkTriggered);
	ofAddListener(blinkTrigger.longBlinkE,this,&testApp::longBlinkTriggered);

	ofAddListener(blinkRecorder.recordedE,this,&testApp::videoRecorded);


	// init other utils classes
	autoExposure.setup(0,w,h);
	blinkRecorder.setup(camTracker);
	snapshotSaver.setup("screenshots");


	// gui
	showGui = false;
	ofHideCursor();
	gui.setup(&faceLoader,&blinkTrigger,&camMesh,&camTracker,&videoFader,&blinkRecorder, &autoExposure, &rotatedInputTex, numInputRotation90);
	gui.showVideos.addListener(this,&testApp::showVideosChanged);


	showVideosChanged(gui.showVideos);
}

void testApp::showVideosChanged(bool & v){
	if(v)
		ofAddListener(blinkRecorder.recordingE,this,&testApp::recording);
	else
		ofRemoveListener(blinkRecorder.recordingE,this,&testApp::recording);
}

void testApp::recording(bool & rec){
	if(rec) videoFader.setup(video);
}

void testApp::videoRecorded(bool & r){
	recordVideo = true;
	recorder.setup("recordings_interaction/"+ofGetTimestampString()+".mov",w,h,blinkRecorder.getFps());
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

	if(!cloneReady && recordVideo){
		recorder.encodeVideo();
		recordVideo = false;
	}

	if(!blinkRecorder.isRecording() && cloneReady && frameProcessed) {
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

		bool takeSnapshot = takeSnapshotFrom>0 && ofGetElapsedTimef()-takeSnapshotFrom>1.5;

		if(takeSnapshot || recordVideo){
			clone.readToPixels(snapshot);

			if(takeSnapshot){
				snapshotSaver.save(snapshot);
				takeSnapshotFrom = 0;
			}else{
				recorder.addFrame(snapshot);
			}
		}

		if(adjustExposure){
			adjustExposure = false;
			maskFbo.readToPixels(maskPixels);
			autoExposure.update(video->getPixelsRef(),maskPixels);
		}
	}

	video->update();
	if(video->isFrameNew()) {
		if(numInputRotation90!=0 && numInputRotation90!=2){
			if(video->getWidth()!=rotatedInput.getHeight()){
				rotatedInput.allocate(video->getHeight(),video->getWidth(),3);
			}
			video->getPixelsRef().rotate90To(rotatedInput,numInputRotation90);
			if(gui.showInput){
				if(rotatedInputTex.getWidth()!=rotatedInput.getWidth()){
					rotatedInputTex.allocate(rotatedInput);
				}
				rotatedInputTex.loadData(rotatedInput);
			}
			camTracker.update(toCv(rotatedInput));
		}else{
			if(gui.showInput){
				if(video->getWidth()!=rotatedInput.getWidth() || rotatedInput.getWidth()!=rotatedInputTex.getWidth()){
					rotatedInput.allocate(video->getWidth(),video->getHeight(),3);
					rotatedInputTex.allocate(rotatedInput);
				}
				rotatedInput = video->getPixelsRef();
				rotatedInputTex.loadData(rotatedInput);
			}
			camTracker.update(toCv(*video));
		}
		if(gui.showVideos) blinkRecorder.update(video->getPixelsRef());
	}

	if(blinkRecorder.isRecording()){
		videoFader.update();
	}

	if(gui.showVideos){
		blinkRecorder.setEyesClosed(blinkTrigger.areEyesClosed());
	}

	ofSetOrientation(orientation);
	gui.update();

}


void testApp::blinkTriggered(bool & eyesClosed){

}

void testApp::longBlinkTriggered(bool & eyesClosed){
	if(eyesClosed){
		loadNextFace = true;
		adjustExposure = true;
	}else{
		takeSnapshotFrom = ofGetElapsedTimef();
	}
}

void testApp::draw() {
	/*ofOrientation orientation = ofGetOrientation();

	ofSetOrientation(OF_ORIENTATION_DEFAULT);
	ofPushView();
	ofSetupScreenPerspective();*/

	ofSetColor(255);
	
	float ratio;
	int width,height;


	if(numInputRotation90==0 || numInputRotation90==2){
		ratio = clone.getTextureRef().getHeight()/clone.getTextureRef().getWidth();
		width = -ofGetWidth()*gui.screenScale;
		height = ofGetWidth()*gui.screenScale*ratio;
	}else{
		ratio = clone.getTextureRef().getHeight()/clone.getTextureRef().getWidth();
		width = -ofGetHeight()*gui.screenScale;
		height = ofGetHeight()*gui.screenScale*ratio;

	}
	//int x = ofGetWidth();
	//int y = (ofGetHeight()-float(ofGetWidth())*ratio)*.5;


	/*if(numInputRotation90==0 || numInputRotation90==1){
		x = width;
		width = -width;
	}

	if(numInputRotation90==3){
		y = ofGetHeight() - y;
		height = -height;
	}*/

	ofSetRectMode(OF_RECTMODE_CENTER);
	ofPushMatrix();
	ofTranslate(ofGetWidth()*.5,ofGetHeight()*.5);
	ofRotate(-90*numInputRotation90,0,0,1);
	if(blinkRecorder.isRecording()){
		videoFader.draw(0,0,width,height);
	}else{
		if(faceLoader.getCurrentImg().getWidth()> 0 && cloneReady) {
			clone.draw(0,0,width,height);
		} else {
			video->draw(0,0,width,height);
		}
	}
	ofPopMatrix();
	ofSetRectMode(OF_RECTMODE_CORNER);
	//ofPopView();
	//ofSetOrientation(orientation);

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
