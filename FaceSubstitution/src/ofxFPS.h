/*
 * ofxFPS.h
 *
 *  Created on: Jul 20, 2013
 *      Author: arturo castro
 */

#ifndef OFXFPS_H_
#define OFXFPS_H_

// class to calculate the fps of video streams

class ofxFPS {
public:
	ofxFPS();
	virtual ~ofxFPS();

	void newFrame();
	float getFPS();

private:
	unsigned long long microsOneFrame;
	unsigned int frames;
	float fps;
};

#endif /* OFXFPS_H_ */
