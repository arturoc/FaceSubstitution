/*
 * ofxGstVideoRecorder.h
 *
 *  Created on: 14/01/2010
 *      Author: arturo
 */

#ifndef OFXGSTVIDEORECORDER_H_
#define OFXGSTVIDEORECORDER_H_

//#include "ofMain.h"
#include <gst/app/gstappsrc.h>
#include "ofGstUtils.h"

class ofxGstBufferPool;


class ofxGstVideoRecorder: public ofGstUtils, public ofGstAppSink {
public:
	ofxGstVideoRecorder();
	virtual ~ofxGstVideoRecorder();

	enum Codec{
		THEORA,
		H264,
		MP4,
		XVID,
		JPEG,
		PNG_SEQUENCE,
		JPEG_SEQUENCE,
		TIFF_SEQUENCE,
		FLV,
		FLV_H264,
		YUV,
		Y4M
	};

	void udpStreamTo(string comma_separated_ips);
	void tcpStreamTo(string host, int port);
	void pipeTo(string command);

	void setUseAudio(bool useAudio,string device="");
	void setup(int width, int height, int bpp, string file, Codec codec, int fps=30);

	void shutdown();
	bool isClosed();

	void newFrame(ofPixels & pixels);


protected:
	void			on_eos();
	
	GstAppSrc * gstSrc;
	string src;
	string sink;
	bool bIsUdpStream, bIsTcpStream;
	Codec codec;
	ofxGstBufferPool * bufferPool;
	bool closed;
	string audioDevice;

};

#endif /* OFXGSTVIDEORECORDER_H_ */
