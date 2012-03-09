/*
 * MouthOpenDetector.cpp
 *
 *  Created on: 09/03/2012
 *      Author: arturo
 */

#include "MouthOpenDetector.h"

string MouthOpenDetector::LOG_NAME = "MouthOpenDetector";

MouthOpenDetector::MouthOpenDetector() {
	mouthOpenned = false;
	prevArea = 0;

}

void MouthOpenDetector::setup(ofxFaceTracker * _tracker){
	tracker = _tracker;
	mouth = ofxFaceTracker::INNER_MOUTH;
}

void MouthOpenDetector::update(){
	const ofPolyline & mouthContour = tracker->getObjectFeature(mouth);
	float area = mouthContour.getArea();


	float max=-1, min=99;

	deque<float>::iterator it;
	for(it=latestMouthOpennes.begin();it!=latestMouthOpennes.end();it++){
		if(max<*it) max=*it;
		if(min>*it) min=*it;
	}

	//float faceInclination = tracker->getObjectPoint(27).z - tracker->getObjectPoint(21).z;

	//ofLogVerbose(LOG_NAME) << "area" << area;

	//ofLogVerbose(LOG_NAME) << "face angle" << faceInclination;

	//ofLogVerbose(LOG_NAME) << "thres" << (max+min)*0.5-area;

	latestMouthOpennes.push_back(area);
	if(latestMouthOpennes.size()>60){
		latestMouthOpennes.pop_front();
	}

	if(latestMouthOpennes.size()>5 && (max+min)*0.5-area > area*.2){
		mouthOpenned = false;
	}else{ // if(!eyesOpened && avg-(max+min)*0.5 > .2){
		mouthOpenned = true;
	}

	graphArea.setThreshold(((max+min)*0.5-area*.2));
	graphArea.addSample(area);
	graphDerivative.addSample(area-prevArea);
	graphBool.addSample(mouthOpenned);

	prevArea = area;
}

float MouthOpenDetector::getOpennes(){
	if(!latestMouthOpennes.empty())
		return latestMouthOpennes[latestMouthOpennes.size()-1];
	else
		return 0;
}

bool MouthOpenDetector::isOpenned(){
	return mouthOpenned;
}

void MouthOpenDetector::reset(){
	latestMouthOpennes.clear();
}

deque<float> MouthOpenDetector::getHistory(){
	return latestMouthOpennes;
}

