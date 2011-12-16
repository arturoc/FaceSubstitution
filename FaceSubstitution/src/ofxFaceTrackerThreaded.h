#pragma once

#include "ofxFaceTracker.h"
#include "ofEvents.h"

class ofxFaceTrackerThreaded : public ofThread {
public:
	void setup();
	void update(cv::Mat mat);
	void draw();

	bool isFrameNew();

	bool getFound();
	ofMesh getImageMesh();
	float getGesture(ofxFaceTracker::Gesture gesture);

	ofPolyline getImageFeature(ofxFaceTracker::Feature feature);
	ofPolyline getObjectFeature(ofxFaceTracker::Feature feature);

	ofEvent<ofEventArgs> threadedUpdateE;

protected:
	void threadedFunction();
	bool isRunningOnThread();
	
	ofMesh mesh;
	ofMutex meshMutex;
	ofxFaceTracker tracker;
	cv::Mat buffer;
	Poco::Condition newFrame;
	bool frameProcessed;
	unsigned int threadId;
};
