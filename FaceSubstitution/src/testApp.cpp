#include "testApp.h"
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>

//#define USE_GST_VIRTUAL_CAMERA

using namespace ofxCv;

void testApp::allocateGstVirtualCamera(){
#ifdef USE_GST_VIRTUAL_CAMERA
	string appsrc = "appsrc  name=video_src is-live=true do-timestamp=true ! "
				"video/x-raw-rgb,width=640,height=480,depth=24,bpp=24,framerate=30/1,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! queue ! ";;
	string videorate;//  = "videorate ! video/x-raw-rgb,depth=24,bpp=24,framerate=25/2,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! ";
	string videoscale;// = "videoscale ! video/x-raw-rgb,width=" + ofToString(width) + ",height=" + ofToString(height) + ",depth=24,bpp=24,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! ";
	string colorspace = " ffmpegcolorspace ! video/x-raw-yuv,width=" + ofToString(width) + ",height=" + ofToString(height) + " ! ";

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
#ifdef TARGET_OSX
	ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);
	cloneReady = false;

	int width = 640;
	int height = 480;

	live = true;
	if(live){
		cam.initGrabber(width, height);
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
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);

	faces.allowExt("jpg");
	faces.allowExt("png");
	faces.listDir("faces");
	currentFace = 0;
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}

	if(!live) vid.play();

	allocateGstVirtualCamera();


	millisEyesClosed = 0;
	firstEyesClosedEvent = 0;
	faceChangedOnEyesClosed = false;
	millisToChange = 1000;
}

void testApp::update() {
	video->update();
	if(video->isFrameNew()) {
		camTracker.update(toCv(*video));
		cloneReady = camTracker.getFound();
	}
	if(camTracker.isFrameNew()) {
		if(cloneReady) {
			camMesh = camTracker.getImageMesh();
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

			leftBD.update(camTracker.getObjectFeature(ofxFaceTracker::LEFT_EYE));
			rightBD.update(camTracker.getObjectFeature(ofxFaceTracker::RIGHT_EYE));



			if(leftBD.isClosed() && rightBD.isClosed()){
				cout << "eyesClosed " << millisEyesClosed <<  endl;
				if(firstEyesClosedEvent==0){
					firstEyesClosedEvent = ofGetElapsedTimeMillis();
				}
				if(!faceChangedOnEyesClosed ){
					millisEyesClosed = ofGetElapsedTimeMillis()-firstEyesClosedEvent;
					if(millisEyesClosed>millisToChange){
						currentFace++;
						currentFace = ofClamp(currentFace,0,faces.size()-1);
						if(faces.size()!=0){
							loadFace(faces.getPath(currentFace));
						}
						faceChangedOnEyesClosed = true;
					}
				}
			}else{
				millisEyesClosed = 0;
				firstEyesClosedEvent = 0;
				faceChangedOnEyesClosed = false;
			}
		}else{
			leftBD.reset();
			rightBD.reset();
		}
			
		clone.setStrength(16);
		clone.update(srcFbo.getTextureReference(), video->getTextureReference(), maskFbo.getTextureReference());

		updateGstVirtualCamera();
	}
}

void testApp::draw() {
	ofSetColor(255);
	
	if(src.getWidth()> 0 && cloneReady) {
		clone.draw(0, 0);
	} else {
		video->draw(0, 0);
	}
	
	if(debug){
		if(!camTracker.getFound()) {
			drawHighlightString("camera face not found", 10, 10);
		}
		if(src.getWidth() == 0) {
			drawHighlightString("drag an image here", 10, 30);
		} else if(!srcTracker.getFound()) {
			drawHighlightString("image face not found", 10, 30);
		}


		ofNoFill();
		const deque<float> & leftEyeHist = leftBD.getHistory();
		ofBeginShape();
		for(int i=0;i<(int)leftEyeHist.size();i++){
			ofVertex(i*10,leftEyeHist[i]*10);
		}
		ofEndShape();


		const deque<float> & rightEyeHist = rightBD.getHistory();
		ofBeginShape();
		for(int i=0;i<(int)rightEyeHist.size();i++){
			ofVertex(i*10,rightEyeHist[i]*10+300);
		}
		ofEndShape();
	}
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

void testApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		currentFace++;
		break;
	case OF_KEY_DOWN:
		currentFace--;
		break;
	case 'f':
		ofToggleFullscreen();
		return;
	case 'd':
		debug = !debug;
		return;
	}
	currentFace = ofClamp(currentFace,0,faces.size()-1);
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
}
