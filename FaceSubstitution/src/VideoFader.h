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

	enum State{
		Video1,
		Video2,
		Live
	};

	ofEvent<State> stateChanged;

private:
	ofVideoPlayer player1, player2;
	ofBaseVideoDraws * live;
	State state;
	int startTime;
	int millisFinishedLastVideo;
};

#endif /* VIDEOFADER_H_ */
