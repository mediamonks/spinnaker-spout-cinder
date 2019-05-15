#include "CameraParam.h"
#include "Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define POLL_INTERVAL 1 // seconds

CameraParam::CameraParam(string _spinnakerName, CameraPtr _camera, bool _needsCameraStop, bool _poll, int _cameraIndex) {
	spinnakerName = _spinnakerName;
	if (_cameraIndex != -1) {
		if (_camera->IsInitialized()) {
			stringstream ss;
			ss << _spinnakerName << "_" << _camera->DeviceSerialNumber();
			settingName = ss.str();
		}
		else {
			Log() << "Error: camera not initialized when creating parameter";
		}
	}

	camera = _camera;
	needsCameraStop = _needsCameraStop;
	poll = _poll;
	cameraIndex = _cameraIndex;
}

string CameraParam::getCameraName(int cameraIndex, CameraPtr camera) {
	stringstream ss;
	ss << "Cam " << to_string(cameraIndex) << ": " << camera->DeviceSerialNumber() << " (" << camera->DeviceModelName() << ")";
	return ss.str();
}

/***** Enum *****/

CameraParamEnum::CameraParamEnum(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, int _cameraIndex, bool _needsCameraStop, bool _poll) : CameraParam (_spinnakerName, _camera, _needsCameraStop, _poll, _cameraIndex) {
	auto enumOptions = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, spinnakerName);
	string currentValue = SpinnakerDeviceCommunication::getParameterEnumValue(camera, spinnakerName);

	enumIndex = find(enumOptions.begin(), enumOptions.end(), currentValue) - enumOptions.begin();

	if (cameraIndex != -1) uiText = uiText + " " + to_string(cameraIndex);

	auto options = paramGUI->addParam(uiText, enumOptions, &enumIndex).updateFn([this] {
		dirty = true;
	});

	if (cameraIndex != -1) options.group(getCameraName(cameraIndex, camera));
	applyParam();
}

bool CameraParamEnum::applyParam() {
	auto options = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, spinnakerName);
	if (enumIndex > (int)options.size() - 1) { // cast from long to int is necessary, otherwise we get a crash comparing
		Log() << "Error applying param value " << enumIndex << " to enum " << spinnakerName << ". There are only " << options.size() << " options.";
		return false;
	}

	bool cameraStopped = false;
	if (SpinnakerDeviceCommunication::getParameterEnumValue(camera, spinnakerName) != options[enumIndex]) {
		if (needsCameraStop) {
			Log() << spinnakerName << " needs camera to be stopped.";
			cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		}

		//Log() << "Setting " << spinnakerName << " to " << options[enumIndex];

		string newChoice = SpinnakerDeviceCommunication::setParameterEnum(camera, spinnakerName, options[enumIndex]);
		if (newChoice != "error") {
			int newIndex = find(options.begin(), options.end(), newChoice) - options.begin();
			if (!poll && newIndex != enumIndex) {
				enumIndex = newIndex;
			}
		}
		else {
			Log() << "Error writing param value " << options[enumIndex] << " to enum " << spinnakerName;
		}
	}
	return cameraStopped;
}

void CameraParamEnum::updateFromCamera() {
	vector<string> options = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, spinnakerName);
	string newChoice = SpinnakerDeviceCommunication::getParameterEnumValue(camera, spinnakerName);
	int newIndex = find(options.begin(), options.end(), newChoice) - options.begin();
	if (newIndex != enumIndex) {
		enumIndex = newIndex;
	}
}

/***** Float *****/

CameraParamFloat::CameraParamFloat(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, double defaultValue, int _cameraIndex, bool _needsCameraStop, bool _poll) : CameraParam(_spinnakerName, _camera, _needsCameraStop, _poll, _cameraIndex) {
	value = SpinnakerDeviceCommunication::getParameterFloatValue(camera, spinnakerName);

	range = SpinnakerDeviceCommunication::getFloatParameterMinMax(camera, spinnakerName);

	if (cameraIndex != -1) uiText = uiText + " " + to_string(cameraIndex);
	
	auto options = paramGUI->addParam(uiText, &value).updateFn([this] {
		dirty = true;
	}).min((float)range.first).max((float)range.second);

	if (cameraIndex != -1) options.group(getCameraName(cameraIndex, camera));
	applyParam();
}

bool CameraParamFloat::applyParam() {
	bool cameraStopped = false;
	if (SpinnakerDeviceCommunication::getParameterFloatValue(camera, spinnakerName) != value) {
		if (needsCameraStop) {
			Log() << spinnakerName << " needs camera to be stopped.";
			cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		}

		//Log() << "Setting " << spinnakerName << " to " << value;

		float newValue = SpinnakerDeviceCommunication::setParameterFloat(camera, spinnakerName, value);
		if (!poll && newValue != -1 && newValue != value) {
			value = newValue;
		}
	}
	return cameraStopped;
}

// called repetitively if poll is true
void CameraParamFloat::updateFromCamera() {
	double newValue = SpinnakerDeviceCommunication::getParameterFloatValue(camera, spinnakerName);
	if (newValue != -1 && newValue != value) {
		value = newValue;
	}
}

/***** Int *****/

CameraParamInt::CameraParamInt(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, int _cameraIndex, bool _needsCameraStop, bool _poll) : CameraParam(_spinnakerName, _camera, _needsCameraStop, _poll, _cameraIndex) {
	//value = UserSettings::getSetting<int>(settingName, defaultValue);
	value = SpinnakerDeviceCommunication::getParameterIntValue(camera, spinnakerName);

	range = SpinnakerDeviceCommunication::getIntParameterMinMax(camera, spinnakerName);

	if (cameraIndex != -1) uiText = uiText + " " + to_string(cameraIndex);

	auto options = paramGUI->addParam(uiText, &value).updateFn([this] {
		dirty = true;
	}).min((float)range.first).max((float)range.second);

	if (cameraIndex != -1) options.group(getCameraName(cameraIndex, camera));
	applyParam();
}

bool CameraParamInt::applyParam() {
	bool cameraStopped = false;

	if (SpinnakerDeviceCommunication::getParameterIntValue(camera, spinnakerName) != value) {
		if (needsCameraStop) {
			Log() << spinnakerName << " needs camera to be stopped.";
			cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		}

		int newValue = SpinnakerDeviceCommunication::setParameterInt(camera, spinnakerName, value);
		if (!poll && newValue != -1 && newValue != value) {
			value = newValue;
		}
	}
	return cameraStopped;
}

// called repetitively if poll is true
void CameraParamInt::updateFromCamera() {
	int newValue = SpinnakerDeviceCommunication::getParameterIntValue(camera, spinnakerName);
	if (newValue != -1 && newValue != value) {
		value = newValue;
	}
}