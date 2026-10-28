#pragma once
#include "Common/Modules/IListener.h"
#include "Common/Modules/IDevice.h"
#include "Common/Os/CrossPlatform/CThread.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite {

	class WatchDogService : public juce::Thread
	{
	public:
		WatchDogService() : juce::Thread("WatchdogServiceThread")
		{

		}
		void run() override
		{
			DBG("WatchdogService Thread has started.");
			while (!threadShouldExit())
			{
				wait(100);
				if (juce::Time::getCurrentTime().toMilliseconds() - lastMainThreadPing > 5000)
				{
					std::cout << "Watchdog not serviced, stop watchdog service" << std::endl;
					this->signalThreadShouldExit();
					sleep(5);
				}
			}
			DBG("WatchdogService Thread has exited.");
		}

		void pingWatchdogThread()
		{
#if defined( __linux__ ) || defined( __linux ) || defined( linux )
			lastMainThreadPing = juce::Time::getCurrentTime().toMilliseconds();
			system(command.c_str());
#endif
		}
	private:
		int64 lastMainThreadPing = juce::Time::getCurrentTime().toMilliseconds();
		std::string command = "systemd-notify WATCHDOG=1";
	};


	class UiInterface : public IListener, private juce::Timer, public juce::Thread {
	public:
		 UiInterface(IDevice* device);
		~UiInterface() override;
	/*	void publish(moduleId_t module, paramId_t param, const uint8_t value) override;
		void publish(moduleId_t module, paramId_t param, const uint32_t value) override;
		void publish(moduleId_t module, paramId_t param, const float value) override;
		void publish(moduleId_t module, paramId_t param, const std::string& value) override;*/
		void publish(moduleId_t module, moduleId_t moduleSrc, paramId_t param, const AnyType& value, bool isResponse) override;
		void publishHandler(moduleId_t module, moduleId_t moduleSrc, paramId_t param, const AnyType& value, bool isResponse);
		void timerCallback() override;
	private:
		IDevice* engine = nullptr;
		CMsgQueue<IgniteMessageParam> enginePublishQ;
		std::unique_ptr<WatchDogService> watchdogThread;
		uint32_t lastWatchdogPing = 0;
		void run() override;
	};
}
