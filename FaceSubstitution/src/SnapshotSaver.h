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

class SnapshotSaver: public ofThread {
public:
	SnapshotSaver();
	virtual ~SnapshotSaver();

	void setup(string folder);

	void save(ofPixels & pixels);
	void threadedFunction();

private:
	ofPixels * pixels;
	Poco::Condition saveFrame;
	string folder;
	ofPixels rgbPixels;
};

#endif /* SNAPSHOTSAVER_H_ */
