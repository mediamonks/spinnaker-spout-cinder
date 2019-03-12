#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define MAX_CHARS 35

class SpinnakerDeviceCommunication {
public:
	static string getParameterEnumValue(CameraPtr camera, string paramName);
	static vector<string> getParameterEnumOptions(CameraPtr camera, string paramName);
	static int getParameterIntValue(CameraPtr camera, string paramName);
	static double getParameterFloatValue(CameraPtr camera, string paramName);

	static string setParameterEnum(CameraPtr camera, string paramName, string enumValueName);
	static int setParameterInt(CameraPtr camera, string paramName, int newValue);
	static double setParameterFloat(CameraPtr camera, string paramName, double newValue);

	static void printDeviceInfo(CameraPtr camera);
	static int printValueNode(CNodePtr node, unsigned int level);
	static int printCategoryNodeAndAllFeatures(CNodePtr node, unsigned int level);
	static void indent(unsigned int level);
	static string paramTypeToString(EInterfaceType interfaceType);
	static string accessModeToString(GenApi::EAccessMode mode);

	static bool checkStreamingStarted(CameraPtr camera);
	static bool checkStreamingStopped(CameraPtr camera);
	static bool getCameraTexture(CameraPtr camera, gl::TextureRef& outputTexture);
};