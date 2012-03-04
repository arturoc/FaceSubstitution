/*
 * BlinkTrigger.h
 *
 *  Created on: 03/02/2012
 *      Author: arturo
 */

#ifndef BLINKTRIGGER_H_
#define BLINKTRIGGER_H_

#include "BlinkDetector.h"
#include "ofEvents.h"
#include "ofxGui.h"
#include "ofxFaceTrackerThreaded.h"

class BlinkTrigger {
public:
	BlinkTrigger();
	virtual ~BlinkTrigger();

	void setup(ofxFaceTrackerThreaded & tracker);

	void threadedUpdate(ofEventArgs & args);

	bool areEyesClosed();

	ofEvent<bool> blinkE;
	ofEvent<bool> longBlinkE;

	ofxParameter<int> millisLongBlink;
	BlinkDetector leftBD,rightBD;

private:
	ofxFaceTrackerThreaded * tracker;

	int millisEyesClosed;
	int firstEyesClosedEvent;
	bool longBlinkTriggered;
	bool eyesClosed;
};

#endif /* BLINKTRIGGER_H_ */
