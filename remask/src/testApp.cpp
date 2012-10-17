#include "testApp.h"
#include "ofxFaceTracker.h"
#include "ofxTween.h"

using namespace ofxCv;

#define USE_PLAYER

//--------------------------------------------------------------
void testApp::setup(){
	bool usePlayer = false;
	if(usePlayer){
		player.loadMovie("recordings_interaction/2012-10-15-12-54-49-971.mov");
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
	faceTracker1.setIterations(15);
	faceTracker2.setIterations(15);


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
	dottedLineSegments.addListener(this,&testApp::dottedLineSegmentsChanged);
	dottedLineWidth.addListener(this,&testApp::dottedLineWidthChanged);

	exposure.setup(0,1280,720);

	clone1.strength=0;
	clone2.strength=0;
	gui.add(clone1.strength);
	gui.add(clone2.strength);
	gui.add(updateOnLessOrientation.set("updateOnLessOrientation",false));
	gui.add(thresholdFaceRot.set("thresholdFaceRot",10,0,90));

	gui.add(videoPosition.set("videoPosition",0,0,1));
	gui.add(vSync.set("vSync",true));
	gui.add(showSecondScreen.set("showSecondScreen",false));
	gui.add(showDebug.set("showDebug",false));
	gui.add(drawMesh1.set("drawMesh1",false));
	gui.add(drawMesh2.set("drawMesh2",false));
	gui.add(found1.set("found1",false));
	gui.add(found2.set("found2",false));
	gui.add(rampStrenghtMS.set("rampStrenghtMS",1500,0,5000));
	gui.add(noSwapMS.set("noSwapMS",2500,0,10000));
	gui.add(showWireMS.set("showWireMS",1500,0,10000));

	gui.add(maxStrength.set("maxStrength",20,0,100));
	gui.add(dottedLineSegments.set("dottedLineSegments",30,0,100));
	gui.add(dottedLineWidth.set("dottedLineWidth",4,0,15));
	gui.add(dottedLineColor.set("dottedLineColor",ofColor(255,188,0),ofColor(0,0),ofColor(255)));
	gui.add(meshColor.set("meshColor",ofColor(255,188,0,100),ofColor(0,0),ofColor(255)));

	gui.add(exposure.maxExposure);
	gui.add(exposure.minExposure);
	gui.add(exposure.settings.parameters);


	videoPosition.addListener(this,&testApp::setVideoPosition);
	mouthOpenDetector1.setup(&faceTracker2.tracker);
	mouthOpenDetector2.setup(&faceTracker2.tracker);

	lastOrientation1.set(359,359);
	lastOrientation2.set(359,359);

	meshesInitialized = 0;
	lastTimeFaceFound = 0;
	found = false;
	newFrame = false;
	playerRecorderShutdown = false;
	videoFrame = 0;
	showGui = false;

	exposure.settings.set("Focus, Auto",0);
	exposure.settings.set("Focus (absolute)",60);
	exposure.settings.set("Zoom, Absolute",2);

	ofEnableAlphaBlending();
	ofSetFullscreen(true);
	ofHideCursor();
	ofBackground(0);
	//ofSetFrameRate(30);
	//ofSetWindowPosition(0,-10);
	//ofSetWindowShape(1280+1024,768);
}

void testApp::dottedLineSegmentsChanged(int & segments){
	ofPath dottedLinePath;
	dottedLinePath.setStrokeWidth(0);
	float height = double(720)/double(segments*2);
	float heightEmpty = double(720-int(height)*segments)/double(segments-1);
	for(int i=0;i<segments+1;i++){
		dottedLinePath.moveTo(ofVec3f(1280*.5-dottedLineWidth*.5,height*i+heightEmpty*i));
		dottedLinePath.lineTo(ofVec3f(1280*.5+dottedLineWidth*.5,height*i+heightEmpty*i));
		dottedLinePath.lineTo(ofVec3f(1280*.5+dottedLineWidth*.5,height*(i+1)+heightEmpty*i));
		dottedLinePath.lineTo(ofVec3f(1280*.5-dottedLineWidth*.5,height*(i+1)+heightEmpty*i));
		dottedLinePath.close();
	}
	dottedLine = dottedLinePath.getTessellation();
}


void testApp::dottedLineWidthChanged(int & width){
	int segments = dottedLineSegments;
	dottedLineSegmentsChanged(segments);
}

void testApp::vSyncPressed(bool & pressed){
	ofSetVerticalSync(pressed);
}

void testApp::setVideoPosition(float & position){
	player.setPosition(position);
}

void testApp::newBuffer(ofPixels & buffer){
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
			faceTracker1.update(toCv(half1));
		}

		#pragma omp section
		{
			faceTracker2.update(toCv(half2));
		}
	}


	videoMutex.lock();
	found1 = faceTracker1.getFound();
	found2 = faceTracker2.getFound();
	found = found1 && found2;

	if(found && !meshesInitialized){
		mesh1 = faceTracker1.getImageMesh();
		mesh2 = faceTracker2.getImageMesh();
		meshesInitialized = true;
	}

	if(meshesInitialized && found1){
		mouthOpenDetector1.update();
		ofVec2f currentOrientation1(fabs(faceTracker1.getOrientation().x),fabs(faceTracker1.getOrientation().y));
		if((!updateOnLessOrientation && ofRadToDeg(currentOrientation1.y)<thresholdFaceRot)||
				(currentOrientation1.x<lastOrientation1.x && currentOrientation1.y<lastOrientation1.y) ||
				(lastOrientationMouthOpenness1 > mouthOpenDetector1.getOpennes())){
			half1Src.getPixelsRef() = half1.getPixelsRef();
			mesh2.getTexCoords() = faceTracker1.getImagePoints();
			lastOrientation1 = currentOrientation1;
			lastOrientationMouthOpenness1 = mouthOpenDetector1.getOpennes();
			half1NeedsUpdate = true;
		}
		mesh1.getVertices() = faceTracker1.getImageMesh().getVertices();
		mesh1.getIndices() = faceTracker1.getImageMesh().getIndices();
	}

	if(meshesInitialized && found2){
		mouthOpenDetector2.update();
		ofVec2f currentOrientation2(fabs(faceTracker2.getOrientation().x),fabs(faceTracker2.getOrientation().y));
		if((!updateOnLessOrientation && ofRadToDeg(currentOrientation2.y)<thresholdFaceRot) ||
				(currentOrientation2.x<lastOrientation2.x && currentOrientation2.y<lastOrientation2.y) ||
				(lastOrientationMouthOpenness2 > mouthOpenDetector2.getOpennes())){
			half2Src.getPixelsRef() = half2.getPixelsRef();
			mesh1.getTexCoords() = faceTracker2.getImagePoints();
			lastOrientation2 = currentOrientation2;
			lastOrientationMouthOpenness2 = mouthOpenDetector2.getOpennes();
			half2NeedsUpdate = true;
		}
		mesh2.getVertices() = faceTracker2.getImageMesh().getVertices();
		mesh2.getIndices() = faceTracker2.getImageMesh().getIndices();
	}


	newFrame = true;
	videoMutex.unlock();
}

