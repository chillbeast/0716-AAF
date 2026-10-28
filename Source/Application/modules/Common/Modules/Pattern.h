#pragma once
#include <stdint.h>
#include <list>
#include <string>
#include "Common/Modules/Bar.h"
#include "Common/Modules/Step.h"
#include "common_defs/source/EngineDataStruct.h"

namespace Ignite
{
	struct UsedByData;
	struct Aux;
	class PatternManager;

	class Bar;
	struct PatternData
	{
		uint8_t numBars = 1;	//defualt number of bars 1
		TimeBase timeBase = TimeBase::Base_4;	//default time base 4
		std::string patternName = " ";
		std::string patternDispName = " ";;
		std::multimap <uint16_t, Bar> barList;
		uint32_t UsedByCount = 0;
		std::list <UsedByData> usedByList;
		int16_t patternLastStepId = 0;
		int32_t id = 0;
		bool _barLoopOn = false;
		int8_t _barLoopStartId = 0;
		int8_t _barLoopEndId = 0;
	};

	class Pattern
	{
	public:
		explicit Pattern (int32_t id, std::string name, uint8_t direction, TimeBase time, uint8_t bars, bool isWithData, PatternManager* patternMgr);
		Pattern (PatternData& patternData, std::list<BarData> *barList, std::list<std::list<StepData>> *StepDataList, PatternManager* patternMgr);
		Pattern (const Pattern&);
		Pattern& operator= (const Pattern&);
		~Pattern ();
		void UpdatePattern(PatternData& patternData, std::list<BarData>* barList, std::list<std::list<StepData>>* StepDataList);

		void setTimeBase (TimeBase val);
		void setName (std::string name);
		void setTotalSteps (uint8_t val);
		void setPlayStep (uint8_t val);
		void setEditStep (uint8_t val);
		void setId (int32_t id);

		TimeBase getTimeBase (void) const;
		int16_t getTicksPerStep (void) const;

		std::vector <PatternDirection> getPlayDirection (void) const;

		std::string getName (void) const;
		uint8_t getPlayStepId (void) const;
		uint8_t getEditStepId (void)const;
		uint8_t getTotalBars (void) const;
		int16_t getTotalSteps (void) const;
		int16_t getLastStepId (void) const;
		int32_t getId (void) const;

		std::vector<StepDataUpdateStatus> PatternSetStepData (uint16_t barId, uint16_t stepId, ClientPatternStepData stepData);
		bool PatternAddNodeData (uint16_t barId, uint16_t stepId, ClientPatternNodeData nodeData);
		bool PatternAddNodeData (uint16_t barId, std::map<ClientPatternNodeData, std::list<uint8_t>> stepMap);
		std::map<int8_t, BarDataUpdateStatus> PatternSetBarDataList(ClientPatternData* clientPatternData);
		//std::tuple<int8_t, BarDataUpdateStatus> PatternSetBarDataList(ClientPatternData& clientPatternData, PatternDataUpdateLocation location);
		BarDataUpdateShallowStatus PatternSetBarData (uint16_t barId, ClienPatterntBarData barData);
		uint16_t AddBar (bool copyAndCreate, uint16_t fromBarId);
		int16_t addBars (int16_t numBarsToAdd);
		uint16_t insertBar (uint16_t position);
		bool RemoveBar (uint16_t barId);
		bool CopyBar (uint16_t srcBarId, uint16_t destBarId);
		bool InsertStep (uint16_t BarId, uint8_t stepCount, uint8_t InsertAt, std::list<ClientPatternStepData> ToStepDataList);
		bool UpdateUsedByList (bool isFillPattern, std::string SongName, std::string SceneName, std::string TrackName);
		bool RemoveFromUsedByList (bool isFillPattern, std::string SongName, std::string SceneName, std::string TrackName);
		bool PatternDelNodeData (uint16_t barId, uint16_t stepId, uint16_t nodeId);
		bool getPatternBarLoopStatus (void);
		int16_t getPatternBarLoopEndBarId (void);
		int16_t getPatternBarLoopStartBarId (void);
		void savePatternUsedByList (void);
		std::string PatternGetDispName (void);
		void clearPattern (void);

		Step* editStep = nullptr;
		Step* playStep = nullptr;
		PatternData* _patternData;
		bool PatternGetBarDataList (ClientPatternData* clientPatternData);
		std::list<ClienPatterntBarData> PatternGetBarData ();
		std::list<ClientPatternStepData> PatternGetStepData (uint16_t barId);
		PatternManager* _patternManager = nullptr;

	private:
		void rebindChildren();
		std::string name;
		uint8_t editStepId;
		uint8_t playStepId;
		uint8_t _NumOfStepsPerBar = MAX_STEPS_IN_PATTERN_BAR;
		void reAssignSequentialIdToBars (void);
		void reAssignSequentialIdToSteps (void);
		std::list<StepData> CreateEmptyStepdata (uint8_t stepCount, uint16_t StartPos);
		std::list<StepData> CreateStepWithRandomdata (uint8_t stepCount, uint16_t StartPos);
		std::list<StepData> AddUpdateBarSteps (Bar Bar, uint8_t InsertAt, std::list<StepData>  stepDataList);
		std::list<StepData> CopyClientStepDataToStepData (std::list<ClientPatternStepData> clientPatternStepData);
	};
}
