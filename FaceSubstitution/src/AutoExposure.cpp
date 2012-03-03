/*
 * AutoExposure.cpp
 *
 *  Created on: 03/03/2012
 *      Author: arturo
 */

#include "AutoExposure.h"
#include "ofxCv.h"

AutoExposure::AutoExposure() {
	// TODO Auto-generated constructor stub

}

AutoExposure::~AutoExposure() {
	// TODO Auto-generated destructor stub
}

bool AutoExposure::setup(int device, int w, int h){
	if(!settings.setup("/dev/video" + ofToString(device))){
		return false;
	}
	grayPixels.allocate(w,h,1);
	grayPixelsMask.allocate(w,h,1);
	//settings["Exposure, Auto"] = 1;
	//settings["Exposure (Absolute)"] = 1033;
	return true;
}

void AutoExposure::update(ofPixels & frame, ofPixels & mask){
	ofxCv::convertColor(frame,grayPixels,CV_RGB2GRAY);
	ofxCv::convertColor(mask,grayPixelsMask,CV_RGB2GRAY);
	cv::Scalar avg = cv::mean(ofxCv::toCv(grayPixels),ofxCv::toCv(grayPixelsMask));
	//thinkpad int exposure = ofMap(avg.val[0],40,200,1400,800);

	//logitech
	int exposure = ofMap(avg.val[0],40,200,300,200);
	//cout << "avg: " << avg.val[0] << " -> exposure " << exposure << endl;

	settings["Exposure (Absolute)"] = exposure;
}
