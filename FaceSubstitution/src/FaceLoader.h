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
#include "ofxFaceTracker.h"
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

	void setMode(Mode mode);

	ofImage & getCurrentImg();
	ofxFaceTracker & getTracker();
    vector<glm::vec2> & getCurrentImagePoints();

	void threadedFunction();

	string loadNext();
	string loadPrevious();

	int getTotalFaces();
	int getCurrentFace();

	string getCurrentFacePath();
	string getPreviousFacePath();

	ofEvent<string> newFaceLoadedE;
private:
	void resizeAndDiscardImages(string path);
	void loadFace(string face);

	ofxFaceTracker tracker;
	ofImage src[2], *currentImg, *nextImg;
    vector<glm::vec2> srcPoints[2], *currentPoints, *nextPoints;
	ofDirectory faces;
	bool loadNextFace;
    std::condition_variable loadNew;
	Mode mode;
	int currentFace, previousFace;;

	//ofMutex mutex;

};

#endif /* FACELOADER_H_ */
