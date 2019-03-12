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

void CameraParam::createEnum(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, bool needsCameraStop) {
	params.push_back(new CameraParamEnum(uiText, spinnakerName, paramGUI, camera, defaultValue, needsCameraStop));
}

// returns true if camera stopped
bool CameraParam::applyParams() {
	if (!cameraSettingsDirty) return false;

	bool cameraStopped = false;
	for (auto param : params) {
		cameraStopped = cameraStopped || param->applyParam();
	}
	return cameraStopped;
}

CameraParamEnum::CameraParamEnum(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, bool _needsCameraStop)
{
	spinnakerName = _spinnakerName;
	camera = _camera;
	needsCameraStop = _needsCameraStop;

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
			//console() << "Error writing param value " << options[param->enumIndex] << " to enum " << param->spinnakerName << endl;
		}
	}
	return cameraStopped;
}