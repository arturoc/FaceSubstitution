#include "ofApp.h"


int main() {
    ofSetupOpenGL(640 * 2, 480 * 2, OF_WINDOW);
    return ofRunApp(std::make_shared<ofApp>());
}
