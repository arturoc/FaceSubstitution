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
	vframe = NULL;

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
	if(!recording && tracker->getFound() && t - prevFound >= 10){
		ofLogVerbose(LOG_NAME) << "face found after" << t - prevFound << "seconds starting recording";
		recording = true;
		ofNotifyEvent(recordingE,recording);
	}else if(recording){
		if(eyesClosed && t-firstEyesClosed >= .6){
			ofLogVerbose(LOG_NAME) << "eyes closed for" << t - firstEyesClosed << "seconds encoding video";
			encodeVideo.signal();
			recording = false;
			ofNotifyEvent(recordingE,recording);
		}else if(!tracker->getFound() && t - prevFound >= 1){
			ofLogVerbose(LOG_NAME) << "face lost for" << t - prevFound << "seconds dropping video";
			if(mutex.tryLock()){
				buffer.clear();
				mutex.unlock();
			}
			recording = false;
			ofNotifyEvent(recordingE,recording);
		}
	}

	if(tracker->getFound()){
		prevFound = t;
	}


	if(recording && mutex.tryLock()){
		if(vframe) vframe->release();
		vframe = VideoFrame::newVideoFrame(frame);
		newFrameEvent.notify(this,*vframe);
		mutex.unlock();
	}
}

void FaceBlinkRecorder::threadedFunction(){
	while(isThreadRunning()){
		encodeVideo.wait(mutex);
		ofLogVerbose(LOG_NAME) << "start recording" << buffer.size() << "frames at " << fps << "fps";
		encoding = true;
		recorder.setup("recordings/" + ofGetTimestampString()+".mp4",vframe->getWidth(),vframe->getHeight(),fps,false);
		for(int i=0;i<(int)buffer.size();i++){
			VideoFrame * frame = buffer.getVideoFrame(i);
			recorder.addFrame(frame->getPixelsRef());
			frame->release();
		}
		buffer.clear();
		vframe->release();
		vframe = NULL;
		recorder.encodeVideo();
		encoding = false;
	}
}


void FaceBlinkRecorder::setEyesClosed(bool closed){
	if(!eyesClosed && closed){
		firstEyesClosed = ofGetElapsedTimef();
	}
	eyesClosed = closed;
}

VideoFrame * FaceBlinkRecorder::getNextVideoFrame(){
    return vframe;
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
