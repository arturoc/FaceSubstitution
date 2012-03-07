/*
 * Gui.cpp
 *
 *  Created on: 03/03/2012
 *      Author: arturo
 */

#include "Gui.h"
#include "ofxCv.h"

using namespace ofxCv;

Gui::Gui() {
	// TODO Auto-generated constructor stub

}

Gui::~Gui() {
	// TODO Auto-generated destructor stub
}

void Gui::setup(FaceLoader * _faceLoader, BlinkTrigger * _trigger, ofMesh * _camMesh, ofxFaceTrackerThreaded * _tracker, VideoFader * _videoFader, FaceBlinkRecorder * _faceBlinkRecorder, AutoExposure * _autoExposure, ofTexture * _input, ofxParameter<int> numInputRotations){
	faceLoader = _faceLoader;
	blinkTrigger = _trigger;
	camMesh = _camMesh;
	tracker = _tracker;
	videoFader = _videoFader;
	faceBlinkRecorder = _faceBlinkRecorder;
	autoExposure = _autoExposure;
	input = _input;

	gui.setup("face substitution");
	gui.add(faceLoaderMode.setup("face loader seq/rnd",false));
	gui.add(showGraphs.setup("show graphs",false));
	gui.add(showMesh.setup("show mesh",false));
	gui.add(showVideos.setup("show videos",false));
	gui.add(showMugs.setup("show mugs",false));
	gui.add(showInput.setup("show input",false));
	gui.add(rotationCamera.setup("rot camera",numInputRotations,0,3));
	gui.add(rotationScreen.setup("rot screen",0,0,3));
	gui.add(millisLongBlink.setup("millis long blink",blinkTrigger->millisLongBlink,0,1000));
	gui.add(secsFaceLostBetweenRecordings.setup("secs between recordings",faceBlinkRecorder->secsFaceLostBetweenRecordings,0,20));
	gui.add(secsFaceLostToDropRecording.setup("secs drop recording",faceBlinkRecorder->secsFaceLostToDropRecording,0,5));
	gui.add(secsEyesClosedToEndRecording.setup("secs eyes closed finish rec",faceBlinkRecorder->secsEyesClosedToEndRecording,0,3));
	gui.add(secsToRecord.setup("secs to record",faceBlinkRecorder->secsToRecord,0,5));
	gui.add(fadeMillis.setup("fade millis",videoFader->fadeMillis,0,2000));
	gui.add(minExposure.setup("min exposure",autoExposure->minExposure,0,1000));
	gui.add(maxExposure.setup("max exposure",autoExposure->maxExposure,0,1000));
	gui.add(screenScale.setup("screen scale",1,0,1));
	gui.add(currentFace.setup("current face",faceLoader->getCurrentFace(),0,faceLoader->getTotalFaces()));
	gui.add(faderRemaining.setup("fader remaining",videoFader->getRemainingPct(),0,videoFader->getDuration()));
	gui.add(videoFps.setup("video fps",faceBlinkRecorder->getFps(),0,60));


	map<string,ofxV4L2Settings::Control>::iterator it;
	for(it=autoExposure->settings.controls.begin();it!=autoExposure->settings.controls.end();it++){
		gui.add(new ofxIntSlider(it->second.parameter.getName(),it->second.parameter,it->second.minimum,it->second.maximum));
	}

	faceLoaderMode.addListener(this,&Gui::faceLoaderModeChanged);
	rotationScreen.addListener(this,&Gui::rotationScreenChanged);
	rotationCamera.addListener(this,&Gui::rotationCameraChanged);

	ofAddListener(videoFader->stateChanged,this,&Gui::videoFaderStateChanged);

	gui.loadFromFile("settings.xml");

}

void Gui::faceLoaderModeChanged(bool & m){
	if(m){
		faceLoader->setMode(FaceLoader::Random);
	}else{
		faceLoader->setMode(FaceLoader::Sequential);
	}
}

void Gui::videoFaderStateChanged(VideoFader::State & state){
	gui.add(faderRemaining.setup("fader remaining",videoFader->getRemainingPct(),0,videoFader->getDuration()));
}

void Gui::update(){
	currentFace = faceLoader->getCurrentFace();
	faderRemaining = videoFader->getRemainingPct();
	videoFps = faceBlinkRecorder->getFps();
}

void Gui::rotationScreenChanged(int & rot){
	if(rot==0){
		ofSetOrientation(OF_ORIENTATION_DEFAULT);
	}else if(rot==1){
		ofSetOrientation(OF_ORIENTATION_90_LEFT);
	}else if(rot==2){
		ofSetOrientation(OF_ORIENTATION_180);
	}else if(rot==3){
		ofSetOrientation(OF_ORIENTATION_90_RIGHT);
	}
}

void Gui::rotationCameraChanged(int & rot){
}

void Gui::draw(){
	gui.draw();

	if(showGraphs){
		int x = ofGetWidth()-260;
		blinkTrigger->leftBD.graphArea.draw(x,10,50);
		blinkTrigger->leftBD.graphDerivative.draw(x,60,50);
		blinkTrigger->leftBD.graphBool.draw(x,110,50);
		blinkTrigger->rightBD.graphArea.draw(x+130,10,50);
		blinkTrigger->rightBD.graphDerivative.draw(x+130,60,50);
		blinkTrigger->rightBD.graphBool.draw(x+130,110,50);
	}

	if(showMesh){
		camMesh->clearTexCoords();
		camMesh->drawWireframe();
	}

	if(showMugs){
		float ratio = faceLoader->getCurrentImg().getWidth() / faceLoader->getCurrentImg().getHeight();
		faceLoader->getCurrentImg().draw(ofGetWidth()-ofGetWidth()/3,ofGetHeight()-ofGetWidth()/3/ratio,ofGetWidth()/3,ofGetWidth()/3/ratio);
	}

	if(showInput){
		float ratio = input->getWidth() / input->getHeight();
		input->draw(ofGetWidth()-ofGetWidth()/3,10,ofGetWidth()/3,ofGetWidth()/3/ratio);
	}

	if(!tracker->getFound()) {
		drawHighlightString("camera face not found", 10, gui.getHeight() + 20);
	}
	if(!faceLoader->getTracker().getFound()) {
		drawHighlightString("image face not found", 10, gui.getHeight() + 40);
	}

	drawHighlightString("recorder state " + faceBlinkRecorder->getState(), 10, gui.getHeight() + 60);

	drawHighlightString("mouse: " + ofToString(ofGetMouseX())+","+ofToString(ofGetMouseY()),10, gui.getHeight() + 80);

	drawHighlightString("fps: " + ofToString(ofGetFrameRate()),10, gui.getHeight() + 100);
}
