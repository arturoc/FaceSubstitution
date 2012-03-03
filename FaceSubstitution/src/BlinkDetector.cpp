/*
 * BlinkDetector.cpp
 *
 *  Created on: 15/12/2011
 *      Author: arturo
 */

#include "BlinkDetector.h"

ofVec3f normal_Polygon(const ofPolyline & V){
	ofVec3f N;
	int t = (int)V.size();
	for (int n=0;n<(int)V.size();n++)
	{
		int j = (n + 1) % t;
		N.x += (V[n].y - V[j].y) * (V[n].z + V[j].z);
		N.y += (V[n].z - V[j].z) * (V[n].x + V[j].x);
		N.z += (V[n].x - V[j].x) * (V[n].y + V[j].y);
	}

	return N.getNormalized();
}

float area3D_Polygon( const ofPolyline & V ){
	ofVec3f va;
	for(int i=1;i<(int)V.size();i++){
		va += (V[i]*V[i-1])*0.5;
	}
	return abs(normal_Polygon(V).dot(va));
}

/*float area3D_Polygon( const ofPolyline & V ){
	int n = V.size();
	ofVec3f N = normal_Polygon(V);
    float area = 0;
    float an, ax, ay, az;  // abs value of normal and its coords
    int   coord;           // coord to ignore: 1=x, 2=y, 3=z
    int   i, j, k;         // loop indices

    // select largest abs coordinate to ignore for projection
    ax = (N.x>0 ? N.x : -N.x);     // abs x-coord
    ay = (N.y>0 ? N.y : -N.y);     // abs y-coord
    az = (N.z>0 ? N.z : -N.z);     // abs z-coord

    coord = 3;                     // ignore z-coord
    if (ax > ay) {
        if (ax > az) coord = 1;    // ignore x-coord
    }
    else if (ay > az) coord = 2;   // ignore y-coord

    // compute area of the 2D projection
    for (i=1, j=2, k=0; i<=n; i++, j++, k++)
        switch (coord) {
        case 1:
            area += (V[i].y * (V[j].z - V[k].z));
            continue;
        case 2:
            area += (V[i].x * (V[j].z - V[k].z));
            continue;
        case 3:
            area += (V[i].x * (V[j].y - V[k].y));
            continue;
        }

    // scale to get area before projection
    an = sqrt( ax*ax + ay*ay + az*az);  // length of normal vector
    switch (coord) {
    case 1:
        area *= (an / (2*ax));
        break;
    case 2:
        area *= (an / (2*ay));
        break;
    case 3:
        area *= (an / (2*az));
        break;
    }
    return area;
}*/

string BlinkDetector::LOG_NAME = "BlinkDetector";

BlinkDetector::BlinkDetector() {
	eyeClosed = false;
	prevArea = 0;

}

void BlinkDetector::setup(ofxFaceTracker * _tracker, ofxFaceTracker::Feature _eye){
	tracker = _tracker;
	eye = _eye;
}

void BlinkDetector::update(){
	const ofPolyline & eyeContour = tracker->getObjectFeature(eye);
	float area = eyeContour.getArea();

	/*mesh.clear();
	tess.tessellateToMesh(eyeContour,OF_POLY_WINDING_ODD,mesh);
	for(int i=0;i<mesh.getNumIndices();i+=3){
		ofPolyline tri;
		tri.addVertex(mesh.getVertex(mesh.getIndex(i)));
		tri.addVertex(mesh.getVertex(mesh.getIndex(i+1)));
		tri.addVertex(mesh.getVertex(mesh.getIndex(i+2)));
		area += area3D_Polygon(tri);
	}*/

	float max=-1, min=99;

	deque<float>::iterator it;
	for(it=latestEyeOpennes.begin();it!=latestEyeOpennes.end();it++){
		if(max<*it) max=*it;
		if(min>*it) min=*it;
	}

	float faceInclination = tracker->getObjectPoint(27).z - tracker->getObjectPoint(21).z;

	//ofLogVerbose(LOG_NAME) << "area" << area;

	//ofLogVerbose(LOG_NAME) << "face angle" << faceInclination;

	//ofLogVerbose(LOG_NAME) << "thres" << (max+min)*0.5-area;

	latestEyeOpennes.push_back(area);
	if(latestEyeOpennes.size()>60){
		latestEyeOpennes.pop_front();
	}

	if(latestEyeOpennes.size()>5 && (max+min)*0.5-area > area*.2){
		eyeClosed = true;
	}else{ // if(!eyesOpened && avg-(max+min)*0.5 > .2){
		eyeClosed = false;
	}

	graphArea.setThreshold(((max+min)*0.5-area*.2));
	graphArea.addSample(area);
	graphDerivative.addSample(area-prevArea);
	graphBool.addSample(eyeClosed);

	prevArea = area;
}

float BlinkDetector::getOpennes(){
	if(!latestEyeOpennes.empty())
		return latestEyeOpennes[latestEyeOpennes.size()-1];
	else
		return 0;
}

bool BlinkDetector::isClosed(){
	return eyeClosed;
}

void BlinkDetector::reset(){
	latestEyeOpennes.clear();
}

deque<float> BlinkDetector::getHistory(){
	return latestEyeOpennes;
}

