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
    
    copy(dst, result);
}

void testApp::update() {
}

Mat summed;
void maskedBlur(Mat tex, Mat mask, Mat dist, Mat& result) {
    imitate(result, tex);
    integral(tex, summed, CV_32S);
	int rows = result.rows;
	int cols = result.cols;
	for(int row = 0; row < rows; row++) {
		for(int col = 0; col < cols; col++) {
			if(mask.at<uchar>(row, col) > 0) {
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
					result.at<Vec3b>(row, col) = (Vec3b) curResult;
				} else {
					result.at<Vec3b>(row, col) = tex.at<Vec3b>(row, col);
				}
			}
		}
	}
}

Mat dist, srcBlur, dstBlur;
void blend(Mat srcFull, Mat dstFull, Mat maskFull, Mat resultFull, cv::Rect roi) {
    Mat src((srcFull), roi);
    Mat dst((dstFull), roi);
    Mat mask((maskFull), roi);
    Mat result((resultFull), roi);
    
    imitate(srcBlur, src);
    imitate(dstBlur, dst);
    
	distanceTransform(mask, dist, CV_DIST_L2, 3); // what does CV_DIST_C do?
	dist *= (1 / sqrt(2)); // normalize to the square corner
    
    maskedBlur(src, mask, dist, srcBlur);
	maskedBlur(dst, mask, dist, dstBlur);
    
    int rows = result.rows;
	int cols = result.cols;
	for(int row = 0; row < rows; row++) {
		for(int col = 0; col < cols; col++) {
			if(mask.at<uchar>(row, col) > 0) {
                Vec3s srcColor = src.at<Vec3b>(row, col);
                Vec3s srcBlurColor = srcBlur.at<Vec3b>(row, col);
                Vec3s dstBlurColor = dstBlur.at<Vec3b>(row, col);
                Vec3s offset = dstBlurColor - srcBlurColor;
                result.at<Vec3b>(row, col) = srcColor + offset;
            } else {
                result.at<Vec3b>(row, col) = dst.at<Vec3b>(row, col);
            }
        }
    }
}


void testApp::draw() {
	ofBackground(0);
    
    cv::Rect roi(97, 76, 368, 372);
    blend(toCv(src), toCv(dst), toCv(mask), toCv(result), roi);
    result.update();
	
	src.draw(0, 0);
	dst.draw(640, 0);
    mask.draw(0, 480);
    result.draw(640, 480);
	
	drawHighlightString(ofToString((int) ofGetFrameRate()), 10, 20);
}

