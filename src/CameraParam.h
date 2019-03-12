#pragma once

#include "cinder/app/App.h"
#include "cinder/params/Params.h"

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "SpinnakerDeviceCommunication.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

class CameraParam
{
public:
	static void create(std::string uiText, std::string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, bool needsCameraStop = false);
	static bool applyParams();

	CameraParam(std::string uiText, std::string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, bool needsCameraStop = false);
	~CameraParam();

private:
	static vector<CameraParam*> params;
	static bool cameraSettingsDirty;

	static CameraPtr camera;
	int enumIndex = 0;
	string spinnakerName;
	bool needsCameraStop;
};

