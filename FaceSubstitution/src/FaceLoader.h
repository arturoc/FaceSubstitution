/*
 * FaceLoader.h
 *
 *  Created on: 22/01/2012
 *      Author: arturo
 */

#ifndef FACELOADER_H_
#define FACELOADER_H_

#include "ofConstants.h"
#include "ofVectorMath.h"
#include "ofImage.h"
#include "ofxFaceTrackerThreaded.h"
#include "ofThread.h"
#include "ofFileUtils.h"

class FaceLoader: public ofThread {
public:
	FaceLoader();
	virtual ~FaceLoader();

	enum Mode{
		Random,
		Sequential
	};

	void setup(string folder, Mode mode);
	void update();

	ofImage & getCurrentImg();
	ofxFaceTracker & getTracker();
	vector<ofVec2f> & getCurrentImagePoints();

	void threadedFunction();

	void loadNext();

private:
	void resizeAndDiscardImages(string path);
	void loadFace(string face);

	ofxFaceTracker tracker;
	ofImage src[2], *currentImg, *nextImg;
	vector<ofVec2f> srcPoints[2], *currentPoints, *nextPoints;
	ofDirectory faces;
	int currentFace;
	bool loadNextFace;
	Poco::Condition loadNew;
	Mode mode;

	//ofMutex mutex;

};

#endif /* FACELOADER_H_ */
