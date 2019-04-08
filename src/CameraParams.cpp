#include "CameraParams.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define POLL_INTERVAL 5 // seconds

bool CameraParams::pollingEnabled = true;

void CameraParams::createEnum(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, int cameraIndex, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamEnum(uiText, spinnakerName, paramGUI, camera, defaultValue, cameraIndex, needsCameraStop, poll));
}

void CameraParams::createFloat(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, double defaultValue, int cameraIndex, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamFloat(uiText, spinnakerName, paramGUI, camera, defaultValue, cameraIndex, needsCameraStop, poll));
}

void CameraParams::createInt(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, int cameraIndex, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamInt(uiText, spinnakerName, paramGUI, camera, defaultValue, cameraIndex, needsCameraStop, poll));
}

// returns true if camera stopped
bool CameraParams::applyParams() {
	bool cameraStopped = false;
	for (auto param : params) {
		if (!param->dirty) continue;
		param->dirty = false;
		cameraStopped = cameraStopped || param->applyParam();
	}
	return cameraStopped;
}

void CameraParams::pollParamsFromCamera(bool forceUpdateAll) {
	if (!pollingEnabled && !forceUpdateAll) return;

	if (!forceUpdateAll && getElapsedSeconds() - lastPollingTime < POLL_INTERVAL) return;
	lastPollingTime = getElapsedSeconds();

	for (auto param : params) {
		if (param->poll || forceUpdateAll) {
			param->updateFromCamera();
		}
	}

	forceUpdateAll = false;
}