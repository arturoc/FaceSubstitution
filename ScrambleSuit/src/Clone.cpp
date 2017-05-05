#include "Clone.h"

char maskBlurShaderSource[] =
"#extension GL_ARB_texture_rectangle : enable\n"
"uniform sampler2DRect tex, mask;\
uniform vec2 direction;\
uniform int k;\
void main() {\
	vec2 pos = gl_TexCoord[0].st;\
	vec4 sum = texture2DRect(tex, pos);\
	int i;\
	for(i = 1; i < k; i++) {\
		vec2 curOffset = float(i) * direction;\
		vec4 leftMask = texture2DRect(mask, pos - curOffset);\
		vec4 rightMask = texture2DRect(mask, pos + curOffset);\
		bool valid = leftMask.r == 1. && rightMask.r == 1.;\
		if(valid) {\
			sum +=\
				texture2DRect(tex, pos + curOffset) +\
				texture2DRect(tex, pos - curOffset);\
		} else {\
			break;\
		}\
	}\
	int samples = 1 + (i - 1) * 2;\
	gl_FragColor = sum / float(samples);\
}";

char cloneShaderSource[] = 
"#extension GL_ARB_texture_rectangle : enable\n"
"uniform sampler2DRect src, srcBlur, dstBlur;\
void main() {\
	vec2 pos = gl_TexCoord[0].st;	\
	vec4 srcColorBlur = texture2DRect(srcBlur, pos);\
	if(srcColorBlur.a > 0.) {\
		vec3 srcColor = texture2DRect(src, pos).rgb;\
		vec4 dstColorBlur = texture2DRect(dstBlur, pos);\
		vec3 offset = (dstColorBlur.rgb - srcColorBlur.rgb);\
		gl_FragColor = vec4(srcColor + offset, 1.);\
	} else {\
		gl_FragColor = vec4(0.);\
	}\
}";

void Clone::setup(int width, int height) {
	ofFbo::Settings settings;
	settings.width = width;
	settings.height = height;
	
	buffer.allocate(settings);
	srcBlur.allocate(settings);
	dstBlur.allocate(settings);
	
	maskBlurShader.setupShaderFromSource(GL_FRAGMENT_SHADER, maskBlurShaderSource);
	cloneShader.setupShaderFromSource(GL_FRAGMENT_SHADER, cloneShaderSource);
	maskBlurShader.linkProgram();
	cloneShader.linkProgram();
	
	strength = 0;
}

void Clone::maskedBlur(ofTexture& tex, ofTexture& mask, ofFbo& result) {
	int k = strength;
	
	buffer.begin();
	maskBlurShader.begin();
	maskBlurShader.setUniformTexture("tex", tex, 1);
	maskBlurShader.setUniformTexture("mask", mask, 2);
	maskBlurShader.setUniform2f("direction", 1, 0);
	maskBlurShader.setUniform1i("k", k);
	tex.draw(0, 0);
	maskBlurShader.end();
	buffer.end();
	
	result.begin();
	maskBlurShader.begin();
	maskBlurShader.setUniformTexture("tex", buffer, 1);
	maskBlurShader.setUniformTexture("mask", mask, 2);
	maskBlurShader.setUniform2f("direction", 0, 1);
	maskBlurShader.setUniform1i("k", k);
	buffer.draw(0, 0);
	maskBlurShader.end();
	result.end();
}

void Clone::setStrength(int strength) {
	this->strength = strength;
}

void Clone::update(ofTexture& src, ofTexture& dst, ofTexture& mask) {
	maskedBlur(src, mask, srcBlur);
	maskedBlur(dst, mask, dstBlur);
	
	buffer.begin();
	ofPushStyle();
	ofEnableAlphaBlending();
	dst.draw(0, 0);	
	cloneShader.begin();
	cloneShader.setUniformTexture("src", src, 1);
	cloneShader.setUniformTexture("srcBlur", srcBlur, 2);
	cloneShader.setUniformTexture("dstBlur", dstBlur, 3);
	dst.draw(0, 0);
	cloneShader.end();
	ofDisableAlphaBlending();
	ofPopStyle();
	buffer.end();
}

void Clone::draw(float x, float y) {
	buffer.draw(x, y);
}
