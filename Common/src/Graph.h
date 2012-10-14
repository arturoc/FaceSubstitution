/*
 * Graph.h
 *
 *  Created on: 03/03/2012
 *      Author: arturo
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include <deque>
#include "ofPolyline.h"
#include "ofRectangle.h"
#include "ofGraphics.h"

class Graph {
public:
	Graph()
	:threshold(0)
	,maxSize(128) {
	}
	void setup(int maxSize) {
		this->maxSize = maxSize;
	}
	void addSample(float sample) {
		buffer.push_back(sample);
		if(buffer.size() > maxSize) {
			buffer.pop_front();
		}
	}
	void setThreshold(float threshold) {
		this->threshold = threshold;
	}
	void glMapX(float minInput, float maxInput, float minOutput, float maxOutput) {
		float inputRange = maxInput - minInput, outputRange = maxOutput - minOutput;
		ofTranslate(minOutput, 0);
		ofScale(outputRange, 1);
		ofScale(1. / inputRange, 1);
		ofTranslate(-minInput, 0);
	}
	void glMapY(float minInput, float maxInput, float minOutput, float maxOutput) {
		float inputRange = maxInput - minInput, outputRange = maxOutput - minOutput;
		ofTranslate(0, minOutput);
		ofScale(1, outputRange);
		ofScale(1, 1. / inputRange);
		ofTranslate(0, -minInput);
	}
	void draw(int x, int y, int height) {
		line.clear();
		for(int i = 0; i < buffer.size(); i++) {
			line.addVertex(ofVec2f(i, buffer[i]));
		}
		ofPushMatrix();
		ofPushStyle();
		ofTranslate(x, y);
		ofFill();
		bool bright = !buffer.empty() && threshold != 0 && buffer.back() > threshold;
		ofSetColor(bright ? 128 : 0);
		ofRect(0, 0, maxSize, height);
		ofNoFill();
		ofSetColor(255);
		ofRect(0, 0, maxSize, height);

		ofRectangle box = line.getBoundingBox();
		glMapX(box.x, box.x + box.width, 0, maxSize);
		glMapY(box.y, box.y + box.height, height, 0);
		line.draw();
		if(threshold != 0) {
			ofLine(0, threshold, buffer.size(), threshold);
		}
		ofPopStyle();
		ofPopMatrix();
	}
private:
	ofPolyline line;
	int maxSize;
	deque<float> buffer;
	float threshold;
};



#endif /* GRAPH_H_ */
