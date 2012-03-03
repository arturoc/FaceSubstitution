/*
 * FaceBlinkRecorder.cpp
 *
 *  Created on: 03/02/2012
 *      Author: arturo
 */

#include "FaceBlinkRecorder.h"
#include "ofUtils.h"
using namespace ofxPm;

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
	buffer.setup(*this,5*30);
	oneSec = ofGetElapsedTimef();
	fps = 30;
	startThread(true,false);
}

void FaceBlinkRecorder::update(ofPixels & frame){
	float t = ofGetElapsedTimef();
	framesOneSec++;
	if(t-oneSec >= 1){
		fps = float(framesOneSec)/(t-oneSec);
		framesOneSec=0;
		oneSec = t;
	}
	if(!recording && tracker->getFound() && ofGetFrameNum() - prevFound >= fps*10){
		ofLogVerbose(LOG_NAME) << "face found after" << ofGetFrameNum() - prevFound << "frames starting recording";
		recording = true;
		ofNotifyEvent(recordingE,recording);
	}else if(recording){
		if(eyesClosed && ofGetFrameNum()-firstEyesClosed >= fps*1){
			ofLogVerbose(LOG_NAME) << "eyes closed for" << ofGetFrameNum() - firstEyesClosed << "frames encoding video";
			encodeVideo.signal();
			recording = false;
			ofNotifyEvent(recordingE,recording);
		}else if(!tracker->getFound() && ofGetFrameNum() - prevFound >= fps){
			ofLogVerbose(LOG_NAME) << "face lost for" << ofGetFrameNum() - prevFound << "frames dropping video";
			buffer.clear();
			recording = false;
			ofNotifyEvent(recordingE,recording);
		}
	}

	if(tracker->getFound()){
		prevFound = ofGetFrameNum();
	}


	if(recording){
		VideoFrame * vframe = VideoFrame::newVideoFrame(frame);
		newFrameEvent.notify(this,*vframe);
		vframe->release();
	}

	pixels = &frame;
}

void FaceBlinkRecorder::threadedFunction(){
	while(isThreadRunning()){
		encodeVideo.wait(mutex);
		ofLogVerbose(LOG_NAME) << "start recording" << buffer.size() << "frames at " << fps << "fps";
		encoding = true;
		recorder.setup("recordings/" + ofGetTimestampString()+".mp4",pixels->getWidth(),pixels->getHeight(),fps,false);
		for(int i=0;i<buffer.size();i++){
			VideoFrame * frame = buffer.getVideoFrame(i);
			recorder.addFrame(frame->getPixelsRef());
			frame->release();
		}
		buffer.clear();
		pixels = NULL;
		recorder.encodeVideo();
		encoding = false;
	}
}


void FaceBlinkRecorder::setEyesClosed(bool closed){
	if(!eyesClosed && closed){
		firstEyesClosed = ofGetFrameNum();
	}
	eyesClosed = closed;
}

VideoFrame * FaceBlinkRecorder::getNextVideoFrame(){
    VideoFrame * frame = VideoFrame::newVideoFrame(*pixels);
    return frame;
}

int FaceBlinkRecorder::getFps(){
	return fps;
}

string FaceBlinkRecorder::getState(){
	if(recording){
		return "recording";
	}else if(encoding){
		return "encoding";
	}else{
		return "stopped";
	}
}
