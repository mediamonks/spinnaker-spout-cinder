#include <chrono> 

#include "SpinnakerCamera.h"

#include "SpinnakerDeviceCommunication.h"
#include "CameraParam.h"
#include "Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define CAMERA_AVAILABLE_CHECK_INTERVAL 2

// Whether to save the changed settings to the non-volatile memory of the camera. 
// If true, always saves to User Set 1 and makes User Set 1 the default for initialization
#define SAVE_TO_NON_VOLATILE_ON_CHANGE true 

SpinnakerCamera::SpinnakerCamera(SystemPtr _system, int index, params::InterfaceGlRef _paramGUI) {
	cameraIndex = index;
	system = _system;
	paramGUI = _paramGUI;

	frameBuffer = new ConcurrentCircularBuffer<gl::TextureRef>(5); // room for 5 frames

	gl::ContextRef backgroundCtx = gl::Context::create(gl::context());
	captureThread = shared_ptr<thread>(new thread(bind(&SpinnakerCamera::captureThreadFn, this, backgroundCtx)));
}

void SpinnakerCamera::captureThreadFn(gl::ContextRef context) {
	ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread
								 // we received as a parameter a gl::Context we can use safely that shares resources with the primary Context
	context->makeCurrent();

	while (!shouldQuit) {
		if (getElapsedSeconds() - lastCameraInitFailTime < CAMERA_AVAILABLE_CHECK_INTERVAL) {
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		if (!checkCameraUpdatedAndRunning()) {
			Log() << "Camera " + to_string(cameraIndex) + " initialization failed, retrying in " << CAMERA_AVAILABLE_CHECK_INTERVAL << " seconds.";
			lastCameraInitFailTime = getElapsedSeconds();
			continue;
		}

		try {
			auto tex = getNextCameraTexture();

			if (tex == NULL) {
				droppedFrames++;
			}
			else {
				// we need to wait on a fence before alerting the primary thread that the Texture is ready
				auto fence = gl::Sync::create();
				fence->clientWaitSync();

				frameBuffer->pushFront(tex);
			}
		}
		catch (ci::Exception &exc) {
			Log() << "Failed to create camera texture: " << exc.what();
		}
	}
}

gl::TextureRef SpinnakerCamera::getLatestCameraTexture() {
	while (frameBuffer->getSize() > 0) {
		frameBuffer->popBack(&latestTexture);
	}
	return latestTexture;
}

bool SpinnakerCamera::checkCameraUpdatedAndRunning() {
	if (!checkCameraAssigned()) {
		Log() << "Camera " + to_string(cameraIndex) + " not available.";
		return false;
	}

	if (!checkCameraInitialized()) { // blocks while initializing
		Log() << "Could not initialize Camera " + to_string(cameraIndex) << ".";
		return false;
	}

	checkParamInterfaceInitialized();

	pair<bool, bool> feedback = cameraParams.applyParams();  // note: potentially stops camera acquisition to apply changed settings
	bool paramApplied = feedback.first;
	bool cameraStopped = feedback.second;

	// make sure to register if the camera is no longer in streaming state
	cameraStarted = cameraStarted && !cameraStopped;

	if (SAVE_TO_NON_VOLATILE_ON_CHANGE && paramApplied) save(); // note: this stops the camera

	cameraParams.pollParamsFromCamera(cameraStopped); // if camera was stopped, poll all parameters once

	if (!checkCameraStreaming()) {
		Log() << "Unable to start Camera " + to_string(cameraIndex) << ".";
		return false;
	}

	return true;
}

void SpinnakerCamera::save() {
	checkCameraStopped();
	camera->UserSetDefault.SetIntValue(1);
	camera->UserSetSelector.SetIntValue(1);
	camera->UserSetSave.Execute();
}

gl::TextureRef SpinnakerCamera::getNextCameraTexture() {
	bool success = SpinnakerDeviceCommunication::getCameraTexture(camera, cameraTexture); // block until a new frame is available or a frame is reported incomplete. (re-)initializes output texture if needed

	if (success == false || cameraTexture == NULL) {
		return NULL;
	}
	else {
		int w = cameraTexture->getWidth();
		int h = cameraTexture->getHeight();

		if (prevCaptureWidth != w || prevCaptureHeight != h) {
			Log() << "Camera " << camera->DeviceSerialNumber() << " now grabbing images at " << w << " x " << h;
			prevCaptureWidth = w;
			prevCaptureHeight = h;
		}

		double now = getElapsedSeconds();
		if (prevFrameTime == 0) {
			prevFrameTime = now;
			fps = 0;
		}
		else {
			double thisFrameFps = 1 / (now - prevFrameTime);
			fps = fps * 0.9f + 0.1f * thisFrameFps;
			prevFrameTime = now;
		}
		return cameraTexture;
	}
}

bool SpinnakerCamera::checkCameraAssigned() {
	if (camera != NULL) return true;

	CameraList camList = system->GetCameras();

	int numCameras = camList.GetSize();
	if (numCameras == 0)
	{
		Log() << "No cameras found.";
		cameraInitialized = false;
		return false;
	}
	if (cameraIndex > numCameras - 1) {
		Log() << "No camera found at index " << cameraIndex << ", only " << numCameras << " available.";
		cameraInitialized = false;
		return false;
	}

	camera = camList.GetByIndex(cameraIndex);
	Log() << "Assigned camera " << cameraIndex << " (of " << numCameras << ")";
	return true;
}

bool SpinnakerCamera::checkCameraInitialized() {
	if (cameraInitialized) return true;

	if (camera != NULL && camera->IsInitialized()) {
		cameraInitialized = true;
		return true;
	}

	try {
		camera->Init();
		cameraInitialized = true;
		Log() << "Initialized camera " << camera->DeviceModelName() << " (" << camera->DeviceSerialNumber() << ")";		
		return true;
	}
	catch (Spinnaker::Exception &e) {
		Log() << "Error initializing camera " << cameraIndex << ": " << e.what();
		cameraInitialized = false;
		return false;
	}
}

void SpinnakerCamera::printInfo() {
	SpinnakerDeviceCommunication::printDeviceInfo(camera);
}

// requires an active camera
void SpinnakerCamera::checkParamInterfaceInitialized() {
	if (paramInterfaceInitialized) return;

	cameraParams.createEnum("Video Mode", "VideoMode", paramGUI, camera, 1, cameraIndex, true); // determines what features like binning and color mode are available
	cameraParams.createInt("Binning", "BinningVertical", paramGUI, camera, 1, cameraIndex, true); // 1 is no binning (1x scale), 2 = factor 2 binning (0.5x scale)
	cameraParams.createEnum("Gain Auto", "GainAuto", paramGUI, camera, 0, cameraIndex);
	cameraParams.createFloat("Gain", "Gain", paramGUI, camera, 15, cameraIndex, false, true); // in dB
	cameraParams.createEnum("White Balance Auto", "BalanceWhiteAuto", paramGUI, camera, 0, cameraIndex);
	cameraParams.createFloat("White Balance Ratio", "BalanceRatio", paramGUI, camera, 1, cameraIndex, false, true);
	cameraParams.createEnum("Exposure Auto", "ExposureAuto", paramGUI, camera, 0, cameraIndex);
	cameraParams.createFloat("Exposure", "ExposureTimeAbs", paramGUI, camera, 10000, cameraIndex, false, true); // in microseconds
	cameraParams.createEnum("Pixel Format", "PixelFormat", paramGUI, camera, 4, cameraIndex, true);
	paramGUI->addSeparator("Stream " + to_string(cameraIndex));
	cameraParams.createInt("Device Link Throughput Limit", "DeviceLinkThroughputLimit", paramGUI, camera, 10000000, cameraIndex);  // max bandwidth used by this camera in bytes/second

	paramInterfaceInitialized = true;
}

bool SpinnakerCamera::checkCameraStreaming() {
	if (cameraStarted) {
		if (!camera->IsValid()) {
			Log() << "Camera " << cameraIndex << " is invalid, stopping it.";
			SpinnakerDeviceCommunication::checkStreamingStopped(camera);
			camera->DeInit();
			cameraInitialized = false;
			cameraStarted = false;
		}
	}

	if (cameraStarted) return true;

	cameraStarted = SpinnakerDeviceCommunication::checkStreamingStarted(camera);
	prevCaptureWidth = 0;
	prevCaptureHeight = 0;

	return cameraStarted;
}

bool SpinnakerCamera::checkCameraStopped() {
	if (!cameraStarted) return true;
	cameraStarted = !SpinnakerDeviceCommunication::checkStreamingStopped(camera);
}

int SpinnakerCamera::getLatestDroppedFrames() {
	if (getElapsedSeconds() - lastDroppedFramesTime > 1) {
		droppedFrames = 0;
		lastDroppedFramesTime = getElapsedSeconds();
	}
	return droppedFrames;
}

string SpinnakerCamera::getSerialNumber() {
	if (camera != NULL) {
		stringstream ss;
		ss << camera->DeviceSerialNumber();
		return ss.str();
	}
	else {
		return "";
	}
}

float SpinnakerCamera::getFps() {
	return fps;
}

void SpinnakerCamera::cleanup() {
	if (!shouldQuit) {
		shouldQuit = true;
		frameBuffer->cancel(); // note, order matters here. joining before cancelling the frame buffer may hang the app
		captureThread->join();
	}

	if (camera != NULL) {
		checkCameraStopped();
		if (camera->IsInitialized()) camera->DeInit();
		camera = NULL;
	}
}

SpinnakerCamera::~SpinnakerCamera()
{
	cleanup();
}
