#include "Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

ci::ConcurrentCircularBuffer<std::string>* Log::logBuffer = new ci::ConcurrentCircularBuffer<std::string>(500);

void Log::printAll() {
	//console() << "***** printall " << logBuffer->getSize() << endl;
	while (logBuffer->getSize() > 0) {
		string message;
		logBuffer->popBack(&message);
		console() << message << endl;
	}
}

Log::~Log() {
	string message = _message.str();
	logBuffer->pushFront(message);
	//console() << "pushed " << message << endl;
}