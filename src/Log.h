#pragma once

#include <sstream>

#include "cinder/app/App.h"
#include "cinder/ConcurrentCircularBuffer.h"

class Log {
public:
	static void printAll();

	~Log();

	template<class T>Log& operator<<(const T& value) {
		_message << value;
		return *this;
	}

	Log& operator<<(std::ostream& (*func)(std::ostream&)) {
		func(_message);
		return *this;
	}

private:
	static ci::ConcurrentCircularBuffer<std::string>* logBuffer;

	std::ostringstream _message;
};
