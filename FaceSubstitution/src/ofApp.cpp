#include "ofApp.h"
#include "ofGstUtils.h"
#include "ofGstVideoGrabber.h"

using namespace ofxCv;

void testApp::setup() {
#ifdef TARGET_OSX
	ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);


	cloneReady = false;
    cam.setup(1920,1080,false);
    camTracker.setup();
    clone.setup(cam.getWidth(), cam.getHeight());
    autoExposure.setup(0,cam.getWidth(),cam.getHeight());
    gui.setup(autoExposure,clone,cam);
    int numRotations = gui.numRotations;
    this->allocate(numRotations);

    gui.numRotations.addListener(this, &testApp::allocate);

    faceLoader.setup("faces", FaceLoader::Random);

	lastFound = 0;
	faceChanged = false;

	ofSetBackgroundAuto(false);

	ofGstVideoUtils * gst = ((ofGstVideoGrabber*)cam.getGrabber().get())->getGstVideoUtils();
	ofAddListener(gst->bufferEvent,this,&testApp::onNewFrame);

	ofHideCursor();


	refreshOnNewFrameOnly = false;
    //mutex.lock();
	numCamFrames = 0;

}

void testApp::allocate(int & numRotations) {
    if(numRotations == 1 || numRotations == 3) {
        camRotated.allocate(cam.getHeight(), cam.getWidth(), 3);
    }else{
        camRotated.allocate(cam.getWidth(), cam.getHeight(), 3);
    }
    grayPixels.allocate(camRotated.getWidth(),camRotated.getHeight(),1);
    camTex.allocate(camRotated.getWidth(), camRotated.getHeight(), GL_RGB);
    clone.allocate(camRotated.getWidth(), camRotated.getHeight());
    ofFbo::Settings settings;
    settings.width = camRotated.getWidth();
    settings.height = camRotated.getHeight();
    srcFbo.allocate(settings);
}

void testApp::onNewFrame(ofPixels & pixels){
    if(refreshOnNewFrameOnly) condition.notify_all();
    gui.newCamFrame();
}

void testApp::update() {
    auto lock = std::unique_lock(mutex);
    if(refreshOnNewFrameOnly) condition.wait(lock);

	cam.update();
	faceLoader.update();
    if(refreshOnNewFrameOnly || cam.isFrameNew()){
        gui.newCamProcessFrame();
        if(gui.numRotations>0){
            cam.getPixels().rotate90To(camRotated, gui.numRotations);
            camTex.loadData(camRotated);
            convertColor(camRotated, grayPixels, cv::COLOR_RGB2GRAY);
        }else{
            camTex.loadData(cam.getPixels());
            convertColor(cam, grayPixels, cv::COLOR_RGB2GRAY);
        }
        camTracker.update(toCv(grayPixels));
		cloneReady = camTracker.getFound();
		if(cloneReady) {
            camMesh = camTracker.getImageMesh();
			camMeshWithPicTexCoords = camMesh;
			camMeshWithPicTexCoords.getTexCoords() = faceLoader.getCurrentImagePoints();

            srcFbo.begin();
            ofClear(0, 0);
			faceLoader.getCurrentImg().bind();
			camMeshWithPicTexCoords.draw();
			faceLoader.getCurrentImg().unbind();
			srcFbo.end();

			lastFound = 0;
			faceChanged = false;

			if(numCamFrames%2==0){
				autoExposureBB = camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE).getBoundingBox();
				autoExposureBB.scaleFromCenter(.5);
			}

		}else{
			if(!faceChanged){
				lastFound++;
				if(lastFound>5){
					faceLoader.loadNext();
					faceChanged = true;
					lastFound = 0;
				}
			}

            if(numCamFrames%2==0){
                autoExposureBB.set(0,0,camRotated.getWidth(),camRotated.getHeight());
				autoExposureBB.scaleFromCenter(.5);
			}
		}

		//TODO: fix BB when image is rotated
        if(numCamFrames%2==0){
            autoExposure.update(grayPixels,autoExposureBB);
		}
		numCamFrames++;
	}
    gui.update(autoExposureBB);
}

void testApp::draw() {
	ofSetColor(255);
    ofPushMatrix();

    if(faceLoader.getCurrentImg().getWidth() > 0 && cloneReady) {
        ofTranslate(ofGetWidth(),0);
        float s = float(ofGetWidth())/ float(camRotated.getWidth());
        ofScale(-s, s, 1);
        clone.draw(srcFbo.getTexture(), camTex, camMesh);


        if (gui.show) {
//            for(const auto & uv: camMeshWithPicTexCoords.getTexCoords()) {
//                camMeshWithPicTexCoords.addColor({uv.x, uv.y, 0.});
//                std::cout << ofColor{uv.x, uv.y, 0.} << std::endl;
//            }

//            faceLoader.getCurrentImg().bind();
//            camMeshWithPicTexCoords.draw();
//            faceLoader.getCurrentImg().unbind();

//            ofClear(0, 0);
//            srcFbo.draw(0, 0);
//            camMesh.drawWireframe();

//            std::cout << ofGetWidth() << "," << camRotated.getWidth() << std::endl;
//            camTex.draw(0, 0, camTex.getWidth(), camTex.getHeight());
        }
    } else {
        ofTranslate(ofGetWidth(), 0);
        camTex.draw(0, 0, -ofGetWidth(), ofGetHeight());
    }

	ofPopMatrix();


    gui.draw();
}


void testApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		faceLoader.loadNext();
		break;
	case OF_KEY_DOWN:
		faceLoader.loadPrevious();
		break;
	case ' ':
		refreshOnNewFrameOnly = !refreshOnNewFrameOnly;
		break;
	case 'g':
		gui.show=!gui.show;
		if(gui.show){
			ofShowCursor();
		}else{
			ofHideCursor();
		}
		break;
	}
}
