#pragma once
#include "Common/Modules/Pattern.h"
#include "Firmware/Application/Model/ModelParams.h"
#include <stdint.h>
#include <string>
#include <unordered_set>
#include "module.h"
#include "common_defs/source/EngineDataStruct.h"



namespace Ignite
{
	class IgniteEngine;
	struct PatternBarStepInfo;
	struct PatternBarInfo;
	struct PatternInfo;

	struct PatternChangeStatus
	{
		bool barCount;
		bool direction;
		bool lastStepId;
		bool timebase;
		bool firstStepId;
		bool barLoopOn;
		bool startBarId;
		bool endBarId;
	};

	struct PatternChangeValue
	{
		uint8_t barCount;
		PatternDirection direction;
		uint16_t lastStepId;
		TimeBase timebase;
		uint16_t firstStepId;
		bool barLoopOn;
		uint8_t startBarId;
		uint8_t endBarId;
	};

	struct PatternDataUpdateStatus
	{
		PatternChangeStatus changedStatus;
		PatternChangeValue value;
	};

	class StepChangeListener
	{
	public:
		StepChangeListener() {}
		virtual ~StepChangeListener() {}

		virtual void handleStepSkipChange(bool skip, PatternBarStepInfo stepAncestory)=0;
		virtual void handleStepTieChange(bool tie, PatternBarStepInfo stepAncestory)=0;
		virtual void handleStepTransposeDefeatChange(bool transposeDefeat, PatternBarStepInfo stepAncestory)=0;
		virtual void handleStepGateChange(bool stepGate, PatternBarStepInfo stepAncestory)=0;
		virtual void handleStepNoteChange(PatternBarStepInfo stepAncestory) = 0;
		virtual void handleStepAuxChange(PatternBarStepInfo stepAncestory) = 0;
	};

	class BarChangeListener
	{
	public:
		BarChangeListener() {}
		virtual ~BarChangeListener() {}

		virtual void handleBarRepeatChange(PatternBarInfo barAncestory) = 0;
		virtual void handleBarGbarSyncChange(PatternBarInfo barAncestory) = 0;
		virtual void handleBarTransposeDefeatChange(bool transposeDefeat, PatternBarInfo barAncestory) = 0;
		//virtual void handleBarTransposeChange(uint8_t octave, PatternBarInfo barAncestory) = 0;
		virtual void handleBarStepCountChange(uint8_t count, PatternBarInfo barAncestory) = 0;
	};

	class PatternChangeListener
	{
	public:
		PatternChangeListener() {}
		virtual ~PatternChangeListener() {}

		virtual void handleBarCountChange(uint8_t count, std::string patternName) = 0;
		virtual void handleDirectionChange(PatternDirection direction, std::string patternName) = 0;
		virtual void handleLastStepChange(uint16_t stepId, std::string patternName) = 0;
		virtual void handleTimeBaseChange(TimeBase tb, std::string patternName) = 0;
		virtual void handleFirstStepChange(uint16_t stepId, std::string patternName) = 0;
		virtual void handleBarLoopChange(bool state, std::string patternName) = 0;
		virtual void handleBarLoopStartChange(uint8_t barId, std::string patternName) = 0;
		virtual void handleBarLoopEndChange(uint8_t barId, std::string patternName) = 0;
	};

	class PatternManager : public Module
	{
	public:
		//PatternManager(IgniteEngine& model);
		PatternManager(IParamChangeListener* listener, IgniteEngine& model);
		PatternManager(const PatternManager&) = delete;
		PatternManager& operator= (const PatternManager&) = delete;
		PatternManager(PatternManager&&) = delete;
		PatternManager& operator= (PatternManager&&) = delete;
		~PatternManager() = default;

