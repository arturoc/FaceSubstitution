/*
 * InteractionRecorder.cpp
 *
 *  Created on: 10/03/2012
 *      Author: arturo
 */

#include "InteractionRecorder.h"

InteractionRecorder::InteractionRecorder() {
	settings.loadFile("serversettings.xml");
	ftpServer = settings.getValue("server","arturocastro.net");
	user = settings.getValue("user","user");
	password = settings.getValue("password","password");
	serverPath = settings.getValue("interaction_recordings_path","interaction_recordings_path");
	folder = "recordings_interaction";
}

InteractionRecorder::~InteractionRecorder() {
	// TODO Auto-generated destructor stub
}



void InteractionRecorder::setup(string path,string currentFace, int w, int h, int fps){
	cout << "start recording at " << path << " with current face " << currentFace;
	recorder = ofPtr<ofxVideoRecorder>(new ofxVideoRecorder);
	recordedVideoPath = path;
	recorder->setup("recordings_interaction/"+recordedVideoPath,w,h,fps);
	framesRecorded = 0;
	ofFile recordedVideoMeta(ofFilePath::join(folder,recordedVideoPath)+".meta",ofFile::WriteOnly);
	recordedVideoMeta << currentFace << " " << framesRecorded << endl;
	if(!isThreadRunning()) startThread(true,false);
}

void InteractionRecorder::addFrame(ofPixels & frame){
	recorder->addFrame(frame);
	framesRecorded++;
}

void InteractionRecorder::changeFace(string face){
	cout << "changed face to " << face;
	ofFile recordedVideoMeta(ofFilePath::join(folder,recordedVideoPath)+".meta",ofFile::Append);
	recordedVideoMeta << face << " " << framesRecorded << endl;
}

void InteractionRecorder::close(){
	lock();
	recordersQueue.push(recorder);
	uploadQueue.push(recordedVideoPath);
	unlock();
	upload.signal();
}

void InteractionRecorder::threadedFunction(){
	while(isThreadRunning()){
		upload.wait(mutex);
		while(!uploadQueue.empty()){
			string nextUpload = uploadQueue.front();
			uploadQueue.pop();
			{
				ofPtr<ofxVideoRecorder> recorder = recordersQueue.front();
				recordersQueue.pop();
				unlock();
				recorder->close();
			}
			string ftpCommand = "curl -u " + user+ ":" + password + " -T " + ofToDataPath(ofFilePath::join(folder,nextUpload + ".mjpg")) + " ftp://" + ofFilePath::join(ftpServer , serverPath) + nextUpload + ".mjpg";
			system(ftpCommand.c_str());
			ftpCommand = "curl -u " + user+ ":" + password + " -T " + ofToDataPath(ofFilePath::join(folder,nextUpload + ".meta")) + " ftp://" + ofFilePath::join(ftpServer , serverPath) + nextUpload + ".meta";
			system(ftpCommand.c_str());
			lock();
		}
	}
}
