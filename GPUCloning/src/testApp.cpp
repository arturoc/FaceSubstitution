#include "testApp.h"

void testApp::setup() {
	//ofSetVerticalSync(true);
	
	src.loadImage("src.jpg");
	src.setImageType(OF_IMAGE_COLOR);
	dst.loadImage("dst.jpg");
	dst.setImageType(OF_IMAGE_COLOR);
	mask.loadImage("mask.png");
	mask.setImageType(OF_IMAGE_GRAYSCALE);
	
	ofFbo::Settings settings;
	settings.width = src.getWidth();
	settings.height = src.getHeight();
	
	halfBlur.allocate(settings);
	srcBlur.allocate(settings);
	dstBlur.allocate(settings);
	
	maskBlurShader.load("", "MaskBlur.frag");
	cloneShader.load("", "Clone.frag");
}

void testApp::update() {
}

void testApp::maskedBlur(ofBaseHasTexture& tex, ofBaseHasTexture& mask, ofFbo& result) {
	int k = ofMap(mouseX, 0, ofGetWidth(), 0, 128, true);
	
	halfBlur.begin();
	maskBlurShader.begin();
	maskBlurShader.setUniformTexture("tex", tex, 1);
	maskBlurShader.setUniformTexture("mask", mask, 2);
	maskBlurShader.setUniform2f("direction", 1, 0);
	maskBlurShader.setUniform1i("k", k);
	tex.getTextureReference().draw(0, 0);
	maskBlurShader.end();
	halfBlur.end();
	
	result.begin();
	maskBlurShader.begin();
	maskBlurShader.setUniformTexture("tex", halfBlur, 1);
	maskBlurShader.setUniformTexture("mask", mask, 2);
	maskBlurShader.setUniform2f("direction", 0, 1);
	maskBlurShader.setUniform1i("k", k);
	halfBlur.draw(0, 0);
	maskBlurShader.end();
	result.end();
}

void testApp::draw() {
	ofBackground(0);
	
	ofPushMatrix();
	ofScale(.5, .5);
	
	src.draw(0, 0);
	dst.draw(640, 0);
	
	if(!ofGetMousePressed()) {
		maskedBlur(src, mask, srcBlur);
	}
	maskedBlur(dst, mask, dstBlur);
	
	srcBlur.draw(0, 480);
	dstBlur.draw(640, 480);
	ofPopMatrix();
	
	ofEnableAlphaBlending();
	dst.draw(0, 480);	
	cloneShader.begin();
	cloneShader.setUniformTexture("src", src, 1);
	cloneShader.setUniformTexture("srcBlur", srcBlur, 2);
	cloneShader.setUniformTexture("dstBlur", dstBlur, 3);
	dst.draw(0, 480);
	cloneShader.end();
	ofDisableAlphaBlending();
	
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
}

void testApp::keyPressed(int key) {
	if(key == 'r') {
		maskBlurShader.load("", "MaskBlur.frag");
		cloneShader.load("", "Clone.frag");
	}
}