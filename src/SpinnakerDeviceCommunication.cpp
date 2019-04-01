#include "SpinnakerDeviceCommunication.h"
#include "Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

int SpinnakerDeviceCommunication::getParameterIntValue(CameraPtr camera, string paramName) {
	INodeMap& nodeMap = camera->GetNodeMap();
	CIntegerPtr node = nodeMap.GetNode(paramName.c_str());
	if (IsAvailable(node) && IsReadable(node))
	{
		return node->GetValue();
	}
	else {
		Log() << " > " << paramName << " setting not available for reading as Integer.";
		return 0;
	}
}

pair<int, int> SpinnakerDeviceCommunication::getIntParameterMinMax(CameraPtr camera, string paramName) {
	INodeMap& nodeMap = camera->GetNodeMap();
	CIntegerPtr node = nodeMap.GetNode(paramName.c_str());
	if (IsAvailable(node) && IsReadable(node))
	{
		return make_pair(node->GetMin(), node->GetMax());
	}
	else {
		Log() << " > " << paramName << " setting not available as Integer for getting range.";
		return make_pair(0, 0);
	}
}

double SpinnakerDeviceCommunication::getParameterFloatValue(CameraPtr camera, string paramName) {
	INodeMap& nodeMap = camera->GetNodeMap();
	CFloatPtr node = nodeMap.GetNode(paramName.c_str());
	if (IsAvailable(node) && IsReadable(node))
	{
		return node->GetValue();
	}
	else {
		Log() << " > " << paramName << " setting not available for reading as Float.";
		return 0;
	}
}

pair<double, double> SpinnakerDeviceCommunication::getFloatParameterMinMax(CameraPtr camera, string paramName) {
	INodeMap& nodeMap = camera->GetNodeMap();
	CFloatPtr node = nodeMap.GetNode(paramName.c_str());
	if (IsAvailable(node) && IsReadable(node))
	{
		return make_pair(node->GetMin(), node->GetMax());
	}
	else {
		Log() << " > " << paramName << " setting not available as Float for getting range.";
		return make_pair(0.0, 0.0);
	}
}

string SpinnakerDeviceCommunication::getParameterEnumValue(CameraPtr camera, string paramName) {
	INodeMap& nodeMap = camera->GetNodeMap();
	CEnumerationPtr node = nodeMap.GetNode(paramName.c_str());
	if (IsAvailable(node) && IsReadable(node))
	{
		int intValue = node->GetIntValue();
		CEnumEntryPtr entry = node->GetEntry(intValue);
		return entry->GetSymbolic();
	}
	else {
		Log() << " > "  << paramName << " setting not available for reading as Enumeration.";
		return "";
	}
}

vector<string> SpinnakerDeviceCommunication::getParameterEnumOptions(CameraPtr camera, string paramName) {
	INodeMap& nodeMap = camera->GetNodeMap();
	CEnumerationPtr node = nodeMap.GetNode(paramName.c_str());
	vector<string> entryStrings;
	if (IsAvailable(node) && IsReadable(node))
	{
		NodeList_t entries;
		node->GetEntries(entries);

		for (CEnumEntryPtr entry : entries) {
			entryStrings.push_back((string)entry->GetSymbolic());
		}
		return entryStrings;
	}
	else {
		Log() << " > "  << paramName << " setting not available for reading as Enumeration.";
		return entryStrings;
	}
}

string SpinnakerDeviceCommunication::setParameterEnum(CameraPtr camera, string paramName, string enumValueName) {
	INodeMap& nodeMap = camera->GetNodeMap();

	CEnumerationPtr node = nodeMap.GetNode(paramName.c_str());
	if (IsAvailable(node) && IsWritable(node))
	{
		CEnumEntryPtr entry = node->GetEntryByName(enumValueName.c_str()); // Retrieve the desired entry node from the enumeration node
		if (IsAvailable(entry) && IsReadable(entry))
		{
			int64_t value = entry->GetValue(); // Retrieve the integer value from the entry node
			node->SetIntValue(value); // Set integer as new value for enumeration node
			string currentValue = node->GetCurrentEntry()->GetSymbolic();
			if (enumValueName != currentValue) {
				Log() << " > "  << paramName << " set to " << enumValueName << " and is now " << currentValue;
			}
			else {
				Log() << " > "  << paramName << " set to " << currentValue;
			}
			return currentValue;
		}
		else {
			Log() << "> Value " << enumValueName << " not available in Enumerator parameter " << paramName;
		}
	}
	else {
		Log() << " > "  << paramName << " parameter not available for writing as Enumeration.";
	}
	return "error";
}

