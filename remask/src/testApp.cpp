#include "testApp.h"
#include "ofxFaceTracker.h"
#include "ofxTween.h"

using namespace ofxCv;

#define USE_PLAYER

//--------------------------------------------------------------
void testApp::setup(){
	bool usePlayer = true;
	if(usePlayer){
		player.loadMovie("fullrt.avi");
		player.play();
		video = &player;
		ofAddListener(((ofGstVideoPlayer*)player.getPlayer().get())->getGstVideoUtils()->bufferEvent,this,&testApp::newBuffer);
	}else{
		grabber.initGrabber(1280,720);
		video = &grabber;
		ofAddListener(((ofGstVideoGrabber*)grabber.getGrabber().get())->getGstVideoUtils()->bufferEvent,this,&testApp::newBuffer);
	}

	faceTracker1.setup();
	faceTracker2.setup();
	threadedFaceTracker1.setup();
	threadedFaceTracker2.setup();
	//threadedFaceTracker1.threadedIfFound = true;
	//threadedFaceTracker2.threadedIfFound = true;


	half1.allocate(640,720,OF_IMAGE_COLOR);
	half2.allocate(640,720,OF_IMAGE_COLOR);
	clone1.setup(640,720);
	clone2.setup(640,720);
	src1Fbo.allocate(640,720,GL_RGBA);
	mask1Fbo.allocate(640,720,GL_RGB);
	src2Fbo.allocate(640,720,GL_RGBA);
	mask2Fbo.allocate(640,720,GL_RGB);
	pixelsCombined.allocate(1280,720,3);
	maskPixels.allocate(1280,720,3);

	vSync.addListener(this,&testApp::vSyncPressed);

	clone1.strength=0;
	clone2.strength=0;
	gui.add(clone1.strength);
	gui.add(clone2.strength);
	gui.add(updateOnLessOrientation.set("updateOnLessOrientation",true));
	gui.add(videoPosition.set("videoPosition",0,0,1));
	gui.add(vSync.set("vSync",true));
	gui.add(showDebug.set("showDebug",false));
	gui.add(drawMesh1.set("drawMesh1",false));
	gui.add(drawMesh2.set("drawMesh2",false));
	gui.add(found1.set("found1",false));
	gui.add(found2.set("found2",false));
	gui.add(rampStrenghtMS.set("rampStrenghtMS",1500,0,5000));
	gui.add(noSwapMS.set("noSwapMS",1000,0,5000));
	gui.add(maxStrength.set("maxStrength",7,0,100));


	videoPosition.addListener(this,&testApp::setVideoPosition);
	mouthOpenDetector1.setup(&threadedFaceTracker1.tracker);
	mouthOpenDetector2.setup(&threadedFaceTracker2.tracker);

	lastOrientation1.set(359,359);
	lastOrientation2.set(359,359);

	meshesInitialized = 0;
	lastTimeFaceFound = 0;
	found = false;
	newFrame = false;
	playerRecorderShutdown = false;
	videoFrame = 0;

	ofEnableAlphaBlending();
	exposure.setup(0,1280,720);
	//ofSetFrameRate(30);
}

void testApp::vSyncPressed(bool & pressed){
	ofSetVerticalSync(pressed);
}

void testApp::setVideoPosition(float & position){
	player.setPosition(position);
}

void testApp::newBuffer(ofPixels & buffer){
	if(!found){
		currentFaceTracker1 = &threadedFaceTracker1;
		currentFaceTracker2 = &threadedFaceTracker2;
	}else{
		currentFaceTracker1 = &threadedFaceTracker1;
		currentFaceTracker2 = &threadedFaceTracker2;
	}

	videoMutex.lock();
	#pragma omp parallel sections num_threads(2)
	{
		#pragma omp section
		{
			buffer.cropTo(half1,0,0,640,720);
		}

		#pragma omp section
		{
			buffer.cropTo(half2,640,0,640,720);
		}
	}
	videoMutex.unlock();


	#pragma omp parallel sections num_threads(2)
	{
		#pragma omp section
		{
			currentFaceTracker1->update(toCv(half1));
		}

		#pragma omp section
		{
			currentFaceTracker2->update(toCv(half2));
		}
	}


	videoMutex.lock();
	found1 = currentFaceTracker1->getFound();
	found2 = currentFaceTracker2->getFound();
	found = found1 && found2;
	if(found){
		mouthOpenDetector1.update();
		mouthOpenDetector2.update();

		ofVec2f currentOrientation1(fabs(currentFaceTracker1->getOrientation().x),fabs(currentFaceTracker1->getOrientation().y));
		if(!updateOnLessOrientation || (currentOrientation1.x<lastOrientation1.x && currentOrientation1.y<lastOrientation1.y) || (lastOrientationMouthOpenness1 > mouthOpenDetector1.getOpennes())){
			half1Src.getPixelsRef() = half1.getPixelsRef();
			mesh2.getTexCoords() = currentFaceTracker1->getImagePoints();
			lastOrientation1 = currentOrientation1;
			lastOrientationMouthOpenness1 = mouthOpenDetector1.getOpennes();
			half1NeedsUpdate = true;
		}

		ofVec2f currentOrientation2(fabs(currentFaceTracker2->getOrientation().x),fabs(currentFaceTracker2->getOrientation().y));
		if(!updateOnLessOrientation || (currentOrientation2.x<lastOrientation2.x && currentOrientation2.y<lastOrientation2.y) || (lastOrientationMouthOpenness2 > mouthOpenDetector2.getOpennes())){
			half2Src.getPixelsRef() = half2.getPixelsRef();
			mesh1.getTexCoords() = currentFaceTracker2->getImagePoints();
			lastOrientation2 = currentOrientation2;
			lastOrientationMouthOpenness2 = mouthOpenDetector2.getOpennes();
			half2NeedsUpdate = true;
		}


		if(!meshesInitialized){
			mesh1 = currentFaceTracker1->getImageMesh();
			mesh2 = currentFaceTracker2->getImageMesh();
			meshesInitialized = true;
		}else{
			mesh1.getVertices() = currentFaceTracker1->getImageMesh().getVertices();
			mesh2.getVertices() = currentFaceTracker2->getImageMesh().getVertices();
			mesh1.getIndices() = threadedFaceTracker1.tracker.getMesh(threadedFaceTracker1.getImagePoints()).getIndices();
			mesh2.getIndices() = threadedFaceTracker2.tracker.getMesh(threadedFaceTracker2.getImagePoints()).getIndices();
		}
	}
	newFrame = true;
	videoMutex.unlock();
}

