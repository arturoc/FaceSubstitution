/*
 * VideoFader.cpp
 *
 *  Created on: 17/02/2012
 *      Author: arturo
 */

#include "VideoFader.h"
#include "ofMath.h"
#include "ofGraphics.h"
#include "Utils.h"

string VideoFader::LOG_NAME = "VideoFader";

VideoFader::VideoFader() {
	state = Live;
	numVideos = 3;
	fadeMillis = 1200;
}

VideoFader::~VideoFader() {
	// TODO Auto-generated destructor stub
}


void VideoFader::setup(ofBaseVideoDraws * video){
	ofDirectory dir("recordings");
	if(!dir.exists()) dir.create(true);
	dir.allowExt("mp4");
	dir.listDir();
	if(dir.size()){
		currentVideo = 0;
		state = Video;
		unsigned int pos = ofRandom(0,dir.size()-1);
		players.resize(numVideos);
		for(int i=0;i<players.size();i++){
			players[i].loadMovie(dir.getPath(pos));
			ofLogVerbose(LOG_NAME) << "loading video" << i << dir.getPath(pos);
			pos = randomDifferent(0,dir.size()-1,pos);
		}
		players[0].play();
		currentVideo = 0;
		ofNotifyEvent(stateChanged,state);
	}
	startTime = ofGetElapsedTimeMillis();
	live = video;
}

void VideoFader::update(){

	for(int i=0;i<players.size();i++){
		players[i].update();
	}

	if(state==Video && players[currentVideo].getIsMovieDone()){
		currentVideo++;
		if(currentVideo<players.size()){
			startTime = ofGetElapsedTimeMillis();
			players[currentVideo].play();
		}else{
			state = Live;
			ofNotifyEvent(stateChanged,state);
		}
	}

}

void VideoFader::draw(float x, float y, float w, float h){
	int time = ofGetElapsedTimeMillis() - startTime;
	if(state==Video && currentVideo==0){
		ofSetColor(255,255,255,ofMap(time,0,fadeMillis,255,0,true));
		live->draw(x,y,w,h);
		ofSetColor(255,255,255,ofMap(time,0,fadeMillis,0,255,true));
		players[currentVideo].draw(x,y,w,h);
	}else if(state==Video){
		ofSetColor(255,255,255,ofMap(time,0,fadeMillis,255,0,true));
		players[currentVideo-1].draw(x,y,w,h);
		ofSetColor(255,255,255,ofMap(time,0,fadeMillis,0,255,true));
		players[currentVideo].draw(x,y,w,h);
	}else if(state==Live){
		if(!players.empty()){
			ofSetColor(255,255,255,ofMap(time,0,fadeMillis,255,0,true));
			players[players.size()-1].draw(x,y,w,h);
		}
		ofSetColor(255,255,255,ofMap(time,0,fadeMillis,0,255,true));
		live->draw(x,y,w,h);
	}
}

float VideoFader::getDuration(){
	if(state==Video){
		return players[currentVideo].getDuration();
	}
	return 1;
}

float VideoFader:: getRemainingPct(){
	if(state==Video){
		return players[currentVideo].getDuration() - players[currentVideo].getDuration()*players[currentVideo].getPosition();
	}
	return 1;
}
