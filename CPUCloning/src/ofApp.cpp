#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

void ofApp::setup() {
	//ofSetVerticalSync(true);
	
	src.loadImage("src.jpg");
	src.setImageType(OF_IMAGE_COLOR);
	dst.loadImage("dst.jpg");
	dst.setImageType(OF_IMAGE_COLOR);
	mask.loadImage("mask.png");
	mask.setImageType(OF_IMAGE_GRAYSCALE);
	
	imitate(srcBlur, src);
	imitate(dstBlur, dst);
	imitate(dist, mask, CV_32FC1);
	
	cloneShader.load("", "Clone.frag");
}

void ofApp::update() {
}

void ofApp::maskedBlur(ofImage& tex, ofImage& mask, ofImage& result) {	
	Mat texMat = toCv(tex);
	Mat resultMat = toCv(result);
	Mat maskMat = toCv(mask);
	
	integral(texMat, sum, CV_32S);
		
	distanceTransform(maskMat, dist, CV_DIST_L2, 3); // what does CV_DIST_C do?
	dist *= (1 / sqrt(2)); // normalize to the square corner
	
	resultMat = Scalar(0);
	
	int rows = resultMat.rows;
	int cols = resultMat.cols;
	for(int row = 0; row < rows; row++) {
		for(int col = 0; col < cols; col++) {
			if(maskMat.at<uchar>(row, col) > 0) {
				int k = dist.at<float>(row, col);
				if(k > 0) {
					int row1 = row - k;
					int row2 = row + k;
					int col1 = col - k;
					int col2 = col + k;
					
					Vec3i curResult =
						sum.at<Vec3i>(row2, col2) -
						sum.at<Vec3i>(row2, col1) -
						sum.at<Vec3i>(row1, col2) +
						sum.at<Vec3i>(row1, col1);
						
					float norm = k * 2;
					norm *= norm;
					curResult *= 1 / norm;
					
					resultMat.at<Vec3b>(row, col) = (Vec3b) curResult;
				} else {
					resultMat.at<Vec3b>(row, col) = texMat.at<Vec3b>(row, col);
				}
			}
		}
	}
	
	result.update();
}

void ofApp::draw() {
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
	dst.draw(640, 0);
	cloneShader.begin();
	cloneShader.setUniformTexture("src", src, 1);
	cloneShader.setUniformTexture("srcBlur", srcBlur, 2);
	cloneShader.setUniformTexture("dstBlur", dstBlur, 3);
	dst.draw(640, 0);
	cloneShader.end();
	ofDisableAlphaBlending();
	
	drawHighlightString(ofToString((int) ofGetFrameRate()), 10, 20);
}

void ofApp::keyPressed(int key) {
}