double SpinnakerDeviceCommunication::setParameterFloat(CameraPtr camera, string paramName, double newValue) {
	INodeMap& nodeMap = camera->GetNodeMap();
	CFloatPtr node = nodeMap.GetNode(paramName.c_str());
	if (IsAvailable(node) && IsWritable(node))
	{
		double valueMax = node->GetMax();
		double valueMin = node->GetMin();
		double newValueInRange = newValue;
		if (newValueInRange < valueMin || newValueInRange > valueMax)
		{
			if (newValueInRange < valueMin) newValueInRange = valueMin;
			if (newValueInRange > valueMax) newValueInRange = valueMax;
			Log() << "> Value " << newValue << " out of range for Float parameter " << paramName << ". Clipped to " << newValueInRange;
		}

		node->SetValue(newValueInRange);
		double currentValue = node->GetValue();
		if (math<float>::abs(newValueInRange - currentValue) > 0.00001) {
			Log() << " > "  << paramName << " set to " << newValueInRange << " and is now " << currentValue;
		}
		else {
			Log() << " > "  << paramName << " set to " << currentValue << endl;
		}
		return currentValue;
	}
	else {
		Log() << " > "  << paramName << " parameter not available for writing as Float.";
	}
	return -1;
}

int SpinnakerDeviceCommunication::setParameterInt(CameraPtr camera, string paramName, int newValue) {
	INodeMap& nodeMap = camera->GetNodeMap();
	CIntegerPtr node = nodeMap.GetNode(paramName.c_str());
	if (IsAvailable(node) && IsWritable(node))
	{
		int valueMax = node->GetMax();
		int valueMin = node->GetMin();
		int increment = node->GetInc();

		int newValueInRange = newValue;
		if (newValueInRange < valueMin || newValueInRange > valueMax)
		{
			if (newValueInRange < valueMin) newValueInRange = valueMin;
			if (newValueInRange > valueMax) newValueInRange = valueMax;
			Log() << "> Value " << newValue << " out of range for Int parameter " << paramName << ". Clipped to " << newValueInRange;
		}

		int valueFromMin = newValueInRange - valueMin;
		valueFromMin = increment * (int)(valueFromMin / increment);
		newValueInRange = valueMin + valueFromMin;

		node->SetValue(newValueInRange);
		int currentValue = node->GetValue();
		if (newValueInRange != currentValue) {
			Log() << " > "  << paramName << " set to " << newValueInRange << " and is now " << currentValue;
		}
		else {
			Log() << " > "  << paramName << " set to " << currentValue << endl;
		}
		return currentValue;
	}
	else {
		Log() << " > "  << paramName << " parameter not available for writing as Integer.";
	}
	return -1;
}

// assumes camera is initialized
void SpinnakerDeviceCommunication::printDeviceInfo(CameraPtr camera)
{
	unsigned int level = 0;

	try
	{
		//
		// Retrieve TL device nodemap
		//
		// *** NOTES ***
		// The TL device nodemap is available on the transport layer. As such, 
		// camera initialization is unnecessary. It provides mostly immutable
		// information fundamental to the camera such as the serial number,
		// vendor, and model.
		//
		Log() << endl << "*** PRINTING TRANSPORT LAYER DEVICE NODEMAP ***" << endl;

		INodeMap & genTLNodeMap = camera->GetTLDeviceNodeMap();

		printCategoryNodeAndAllFeatures(genTLNodeMap.GetNode("Root"), level);

		//
		// Retrieve TL stream nodemap
		//
		// *** NOTES ***
		// The TL stream nodemap is also available on the transport layer. Camera
		// initialization is again unnecessary. As you can probably guess, it
		// provides information on the camera's streaming performance at any
		// given moment. Having this information available on the transport
		// layer allows the information to be retrieved without affecting camera
		// performance.
		//
		Log() << "*** PRINTING TL STREAM NODEMAP ***" << endl;

		INodeMap & nodeMapTLStream = camera->GetTLStreamNodeMap();

		printCategoryNodeAndAllFeatures(nodeMapTLStream.GetNode("Root"), level);

		//
		// Initialize camera
		//
		// *** NOTES ***
		// The camera becomes connected upon initialization. This provides
		// access to configurable options and additional information, accessible
		// through the GenICam nodemap.
		//
		// *** LATER ***
		// Cameras should be deinitialized when no longer needed.
		//
		Log() << "*** PRINTING GENICAM NODEMAP ***" << endl;

		// 
		// Retrieve GenICam nodemap
		//
		// *** NOTES ***
		// The GenICam nodemap is the primary gateway to customizing
		// and configuring the camera to suit your needs. Configuration options 
		// such as image height and width, trigger mode enabling and disabling,
		// and the sequencer are found on this nodemap.
		//
		INodeMap & appLayerNodeMap = camera->GetNodeMap();

		printCategoryNodeAndAllFeatures(appLayerNodeMap.GetNode("Root"), level);
	}
	catch (Spinnaker::Exception &e)
	{
		Log() << "Error: " << e.what();
	}
}

