#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "SpoutLibrary.h"
#include "..\SpoutSDK\Spout.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SpoutPool {
public:
	void sendToSpout(string name, int width, int height, gl::TextureRef &sendTexture);
	void cleanup();

private:
	map<string, SpoutSender*> spoutSenders;
	gl::FboRef sendFbo = NULL;

	SpoutSender* getSpoutSender(string name, int width, int height);
};