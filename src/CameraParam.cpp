#include "CameraParam.h"
#include "UserSettings.h"

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
			console() << "Error: camera not initialized when creating parameter" << endl;
		}
	}

	camera = _camera;
	needsCameraStop = _needsCameraStop;
	poll = _poll;
	cameraIndex = _cameraIndex;
}

string CameraParam::getCameraName(int cameraIndex, CameraPtr camera) {
	stringstream ss;
	ss << "Camera " << to_string(cameraIndex) << " (" << camera->DeviceModelName() << ")";
	return ss.str();
}

/***** Enum *****/

CameraParamEnum::CameraParamEnum(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, int _cameraIndex, bool _needsCameraStop, bool _poll) : CameraParam (_spinnakerName, _camera, _needsCameraStop, _poll, _cameraIndex) {
	enumIndex = UserSettings::getSetting<int>(settingName, defaultValue); // enum, but index stored as int

	if (cameraIndex != -1) uiText = uiText + " " + to_string(cameraIndex);

	auto options = paramGUI->addParam(uiText, SpinnakerDeviceCommunication::getParameterEnumOptions(camera, spinnakerName), &enumIndex).updateFn([this] {
		dirty = true;
		UserSettings::writeSetting<int>(settingName, enumIndex);
	});

	if (cameraIndex != -1) options.group(getCameraName(cameraIndex, camera));
}

bool CameraParamEnum::applyParam() {
	auto options = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, spinnakerName);
	if (enumIndex > (int)options.size() - 1) { // cast from long to int is necessary, otherwise we get a crash comparing
		console() << "Error applying param value " << enumIndex << " to enum " << spinnakerName << ". There are only " << options.size() << " options." << endl;
		return false;
	}

	bool cameraStopped = false;
	if (SpinnakerDeviceCommunication::getParameterEnumValue(camera, spinnakerName) != options[enumIndex]) {
		if (needsCameraStop) {
			console() << spinnakerName << " needs camera to be stopped." << endl;
			cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		}

		console() << "Setting " << spinnakerName << " to " << options[enumIndex] << endl;

		string newChoice = SpinnakerDeviceCommunication::setParameterEnum(camera, spinnakerName, options[enumIndex]);
		if (newChoice != "error") {
			int newIndex = find(options.begin(), options.end(), newChoice) - options.begin();
			if (!poll && newIndex != enumIndex) {
				enumIndex = newIndex;
				UserSettings::writeSetting<int>(settingName, enumIndex);
			}
		}
		else {
			console() << "Error writing param value " << options[enumIndex] << " to enum " << spinnakerName << endl;
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
		UserSettings::writeSetting<int>(settingName, enumIndex);
	}
}

/***** Float *****/

CameraParamFloat::CameraParamFloat(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, double defaultValue, int _cameraIndex, bool _needsCameraStop, bool _poll) : CameraParam(_spinnakerName, _camera, _needsCameraStop, _poll, _cameraIndex) {
	value = UserSettings::getSetting<double>(settingName, defaultValue);
	range = SpinnakerDeviceCommunication::getFloatParameterMinMax(camera, spinnakerName);

	if (cameraIndex != -1) uiText = uiText + " " + to_string(cameraIndex);
	
	auto options = paramGUI->addParam(uiText, &value).updateFn([this] {
		dirty = true;
		UserSettings::writeSetting<double>(settingName, value);
	}).min((float)range.first).max((float)range.second);

	if (cameraIndex != -1) options.group(getCameraName(cameraIndex, camera));
}

bool CameraParamFloat::applyParam() {
	bool cameraStopped = false;
	if (SpinnakerDeviceCommunication::getParameterFloatValue(camera, spinnakerName) != value) {
		if (needsCameraStop) {
			console() << spinnakerName << " needs camera to be stopped." << endl;
			cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		}

		console() << "Setting " << spinnakerName << " to " << value << endl;

		float newValue = SpinnakerDeviceCommunication::setParameterFloat(camera, spinnakerName, value);
		if (!poll && newValue != -1 && newValue != value) {
			value = newValue;
			UserSettings::writeSetting<double>(settingName, value);
		}
	}
	return cameraStopped;
}

// called repetitively if poll is true
void CameraParamFloat::updateFromCamera() {
	double newValue = SpinnakerDeviceCommunication::getParameterFloatValue(camera, spinnakerName);
	if (newValue != -1 && newValue != value) {
		value = newValue;
		UserSettings::writeSetting<double>(settingName, value);
	}
}

/***** Int *****/

CameraParamInt::CameraParamInt(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, int _cameraIndex, bool _needsCameraStop, bool _poll) : CameraParam(_spinnakerName, _camera, _needsCameraStop, _poll, _cameraIndex) {
	value = UserSettings::getSetting<int>(settingName, defaultValue);
	range = SpinnakerDeviceCommunication::getIntParameterMinMax(camera, spinnakerName);

	if (cameraIndex != -1) uiText = uiText + " " + to_string(cameraIndex);

	auto options = paramGUI->addParam(uiText, &value).updateFn([this] {
		dirty = true;
		UserSettings::writeSetting<int>(settingName, value);
	}).min((float)range.first).max((float)range.second);

	if (cameraIndex != -1) options.group(getCameraName(cameraIndex, camera));
}

bool CameraParamInt::applyParam() {
	bool cameraStopped = false;

	if (SpinnakerDeviceCommunication::getParameterIntValue(camera, spinnakerName) != value) {
		if (needsCameraStop) {
			console() << spinnakerName << " needs camera to be stopped." << endl;
			cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		}

		int newValue = SpinnakerDeviceCommunication::setParameterInt(camera, spinnakerName, value);
		if (!poll && newValue != -1 && newValue != value) {
			value = newValue;
			UserSettings::writeSetting<int>(settingName, value);
		}
	}
	return cameraStopped;
}

// called repetitively if poll is true
void CameraParamInt::updateFromCamera() {
	int newValue = SpinnakerDeviceCommunication::getParameterIntValue(camera, spinnakerName);
	if (newValue != -1 && newValue != value) {
		value = newValue;
		UserSettings::writeSetting<int>(settingName, value);
	}
}