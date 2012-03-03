/*
 * VideoFader.cpp
 *
 *  Created on: 17/02/2012
 *      Author: arturo
 */

#include "VideoFader.h"
#include "ofMath.h"
#include "ofGraphics.h"

string VideoFader::LOG_NAME = "VideoFader";

VideoFader::VideoFader() {
	// TODO Auto-generated constructor stub

}

VideoFader::~VideoFader() {
	// TODO Auto-generated destructor stub
}


void VideoFader::setup(ofBaseVideoDraws * video){
	state = Video1;
	ofDirectory dir;
	dir.allowExt("mp4");
	dir.listDir("recordings");
	unsigned int pos = ofRandom(0,dir.size()-1);
	player1.loadMovie(dir.getPath(pos));
	player2.loadMovie(dir.getPath((pos+1)%dir.size()));
	ofLogVerbose(LOG_NAME) << "loading video 1" << dir.getPath(pos);
	ofLogVerbose(LOG_NAME) << "loading video 2" << dir.getPath((pos+1)%dir.size());
	player1.play();
	startTime = ofGetElapsedTimeMillis();
	live = video;
	ofNotifyEvent(stateChanged,state);
}

void VideoFader::update(){
	player1.update();
	player2.update();

	if(state==Video1 && player1.getIsMovieDone()){
		state = Video2;
		startTime = ofGetElapsedTimeMillis();
		player2.play();
		ofNotifyEvent(stateChanged,state);
	}

	if(state==Video2 && player2.getIsMovieDone()){
		state = Live;
		startTime = ofGetElapsedTimeMillis();
		ofNotifyEvent(stateChanged,state);
	}
}

void VideoFader::draw(float x, float y, float w, float h){
	int time = ofGetElapsedTimeMillis() - startTime;
	if(state==Video1){
		ofSetColor(255,255,255,ofMap(time,0,1000,255,0,true));
		live->draw(x,y,w,h);
		ofSetColor(255,255,255,ofMap(time,0,1000,0,255,true));
		player1.draw(x,y,w,h);
	}else if(state==Video2){
		ofSetColor(255,255,255,ofMap(time,0,1000,255,0,true));
		player1.draw(x,y,w,h);
		ofSetColor(255,255,255,ofMap(time,0,1000,0,255,true));
		player2.draw(x,y,w,h);
	}else if(state==Live){
		ofSetColor(255,255,255,ofMap(time,0,1000,255,0,true));
		player2.draw(x,y,w,h);
		ofSetColor(255,255,255,ofMap(time,0,1000,0,255,true));
		live->draw(x,y,w,h);
	}
}

float VideoFader::getDuration(){
	if(state==Video1){
		return player1.getDuration();
	}
	if(state == Video2){
		return player2.getDuration();
	}
	return 0;
}

float VideoFader:: getRemainingPct(){
	if(state==Video1){
		return player1.getDuration() - player1.getDuration()*player1.getPosition();
	}
	if(state == Video2){
		return player2.getDuration() - player2.getDuration()*player2.getPosition();
	}
	return 0;
}
