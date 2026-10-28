#include <stdint.h>
#include <list>
#include <string>
#include <memory>
#include "Common/Modules/Bar.h"
#include "Common/Modules/Step.h"
#include "Firmware/Application/Model/ModelParams.h"

using namespace Ignite;

Bar::Bar(Pattern *pat, const Bar& Src)
{
	_BarData = new (BarData);
	*_BarData = { 0 };

	_BarData->BarId = Src._BarData->BarId;
	_BarData->stepCount = Src._BarData->stepCount;
	_BarData->barRepeatCount = Src._BarData->barRepeatCount;
	_BarData->barTransposeDefeat = Src._BarData->barTransposeDefeat;
	_BarData->gBarSyncOn = Src._BarData->gBarSyncOn;
	_BarData->direction = Src._BarData->direction;
	_BarData->stepList = Src._BarData->stepList;

	rebindPattern(pat);
}
Bar::Bar(const Bar& source) : Bar(source._pattern, source) {}
Bar::~Bar() { delete _BarData; }
void Bar::rebindPattern(Pattern* pattern)
{
    _pattern = pattern;
    for (auto& step : _BarData->stepList) step.second.rebindPattern(pattern);
}
Bar& Bar::operator=(const Bar& source)
{
    if (this != &source)
    {
        auto data = std::make_unique<BarData>(*source._BarData);
        delete _BarData;
        _BarData = data.release();
        rebindPattern(source._pattern);
    }
    return *this;
}

Bar::Bar (Pattern *pat, uint16_t BarID, bool BarTransposeDefeat, uint8_t BarRepeatCount, bool gBarSync, PatternDirection direction, std::list<StepData> StepDataList)
{
	_BarData = new (BarData);
	*_BarData = { 0 };

	_BarData->BarId = static_cast<uint8_t>(BarID);
	_BarData->stepCount = (uint16_t)StepDataList.size ();
	_BarData->barRepeatCount = BarRepeatCount;
	_BarData->barTransposeDefeat = BarTransposeDefeat;
	_BarData->gBarSyncOn = gBarSync;
	_BarData->direction = direction;

	_pattern = pat;

	for (auto it = StepDataList.begin (); it != StepDataList.end (); ++it)
	{
		_BarData->stepList.emplace (it->stepId, Step (_pattern, *it));
	}

}

/*
This function is currently not invoked.
Call is commented in EngineInsertPatternStep()
Check and upadate as required, if it is used in future
*/
std::list<StepData> Bar::AddStep (uint16_t BarID, uint8_t InsertAt, uint8_t NumOfStepsPerBar, std::list<StepData> StepDataList)
{
	uint8_t InserId = InsertAt;
	uint8_t insertCount = (uint8_t)StepDataList.size ();
	bool found = false;
	std::list<StepData> ReturnStepDataList;

	auto StepIt = _BarData->stepList.begin ();
	while (StepIt != _BarData->stepList.end ())
	{
		if (!found && StepIt->second._stepData->stepId == InsertAt)
		{
			found = true;
		}
		if (found)
			StepIt->second._stepData->stepId += insertCount; //update Steps with new StepID
		++StepIt;
	}

	StepIt = _BarData->stepList.find (InsertAt);

	for (auto it = StepDataList.begin (); it != StepDataList.end (); ++it)
	{
		it->stepId = InserId;
		_BarData->stepList.emplace (StepIt->first, Step (_pattern, *it)); //Create new Step and Insert elements
		_BarData->stepCount++;
		StepIt++;
		InserId++;
	}

	if (_BarData->stepList.size () > NumOfStepsPerBar)
	{
		uint16_t NewId = 0;
		StepIt = _BarData->stepList.find (NumOfStepsPerBar + 1);
		// copy the excess step to return list
		while ( (_BarData->stepList.size () > NumOfStepsPerBar) && StepIt != _BarData->stepList.end ())
		{
			StepIt->second._stepData->stepId = ++NewId;
			ReturnStepDataList.push_back (*StepIt->second._stepData);
			StepIt = _BarData->stepList.erase (StepIt);
			_BarData->stepCount--;
		}
	}
	return ReturnStepDataList;
}

/*
 * Iterate through the existing steps
 * Set the data for each step
 * TODO check -  always returning false
*/
std::map <int16_t, StepDataUpdateStatus> Bar::BarSetStepDataList (std::unordered_map <uint16_t, ClientPatternStepData> *StepDataList)
{
	std::map <int16_t, StepDataUpdateStatus> stepDataRet;
	bool isFound = false;

	for (auto stepIt = _BarData->stepList.begin (); stepIt != _BarData->stepList.end (); ++stepIt)
	{
		auto index = stepIt->first;
		for (auto srcStep = StepDataList->begin(); srcStep != StepDataList->end(); ++srcStep)
		{
			if (srcStep->second.stepId == index)
			{
				if (srcStep != StepDataList->end())
				{
					auto ret = stepIt->second.SetStepDataList(&(srcStep->second));
					stepDataRet.emplace(index, ret);
					isFound = true;
					break;
				}
			}
		}
		if(!isFound)
		{
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " WARN Step " + String(index) + " missing in ClientPatternStepData");
		}
	}
	return stepDataRet;
}

