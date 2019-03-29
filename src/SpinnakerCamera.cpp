#include "SpinnakerCamera.h"

#include "SpinnakerDeviceCommunication.h"
#include "CameraParam.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define CAMERA_AVAILABLE_CHECK_INTERVAL 2

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
		try {
			auto tex = getNextCameraTexture();

			if (tex != NULL) {
				// we need to wait on a fence before alerting the primary thread that the Texture is ready
				auto fence = gl::Sync::create();
				fence->clientWaitSync();

				frameBuffer->pushFront(tex);
			}
			else {
				droppedFrames++;
			}
		}
		catch (ci::Exception &exc) {
			console() << "Failed to create camera texture: " << exc.what() << endl;
		}
	}
}

gl::TextureRef SpinnakerCamera::getLatestCameraTexture() {
	while (frameBuffer->getSize() > 0) {
		frameBuffer->popBack(&latestTexture);
	}
	return latestTexture;
}

gl::TextureRef SpinnakerCamera::getNextCameraTexture() {
	if (getElapsedSeconds() - lastCameraInitFailTime < CAMERA_AVAILABLE_CHECK_INTERVAL) return false;

	if (!checkCameraAssigned()) {
		console() << "Camera " + to_string(cameraIndex) + " not available, retrying in " << CAMERA_AVAILABLE_CHECK_INTERVAL << " seconds." << endl;
		lastCameraInitFailTime = getElapsedSeconds();
		return NULL;
	}

	if (!checkCameraInitialized()) { // blocks while initializing
		console() << "Could not initialize Camera " + to_string(cameraIndex) << ", retrying in " << CAMERA_AVAILABLE_CHECK_INTERVAL << " seconds." << endl;
		lastCameraInitFailTime = getElapsedSeconds();
		return NULL;
	}

	checkParamInterfaceInitialized();
	cameraParams.pollParamsFromCamera();

	bool cameraStopped = cameraParams.applyParams();  // potentially stops camera acquisition to apply changed settings
	cameraStarted = cameraStarted && !cameraStopped;

	if (!checkCameraStarted()) {
		console() << "Unable to start Camera " + to_string(cameraIndex) << ", retrying in " << CAMERA_AVAILABLE_CHECK_INTERVAL << " seconds." << endl;
		lastCameraInitFailTime = getElapsedSeconds();
		return NULL;
	}

	bool success = SpinnakerDeviceCommunication::getCameraTexture(camera, cameraTexture); // block until a new frame is available or a frame is reported incomplete. (re-)initializes output texture if needed

	if (success == false || cameraTexture == NULL) {
		return NULL;
	}
	else {
		int w = cameraTexture->getWidth();
		int h = cameraTexture->getHeight();

		if (prevCaptureWidth != w || prevCaptureHeight != h) {
			console() << "Now grabbing images at " << w << " x " << h << endl;
			prevCaptureWidth = w;
			prevCaptureHeight = h;
		}

		return cameraTexture;
	}
}

bool SpinnakerCamera::checkCameraAssigned() {
	if (camera != NULL) return true;

	CameraList camList = system->GetCameras();

	unsigned int numCameras = camList.GetSize();
	if (numCameras == 0)
	{
		console() << "No cameras found." << endl;
		cameraInitialized = false;
		return false;
	}
	if (cameraIndex > numCameras - 1) {
		console() << "No camera found at index " << cameraIndex << ", only " << numCameras << " available." << endl;
		cameraInitialized = false;
		return false;
	}

	camera = camList.GetByIndex(cameraIndex);
	console() << "Assigned camera " << cameraIndex << " (of " << numCameras << ")" << endl;
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
		console() << "Initialized camera " << camera->DeviceModelName() << endl;
		//SpinnakerDeviceCommunication::printDeviceInfo(camera);
		return true;
	}
	catch (Spinnaker::Exception &e) {
		console() << "Error initializing camera: " << e.what() << endl;
		cameraInitialized = false;
		return false;
	}
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

bool SpinnakerCamera::checkCameraStarted() {
	if (cameraStarted) {
		if (!camera->IsValid()) {
			console() << "Camera " << cameraIndex << " is invalid, stopping it." << endl;
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

void SpinnakerCamera::cleanup() {
	if (camera != NULL) {
		SpinnakerDeviceCommunication::checkStreamingStopped(camera);
		if (camera->IsStreaming()) camera->EndAcquisition();
		if (camera->IsInitialized()) camera->DeInit();
		cameraStarted = false;
		camera = NULL;
	}
}

SpinnakerCamera::~SpinnakerCamera()
{
	shouldQuit = true;
	frameBuffer->cancel();
	captureThread->join();
	cleanup();
}
