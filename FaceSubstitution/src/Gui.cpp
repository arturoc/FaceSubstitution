/*
 * Gui.cpp
 *
 *  Created on: Sep 8, 2013
 *      Author: arturo
 */

#include "Gui.h"

ofOrientation getOrientation(int numRotations){
	switch(numRotations){
	case 0:
		return OF_ORIENTATION_DEFAULT;
	case 1:
		return OF_ORIENTATION_90_LEFT;
	case 2:
		return OF_ORIENTATION_180;
	case 3:
		return OF_ORIENTATION_90_RIGHT;
	default:
		return OF_ORIENTATION_UNKNOWN;
	}
}



void Gui::setup(AutoExposure & autoExposure, Clone & clone, ofVideoGrabber & cam){
	parameters.setName("Faces");
	parameters.add(autoExposure.settings.parameters);
	parameters.add(autoExposure.maxExposure);
	parameters.add(autoExposure.minExposure);
	parameters.add(clone.strength);
	parameters.add(show.set("show",false));
	parameters.add(numRotations.set("numRotations",1,0,3));

	autoExposure.settings["Exposure (Absolute)"].setSerializable(false);

	gui.setup(parameters);

	oscParamSync.setup(parameters,8024,"laptop.local",8025);

	this->autoExposure = &autoExposure;
	this->clone = &clone;
	this->cam = &cam;
}

void Gui::update(const ofRectangle & bb){
	oscParamSync.update();
	autoExposureBB = bb;
}


void Gui::newCamProcessFrame(){
	camFPS.newFrame();
}

void Gui::newCamFrame(){
	camRealFPS.newFrame();
}

void Gui::draw(){
	if(!show) return;
	ofPushView();
	ofSetOrientation(getOrientation(numRotations),true);
	ofSetupScreenPerspective();

	ofSetColor(255);
	gui.draw();

	ofSetColor(0);
	ofDrawBitmapString("app: " + ofToString((int)ofGetFrameRate()),ofGetWidth()-220,20);
	ofDrawBitmapString("cam: " + ofToString((int)camFPS.getFPS()),ofGetWidth()-220,40);
	ofDrawBitmapString("cam real: " + ofToString((int)camRealFPS.getFPS()),ofGetWidth()-220,60);
	ofDrawBitmapString("exp: " + ofToString((int)autoExposure->settings["Exposure (Absolute)"]),ofGetWidth()-220,80);


	ofSetOrientation(OF_ORIENTATION_DEFAULT);
	ofPopView();

	ofSetColor(ofColor::magenta);
	ofPushMatrix();
	ofTranslate(ofGetWidth(),0);
	ofScale(-ofGetWidth()/cam->getWidth(),ofGetWidth()/cam->getWidth(),1);
	ofNoFill();
	ofRect(autoExposureBB);
	ofFill();
	ofPopMatrix();
	ofSetColor(255);
}
