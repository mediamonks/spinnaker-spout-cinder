#include "SpinnakerDeviceCommunication.h"

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
		console() << paramName << " setting not available for reading as Integer..." << endl;
		return 0;
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
		console() << paramName << " setting not available for reading as Float..." << endl;
		return 0;
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
		console() << paramName << " setting not available for reading as Enumeration..." << endl;
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
		console() << paramName << " setting not available for reading as Enumeration..." << endl;
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
				console() << paramName << " set to " << enumValueName << " and is now " << currentValue << endl;
			}
			else {
				console() << paramName << " set to " << currentValue << endl;
			}
			return currentValue;
		}
		else {
			console() << "Value " << enumValueName << " not available in Enumerator parameter " << paramName << endl;
		}
	}
	else {
		console() << paramName << " parameter not available for writing as Enumeration..." << endl;
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
			console() << "Value " << newValue << " out of range for Float parameter " << paramName << ". Clipped to " << newValueInRange << endl;
		}

		node->SetValue(newValueInRange);
		double currentValue = node->GetValue();
		if (newValueInRange != currentValue) {
			console() << paramName << " set to " << newValueInRange << " and is now " << currentValue << endl;
		}
		else {
			console() << paramName << " set to " << currentValue << endl;
		}
		return currentValue;
	}
	else {
		console() << paramName << " parameter not available for writing as Float..." << endl;
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
			console() << "Value " << newValue << " out of range for Int parameter " << paramName << ". Clipped to " << newValueInRange << endl;
		}

		int valueFromMin = newValueInRange - valueMin;
		valueFromMin = increment * (int)(valueFromMin / increment);
		newValueInRange = valueMin + valueFromMin;

		node->SetValue(newValueInRange);
		int currentValue = node->GetValue();
		if (newValueInRange != currentValue) {
			console() << paramName << " set to " << newValueInRange << " and is now " << currentValue << endl;
		}
		else {
			console() << paramName << " set to " << currentValue << endl;
		}
		return currentValue;
	}
	else {
		console() << paramName << " parameter not available for writing as Integer..." << endl;
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
		console() << endl << "*** PRINTING TRANSPORT LAYER DEVICE NODEMAP ***" << endl << endl;

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
		console() << "*** PRINTING TL STREAM NODEMAP ***" << endl << endl;

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
		console() << "*** PRINTING GENICAM NODEMAP ***" << endl << endl;

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
		console() << "Error: " << e.what() << endl;
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

			console() << displayName << " (" << nodeName << ", " << interfaceTypeString << "): " << value << " (" << ss.str() << ")" << endl;
		}
		else if (interfaceType == intfIFloat || interfaceType == intfIInteger) {
			gcstring unit;
			gcstring attribute;
			node->GetProperty("Unit", unit, attribute);

			console() << displayName << " (" << nodeName << ", " << interfaceTypeString << "): " << value << " " << unit << endl;
		}
		else if (interfaceType == intfIBoolean) {
			console() << displayName << " (" << nodeName << ", " << interfaceTypeString << "): " << (std::stoi(value.c_str()) == 0 ? "Off" : "On") << endl;
		}
		else {
			console() << displayName << " (" << nodeName << ", " << interfaceTypeString << "): " << value << endl;
		}
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}