//--------------------------------------------------------------
void testApp::update(){
	video->update();

	videoMutex.lock();
	bool isNewFrame = newFrame;
	bool foundFaces = found;
	newFrame = false;
	videoMutex.unlock();

	if(isNewFrame){
		u_long now = ofGetElapsedTimeMillis();
		if(foundFaces){
			if(lastTimeFaceFound==0){
				recorder.setup(ofGetTimestampString()+".mov","",1280,720,30);
				lastTimeFaceFound = now;
			}else if (now - lastTimeFaceFound>noSwapMS){
				ofxEasingQuart easing;
				int s = ofxTween::map(now-lastTimeFaceFound,0,rampStrenghtMS,0,maxStrength,true,easing,ofxTween::easeIn);
				clone1.strength = s;
				clone2.strength = s;
			}
			recorder.addFrame(video->getPixelsRef());

			videoMutex.lock();
			half1.update();
			half2.update();
			if(half1NeedsUpdate){
				half1Src.update();
				half1NeedsUpdate=false;
				vboMesh2.getTexCoords() = mesh2.getTexCoords();
			}
			if(half2NeedsUpdate){
				half2Src.update();
				half2NeedsUpdate=false;
				vboMesh1.getTexCoords() = mesh1.getTexCoords();
			}
			if(!vbosInitialized){
				vboMesh1 = mesh1;
				vboMesh1.setUsage(GL_DYNAMIC_DRAW);
				vboMesh2 = mesh2;
				vboMesh2.setUsage(GL_DYNAMIC_DRAW);
				vbosInitialized = true;
			}else{
				vboMesh1.getVertices() = mesh1.getVertices();
				vboMesh2.getVertices() = mesh2.getVertices();
				vboMesh1.getIndices() = mesh1.getIndices();
				vboMesh2.getIndices() = mesh2.getIndices();
			}
			videoMutex.unlock();

			mask1Fbo.begin();
			ofClear(0, 255);
			vboMesh1.draw();
			mask1Fbo.end();

			mask2Fbo.begin();
			ofClear(0, 255);
			vboMesh2.draw();
			mask2Fbo.end();

			src1Fbo.begin();
			ofClear(0, 255);
			half2Src.getTextureReference().bind();
			vboMesh1.draw();
			half2Src.getTextureReference().unbind();
			src1Fbo.end();

			src2Fbo.begin();
			ofClear(0, 255);
			half1Src.getTextureReference().bind();
			vboMesh2.draw();
			half1Src.getTextureReference().unbind();
			src2Fbo.end();

			clone1.update(src1Fbo.getTextureReference(), half1.getTextureReference(), mesh1, mask1Fbo.getTextureReference());
			clone2.update(src2Fbo.getTextureReference(), half2.getTextureReference(), mesh2, mask2Fbo.getTextureReference());

			if(videoFrame%10==0){
				mask1Fbo.readToPixels(mask1);
				mask2Fbo.readToPixels(mask2);
				mask1.pasteInto(maskPixels,0,0);
				mask2.pasteInto(maskPixels,0,0);
				half1.getPixelsRef().pasteInto(pixelsCombined,0,0);
				half2.getPixelsRef().pasteInto(pixelsCombined,0,0);
				if(video==&grabber){
					exposure.update(pixelsCombined,maskPixels);
				}
			}
			lastTimeFaceDetected = now;
		}else{
			if(now-lastTimeFaceDetected>2000){
				lastOrientation1.set(359,359);
				lastOrientation2.set(359,359);
				lastTimeFaceFound = 0;
				recorder.close();
			}
		}
		videoFrame++;
	}

	if(video==&player && !playerRecorderShutdown && player.getIsMovieDone()){
		playerRecorderShutdown = true;
		recorder.close();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	video->draw(0,0);
	if(found){
		clone1.draw(0,0);
		clone2.draw(640,0);
	}
	if(showDebug){
		mask1Fbo.draw(150,0,320,360);
		src1Fbo.draw(150+320,0,320,360);
		half1Src.draw(150+640,0,320,360);

		mask2Fbo.draw(150,360,320,360);
		src2Fbo.draw(150+320,360,320,360);
		half2Src.draw(150+640,360,320,360);
	}

	ofSetColor(255,255,255,50);
	if(drawMesh1){
		vboMesh1.drawWireframe();
	}
	if(drawMesh2){
		ofPushMatrix();
		ofTranslate(640,0);
		vboMesh2.drawWireframe();
		ofPopMatrix();
	}

	ofSetColor(255,255,255);
	ofDrawBitmapString(ofToString(ofGetFrameRate()),ofGetWidth()-100,20);
	gui.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
