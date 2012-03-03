/*
 * BlinkDetector.h
 *
 *  Created on: 15/12/2011
 *      Author: arturo
 */

#ifndef BLINKDETECTOR_H_
#define BLINKDETECTOR_H_

#include <deque>
#include "ofPolyline.h"
#include "ofxFaceTracker.h"
#include "Graph.h"

class BlinkDetector {
public:
	BlinkDetector();

	void setup(ofxFaceTracker * tracker, ofxFaceTracker::Feature eye);
	void update();
	float getOpennes();
	bool isClosed();

	void reset();

	deque<float> getHistory();

	static string LOG_NAME;


	Graph graphArea;
	Graph graphDerivative;
	Graph graphBool;

private:
	ofxFaceTracker * tracker;
	ofxFaceTracker::Feature eye;
	ofTessellator tess;
	ofMesh mesh;
	deque<float> latestEyeOpennes;
	bool eyeClosed;
	float prevArea;
};

#endif /* BLINKDETECTOR_H_ */
