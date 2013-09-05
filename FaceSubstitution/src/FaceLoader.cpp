/*
 * FaceLoader.cpp
 *
 *  Created on: 22/01/2012
 *      Author: arturo
 */

#include "FaceLoader.h"
#include "ofxFaceTracker.h"
#include "Poco/ScopedLock.h"
#include "Utils.h"

#include "ofxCv.h"
using namespace ofxCv;

FaceLoader::FaceLoader() {
	currentImg = &src[0];
	currentPoints = &srcPoints[0];
	nextImg = &src[1];
	nextPoints = &srcPoints[1];
}

FaceLoader::~FaceLoader() {
	// TODO Auto-generated destructor stub
}

void FaceLoader::resizeAndDiscardImages(string folder){
	ofxFaceTracker	tracker;
	tracker.setup();
	tracker.setIterations(25);
	tracker.setAttempts(4);
	faces.listDir(folder);
	if(faces.size()==0) return;

	for(int i=0;i<(int)faces.size();i++){
		string path = faces.getPath(i);
		src[0].loadImage(path);
		bool saveCopy = false;
		if((src[0].getWidth()>1000 || src[0].getHeight()>1000)){
			saveCopy = true;
			while(src[0].getWidth()>1000 || src[0].getHeight()>1000){
				src[0].resize(src[0].getWidth()/2., src[0].getHeight()/2.);
			}
		}
		if(src[0].getWidth() > 0) {
			tracker.update(toCv(src[0]));
			srcPoints[0] = tracker.getImagePoints();
		}
		if (!tracker.getFound()){
			ofLogVerbose("testApp") << "moving" << path;
			ofFile(path).moveTo("non_working");
		}else if(saveCopy){
			ofLogVerbose("testApp") << "scaling" << path;
			src[0].saveImage(path,OF_IMAGE_QUALITY_BEST);
		}
	}
}


void FaceLoader::setup(string folder, Mode _mode){
	tracker.setup();
	tracker.setIterations(25);
	tracker.setAttempts(4);

	faces.allowExt("jpg");
	faces.allowExt("png");
	//resizeAndDiscardImages(folder);

	currentFace = 0;
	faces.listDir(folder);
	faces.sort();

	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
		std::swap(nextPoints,currentPoints);
		std::swap(nextImg,currentImg);
		previousFace = currentFace;
		currentFace = randomDifferent(0, faces.size() - 1, currentFace);
		loadFace(faces.getPath(currentFace));
	}

	mode = _mode;
	loadNextFace = false;
	startThread(true,false);
}

void FaceLoader::update(){
	if(loadNextFace){
		currentImg->setUseTexture(true);
		currentImg->update();
		loadNextFace = false;
	}
}

void FaceLoader::setMode(Mode _mode){
	mode = _mode;
}


void FaceLoader::threadedFunction(){
	mutex.lock();
	while(isThreadRunning()) {
		loadNew.wait(mutex);
		cout << "loading " << faces.getPath(currentFace) << endl;
		string facePath = faces.getPath(currentFace);
		loadFace(facePath);
	}
}


void FaceLoader::loadFace(string face){
	nextImg->loadImage(face);
	if(nextImg->getWidth() > 0) {
		tracker.update(toCv(*nextImg));
		ofMesh imageMesh;
		ofFile loadedMeshFile(face+".ply",ofFile::Reference);
		if(loadedMeshFile.exists()){
			imageMesh.load(face+".ply");
			*nextPoints = imageMesh.getTexCoords();
		}else{
			*nextPoints = tracker.getImagePoints();
		}
	}
}

string FaceLoader::getCurrentFacePath(){
	return faces.getPath(currentFace);
}

string FaceLoader::getPreviousFacePath(){
	return faces.getPath(previousFace);
}

ofImage & FaceLoader::getCurrentImg(){
	return *currentImg;
}

ofxFaceTracker & FaceLoader::getTracker(){
	return tracker;
}

vector<ofVec2f> & FaceLoader::getCurrentImagePoints(){
	return *currentPoints;
}

string FaceLoader::loadNext(){
	mutex.lock();
	previousFace = currentFace;
	std::swap(nextPoints,currentPoints);
	std::swap(nextImg,currentImg);
	nextImg->setUseTexture(false);
	loadNextFace = true;
	mutex.unlock();
	if(mode==Random){
		currentFace = randomDifferent(0, faces.size(), currentFace);
	}else{
		currentFace++;
		currentFace %= faces.size();
	}
	loadNew.signal();
	return faces.getPath(currentFace);
}

string FaceLoader::loadPrevious(){
	mutex.lock();
	previousFace = currentFace;
	std::swap(nextPoints,currentPoints);
	std::swap(nextImg,currentImg);
	nextImg->setUseTexture(false);
	loadNextFace = true;
	mutex.unlock();
	if(mode==Random){
		currentFace = randomDifferent(0, faces.size(), currentFace);
	}else{
		currentFace--;
		currentFace %= faces.size();
	}
	loadNew.signal();
	return faces.getPath(currentFace);
}

int FaceLoader::getTotalFaces(){
	return faces.size();
}

int FaceLoader::getCurrentFace(){
	return currentFace;
}
