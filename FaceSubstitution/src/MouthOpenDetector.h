/*
 * MouthOpenDetector.h
 *
 *  Created on: 09/03/2012
 *      Author: arturo
 */

#ifndef MOUTHOPENDETECTOR_H_
#define MOUTHOPENDETECTOR_H_

#include <deque>
#include "ofPolyline.h"
#include "ofxFaceTracker.h"
#include "Graph.h"

class MouthOpenDetector {
public:
	MouthOpenDetector();

	void setup(ofxFaceTracker * tracker);
	void update();
	float getOpennes();
	bool isOpenned();

	void reset();

	deque<float> getHistory();

	static string LOG_NAME;


	Graph graphArea;
	Graph graphDerivative;
	Graph graphBool;

private:
	ofxFaceTracker * tracker;
	ofxFaceTracker::Feature outer_mouth, inner_mouth;
	ofMesh mesh;
	deque<float> latestMouthOpennes;
	bool mouthOpenned;
	float prevArea;
};

#endif /* MOUTHOPENDETECTOR_H_ */
