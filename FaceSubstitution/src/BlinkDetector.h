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

class BlinkDetector {
public:
	BlinkDetector();

	void update(const ofPolyline & eyeContour);
	float getOpennes();
	bool isClosed();

	void reset();

	deque<float> getHistory();

private:
	deque<float> latestEyeOpennes;
	bool eyeClosed;

};

#endif /* BLINKDETECTOR_H_ */
