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

	void setup(FaceLoader * faceLoader, BlinkTrigger * trigger, ofMesh * camMesh, ofxFaceTrackerThreaded * tracker, VideoFader * videoFader, FaceBlinkRecorder * faceBlinkRecorder, AutoExposure * autoExposure, ofTexture * input, ofxParameter<int> numInputRotations);
	void update();
	void draw();

	void faceLoaderModeChanged(bool & m);
	void rotationScreenChanged(int & rot);
	void rotationCameraChanged(int & rot);
	void videoFaderStateChanged(VideoFader::State & state);

	ofxPanel gui;
	ofxToggle faceLoaderMode;
	ofxToggle showGraphs;
	ofxToggle showMesh;
	ofxToggle showVideos;
	ofxToggle showMugs;
	ofxToggle showInput;
	ofxIntSlider currentFace;
	ofxIntSlider rotationCamera,rotationScreen;
	ofxIntSlider millisLongBlink;
	ofxFloatSlider secsFaceLostBetweenRecordings;
	ofxFloatSlider secsFaceLostToDropRecording;
	ofxFloatSlider secsEyesClosedToEndRecording;
	ofxFloatSlider secsToRecord;
	ofxIntSlider fadeMillis;
	ofxIntSlider minExposure, maxExposure;
	ofxFloatSlider screenScale;
	ofxFloatSlider faderRemaining;
	ofxFloatSlider videoFps;
	ofxIntSlider cloneStrenght;
	ofxToggle recordInteraction;

private:
	FaceLoader * faceLoader;
	BlinkTrigger * blinkTrigger;
	ofMesh * camMesh;
	ofxFaceTrackerThreaded * tracker;
	VideoFader * videoFader;
	FaceBlinkRecorder * faceBlinkRecorder;
	AutoExposure * autoExposure;
	ofTexture * input;
};

#endif /* GUI_H_ */
