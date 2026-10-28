#pragma once
#include <stdint.h>
#include <list>
#include <string>
#include <vector>
#include <map>
#include "Common/Modules/Step.h"
#include "Firmware/Application/Model/ModelParams.h"
#include "common_defs/source/EngineDataStruct.h"

namespace Ignite
{
	class Step;
	struct StepDataUpdateStatus;

	struct BarData
	{
		uint8_t BarId = 0;
		uint16_t stepCount = 0;
		PatternDirection direction = PatternDirection::forward;
		uint8_t  barRepeatCount = 0;
		bool  barTransposeDefeat = false;
		bool gBarSyncOn = false;
		std::multimap <uint16_t, Step> stepList;
	};

	struct BarChangeStatus
	{
		bool stepCount;
		bool  barRepeatCount;
		bool  barTransposeDefeat;
		bool gBarSyncOn;
	};

	struct BarChangeValue
	{
		uint16_t stepCount;
		uint8_t  barRepeatCount;
		bool  barTransposeDefeat;
		bool gBarSyncOn;
	};

	struct BarDataUpdateShallowStatus
	{
		BarChangeStatus changedStatus;
		BarChangeValue value;
	};

	struct BarDataUpdateStatus
	{
		BarDataUpdateShallowStatus barData;
		std::map<int16_t, StepDataUpdateStatus> stepData;
	};


	class Bar
	{
	public:
		explicit Bar (Pattern* pat, uint16_t BarID, bool BarTransposeDefeat, uint8_t BarRepeatCount, bool gBarSync, PatternDirection direction, std::list<StepData> StepDataList);
		Bar (Pattern *pat, const Bar&);
		Bar(const Bar&);
		Bar& operator= (const Bar&);
		~Bar ();
		void rebindPattern(Pattern* pattern);

		BarData* _BarData;

		std::list<StepData>  AddStep (uint16_t BarID, uint8_t InsertAt, uint8_t NumOfStepsPerBar, std::list<StepData> StepDataList);
		std::vector <StepDataUpdateStatus> BarSetStepData (uint16_t stepId, ClientPatternStepData stepData);
		std::map <int16_t, StepDataUpdateStatus> BarSetStepDataList (std::unordered_map <uint16_t, ClientPatternStepData>* StepDataList);
		BarDataUpdateShallowStatus SetBarData (ClienPatterntBarData barData);
		bool BarUpdateStepNodeData (uint16_t stepId, ClientPatternNodeData nodeData);
		bool BarUpdateStepNodeData (std::map<ClientPatternNodeData, std::list<uint8_t>> stepMap);

		uint8_t getbarRepeatCount (void);

		bool GetStepDataList (std::unordered_map <uint16_t, ClientPatternStepData>* StepDataList);
		std::list<ClientPatternStepData> GetStepData ();
		bool BarDelStepNodeData (uint8_t stepId, uint16_t nodeId);

		Pattern* _pattern = nullptr;
	};
}
