/*
 * VideoFader.h
 *
 *  Created on: 17/02/2012
 *      Author: arturo
 */

#ifndef VIDEOFADER_H_
#define VIDEOFADER_H_

#include "ofVideoPlayer.h"
#include "ofBaseTypes.h"
#include "ofEvents.h"
#include "ofxGui.h"

class VideoFader {
public:
	VideoFader();
	virtual ~VideoFader();

	void setup(ofBaseVideoDraws * video);
	void update();
	void draw(float x, float y, float w, float h);

	float getRemainingPct();
	float getDuration();

	static string LOG_NAME;

	ofxParameter<int> numVideos;
	ofxParameter<int> fadeMillis;

	enum State{
		Live,
		Video
	};

	ofEvent<State> stateChanged;

private:
	vector<ofVideoPlayer> players;
	ofBaseVideoDraws * live;
	int startTime;
	int millisFinishedLastVideo;
	int currentVideo;
	State state;
};

#endif /* VIDEOFADER_H_ */
