#pragma once

#include "ofMain.h"

class Clone {
public:
	void setup(int width, int height);
	void setStrength(int strength);
	void update(ofTexture& src, ofTexture& dst, ofMesh& mask,  ofTexture & texMask);
	void draw(float x, float y);
	void draw(float x, float y, float w, float h);
	
	ofTexture & getTextureRef();
	void readToPixels(ofPixels & pix);

protected:
	void maskedBlur(ofTexture& tex, ofMesh& mask, ofTexture & maskTex, ofFbo& result, bool dst);
	ofFbo buffer, srcBlur, dstBlur;
	ofShader maskBlurShaderH, maskBlurShaderV, cloneShader;
	int strength;
};
