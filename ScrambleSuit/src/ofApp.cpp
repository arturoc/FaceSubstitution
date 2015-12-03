#include "ofApp.h"

ofMesh texturedRectMesh;
void texturedRect(float width, float height) {
	if(texturedRectMesh.getNumVertices() == 0) {
		texturedRectMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
		texturedRectMesh.addTexCoord(ofVec2f(0, 0));
		texturedRectMesh.addVertex(ofVec2f(0, 0));
		texturedRectMesh.addTexCoord(ofVec2f(0, 1));
		texturedRectMesh.addVertex(ofVec2f(0, 1));
		texturedRectMesh.addTexCoord(ofVec2f(1, 0));
		texturedRectMesh.addVertex(ofVec2f(1, 0));
		texturedRectMesh.addTexCoord(ofVec2f(1, 1));
		texturedRectMesh.addVertex(ofVec2f(1, 1));
	}
	ofPushMatrix();
	ofScale(width, height);
	texturedRectMesh.drawFaces();
	ofPopMatrix();
}

// this is important for avoiding slightl discrepencies when the mesh is
// projected, or processed by GL transforms vs OF transforms
void ofApp::normalizeMesh(ofMesh& mesh) {
	vector<ofVec3f>& vertices = mesh.getVertices();
	for(int i = 0; i < vertices.size(); i++) {
		vertices[i] *= normalizedMeshScale / normalizedWidth;
		vertices[i] += ofVec2f(normalizedWidth, normalizedHeight) / 2.;
		vertices[i].z = 0;
	}
}

void ofApp::drawNormalized(ofxFaceTracker& tracker) {
	ofClear(0, 0);
	ofMesh mesh = tracker.getMeanObjectMesh();
	normalizeMesh(mesh);	
	mesh.draw();
}

void ofApp::drawNormalized(ofxFaceTracker& tracker, ofBaseHasTexture& tex, ofFbo& result) {
	result.begin();
	tex.getTextureReference().bind();
	drawNormalized(tracker);
	tex.getTextureReference().unbind();
	result.end();
}

void ofApp::maskBlur(ofBaseHasTexture& tex, ofFbo& result) {
	int k = ofMap(mouseX, 0, ofGetWidth(), 1, 128, true);
	
	halfMaskBlur.begin();
	ofClear(0, 0);
	maskBlurShader.begin();
	maskBlurShader.setUniformTexture("tex", tex, 1);
	maskBlurShader.setUniformTexture("mask", faceMask, 2);
	maskBlurShader.setUniform2f("direction", 1, 0);
	maskBlurShader.setUniform1i("k", k);
	tex.getTextureReference().draw(0, 0);
	maskBlurShader.end();
	halfMaskBlur.end();
	
	result.begin();
	ofClear(0, 0);
	maskBlurShader.begin();
	maskBlurShader.setUniformTexture("tex", halfMaskBlur, 1);
	maskBlurShader.setUniformTexture("mask", faceMask, 2);
	maskBlurShader.setUniform2f("direction", 0, 1);
	maskBlurShader.setUniform1i("k", k);
	halfMaskBlur.draw(0, 0);
	maskBlurShader.end();
	result.end();
}
void ofApp::alphaBlur(ofBaseHasTexture& tex, ofFbo& result) {
	int k = ofMap(mouseY, 0, ofGetHeight(), 1, 25, true);
	
	halfAlphaBlur.begin();
	ofClear(0, 0);
	blurAlphaShader.begin();
	blurAlphaShader.setUniformTexture("tex", tex, 1);
	blurAlphaShader.setUniform2f("direction", 1, 0);
	blurAlphaShader.setUniform1i("k", k);
	tex.getTextureReference().draw(0, 0);
	blurAlphaShader.end();
	halfAlphaBlur.end();
	
	result.begin();
	ofClear(0, 0);
	blurAlphaShader.begin();
	blurAlphaShader.setUniformTexture("tex", halfAlphaBlur, 1);
	blurAlphaShader.setUniform2f("direction", 0, 1);
	blurAlphaShader.setUniform1i("k", k);
	halfAlphaBlur.draw(0, 0);
	blurAlphaShader.end();
	result.end();
}

void ofApp::normalizeImage(ofImage& img, ofImage& normalized) {
	srcTracker.update(toCv(img));
	if(srcTracker.getFound()) {
		drawNormalized(srcTracker, img, srcNormalized);
		normalized.allocate(normalizedWidth, normalizedHeight, OF_IMAGE_COLOR);
		srcNormalized.readToPixels(normalized.getPixelsRef());
		normalized.update();
	} else {
		ofLogWarning() << "couldn't find the face" << endl;
	}
}

