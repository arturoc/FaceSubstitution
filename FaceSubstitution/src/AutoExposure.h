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
#include "ofMesh.h"

class AutoExposure {
public:
	AutoExposure();
	virtual ~AutoExposure();

	bool setup(int device, int w, int h);
	void update(ofPixels & frame, ofPixels & mask);


	ofxV4L2Settings settings;

private:
	ofPixels grayPixels,grayPixelsMask;
};

#endif /* AUTOEXPOSURE_H_ */
