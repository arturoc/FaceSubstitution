/*
 * ofxFaceTrackerThreaded.cpp
 *
 *  Created on: 16/12/2011
 *      Author: arturo
 */

#include "ofxFaceTrackerThreaded.h"

void ofxFaceTrackerThreaded::setup() {
	tracker.setup();
	startThread(true, false);
}

void ofxFaceTrackerThreaded::update(cv::Mat mat) {
	if(!isRunningOnThread()){
		if(!tracker.getFound() && mutex.tryLock()){
			ofxCv::copy(mat, buffer);
			frameProcessed = false;
			unlock();
			newFrame.signal();
		}else if(tracker.getFound()){
			mutex.lock();
			ofxCv::copy(mat, buffer);
			frameProcessed = false;
			unlock();
			newFrame.signal();
		}
	}else{
		ofxCv::copy(mat, buffer);
		frameProcessed = false;
	}
}

void ofxFaceTrackerThreaded::draw() {
	if(!isRunningOnThread()) lock();
		tracker.draw();
	if(!isRunningOnThread()) unlock();
}

bool ofxFaceTrackerThreaded::getFound() {
	return tracker.getFound();
}

ofMesh ofxFaceTrackerThreaded::getImageMesh() {
	if(!isRunningOnThread()){
		Poco::ScopedLock<ofMutex> lock(meshMutex);
		return mesh;
	}else{
		return mesh;
	}
}

bool ofxFaceTrackerThreaded::isFrameNew(){
	if(frameProcessed){
		frameProcessed = false;
		return true;
	}else{
		return false;
	}
}

void ofxFaceTrackerThreaded::threadedFunction() {
	ofEventArgs args;
	threadId = (unsigned int)pthread_self();
	mutex.lock();
	while(isThreadRunning()) {
		newFrame.wait(mutex);
		tracker.update(buffer);
		mesh = tracker.getImageMesh();
		frameProcessed = true;
		ofNotifyEvent(threadedUpdateE,args);
	}
}

bool ofxFaceTrackerThreaded::isRunningOnThread(){
	return threadId == (unsigned int)pthread_self();
}

ofxFaceTracker * ofxFaceTrackerThreaded::getTracker(){
	return &tracker;
}
