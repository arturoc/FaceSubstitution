/*
 * InteractionRecorder.h
 *
 *  Created on: 10/03/2012
 *      Author: arturo
 */

#ifndef INTERACTIONRECORDER_H_
#define INTERACTIONRECORDER_H_

#include <queue>
#include "ofConstants.h"
#include "ofThread.h"
#include "ofxGstVideoRecorder.h"
#include "ofxXmlSettings.h"

class InteractionRecorder: public ofThread {
public:
	InteractionRecorder();
	virtual ~InteractionRecorder();

	void setup(string path,string currentFace,int w, int h, int fps=30);
	void addFrame(ofPixels & frame);
	void changeFace(string face);
	void close();

protected:
	void threadedFunction();

private:
	queue<ofxGstVideoRecorder*> recordersQueue;
	ofxGstVideoRecorder* recorder;
	string recordedVideoPath;
	int framesRecorded;
	queue<string> uploadQueue;
	Poco::Condition upload;
	ofxXmlSettings settings;
	string ftpServer;
	string user;
	string password;
	string serverPath;
	string folder;
};

#endif /* INTERACTIONRECORDER_H_ */
