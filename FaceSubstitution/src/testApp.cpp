






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

void testApp::allocateGstVirtualCamera(){
#ifdef USE_GST_VIRTUAL_CAMERA
	string appsrc = "appsrc  name=video_src is-live=true do-timestamp=true ! "
				"video/x-raw-rgb,width=640,height=480,depth=24,bpp=24,framerate=30/1,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! queue ! ";;
	string videorate;//  = "videorate ! video/x-raw-rgb,depth=24,bpp=24,framerate=25/2,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! ";
	string videoscale;// = "videoscale ! video/x-raw-rgb,width=" + ofToString(width) + ",height=" + ofToString(height) + ",depth=24,bpp=24,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! ";
	string colorspace = " ffmpegcolorspace ! video/x-raw-yuv,width=" + ofToString(w) + ",height=" + ofToString(h) + " ! ";

	string pipeline = appsrc + videorate + videoscale + colorspace + " v4l2sink device=/dev/video2";

	gst.setPipelineWithSink(pipeline,"v4l2sink",false);
	gstSrc = (GstAppSrc*)gst_bin_get_by_name(GST_BIN(gst.getPipeline()),"video_src");
	if(gstSrc){
		gst_app_src_set_stream_type (gstSrc,GST_APP_STREAM_TYPE_STREAM);
		g_object_set (G_OBJECT(gstSrc), "format", GST_FORMAT_TIME, NULL);
	}

	pixels.allocate(640,480,OF_IMAGE_COLOR);

	gst.play();
#endif
}

void testApp::updateGstVirtualCamera(){
#ifdef USE_GST_VIRTUAL_CAMERA
	clone.readToPixels(pixelsAlpha);
	pixels.setChannel(0,pixelsAlpha.getChannel(0));
	pixels.setChannel(1,pixelsAlpha.getChannel(1));
	pixels.setChannel(2,pixelsAlpha.getChannel(2));
	GstBuffer * buffer;
	buffer = gst_app_buffer_new (pixels.getPixels(), 640*480*3, NULL, pixels.getPixels());
	GstFlowReturn flow_return = gst_app_src_push_buffer(gstSrc, buffer);
	if (flow_return != GST_FLOW_OK) {
		ofLog(OF_LOG_WARNING,"error pushing buffer");
	}
#endif
}


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
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();
	camTracker.getTracker()->setRescale(.5);
	camTracker.getTracker()->setIterations(10);

	faceLoader.setup(FACES_DIR,LOAD_MODE);

	if(!live) vid.play();

	allocateGstVirtualCamera();


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
	rotatedInputTex.allocate(video->getHeight(),video->getWidth(),GL_RGB);


	clone.setStrength(16);

	autoExposure.setup(0,w,h);

	blinkRecorder.setup(camTracker);
	gui.setup(&faceLoader,&leftBD,&rightBD,&camMesh,&camTracker,&videoFader,&blinkRecorder, &autoExposure);

	showVideosChanged(gui.showVideos);
	gui.showVideos.addListener(this,&testApp::showVideosChanged);

	ofEnableAlphaBlending();

	showGui = false;
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
	if(loadNextFace){
		faceLoader.loadNext();
		loadNextFace  = false;
	}
	faceLoader.update();
	bool prevFound = cloneReady;
	cloneReady = camTracker.getFound();
	bool frameProcessed = camTracker.isFrameNew();

	if(cloneReady && frameProcessed) {
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

		if(1){
			maskFbo.readToPixels(maskPixels);
			autoExposure.update(video->getPixelsRef(),maskPixels);
		}

		srcFbo.begin();
		faceLoader.getCurrentImg().bind();
		camMesh.draw();
		faceLoader.getCurrentImg().unbind();
		srcFbo.end();

		clone.update(srcFbo.getTextureReference(), video->getTextureReference(), camMesh, maskFbo.getTextureReference());

		updateGstVirtualCamera();
	}

	video->update();
	if(video->isFrameNew()) {
		if(numInputRotation90!=0 && numInputRotation90!=2){
			video->getPixelsRef().rotate90To(rotatedInput,numInputRotation90);
			camTracker.update(toCv(rotatedInput));
			if(debug) rotatedInputTex.loadData(rotatedInput);
		}else{
			camTracker.update(toCv(*video));
		}
		if(gui.showVideos) blinkRecorder.update(video->getPixelsRef());
	}

	videoFader.update();
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
	case 'd':
		debug = !debug;
		return;
	case 'g':
		showGui = !showGui;
		break;
	}
}
