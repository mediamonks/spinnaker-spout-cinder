#include "SpinnakerSpoutApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define CAMERA_AVAILABLE_CHECK_INTERVAL 2

/*
Make sure to enable jumbo packets on your network interface and set them to 9014 bytes:
https://www.ptgrey.com/KB/10899#Jumbo%20Packet (assuming you have a network interface that supports this)

When working with multiple camera's on a network with limited bandwidth, make sure they distribute the bandwidth equally using the Device Link Throughput Limit setting.

In general, keep these guidelines in mind:
https://www.ptgrey.com/KB/11151
*/

void prepareSettings(App::Settings *settings)
{
	settings->setHighDensityDisplayEnabled();
	settings->setTitle("Spinnaker to Spout");
	settings->setWindowSize(1024, 768);
	settings->setFullScreen(false);
	settings->setResizable(true);
	settings->setFrameRate(60.0f);
}

void SpinnakerSpoutApp::setup()
{
	sendWidth = UserSettings::getSetting<int>("sendWidth", sendWidth);
	sendHeight = UserSettings::getSetting<int>("sendHeight", sendHeight);
	binning = UserSettings::getSetting<int>("binning", binning);
	gainAutoIndex = UserSettings::getSetting<int>("gainAuto", gainAutoIndex); // enum, but index stored as int
	exposureAutoIndex = UserSettings::getSetting<int>("exposureAuto", exposureAutoIndex); // enum, but index stored as int
	exposure = UserSettings::getSetting<double>("exposureTimeAbs", exposure);
	pixelFormatIndex = UserSettings::getSetting<int>("pixelFormat", pixelFormatIndex); // enum, but index stored as int
	logLevelIndex = UserSettings::getSetting<int>("logLevelIndex", logLevelIndex);
	deviceLinkThroughputLimit = UserSettings::getSetting<int>("deviceLinkThroughputLimit", deviceLinkThroughputLimit);

	gl::enableAlphaBlending();
	gl::color(ColorA(1, 1, 1, 1));
}

void SpinnakerSpoutApp::draw()
{
	gl::clear(ColorA(0, 0, 0, 1));
	gl::setMatricesWindow(getWindowSize());

	string status;
	int fps = 0;

	if (needsInitText) {
		// draw this before starting camera initialization because it blocks app
		status = "Initializing...";
		needsInitText = false;
	}
	else {
		gl::TextureRef cameraTexture = getCameraTexture(status); // blocks during intializion of the camera and every frame until a new texture arrived 

		if (cameraTexture != NULL) {
			if (paramGUI == NULL) initParamInterface(); // requires an active camera
			updateParamsFromCamera();

			bool flip = true;
			gl::draw(cameraTexture, Rectf(0, flip ? getWindowHeight() : 0, getWindowWidth(), flip ? 0 : getWindowHeight()));

			sendToSpout(cameraTexture);
			fps = (int)getAverageFps();
		}
	}

	int fontSize = getWindowWidth() / 50;

	int fpsLeft = getWindowWidth() - toPixels(240);
	int fpsTop = toPixels(20);

	int infoLeft = toPixels(20);
	int infoTop = getWindowHeight() - toPixels(20) - fontSize;

	{
		gl::ScopedColor colorScope(ColorA(0, 0, 0, 0.5));
		gl::drawSolidRect(Rectf(fpsLeft - 10, fpsTop - 10, getWindowWidth(), fpsTop + fontSize + 10));
		gl::drawSolidRect(Rectf(0, infoTop - 10, getWindowWidth(), infoTop + fontSize + 10));
	}

	gl::drawString(status, vec2(infoLeft, infoTop), ColorA(1, 1, 1, 1), Font("Verdana", fontSize));

	stringstream ss;
	ss << "Fps: " << fps << ", dropped " << geLatestDroppedFrames();
	gl::drawString(ss.str(), vec2(fpsLeft, fpsTop), ColorA(1, 1, 1, 1), Font("Verdana", fontSize));

	if (paramGUI != NULL) paramGUI->draw();
}

