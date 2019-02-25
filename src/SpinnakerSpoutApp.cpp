#include "SpinnakerSpoutApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define CAMERA_AVAILABLE_CHECK_INTERVAL 2

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
	sendWidth = UserSettings::getSetting<int>("sendWidth", 640);
	sendHeight = UserSettings::getSetting<int>("sendHeight", 480);
	binning = UserSettings::getSetting<int>("binning", 1);
	pixelFormat = UserSettings::getSetting<string>("pixelFormat", "BayerRG8");
}

void SpinnakerSpoutApp::draw()
{
	gl::clear(ColorA(0, 0, 0, 1));
	gl::color(ColorA(1, 1, 1, 1));
	gl::enableAlphaBlending();

	int fontSize = getWindowWidth() / 50;

	int fpsLeft = getWindowWidth() - toPixels(240);
	int fpsTop = toPixels(20);

	int infoLeft = toPixels(20);
	int infoTop = getWindowHeight() - toPixels(20) - fontSize;

	stringstream info;
	int fps = 0;

	if (needsInitText) {
		info << "Initializing...";
		needsInitText = false;
	}
	else {
		checkCameraAvailable(); // blocks while initializing
		if (!cameraFound) {
			info << "No camera found...";
		}
		else {
			if (!paramInterfaceInited) {
				initParamInterface();
				paramInterfaceInited = true;
			}

			bool cameraWasStopped = applyParamsIfNeeded(); // potentially stops camera acquisition to apply changed settings
			cameraRunning = cameraRunning && !cameraWasStopped;
			if (!cameraRunning) cameraRunning = startCamera();

			if (!cameraRunning) {
				info << "Unable to start camera...";
			}
			else {
				if (!camera->IsValid()) {
					cameraRunning = false;
					cameraFound = false;	
					info << "Camera status invalid. Attempting to restart.";
					console() << "Camera status invalid." << endl;
				}
				else {
					gl::TextureRef cameraTexture = getCameraTexture(); // block until a new frame is available or a frame is reported incomplete
					if (!cameraTexture) {
						info << "Dropped frame.";
					}
					else {
						gl::setMatricesWindow(getWindowSize());
						gl::draw(cameraTexture, Rectf(0, 0, getWindowWidth(), getWindowHeight()));

						info << "Capturing from " << camera->DeviceModelName.GetValue() << " at " << cameraTexture->getWidth() << " x " << cameraTexture->getHeight() << ", sending as " << senderName.c_str() << " at " << sendWidth << " x " << sendHeight;

						fps = (int)getAverageFps();

						// -------- SPOUT --------
						if (spoutInitialized) {
							gl::ScopedFramebuffer frameBufferScope(sendFbo);
							gl::ScopedViewport viewPortScope(sendFbo->getSize());
							gl::ScopedMatrices matrixScope();
							gl::setMatricesWindow(sendFbo->getSize(), false);

							gl::draw(cameraTexture, Rectf(0, 0, sendFbo->getWidth(), sendFbo->getHeight()));

							// Send the texture for all receivers to use
							// NOTE : if SendTexture is called with a framebuffer object bound,
							// include the FBO id as an argument so that the binding is restored afterwards
							// because Spout uses an fbo for intermediate rendering
							auto tex = sendFbo->getColorTexture();
							spoutSender.SendTexture(tex->getId(), tex->getTarget(), tex->getWidth(), tex->getHeight());
						}
					}
				}
			}
		}
	}

	// draw info

	gl::setMatricesWindow(getWindowSize());
	gl::enableAlphaBlending();

	gl::color(ColorA(0, 0, 0, 0.5));
	gl::drawSolidRect(Rectf(fpsLeft - 10, fpsTop - 10, getWindowWidth(), fpsTop + fontSize + 10));
	gl::drawSolidRect(Rectf(0, infoTop - 10, getWindowWidth(), infoTop + fontSize + 10));

	stringstream fpsSS;
	fpsSS << "Fps: " << fps << ", dropped " << geLatestDroppedFrames();
	gl::drawString(fpsSS.str(), vec2(fpsLeft, fpsTop), ColorA(1, 1, 1, 1), Font("Verdana", fontSize));
	gl::drawString(info.str(), vec2(infoLeft, infoTop), ColorA(1, 1, 1, 1), Font("Verdana", fontSize));

	gl::disableAlphaBlending();

	// -------- CINDER --------
	if (params != NULL) params->draw();
}

