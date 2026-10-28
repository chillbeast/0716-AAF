#pragma once

#include "CHardwareTimer.h"
#include "IDevice.h"
#include "../modules/Firmware/Application/Model/ModelParams.h"
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>

namespace Ignite
{
	class CMidiClock
	{
	public:
		static CMidiClock& getInstance ()
		{
			static CMidiClock instance;
			return instance;
		}
		void initialize (IgniteEngine* eng);
		void setTempo (float bpm);
		float getTempo();
		
		void pause ();
		void start ();
		void stop ();
		void shutdownDesktop() { _synctimer->shutdownDesktop(); }

		bool isPlaying ();

	private:
		CMidiClock ();
		~CMidiClock () = default;
		CMidiClock (const CMidiClock&) = delete;
		CMidiClock& operator=(const CMidiClock&) = delete;
		void inline syncTimerCb (void);

		const uint16_t ppqn = SET_PPQN;
		const float _minTempo = 20.0f;
		const float _maxTempo = 360.0f;
		
		float _tempo = 60.0f;
		
		uint16_t _timerIntervalUs = 0;
		uint16_t _pulse = 0;
		
		bool _playing = false;
		bool _pause = false;
		
		IgniteEngine* _engine = nullptr;
		std::unique_ptr<CHardwareTimer> _synctimer;
		std::mutex inProviderUpdate;
	};
}