void SpinnakerSpoutApp::sendToSpout(gl::TextureRef &sendTexture) {
	if (!checkSpoutInitialized()) return;

	gl::ScopedFramebuffer frameBufferScope(sendFbo);
	gl::ScopedViewport viewPortScope(sendFbo->getSize());
	gl::ScopedMatrices matrixScope;
	gl::setMatricesWindow(sendFbo->getSize(), false);

	gl::draw(sendTexture, sendFbo->getBounds());

	// Send the texture for all receivers to use
	// NOTE : if SendTexture is called with a framebuffer object bound,
	// include the FBO id as an argument so that the binding is restored afterwards
	// because Spout uses an fbo for intermediate rendering
	auto tex = sendFbo->getColorTexture();
	spoutSender.SendTexture(tex->getId(), tex->getTarget(), tex->getWidth(), tex->getHeight());
}

bool spoutInitialized = false;
bool SpinnakerSpoutApp::checkSpoutInitialized() {
	if (spoutInitialized && sendFbo != NULL && sendFbo->getWidth() == sendWidth && sendFbo->getHeight() == sendHeight) return true;

	if (spoutInitialized) spoutSender.ReleaseSender();

	sendFbo = gl::Fbo::create(sendWidth, sendHeight);
	spoutInitialized = spoutSender.CreateSender(senderName.c_str(), sendWidth, sendHeight);

	// MemoryShareMode informs us whether Spout is initialized for texture share or for memory share
	bool memoryMode = spoutSender.GetMemoryShareMode();
	if (spoutInitialized) console() << "Spout initialized using " << (memoryMode ? "Memory" : "Texture") << " sharing at " << sendWidth << " x " << sendHeight << endl;
	else console() << "Spout initialization failed" << endl;

	return spoutInitialized;
}

void SpinnakerSpoutApp::initParamInterface() {
	paramGUI = params::InterfaceGl::create(getWindow(), "Parameters", toPixels(ivec2(200, 300)));

	// -------- SPINNAKER --------
	std::vector<string> logLevelEnums;
	logLevelEnums.push_back("Off");
	logLevelEnums.push_back("Fatal");
	logLevelEnums.push_back("Alert");
	logLevelEnums.push_back("Crit");
	logLevelEnums.push_back("Error");
	logLevelEnums.push_back("Warn");
	logLevelEnums.push_back("Notice");
	logLevelEnums.push_back("Info");
	logLevelEnums.push_back("Debug");
	logLevelEnums.push_back("All");

	paramGUI->addParam("Camera Log Level", logLevelEnums, &logLevelIndex).min(20).updateFn([this] {
		system->SetLoggingEventPriorityLevel(indexToSpinnakerLogLevel(logLevelIndex));
		UserSettings::writeSetting<int>("logLevelIndex", logLevelIndex);
	});

	paramGUI->addSeparator();

	std::vector<string> binningEnums;
	binningEnums.push_back("No binning");
	binningEnums.push_back("Binning 2x");
	paramGUI->addParam("Binning", binningEnums, &binning).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<int>("binning", binning);
	});

	paramGUI->addParam("Gain Auto", SpinnakerDeviceCommunication::getParameterEnumOptions(camera, "GainAuto"), &gainAutoIndex).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<int>("gainAuto", gainAutoIndex);
	});

	paramGUI->addParam("Exposure Auto", SpinnakerDeviceCommunication::getParameterEnumOptions(camera, "ExposureAuto"), &exposureAutoIndex).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<int>("exposureAuto", exposureAutoIndex);
	});

	paramGUI->addParam("Exposure", &exposure).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<double>("exposureTimeAbs", exposure);
	});

	paramGUI->addParam("Pixel Format", SpinnakerDeviceCommunication::getParameterEnumOptions(camera, "PixelFormat"), &pixelFormatIndex).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<int>("pixelFormat", pixelFormatIndex);
	});

	paramGUI->addSeparator("Stream");

	paramGUI->addParam("Device Link Throughput Limit", &deviceLinkThroughputLimit).updateFn([this] {
		cameraSettingsDirty = true;
		UserSettings::writeSetting<int>("deviceLinkThroughputLimit", deviceLinkThroughputLimit);
	});

	paramGUI->addSeparator("Spout");

	// -------- SPOUT --------
	paramGUI->addParam("Send Width", &sendWidth).min(20).updateFn([this] {
		UserSettings::writeSetting<int>("sendWidth", sendWidth);
	});

	paramGUI->addParam("Send Height", &sendHeight).min(20).updateFn([this] {
		UserSettings::writeSetting<int>("sendHeight", sendHeight);
	});
}

