#include "Clone.h"

string getMaskBlurShaderSourceH(int strenght){
	return
	"#extension GL_ARB_texture_rectangle : enable\n"
	"uniform sampler2DRect tex, mask;\
	void main() {\
		vec2 pos = gl_FragCoord.xy;\
		vec4 sum = texture2DRect(tex, pos);\
		int samples=0;\
		int i = 2;\
		for(; i < " +  ofToString(strenght) + "; i+=2) {\
			vec2 curOffset = vec2(float(i)- .5,0);\
			vec4 maskL = texture2DRect(mask, pos- curOffset);\
			vec4 maskR = texture2DRect(mask, pos+ curOffset);\
			if(maskL.r==1. && maskR.r==1.){\
				sum += (texture2DRect(tex, pos + curOffset)+\
					   texture2DRect(tex, pos - curOffset));\
				samples++;\
			}else{\
				break;\
			}\
		}\
        samples = 1 + (i - 2); \
		gl_FragColor = sum / float(samples);\
	}";
}

string getMaskBlurShaderSourceV(int strenght){
	return
	"#extension GL_ARB_texture_rectangle : enable\n"
	"uniform sampler2DRect tex, mask;\
	void main() {\
		vec2 pos = gl_FragCoord.xy;\
		vec4 sum = texture2DRect(tex, pos);\
		int samples=0;\
		int i = 2;\
		for(;i < " +  ofToString(strenght) + "; i+=2) {\
			vec2 curOffset = vec2(0,float(i) - .5);\
			vec4 maskL = texture2DRect(mask, pos- curOffset);\
			vec4 maskR = texture2DRect(mask, pos+ curOffset);\
			if(maskL.r==1. && maskR.r==1.){\
				sum += (texture2DRect(tex, pos + curOffset)+\
					   texture2DRect(tex, pos - curOffset));\
				samples++;\
			}else{\
				break;\
			}\
		}\
        samples = 1 + (i - 2); \
		gl_FragColor = sum / float(samples);\
	}";
}

char cloneShaderSource[] = 
"#extension GL_ARB_texture_rectangle : enable\n"
"uniform sampler2DRect src, srcBlur, dstBlur;\
void main() {\
	vec2 pos = gl_FragCoord.xy;	\
	vec4 srcColorBlur = texture2DRect(srcBlur, pos);\
	vec3 srcColor = texture2DRect(src, pos).rgb;\
	vec4 dstColorBlur = texture2DRect(dstBlur, pos);\
	vec3 offset = (dstColorBlur.rgb - srcColorBlur.rgb);\
	gl_FragColor = vec4(srcColor + offset, 1.);\
}";

void Clone::setup(int width, int height) {
	ofFbo::Settings settings;
	settings.width = width;
	settings.height = height;
	settings.internalformat = GL_RGB;
	
	buffer.allocate(settings);
	srcBlur.allocate(settings);
	dstBlur.allocate(settings);
	
	strength = 0;

	maskBlurShaderH.setupShaderFromSource(GL_FRAGMENT_SHADER, getMaskBlurShaderSourceH(strength));
	maskBlurShaderV.setupShaderFromSource(GL_FRAGMENT_SHADER, getMaskBlurShaderSourceV(strength));
	maskBlurShaderH.linkProgram();
	maskBlurShaderV.linkProgram();
	cloneShader.setupShaderFromSource(GL_FRAGMENT_SHADER, cloneShaderSource);
	cloneShader.linkProgram();
	
	strength.addListener(this,&Clone::setStrength);
	strength.set("strength",7,0,30);
}

void Clone::maskedBlur(ofTexture& tex, ofMesh& mask, ofTexture & maskTex, ofFbo& result, bool dst) {
	buffer.begin();
	ofClear(0, 0);
	maskBlurShaderH.begin();
	maskBlurShaderH.setUniformTexture("mask", maskTex, 0);
	maskBlurShaderH.setUniformTexture("tex", tex, 1);
	mask.draw();
	maskBlurShaderH.end();
	buffer.end();
	
	result.begin();
	if(dst){
		tex.draw(0,0);
	}else{
		ofClear(0,0);
	}
	maskBlurShaderV.begin();
	maskBlurShaderV.setUniformTexture("mask", maskTex, 0);
	maskBlurShaderV.setUniformTexture("tex", buffer, 1);
	mask.draw();
	maskBlurShaderV.end();
	result.end();
}

void Clone::setStrength(int & strength) {
	this->strength = strength;
	maskBlurShaderH.unload();
	maskBlurShaderV.unload();
	maskBlurShaderH.setupShaderFromSource(GL_FRAGMENT_SHADER, getMaskBlurShaderSourceH(strength));
	maskBlurShaderV.setupShaderFromSource(GL_FRAGMENT_SHADER, getMaskBlurShaderSourceV(strength));
	maskBlurShaderH.linkProgram();
	maskBlurShaderV.linkProgram();
}

void Clone::update(ofTexture& src, ofTexture& dst, ofMesh& mask, ofTexture & texMask) {
	maskedBlur(src, mask, texMask, srcBlur, false);
	maskedBlur(dst, mask, texMask, dstBlur, true);
	
	buffer.begin();
	//dstBlur.draw(0,0);
	ofPushStyle();
	ofEnableAlphaBlending();
	dst.draw(0, 0);	
	cloneShader.begin();
	cloneShader.setUniformTexture("src", src, 0);
	cloneShader.setUniformTexture("srcBlur", srcBlur, 1);
	cloneShader.setUniformTexture("dstBlur", dstBlur, 2);
	mask.draw();
	cloneShader.end();
	ofDisableAlphaBlending();
	ofPopStyle();
	buffer.end();
}

void Clone::draw(float x, float y) {
	buffer.draw(x, y);
}

void Clone::draw(float x, float y, float w, float h){
	buffer.draw(x,y,w,h);
}

ofTexture & Clone::getTextureRef(){
	return buffer.getTextureReference();
}

void Clone::readToPixels(ofPixels & pix){
	buffer.readToPixels(pix);
}
