#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <assert.h>
#include <memory>
#include "Common/Modules/Step.h"

using namespace Ignite;

struct OffsetRange
{
	TickOffset_t min;
	TickOffset_t max;
};

std::unordered_map <TimeBase, OffsetRange> timeBaseToOffsetRange = {
	{TimeBase::Base_1,   {-96, 95}},
	{TimeBase::Base_2,   {-48, 47}},
	{TimeBase::Base_2T,  {-32, 31}},
	{TimeBase::Base_4,   {-24, 23}},
	{TimeBase::Base_4T,  {-16, 15}},
	{TimeBase::Base_8,	 {-12, 11}},
	{TimeBase::Base_8T,  {-8, 7}},
	{TimeBase::Base_16,  {-6, 5}},
	{TimeBase::Base_16T, {-4, 3}},
	{TimeBase::Base_32,  {-3, 2}},
	{TimeBase::Base_32T, {-2, 1}},
};

Step::Step (const Step& src)
{
	_stepData = new (StepData);
	*_stepData = { 0 };
	*_stepData = *(src._stepData);

	this->_pattern = src._pattern;
}

Step::Step (Pattern* pat, int16_t id, uint8_t note, float length, uint8_t velocity, uint8_t delay)
{
	_pattern = pat;
	_stepData = new (StepData);
	*_stepData = { 0 };
	_stepData->stepId = id;
}

Step::Step (Pattern* pat, StepData& stepData)
{
	_pattern = pat;
	_stepData = new (StepData);
	*_stepData = { 0 };
	*_stepData = stepData;
}
Step::~Step() { delete _stepData; }
Step& Step::operator=(const Step& source)
{
    if (this != &source)
    {
        auto data = std::make_unique<StepData>(*source._stepData);
        delete _stepData;
        _stepData = data.release();
        _pattern = source._pattern;
        isUpdating = false;
    }
    return *this;
}

/*
* Step level changes handled here
*	-> Tie, Transpose, Skip, Step On/off, Note Add, Event Add (step view)
* Node Level Changes handled here
*	-> NOTE -> Offset, Len(step), Len(tick), Pitch, Velocity (Step View, Bar View, Grid View)
*	-> AUX  -> On/off, Data2, ?? (step view)
*
* If these params are handled anywhere else, inform SequenceManager, when it is done.
*/
StepDataUpdateStatus Step::SetStepDataList (ClientPatternStepData* stepData)
{
	StepDataUpdateStatus ret;
	int count = 5000;
	while (isUpdating && count-- > 0); //wait and try, not to loose UI req;
	if (isUpdating == false)
	{
		isUpdating = true;
		*_stepData = *stepData;
		_stepData->nodeCount = (uint8_t)_stepData->nodeList.size();
		isUpdating = false;
	}
	return ret;
}


StepDataUpdateStatus Step::SetStepData (ClientPatternStepData& stepData)
{
	StepDataUpdateStatus ret;

	if (isUpdating == false)
	{
		isUpdating = true;
		*_stepData = stepData;
		_stepData->nodeCount = (uint8_t)_stepData->nodeList.size();
		isUpdating = false;
	}
	return ret;
}

bool Step::SetStepDataDuringRecord (ClientPatternStepData& stepData)
{
	if (isUpdating == false)
	{
		isUpdating = true;
		//_stepData->stepId = stepData.stepId;
		//_stepData->nodeCount = stepData.nodeCount;
		for (auto aux : stepData.auxList)
		{
			_stepData->auxList.push_back (aux);
		}

		_stepData->enable = stepData.enable;
		_stepData->skipStatus = stepData.skipStatus;
		_stepData->transposeDefeatStatus = stepData.transposeDefeatStatus;
		_stepData->tieStatus = stepData.tieStatus;

		_stepData->nodeCount = (uint8_t)_stepData->nodeList.size ();

		for (auto& node : stepData.nodeList)
		{
			for (auto& nodeData : node.second)
			{
				NodeData tmp;
				tmp.nodeId = nodeData.second.nodeId;
				tmp.offset = validateDelayDuringRecord(nodeData.second.offset);
				tmp.lenInSteps = validateLengthInSteps(nodeData.second.lenInSteps);
				tmp.lenInTicks = validateLengthInTicks(nodeData.second.lenInTicks);
				tmp.note = validateNote(nodeData.second.note);
				tmp.velocity = validateVelocity(nodeData.second.velocity);
				tmp.onState = nodeData.second.onState;
				// TODO : limit to 8 ?
				_stepData->nodeList[tmp.note][tmp.offset] = tmp;// node will be created if the data not preset already, else overwrites existing node

#if 0
			printf("PAT NON : NId %2d  Not %2d Vel %3d Len %2d:%2d Del %2d\n", tmp.note, tmp.note, tmp.velocity, tmp.lenInSteps, tmp.lenInTicks, tmp.delay);
			DBG ("PAT NON : NId " + String (tmp.note) + " Not " + String (tmp.note) + " Vel " + String (tmp.velocity)
				+ " Len " + String (tmp.lenInSteps) + ":" + String (tmp.lenInTicks) + " Del " + String (tmp.delay));
#endif
			}
		}
		_stepData->nodeCount = (uint8_t)_stepData->nodeList.size();
		isUpdating = false;
	}



	return true;
}

