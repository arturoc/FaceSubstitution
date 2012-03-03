/*
 * FaceBlinkRecorder.cpp
 *
 *  Created on: 03/02/2012
 *      Author: arturo
 */

#include "FaceBlinkRecorder.h"
#include "ofUtils.h"

string FaceBlinkRecorder::LOG_NAME = "FaceBlinkRecorder";

FaceBlinkRecorder::FaceBlinkRecorder() {
	// TODO Auto-generated constructor stub

}

FaceBlinkRecorder::~FaceBlinkRecorder() {
	// TODO Auto-generated destructor stub
}

void FaceBlinkRecorder::setup(ofxFaceTrackerThreaded & _tracker){
	tracker = &_tracker;
	prevFound = 0;
	firstEyesClosed = 0;
	recording = false;
	eyesClosed = false;
	startThread(true,false);
}

void FaceBlinkRecorder::update(ofPixels & frame){
	if(!recording && tracker->getFound() && ofGetFrameNum() - prevFound>240){
		ofLogVerbose(LOG_NAME) << "face found after" << ofGetFrameNum() - prevFound << "frames starting recording";
		recording = true;
		recorder.setup("recordings/" + ofGetTimestampString()+".mp4",frame.getWidth(),frame.getHeight(),30);
		ofNotifyEvent(recordingE,recording);
	}else if(recording){
		if(eyesClosed && ofGetFrameNum()-firstEyesClosed>30){
			ofLogVerbose(LOG_NAME) << "eyes closed for" << ofGetFrameNum() - firstEyesClosed << "frames encoding video";
			encodeVideo.signal();
			recording = false;
			ofNotifyEvent(recordingE,recording);
		}else if(!tracker->getFound() && ofGetFrameNum() - prevFound>10){
			ofLogVerbose(LOG_NAME) << "face lost for" << ofGetFrameNum() - prevFound << "frames dropping video";
			recorder.discard();
			recording = false;
			ofNotifyEvent(recordingE,recording);
		}
	}

	if(tracker->getFound()){
		prevFound = ofGetFrameNum();
	}


	if(recording){
		recorder.addFrame(frame);
	}
}

void FaceBlinkRecorder::threadedFunction(){
	while(isThreadRunning()){
		encodeVideo.wait(mutex);
		recorder.encodeVideo();
	}
}


void FaceBlinkRecorder::setEyesClosed(bool closed){
	if(!eyesClosed && closed){
		firstEyesClosed = ofGetFrameNum();
	}
	eyesClosed = closed;
}
