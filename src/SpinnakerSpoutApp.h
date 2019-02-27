#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "SpoutLibrary.h"
#include "..\SpoutSDK\Spout.h"

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "SpinnakerDeviceCommunication.h"

#include "UserSettings.h"

class SpinnakerSpoutApp : public App {
	// -------- SPINNAKER --------
	SystemPtr system = NULL;
	CameraPtr camera = NULL;
	bool cameraFound = false;
	bool cameraRunning = false;

	void checkCameraAvailable();
	bool startCamera();
	bool stopCamera();
	float lastCameraStartCheckTime = -100;

	int prevCaptureWidth = 0;
	int prevCaptureHeight = 0;
	gl::TextureRef texture;
	gl::TextureRef getCameraTexture();

	// -------- CINDER PARAM UI --------
	params::InterfaceGlRef	params = NULL;
	int binning = 0; // 0 is no binning (1x scale), 1 = factor 2 binning (0.5x scale)
	string gainAuto = "Off"; // 0 is Off, 1 = Once, 2 = Continuous
	int gainAutoEnumIndex = 0;
	string exposureAuto = "Off"; // 0 is Off, 1 = Once, 2 = Continuous
	int exposureAutoEnumIndex = 0;
	string pixelFormat = "BayerRG8"; //Color mode of camera aquisition. Will be converted to RGB8 software-side, note that capturing at RGB8Packed would use a lot of bandwidth.
	int pixelFormatEnumIndex = 0;
	void initParamInterface();
	bool applyParamsIfNeeded();
	bool paramInterfaceInited = false;

	// -------- SPOUT --------
	SpoutSender spoutSender;					// Create a Spout sender object
	bool spoutInitialized;						// true if a sender initializes OK
	int sendWidth = 640;
	int sendHeight = 480;
	string senderName = "Point Grey";
	gl::FboRef sendFbo;
	void initSpout();

	// -------- MISC --------
	bool needsInitText = true;

	int droppedFrames = 0;
	float lastDroppedFramesTime = 0;
	int geLatestDroppedFrames();

public:
	void setup() override;
	void draw() override;
	void cleanup() override;
};