bool Step::SetNodeData (ClientPatternNodeData nodeData)
{
	if (isUpdating == false)
	{
		isUpdating = true;
		NodeData tmpNodeData = { 0 };
		tmpNodeData.nodeId = nodeData.nodeId;
		tmpNodeData.offset = validateDelay (nodeData.offset);
		tmpNodeData.lenInSteps = validateLengthInSteps (nodeData.lenInSteps);
		tmpNodeData.lenInTicks = validateLengthInTicks (nodeData.lenInTicks);
		tmpNodeData.note = validateNote (nodeData.note);
		tmpNodeData.velocity = validateVelocity (nodeData.velocity);
		_stepData->nodeList[tmpNodeData.note][tmpNodeData.offset] = tmpNodeData;
		_stepData->nodeCount = (uint8_t)_stepData->nodeList.size();
		isUpdating = false;
	}
	return true;
}

bool Step::DelNodeData (uint16_t nodeId)
{
	bool found = false;

	for (auto nVal = _stepData->nodeList.begin (); nVal != _stepData->nodeList.end (); ++nVal)
	{
		if (nVal->first == nodeId)
		{
			_stepData->nodeList.erase (nVal);
			_stepData->nodeCount = _stepData->nodeCount - 1;
			return true;
		}
	}

	return found;
}

std::vector<uint8_t> Step::getStepLengthInSteps (void) const
{
	std::vector<uint8_t> lensteps;

	for (auto ndata = _stepData->nodeList.begin (); ndata != _stepData->nodeList.end (); ++ndata)
	{
		for (auto& nodeData : ndata->second)
			lensteps.push_back(nodeData.second.lenInSteps);
	}
	return lensteps;
}

std::vector<TickLength_t> Step::getStepLengthInTicks (void) const
{
	std::vector<TickLength_t> lenticks;

	for (auto ndata = _stepData->nodeList.begin (); ndata != _stepData->nodeList.end (); ++ndata)
	{
		for (auto& nodeData : ndata->second)
			lenticks.push_back(nodeData.second.lenInTicks);
	}

	return lenticks;
}

std::vector<uint8_t> Step::getStepVelocity (void) const
{
	std::vector<uint8_t> velocity;

	for (auto ndata = _stepData->nodeList.begin (); ndata != _stepData->nodeList.end (); ++ndata)
	{
		for (auto& nodeData : ndata->second)
			velocity.push_back(nodeData.second.velocity);
	}
	return velocity;
}
std::vector<TickOffset_t> Step::getStepDelay (void) const
{
	std::vector<TickOffset_t>offsets;

	for (auto ndata = _stepData->nodeList.begin (); ndata != _stepData->nodeList.end (); ++ndata)
	{
		for (auto& nodeData : ndata->second)
			offsets.push_back(nodeData.second.offset);
	}
	return offsets;
}
std::vector<uint8_t> Step::getStepNote (void) const
{
	std::vector<uint8_t> note;

	for (auto ndata = _stepData->nodeList.begin (); ndata != _stepData->nodeList.end (); ++ndata)
	{
		for (auto& nodeData : ndata->second)
			note.push_back(nodeData.second.note);
	}
	return note;
}
std::vector<NodeData> Step::getNodeList (void) const
{
	std::vector<NodeData> NodeList;

	for (auto ndata = _stepData->nodeList.begin (); ndata != _stepData->nodeList.end (); ++ndata)
	{
		for (auto& nodeData : ndata->second)
			NodeList.push_back(nodeData.second);
	}
	return NodeList;
}