//--------------------------------------------------------------
void testApp::update(){
	now = ofGetElapsedTimeMillis();
	video->update();

	videoMutex.lock();
	if(newFrame){
		newFrame = false;
		if(found || (vbosInitialized && lastTimeFaceFound>0 && (found1 || found2))){
			if(found && !vbosInitialized){
				vboMesh1 = mesh1;
				vboMesh1.setUsage(GL_DYNAMIC_DRAW);
				vboMesh2 = mesh2;
				vboMesh2.setUsage(GL_DYNAMIC_DRAW);
				vbosInitialized = true;
			}

			if(found1){
				half1.update();
				if(half1NeedsUpdate){
					half1Src.update();
					half1NeedsUpdate=false;
					vboMesh2.getTexCoords() = mesh2.getTexCoords();
				}
				vboMesh1.getVertices() = mesh1.getVertices();
				vboMesh1.getIndices() = mesh1.getIndices();
			}
			if(found2){
				half2.update();
				if(half2NeedsUpdate){
					half2Src.update();
					half2NeedsUpdate=false;
					vboMesh1.getTexCoords() = mesh1.getTexCoords();
				}
				vboMesh2.getVertices() = mesh2.getVertices();
				vboMesh2.getIndices() = mesh2.getIndices();
			}
			videoMutex.unlock();


			if(found){
				if(lastTimeFaceFound==0){
					recorder.setup(ofGetTimestampString()+".mov","",1280,720,30);
					lastTimeFaceFound = now;
					clone1.strength = 0;
					clone2.strength = 0;
				}else if(now-lastTimeFaceFound<showWireMS){
					interpolatedMesh1 = vboMesh1;
					interpolatedMesh2 = vboMesh2;
				}else if(now-lastTimeFaceFound>showWireMS && now-lastTimeFaceFound<noSwapMS+showWireMS){
					float pct = double(now-lastTimeFaceFound-showWireMS)/double(noSwapMS);
					interpolatedMesh1 = vboMesh1;
					for(int i=0;i<interpolatedMesh1.getNumVertices();i++){
						interpolatedMesh1.getVertices()[i].interpolate(vboMesh2.getVertices()[i],pct);
					}
					interpolatedMesh2 = vboMesh2;
					for(int i=0;i<interpolatedMesh2.getNumVertices();i++){
						interpolatedMesh2.getVertices()[i].interpolate(vboMesh1.getVertices()[i],pct);
					}
				}else if (now - lastTimeFaceFound - noSwapMS - showWireMS<rampStrenghtMS){
					interpolatedMesh1.getVertices()=vboMesh2.getVertices();
					interpolatedMesh2.getVertices()=vboMesh1.getVertices();
					ofxEasingQuart easing;
					int s = ofxTween::map(now-lastTimeFaceFound-noSwapMS-showWireMS,0,rampStrenghtMS,0,maxStrength,true,easing,ofxTween::easeIn);
					clone1.strength = s;
					clone2.strength = s;
				}
			}
			recorder.addFrame(video->getPixelsRef());


			if(found1){
				mask1Fbo.begin();
				ofClear(0, 255);
				vboMesh1.draw();
				mask1Fbo.end();

				src1Fbo.begin();
				ofClear(0, 255);
				half2Src.getTextureReference().bind();
				vboMesh1.draw();
				half2Src.getTextureReference().unbind();
				src1Fbo.end();

				clone1.update(src1Fbo.getTextureReference(), half1.getTextureReference(), mesh1, mask1Fbo.getTextureReference());
			}

			if(found2){
				mask2Fbo.begin();
				ofClear(0, 255);
				vboMesh2.draw();
				mask2Fbo.end();

				src2Fbo.begin();
				ofClear(0, 255);
				half1Src.getTextureReference().bind();
				vboMesh2.draw();
				half1Src.getTextureReference().unbind();
				src2Fbo.end();

				clone2.update(src2Fbo.getTextureReference(), half2.getTextureReference(), mesh2, mask2Fbo.getTextureReference());
			}

			if(videoFrame%30==0){
				if(found1){
					mask1Fbo.readToPixels(mask1);
					mask1.pasteInto(maskPixels,0,0);
					half1.getPixelsRef().pasteInto(pixelsCombined,0,0);
				}
				if(found2){
					mask2Fbo.readToPixels(mask2);
					mask2.pasteInto(maskPixels,0,0);
					half2.getPixelsRef().pasteInto(pixelsCombined,0,0);
				}
				if(video==&grabber){
					exposure.update(pixelsCombined,maskPixels);
				}
			}
			lastTimeFaceDetected = now;
		}else{
			videoMutex.unlock();
			if(lastTimeFaceFound!=0 && now-lastTimeFaceDetected>2000){
				lastOrientation1.set(359,359);
				lastOrientation2.set(359,359);
				lastTimeFaceFound = 0;
				recorder.close();
			}
		}
		videoFrame++;
	}else{
		videoMutex.unlock();
	}

	if(video==&player && !playerRecorderShutdown && player.getIsMovieDone()){
		playerRecorderShutdown = true;
		recorder.close();
	}
}

