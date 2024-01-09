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
	parameters.add(save.set("save",false));
	parameters.add(numRotations.set("numRotations",1,0,3));


	show.setSerializable(false);
	save.setSerializable(false);

	autoExposure.settings["Exposure (Absolute)"].setSerializable(false);
	autoExposure.settings["Focus, Auto"] = 0;
	autoExposure.settings["Focus (absolute)"] = 3;


	gui.setup(parameters);

    //oscParamSync.setup(parameters,8024,"laptop.local",8025);

    listener = clone.strength.newListener([](int & strength){
        std::cout << strength << std::endl;
    });

	gui.loadFromFile("settings.xml");

	save = false;
	show = false;

    save.addListener(this, &Gui::onSave);

	this->autoExposure = &autoExposure;
	this->clone = &clone;
	this->cam = &cam;

    font.load("Ubuntu Mono", 18);
}

void Gui::onSave(bool & save){
	if(save){
		gui.saveToFile("settings.xml");
		this->save = false;
	}
}

void Gui::update(const ofRectangle & bb){
    //oscParamSync.update();
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
    //ofPushView();
    //ofSetOrientation(getOrientation(numRotations),true);
    //ofSetupScreenPerspective();

	ofSetColor(255);
	gui.draw();

	ofSetColor(50);
	font.drawString("app: " + ofToString((int)ofGetFrameRate()),ofGetWidth()-140,45);
	font.drawString("proc: " + ofToString((int)camFPS.getFPS()),ofGetWidth()-140,80);
	font.drawString("cam: " + ofToString((int)camRealFPS.getFPS()),ofGetWidth()-140,115);
	font.drawString("exp: " + ofToString((int)autoExposure->settings["Exposure (Absolute)"]),ofGetWidth()-140,150);

	float imgWidth;
	if(numRotations==1 || numRotations==3){
		imgWidth = cam->getHeight();
	}else{
		imgWidth = cam->getWidth();
	}

	ofSetColor(ofColor::magenta);
	ofPushMatrix();
	ofTranslate(ofGetWidth(),0);
	ofScale(-ofGetWidth()/imgWidth,ofGetWidth()/imgWidth,1);
	ofNoFill();
	ofRect(autoExposureBB);
	ofFill();
	ofPopMatrix();
	ofSetColor(255);


    //ofSetOrientation(OF_ORIENTATION_DEFAULT);
    //ofPopView();
}
