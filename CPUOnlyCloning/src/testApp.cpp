#include "testApp.h"

using namespace ofxCv;
using namespace cv;

void testApp::setup() {
    ofSetVerticalSync(false);
	
	src.loadImage("src.jpg");
	src.setImageType(OF_IMAGE_COLOR);
	dst.loadImage("dst.jpg");
	dst.setImageType(OF_IMAGE_COLOR);
	mask.loadImage("mask.png");
	mask.setImageType(OF_IMAGE_GRAYSCALE);
	
	imitate(srcBlur, src);
	imitate(dstBlur, dst);
    imitate(result, dst);
    
//	imitate(dist, mask, CV_32FC1);
    cloneShader.load("", "Clone.frag");
}

void testApp::update() {
}

cv::Mat dist, summed;
void maskedBlur(ofImage& tex, ofImage& mask, ofImage& result) {
	Mat texMat = toCv(tex);
	Mat resultMat = toCv(result);
	Mat maskMat = toCv(mask);
	
    integral(texMat, summed, CV_32S);
		
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
						summed.at<Vec3i>(row2, col2) -
						summed.at<Vec3i>(row2, col1) -
						summed.at<Vec3i>(row1, col2) +
						summed.at<Vec3i>(row1, col1);
						
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

void blend(ofImage& src, ofImage& dst, ofImage& srcBlur, ofImage& dstBlur, ofImage& result) {
    imitate(result, dstBlur);
    Mat srcMat = toCv(src);
    Mat dstMat = toCv(dst);
    Mat srcBlurMat = toCv(srcBlur);
    Mat dstBlurMat = toCv(dstBlur);
    Mat resultMat = toCv(result);
    
    int rows = resultMat.rows;
	int cols = resultMat.cols;
	for(int row = 0; row < rows; row++) {
		for(int col = 0; col < cols; col++) {
            // should use the mask here instead
            Vec3s srcBlurColor = srcBlurMat.at<Vec3b>(row, col);
			if(srcBlurColor[0] > 0) {
                Vec3s srcColor = srcMat.at<Vec3b>(row, col);
                Vec3s dstBlurColor = dstBlurMat.at<Vec3b>(row, col);
                Vec3s offset = dstBlurColor - srcBlurColor;
                resultMat.at<Vec3b>(row, col) = srcColor + offset;
            } else {
                resultMat.at<Vec3b>(row, col) = dstMat.at<Vec3b>(row, col);
            }
        }
    }
    
    result.update();
}


void testApp::draw() {
	ofBackground(0);
	
	ofPushMatrix();
	ofScale(.5, .5);
	
	src.draw(0, 0);
	dst.draw(640, 0);
	
    maskedBlur(src, mask, srcBlur);
	maskedBlur(dst, mask, dstBlur);
	
	srcBlur.draw(0, 480);
	dstBlur.draw(640, 480);
	ofPopMatrix();
    
    blend(src, dst, srcBlur, dstBlur, result);
    
    result.draw(640, 0);
	
	drawHighlightString(ofToString((int) ofGetFrameRate()), 10, 20);
}

