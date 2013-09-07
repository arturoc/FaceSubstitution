/*
 * ofxFPS.cpp
 *
 *  Created on: Jul 20, 2013
 *      Author: arturo
 */

#include "ofxFPS.h"
#include "ofUtils.h"

ofxFPS::ofxFPS()
:microsOneFrame(0)
,frames(0)
,fps(0){
	// TODO Auto-generated constructor stub

}

ofxFPS::~ofxFPS() {
	// TODO Auto-generated destructor stub
}


void ofxFPS::newFrame(){
	unsigned long long now = ofGetElapsedTimeMicros();
	frames++;
	fps=frames/(double(now-microsOneFrame)/1000000.);
	if(now-microsOneFrame>=1000000){
		frames=0;
		microsOneFrame = now;
	}
}

float ofxFPS::getFPS(){
	return fps;
}
