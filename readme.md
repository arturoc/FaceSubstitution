# FaceSubstitution

This repository stores some experiments from Arturo Castro and Kyle McDonald exploring realtime face substitution using face tracking and cloning. These examples here are based on a few other packages, including:

* [openFrameworks](http://www.openframeworks.cc/)
* [ofxCv](https://github.com/kylemcdonald/ofxCv)
* [ofxFaceTracker](https://github.com/kylemcdonald/ofxFaceTracker)

They all require openFrameworks. FaceSubstitution and ScrambleSuit require ofxCv and ofxFaceTracker. Example projects are provided as openFrameworks XCode projects. To compile on other operating systems, you should first build a working ofxCv project, then an ofxFaceTracker project, and finally switch out the source for the example you're interested in.

Keep in mind that openFrameworks, ofxCv, and ofxFaceTracker are constantly evolving. To compile these examples you'll need versions of each that were released before or at the same time that the commit was made, as newer versions may be incompatible.