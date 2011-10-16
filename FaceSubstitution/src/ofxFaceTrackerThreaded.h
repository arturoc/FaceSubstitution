#pragma once

#include "ofxFaceTracker.h"

class ofxFaceTrackerThreaded : public ofThread {
public:
	void setup() {
		startThread(false, false);
	}
	void update(cv::Mat mat) {
		if(lock()) {
			ofxCv::copy(mat, buffer);
			unlock();
			newFrame = true;
			ofSleepMillis(30); // give the tracker a moment
		}
	}
	void draw() {
		if(lock()) {
			tracker->draw();
			unlock();
		}
	}
	bool getFound() {
		bool found = false;
		if(lock()) {
			found = tracker->getFound();
			unlock();
		}
		return found;
	}
	ofMesh getImageMesh() {
		ofMesh imageMesh;
		if(lock()) {
			imageMesh = tracker->getImageMesh();
			unlock();
		}
		return imageMesh;
	}
protected:
	void threadedFunction() {
		newFrame = false;
		tracker = new ofxFaceTracker();
		tracker->setup();
		while(isThreadRunning()) {
			if(newFrame) {
				if(lock()) {
					newFrame = false;
					tracker->update(buffer);
					unlock();
				}
			}
			ofSleepMillis(1);
		}
	}
	
	ofxFaceTracker* tracker;
	cv::Mat buffer;
	bool newFrame;
};