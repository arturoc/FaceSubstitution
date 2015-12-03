#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void maskedBlur(ofBaseHasTexture& tex, ofBaseHasTexture& mask, ofFbo& result);
	void keyPressed(int key);
	
	ofImage src, dst, mask;
	ofFbo halfBlur;
	ofFbo srcBlur, dstBlur;
	ofShader maskBlurShader;
	ofShader cloneShader;
};
