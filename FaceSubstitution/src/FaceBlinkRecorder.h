/*
 * FaceBlinkRecorder.h
 *
 *  Created on: 03/02/2012
 *      Author: arturo
 */

#ifndef FACEBLINKRECORDER_H_
#define FACEBLINKRECORDER_H_

#include "ofxVideoRecorder.h"
#include "ofxFaceTrackerThreaded.h"
#include "Poco/Condition.h"
#include "ofxPlaymodes.h"

class FaceBlinkRecorder: public ofThread {
public:
	FaceBlinkRecorder();
	virtual ~FaceBlinkRecorder();

	void setup(ofxFaceTrackerThreaded & tracker);
	void update(ofPixels & frame);

	void threadedFunction();

	void setEyesClosed(bool closed);

	static string LOG_NAME;

	ofEvent<bool> recordingE;

private:
	ofxFaceTrackerThreaded * tracker;
	ofxVideoRecorder recorder;
	Poco::Condition encodeVideo;
	int prevFound;
	int firstEyesClosed;
	bool recording;
	bool eyesClosed;
};
#endif /* FACEBLINKRECORDER_H_ */
