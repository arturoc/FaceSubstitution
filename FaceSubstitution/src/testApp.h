#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTrackerThreaded.h"
#include "FaceLoader.h"
#include "FaceBlinkRecorder.h"
#include "VideoFader.h"

//#define USE_GST_VIRTUAL_CAMERA

#ifdef USE_GST_VIRTUAL_CAMERA
#include <gst/app/gstappsrc.h>
#endif

#include "BlinkDetector.h"
#include "Gui.h"
#include "AutoExposure.h"
#include "SnapshotSaver.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
	
	void threadedUpdate(ofEventArgs & args);

	void keyPressed(int key);
	void recording(bool & rec);
	void showVideosChanged(bool & v);

	ofxFaceTrackerThreaded camTracker;
	ofVideoGrabber cam;
	ofVideoPlayer vid;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo, maskFbo;
	ofPixels maskPixels;

	FaceLoader faceLoader;

	bool live;

	ofBaseVideoDraws * video;
	ofMesh camMesh;

	ofPixels snapshot;
	float takeSnapshotFrom;
	SnapshotSaver snapshotSaver;

	int millisEyesClosed;
	int firstEyesClosedEvent;
	int millisToChange;
	bool faceChangedOnEyesClosed;

	BlinkDetector leftBD,rightBD;

	bool loadNextFace;

	ofxParameter<int> numInputRotation90;
	ofPixels rotatedInput;
	
	FaceBlinkRecorder blinkRecorder;
	VideoFader videoFader;
	bool isRecording;

	Gui gui;
	bool showGui;

	AutoExposure autoExposure;

};
