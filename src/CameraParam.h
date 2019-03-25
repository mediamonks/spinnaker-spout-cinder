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
	CameraParam(string spinnakerName, CameraPtr camera, bool needsCameraStop, bool poll, int cameraIndex);
	virtual bool applyParam() = 0;
	virtual void updateFromCamera() = 0;
	bool needsCameraStop = false;
	bool poll = false;
	string getName() {
		return settingName;
	}

protected:
	static string getCameraName(int cameraIndex, CameraPtr camera);

	CameraPtr camera;
	string spinnakerName;
	string settingName;
	int cameraIndex = -1;
};

class CameraParamEnum : public CameraParam
{
public:
	CameraParamEnum(std::string uiText, std::string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, int cameraIndex, bool needsCameraStop, bool poll, bool* dirty);
	bool applyParam() override;
	void updateFromCamera() override;

private:
	int enumIndex = 0;
};

class CameraParamFloat: public CameraParam
{
public:
	CameraParamFloat(std::string uiText, std::string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, double defaultValue, int cameraIndex, bool needsCameraStop, bool poll, bool* dirty);
	bool applyParam() override;
	void updateFromCamera() override;

private:
	double value = 0;
	pair<double, double> range;

};

class CameraParamInt: public CameraParam
{
public:
	CameraParamInt(std::string uiText, std::string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, int cameraIndex, bool needsCameraStop, bool poll, bool* dirty);
	bool applyParam() override;
	void updateFromCamera() override;

private:
	int value = 0;
	pair<int, int> range;
};