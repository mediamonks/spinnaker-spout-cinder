#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/Thread.h"
#include "cinder/ConcurrentCircularBuffer.h"

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "CameraParams.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

class SpinnakerCamera {
public:
	SpinnakerCamera(SystemPtr system, int index, params::InterfaceGlRef paramGUI);
	~SpinnakerCamera();

	gl::TextureRef getLatestCameraTexture();
	float getFps();

	int getLatestDroppedFrames();
	string getSerialNumber();
	void cleanup();

	void printInfo();

private:
	SystemPtr system;
	int cameraIndex = -1;
	params::InterfaceGlRef paramGUI;

	CameraPtr camera = NULL;
	gl::TextureRef cameraTexture = NULL;
	double lastDroppedFramesTime = 0;
	atomic<int> droppedFrames = 0;

	double prevFrameTime = 0;
	atomic<float> fps = 0;

	CameraParams cameraParams;

	bool checkCameraAssigned();

	bool checkCameraInitialized();
	bool cameraInitialized = false;
	double lastCameraInitFailTime = -100;

	void checkParamInterfaceInitialized();
	bool paramInterfaceInitialized = false;

	bool checkCameraStreaming();
	bool checkCameraStopped();
	bool cameraStarted = false; // cache of camera streaming state
	
	int prevCaptureWidth = 0;
	int prevCaptureHeight = 0;

	shared_ptr<thread> captureThread;
	ConcurrentCircularBuffer<gl::TextureRef> *frameBuffer;
	void captureThreadFn(gl::ContextRef sharedGlContext);
	bool shouldQuit = false;

	gl::TextureRef latestTexture = NULL;

	bool checkCameraUpdatedAndRunning();
	gl::TextureRef getNextCameraTexture(); // also makes user camera is initialized. blocks during camera initialization and until next texture is received

	void save();
};