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
	minExposure.set("min exposure",200,0,2047);
	maxExposure.set("max exposure",1000,0,2047);

	return true;
}

void AutoExposure::update(ofPixels & frame, ofPixels & mask){
	cv::Mat cvGray;
	if(frame.getNumChannels()>1){
		ofxCv::convertColor(frame,grayPixels,CV_RGB2GRAY);
		cvGray = ofxCv::toCv(grayPixels);
	}else{
		cvGray = ofxCv::toCv(frame);
	}
	cv::Scalar mean;
	if(mask.getNumChannels()==4){
		ofxCv::convertColor(mask,grayPixelsMask,CV_RGBA2GRAY);
		mean = cv::mean(cvGray,ofxCv::toCv(grayPixelsMask));
	}else if(mask.getNumChannels()==3){
		ofxCv::convertColor(mask,grayPixelsMask,CV_RGB2GRAY);
		mean = cv::mean(cvGray,ofxCv::toCv(grayPixelsMask));
	}else if(mask.getNumChannels()==1){
		mean = cv::mean(cvGray,ofxCv::toCv(mask));
	}

	int exposure = ofMap(mean.val[0],40,200,maxExposure,minExposure);

	settings["Exposure (Absolute)"] = settings["Exposure (Absolute)"]*.99 + exposure*.01;
}

void AutoExposure::update(ofPixels & frame, ofRectangle & roi){
	cv::Mat cvGray;
	if(frame.getNumChannels()>1){
		ofxCv::convertColor(frame,grayPixels,CV_RGB2GRAY);
		cvGray = ofxCv::toCv(grayPixels);
	}else{
		cvGray = ofxCv::toCv(frame);
	}

	cv::Mat cvGrayRoi = cvGray(ofxCv::toCv(roi));

	cv::Scalar mean;
	mean = cv::mean(cvGrayRoi);

	int exposure = ofMap(mean.val[0],40,200,maxExposure,minExposure);

	settings["Exposure (Absolute)"] = exposure;
}
