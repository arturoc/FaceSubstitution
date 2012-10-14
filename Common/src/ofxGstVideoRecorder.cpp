/*
 * ofxGstVideoRecorder.cpp
 *
 *  Created on: 14/01/2010
 *      Author: arturo
 */

#include "ofxGstVideoRecorder.h"
#include <list>
#include <map>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>
#include "ofAppRunner.h"


class PooledPixels: public ofPixels{
public:
	ofxGstBufferPool * pool;
};

class ofxGstBufferPool{
public:
	ofxGstBufferPool(int width, int height, int channels)
	:width(width), height(height), channels(channels){};
	PooledPixels * newBuffer(){
		PooledPixels * pixels;
		mutex.lock();
		if(pool.empty()){
			mutex.unlock();
			pixels = new PooledPixels;
			pixels->allocate(width,height,channels);
			pixels->pool = this;
		}else{
			pixels = pool.back();
			pool.pop_back();
			mutex.unlock();
		}
		return pixels;
	}

	void releaseBuffer(PooledPixels * pixels){
		mutex.lock();
		pool.push_back(pixels);
		mutex.unlock();
	}

private:
	list<PooledPixels*> pool;
	int width, height, channels;
	ofMutex mutex;
};

static void ofxGstRecorderRelaseBuffer(void * buffer){
	PooledPixels* pixels = (PooledPixels*) buffer;
	pixels->pool->releaseBuffer(pixels);
}

ofxGstVideoRecorder::ofxGstVideoRecorder() {
	bufferPool = NULL;
	bIsTcpStream = false;
	bIsUdpStream = false;
	gstSrc = NULL;
	closed = true;
}

void ofxGstVideoRecorder::shutdown()
{
    if(gstSrc){
	    //gst_app_src_end_of_stream (gstSrc);
		gst_element_send_event(getPipeline(), gst_event_new_eos());
		gst_object_unref(gstSrc);
		gstSrc = NULL;
	}else{
		if(!closed) close();
		closed = true;
	}
}

void ofxGstVideoRecorder::on_eos(){
	ofLogVerbose() << "got eos message, closing pipeline" << endl;
	if(!closed) close();
	closed = true;
}

ofxGstVideoRecorder::~ofxGstVideoRecorder() {
	setSinkListener(NULL);
	shutdown();
	if(bufferPool) delete bufferPool;
}

void ofxGstVideoRecorder::udpStreamTo(string comma_separated_ips){
	sink  = " multiudpsink clients=" + comma_separated_ips;
	bIsUdpStream = true;
}

void ofxGstVideoRecorder::tcpStreamTo(string host, int port){
	sink = " tcpserversink host="+host+" port="+ofToString(port);
	bIsTcpStream = true;
}

