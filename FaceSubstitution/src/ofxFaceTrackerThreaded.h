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

	ofEvent<ofEventArgs> threadedUpdateE;

	ofxFaceTracker * getTracker();

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
