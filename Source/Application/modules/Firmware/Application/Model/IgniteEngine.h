#pragma once
#include <stdint.h>
#include <mutex>
#include "Common/Os/CrossPlatform/CThread.h"
#include "Common/Modules/IModule.h"
#include "common_defs/source/IgniteMessage.h"
#include "Common/Os/CrossPlatform/CMsgQueue.h"
#include "Firmware/Application/Model/SongManager.h"
#include "Firmware/Application/Model/SceneManager.h"
#include "Firmware/Application/Model/TrackManager.h"
#include "Common/Modules/IDevice.h"
#include "Firmware/Application/Model/PatternManager.h"
#include "Firmware/Application/Model/InstrumentManager.h"
#include "Common/Modules/Subscriptions.h"
#include "Firmware/Application/Model/ModelParams.h"
#include "anytype.h"

#include "Firmware/Application/Model/EngineFileHandler.h"
#include "Firmware/Application/Model/FrontPannelManager.h"
#include "Firmware/Application/Model/SettingsManager/SettingsMidiCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsCvioCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsGeneralCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsSceneCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsSongCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsTrackCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsPatternCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsMidiClkCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsBRayCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsTransposeControlCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsMidiMachineControlCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsRecordCnfg.h"
#include "Firmware/Application/Model/SequenceManager.h"

#if !defined(__APPLE__) && !defined(_WIN32)
#define ENABLE_PARALLEL_EXE //Enable for parallel execution in Engine
#endif

#define DISABLE_USED_BY_LIST

namespace Ignite
{
	class IgniteEngine : public IDevice, public IParamChangeListener
	{
	public:
		// Serialises model edits, clock events, MIDI and desktop synthesis.
		mutable std::recursive_mutex stateMutex;
		[[nodiscard]] std::unique_lock<std::recursive_mutex> lockDesktopState() const
		{
#ifdef _WIN32
			return std::unique_lock<std::recursive_mutex>(stateMutex);
#else
			return {};
#endif
		}
		IgniteEngine();
		void shutdownDesktop();

		void initialize(SongManager* songmgr, PatternManager* patternmgr, InstrumentManager* instrumentmgr);
		bool isIDeviceReady(void) override
		{
			return isEngineReady;
		};
		void addListener(IListener* listener) override;


		void setParam(moduleId_t moduleSrc, moduleId_t moduleDst, paramId_t param, const AnyType value, bool isPlayMode = false)override;
		void setParam(moduleId_t moduleSrc, moduleId_t moduleDst, std::vector<IgniteParamList> paramlist, bool isPlayMode = false)override;
		void SetCommandParam(moduleId_t moduleSrc, moduleId_t moduleDst, paramId_t param, bool isPlayMode = false)override;
		bool SetCommand(moduleId_t moduleSrc, moduleId_t moduleDst, paramId_t param, AnyType value, bool isPlayMode = false)override;

		std::vector<IgniteParamList> getParam(moduleId_t moduleSrc, moduleId_t moduleDst, std::vector<IgniteParamList> paramlist)override;
		void getParam(moduleId_t moduleSrc, moduleId_t moduleDst, paramId_t param)override;

		void subscribe(moduleId_t module, paramId_t param, IListener* src) override;
		void unsubscribe(moduleId_t module, paramId_t param, IListener* src) override;
		bool SaveAllData() override;
		void onParameterChanged(moduleId_t moduleSrc, ParameterBase* param) override;
		void SendResponse(moduleId_t module, moduleId_t moduleSrc, paramId_t param, const AnyType& value);

		std::list<std::string> EngineGetPatternList(bool isDisplayName = false);
		ClientPatternData EngineGetPatternData(std::string patternName);
		std::list<ClienPatterntBarData>  EngineGetBarData(std::string patternName);
		std::list<ClientPatternStepData> EngineGetStepData(std::string patternName, uint16_t BarId);

		bool EngineSetPatternData(moduleId_t moduleSrc, std::string patternName, ClientPatternData patternData, bool isPlayModeData = false, PatternDataUpdateReasonType UpdateReasonType = PatternDataUpdateReasonType::PatternDataUpdateReason_PATTERNDATA);
		//bool EngineSetPatternData(moduleId_t moduleSrc, std::string patternName, ClientPatternData& patternData, PatternDataUpdateLocation location, bool isPlayModeData = false, PatternDataUpdateReasonType UpdateReasonType = PatternDataUpdateReasonType::PatternDataUpdateReason_PATTERNDATA);

		bool EngineSetBarData(moduleId_t moduleSrc, std::string patternName, uint16_t BarId, ClienPatterntBarData barData);
		bool EngineSetStepData(moduleId_t moduleSrc, std::string patternName, uint16_t BarId, uint16_t stepId, ClientPatternStepData stepData);

