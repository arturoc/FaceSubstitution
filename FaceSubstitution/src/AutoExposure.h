/*
 * AutoExposure.h
 *
 *  Created on: 03/03/2012
 *      Author: arturo
 */

#ifndef AUTOEXPOSURE_H_
#define AUTOEXPOSURE_H_

#ifdef TARGET_LINUX
#include "ofxV4L2Settings.h"
#endif 

#include "ofPixels.h"
#include "ofxGui.h"

class AutoExposure {
public:
	AutoExposure();
	virtual ~AutoExposure();
	
	bool setup(int device, int w, int h);
	void update(ofPixels & frame, ofPixels & mask);
	
	
#ifdef TARGET_LINUX
	ofxV4L2Settings settings;
#endif
	
	ofxParameter<int> minExposure, maxExposure;
	
private:
	ofPixels grayPixels,grayPixelsMask;
};

#endif /* AUTOEXPOSURE_H_ */
