#include "SpoutPool.h"
#include "Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void SpoutPool::sendToSpout(string name, int width, int height, gl::TextureRef &sendTexture) {
	auto sender = getSpoutSender(name, width, height);

	gl::ScopedFramebuffer frameBufferScope(sendFbo);
	gl::ScopedViewport viewPortScope(sendFbo->getSize());
	gl::ScopedMatrices matrixScope;
	gl::setMatricesWindow(sendFbo->getSize(), false);

	gl::draw(sendTexture, sendFbo->getBounds());

	// Send the texture for all receivers to use
	// NOTE : if SendTexture is called with a framebuffer object bound,
	// include the FBO id as an argument so that the binding is restored afterwards
	// because Spout uses an fbo for intermediate rendering
	auto tex = sendFbo->getColorTexture();
	sender->SendTexture(tex->getId(), tex->getTarget(), tex->getWidth(), tex->getHeight());
}

int prevSendWidth = 0;
int prevSendHeight = 0;
SpoutSender* SpoutPool::getSpoutSender(string name, int width, int height) { // also makes sure send fbo is initialized
	if (prevSendWidth != width || prevSendHeight != height)
	{
		for (auto senderKv : spoutSenders) {
			auto name = senderKv.first;
			auto sender = senderKv.second;
			sender->UpdateSender(name.c_str(), width, height);
		}
		prevSendWidth = width;
		prevSendHeight = height;
		sendFbo = gl::Fbo::create(width, height);
	}

	if (spoutSenders.count(name) <= 0) {
		spoutSenders[name] = new SpoutSender();
		bool success = spoutSenders[name]->CreateSender(name.c_str(), width, height);
		bool memoryMode = spoutSenders[name]->GetMemoryShareMode();

		if (success) Log() << "Spout sender " << name << " initialized using " << (memoryMode ? "Memory" : "Texture") << " sharing at " << width << " x " << height;
		else Log() << "Spout initialization failed";
	}

	return spoutSenders[name];
}

void SpoutPool::cleanup() {
	for (auto senderKv : spoutSenders) {
		auto sender = senderKv.second;
		sender->ReleaseSender();
		delete sender;
	}
}