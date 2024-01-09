/*
 * Gui.h
 *
 *  Created on: Sep 8, 2013
 *      Author: arturo
 */

#ifndef GUI_H_
#define GUI_H_

#include "AutoExposure.h"
#include "Clone.h"
#include "ofParameter.h"
#include "ofxGui.h"
#include "ofxOscParameterSync.h"
#include "ofxFPS.h"

class Gui {
public:

	void setup(AutoExposure & autoExposure, Clone & clone, ofVideoGrabber & cam);
	void update(const ofRectangle & bb);
	void draw();

	void onSave(bool & save);

	void newCamProcessFrame();
	void newCamFrame();

	ofParameter<bool> show;
	ofParameter<int> numRotations;
    ofParameter<bool> save;
private:
	AutoExposure * autoExposure;
	Clone * clone;
    ofVideoGrabber * cam;
    ofParameterGroup parameters;
	ofxPanel gui;

    //ofxOscParameterSync oscParamSync;
	ofRectangle autoExposureBB;
	ofxFPS camFPS, camRealFPS;
	ofTrueTypeFont font;
    ofEventListener listener;
};

#endif /* GUI_H_ */
