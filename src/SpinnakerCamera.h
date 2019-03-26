#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

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
	gl::TextureRef getCameraTexture();

	int getLatestDroppedFrames();
	string getSerialNumber();
	void cleanup();

private:
	SystemPtr system;
	int cameraIndex = -1;
	params::InterfaceGlRef paramGUI;

	CameraPtr camera = NULL;
	gl::TextureRef cameraTexture = NULL;
	double lastDroppedFramesTime = 0;
	int droppedFrames = 0;

	CameraParams cameraParams;

	bool checkCameraAssigned();

	bool checkCameraInitialized();
	bool cameraInitialized = false;
	double lastCameraInitFailTime = -100;

	void checkParamInterfaceInitialized();
	bool paramInterfaceInitialized = false;

	bool checkCameraStarted();
	bool cameraStarted = false;
	
	int prevCaptureWidth = 0;
	int prevCaptureHeight = 0;
};