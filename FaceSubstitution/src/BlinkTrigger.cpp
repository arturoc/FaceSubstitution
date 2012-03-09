/*
 * BlinkTrigger.cpp
 *
 *  Created on: 03/02/2012
 *      Author: arturo
 */

#include "BlinkTrigger.h"

BlinkTrigger::BlinkTrigger() {
	tracker = 0;

}

BlinkTrigger::~BlinkTrigger() {
	// TODO Auto-generated destructor stub
}

void BlinkTrigger::setup(ofxFaceTrackerThreaded & _tracker){
	tracker = &_tracker;

	leftBD.setup(tracker->getTracker(),ofxFaceTracker::LEFT_EYE);
	rightBD.setup(tracker->getTracker(),ofxFaceTracker::RIGHT_EYE);

	millisEyesClosed = 0;
	firstEyesClosedEvent = 0;
	millisLongBlink = 200;
	longBlinkTriggered = false;
	eyesClosed = false;

	ofAddListener(tracker->threadedUpdateE,this,&BlinkTrigger::threadedUpdate);
}

void BlinkTrigger::threadedUpdate(ofEventArgs & args){
	if(tracker->getFound()){
		leftBD.update();
		rightBD.update();

		if(leftBD.isClosed() && rightBD.isClosed() && !mouthDetector.isOpenned()){
			ofLogVerbose("testApp") << "eyesClosed" << millisEyesClosed;
			if(firstEyesClosedEvent==0){
				firstEyesClosedEvent = ofGetElapsedTimeMillis();
				eyesClosed = true;
				ofNotifyEvent(blinkE,eyesClosed);
			}
			millisEyesClosed = ofGetElapsedTimeMillis()-firstEyesClosedEvent;
			if(!longBlinkTriggered  && millisEyesClosed>millisLongBlink){
				longBlinkTriggered = true;
				ofNotifyEvent(longBlinkE,longBlinkTriggered);
			}
		}else{
			if(eyesClosed){
				eyesClosed = false;
				ofNotifyEvent(blinkE,eyesClosed);
			}
			if(longBlinkTriggered){
				longBlinkTriggered = false;
				ofNotifyEvent(longBlinkE,longBlinkTriggered);
			}
			millisEyesClosed = 0;
			firstEyesClosedEvent = 0;
		}
	}else{
		leftBD.reset();
		rightBD.reset();
	}
}

bool BlinkTrigger::areEyesClosed(){
	return eyesClosed;
}
