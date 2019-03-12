#include "UserSettings.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;
using namespace std;

template<typename T> void UserSettings::writeSetting(std::string name, T value) {}

template<> void UserSettings::writeSetting<int>(std::string name, int value) {
	XmlTree doc(loadAsset("settings.xml"));
	if (!doc.hasChild("spinnakerSpoutCinder")) {
		console() << "Error: Invalid settings file" << endl;
		return;
	}
	XmlTree &spinnakerSpoutCinder = doc.getChild("spinnakerSpoutCinder");

	if (spinnakerSpoutCinder.hasChild(name)) {
		XmlTree &property = spinnakerSpoutCinder.getChild(name);
		property.setValue(value);
	}
	else {
		XmlTree newChild(name, "");
		newChild.setValue(value);
		spinnakerSpoutCinder.push_back(newChild);
	}

	doc.write(writeFile(Platform::get()->getAssetPath("settings.xml")));
}

template<> void UserSettings::writeSetting<double>(std::string name, double value) {
	XmlTree doc(loadAsset("settings.xml"));
	if (!doc.hasChild("spinnakerSpoutCinder")) {
		console() << "Error: Invalid settings file" << endl;
		return;
	}
	XmlTree &spinnakerSpoutCinder = doc.getChild("spinnakerSpoutCinder");

	if (spinnakerSpoutCinder.hasChild(name)) {
		XmlTree &property = spinnakerSpoutCinder.getChild(name);
		property.setValue(value);
	}
	else {
		XmlTree newChild(name, "");
		newChild.setValue(value);
		spinnakerSpoutCinder.push_back(newChild);
	}

	doc.write(writeFile(Platform::get()->getAssetPath("settings.xml")));
}

template<> void UserSettings::writeSetting<string>(std::string name, string value) {
	XmlTree doc(loadAsset("settings.xml"));
	if (!doc.hasChild("spinnakerSpoutCinder")) {
		console() << "Error: Invalid settings file" << endl;
		return;
	}
	XmlTree &spinnakerSpoutCinder = doc.getChild("spinnakerSpoutCinder");

	if (spinnakerSpoutCinder.hasChild(name)) {
		XmlTree &property = spinnakerSpoutCinder.getChild(name);
		property.setValue(value);
	}
	else {
		XmlTree newChild(name, "");
		newChild.setValue(value);
		spinnakerSpoutCinder.push_back(newChild);
	}

	doc.write(writeFile(Platform::get()->getAssetPath("settings.xml")));
}

template<typename T> T UserSettings::getSetting(std::string name, T defaultValue) {
	T value;
	return value;
}

template<> int UserSettings::getSetting(std::string name, int defaultValue) {
	XmlTree doc(loadAsset("settings.xml"));
	if (!doc.hasChild("spinnakerSpoutCinder")) {
		console() << "Error: Invalid settings file" << endl;
		return 0;
	}
	XmlTree &spinnakerSpoutCinder = doc.getChild("spinnakerSpoutCinder");

	if (spinnakerSpoutCinder.hasChild(name)) {
		XmlTree &property = spinnakerSpoutCinder.getChild(name);
		int value = property.getValue<int>();
		return value;
	}
	else {
		return defaultValue;
	}
}

template<> double UserSettings::getSetting(std::string name, double defaultValue) {
	XmlTree doc(loadAsset("settings.xml"));
	if (!doc.hasChild("spinnakerSpoutCinder")) {
		console() << "Error: Invalid settings file" << endl;
		return 0.0;
	}
	XmlTree &spinnakerSpoutCinder = doc.getChild("spinnakerSpoutCinder");

	if (spinnakerSpoutCinder.hasChild(name)) {
		XmlTree &property = spinnakerSpoutCinder.getChild(name);
		double value = property.getValue<double>();
		return value;
	}
	else {
		return defaultValue;
	}
}

template<> string UserSettings::getSetting(std::string name, string defaultValue) {
	XmlTree doc(loadAsset("settings.xml"));
	if (!doc.hasChild("spinnakerSpoutCinder")) {
		console() << "Error: Invalid settings file" << endl;
		return "";
	}
	XmlTree &spinnakerSpoutCinder = doc.getChild("spinnakerSpoutCinder");

	if (spinnakerSpoutCinder.hasChild(name)) {
		XmlTree &property = spinnakerSpoutCinder.getChild(name);
		string value = property.getValue<string>();
		return value;
	}
	else {
		return defaultValue;
	}
}