// This function retrieves and prints the display name and value of all node
// types as value nodes. A value node is a general node type that allows for
// the reading and writing of any node type as a string.
int SpinnakerDeviceCommunication::printValueNode(CNodePtr node, unsigned int level)
{
	int result = 0;

	try
	{
		// Cast as value node
		CValuePtr ptrValueNode = static_cast<CValuePtr>(node);

		//
		// Retrieve display name
		//
		// *** NOTES ***
		// A node's 'display name' is generally more appropriate for output and
		// user interaction whereas its 'name' is what the camera understands.
		// Generally, its name is the same as its display name but without
		// spaces - for instance, the name of the node that houses a camera's
		// serial number is 'DeviceSerialNumber' while its display name is
		// 'Device Serial Number'.
		//
		gcstring displayName = node->GetDisplayName();
		gcstring nodeName = node->GetName();
		EInterfaceType interfaceType = node->GetPrincipalInterfaceType();
		string interfaceTypeString = paramTypeToString(interfaceType);

		//
		// Retrieve value of any node type as string
		//
		// *** NOTES ***
		// Because value nodes return any node type as a string, it can be much 
		// easier to deal with nodes as value nodes rather than their actual
		// individual types.
		//
		gcstring value = ptrValueNode->ToString();

		// Ensure that the value length is not excessive for printing
		if (value.size() > MAX_CHARS)
		{
			value = value.substr(0, MAX_CHARS) + "...";
		}

		// Print value
		indent(level);
		if (interfaceType == intfIEnumeration) {
			vector<string> entryStrings;

			NodeList_t entries;
			CEnumerationPtr ptrEnumValueNode = static_cast<CEnumerationPtr>(ptrValueNode);
			ptrEnumValueNode->GetEntries(entries);

			for (CEnumEntryPtr entry : entries) {
				entryStrings.push_back((string)entry->GetSymbolic());
			}

			stringstream ss;
			int i = 0;
			for (string entryString : entryStrings) {
				ss << entryString << (i < entryStrings.size() - 1 ? ", " : "");
				i++;
			}

			Log() << displayName << " (" << nodeName << ", " << interfaceTypeString << "): " << value << " (" << ss.str() << ")";
		}
		else if (interfaceType == intfIFloat || interfaceType == intfIInteger) {
			gcstring unit;
			gcstring attribute;
			node->GetProperty("Unit", unit, attribute);

			Log() << displayName << " (" << nodeName << ", " << interfaceTypeString << "): " << value << " " << unit;
		}
		else if (interfaceType == intfIBoolean) {
			Log() << displayName << " (" << nodeName << ", " << interfaceTypeString << "): " << (std::stoi(value.c_str()) == 0 ? "Off" : "On");
		}
		else {
			Log() << displayName << " (" << nodeName << ", " << interfaceTypeString << "): " << value;
		}
	}
	catch (Spinnaker::Exception &e)
	{
		Log() << "Error: " << e.what();
		result = -1;
	}

	return result;
}

// This function retrieves and prints out the display name of a category node 
// before printing all child nodes. Child nodes that are also category nodes are
// printed recursively.
int SpinnakerDeviceCommunication::printCategoryNodeAndAllFeatures(CNodePtr node, unsigned int level)
{
	int result = 0;

	try
	{
		// Cast as category node
		CCategoryPtr ptrCategoryNode = static_cast<CCategoryPtr>(node);

		// Retrieve display name
		gcstring displayName = ptrCategoryNode->GetDisplayName();

		// Print display name
		indent(level);
		Log() << displayName;

		//
		// Retrieve children
		//
		// *** NOTES ***
		// The two nodes that typically have children are category nodes and
		// enumeration nodes. Throughout the examples, the children of category
		// nodes are referred to as features while the children of enumeration
		// nodes are referred to as entries. Keep in mind that enumeration
		// nodes can be cast as category nodes, but category nodes cannot be 
		// cast as enumerations.
		//
		FeatureList_t features;
		ptrCategoryNode->GetFeatures(features);

		//
		// Iterate through all children
		// 
		// *** NOTES ***
		// If dealing with a variety of node types and their values, it may be
		// simpler to cast them as value nodes rather than as their individual
		// types. However, with this increased ease-of-use, functionality is
		// sacrificed.
		//
		FeatureList_t::const_iterator it;
		for (it = features.begin(); it != features.end(); ++it)
		{
			CNodePtr ptrFeatureNode = *it;

			// Ensure node is available and readable
			if (!IsAvailable(ptrFeatureNode) || !IsReadable(ptrFeatureNode))
			{
				continue;
			}

			// Category nodes must be dealt with separately in order to
			// retrieve subnodes recursively.
			if (ptrFeatureNode->GetPrincipalInterfaceType() == intfICategory)
			{
				result = result | printCategoryNodeAndAllFeatures(ptrFeatureNode, level + 1);
			}
			result = result | printValueNode(ptrFeatureNode, level + 1);
		}
		Log() << endl;
	}
	catch (Spinnaker::Exception &e)
	{
		Log() << "Error: " << e.what();
		result = -1;
	}

	return result;
}