/*
* Find the required step using step id
* Set the data for that step
*/
std::vector<StepDataUpdateStatus> Bar::BarSetStepData (uint16_t stepId, ClientPatternStepData stepData)
{
	(void) stepId;//not using, avoid warnings
	std::vector<StepDataUpdateStatus> stepUpdateStatusList;
	bool isFound = false;


//	auto StepIt = _BarData->stepList.find (stepData.stepId);
	for (auto StepIt = _BarData->stepList.begin(); StepIt != _BarData->stepList.end(); ++StepIt)
	{
		if (StepIt->second.getStepId() == stepData.stepId)
		{
			auto ret = StepIt->second.SetStepData(stepData);
			stepUpdateStatusList.push_back(ret);
			isFound = true;
			break;
		}
	}
	if (!isFound)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to locate requested step " + String (stepData.stepId));
	}

	return stepUpdateStatusList;
}

/*
* Find the required step using step id
* Set the node data for that step
*/
bool Bar::BarUpdateStepNodeData (uint16_t stepId, ClientPatternNodeData nodeData)
{
	bool isFound = false;
	for (auto StepIt = _BarData->stepList.begin(); StepIt != _BarData->stepList.end(); ++StepIt)
	{
		if (StepIt->second.getStepId() == stepId)
		{
			isFound = true;
			return StepIt->second.SetNodeData(nodeData);
		}
	}
	if (!isFound)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to locate requested step " + String (stepId));
	}
	return isFound;
}

/*
 * Called from bool IgniteEngine::EngineAddNodeData (std::string patternName, uint16_t BarId, std::map<ClientPatternNodeData, std::list<uint8_t>> stepMap)
 * Currently not invoked by any code
 * Update as requied (to remove 2 for loops), if used in future
 */
bool Bar::BarUpdateStepNodeData (std::map <ClientPatternNodeData, std::list <uint8_t>> stepMap)
{
	bool found = false;
	for (auto StepIt = _BarData->stepList.begin (); StepIt != _BarData->stepList.end (); ++StepIt)
	{
		for (auto tmpStepData : stepMap)
		{
			for (auto tmpStepId : tmpStepData.second)
			{
				if (StepIt->second._stepData->stepId == tmpStepId)
				{
					StepIt->second.SetNodeData (tmpStepData.first);
				}
			}
		}
	}
	return found;
}

/*
* Delete a specific node in a specific step
*/
bool Bar::BarDelStepNodeData (uint8_t stepId, uint16_t nodeId)
{
	bool found = false;
	auto stepIt = _BarData->stepList.find (stepId);

	if (stepIt != _BarData->stepList.end())
	{
		return stepIt->second.DelNodeData (nodeId);
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to locate step " + String (stepId));
		return found;
	}
}

BarDataUpdateShallowStatus Bar::SetBarData (ClienPatterntBarData barData)
{
	BarDataUpdateShallowStatus ret = { false, false, false, false };

	if(_BarData->stepCount != barData.stepCount)
	{
		_BarData->stepCount = barData.stepCount;
		ret.changedStatus.stepCount = true;
		ret.value.stepCount = _BarData->stepCount;
	}

	if(_BarData->barRepeatCount != barData.barRepeatCount)
	{
		_BarData->barRepeatCount = barData.barRepeatCount;
		ret.changedStatus.barRepeatCount = true;
		ret.value.barRepeatCount = _BarData->barRepeatCount;
	}

	if(_BarData->barTransposeDefeat != barData.barTransposeDefeat)
	{
		_BarData->barTransposeDefeat = barData.barTransposeDefeat;
		ret.changedStatus.barTransposeDefeat = true;
		ret.value.barTransposeDefeat = _BarData->barTransposeDefeat;
	}

	if (_BarData->gBarSyncOn != barData.gBarSyncOn)
	{
		_BarData->gBarSyncOn = barData.gBarSyncOn;
		ret.changedStatus.gBarSyncOn = true;
		ret.value.gBarSyncOn = _BarData->gBarSyncOn;
	}

	_BarData->direction = static_cast<PatternDirection> (barData.direction);

	return ret;
}

bool Bar::GetStepDataList (std::unordered_map <uint16_t, ClientPatternStepData>* StepDataList)
{
	ClientPatternStepData clientPatternStepData = { 0 };
	ClientPatternNodeData nodeListData;
	uint8_t StepInBarPosition = 0;
	for (auto StepIt = _BarData->stepList.begin (); StepIt != _BarData->stepList.end (); ++StepIt)
	{
		nodeListData = { 0 };
		clientPatternStepData = { 0 };
		clientPatternStepData.BarId = _BarData->BarId;
		clientPatternStepData = *(StepIt->second._stepData);
		clientPatternStepData.nodeCount = (uint8_t)clientPatternStepData.nodeList.size();
		StepDataList->insert ({ StepInBarPosition, clientPatternStepData });
		StepInBarPosition++;
	}
	return true;
}
std::list<ClientPatternStepData> Bar::GetStepData ()
{
	std::list<ClientPatternStepData> ClientPatternStepDataList;
	ClientPatternStepData clientPatternStepData = { 0 };
	ClientPatternNodeData nodeListData;
	for (auto StepIt = _BarData->stepList.begin (); StepIt != _BarData->stepList.end (); ++StepIt)
	{
		nodeListData = { 0 };
		clientPatternStepData = { 0 };
		clientPatternStepData.BarId = _BarData->BarId;
		clientPatternStepData = *(StepIt->second._stepData);
		clientPatternStepData.nodeCount = (uint8_t)clientPatternStepData.nodeList.size();
		ClientPatternStepDataList.push_back (clientPatternStepData);
	}
	return(ClientPatternStepDataList);
}


uint8_t Bar::getbarRepeatCount (void)
{
	return _BarData->barRepeatCount;
}
