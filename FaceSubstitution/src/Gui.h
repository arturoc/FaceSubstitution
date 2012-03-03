/*
 * Gui.h
 *
 *  Created on: 03/03/2012
 *      Author: arturo
 */

#ifndef GUI_H_
#define GUI_H_

#include "ofxGui.h"
#include "FaceLoader.h"
#include "BlinkDetector.h"
#include "VideoFader.h"
#include "FaceBlinkRecorder.h"

class Gui {
public:
	Gui();
	virtual ~Gui();

	void setup(FaceLoader * faceLoader, BlinkDetector * leftBd, BlinkDetector * rightBd, ofMesh * camMesh, ofxFaceTrackerThreaded * tracker, VideoFader * videoFader, FaceBlinkRecorder * faceBlinkRecorder);
	void update();
	void draw();

	void faceLoaderModeChanged(bool & m);
	void videoFaderStateChanged(VideoFader::State & state);

	ofxPanel gui;
	ofxToggle faceLoaderMode;
	ofxToggle showGraphs;
	ofxToggle showMesh;
	ofxToggle showVideos;
	ofxToggle showMugs;
	ofxIntSlider currentFace;
	ofxFloatSlider faderRemaining;
	ofxFloatSlider videoFps;

private:
	FaceLoader * faceLoader;
	BlinkDetector * leftBD;
	BlinkDetector * rightBD;
	ofMesh * camMesh;
	ofxFaceTrackerThreaded * tracker;
	VideoFader * videoFader;
	FaceBlinkRecorder * faceBlinkRecorder;
};

#endif /* GUI_H_ */
