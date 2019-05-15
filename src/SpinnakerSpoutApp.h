#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "SpinnakerDeviceCommunication.h"

#include "UserSettings.h"

#include "CameraParam.h"
#include "SpinnakerCamera.h"
#include "SpoutPool.h"

#include "Log.h"

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
		Log() << "Camera Log (" << loggingEventDataPtr->GetTimestamp() << ")";
		Log() << " " << loggingEventDataPtr->GetCategoryName() << " | " << loggingEventDataPtr->GetPriorityName() << ": ";
		Log() << loggingEventDataPtr->GetLogMessage();
	}
};

class SpinnakerSpoutApp : public App {
	// -------- SPINNAKER --------
	SystemPtr system = NULL;
	vector<SpinnakerCamera*> cameras;
	void initSystem();
	void initCameras(params::InterfaceGlRef paramsGUI);
	int visibleCamera = 0;

	params::InterfaceGlRef paramGUI = NULL;

	// -------- SPOUT --------
	SpoutPool spoutPool;
	int sendWidth = 640;
	int sendHeight = 480;

	// -------- MISC --------
	bool needsInitText = true;

	void drawInfoBoxes(string status, int fps);

	LoggingEventHandler loggingEventHandler;
	int logLevelIndex = 4; // 4 = LOG_LEVEL_ERROR

public:
	void setup() override;
	void draw() override;
	void cleanup() override;
	void keyDown(KeyEvent e);
};