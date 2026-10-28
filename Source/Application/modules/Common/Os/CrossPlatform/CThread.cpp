#include "CThread.h"
using namespace Ignite;

CThread::CThread (const char* name, uint16_t stacksizebytes, ThreadPriority priority)
{
}

CThread::~CThread()
{
	stopThread();
}

void* CThread::startThread (ThreadFunction func, void* context)
{
	if (!_started)
	{
		_thread = std::thread (func, context);
		_started = true;
		// give this thread opportunity to run
		std::this_thread::sleep_for (std::chrono::milliseconds (10));
	}
	return this;
}

void CThread::yield (void)
{
	std::this_thread::yield();
}

void CThread::delayMs (const uint32_t DelayMs)
{
	std::this_thread::sleep_for (std::chrono::milliseconds (DelayMs));
}

void CThread::delayUntil (const uint32_t DelayMs)
{
	auto timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds (DelayMs);
	std::this_thread::sleep_until (timePoint);
}

void CThread::stopThread()
{
	if (_thread.joinable())
		_thread.join();
}