gl::TextureRef SpinnakerSpoutApp::getCameraTexture(string &status) {
	if (!checkCameraInitialized()) { // blocks while initializing
		status = "No camera available.";
		return NULL;
	}

	if (cameraSettingsDirty) {
		cameraSettingsDirty = false;
		applyCameraSettings(); // potentially stops camera acquisition to apply changed settings
	}

	if (!checkStreamingStarted()) {
		status = "Unable to start camera...";
		return NULL;
	}

	if (!camera->IsValid()) {
		checkStreamingStopped();
		camera->DeInit();
		status = "Camera status invalid. Attempting to restart.";
		console() << "Camera status invalid." << endl;
		return NULL;
	}

	gl::TextureRef cameraTexture = getCameraTexture(); // block until a new frame is available or a frame is reported incomplete
	if (!cameraTexture) {
		status = "Dropped frame.";
		return NULL;
	}

	stringstream ss;
	ss << "Capturing from " << camera->DeviceModelName.GetValue() << " at " << cameraTexture->getWidth() << " x " << cameraTexture->getHeight() << ", sending as " << senderName.c_str() << " at " << sendWidth << " x " << sendHeight;
	status = ss.str();
	return cameraTexture;
}

// Stops camera if necessary. Make sure to check whether it is started after calling this method.
// returns true if camera was stopped, false otherwise
bool SpinnakerSpoutApp::applyCameraSettings() {
	if (camera == NULL) return true;

	bool cameraWasStopped = false;
	// API is weird here since you can only read binning from Horizontal but need to write to Horizontal and Vertical
	if (SpinnakerDeviceCommunication::getParameterIntValue(camera, "BinningHorizontal") - 1 != binning) {
		cameraWasStopped = checkStreamingStopped();
		SpinnakerDeviceCommunication::setParameterInt(camera, "BinningHorizontal", binning + 1); // binning param values are set as int starting from 1
		SpinnakerDeviceCommunication::setParameterInt(camera, "BinningVertical", binning + 1);
	}

	vector<string> gainAutoOptions = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, "GainAuto");
	if (SpinnakerDeviceCommunication::getParameterEnumValue(camera, "GainAuto") != gainAutoOptions[gainAutoIndex]) {
		string newGainAuto = SpinnakerDeviceCommunication::setParameterEnum(camera, "GainAuto", gainAutoOptions[gainAutoIndex]);
		int newGainAutoIndex = find(gainAutoOptions.begin(), gainAutoOptions.end(), newGainAuto) - gainAutoOptions.begin();
		if (newGainAutoIndex != gainAutoIndex) {
			gainAutoIndex = newGainAutoIndex;
			UserSettings::writeSetting<int>("gainAuto", gainAutoIndex);
		}
	}

	vector<string> exposureAutoOptions = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, "ExposureAuto");
	if (SpinnakerDeviceCommunication::getParameterEnumValue(camera, "ExposureAuto") != exposureAutoOptions[exposureAutoIndex]) {
		string newExposureAuto = SpinnakerDeviceCommunication::setParameterEnum(camera, "ExposureAuto", exposureAutoOptions[exposureAutoIndex]);
		int newExposureAutoIndex = find(exposureAutoOptions.begin(), exposureAutoOptions.end(), newExposureAuto) - exposureAutoOptions.begin();
		if (newExposureAutoIndex != exposureAutoIndex) {
			exposureAutoIndex = newExposureAutoIndex;
			UserSettings::writeSetting<int>("exposureAuto", exposureAutoIndex);
		}
	}

	if (SpinnakerDeviceCommunication::getParameterFloatValue(camera, "ExposureTimeAbs") != exposure) {
		exposure = SpinnakerDeviceCommunication::setParameterFloat(camera, "ExposureTimeAbs", exposure);
	}

	if (SpinnakerDeviceCommunication::getParameterIntValue(camera, "DeviceLinkThroughputLimit") != deviceLinkThroughputLimit) {
		int newDeviceLinkThroughputLimit = SpinnakerDeviceCommunication::setParameterInt(camera, "DeviceLinkThroughputLimit", deviceLinkThroughputLimit);
		if (newDeviceLinkThroughputLimit != deviceLinkThroughputLimit) {
			deviceLinkThroughputLimit = newDeviceLinkThroughputLimit;
			UserSettings::writeSetting<int>("deviceLinkThroughputLimit", newDeviceLinkThroughputLimit);
		}
	}

	vector<string> pixelFormatOptions = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, "PixelFormat");
	if (SpinnakerDeviceCommunication::getParameterEnumValue(camera, "PixelFormat") != pixelFormatOptions[pixelFormatIndex]) {
		cameraWasStopped = checkStreamingStopped();
		string pixelFormat = SpinnakerDeviceCommunication::setParameterEnum(camera, "PixelFormat", pixelFormatOptions[pixelFormatIndex]);
		int newPixelFormatIndex = find(pixelFormatOptions.begin(), pixelFormatOptions.end(), pixelFormat) - pixelFormatOptions.begin();
		if (newPixelFormatIndex != pixelFormatIndex) {
			pixelFormatIndex = newPixelFormatIndex;
			UserSettings::writeSetting<int>("pixelFormat", pixelFormatIndex);
		}
	}
	return cameraWasStopped;
}

