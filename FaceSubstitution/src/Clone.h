#pragma once

#include "ofMain.h"

class Clone {
public:
	void setup(int width, int height);
	void draw(ofTexture& src, ofTexture& dst, const ofMesh& mask);
	ofParameter<int> strength;

protected:
	void setStrength(int & strength);
	void maskedBlur(ofTexture& tex, const ofMesh& mask, ofTexture & maskTex, ofFbo& result, bool dst);
	ofFbo buffer, srcBlur, dstBlur;
	ofShader maskBlurShaderH, maskBlurShaderV, cloneShader;
};
