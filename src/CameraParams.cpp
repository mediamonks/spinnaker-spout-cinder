#include "CameraParams.h"
#include "UserSettings.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define POLL_INTERVAL 5 // seconds

bool CameraParams::pollingEnabled = false;

void CameraParams::createEnum(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, int cameraIndex, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamEnum(uiText, spinnakerName, paramGUI, camera, defaultValue, cameraIndex, needsCameraStop, poll, &dirty));
	dirty = true;
}

void CameraParams::createFloat(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, double defaultValue, int cameraIndex, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamFloat(uiText, spinnakerName, paramGUI, camera, defaultValue, cameraIndex, needsCameraStop, poll, &dirty));
	dirty = true;
}

void CameraParams::createInt(string uiText, string spinnakerName, params::InterfaceGlRef paramGUI, CameraPtr camera, int defaultValue, int cameraIndex, bool needsCameraStop, bool poll) {
	params.push_back(new CameraParamInt(uiText, spinnakerName, paramGUI, camera, defaultValue, cameraIndex, needsCameraStop, poll, &dirty));
	dirty = true;
}

// returns true if camera stopped
bool CameraParams::applyParams() {
	if (dirty == false) return false;
	dirty = false;

	bool cameraStopped = false;
	for (auto param : params) {
		cameraStopped = cameraStopped || param->applyParam();
	}
	return cameraStopped;
}

void CameraParams::pollParamsFromCamera() {
	if (!pollingEnabled) return;

	if (getElapsedSeconds() - lastPollingTime < POLL_INTERVAL) return;
	lastPollingTime = getElapsedSeconds();

	for (auto param : params) {
		if (param->poll) {
			param->updateFromCamera();
		}
	}
}