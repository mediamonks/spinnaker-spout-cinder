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

	bool applyParams();
	void pollParamsFromCamera();

private:
	vector<CameraParam*> params;
	double lastPollingTime = 0;
};