void SpinnakerSpoutApp::updateParamsFromCamera() {
	float newExposure = SpinnakerDeviceCommunication::getParameterFloatValue(camera, "ExposureTimeAbs");
	if (newExposure != exposure) {
		exposure = newExposure;
		UserSettings::writeSetting<double>("exposureTimeAbs", exposure);
	}
}

float lastCameraInitCheckTime = -100;
bool SpinnakerSpoutApp::checkCameraInitialized() {
	if (camera != NULL && camera->IsInitialized()) return true;

	if (getElapsedSeconds() - lastCameraInitCheckTime < CAMERA_AVAILABLE_CHECK_INTERVAL) return false;
	lastCameraInitCheckTime = getElapsedSeconds();

	try {
		if (system == NULL) {
			system = System::GetInstance();

			const LibraryVersion spinnakerLibraryVersion = system->GetLibraryVersion();
			console() << "Spinnaker library version: "
				<< spinnakerLibraryVersion.major << "."
				<< spinnakerLibraryVersion.minor << "."
				<< spinnakerLibraryVersion.type << "."
				<< spinnakerLibraryVersion.build << endl << endl;

			system->RegisterLoggingEvent(loggingEventHandler);
			system->SetLoggingEventPriorityLevel(indexToSpinnakerLogLevel(logLevelIndex));
		}

		CameraList camList = system->GetCameras();

		unsigned int numCameras = camList.GetSize();
		if (numCameras == 0)
		{
			console() << "No cameras found" << endl;
		}
		else {
			camera = camList.GetByIndex(0);
			try {
				console() << "Initializing camera 0 (" << camList.GetSize() << " available)..." << endl;
				camera->Init();
				//SpinnakerDeviceCommunication::printDeviceInfo(camera);
				return true;
			}
			catch (Spinnaker::Exception &e) {
				console() << "Error initializing camera: " << e.what() << endl;
			}
		}
	}
	catch (exception e) {
		console() << "Error initializing Spinnaker library: " << e.what() << ". Do the included Spinnaker header and lib files match the dll version?" << endl;
	}

	console() << "Initializing camera failed, retrying in " << CAMERA_AVAILABLE_CHECK_INTERVAL << " seconds." << endl;

	return false;
}