		void setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
		{}
		void SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode)override;
		void onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification, bool isPlayMode);
		std::vector<IgniteParamList> getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
		{
			std::vector<IgniteParamList> igniteParamList;
			return igniteParamList;
		}
		void getParam(moduleId_t moduleSrc, paramId_t param)
		{}

		std::list<std::string> PatternMngrGetPatterList(bool isDisplayName = false);
		ClientPatternData PatternMngrGetPatternData(std::string patternName);
		std::list<ClienPatterntBarData>  PatternMngrGetBarData(std::string patternName);
		std::list<ClientPatternStepData> PatternMngrGetStepData(std::string patternName, uint16_t BarId);
		bool PatternMngrSetPatternDataOptimized (moduleId_t moduleSrc, std::string patternName, ClientPatternData* patternData, PatternDataUpdateReasonType UpdateReasonType = PatternDataUpdateReasonType::PatternDataUpdateReason_PATTERNDATA, bool undoCmd = false);
		//bool PatternMngrSetPatternDataOptimized (moduleId_t moduleSrc, std::string patternName, ClientPatternData& patternData, PatternDataUpdateLocation location, PatternDataUpdateReasonType UpdateReasonType = PatternDataUpdateReasonType::PatternDataUpdateReason_PATTERNDATA, bool undoCmd = false);
		bool PatternMngrSetPatternData(moduleId_t moduleSrc, std::string patternName, ClientPatternData&& clientPatternData, PatternDataUpdateReasonType UpdateReasonType = PatternDataUpdateReasonType::PatternDataUpdateReason_PATTERNDATA, bool undoCmd = false);
		bool PatternMngrSetBarData(moduleId_t moduleSrc, std::string patternName, uint16_t BarId, ClienPatterntBarData barData, bool undoCmd = false);
		bool PatternMngrSetStepData(moduleId_t moduleSrc, std::string patternName, uint16_t BarId, uint16_t stepId, ClientPatternStepData stepData, bool undoCmd = false);
		bool PatternMngrAddNodeData(std::string patternName, uint16_t BarId, uint16_t stepId, ClientPatternNodeData nodeData, bool undoCmd);
		bool PatternMngrAddNodeData(std::string patternName, uint16_t BarId, std::map<ClientPatternNodeData, std::list<uint8_t>> stepMap, bool undoCmd);
		bool PatternCopyBar(std::string patternName, uint16_t srcBarId, uint16_t destBarId);
		bool PatternAddBar(std::string patternName, bool CopyAndCreate, uint16_t FromBarId, uint16_t NumBarToAdd);
		bool patternInsertBar(std::string patternName, uint16_t postion);
		bool PatternRemoveBar(std::string patternName, uint16_t BarId, uint16_t NumBarToRemove);
		bool PatternInsertStep(std::string patternName, uint16_t BarId, uint8_t stepCount, uint8_t InsertAt, std::list<ClientPatternStepData> ToStepDataList);
		bool PatternMngrDelNodeData(std::string patternName, uint16_t BarId, uint16_t stepId, uint16_t nodeData, bool undoCmd);
		Pattern* GetPattern(std::string patternName);
		bool SavePattern(Pattern* pattern, bool isCreate);
		void desktopMarkSaved();
		void modifyPatternNameIfDuplicate(std::string& name);
		bool copyPattern(std::string SrcpatternName, std::string DstpatternName);
		void movePattern(uint64_t positionData);
		int32_t getPatternId(std::string patternName);
		void sortPatternsById(void);
		void reAssignSequentialIdsForPatternsAndSave(void);
		void savePatternInfo(std::string name, PatternData& data);
		std::list<PatternDataUpdateReasonType> PatternGetUndoList (void);
		bool PatternRotate (std::string patternName, uint16_t BarID, uint16_t StepID);
		void clearAllPattern ();
		void PatternMngrPowerOnHousekeeping ();

		std::string addPattern (void);
        std::string addDesktopPattern(ClientPatternData data);
		std::string addPatternWithRandomData (uint8_t numBars);
		Pattern* GetPatternFromIndex (int32_t index);
		IgniteEngine& engine;
		std::list<Ignite::Pattern> patternList;
		int32_t getPatternCount (void);

		void addStepListener (StepChangeListener* listener);
		void addBarListener(BarChangeListener* listener);
		void addPatternListener(PatternChangeListener* listener);

		uint8_t mapTbToUiIndex(TimeBase tb);
		TimeBase mapUiIndexToTb(uint8_t index);

		float getTickScalingFactor(TimeBase old, TimeBase next);
		void updateLengthAndOffsetOnTbChange(ClientPatternData* clientPatternData, TimeBase old, TimeBase next);

	private:
		std::future<bool> fut_delayedLoad;
		std::thread ThreadFsLoad;
		bool is_initCompleted = false;
		std::list <std::string> patternNameList;
		void LoadPatternFromFilesystem ();
		bool isInitCompleted ();

		struct PatternUndoData
		{
			PatternDataUpdateReasonType UpdateReasonType = PatternDataUpdateReasonType::PatternDataUpdateReason_UNKNOWN;
			ClientPatternData _clientPatternData;
		};
		std::list<PatternUndoData> UndoDataList;
		IParamChangeListener* paramlistener;

		std::string editPatternName;
		std::string playPatternName;
		int32_t numberOfPatterns;

		std::unordered_set <StepChangeListener*> _stepListeners;
		std::unordered_set <BarChangeListener*> _barListeners;
		std::unordered_set <PatternChangeListener*> _patternListeners;

		void removePattern(std::string patternName);
		void UpdateUndoList(std::string patternName, PatternDataUpdateReasonType _dataUpdateReasonType, ClientPatternData *clientPatternData = nullptr);
		PatternDataUpdateReasonType PatternUndo (bool resetToDefault = false);
		void UpdatePattern(Pattern* pattern, ClientPatternData _clientPatternData);
		Pattern CreatePattern (ClientPatternData _clientPatternData);
		void SaveData();
	};
}

