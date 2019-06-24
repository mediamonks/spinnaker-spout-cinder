#pragma once

#include "cinder/app/App.h"

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "CameraParam.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

class CameraParams {
public:
	static bool pollingEnabled;

	void createEnum(std::string uiText, std::string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, int cameraIndex = -1, bool needsCameraStop = false, bool poll = false);
	void createFloat(std::string uiText, std::string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, double defaultValue, int cameraIndex = -1, bool needsCameraStop = false, bool poll = false);
	void createInt(std::string uiText, std::string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, int cameraIndex = -1, bool needsCameraStop = false, bool poll = false);

	pair<bool, bool> applyParams(); // returns pair: 1) was new parameter value applied?, 2) was camera stopped in the process?
	void pollParamsFromCamera(bool forceUpdateAll = false);

private:
	vector<CameraParam*> params;
	double lastPollingTime = 0;
};