/*
 * AutoExposure.h
 *
 *  Created on: 03/03/2012
 *      Author: arturo
 */

#ifndef AUTOEXPOSURE_H_
#define AUTOEXPOSURE_H_

#include "ofxV4L2Settings.h"
#include "ofPixels.h"
#include "ofRectangle.h"

class AutoExposure {
public:
	AutoExposure();
	virtual ~AutoExposure();

	bool setup(int device, int w, int h);
	void update(ofPixels & frame, ofPixels & mask);
	void update(ofPixels & frame, ofRectangle & roi);


	ofxV4L2Settings settings;

	ofParameter<int> minExposure, maxExposure;

private:
	ofPixels grayPixels,grayPixelsMask;
};

#endif /* AUTOEXPOSURE_H_ */