void SpinnakerSpoutApp::initParamInterface() {
	params = params::InterfaceGl::create(getWindow(), "Camera parameters", toPixels(ivec2(200, 300)));

	params->addParam("Send Width", &sendWidth).min(20).updateFn([this] {
		UserSettings::writeSetting<int>("sendWidth", sendWidth);
	});

	params->addParam("Send Height", &sendHeight).min(20).updateFn([this] {
		UserSettings::writeSetting<int>("sendHeight", sendHeight);
	});

	std::vector<string> binningEnums;
	binningEnums.push_back("No binning");
	binningEnums.push_back("Binning 2x");
	binningEnumIndex = binning - 1;
	params->addParam("Binning", binningEnums, &binningEnumIndex).updateFn([this] {
		binning = binningEnumIndex + 1;
		UserSettings::writeSetting<int>("binning", binning);
	});

	vector<string> availablePixelFormats = SpinnakerDeviceCommunication::getParameterEnumOptions(camera, "PixelFormat");
	pixelFormatEnumIndex = find(availablePixelFormats.begin(), availablePixelFormats.end(), pixelFormat) - availablePixelFormats.begin();
	params->addParam("Pixel Format", availablePixelFormats, &pixelFormatEnumIndex).updateFn([this, availablePixelFormats] {
		pixelFormat = availablePixelFormats[pixelFormatEnumIndex];
		UserSettings::writeSetting<string>("pixelFormat", pixelFormat);
	});
}

// Stops camera if necessary. Make sure to check whether it is started after calling this method.
// returns true if camera was stopped, false otherwise
bool SpinnakerSpoutApp::applyParamsIfNeeded() {
	bool cameraWasStopped = false;

	if (sendFbo == NULL || sendFbo->getWidth() != sendWidth || sendFbo->getHeight() != sendHeight || !spoutInitialized) {
		initSpout();
	}

	if (camera != NULL) {
		int currentBinning = SpinnakerDeviceCommunication::getParameterIntValue(camera, "BinningHorizontal");
		if (currentBinning != binning) {
			cameraWasStopped = stopCamera();
			SpinnakerDeviceCommunication::setParameterInt(camera, "BinningHorizontal", binning);
			SpinnakerDeviceCommunication::setParameterInt(camera, "BinningVertical", binning);
		}

		string currentPixelFormat = SpinnakerDeviceCommunication::getParameterEnumValue(camera, "PixelFormat");
		if (currentPixelFormat != pixelFormat) {
			cameraWasStopped = stopCamera();
			SpinnakerDeviceCommunication::setParameterEnum(camera, "PixelFormat", pixelFormat);
		}
	}
	return cameraWasStopped;
}

void SpinnakerSpoutApp::initSpout() {
	if (spoutInitialized) spoutSender.ReleaseSender();

	sendFbo = gl::Fbo::create(sendWidth, sendHeight);
	spoutInitialized = spoutSender.CreateSender(senderName.c_str(), sendWidth, sendHeight);

	// MemoryShareMode informs us whether Spout is initialized for texture share or for memory share
	bool memoryMode = spoutSender.GetMemoryShareMode();
	if (spoutInitialized) console() << "Spout initialized using " << (memoryMode ? "Memory" : "Texture") << " sharing at " << sendWidth << " x " << sendHeight << endl;
	else console() << "Spout initialization failed" << endl;
}

void SpinnakerSpoutApp::checkCameraAvailable() {
	if (cameraFound) return;

	if (getElapsedSeconds() - lastCameraStartCheckTime < CAMERA_AVAILABLE_CHECK_INTERVAL) return;
	lastCameraStartCheckTime = getElapsedSeconds();

	if (system == NULL) system = System::GetInstance();
	SetGenICamLogConfig("cameraLog.log");
	CameraList camList = system->GetCameras();

	unsigned int numCameras = camList.GetSize();
	if (numCameras == 0)
	{
		console() << "No cameras found, retrying in " << CAMERA_AVAILABLE_CHECK_INTERVAL << " seconds." << endl;
	}
	else {
		camera = camList.GetByIndex(0);
		try {
			console() << "Initializing camera 0 (" << camList.GetSize() << " available)..." << endl;
			camera->Init();
			//SpinnakerDeviceCommunication::printDeviceInfo(camera);
			cameraFound = true;
		}
		catch (Spinnaker::Exception &e) {
			console() << "Error initializing camera: " << e.what() << endl;
		}
	}
}

bool SpinnakerSpoutApp::startCamera() {
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
		return false;
	}
}

bool SpinnakerSpoutApp::stopCamera() {
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
	if (camera != NULL && cameraRunning) {
		camera->EndAcquisition();
		camera->DeInit();
		camera = NULL;
	}
	system->ReleaseInstance(); // Release system
	spoutSender.ReleaseSender();
}

CINDER_APP(SpinnakerSpoutApp, RendererGl, prepareSettings)
