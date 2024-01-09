#include "Clone.h"

string getMaskBlurShaderSourceH(int strenght){
	return
	"#extension GL_ARB_texture_rectangle : enable\n"
	"uniform sampler2DRect tex, mask;\
	void main() {\
		vec2 pos = gl_FragCoord.xy;\
		vec4 sum = texture2DRect(tex, pos);\
		int samples=0;\
		for(int i = 1; i < " +  ofToString(strenght) + "; i++) {\
			vec2 curOffset = vec2(float(i),0);\
			vec4 maskL = texture2DRect(mask, pos- curOffset);\
			vec4 maskR = texture2DRect(mask, pos+ curOffset);\
			if(maskL.a>0. && maskR.a>0.){\
				sum += (texture2DRect(tex, pos + curOffset)+\
					   texture2DRect(tex, pos - curOffset));\
				samples++;\
			}else{\
				break;\
			}\
		}\
		gl_FragColor = sum / float(samples*2+1);\
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
		for(int i = 1; i < " +  ofToString(strenght) + "; i++) {\
			vec2 curOffset = vec2(0,float(i));\
			vec4 maskL = texture2DRect(mask, pos- curOffset);\
			vec4 maskR = texture2DRect(mask, pos+ curOffset);\
			if(maskL.a>0. && maskR.a>0.){\
				sum += (texture2DRect(tex, pos + curOffset)+\
					   texture2DRect(tex, pos - curOffset));\
				samples++;\
			}else{\
				break;\
			}\
		}\
		gl_FragColor = sum / float(samples*2+1);\
	}";
}

char cloneShaderSource[] = 
"#extension GL_ARB_texture_rectangle : enable\n"
"uniform sampler2DRect src, srcBlur, dstBlur;\
void main() {\
	vec2 pos = gl_TexCoord[0].st;//vec2(gl_FragCoord.x,900.-gl_FragCoord.y);\n	\
	vec4 srcColorBlur = texture2DRect(srcBlur, pos);\
	vec3 srcColor = texture2DRect(src, pos).rgb;\
	vec4 dstColorBlur = texture2DRect(dstBlur, pos);\
	vec3 offset = (dstColorBlur.rgb - srcColorBlur.rgb);\
	gl_FragColor = vec4(srcColor + offset, 1.);\
}";

void Clone::setup(int width, int height) {
    allocate(width, height);
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


void Clone::allocate(int width, int height) {
    ofFbo::Settings settings;
    settings.width = width;
    settings.height = height;
    settings.internalformat = GL_RGB;

    buffer.allocate(settings);
    srcBlur.allocate(settings);
    dstBlur.allocate(settings);
}

void Clone::maskedBlur(ofTexture& tex, const ofMesh& mask, ofTexture & maskTex, ofFbo& result, bool dst) {
	buffer.begin();
	ofClear(0, 0);
	maskBlurShaderH.begin();
	maskBlurShaderH.setUniformTexture("mask", maskTex, 0);
	maskBlurShaderH.setUniformTexture("tex", tex, 1);
	((ofMesh & )mask).draw();
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
	((ofMesh & )mask).draw();
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

void Clone::draw(ofTexture& src, ofTexture& dst, const ofMesh& mask) {
	maskedBlur(src, mask, src, srcBlur, false);
	maskedBlur(dst, mask, src, dstBlur, true);
	
	//buffer.begin();
	//dstBlur.draw(0,0);
	ofPushStyle();
	ofEnableAlphaBlending();
	dst.draw(0, 0);
	cloneShader.begin();
	cloneShader.setUniformTexture("src", src, 0);
	cloneShader.setUniformTexture("srcBlur", srcBlur, 1);
	cloneShader.setUniformTexture("dstBlur", dstBlur, 2);
	((ofMesh & )mask).draw();
	cloneShader.end();
	ofDisableAlphaBlending();
	ofPopStyle();
	//buffer.end();
}

