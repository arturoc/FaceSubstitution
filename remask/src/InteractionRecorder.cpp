/*
 * InteractionRecorder.cpp
 *
 *  Created on: 10/03/2012
 *      Author: arturo
 */

#include "InteractionRecorder.h"

InteractionRecorder::InteractionRecorder() {
	settings.loadFile("serversettings.xml");
	ftpServer = settings.getValue("interaction_server","arturocastro.net");
	user = settings.getValue("interaction_user","user");
	password = settings.getValue("interaction_password","password");
	serverPath = settings.getValue("interaction_recordings_path","interaction_recordings_path");
	folder = "recordings_interaction";
	recorder = NULL;
}

InteractionRecorder::~InteractionRecorder() {
	// TODO Auto-generated destructor stub
}



void InteractionRecorder::setup(string path,string currentFace, int w, int h, int fps){
	cout << "start recording at " << path << " with current face " << currentFace;
	recorder = new ofxGstVideoRecorder;
	recordedVideoPath = path;
	recorder->setUseAudio(true,"alsa_input.usb-046d_HD_Webcam_C525_4AB5B2F0-00-C525.analog-mono");
	recorder->setup(w,h,24,"recordings_interaction/"+recordedVideoPath,ofxGstVideoRecorder::THEORA,fps);
	framesRecorded = 0;

	if(currentFace!=""){
		ofFile recordedVideoMeta(ofFilePath::join(folder,recordedVideoPath+".meta"),ofFile::WriteOnly);
		recordedVideoMeta << currentFace << " " << framesRecorded << endl;
	}
	if(!isThreadRunning()) startThread(true,false);
}

void InteractionRecorder::addFrame(ofPixels & frame){
	recorder->newFrame(frame);
	framesRecorded++;
}

void InteractionRecorder::changeFace(string face){
	cout << "changed face to " << face;
	ofFile recordedVideoMeta(ofFilePath::join(folder,recordedVideoPath+".meta"),ofFile::Append);
	recordedVideoMeta << face << " " << framesRecorded << endl;
}

void InteractionRecorder::close(){
	cout << endl << "closing InteractionRecorder " << recordedVideoPath << " with " << framesRecorded << " frames" << endl;

	if(framesRecorded>180){
		lock();
		framesRecorded = 0;
		recorder->shutdown();
		recordersQueue.push(recorder);
		uploadQueue.push(recordedVideoPath);
		unlock();
		upload.signal();
	}else{
		if(recorder){
			delete recorder;
			recorder = NULL;
			ofFile(ofFilePath::join(ofToDataPath(folder),recordedVideoPath)).remove();
		}
	}
}

void InteractionRecorder::threadedFunction(){
	while(isThreadRunning()){
		cout << "InteractionRecorder thread waiting" << endl;
		upload.wait(mutex);
		cout << "InteractionRecorder thread passed waiting" << endl;
		while(!uploadQueue.empty()){
			string nextUpload = uploadQueue.front();
			uploadQueue.pop();
			ofxGstVideoRecorder * recorder = recordersQueue.front();
			recordersQueue.pop();
			unlock();
			cout << "closing recorder for " << nextUpload << endl;
			string videoFile = ofToDataPath(ofFilePath::join(ofToDataPath(folder),nextUpload));
			string ftpVideoFile = " ftp://" + ofFilePath::join(ofFilePath::join(ftpServer , serverPath) , nextUpload);
			string ftpCommand = "curl -u " + user+ ":" + password + " -T " +  videoFile + " " + ftpVideoFile;
			cout << endl << ftpCommand << " 2> /dev/null " << endl;
			system(ftpCommand.c_str());

			string metaFile = ofToDataPath(string(ofFilePath::join(folder,nextUpload + ".meta")));
			if(ofFile(metaFile).exists()){
				ftpCommand = "curl -u " + user+ ":" + password + " -T " + metaFile + " ftp://" + string(ofFilePath::join(ftpServer , serverPath)) + nextUpload + ".meta";
				cout << endl << ftpCommand << " 2> /dev/null " << endl;
				system(ftpCommand.c_str());
			}
			while(!recorder->isClosed()){
				ofSleepMillis(100);
			}
			delete recorder;
			lock();
		}
		cout << "no more videos in queue" << endl;
	}
}
