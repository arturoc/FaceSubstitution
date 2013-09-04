#pragma once

#include "ofMain.h"

class Clone {
public:
	void setup(int width, int height);
	void update(ofTexture& src, ofTexture& dst, ofMesh& mask,  ofTexture & texMask);
	void draw(float x, float y);
	void draw(float x, float y, float w, float h);
	
	ofTexture & getTextureRef();
	void readToPixels(ofPixels & pix);
	ofParameter<int> strength;

protected:
	void setStrength(int & strength);
	void maskedBlur(ofTexture& tex, ofMesh& mask, ofTexture & maskTex, ofFbo& result, bool dst);
	ofFbo buffer, srcBlur, dstBlur;
	ofShader maskBlurShaderH, maskBlurShaderV, cloneShader;
};