bool Step::getNodeTransposeDefeat (void) const
{
	return _stepData->transposeDefeatStatus;
}

bool Step::getStepTie (void) const
{
	return _stepData->tieStatus;
}

bool Step::getStepSkip (void) const
{
	return _stepData->skipStatus;
}

bool Step::getStepGate (void) const
{
	return  _stepData->enable;
}

std::vector <Aux> Step::getStepAuxList (void)
{
	return _stepData->auxList;
}

int16_t Step::getStepId (void) const
{
	return _stepData->stepId;
}

uint8_t Step::validateNote (uint8_t note)
{
	if (note > MAX_NOTE)
	{
		//assert (0);
		return MAX_NOTE;
	}
	else
	{
		return note;
	}
}

uint8_t Step::validateVelocity (uint8_t velocity)
{
	if (velocity > MAX_NOTE_VELOCITY)
	{
		//assert (0);
		return MAX_NOTE_VELOCITY;
	}
	else
	{
		return velocity;
	}
}

uint8_t Step::validateLengthInSteps (uint8_t lenInSteps)
{
	if (lenInSteps > MAX_LENGTH_OF_A_NOTE_IN_STEPS)
	{
		//assert (0);
		return 0;
	}
	else
	{
		return lenInSteps;
	}
}

TickLength_t Step::validateLengthInTicks (TickLength_t lenInTicks)
{
	auto maxTicksPerStep = _pattern->getTicksPerStep();

	if (lenInTicks > maxTicksPerStep)
	{
		//assert (0);
		return maxTicksPerStep / 2;
	}
	else
	{
		return lenInTicks;
	}
}

TickOffset_t Step::validateDelay (TickOffset_t offset)
{
	TickOffset_t minStepOffset = timeBaseToOffsetRange[_pattern->getTimeBase()].min;
	TickOffset_t maxStepOffset = timeBaseToOffsetRange[_pattern->getTimeBase()].max;

	if ((offset < minStepOffset) || (offset > maxStepOffset))
	{
		//assert(0);
		return 0;
	}
	else
	{
		return offset;
	}
}

//Live-Record: input can come in at any time
//To avoid complexity, allow delays upto max duration a step
//UI Input: input delays will be based on patterntime base max -96 to +95(for TB1) and min -2 to +1 (for TB 32T)
//Play back can handle both delays
TickOffset_t Step::validateDelayDuringRecord (TickOffset_t offset)
{
	auto maxTicksPerStep = _pattern->getTicksPerStep();

	if ((offset < 0) || (offset >= maxTicksPerStep))
	{
		//assert(0);
		offset = 0;
	}
	return offset;
}

//***** TODO need to be redesign used only by sculpt
void Step::setNodeList (std::vector<NodeData> nodeDataList)
{
	/*for (auto& newNodeData : nodeDataList)
	{
		if (isUpdating == true)
			break;
		for (auto& patternNodeData : _stepData->nodeList)
		{
			if (isUpdating == true)
				break;
			if (patternNodeData.second.nodeId == newNodeData.nodeId)
			{
				patternNodeData.second.delay = newNodeData.delay;
				patternNodeData.second.lenInSteps = newNodeData.lenInSteps;
				patternNodeData.second.lenInTicks = newNodeData.lenInTicks;
				patternNodeData.second.velocity = newNodeData.velocity;
			}
		}
	}*/
}

NodeData Step::clientPatternNodeDataToNodeData(ClientPatternNodeData dat)
{
	NodeData ret;

	ret.offset = validateDelay(dat.offset);
	ret.lenInSteps = validateLengthInSteps(dat.lenInSteps);
	ret.lenInTicks = validateLengthInTicks(dat.lenInTicks);
	ret.nodeId = dat.nodeId;
	ret.note = validateNote(dat.note);
	ret.velocity = validateVelocity(dat.velocity);

	//don't touch onState here, user has no control on it.

	return ret;
}

Pattern* Step::getPattern(void)
{
	return _pattern;
}