// This function retrieves and prints the display name and value of a string 
// node, limiting the number of printed characters to a maximum defined by
// MAX_CHARS macro. Level parameter determines the indentation level for the
// output.
int SpinnakerDeviceCommunication::printStringNode(CNodePtr node, unsigned int level)
{
	int result = 0;

	try
	{
		// Cast as string node
		CStringPtr ptrStringNode = static_cast<CStringPtr>(node);

		// Retrieve display name
		gcstring displayName = ptrStringNode->GetDisplayName();

		//
		// Retrieve string node value
		//
		// *** NOTES ***
		// The Spinnaker SDK includes its own wrapped string class, gcstring. As 
		// such, there is no need to import the 'string' library when using the
		// SDK. If a standard string object is preferred, simply use a c-style
		// or static cast on the gcstring object.
		//
		gcstring value = ptrStringNode->GetValue();

		// Ensure that the value length is not excessive for printing
		if (value.size() > MAX_CHARS)
		{
			value = value.substr(0, MAX_CHARS) + "...";
		}

		// Print value; 'level' determines the indentation level of output
		indent(level);
		console() << displayName << ": " << value << endl;
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}

// This function retrieves and prints the display name and value of an integer 
// node.
int SpinnakerDeviceCommunication::printIntegerNode(CNodePtr node, unsigned int level)
{
	int result = 0;

	try
	{
		// Cast node as integer node
		CIntegerPtr ptrIntegerNode = static_cast<CIntegerPtr>(node);

		// Retrieve display name
		gcstring displayName = ptrIntegerNode->GetDisplayName();

		//
		// Retrieve integer node value
		//
		// *** NOTES ***
		// Keep in mind that the data type of an integer node value is an 
		// int64_t as opposed to a standard int. While it is true that the two
		// are often interchangeable, it is recommended to use the int64_t 
		// to avoid the introduction of bugs.
		//
		// All node types except for base and port nodes include a handy 
		// ToString() method which returns a value as a gcstring. 
		//
		int64_t value = ptrIntegerNode->GetValue();

		// Print value
		indent(level);
		console() << displayName << ": " << value << endl;
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}

// This function retrieves and prints the display name and value of a float 
// node.
int SpinnakerDeviceCommunication::printFloatNode(CNodePtr node, unsigned int level)
{
	int result = 0;

	try
	{
		// Cast as float node
		CFloatPtr ptrFloatNode = static_cast<CFloatPtr>(node);

		// Retrieve display name
		gcstring displayName = ptrFloatNode->GetDisplayName();

		//
		// Retrieve float node value
		//
		// *** NOTES ***
		// Please take note that floating point numbers in the Spinnaker SDK are
		// almost always represented by the larger data type double rather than 
		// float.
		//
		double value = ptrFloatNode->GetValue();

		// Print value
		indent(level);
		console() << displayName << ": " << value << endl;
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}

// This function retrieves and prints the display name and value of a boolean,
// printing "true" for true and "false" for false rather than the corresponding
// integer value ('1' and '0', respectively).
int SpinnakerDeviceCommunication::printBooleanNode(CNodePtr node, unsigned int level)
{
	int result = 0;

	try
	{
		// Cast as boolean node
		CBooleanPtr ptrBooleanNode = static_cast<CBooleanPtr>(node);

		// Retrieve display name
		gcstring displayName = ptrBooleanNode->GetDisplayName();

		// 
		// Retrieve value as a string representation
		//
		// *** NOTES ***
		// Boolean node type values are represented by the standard bool data
		// type. The boolean ToString() method returns either a '1' or '0' as
		// a string rather than a more descriptive word like 'true' or 'false'.
		//
		gcstring value = (ptrBooleanNode->GetValue() ? "true" : "false");

		// Print value
		indent(level);
		console() << displayName << ": " << value << endl;
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}

// This function retrieves and prints the display name and tooltip of a command 
// node, limiting the number of printed characters to a macro-defined maximum.
// The tooltip is printed below because command nodes do not have an intelligible 
// value.
int SpinnakerDeviceCommunication::printCommandNode(CNodePtr node, unsigned int level)
{
	int result = 0;

	try
	{
		// Cast as command node
		CCommandPtr ptrCommandNode = static_cast<CCommandPtr>(node);

		// Retrieve display name
		gcstring displayName = ptrCommandNode->GetDisplayName();

		//
		// Retrieve tooltip
		//
		// *** NOTES ***
		// All node types have a tooltip available. Tooltips provide useful
		// information about nodes. Command nodes do not have a method to
		// retrieve values as their is no intelligible value to retrieve.
		//
		gcstring tooltip = ptrCommandNode->GetToolTip();

		// Ensure that the value length is not excessive for printing
		if (tooltip.size() > MAX_CHARS)
		{
			tooltip = tooltip.substr(0, MAX_CHARS) + "...";
		}

		// Print tooltip
		indent(level);
		console() << displayName << ": " << tooltip << endl;
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}

// This function retrieves and prints the display names of an enumeration node
// and its current entry (which is actually housed in another node unto itself).
int SpinnakerDeviceCommunication::printEnumerationNodeAndCurrentEntry(CNodePtr node, unsigned int level)
{
	int result = 0;

	try
	{
		// Cast as enumeration node
		CEnumerationPtr ptrEnumerationNode = static_cast<CEnumerationPtr>(node);

		//
		// Retrieve current entry as enumeration node
		//
		// *** NOTES ***
		// Enumeration nodes have three methods to differentiate between: first, 
		// GetIntValue() returns the integer value of the current entry node;
		// second, GetCurrentEntry() returns the entry node itself; and third, 
		// ToString() returns the symbolic of the current entry.
		//
		CEnumEntryPtr ptrEnumEntryNode = ptrEnumerationNode->GetCurrentEntry();

		// Retrieve display name
		gcstring displayName = ptrEnumerationNode->GetDisplayName();

		//
		// Retrieve current symbolic
		//
		// *** NOTES ***
		// Rather than retrieving the current entry node and then retrieving its
		// symbolic, this could have been taken care of in one step by using the
		// enumeration node's ToString() method.
		//
		gcstring currentEntrySymbolic = ptrEnumEntryNode->GetSymbolic();

		// Print current entry symbolic
		indent(level);
		console() << displayName << ": " << currentEntrySymbolic << endl;
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error: " << e.what() << endl;
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
		console() << displayName << endl;

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
			// Cast all non-category nodes as value nodes
			else if (chosenRead == VALUE)
			{
				result = result | printValueNode(ptrFeatureNode, level + 1);
			}
			// Cast all non-category nodes as actual types
			else if (chosenRead == INDIVIDUAL)
			{
				switch (ptrFeatureNode->GetPrincipalInterfaceType())
				{
				case intfIString:
					result = result | printStringNode(ptrFeatureNode, level + 1);
					break;

				case  intfIInteger:
					result = result | printIntegerNode(ptrFeatureNode, level + 1);
					break;

				case intfIFloat:
					result = result | printFloatNode(ptrFeatureNode, level + 1);
					break;

				case intfIBoolean:
					result = result | printBooleanNode(ptrFeatureNode, level + 1);
					break;

				case intfICommand:
					result = result | printCommandNode(ptrFeatureNode, level + 1);
					break;

				case intfIEnumeration:
					result = result | printEnumerationNodeAndCurrentEntry(ptrFeatureNode, level + 1);
					break;
				}
			}
		}
		console() << endl;
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error: " << e.what() << endl;
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
		console() << "   ";
	}
}


int prevCaptureWidth = 0;
int prevCaptureHeight = 0;
// returns true if camera is streaming after calling this method
bool SpinnakerDeviceCommunication::checkStreamingStarted(CameraPtr camera) {
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
	catch (Spinnaker::Exception &e) {
		console() << "Error starting camera aquisition: " << e.what() << endl;
		camera->DeInit();
		return false;
	}
}

// returns true if camera is not streaming after calling this method
bool SpinnakerDeviceCommunication::checkStreamingStopped(CameraPtr camera) {
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

bool SpinnakerDeviceCommunication::getCameraTexture(CameraPtr camera, gl::TextureRef& outputTexture) {
	try {
		// TODO: we only grab one frame at a time. So if frame rate of app is lower than camera, we are building up the frame buffer of the camera and getting slow motion output as a result. Multithread this.

		ImagePtr capturedImage = camera->GetNextImage(1000); // Note: blocks until a new frame is available, limiting the frame rate of the entire app
		if (capturedImage->IsIncomplete())
		{
			console() << "Image incomplete with image status " << capturedImage->GetImageStatus() << "..." << endl;
			capturedImage->Release();
			return false;
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

			if (outputTexture == NULL || outputTexture->getWidth() != w || outputTexture->getHeight() != h) {
				outputTexture = gl::Texture2d::create(w, h);
			}

			outputTexture->update(convertedImage->GetData(), GL_RGB, GL_UNSIGNED_BYTE, 0, w, h);
			return true;
		}
	}
	catch (Spinnaker::Exception &e)
	{
		console() << "Error capturing image: " << e.what() << endl;
	}

	return false;
}