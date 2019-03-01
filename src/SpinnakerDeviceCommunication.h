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
private:

	// Use the following enum and global constant to select whether nodes are read
	// as 'value' nodes or their individual types.
	enum readType {
		VALUE,
		INDIVIDUAL
	};

	const static readType chosenRead = VALUE;

public:
	static string getParameterEnumValue(CameraPtr camera, string paramName);
	static vector<string> getParameterEnumOptions(CameraPtr camera, string paramName);
	static int getParameterIntValue(CameraPtr camera, string paramName);

	static string setParameterEnum(CameraPtr camera, string paramName, string enumValueName);
	static int setParameterInt(CameraPtr camera, string paramName, int newValue);
	static double setParameterFloat(CameraPtr camera, string paramName, float newValue);

	static void printDeviceInfo(CameraPtr camera);
	static int printValueNode(CNodePtr node, unsigned int level);
	static int printStringNode(CNodePtr node, unsigned int level);
	static int printIntegerNode(CNodePtr node, unsigned int level);
	static int printFloatNode(CNodePtr node, unsigned int level);
	static int printBooleanNode(CNodePtr node, unsigned int level);
	static int printCommandNode(CNodePtr node, unsigned int level);
	static int printEnumerationNodeAndCurrentEntry(CNodePtr node, unsigned int level);
	static int printCategoryNodeAndAllFeatures(CNodePtr node, unsigned int level);
	static void indent(unsigned int level);
	static std::string paramTypeToString(EInterfaceType interfaceType);
	static std::string accessModeToString(GenApi::EAccessMode mode);
};