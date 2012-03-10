#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 720,1280,OF_GAME_MODE);
	ofRunApp(new testApp());
}
