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
	// -------- CINDER PARAM UI --------
	params::InterfaceGlRef paramGUI = NULL;
	void initParamInterface();

	// -------- SPINNAKER --------
	SystemPtr system = NULL;
	CameraPtr camera = NULL;

	bool checkCameraInitialized();

	gl::TextureRef cameraTexture = NULL;
	void updateCameraTexture(string &status);

	// -------- SPOUT --------
	SpoutSender spoutSender;					// Create a Spout sender object
	string senderName = "Point Grey";
	int sendWidth = 640;
	int sendHeight = 480;
	gl::FboRef sendFbo;
	bool checkSpoutInitialized();
	void sendToSpout(gl::TextureRef &sendTexture);

	// -------- MISC --------
	bool needsInitText = true;

	void drawInfoBoxes(string status, int fps);
	int droppedFrames = 0;
	int getLatestDroppedFrames();

	LoggingEventHandler loggingEventHandler;
	int logLevelIndex = 4; // 4 = LOG_LEVEL_ERROR

public:
	void setup() override;
	void draw() override;
	void cleanup() override;
};

SpinnakerLogLevel indexToSpinnakerLogLevel(int index);