int prevCaptureWidth = 0;
int prevCaptureHeight = 0;
// returns true if camera is streaming after calling this method
bool SpinnakerSpoutApp::checkStreamingStarted() {
	if (camera->IsStreaming()) return true;

	try
	{
		console() << "Starting camera with access mode " << SpinnakerDeviceCommunication::accessModeToString(camera->GetAccessMode()) << "..." << endl;
		SpinnakerDeviceCommunication::setParameterEnum(camera, "AcquisitionMode", "Continuous");
		camera->BeginAcquisition();
		prevCaptureWidth = 0;
		prevCaptureHeight = 0;
		return true;
	}
	catch (Spinnaker::Exception &e)	{
		console() << "Error starting camera aquisition: " << e.what() << endl;
		camera->DeInit();
		return false;
	}
}

// returns true if camera is not streaming after calling this method
bool SpinnakerSpoutApp::checkStreamingStopped() {
	if (!camera->IsStreaming()) return true;

	try {
		camera->EndAcquisition();
		return true;
	}
	catch (Spinnaker::Exception &e) {
		console() << "Error stopping camera aquisition: " << e.what() << endl;
		return false;
	}
}

gl::TextureRef SpinnakerSpoutApp::getCameraTexture() {
	try {
		ImagePtr capturedImage = camera->GetNextImage(1000); // Note: blocks until a new frame is available, limiting the frame rate of the entire app
		if (capturedImage->IsIncomplete())
		{
			console() << "Image incomplete with image status " << capturedImage->GetImageStatus() << "..." << endl;
			capturedImage->Release();
			droppedFrames++;
		}
		else
		{
			int w = capturedImage->GetWidth();
			int h = capturedImage->GetHeight();

			if (prevCaptureWidth != w || prevCaptureHeight != h) {
				console() << "Now grabbing images at " << capturedImage->GetWidth() << " x " << capturedImage->GetHeight() << ", " << capturedImage->GetPixelFormatName() << endl;
				prevCaptureWidth = w;
				prevCaptureHeight = h;
			}

			ImagePtr convertedImage = capturedImage->Convert(PixelFormat_RGB8, NEAREST_NEIGHBOR); // Note that color processing algorithms other than NEAREST_NEIGHBOR are probably too slow for continous acquisition at high resolutions.
			capturedImage->Release();

			if (texture == NULL || texture->getWidth() != w || texture->getHeight() != h) {
				texture = gl::Texture2d::create(w, h);
			}

			texture->update(convertedImage->GetData(), GL_RGB, GL_UNSIGNED_BYTE, 0, w, h);
			return texture;
		}
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error capturing image: " << e.what() << endl;
	}

	return NULL;
}

int SpinnakerSpoutApp::geLatestDroppedFrames() {
	if (getElapsedSeconds() - lastDroppedFramesTime > 1) {
		droppedFrames = 0;
		lastDroppedFramesTime = getElapsedSeconds();
	}
	return droppedFrames;
}

void SpinnakerSpoutApp::cleanup()
{
	if (camera != NULL) {
		if (camera->IsStreaming()) camera->EndAcquisition();
		if (camera->IsInitialized()) camera->DeInit();
		camera = NULL;
	}
	system->UnregisterLoggingEvent(loggingEventHandler);
	system->ReleaseInstance(); // Release system
	spoutSender.ReleaseSender();
}

SpinnakerLogLevel indexToSpinnakerLogLevel(int index) {
	switch (index) {
	case 0: return LOG_LEVEL_OFF;
	case 1: return LOG_LEVEL_FATAL;
	case 2: return LOG_LEVEL_ALERT;
	case 3: return LOG_LEVEL_CRIT;
	case 4: return LOG_LEVEL_ERROR;
	case 5: return LOG_LEVEL_WARN;
	case 6: return LOG_LEVEL_NOTICE;
	case 7: return LOG_LEVEL_INFO;
	case 8: return LOG_LEVEL_DEBUG;
	case 9: return LOG_LEVEL_NOTSET;
	}
}

CINDER_APP(SpinnakerSpoutApp, RendererGl, prepareSettings)
