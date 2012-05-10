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
#include "ofxGui.h"

class FaceBlinkRecorder: public ofThread, public ofxPm::VideoSource {
public:
	FaceBlinkRecorder();
	virtual ~FaceBlinkRecorder();

	void setup(ofxFaceTrackerThreaded & tracker);
	void update(ofPixels & frame);

	void threadedFunction();

	void setEyesClosed(bool closed);

	ofxPm::VideoFrame * getNextVideoFrame();
    float getFps();

	static string LOG_NAME;

	string getState();

	bool isRecording();

	ofEvent<bool> recordingE;
	ofEvent<bool> recordedE;
	ofEvent<bool> droppedE;

	ofxParameter<float> secsFaceLostBetweenRecordings;
	ofxParameter<float> secsFaceLostToDropRecording;
	ofxParameter<float> secsEyesClosedToEndRecording;
	ofxParameter<float> secsToRecord;

private:
	ofxFaceTrackerThreaded * tracker;
	ofxVideoRecorder recorder;
	Poco::Condition encodeVideo;
	float prevFound;
	float firstEyesClosed;
	bool recording;
	bool eyesClosed;
	ofxPm::VideoBuffer buffer;
	ofxPm::VideoFrame * vframe;
	bool encoding;

	float oneSec;
	int framesOneSec;
	float fps;
};
#endif /* FACEBLINKRECORDER_H_ */
