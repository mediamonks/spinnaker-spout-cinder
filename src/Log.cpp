#include "Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

ci::ConcurrentCircularBuffer<std::string>* Log::logBuffer = new ci::ConcurrentCircularBuffer<std::string>(500);
std::thread::id Log::mainThreadId = std::thread::id();

void Log::markMainThread() {
	mainThreadId = std::this_thread::get_id();
}

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
	if (std::this_thread::get_id() != mainThreadId) {
		logBuffer->pushFront(message);
	}
	else {
		console() << message << endl;
	}
	//console() << "pushed " << message << endl;
}