void ofxGstVideoRecorder::setup(int width, int height, int bpp, string file, Codec _codec, int fps){
	shutdown();
	ofGstUtils::startGstMainLoop();

	bufferPool = new ofxGstBufferPool(width,height,bpp/8);

	file = ofToDataPath(file);
	
	codec = _codec;

	//gst_debug_set_active (true);

	if(!bIsTcpStream && !bIsUdpStream)
		sink= "filesink name=video-sink sync=false location=" + file;

	string encoder;
	string muxer = "avimux ! ";
	string colorspaceconversion = " ffmpegcolorspace ! ";

	if(ofFilePath(file).getExtension()=="avi") muxer = "avimux ! ";
	else if(ofFilePath(file).getExtension()=="mp4") muxer = "mp4mux ! ";
	else if(ofFilePath(file).getExtension()=="mov") muxer = "qtmux ! ";
	else if(ofFilePath(file).getExtension()=="mkv") muxer = "matroskamux ! ";
	else if(ofFilePath(file).getExtension()=="ogg" || ofFilePath(file).getExtension()=="ogv") muxer = " oggmux ! ";

	stringstream encoderformat;
	string pay = "";
	string videorate = " videorate force-fps=" +ofToString(fps)+"/1  ! ";

	switch(codec){
	case THEORA:
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
		encoder = "theoraenc quality=63 ! ";
		muxer = "oggmux ! ";
		pay = "rtptheorapay pt=96 ! ";
	break;
	case H264:
#ifdef TARGET_OSX
		encoderformat << " video/x-raw-yuv, format=(fourcc)NV12, width=" << width << ", height=" << height;
		encoder = "vtenc_h264 ! ";
#else
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
		encoder = "x264enc bitrate=1024 ! ";
#endif
		pay = "rtph264pay pt=96 ! ";
	break;
	case MP4:
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
		encoder = "ffenc_mpeg4 ! ";
		pay = "rtpmp4vpay pt=96 ! ";
	break;
	case XVID:
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
		encoder = "xvidenc ! ";
	break;
		case JPEG:
#ifdef TARGET_OSX
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
#else
		colorspaceconversion = "";
		encoderformat << "";
#endif
		encoder = "jpegenc quality=98 ! ";
		pay = "rtpjpegpay pt=96 !";
		break;
	case JPEG_SEQUENCE:
		colorspaceconversion = "";
		encoderformat << "";
		encoder = "jpegenc quality=100 idct-method=float ! ";
		pay = "";
		sink = "multifilesink name=video-sink location=" + file.substr(0,file.rfind('.')) + "%05d" + file.substr(file.rfind('.'));
		videorate = "";
		muxer = "";
	break;
	case PNG_SEQUENCE:
		colorspaceconversion = "";
		encoderformat << "";
		encoder = "pngenc snapshot=false ! ";
		sink = "multifilesink name=video-sink location=" + file.substr(0,file.rfind('.')) + "%05d" + file.substr(file.rfind('.'));
		videorate = "";
		muxer = "";
	break;
	case TIFF_SEQUENCE:
		colorspaceconversion = "";
		encoderformat << "";
		encoder = "ffenc_tiff ! ";
		sink = "multifilesink name=video-sink location=" + file.substr(0,file.rfind('.')) + "%05d" + file.substr(file.rfind('.'));
		videorate = "";
		muxer = "";
	break;
	case FLV:
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
		encoder = "ffenc_flv ! ";
		muxer = "flvmux ! ";
	break;
	case FLV_H264:
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
		encoder = "x264enc ! ";
		muxer = "flvmux ! ";
		pay = "rtph264pay pt=96 ! ";
	break;
	case YUV:
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
		encoder = "";
		muxer = "avimux ! ";
	break;
	case Y4M:
		encoderformat << " video/x-raw-yuv, format=(fourcc)I420, width=" << width << ", height=" << height;
		encoder = "y4menc ! ";
		muxer = "";
		break;
	break;
	}


	if(bIsUdpStream){
		muxer = "";
		if(pay!=""){
			pay = "queue ! " + pay;
			muxer = muxer + pay;
		}else{
			ofLogError() << "this format doesnt support streaming, choose one of THEORA, H264, MP4, JPEG";
			return;
		}
	}else{
		pay   = "";
		if(bIsTcpStream){
			muxer = "mpegtsmux name=mux ! ";
		}
	}

	string input_mime;
	string other_format;
	if(bpp==24 || bpp==32){
		input_mime = "video/x-raw-rgb";
		//if(codec==PNG_SEQUENCE || codec==TIFF_SEQUENCE){
			other_format = ",endianness=4321,red_mask=16711680,green_mask=65280,blue_mask=255";
		/*}else{
			other_format = ",endianness=4321,red_mask=255,green_mask=65280,blue_mask=16711680";
		}*/
	}
	if(bpp==8){
		input_mime = "video/x-raw-gray";
		other_format = "";
	}

	string input_format = input_mime
			+ ", width=" + ofToString(width)
			+ ", height=" + ofToString(height)
			+ ", depth=" + ofToString(bpp)
			+ ", bpp=" + ofToString(bpp)
			+ other_format;

	if(src==""){
		src="appsrc  name=video_src is-live=true do-timestamp=true ! "
				+ input_format + ",framerate="+ofToString(fps)+"/1 ";
	}

	string videorateformat;
	if(videorate!=""){
		videorateformat = (encoderformat.str()!="" ? encoderformat.str() : input_format) + " ! ";
	}
	string encoderformatstr = (encoderformat.str()!="" ? encoderformat.str() : input_format) + ", framerate=" +ofToString(fps) + "/1 ! ";

	string pipeline_string = src + " ! " +
									"queue ! " + colorspaceconversion + videorateformat +
									videorate + encoderformatstr +
									encoder + muxer +
									sink;

	ofLogVerbose() << "gstreamer pipeline: " << pipeline_string;
	
	setSinkListener(this);

	setPipelineWithSink(pipeline_string,"video-sink");

	gstSrc = (GstAppSrc*)gst_bin_get_by_name(GST_BIN(getPipeline()),"video_src");
	if(gstSrc){
		gst_app_src_set_stream_type (gstSrc,GST_APP_STREAM_TYPE_STREAM);
		g_object_set (G_OBJECT(gstSrc), "format", GST_FORMAT_TIME, NULL);
	}else{
		ofLogError() << "got no gstSrc";
	}
	play();

	closed = false;

}

void ofxGstVideoRecorder::newFrame(ofPixels & pixels){
	if(!bufferPool || !gstSrc) return;

	PooledPixels * pooledPixels = bufferPool->newBuffer();
	(*(ofPixels*)pooledPixels) = pixels;
	/*if(codec==PNG_SEQUENCE || codec==TIFF_SEQUENCE){
		pooledPixels->swapRgb();
	}*/
	GstBuffer * buffer;
	buffer = gst_app_buffer_new (pooledPixels->getPixels(), pooledPixels->size(), &ofxGstRecorderRelaseBuffer, pooledPixels);

	GstFlowReturn flow_return = gst_app_src_push_buffer(gstSrc, buffer);
	if (flow_return != GST_FLOW_OK) {
		ofLogError() << "error pushing buffer: flow_return was " << flow_return;
	}
	//ofGstVideoUtils::update();
}


bool ofxGstVideoRecorder::isClosed(){
	return closed;
}
