#pragma once

#include "ofxFaceTracker.h"

class ofxFaceTrackerThreaded : public ofThread {
public:
	void setup() {
		tracker.setup();
		startThread(true, false);
	}
	void update(cv::Mat mat) {
		if(mutex.tryLock()){
			ofxCv::copy(mat, buffer);
			frameProcessed = false;
			unlock();
			newFrame.signal();
		}
	}
	void draw() {
		lock();
			tracker.draw();
		unlock();
	}
	bool getFound() {
		return tracker.getFound();
	}
	ofMesh getImageMesh() {
		Poco::ScopedLock<ofMutex> lock(meshMutex);
		return mesh;
	}

	float getGesture(ofxFaceTracker::Gesture gesture){
		if(mutex.tryLock()){
			float gest =  tracker.getGesture(gesture);
			mutex.unlock();
			return gest;
		}else{
			return 0;
		}
	}

	bool isFrameNew(){
		if(frameProcessed){
			frameProcessed = false;
			return true;
		}else{
			return false;
		}
	}

	ofPolyline getImageFeature(ofxFaceTracker::Feature feature){
		Poco::ScopedLock<ofMutex> lock(mutex);
		return tracker.getImageFeature(feature);
	}

	ofPolyline getObjectFeature(ofxFaceTracker::Feature feature){
		Poco::ScopedLock<ofMutex> lock(mutex);
		return tracker.getObjectFeature(feature);
	}
protected:
	void threadedFunction() {
		mutex.lock();
		while(isThreadRunning()) {
			newFrame.wait(mutex);
			tracker.update(buffer);
			mesh = tracker.getImageMesh();
			frameProcessed = true;
		}
	}
	
	ofMesh mesh;
	ofMutex meshMutex;
	ofxFaceTracker tracker;
	cv::Mat buffer;
	Poco::Condition newFrame;
	bool frameProcessed;
};
