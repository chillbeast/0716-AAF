#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include "Firmware/Application/Model/ModelParams.h"
#include "common_defs/source/EngineDataStruct.h"

namespace Ignite
{
	struct NodeChangeStatus
	{
		bool offset;
		bool lenSteps;
		bool lenTicks;
		bool pitch;
		bool velocity;

		NodeChangeStatus()
		{
			offset = lenSteps = lenTicks = pitch = velocity = false;
		}
	};

	struct NodeChangeValue
	{
		int8_t offset;
		int8_t lenSteps;
		int8_t lenTicks;
		int8_t pitch;
		int8_t velocity;

		NodeChangeValue()
		{
			offset = lenSteps = lenTicks = pitch = velocity = -1;
		}
	};

	struct NoteDataUpdateStatus
	{
		NodeChangeStatus changedStatus;
		NodeChangeValue value;
	};

	struct AuxChangeStatus
	{
		bool enabled;
		bool offset;
		bool type;
		bool data1;
		bool data2;

		AuxChangeStatus()
		{
			enabled = offset = type = data1 = data2 = false;
		}
	};

	struct AuxChangeValue
	{
		bool enabled;
		int8_t offset;
		int8_t type;
		int8_t data1;
		int8_t data2;

		AuxChangeValue()
		{
			enabled = false;
			offset = type = data1 = data2 = -1;
		}
	};

	struct AuxDataUpdateStatus
	{
		AuxChangeStatus changedStatus;
		AuxChangeValue value;
	};



	struct StepChangeStatus
	{
		bool tie;
		bool enable;
		bool skip;
		bool transposeDefeat;

		StepChangeStatus()
		{
			tie = enable = skip = transposeDefeat = false;
		}
	};

	struct StepChangeValue
	{
		bool tie;
		bool enable;
		bool skip;
		bool transposeDefeat;

		StepChangeValue()
		{
			tie = enable = skip = transposeDefeat = false;
		}
	};

	//Hold data for one step
	struct StepDataUpdateStatus
	{
		StepChangeStatus changedStatus;
		StepChangeValue value;

		//Nodes data
		//Using vectors, as no specific reason for using maps
		std::vector<NoteDataUpdateStatus> pitchNode;
		std::vector<AuxDataUpdateStatus> auxNode;
	};

	class Pattern;
	class Step
	{
	public:
		explicit Step (Pattern* pat, int16_t id, uint8_t note, float length, uint8_t velocity, uint8_t delay);
		Step (Pattern* pat, StepData& stepData);
		Step (const Step&);
		Step& operator= (const Step&);
		~Step ();
		void rebindPattern(Pattern* pattern) { _pattern = pattern; }

		StepDataUpdateStatus SetStepDataList (ClientPatternStepData* stepData);
		StepDataUpdateStatus SetStepData (ClientPatternStepData& stepData);
		bool SetNodeData (ClientPatternNodeData nodeData);


		void setStepLength (uint8_t val);
		void setStepVelocity (uint8_t val);
		void setStepDelay (uint8_t val);
		void setStepNote (uint8_t note);

		std::vector<uint8_t> getStepLengthInSteps (void) const;
		std::vector<TickLength_t> getStepLengthInTicks (void) const;
		std::vector<uint8_t> getStepVelocity (void) const;
		std::vector<TickOffset_t> getStepDelay (void) const;
		std::vector<uint8_t> getStepNote (void) const;
		bool getNodeTransposeDefeat (void) const;
		bool getStepTie (void) const;
		bool getStepSkip (void) const;
		bool getStepGate (void) const;
		int16_t getStepId (void) const;
		std::vector<NodeData> getNodeList (void) const;
		void setNodeList (std::vector<NodeData> nodeDataList);

		bool DelNodeData (uint16_t nodeData);
		bool SetStepDataDuringRecord (ClientPatternStepData& stepData);
		std::vector <Aux> getStepAuxList (void);

		uint8_t validateNote (uint8_t note);
		uint8_t validateVelocity (uint8_t velocity);
		uint8_t validateLengthInSteps (uint8_t lenInSteps);
		TickLength_t validateLengthInTicks (TickLength_t lenInTicks);
		TickOffset_t validateDelay (TickOffset_t offset);
		TickOffset_t validateDelayDuringRecord (TickOffset_t offset);
		Pattern* getPattern(void);
		StepData* _stepData;
		bool isStepOnEdit () {
			return isUpdating;
		};

	private:
		NodeData clientPatternNodeDataToNodeData(ClientPatternNodeData dat);

		bool isUpdating = false; // Sync the Step edit by Sclpt on play time and UI edit;
		Pattern *_pattern = nullptr;
	};
}
