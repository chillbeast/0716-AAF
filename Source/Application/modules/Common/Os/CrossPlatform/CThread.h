#pragma once
#include "Common/Os/IThread.h"
#include <chrono>
#include <thread>
#include <atomic>
namespace Ignite {

	class CThread {
		public:
			CThread(const char* name, uint16_t stacksizebytes = 512, ThreadPriority priority = osPriorityNormal);
			~CThread();

			void* startThread(ThreadFunction func, void* context);
			void yield(void);
			void delayMs(const uint32_t DelayMs);
			void delayUntil(const uint32_t DelayMs);
			void stopThread();
			static void requestDesktopShutdown() { desktopShutdown = true; }
			static bool desktopShutdownRequested() { return desktopShutdown.load(); }
		private:
			inline static std::atomic<bool> desktopShutdown{false};
			std::thread _thread;
			bool _started = false;
	};
}