void testApp::drawOutput(){
	video->draw(1280,0,-1280,720);
	if(found  || (lastTimeFaceFound>0 && (found1 || found2))){
		if(found1) clone1.draw(1280,0,-640,720);
		if(found2) clone2.draw(640,0,-640,720);
		if(now -lastTimeFaceFound<showWireMS){
			float pct = double(now-lastTimeFaceFound)/double(showWireMS);
			pct*=pct;
			pct*=pct;
			ofSetColor(meshColor,meshColor->a*pct);
			ofPushMatrix();
			ofTranslate(1280,0);
			ofScale(-1,1);
			interpolatedMesh1.drawWireframe();
			ofPushMatrix();
			ofTranslate(640,0);
			interpolatedMesh2.drawWireframe();
			ofPopMatrix();
			ofPopMatrix();
		}else if (now - lastTimeFaceFound<noSwapMS+showWireMS){
			ofSetColor(meshColor);
			float pct = double(now-lastTimeFaceFound-showWireMS)/double(noSwapMS);
			pct*=pct;
			pct*=pct;
			ofPushMatrix();
			ofTranslate(1280,0);
			ofScale(-1,1);
			ofPushMatrix();
			ofTranslate(640*pct,0);
			interpolatedMesh1.drawWireframe();
			ofPopMatrix();
			ofPushMatrix();
			ofTranslate(640-640*pct,0);
			interpolatedMesh2.drawWireframe();
			ofPopMatrix();
			ofPopMatrix();
		}else if(now -lastTimeFaceFound>noSwapMS+showWireMS && now -lastTimeFaceFound<noSwapMS +showWireMS+ rampStrenghtMS){
			float pct = double(now-lastTimeFaceFound-noSwapMS-showWireMS)/double(rampStrenghtMS);
			pct *= pct;
			pct *= pct;
			ofSetColor(meshColor,meshColor->a-meshColor->a*pct);
			ofPushMatrix();
			ofTranslate(1280,0);
			ofScale(-1,1);
			ofPushMatrix();
			ofTranslate(640,0);
			interpolatedMesh1.drawWireframe();
			ofPopMatrix();
			interpolatedMesh2.drawWireframe();
			ofPopMatrix();
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	ofPushMatrix();
	ofTranslate((ofGetWidth()-1280)*.5,(ofGetHeight()-720)*.5);
	drawOutput();
	ofPopMatrix();
	if(showSecondScreen){
		ofPushMatrix();
		ofTranslate(1280,(768-720*1024./1280.)*.5);
		ofScale(1024./1280.,1024./1280.);
		drawOutput();
		ofPopMatrix();
	}

	ofSetColor(255);
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
		ofSetColor(meshColor);
		vboMesh1.drawWireframe();
	}
	if(drawMesh2){
		ofSetColor(meshColor);
		ofPushMatrix();
		ofTranslate(640,0);
		vboMesh2.drawWireframe();
		ofPopMatrix();
	}

	ofSetColor(dottedLineColor);
	dottedLine.draw();

	ofSetColor(255);
	if(showGui){
		ofDrawBitmapString(ofToString(ofGetFrameRate()),ofGetWidth()-100,20);
		if(faceTracker1.getFound()){
			ofVec3f degRot(ofRadToDeg(faceTracker1.getOrientation().x),ofRadToDeg(faceTracker1.getOrientation().y),ofRadToDeg(faceTracker1.getOrientation().z));
			ofDrawBitmapString(ofToString(degRot),ofGetWidth()-300,40);
		}

		gui.draw();
	}
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if(key=='g'){
		showGui = !showGui;
		if(!showGui){
			ofHideCursor();
		}else{
			ofShowCursor();
		}
	}
	if(key=='f'){
		ofToggleFullscreen();
	}
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