		bool  EngineCopyPatternBar(std::string patternName, uint16_t srcBarId, uint16_t destBarId);
		bool  EngineAddPatternBar(std::string patternName, bool CopyAndCreate, uint16_t FromBarId, uint16_t NumBarToAdd);
		bool  EngineInsertPatternBar(std::string patternName, uint16_t InsertPosition);
		bool  EngineRemovePatternBar(std::string patternName, uint16_t BarId, uint16_t NumBarToRemove);
		bool  EngineInsertPatternStep(std::string patternName, uint16_t BarId, uint8_t stepCount, uint8_t InsertAt, std::vector<ClientPatternStepData> ToStepDataList);
		bool  EngineAddNodeData(std::string patternName, uint16_t BarId, uint16_t stepId, ClientPatternNodeData nodeData);
		bool  EngineAddNodeData(std::string patternName, uint16_t BarId, std::map<ClientPatternNodeData, std::list<uint8_t>> stepMap);
		bool  EngineDelNodeData(std::string patternName, uint16_t BarId, uint16_t stepId, uint16_t nodeId);
		bool  EnginecopyPattern(std::string SrcpatternName, std::string DstpatternName);
		int8_t getSceneBarCount(void);
		int8_t getSceneGbarStepCount(void);
		bool _autoSaveStatus = false;//TODO : save in settings file .. based on UI
		void setAutoSaveAllModules(bool status);
		bool getAutoSaveStatus(void);
		std::list<PatternDataUpdateReasonType> EnginePatternGetUndoList (void);
		bool PatternMngrPatternRotate (std::string patternName, uint16_t BarID, uint16_t StepID);
		void updateModuleReadyStatus(moduleId_t moduleId, bool isReady);

		EngineFileHandler* get_fileHandler()
		{
			return fileHandler;
		}
		FrontPannelManager* get_frontPannelManager()
		{
			return frontPannelManager;
		}
		SettingsMidiCnfg* get_settingsMidiCnfg ()
		{
			return settingsMidiCnfg;
		}
		SettingsCvioCnfg* get_settingsCvioCnfg ()
		{
			return settingsCvioCnfg;
		}
		SettingsGeneralCnfg* get_settingsGeneralCnfg ()
		{
			return settingsGeneralCnfg;
		}

		SettingsSceneCnfg* get_settingsSceneCnfg()
		{
			return settingsSceneCnfg;
		}
		SettingsSongCnfg* get_settingsSongCnfg ()
		{
			return settingsSongCnfg;
		}

		SettingsRecordCnfg* get_settingsRecordCnfg ()
		{
			return settingsRecordCnfg;
		}

		SongManager* get_SongManager()
		{
			return songManager;
		}


		EngineFileHandler* fileHandler = nullptr;
		FrontPannelManager* frontPannelManager = nullptr; //for faster access
		SongManager* songManager = nullptr;
		SceneManager* sceneManager = nullptr;
		SettingsMidiCnfg* settingsMidiCnfg = nullptr;
		SettingsCvioCnfg* settingsCvioCnfg = nullptr;
		SettingsGeneralCnfg* settingsGeneralCnfg = nullptr;
		PatternManager* patternManager = nullptr;
		TrackManager* trackManager = nullptr;
		InstrumentManager* instrumentManager = nullptr;
		SettingsSceneCnfg* settingsSceneCnfg = nullptr;
		SettingsTrackCnfg* settingsTrackCnfg = nullptr;
		SettingsSongCnfg* settingsSongCnfg = nullptr;
		SettingsPatternCnfg* settingsPatternCnfg = nullptr;
		SettingsMidiClkCnfg* settingsMidiClkCnfg = nullptr;
		SettingsBRayCnfg* settingsBrayCnfg = nullptr;
		SettingsTransposeControlCnfg* settingsTransposeControlCnfg = nullptr;
		SettingsMidiMachineControlCnfg* settingsMidiMachineControlCnfg = nullptr;
		SettingsRecordCnfg* settingsRecordCnfg = nullptr;

		SequenceManager* sequenceManager = nullptr;

	private:
		CThread engineThread;
		CMsgQueue<IgniteMessageParam> engineQueue; // remove?
		CMsgQueue<IgniteMessageParam> engineMsgQ;
		//std::vector<IListener*> engineListenerList;
		std::map<IListener*, Subscriptions> _listeners;
		std::map<moduleId_t, Module*> _modules;
		int8_t modulestatus[(moduleId_t)ModuleName::moduleMax] = { 0 };
		bool isEngineReady = false;
		void engineThreadFunc(void);
		void enginePowerOnHousekeeping (void);
	};
}
