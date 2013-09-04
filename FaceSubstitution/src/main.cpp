#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 960, 544, OF_FULLSCREEN);
	ofRunApp(new testApp());
}