void ofApp::setup() {
	ofSetVerticalSync(true);
	
	maskBlurShader.load("", "MaskBlur.frag");
	cloneShader.load("", "Clone.frag");
	blurAlphaShader.load("", "BlurAlpha.frag");
	voronoiShader.load("", "Voronoi.frag");
	
	dst.initGrabber(1280, 720);
	
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);
	dstTracker.setup();	
	
	ofFbo::Settings settings;
	settings.width = normalizedWidth;
	settings.height = normalizedHeight;
	srcNormalized.allocate(settings);
	dstNormalized.allocate(settings);
	halfMaskBlur.allocate(settings);
	srcBlur.allocate(settings);
	dstBlur.allocate(settings);
	faceMask.allocate(settings);
	
	settings.width = dst.getWidth();
	settings.height = dst.getHeight();
	cloned.allocate(settings);
	halfAlphaBlur.allocate(settings);
	final.allocate(settings);
	
	faceDirectory.listDir("faces");
	faceDirectory.sort();
	int n = 5;
	faces.resize(n);
	for(int i = 0; i < n; i++) {
		ofImage curFace;
		curFace.loadImage(faceDirectory[i]);
		normalizeImage(curFace, faces[i]);
	}
	pointsImage.allocate(n, 1, OF_IMAGE_COLOR);
	
	currentImage = 0;
	updateCurrentImage();
		
	faceMask.begin();
	drawNormalized(srcTracker);
	faceMask.end();
	
	referenceMeanMesh = srcTracker.getMeanObjectMesh();
	normalizeMesh(referenceMeanMesh);
	
	useVoronoi = false;
	debug = false;
}

void ofApp::buildVoronoiFace() {
	ofSeedRandom(0);
	float* pixels = pointsImage.getPixels();
	for(int i = 0; i < faces.size(); i++) {
		float speed = .4;
		pixels[i * 3 + 0] = ofNoise(ofGetElapsedTimef() * speed + ofRandom(1024));
		pixels[i * 3 + 1] = ofNoise(ofGetElapsedTimef() * speed + ofRandom(1024));
		pixels[i * 3 + 2] = 0;
	}
	pointsImage.update();
	
	srcNormalized.begin();
	voronoiShader.begin();
	voronoiShader.setUniform1i("count", faces.size());
	for(int i = 0; i < faces.size(); i++) {
		string texName = "tex" + ofToString(i);
		voronoiShader.setUniformTexture(texName.c_str(), faces[i], i);
	}
	voronoiShader.setUniformTexture("points", pointsImage, faces.size());
	texturedRect(normalizedWidth, normalizedHeight);
	voronoiShader.end();
	srcNormalized.end();
	
	ofTranslate(0, normalizedHeight);
	int y = 0, x = 0;
	float gridScale = .5;
	ofPushMatrix();
	ofScale(gridScale, gridScale);
	for(int i = 0; i < faces.size(); i++) {
		faces[i].draw(x, y);
		x += faces[i].getWidth();
		if(x + faces[i].getWidth() > ofGetWidth() / gridScale) {
			x = 0;
			y += faces[i].getHeight();
		}
	}
	ofPopMatrix();
}

void ofApp::updateCurrentImage() {
	src.loadImage(faceDirectory.getPath(currentImage));
	srcTracker.update(toCv(src));
	if(srcTracker.getFound()) {
		drawNormalized(srcTracker, src, srcNormalized);
	} else {
		ofLogWarning() << "couldn't load " << faceDirectory.getPath(currentImage) << endl;
	}
}

void ofApp::update() {
	dst.update();
	if(dst.isFrameNew()) {
		dstTracker.update(toCv(dst));
		drawNormalized(dstTracker, dst, dstNormalized);
		
		if(useVoronoi) {
			buildVoronoiFace();
		}
		
		maskBlur(srcNormalized, srcBlur);
		maskBlur(dstNormalized, dstBlur);
		
		ofMesh dstMesh = dstTracker.getImageMesh();
		dstMesh.clearTexCoords();
		vector<ofVec3f>& vertices = referenceMeanMesh.getVertices();
		for(int i = 0; i < vertices.size(); i++) {
			dstMesh.addTexCoord(ofVec2f(vertices[i].x, vertices[i].y));
		}
		
		cloned.begin();
		ofClear(0, 0);
		cloneShader.begin();
		cloneShader.setUniformTexture("src", srcNormalized, 1);
		cloneShader.setUniformTexture("srcBlur", srcBlur, 2);
		cloneShader.setUniformTexture("dstBlur", dstBlur, 3);
		dstMesh.draw();
		cloneShader.end();
		cloned.end();
		
		// alpha blur causes black fringes right now..
		//alphaBlur(cloned, final);
	}
}

void ofApp::draw() {
	ofEnableAlphaBlending();
	
	ofSetColor(255);
	dst.draw(0, 0);
	cloned.draw(0, 0);
	if(debug) {
		dstTracker.draw();
		ofPushMatrix();
		ofScale(.5, .5);
		srcNormalized.draw(0, 0);
		dstNormalized.draw(normalizedWidth, 0);
		srcBlur.draw(0, normalizedHeight);
		dstBlur.draw(normalizedWidth, normalizedHeight);
		ofPopMatrix();
	}
	
	ofDisableAlphaBlending();
	
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
}

void ofApp::keyPressed(int key) {
	if(key == 'r') {
		maskBlurShader.load("", "MaskBlur.frag");
		cloneShader.load("", "Clone.frag");
		blurAlphaShader.load("", "BlurAlpha.frag");
		voronoiShader.load("", "Voronoi.frag");
	}
	if(key == 'v') {
		useVoronoi = !useVoronoi;
		if(!useVoronoi) {
			updateCurrentImage();
		}
	}
	if(key == OF_KEY_RIGHT) {
		currentImage = ofClamp(currentImage + 1, 0, faceDirectory.size() - 1);
		updateCurrentImage();
	}
	if(key == OF_KEY_LEFT) {
		currentImage = ofClamp(currentImage - 1, 0, faceDirectory.size() - 1);
		updateCurrentImage();
	}
	if(key == 'd') {
		debug = !debug;
	}
}
