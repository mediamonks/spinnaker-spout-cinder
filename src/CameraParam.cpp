#include "CameraParam.h"
#include "UserSettings.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

CameraPtr CameraParam::camera;
vector<CameraParam*> CameraParam::params;
bool CameraParam::cameraSettingsDirty = true;

void CameraParam::create(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, bool needsCameraStop) {
	params.push_back(new CameraParam(uiText, spinnakerName, paramGUI, camera, needsCameraStop));
}

CameraParam::CameraParam(string uiText, string _spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr _camera, int defaultValue, bool _needsCameraStop)
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

// returns true if camera stopped
bool CameraParam::applyParams() {
	if (!cameraSettingsDirty) return false;

	bool cameraStopped = false;
	for (auto param : params) {
		vector<string> options = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, param->spinnakerName);
		if (SpinnakerDeviceCommunication::getParameterEnumValue(camera, param->spinnakerName) != options[param->enumIndex]) {
			if (param->needsCameraStop) cameraStopped = SpinnakerDeviceCommunication::checkStreamingStopped(camera);
			string newChoice = SpinnakerDeviceCommunication::setParameterEnum(camera, param->spinnakerName, options[param->enumIndex]);
			if (newChoice != "error") {
				int newIndex = find(options.begin(), options.end(), newChoice) - options.begin();
				if (newIndex != param->enumIndex) {
					param->enumIndex = newIndex;
					UserSettings::writeSetting<int>(param->spinnakerName, param->enumIndex);
				}
			}
			else {
				//console() << "Error writing param value " << options[param->enumIndex] << " to enum " << param->spinnakerName << endl;
			}
		}
	}
	return cameraStopped;
}

CameraParam::~CameraParam()
{
}
