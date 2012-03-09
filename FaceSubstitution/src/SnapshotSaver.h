/*
 * SnapshotSaver.h
 *
 *  Created on: 04/03/2012
 *      Author: arturo
 */

#ifndef SNAPSHOTSAVER_H_
#define SNAPSHOTSAVER_H_

#include "ofConstants.h"
#include "ofThread.h"
#include "Poco/Condition.h"
#include "ofPixels.h"
#include "ofxXmlSettings.h"

class SnapshotSaver: public ofThread {
public:
	SnapshotSaver();
	virtual ~SnapshotSaver();

	void setup(string folder);

	void save(ofPixels & pixels);
	void threadedFunction();

	static string LOG_NAME;

private:
	ofPixels * pixels;
	Poco::Condition saveFrame;
	string folder;
	ofPixels rgbPixels;
	ofxXmlSettings settings;
	string ftpServer;
	string user;
	string password;
	string serverPath;
};

#endif /* SNAPSHOTSAVER_H_ */
