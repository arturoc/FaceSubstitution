#pragma once

#include "ofMain.h"
#include "ofxCv.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void maskedBlur(ofImage& tex, ofImage& mask, ofImage& result);
	void keyPressed(int key);
	
	ofImage src, dst, mask;
	ofImage srcBlur, dstBlur;
	cv::Mat dist, sum;
	ofShader cloneShader;
};
