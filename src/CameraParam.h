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
	CameraParam(string spinnakerName, CameraPtr camera, bool needsCameraStop);

	static void createEnum(std::string uiText, std::string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, bool needsCameraStop = false);
	static void createFloat(std::string uiText, std::string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, double defaultValue, bool needsCameraStop = false);
	static void createInt(std::string uiText, std::string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, bool needsCameraStop = false);

	static bool applyParams();
	static void updateParamsFromCamera();
	
protected:
	static bool cameraSettingsDirty;

	virtual bool applyParam() = 0;
	virtual void updateFromCamera() = 0;
	CameraPtr camera;
	string spinnakerName;
	bool needsCameraStop;

private:
	static vector<CameraParam*> params;
};

class CameraParamEnum : public CameraParam
{
public:
	CameraParamEnum(std::string uiText, std::string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, bool needsCameraStop = false);

private:
	bool applyParam() override;
	void updateFromCamera() override;
	int enumIndex = 0;
};

class CameraParamFloat: public CameraParam
{
public:
	CameraParamFloat(std::string uiText, std::string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, double defaultValue, bool needsCameraStop = false);

private:
	bool applyParam() override;
	void updateFromCamera() override;
	double value = 0;
	pair<double, double> range;

};

class CameraParamInt: public CameraParam
{
public:
	CameraParamInt(std::string uiText, std::string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, bool needsCameraStop = false);

private:
	bool applyParam() override;
	void updateFromCamera() override;
	int value = 0;
	pair<int, int> range;
};