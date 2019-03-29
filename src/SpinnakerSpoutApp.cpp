#include "SpinnakerSpoutApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

/*
Make sure to enable jumbo packets on your network interface and set them to 9014 bytes:
https://www.ptgrey.com/KB/10899#Jumbo%20Packet (assuming you have a network interface that supports this). 

If set, make sure to set the GEV SCPS packet size setting on the camera to 9000 as well

When working with multiple camera's on a network with limited bandwidth, make sure they distribute the bandwidth equally using the Device Link Throughput Limit setting.

In general, keep these guidelines in mind:
https://www.ptgrey.com/KB/11151
*/

#define WINDOW_W 1024
#define WINDOW_H 768

#define NEEDED_NUM_CAMERAS 3

void prepareSettings(App::Settings *settings)
{
	settings->setHighDensityDisplayEnabled();
	settings->setTitle("Spinnaker to Spout");
	settings->setWindowSize(WINDOW_W, WINDOW_H);
	settings->setFullScreen(false);
	settings->setResizable(true);
	settings->setFrameRate(60.0f);
}

void SpinnakerSpoutApp::setup()
{
	sendWidth = UserSettings::getSetting<int>("SendWidth", sendWidth);
	sendHeight = UserSettings::getSetting<int>("SendHeight", sendHeight);
	logLevelIndex = UserSettings::getSetting<int>("LogLevelIndex", logLevelIndex);

	gl::enableAlphaBlending();
	gl::color(ColorA(1, 1, 1, 1));

	paramGUI = params::InterfaceGl::create(getWindow(), "Parameters", toPixels(ivec2(200, 300)));
	paramGUI->setSize(vec2(WINDOW_W / 3, WINDOW_H - 60));

	paramGUI->addParam("Camera Log Level", SpinnakerDeviceCommunication::getLogLevelStrings(), &logLevelIndex).updateFn([this] {
		system->SetLoggingEventPriorityLevel(SpinnakerDeviceCommunication::indexToLogLevel(logLevelIndex));
		UserSettings::writeSetting<int>("LogLevelIndex", logLevelIndex);
	});

	paramGUI->addSeparator();

	initCameras(paramGUI);

	paramGUI->addParam<bool>("Polling", &CameraParams::pollingEnabled);
	paramGUI->addParam("Camera to show", &visibleCamera).min(0).max(cameras.size() - 1);

	// -------- SPOUT --------
	paramGUI->addSeparator("Spout");
	paramGUI->addParam("Send Width", &sendWidth).min(20).updateFn([this] {
		UserSettings::writeSetting<int>("SendWidth", sendWidth);
	});

	paramGUI->addParam("Send Height", &sendHeight).min(20).updateFn([this] {
		UserSettings::writeSetting<int>("SendHeight", sendHeight);
	});
}

void SpinnakerSpoutApp::initCameras(params::InterfaceGlRef paramGUI) {
	try {
		if (system == NULL) {
			system = System::GetInstance();

			const LibraryVersion spinnakerLibraryVersion = system->GetLibraryVersion();
			console() << "Spinnaker library initialized, version: " << spinnakerLibraryVersion.major << "." << spinnakerLibraryVersion.minor << "." << spinnakerLibraryVersion.type << "." << spinnakerLibraryVersion.build << endl << endl;

			system->RegisterLoggingEvent(loggingEventHandler);
			system->SetLoggingEventPriorityLevel(SpinnakerDeviceCommunication::indexToLogLevel(logLevelIndex));

			for (int i = 0; i < NEEDED_NUM_CAMERAS; i++) {
				cameras.push_back(new SpinnakerCamera(system, i, paramGUI));
			}
		}
	}
	catch (exception e) {
		console() << "Error initializing Spinnaker library: " << e.what() << ". Do the included Spinnaker header and lib files match the dll version?" << endl;
	}
}

void SpinnakerSpoutApp::draw()
{
	gl::clear(ColorA(0, 0, 0, 1));
	gl::setMatricesWindow(getWindowSize());

	string status;
	int fps = 0;

	bool allCamerasStreaming = true;
	int i = 0;
	for (auto camera : cameras) {
		gl::TextureRef cameraTexture = camera->getLatestCameraTexture(); // gets latest available frame from camera

		if (cameraTexture != NULL) {
			if (i == visibleCamera) {
				bool flip = true;
				gl::draw(cameraTexture, Rectf(0.f, flip ? (float)getWindowHeight() : 0.f, (float)getWindowWidth(), flip ? 0.f : (float)getWindowHeight()));

				stringstream ss;
				ss << "Camera " << camera->getSerialNumber() << " capturing at " << camera->getFps() << " fps.";
				status = ss.str();
			}

			// sending as " << senderName.c_str() << " at " << sendWidth << " x " << sendHeight << "
			spoutPool.sendToSpout(camera->getSerialNumber(), sendWidth, sendHeight, cameraTexture);
		}
		else {
			allCamerasStreaming = false;
		}
		i++;
	}

	if (!allCamerasStreaming) status = "Waiting for cameras...";

	fps = (int)getAverageFps();

	drawInfoBoxes(status, fps);
	if (paramGUI != NULL) paramGUI->draw();
}

void SpinnakerSpoutApp::drawInfoBoxes(string status, int fps) {

	float fontSize = (float)(getWindowWidth() / 50);

	float fpsLeft = getWindowWidth() - toPixels(240);
	float fpsTop = toPixels(20);

	float infoLeft = toPixels(20);
	float infoTop = getWindowHeight() - toPixels(20) - fontSize;

	{
		gl::ScopedColor colorScope(ColorA(0, 0, 0, 0.5));
		gl::drawSolidRect(Rectf(fpsLeft - 10.f, fpsTop - 10.f, (float)getWindowWidth(), fpsTop + fontSize + 10.f));
		gl::drawSolidRect(Rectf(0.f, infoTop - 10.f, (float)getWindowWidth(), infoTop + fontSize + 10.f));
	}

	gl::drawString(status, vec2(infoLeft, infoTop), ColorA(1, 1, 1, 1), Font("Verdana", fontSize));

	stringstream ss;
	ss << "Fps: " << fps << ", dropped ";
	int i = 0;
	for (auto camera : cameras) {
		ss << camera->getLatestDroppedFrames() << (i < cameras.size() - 1 ? " | " : "");
		i++;
	}
	gl::drawString(ss.str(), vec2(fpsLeft, fpsTop), ColorA(1, 1, 1, 1), Font("Verdana", fontSize));
}

void SpinnakerSpoutApp::cleanup()
{
	for (auto camera : cameras) {
		camera->cleanup();
	}
	system->UnregisterLoggingEvent(loggingEventHandler);
	system->ReleaseInstance();
	spoutPool.cleanup();
}

CINDER_APP(SpinnakerSpoutApp, RendererGl, prepareSettings)
