#pragma once
#include <stdint.h>
#include <string>
#include "Common/Modules/CSong.h"
#include "Common/Modules/ISyncListener.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "Firmware/Application/Model/ModelParams.h"
#include "CSoundEngine.h"
#include "module.h"
#include "Common/Modules/Scene.h"

namespace Ignite
{
	class IgniteEngine;
	class CSong;
	class Scene;
	using namespace juce;
	class TrackManager : public Module, public ISyncListener
	{
	public:
		TrackManager(IParamChangeListener* listener, IgniteEngine& model, SongManager* songMgr);
		TrackManager(const TrackManager&) = default;
		TrackManager& operator= (const TrackManager&) = delete;
		TrackManager(TrackManager&&) = delete;
		TrackManager& operator= (TrackManager&&) = delete;

		~TrackManager() = default;

		void setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode);
		std::vector<IgniteParamList> getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		void SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam(moduleId_t moduleSrc, paramId_t param) override;
		void onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		void setSoloStateOnEditTrack(bool state);
		bool getSoloStateOnEditTrack(void);
		void setRecordStateOnEditTrack(bool state);
		bool getRecordStateOnEditTrack(void);
		bool IsPatternAssigned (bool isFillPattern, std::string PatternName, std::string SongName, std::string SceneName, std::string TrackName);
		bool IsInstrumentAssigned (std::string instrumentName, std::string SongName, std::string SceneName, std::string TrackName);
		void saveTrack ();
        void resetDesktopHistory(){ClearUndoList();SetDefaultUndoListData();isUndilistNeedUpdate=false;}

	private:
		IgniteEngine& engine;
		SongManager* SongMgr;
		IParamChangeListener* paramlistener;
		volatile bool isUndilistNeedUpdate = true;

		void SetDefaultUndoListData();
		void setSongEditTrack(uint8_t trackId);
		void setTrackRxInstrument(std::string name);
		void setTrackTxInstrument(std::string name);
		void setTrackMainPattern(std::string name);
		void setSongTrackMuteStatus(uint8_t val);
		void setSceneTrackName(std::string name);
		void setTrackFillPattern(std::string name);
		void swapTrack(int8_t otherTrackId);
		void SaveData();
	};
}

