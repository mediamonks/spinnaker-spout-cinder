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
	static void createEnum(std::string uiText, std::string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, bool needsCameraStop = false);
	static bool applyParams();

	virtual bool applyParam() = 0;

protected:
	static bool cameraSettingsDirty;

	CameraPtr camera;
	string spinnakerName;
	bool needsCameraStop;

private:
	static vector<CameraParam*> params;

};

class CameraParamEnum : public CameraParam
{
public:
	bool applyParam() override;

	CameraParamEnum(std::string uiText, std::string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, bool needsCameraStop = false);
	~CameraParamEnum();

private:
	int enumIndex = 0;
};