std::string SpinnakerDeviceCommunication::paramTypeToString(EInterfaceType interfaceType) {
	switch (interfaceType) {
		case intfIString: return "String";
		case intfIInteger: return "Integer";
		case intfIFloat: return "Float";
		case intfIBoolean: return "Boolean";
		case intfICommand: return "Command";
		case intfIEnumeration: return "Enumeration";
		default: return "Unknown";
	}
}

std::string SpinnakerDeviceCommunication::accessModeToString(GenApi::EAccessMode mode){
	switch (mode) {
		case GenApi::EAccessMode::NI: return "Not implemented";
		case GenApi::EAccessMode::NA: return "Not available";
		case GenApi::EAccessMode::WO: return "Write Only";
		case GenApi::EAccessMode::RO: return "Read Only";
		case GenApi::EAccessMode::RW: return "Read and Write";
		case GenApi::EAccessMode::_UndefinedAccesMode: return "Object is not yet initialized";
		case GenApi::EAccessMode::_CycleDetectAccesMode: return "used internally for AccessMode cycle detection";
	}
}

// This helper function deals with output indentation, of which there is a lot.
void SpinnakerDeviceCommunication::indent(unsigned int level)
{
	for (unsigned int i = 0; i < level; i++)
	{
		Log() << "   ";
	}
}

// returns true if camera is streaming after calling this method
bool SpinnakerDeviceCommunication::checkStreamingStarted(CameraPtr camera) {
	if (camera->IsStreaming()) return true;

	try
	{
		SpinnakerDeviceCommunication::setParameterEnum(camera, "AcquisitionMode", "Continuous");
		Log() << "Starting camera " << camera->DeviceModelName() << " (" << camera->DeviceSerialNumber() << ")" << " with access mode " << SpinnakerDeviceCommunication::accessModeToString(camera->GetAccessMode());
		camera->BeginAcquisition();
		return true;
	}
	catch (Spinnaker::Exception &e) {
		Log() << "Error starting camera aquisition: " << e.what();
		camera->DeInit();
		return false;
	}
}

// returns true if camera is not streaming after calling this method
bool SpinnakerDeviceCommunication::checkStreamingStopped(CameraPtr camera) {
	if (!camera->IsStreaming()) return true;

	try {
		Log() << "Stopping camera " << camera->DeviceModelName() << " (" << camera->DeviceSerialNumber() << ")";
		camera->EndAcquisition();
		return true;
	}
	catch (Spinnaker::Exception &e) {
		Log() << "Error stopping camera aquisition: " << e.what();
		camera->DeInit();
		return false;
	}
}

bool SpinnakerDeviceCommunication::getCameraTexture(CameraPtr camera, gl::TextureRef& outputTexture) {
	try {
		ImagePtr capturedImage = camera->GetNextImage(1000); // Note: blocks until a new frame is available
		if (capturedImage->IsIncomplete())
		{
			//Log() << "Image incomplete with image status " << capturedImage->GetImageStatus() << endl;
			capturedImage->Release();
			return false;
		}
		else
		{
			int w = capturedImage->GetWidth();
			int h = capturedImage->GetHeight();

			ImagePtr convertedImage = capturedImage->Convert(PixelFormat_RGB8, NEAREST_NEIGHBOR); // Note that color processing algorithms other than NEAREST_NEIGHBOR are probably too slow for continous acquisition at high resolutions.
			capturedImage->Release();

			if (outputTexture == NULL || outputTexture->getWidth() != w || outputTexture->getHeight() != h) {
				outputTexture = gl::Texture2d::create(w, h);
			}

			outputTexture->update(convertedImage->GetData(), GL_RGB, GL_UNSIGNED_BYTE, 0, w, h);
			return true;
		}
	}
	catch (Spinnaker::Exception &e)
	{
		Log() << "Error capturing image: " << e.what();
	}

	return false;
}

vector<string> SpinnakerDeviceCommunication::getLogLevelStrings() {
	std::vector<string> strings;
	strings.push_back("Off");
	strings.push_back("Fatal");
	strings.push_back("Alert");
	strings.push_back("Crit");
	strings.push_back("Error");
	strings.push_back("Warn");
	strings.push_back("Notice");
	strings.push_back("Info");
	strings.push_back("Debug");
	strings.push_back("All");
	return strings;
}


SpinnakerLogLevel SpinnakerDeviceCommunication::indexToLogLevel(int index) {
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
	default: return LOG_LEVEL_OFF;
	}
}