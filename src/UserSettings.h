#pragma once

#include "cinder/Xml.h"

class UserSettings {
public:
	template<typename T> static void writeSetting(std::string name, T value);
	template<typename T> static T getSetting(std::string name, T defaultValue);

private:
	static cinder::XmlTree doc;
};
