/*
 * BlinkDetector.cpp
 *
 *  Created on: 15/12/2011
 *      Author: arturo
 */

#include "BlinkDetector.h"

string BlinkDetector::LOG_NAME = "BlinkDetector";

BlinkDetector::BlinkDetector() {
	eyeClosed = false;

}

void BlinkDetector::setup(ofxFaceTracker * _tracker, ofxFaceTracker::Feature _eye){
	tracker = _tracker;
	eye = _eye;
}

void BlinkDetector::update(){
	const ofPolyline & eyeContour = tracker->getObjectFeature(eye);
	float area = eyeContour.getArea();
	float max=-1, min=99;

	deque<float>::iterator it;
	for(it=latestEyeOpennes.begin();it!=latestEyeOpennes.end();it++){
		if(max<*it) max=*it;
		if(min>*it) min=*it;
	}

	float faceInclination = tracker->getObjectPoint(27).z - tracker->getObjectPoint(21).z;

	ofLogVerbose(LOG_NAME) << "face angle" << faceInclination;

	//ofLogVerbose(LOG_NAME) << "thres" << (max+min)*0.5-area;

	if(latestEyeOpennes.size()>5 && (max+min)*0.5-area > area*.2){
		eyeClosed = true;
	}else{ // if(!eyesOpened && avg-(max+min)*0.5 > .2){
		eyeClosed = false;
		latestEyeOpennes.push_back(area);
		if(latestEyeOpennes.size()>60){
			latestEyeOpennes.pop_front();
		}
	}

}

float BlinkDetector::getOpennes(){
	if(!latestEyeOpennes.empty())
		return latestEyeOpennes[latestEyeOpennes.size()-1];
	else
		return 0;
}

bool BlinkDetector::isClosed(){
	return eyeClosed;
}

void BlinkDetector::reset(){
	latestEyeOpennes.clear();
}

deque<float> BlinkDetector::getHistory(){
	return latestEyeOpennes;
}
