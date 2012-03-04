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
#include "BlinkTrigger.h"
#include "VideoFader.h"
#include "FaceBlinkRecorder.h"
#include "AutoExposure.h"

class Gui {
public:
	Gui();
	virtual ~Gui();

	void setup(FaceLoader * faceLoader, BlinkTrigger * trigger, ofMesh * camMesh, ofxFaceTrackerThreaded * tracker, VideoFader * videoFader, FaceBlinkRecorder * faceBlinkRecorder, AutoExposure * autoExposure, ofxParameter<int> numInputRotations);
	void update();
	void draw();

	void faceLoaderModeChanged(bool & m);
	void rotationChanged(int & rot);
	void videoFaderStateChanged(VideoFader::State & state);

	ofxPanel gui;
	ofxToggle faceLoaderMode;
	ofxToggle showGraphs;
	ofxToggle showMesh;
	ofxToggle showVideos;
	ofxToggle showMugs;
	ofxIntSlider currentFace;
	ofxIntSlider rotation;
	ofxIntSlider millisLongBlink;
	ofxFloatSlider faderRemaining;
	ofxFloatSlider videoFps;

private:
	FaceLoader * faceLoader;
	BlinkTrigger * blinkTrigger;
	ofMesh * camMesh;
	ofxFaceTrackerThreaded * tracker;
	VideoFader * videoFader;
	FaceBlinkRecorder * faceBlinkRecorder;
	AutoExposure * autoExposure;
};

#endif /* GUI_H_ */
