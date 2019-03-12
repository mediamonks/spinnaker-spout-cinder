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

#include "CameraParam.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;


// Although logging events are just as flexible and extensible as other events, 
// they are generally only used for logging purposes, which is why a number of 
// helpful functions that provide logging information have been added. Generally,
// if the purpose is not logging, one of the other event types is probably more
// appropriate.
class LoggingEventHandler : public LoggingEvent
{
	// This function displays readily available logging information.
	void OnLogEvent(LoggingEventDataPtr loggingEventDataPtr)
	{
		console() << "Camera Log (" << loggingEventDataPtr->GetTimestamp() << ")";
		console() << " " << loggingEventDataPtr->GetCategoryName() << " | " << loggingEventDataPtr->GetPriorityName() << ": ";
		console() << loggingEventDataPtr->GetLogMessage() << endl;
	}
};

class SpinnakerSpoutApp : public App {
	// -------- SPINNAKER --------
	SystemPtr system = NULL;
	CameraPtr camera = NULL;

	bool checkCameraInitialized();
	bool checkStreamingStarted();
	bool checkStreamingStopped();

	gl::TextureRef cameraTexture = NULL;
	void updateCameraTexture(string &status);

	// -------- CINDER PARAM UI --------
	params::InterfaceGlRef	paramGUI = NULL;
	void initParamInterface();
	bool cameraSettingsDirty = true;
	bool applyCameraSettings();
	void updateMovingCameraParams();

	// -------- PARAMS --------
	int binning = 0; // 0 is no binning (1x scale), 1 = factor 2 binning (0.5x scale)
	double exposure = 10000; // microseconds
	double balanceRatio = 1; // 
	int deviceLinkThroughputLimit = 10000000; // max bandwidth used by this camera in bytes/second
	int sendWidth = 640;
	int sendHeight = 480;

	// -------- SPOUT --------
	SpoutSender spoutSender;					// Create a Spout sender object
	string senderName = "Point Grey";
	gl::FboRef sendFbo;
	bool checkSpoutInitialized();
	void sendToSpout(gl::TextureRef &sendTexture);

	// -------- MISC --------
	bool needsInitText = true;

	int droppedFrames = 0;
	float lastDroppedFramesTime = 0;
	int geLatestDroppedFrames();

	LoggingEventHandler loggingEventHandler;
	int logLevelIndex = 4; // 4 = LOG_LEVEL_ERROR

public:
	void setup() override;
	void draw() override;
	void cleanup() override;
};

SpinnakerLogLevel indexToSpinnakerLogLevel(int index);