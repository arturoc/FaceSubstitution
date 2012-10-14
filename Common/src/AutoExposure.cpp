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

	settings["Exposure, Auto"] = 1;
	//settings["Exposure (Absolute)"] = 1033;

	// thinkpad
	/*minExposure = 800;
	maxExposure = 1400;*/

	// logitech
	minExposure = 200;
	maxExposure = 300;

	return true;
}

void AutoExposure::update(ofPixels & frame, ofPixels & mask){
	ofxCv::convertColor(frame,grayPixels,CV_RGB2GRAY);
	cv::Scalar mean;
	if(mask.getNumChannels()==4){
		ofxCv::convertColor(mask,grayPixelsMask,CV_RGBA2GRAY);
		mean = cv::mean(ofxCv::toCv(grayPixels),ofxCv::toCv(grayPixelsMask));
	}else if(mask.getNumChannels()==3){
		ofxCv::convertColor(mask,grayPixelsMask,CV_RGB2GRAY);
		mean = cv::mean(ofxCv::toCv(grayPixels),ofxCv::toCv(grayPixelsMask));
	}else if(mask.getNumChannels()==1){
		mean = cv::mean(ofxCv::toCv(grayPixels),ofxCv::toCv(mask));

	}

	int exposure = ofMap(mean.val[0],40,200,maxExposure,minExposure);

	settings["Exposure (Absolute)"] = exposure;
}
