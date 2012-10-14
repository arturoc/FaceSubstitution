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
	}else{
		grabber.initGrabber(1280,720);
		video = &grabber;
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
	mask1Fbo.allocate(640,720,GL_RGBA);
	src2Fbo.allocate(640,720,GL_RGBA);
	mask2Fbo.allocate(640,720,GL_RGBA);

	clone1.strength=0;
	clone2.strength=0;
	gui.add(clone1.strength);
	gui.add(clone2.strength);
	gui.add(updateOnLessOrientation.set("updateOnLessOrientation",true));
	gui.add(videoPosition.set("videoPosition",0,0,1));
	gui.add(vSync.set("vSync",false));
	gui.add(showDebug.set("showDebug",false));
	gui.add(drawMesh1.set("drawMesh1",false));
	gui.add(drawMesh2.set("drawMesh2",false));
	gui.add(found1.set("found1",false));
	gui.add(found2.set("found2",false));
	gui.add(rampStrenghtMS.set("rampStrenghtMS",1500,0,5000));
	gui.add(noSwapMS.set("noSwapMS",1000,0,5000));
	gui.add(maxStrength.set("maxStrength",7,0,100));

	videoPosition.addListener(this,&testApp::setVideoPosition);
	vSync.addListener(this,&testApp::vSyncPressed);

	mouthOpenDetector1.setup(&faceTracker1);
	mouthOpenDetector2.setup(&faceTracker2);

	lastOrientation1.set(359,359);
	lastOrientation2.set(359,359);

	meshesInitialized = 0;
	lastTimeFace = 0;
	found = false;

	ofEnableAlphaBlending();
	//ofSetFrameRate(30);
}

void testApp::vSyncPressed(bool & pressed){
	ofSetVerticalSync(pressed);
}

void testApp::setVideoPosition(float & position){
	player.setPosition(position);
}

//--------------------------------------------------------------
void testApp::update(){
	video->update();

	if(video->isFrameNew()){
		if(!found){
			currentFaceTracker1 = &threadedFaceTracker1;
			currentFaceTracker2 = &threadedFaceTracker2;
		}else{
			currentFaceTracker1 = &threadedFaceTracker1;
			currentFaceTracker2 = &threadedFaceTracker2;
		}
		#pragma omp parallel sections num_threads(2)
		{
			#pragma omp section
			{
				video->getPixelsRef().cropTo(half1,0,0,640,720);
				currentFaceTracker1->update(toCv(half1));
			}

			#pragma omp section
			{
				video->getPixelsRef().cropTo(half2,640,0,640,720);
				currentFaceTracker2->update(toCv(half2));
			}
		}

		found1 = currentFaceTracker1->getFound();
		found2 = currentFaceTracker2->getFound();

		found = false;
		if(currentFaceTracker1->getFound() && currentFaceTracker2->getFound()){
			found = true;
			mouthOpenDetector1.update();
			mouthOpenDetector2.update();
			float now = ofGetElapsedTimeMillis();
			if(lastTimeFace==0){
				lastTimeFace = now;
			}else if (now - lastTimeFace>noSwapMS){
				ofxEasingQuart easing;
				int s = ofxTween::map(now-lastTimeFace,0,rampStrenghtMS,0,maxStrength,true,easing,ofxTween::easeIn);
				clone1.strength = s;
				clone2.strength = s;
			}
			half1.update();
			half2.update();

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

			ofVec2f currentOrientation1(fabs(currentFaceTracker1->getOrientation().x),fabs(currentFaceTracker1->getOrientation().y));
			if(!updateOnLessOrientation || (currentOrientation1.x<lastOrientation1.x && currentOrientation1.y<lastOrientation1.y) || (lastOrientationMouthOpenness1 > mouthOpenDetector1.getOpennes())){
				half1Src.getPixelsRef() = half1.getPixelsRef();
				half1Src.update();
				mesh2.getTexCoords() = currentFaceTracker1->getImagePoints();
				lastOrientation1 = currentOrientation1;
				lastOrientationMouthOpenness1 = mouthOpenDetector1.getOpennes();
			}

			ofVec2f currentOrientation2(fabs(currentFaceTracker2->getOrientation().x),fabs(currentFaceTracker2->getOrientation().y));
			if(!updateOnLessOrientation || (currentOrientation2.x<lastOrientation2.x && currentOrientation2.y<lastOrientation2.y) || (lastOrientationMouthOpenness2 > mouthOpenDetector2.getOpennes())){
				half2Src.getPixelsRef() = half2.getPixelsRef();
				half2Src.update();
				mesh1.getTexCoords() = currentFaceTracker2->getImagePoints();
				lastOrientation2 = currentOrientation2;
				lastOrientationMouthOpenness2 = mouthOpenDetector2.getOpennes();
			}

			mask1Fbo.begin();
			ofClear(0, 255);
			mesh1.draw();
			mask1Fbo.end();

			mask2Fbo.begin();
			ofClear(0, 255);
			mesh2.draw();
			mask2Fbo.end();

			src1Fbo.begin();
			ofClear(0, 255);
			half2Src.getTextureReference().bind();
			mesh1.draw();
			half2Src.getTextureReference().unbind();
			src1Fbo.end();

			src2Fbo.begin();
			ofClear(0, 255);
			half1Src.getTextureReference().bind();
			mesh2.draw();
			half1Src.getTextureReference().unbind();
			src2Fbo.end();

			clone1.update(src1Fbo.getTextureReference(), half1.getTextureReference(), mesh1, mask1Fbo.getTextureReference());
			clone2.update(src2Fbo.getTextureReference(), half2.getTextureReference(), mesh2, mask2Fbo.getTextureReference());

		}else{
			lastOrientation1.set(359,359);
			lastOrientation2.set(359,359);
			lastTimeFace = 0;
		}
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
		mesh1.drawWireframe();
	}
	if(drawMesh2){
		ofPushMatrix();
		ofTranslate(640,0);
		mesh2.drawWireframe();
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
