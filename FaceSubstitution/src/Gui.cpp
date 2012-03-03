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

void Gui::setup(FaceLoader * _faceLoader, BlinkDetector * _leftBd, BlinkDetector * _rightBd, ofMesh * _camMesh, ofxFaceTrackerThreaded * _tracker, VideoFader * _videoFader, FaceBlinkRecorder * _faceBlinkRecorder, AutoExposure * _autoExposure, ofxParameter<int> numInputRotations){
	faceLoader = _faceLoader;
	leftBD = _leftBd;
	rightBD = _rightBd;
	camMesh = _camMesh;
	tracker = _tracker;
	videoFader = _videoFader;
	faceBlinkRecorder = _faceBlinkRecorder;
	autoExposure = _autoExposure;

	gui.setup("face substitution");
	gui.add(faceLoaderMode.setup("face loader seq/rnd",false));
	gui.add(showGraphs.setup("show graphs",false));
	gui.add(showMesh.setup("show mesh",false));
	gui.add(showVideos.setup("show videos",false));
	gui.add(showMugs.setup("show mugs",false));
	gui.add(rotation.setup("rotation",numInputRotations,0,3));
	gui.add(currentFace.setup("current face",faceLoader->getCurrentFace(),0,faceLoader->getTotalFaces()));
	gui.add(faderRemaining.setup("fader remaining",videoFader->getRemainingPct(),0,videoFader->getDuration()));
	gui.add(videoFps.setup("video fps",faceBlinkRecorder->getFps(),0,60));


	map<string,ofxV4L2Settings::Control>::iterator it;
	for(it=autoExposure->settings.controls.begin();it!=autoExposure->settings.controls.end();it++){
		gui.add(new ofxIntSlider(it->second.parameter.getName(),it->second.parameter,it->second.minimum,it->second.maximum));
	}

	faceLoaderMode.addListener(this,&Gui::faceLoaderModeChanged);

	ofAddListener(videoFader->stateChanged,this,&Gui::videoFaderStateChanged);

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

void Gui::draw(){
	if(rotation==0){
		ofSetOrientation(OF_ORIENTATION_DEFAULT);
	}else if(rotation==1){
		ofSetOrientation(OF_ORIENTATION_90_RIGHT);
	}else if(rotation==2){
		ofSetOrientation(OF_ORIENTATION_180);
	}else if(rotation==3){
		ofSetOrientation(OF_ORIENTATION_90_LEFT);
	}
	ofPushView();
	ofSetupScreenPerspective();

	gui.draw();

	if(showGraphs){
		int x = ofGetWidth()-260;
		leftBD->graphArea.draw(x,10,50);
		leftBD->graphDerivative.draw(x,60,50);
		leftBD->graphBool.draw(x,110,50);
		rightBD->graphArea.draw(x+130,10,50);
		rightBD->graphDerivative.draw(x+130,60,50);
		rightBD->graphBool.draw(x+130,110,50);
	}

	if(showMesh){
		camMesh->clearTexCoords();
		camMesh->drawWireframe();
	}

	if(showMugs){
		float ratio = faceLoader->getCurrentImg().getWidth() / faceLoader->getCurrentImg().getHeight();
		faceLoader->getCurrentImg().draw(ofGetWidth()-120,ofGetHeight()-120./ratio,120,120./ratio);
	}

	if(!tracker->getFound()) {
		drawHighlightString("camera face not found", 10, gui.getHeight() + 20);
	}
	if(!faceLoader->getTracker().getFound()) {
		drawHighlightString("image face not found", 10, gui.getHeight() + 40);
	}

	drawHighlightString("recorder state " + faceBlinkRecorder->getState(), 10, gui.getHeight() + 60);

	ofPopView();
	ofSetOrientation(OF_ORIENTATION_DEFAULT);
}
