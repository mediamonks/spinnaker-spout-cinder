#include "CameraParam.h"
#include "UserSettings.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

vector<CameraParam*> CameraParam::params;
bool CameraParam::cameraSettingsDirty = true;

#define POLL_INTERVAL 1 // seconds

void CameraParam::createEnum(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamEnum(uiText, spinnakerName, paramGUI, camera, defaultValue, needsCameraStop, poll));
	cameraSettingsDirty = true;
}

void CameraParam::createFloat(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, double defaultValue, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamFloat(uiText, spinnakerName, paramGUI, camera, defaultValue, needsCameraStop, poll));
	cameraSettingsDirty = true;
}

void CameraParam::createInt(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamInt(uiText, spinnakerName, paramGUI, camera, defaultValue, needsCameraStop, poll));
	cameraSettingsDirty = true;
}

CameraParam::CameraParam(string _spinnakerName, CameraPtr _camera, bool _needsCameraStop, bool _poll) {
	spinnakerName = _spinnakerName;
	camera = _camera;
	needsCameraStop = _needsCameraStop;
	poll = _poll;
}

// returns true if camera stopped
bool CameraParam::applyParams() {
	if (cameraSettingsDirty == false) return false;
	cameraSettingsDirty = false;

	bool cameraStopped = false;
	for (auto param : params) {
		cameraStopped = cameraStopped || param->applyParam();
	}
	return cameraStopped;
}

float lastPollingTime = 0;
void CameraParam::pollParamsFromCamera() {
	if (getElapsedSeconds() - lastPollingTime < POLL_INTERVAL) return;
	lastPollingTime = getElapsedSeconds();
	for (auto param : params) {
		if (param->poll) param->updateFromCamera();
	}
}

/***** Enum *****/

CameraParamEnum::CameraParamEnum(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, bool _needsCameraStop, bool _poll) : CameraParam (_spinnakerName, _camera, _needsCameraStop, _poll) {
	enumIndex = UserSettings::getSetting<int>(spinnakerName, defaultValue); // enum, but index stored as int

	paramGUI->addParam(uiText, SpinnakerDeviceCommunication::getParameterEnumOptions(camera, spinnakerName), &enumIndex).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<int>(spinnakerName, enumIndex);
	});
}

bool CameraParamEnum::applyParam() {
	bool cameraStopped = false;
	vector<string> options = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, spinnakerName);
	if (SpinnakerDeviceCommunication::getParameterEnumValue(camera, spinnakerName) != options[enumIndex]) {
		if (needsCameraStop) cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		string newChoice = SpinnakerDeviceCommunication::setParameterEnum(camera, spinnakerName, options[enumIndex]);
		if (newChoice != "error") {
			int newIndex = find(options.begin(), options.end(), newChoice) - options.begin();
			if (newIndex != enumIndex) {
				enumIndex = newIndex;
				UserSettings::writeSetting<int>(spinnakerName, enumIndex);
			}
		}
		else {
			console() << "Error writing param value " << options[enumIndex] << " to enum " << spinnakerName << endl;
		}
	}
	return cameraStopped;
}

void CameraParamEnum::updateFromCamera() {
	// TODO: only update the ones that we want updated, continuously auto-adjusted ones like exposure

	vector<string> options = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, spinnakerName);
	string newChoice = SpinnakerDeviceCommunication::getParameterEnumValue(camera, spinnakerName);
	int newIndex = find(options.begin(), options.end(), newChoice) - options.begin();
	if (newIndex != enumIndex) {
		enumIndex = newIndex;
		UserSettings::writeSetting<int>(spinnakerName, enumIndex);
	}
}

/***** Float *****/

CameraParamFloat::CameraParamFloat(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, double defaultValue, bool _needsCameraStop, bool _poll) : CameraParam(_spinnakerName, _camera, _needsCameraStop, _poll) {
	value = UserSettings::getSetting<double>(spinnakerName, defaultValue);
	range = SpinnakerDeviceCommunication::getFloatParameterMinMax(camera, spinnakerName);

	paramGUI->addParam(uiText, &value).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<double>(spinnakerName, value);
	}).min(range.first).max(range.second);
}

bool CameraParamFloat::applyParam() {
	bool cameraStopped = false;
	if (SpinnakerDeviceCommunication::getParameterFloatValue(camera, spinnakerName) != value) {
		if (needsCameraStop) cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		value = SpinnakerDeviceCommunication::setParameterFloat(camera, spinnakerName, value);
	}
	return cameraStopped;
}

void CameraParamFloat::updateFromCamera() {
	double newValue = SpinnakerDeviceCommunication::getParameterFloatValue(camera, spinnakerName);
	if (newValue != value) {
		value = newValue;
		UserSettings::writeSetting<double>(spinnakerName, value);
	}
}

/***** Int *****/

CameraParamInt::CameraParamInt(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, bool _needsCameraStop, bool _poll) : CameraParam(_spinnakerName, _camera, _needsCameraStop, _poll) {
	value = UserSettings::getSetting<int>(spinnakerName, defaultValue);
	range = SpinnakerDeviceCommunication::getIntParameterMinMax(camera, spinnakerName);

	paramGUI->addParam(uiText, &value).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<int>(spinnakerName, value);
	}).min(range.first).max(range.second);
}

bool CameraParamInt::applyParam() {
	bool cameraStopped = false;

	if (SpinnakerDeviceCommunication::getParameterIntValue(camera, spinnakerName) != value) {
		if (needsCameraStop) cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);

		int newValue = SpinnakerDeviceCommunication::setParameterInt(camera, spinnakerName, value);
		if (newValue != value) {
			value = newValue;
			UserSettings::writeSetting<int>(spinnakerName, value);
		}
	}
	return cameraStopped;
}

void CameraParamInt::updateFromCamera() {
	int newValue = SpinnakerDeviceCommunication::getParameterIntValue(camera, spinnakerName);
	if (newValue != value) {
		value = newValue;
		UserSettings::writeSetting<int>(spinnakerName, value);
	}
}