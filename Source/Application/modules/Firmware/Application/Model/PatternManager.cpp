#include <string>
#include "Firmware/Application/Model/PatternManager.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/ModelParams.h"

using namespace Ignite;

std::map<TimeBase, float> timeBaseScaler = { {TimeBase::Base_32T, (1.0f / 12)},
											 {TimeBase::Base_32,  (1.0f / 8) },
											 {TimeBase::Base_16T, (1.0f / 6) },
											 {TimeBase::Base_16,  (1.0f / 4) },
											 {TimeBase::Base_8T,  (1.0f / 3) },
											 {TimeBase::Base_8,   (1.0f / 2) },
											 {TimeBase::Base_4T,  (2.0f / 3) },
											 {TimeBase::Base_4,   (1.0f)     },
											 {TimeBase::Base_2T,  (4.0f / 3) },
											 {TimeBase::Base_2,   (2.0f)     },
											 {TimeBase::Base_1,   (4.0f)     },
};

PatternManager::PatternManager(IParamChangeListener* listener, IgniteEngine& model) : paramlistener(listener), engine(model), Module("PatternManager", (moduleId_t)ModuleName::modulePattern, (unsigned int)PatternParam::paramPatternMax, listener)
{
	numberOfPatterns = 0;

	AddParameter<uint8_t>((uint8_t)PatternParam::paramAddPattern, 0, 127, 0);
	AddParameter<std::string>((uint8_t)PatternParam::paramSavePatternCmd, "0", "0", "0");
	AddParameter<std::string>((uint8_t)PatternParam::paramPatternName, "0", "0", "0");
	AddParameter<std::string>((uint8_t)PatternParam::paramRemovePattern, "0", "0", "0");
	AddParameter<uint8_t>((uint8_t)PatternParam::paramPatternUpdateNotif, 0, 127, 0);
	AddParameter<uint8_t>((uint8_t)PatternParam::paramUndoCmd, 0, 127, 0);
	AddParameter<uint8_t>((uint8_t)PatternParam::paramClearUndoList, 0, 127, 0);
	AddParameter<uint64_t>((uint8_t)PatternParam::paramMovePatternCmd, 0, 0x3FF000003FF, 0);//limiting to 1000 patterns
	AddParameter<std::string>((uint8_t)PatternParam::setActivePattern, "0", "0", "0");
	AddParameter<uint8_t> ((uint8_t)PatternParam::ParamSelectedStep, 0, 255, 0);
	AddParameter<uint8_t> ((uint8_t)PatternParam::ParamDeleteAllPattern, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)PatternParam::paramSaveAll, 0, 1, 0);

	patternNameList = std::move(engine.fileHandler->GetPatternNameList ());
	if (!patternNameList.empty ())
	{
		std::packaged_task<bool (void)> task ([this]() {PatternManager::LoadPatternFromFilesystem (); return true; });
		fut_delayedLoad = task.get_future ();
		ThreadFsLoad = std::thread (std::move (task));
		ThreadFsLoad.detach ();
	}
	else
	{
		is_initCompleted = true;
		engine.updateModuleReadyStatus((moduleId_t)ModuleName::modulePattern, true);
	}
}
bool PatternManager::isInitCompleted ()
{
	if (!is_initCompleted)
	{
		if (fut_delayedLoad.valid ())
			fut_delayedLoad.get ();
		else
			is_initCompleted = false;

		if (ThreadFsLoad.joinable ())
			ThreadFsLoad.join ();
		is_initCompleted = true;
		engine.updateModuleReadyStatus((moduleId_t)ModuleName::modulePattern, true);
	}
	return is_initCompleted;
}
#ifdef ENABLE_PARALLEL_EXE
void PatternManager::LoadPatternFromFilesystem()
{
	if (patternNameList.empty())
		return;

	numberOfPatterns = 0;
	for (const auto& patternName : patternNameList)//create all patterns to avoid list issue - not loading from file
	{
		patternList.emplace_back(numberOfPatterns, patternName, static_cast<uint8_t>(PatternDirection::forward), TimeBase::Base_4, static_cast<uint8_t>(0), false, this);
		numberOfPatterns++;
	}
	std::for_each(
		std::execution::par_unseq,
		patternList.begin(),
		patternList.end(),
		[this](auto&& PatternIt)
		{
			std::list<BarData> barDataListFs;
			StepData stepData = {};
			engine.fileHandler->GetpatternDataHeader(PatternIt._patternData->patternName, PatternIt._patternData);
			PatternIt._patternData->barList.clear();

			barDataListFs = std::move(engine.fileHandler->GetPatternsBarList(PatternIt._patternData->patternName));
			barDataListFs.sort([](const BarData a, const BarData b) {return (a.BarId < b.BarId); });
			std::for_each(
				std::execution::par_unseq,
				barDataListFs.begin(),
				barDataListFs.end(),
				[this, &PatternIt](auto&& barData)
				{
					std::list<StepData> StepDataList;
					StepDataList = std::move(engine.fileHandler->GetpatternBarStepData(PatternIt._patternData->patternName, barData.BarId, barData.stepCount));
					PatternIt._patternData->barList.emplace(barData.BarId, Bar(&PatternIt, barData.BarId, barData.barTransposeDefeat, barData.barRepeatCount, barData.gBarSyncOn, barData.direction, StepDataList));
				}
			);
		}
	);
	sortPatternsById();
	reAssignSequentialIdsForPatternsAndSave();
	engine.updateModuleReadyStatus(static_cast<moduleId_t>(ModuleName::modulePattern), true);
}
#else
void PatternManager::LoadPatternFromFilesystem ()
{
	if (!patternNameList.empty ())
	{
		for (auto pattIt = patternNameList.begin (); pattIt != patternNameList.end (); ++pattIt)
		{
			PatternData patternData;
			engine.fileHandler->GetpatternDataHeader (*pattIt, &patternData);
			std::list<std::list<StepData>> StepDataListBars;
			std::list<BarData> barDataListFs;
			std::list<BarData> BarDataList;
			numberOfPatterns++;

			barDataListFs = engine.fileHandler->GetPatternsBarList (patternData.patternName);
			barDataListFs.sort ([](const BarData a, const BarData b) {return (a.BarId < b.BarId); });
			uint16_t tmpId = 0;
			for (auto &It : barDataListFs)
			{
				It.BarId = tmpId;
				tmpId++;
			}
			for (auto barIt = barDataListFs.begin (); barIt != barDataListFs.end (); ++barIt)
			{
				StepData stepData = {};
				std::list<StepData> StepDataList;
				BarData barData = {};
				stepData.nodeList.clear ();
				StepDataList = engine.fileHandler->GetpatternBarStepData (patternData.patternName, barIt->BarId, barIt->stepCount);
				barData.BarId = barIt->BarId;
				barData.stepCount = barIt->stepCount;
				barData.barRepeatCount = barIt->barRepeatCount;
				barData.barTransposeDefeat = barIt->barTransposeDefeat;
				barData.gBarSyncOn = barIt->gBarSyncOn;
				StepDataListBars.push_back (StepDataList);
				StepDataList.clear ();
				BarDataList.push_back (barData);
			}
			if (patternData.patternName == " ")
			{
				numberOfPatterns--; //Can't do any on corruption, may not reach here as we knwo there is a file.
			}
			else if (BarDataList.empty() || StepDataListBars.empty()) //create dummy to keep the Song/Scene/Track/Usedby list - allign and boot without issue, which could allow user to restore only pattern later
			{
				uint8_t direction = (uint8_t)PatternDirection::forward;//default direction forward
				TimeBase timeBase = TimeBase::Base_4;//default time base 4
				uint8_t numBars = 1;//defualt number of bars 1

				Pattern tempPattern = (Pattern(patternData.id, patternData.patternDispName, direction, timeBase, numBars, false, this));
				patternList.push_back(tempPattern);
			}
			else
			{
				patternList.push_back(Pattern(patternData, &BarDataList, &StepDataListBars, this));
			}
		}
		sortPatternsById ();
		reAssignSequentialIdsForPatternsAndSave ();
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::modulePattern, true);
}
#endif //ENABLE_PARALLEL_EXE
void PatternManager::onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	ParamHandler(moduleSrc, param, true, isPlayMode);
}
void PatternManager::SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler(moduleSrc, FindParameter(param), false, isPlayMode);
}
void PatternManager::ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification, bool isPlayMode)
{
	isInitCompleted ();

	Pattern* pattern = nullptr;
	if (moduleSrc != (moduleId_t)ModuleName::modulePattern)
	{
		switch ((PatternParam)param->GetParamID())
		{
		case PatternParam::paramClearUndoList:
			PatternUndo(true);
			UndoDataList.clear();
			UpdateUndoList(editPatternName, PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT);
			break;

		case PatternParam::paramAddPattern:
			PatternManager::addPattern();
			break;

		case PatternParam::setActivePattern:
			if (editPatternName != ((std::string)(param->getValue().get<std::string>().c_str())))
			{
				playPatternName = editPatternName = ((std::string)(param->getValue().get<std::string>().c_str()));
				UpdateUndoList(editPatternName, PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT);
			}
			else
				SendNotification = false;
			break;

		case PatternParam::paramSaveAll:
			SaveData();
			break;

		case PatternParam::paramSavePatternCmd:
			pattern = GetPattern((std::string)(param->getValue().get<std::string>().c_str()));
			SavePattern(pattern, false);
			break;

		case PatternParam::paramRemovePattern:
			removePattern((std::string)(param->getValue().get<std::string>().c_str()));
			break;
		case PatternParam::ParamSelectedStep:
			engine.songManager->setStepReSelectedStep(param->getValue().get<uint8_t>());
			break;

		case PatternParam::paramUndoCmd:
			PatternUndo();
			break;

		case PatternParam::ParamDeleteAllPattern:
			clearAllPattern ();
			break;

		case PatternParam::paramMovePatternCmd:
			movePattern(param->getValue().get<uint64_t>());
			break;
		}
		if (engine.getAutoSaveStatus() || engine.settingsPatternCnfg->_SettingsPatternCnfgData.PatternAutoSaveParam)
		{
			SaveData();
		}
		if (SendNotification)
		{
			/* Send notification to the subscribed modules via engine */
			paramlistener->onParameterChanged(moduleSrc, param);
		}
	}
}
void PatternManager::SaveData()
{
	if (UndoDataList.size() == 1)
		return;
	Pattern* pattern = nullptr;
	if (UndoDataList.size() > 1 && UndoDataList.front().UpdateReasonType != PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT)
	{
		ClientPatternData _clientPatternData = UndoDataList.front()._clientPatternData;
		pattern = GetPattern(_clientPatternData.patternName);
		SavePattern(pattern, false);
	}
}
void PatternManager::clearAllPattern ()
{
	UndoDataList.clear ();
	patternNameList.clear ();
	patternList.clear ();
	engine.fileHandler->DeleteAllPattern ();
}

bool PatternManager::SavePattern(Pattern* pattern, bool isCreate)
{
    if (pattern == nullptr) return false;
    const auto patternName = pattern->getName();
    if (engine.fileHandler->Createpattern(patternName, *pattern->_patternData) != FILE_OK) return false;
    if (!isCreate && !UndoDataList.empty())
    {
        const auto oldName = UndoDataList.back()._clientPatternData.patternName;
        if (oldName != patternName) engine.fileHandler->DeletePattern(oldName);
    }
    if(UndoDataList.empty() || UndoDataList.front()._clientPatternData.patternName==patternName)
        UpdateUndoList(patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT);
    engine.desktopSaved("pattern:"+patternName);
    return true;
}

std::string PatternManager::addDesktopPattern(ClientPatternData data)
{
    data.id=numberOfPatterns++;
    data.patternName="PATTERN-"+std::to_string(numberOfPatterns);modifyPatternNameIfDuplicate(data.patternName);
    if(data.patternDispName.empty())data.patternDispName=data.patternName;
    const auto name=data.patternName;patternList.push_back(CreatePattern(std::move(data)));
    engine.desktopEdited("pattern:"+name);return name;
}

std::string PatternManager::addPattern(void)
{
	uint8_t direction = (uint8_t)PatternDirection::forward;//default direction forward
	TimeBase timeBase = TimeBase::Base_4;//default time base 4
	uint8_t numBars = 1;//defualt number of bars 1

	std::string name;

	numberOfPatterns++;
	name = "PATTERN-" + std::to_string(numberOfPatterns);
	modifyPatternNameIfDuplicate(name);

	Pattern tempPattern = Pattern(numberOfPatterns - 1, name, direction, timeBase, numBars, false, this);
	patternList.push_back(tempPattern);

	SavePattern(&tempPattern, true);
	return name;
}
std::string PatternManager::addPatternWithRandomData (uint8_t numBars)
{
	uint8_t direction = (uint8_t)PatternDirection::forward;//default direction forward
	TimeBase timeBase = TimeBase::Base_4;//default time base 4
	std::string name;

	numberOfPatterns++;
	name = "PATTERN-" + std::to_string (numberOfPatterns);
	modifyPatternNameIfDuplicate (name);

	Pattern tempPattern = Pattern (numberOfPatterns - 1, name, direction, timeBase, numBars, true, this);
	patternList.push_back (tempPattern);

	SavePattern (&tempPattern, true);
	return name;
}

void PatternManager::modifyPatternNameIfDuplicate(std::string& name)
{
	for (auto& patternIter : patternList)
	{
		if (patternIter.getName() == name)
		{
			srand((unsigned int)time(NULL));
			int16_t randomTag = rand() % 9999;
			std::string tagString = "_" + std::to_string(randomTag);
			name = name + tagString;
		}
	}
}

Pattern* PatternManager::GetPattern(std::string patternName)
{
	isInitCompleted ();
	for (auto patternIt = patternList.begin(); patternIt != patternList.end(); ++patternIt)
	{
		if ((*patternIt)._patternData->patternName == patternName)
			return &(*patternIt);
	}
	return nullptr;
}

void PatternManager::removePattern(std::string patternName)
{
	for (auto patternIt = patternList.begin(); patternIt != patternList.end(); ++patternIt)
	{
		if ((*patternIt)._patternData->patternName == patternName)
		{
			patternList.erase(patternIt);
			--numberOfPatterns;
			if (!UndoDataList.empty ())
			{
				ClientPatternData _clientPatternData = UndoDataList.back ()._clientPatternData;// Default data
				if (patternName != _clientPatternData.patternName)
				{
					// Pattern renamed, delete with old Pattern name
					patternName = _clientPatternData.patternName;
				}
			}
			engine.fileHandler->DeletePattern(patternName);
			break;
		}
	}
}
std::list<std::string> PatternManager::PatternMngrGetPatterList(bool isDisplayName)
{
	std::list<std::string> NameList;
	if(isDisplayName)
		isInitCompleted(); //TODO : Optimize UI code to send request from Pattern view not on bootup
	if (is_initCompleted)
	{
		if (isDisplayName)
		{
			for (auto it = patternList.begin(); it != patternList.end(); ++it)
			{
				NameList.push_back(it->_patternData->patternDispName);
			}
		}
		else
		{
			for (auto it = patternList.begin(); it != patternList.end(); ++it)
			{
				NameList.push_back(it->_patternData->patternName);
			}
		}
	}
	else if (!patternNameList.empty ())
	{
		NameList = patternNameList;
	}
	return(NameList);
}
ClientPatternData PatternManager::PatternMngrGetPatternData(std::string patternName)
{
	ClientPatternData clientPatternData = { 0 };
	ClientUsedByData cusedbydata = { 0 };

	if (!patternList.empty () && isInitCompleted ())
	{
		if (patternName.empty () && (!editPatternName.empty()))
		{
			patternName = editPatternName;
		}

		for (auto it = patternList.begin(); it != patternList.end(); ++it)
		{
			if (it->_patternData->patternName == patternName)
			{
				clientPatternData.numBars = it->_patternData->numBars;
				clientPatternData.timeBase = mapTbToUiIndex(TimeBase(it->_patternData->timeBase));
				clientPatternData.patternName = it->_patternData->patternName;
				clientPatternData.patternDispName = it->_patternData->patternDispName;
				clientPatternData.patternLastStepId = it->_patternData->patternLastStepId;
				clientPatternData.id = it->_patternData->id;
				clientPatternData.barLoopOn = it->_patternData->_barLoopOn;
				clientPatternData.barLoopStartId = it->_patternData->_barLoopStartId;
				clientPatternData.barLoopEndId = it->_patternData->_barLoopEndId;

				for (auto usedby = it->_patternData->usedByList.begin(); usedby != it->_patternData->usedByList.end(); ++usedby)
				{
					cusedbydata.isFillPattern = usedby->isFillPattern;
					cusedbydata.songName = usedby->songName;
					cusedbydata.sceneName = usedby->sceneName;
					cusedbydata.trackName = usedby->trackName;
					clientPatternData.usedByList.push_back(cusedbydata);
				}
				it->PatternGetBarDataList (&clientPatternData);
				break;
			}
		}
	}
	return(clientPatternData);
}
std::list<ClienPatterntBarData> PatternManager::PatternMngrGetBarData(std::string patternName)
{
	std::list<ClienPatterntBarData> clientBarDataList;
	if (!patternList.empty())
	{
		for (auto it = patternList.begin(); it != patternList.end(); ++it)
		{
			if (it->_patternData->patternName == patternName)
			{
				return(it->PatternGetBarData());
			}
		}
	}
	return(clientBarDataList);
}
std::list<ClientPatternStepData> PatternManager::PatternMngrGetStepData(std::string patternName, uint16_t BarId)
{
	std::list<ClientPatternStepData> ClientPatternStepDataList;
	if (patternList.empty())
		return ClientPatternStepDataList;

	for (auto it = patternList.begin(); it != patternList.end(); ++it)
	{
		if (it->_patternData->patternName == patternName)
		{
			return(it->PatternGetStepData(BarId));
		}
	}
	return (ClientPatternStepDataList);
}

bool PatternManager::PatternMngrSetPatternDataOptimized(moduleId_t moduleSrc, std::string patternName, ClientPatternData* clientPatternData, PatternDataUpdateReasonType UpdateReasonType, bool isPlayMode)
{
	bool updated = false;

	for (auto it = patternList.begin (); it != patternList.end(); ++it)
	{
		if (it->_patternData->patternName == patternName)
		{
			PatternChangeStatus patternChangeStat{false,false,false,false,false,false,false,false};
			//PatternChangeValue patternChangeVal = { 0 };

			if (it->_patternData->numBars != clientPatternData->numBars)
			{
				it->_patternData->numBars = clientPatternData->numBars;
				patternChangeStat.barCount = true;
			}
			if (it->_patternData->timeBase != mapUiIndexToTb(clientPatternData->timeBase))
			{
				TimeBase oldTb = (TimeBase)it->_patternData->timeBase;
				TimeBase newTb = mapUiIndexToTb(clientPatternData->timeBase);

				it->_patternData->timeBase = newTb;
				patternChangeStat.timebase = true;
				updateLengthAndOffsetOnTbChange(clientPatternData, oldTb, newTb);
			}

			if (it->_patternData->patternLastStepId != clientPatternData->patternLastStepId)
			{
				it->_patternData->patternLastStepId = clientPatternData->patternLastStepId;
				patternChangeStat.lastStepId = true;
			}

			if (it->_patternData->patternDispName != clientPatternData->patternDispName)
			{
				it->_patternData->patternDispName = clientPatternData->patternDispName;
			}

			//ui calls this function 3 times to set bar loop
			// 1 bar loop on
			// 2 bar loop on + bar loop start id
			// 3 bar loop on + bar loop start id + bar loop end id
			// so consider bar loop chnage if any of above 3 is set
			if ( (it->_patternData->_barLoopOn != clientPatternData->barLoopOn) || (it->_patternData->_barLoopStartId != clientPatternData->barLoopStartId) || (it->_patternData->_barLoopEndId != clientPatternData->barLoopEndId))
			{
				it->_patternData->_barLoopOn = clientPatternData->barLoopOn;
				patternChangeStat.barLoopOn = true;
			}

			if (it->_patternData->_barLoopOn == false)
			{
				it->_patternData->_barLoopStartId = -1;
				it->_patternData->_barLoopEndId = -1;

				if (patternChangeStat.barLoopOn)
				{
					patternChangeStat.startBarId = true;
					patternChangeStat.endBarId = true;
				}
			}
			else
			{
				it->_patternData->_barLoopStartId = clientPatternData->barLoopStartId;
				it->_patternData->_barLoopEndId = clientPatternData->barLoopEndId;
				if (patternChangeStat.barLoopOn)
				{
					patternChangeStat.startBarId = true;
					patternChangeStat.endBarId = true;
				}
			}

			// Assume all calls from UI, updates the pattern. So re-calculate the sequence, once this function is called.
			// barcount, direction, laststep change would re-calcuate the sequence, since we are doing it by default. Comment those 3 here
			if (patternChangeStat.barCount || patternChangeStat.direction || patternChangeStat.timebase || patternChangeStat.lastStepId || patternChangeStat.barLoopOn)
			{
				for (auto& listener : _patternListeners)
				{
					//if (patternChangeStat.barCount) listener->handleBarCountChange(it->_patternData->numBars, patternName);
					//if (patternChangeStat.direction) listener->handleDirectionChange(static_cast<PatternDirection>(it->_patternData->direction), patternName);
					if (patternChangeStat.timebase) listener->handleTimeBaseChange(static_cast<TimeBase>(it->_patternData->timeBase), patternName);
					//if (patternChangeStat.lastStepId) listener->handleLastStepChange(it->_patternData->patternLastStepId, patternName);
					if (patternChangeStat.barLoopOn) listener->handleBarLoopChange(it->_patternData->_barLoopOn, patternName);
					if (patternChangeStat.startBarId) listener->handleBarLoopStartChange(it->_patternData->_barLoopStartId, patternName);
					if (patternChangeStat.endBarId) listener->handleBarLoopEndChange(it->_patternData->_barLoopEndId, patternName);
				}
			}

			auto ret = it->PatternSetBarDataList((clientPatternData));


#if 1
			// Assume all calls from UI, updates the pattern. So re-calculate the sequence, once this function is called.
			// If required in future, use return value from "PatternSetBarDataList" , to calculate sequence updates for specific scenarios, to reduce load of recalculating pattern seq and event list.
			// Use numBar param, as it updates all 3 (bar params, seq order and event updates)

			for (auto& listener : _patternListeners)
			{
				listener->handleBarCountChange(it->_patternData->numBars, patternName);
				engine.songManager->checkAndApplyNewTimeBaseForScene();
			}
#endif

			//update bar level params - bar repeat count, bar travnspose defeat, gbar sync, bar step count
			//update step level params - step enable, tie, transpose defeat, skip 
			for (auto &individualBarRet : ret)
			{
				auto& barId = std::get<0>(individualBarRet);
				auto& barUpdates = std::get<1>(individualBarRet);

				auto& barChangeStat = barUpdates.barData.changedStatus;
				auto& barChangeData = barUpdates.barData.value;

				if (barChangeStat.barRepeatCount || barChangeStat.barTransposeDefeat || barChangeStat.gBarSyncOn || barChangeStat.stepCount)
				{
					updated = true;
					PatternBarInfo barAncestory = { patternName, barId };
					for (auto& listener : _barListeners)
					{
						if (barChangeStat.barRepeatCount)
						{
							barAncestory.repeatCount = barChangeData.barRepeatCount;
							listener->handleBarRepeatChange(barAncestory);
						}
						if (barChangeStat.barTransposeDefeat) listener->handleBarTransposeDefeatChange(barChangeData.barTransposeDefeat, barAncestory);

						if (barChangeStat.gBarSyncOn)
						{
							barAncestory.gBarSyncState = barChangeData.gBarSyncOn;
							listener->handleBarGbarSyncChange(barAncestory);
						}
						//if (barChangeStat.stepCount) listener->handleBarStepCountChange(barChangeData.stepCount, barAncestory);
					}
				}

				auto& stepDataList = barUpdates.stepData;

				for (auto& stepData : stepDataList)
				{
					auto& stepLevelDataStatus = stepData.second.changedStatus;
					auto& stepLevelDataVal = stepData.second.value;
					if (stepLevelDataStatus.enable || stepLevelDataStatus.skip || stepLevelDataStatus.tie || stepLevelDataStatus.transposeDefeat)
					{
						PatternBarStepInfo stepAncestory = { patternName, barId, stepData.first };

						for (auto& listener : _stepListeners)
						{
							if (stepLevelDataStatus.enable) listener->handleStepGateChange(stepLevelDataVal.enable, stepAncestory);
							if (stepLevelDataStatus.skip) listener->handleStepSkipChange(stepLevelDataVal.skip, stepAncestory);
							if (stepLevelDataStatus.tie)  listener->handleStepTieChange(stepLevelDataVal.tie, stepAncestory);
							if (stepLevelDataStatus.transposeDefeat) listener->handleStepTransposeDefeatChange(stepLevelDataVal.transposeDefeat, stepAncestory);
						}

					}
				}
			}
			if (engine.getAutoSaveStatus() || engine.settingsPatternCnfg->_SettingsPatternCnfgData.PatternAutoSaveParam)
			{
				SaveData();
			}
			if (!isPlayMode)
			{
				UpdateUndoList (patternName, UpdateReasonType, clientPatternData);
			}
			updated = true;
			break;
		}
	}
	if (updated && !isPlayMode)
		paramlistener->onParameterChanged (moduleSrc, FindParameter ((paramId_t)PatternParam::paramPatternUpdateNotif));
	return updated;
}

bool PatternManager::PatternMngrSetPatternData(moduleId_t moduleSrc, std::string patternName, ClientPatternData&& clientPatternData, PatternDataUpdateReasonType UpdateReasonType, bool isPlayMode)
{
	bool updated = false;
	bool NotToSetUndo = true;
	ClientPatternData _clientPatternData = std::move(clientPatternData); // Use move semantics

	for (auto it = patternList.begin(); it != patternList.end(); ++it)
	{
		if (it->_patternData->patternName == patternName)
		{
			it->_patternData->numBars = _clientPatternData.numBars;
			it->_patternData->timeBase = mapUiIndexToTb(_clientPatternData.timeBase);
			it->_patternData->patternLastStepId = _clientPatternData.patternLastStepId;
			//it->_patternData->id = _clientPatternData.id;

			it->_patternData->patternDispName = _clientPatternData.patternDispName;
			//if (it->_patternData->patternName != _clientPatternData.patternName)
			//{
			//    patternName = playPatternName = editPatternName = _clientPatternData.patternName; // pattern rename
			//    it->_patternData->patternName = _clientPatternData.patternName;
			//}
			it->_patternData->_barLoopOn = _clientPatternData.barLoopOn;
			if (!it->_patternData->_barLoopOn)
			{
				it->_patternData->_barLoopStartId = -1;
				it->_patternData->_barLoopEndId = -1;
			}
			else
			{
				it->_patternData->_barLoopStartId = _clientPatternData.barLoopStartId;
				it->_patternData->_barLoopEndId = _clientPatternData.barLoopEndId;
			}

			for (auto& barIt : _clientPatternData.BarDataList)
			{
				PatternMngrSetBarData(moduleSrc, patternName, barIt.second.BarId, std::move(barIt.second), NotToSetUndo);
				for (auto& StepIt : barIt.second.StepDataList)
				{
					PatternMngrSetStepData(moduleSrc, patternName, barIt.second.BarId, StepIt.second.stepId, std::move(StepIt.second), NotToSetUndo);
				}
			}
			if (!isPlayMode)
			{
				UpdateUndoList(patternName, UpdateReasonType);
			}
			updated = true;
			break;
		}
	}
	if (engine.getAutoSaveStatus() || engine.settingsPatternCnfg->_SettingsPatternCnfgData.PatternAutoSaveParam)
	{
		SaveData();
	}
	if (updated && !isPlayMode)
		paramlistener->onParameterChanged(moduleSrc, FindParameter((paramId_t)PatternParam::paramPatternUpdateNotif));
	return updated;
}


bool PatternManager::PatternMngrSetBarData(moduleId_t moduleSrc, std::string patternName, uint16_t BarId, ClienPatterntBarData barData, bool isPlayMode)
{
	BarDataUpdateShallowStatus ret = { false, false, false, false };
	bool updated = false;

	for (auto& pattern : patternList)
	{
		if (pattern._patternData->patternName == patternName)
		{
			ret = pattern.PatternSetBarData(BarId, barData);
			if (ret.changedStatus.barRepeatCount || ret.changedStatus.barTransposeDefeat || ret.changedStatus.gBarSyncOn || ret.changedStatus.stepCount)
			{
				updated = true;
				PatternBarInfo barAncestory = { patternName, static_cast<int8_t>(BarId) };
				for (auto& listener : _barListeners)
				{
					if (ret.changedStatus.barRepeatCount)
					{
						barAncestory.repeatCount = ret.value.barRepeatCount;
						listener->handleBarRepeatChange(barAncestory);
					}
					if (ret.changedStatus.barTransposeDefeat) listener->handleBarTransposeDefeatChange(ret.value.barTransposeDefeat, barAncestory);
					if (ret.changedStatus.gBarSyncOn)
					{
						barAncestory.gBarSyncState = ret.value.gBarSyncOn;
						listener->handleBarGbarSyncChange(barAncestory);
					}
				}
			}
			if (!isPlayMode) {
				UpdateUndoList(patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_BarData);
			}
			break;
		}
	}

	if (updated && !isPlayMode) {
		paramlistener->onParameterChanged(moduleSrc, FindParameter((paramId_t)PatternParam::paramPatternUpdateNotif));
	}
	return updated;
}
bool PatternManager::PatternMngrSetStepData(moduleId_t moduleSrc, std::string patternName, uint16_t BarId, uint16_t stepId, ClientPatternStepData stepData, bool isPlayMode)
{
	bool updated = false;
	for (auto it = patternList.begin(); it != patternList.end(); ++it)
	{
		if (it->_patternData->patternName == patternName)
		{
			it->PatternSetStepData (BarId, stepData.stepId, stepData);
			PatternBarStepInfo stepAncestory = { patternName, static_cast<int8_t>(BarId), static_cast<int16_t>(stepId) };
			engine.sequenceManager->handleRecordChange(stepAncestory);
			updated = true;

			if (!isPlayMode)
				UpdateUndoList(patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_StepData);
			break;
		}
	}
	if (updated && !isPlayMode)
		paramlistener->onParameterChanged(moduleSrc, FindParameter((paramId_t)PatternParam::paramPatternUpdateNotif));
	return updated;
}

bool PatternManager::PatternMngrAddNodeData(std::string patternName, uint16_t BarId, uint16_t stepId, ClientPatternNodeData nodeData, bool isPlayMode)
{
	bool updated = false;
	for (auto it = patternList.begin(); it != patternList.end(); ++it)
	{
		if (it->_patternData->patternName == patternName)
		{
			updated = (it->PatternAddNodeData (BarId, (uint8_t)stepId, nodeData));
			if(!isPlayMode)
				UpdateUndoList(patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_AddNodeData);
		}
	}
	//if (updated && !isPlayMode)
		//paramlistener->onParameterChanged(moduleSrc, FindParameter((paramId_t)PatternParam::paramPatternUpdateNotif));
	return updated;
}

bool PatternManager::PatternMngrAddNodeData(std::string patternName, uint16_t BarId, std::map<ClientPatternNodeData, std::list<uint8_t>> stepMap, bool isPlayMode)
{
	bool updated = false;
	for (auto it = patternList.begin(); it != patternList.end(); ++it)
	{
		if (it->_patternData->patternName == patternName)
		{
			updated = (it->PatternAddNodeData (BarId, stepMap));
			if(!isPlayMode)
				UpdateUndoList(patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_AddNodeData);
		}
	}
	//if (updated && !isPlayMode)
	//	//paramlistener->onParameterChanged(moduleSrc, FindParameter((paramId_t)PatternParam::paramPatternUpdateNotif));
	return updated;
}

bool PatternManager::PatternMngrDelNodeData(std::string patternName, uint16_t BarId, uint16_t stepId, uint16_t nodeId, bool isPlayMode)
{
	bool updated = false;
	for (auto it = patternList.begin(); it != patternList.end(); ++it)
	{
		if (it->_patternData->patternName == patternName)
		{
			updated = (it->PatternDelNodeData(BarId, (uint8_t)stepId, nodeId));
			if(!isPlayMode)
				UpdateUndoList (patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_DeleteNodeData);
		}
	}
	//if (updated && !isPlayMode)
		//paramlistener->onParameterChanged(moduleSrc, FindParameter((paramId_t)PatternParam::paramPatternUpdateNotif));
	return updated;
}


bool PatternManager::PatternCopyBar(std::string patternName, uint16_t srcBarId, uint16_t destBarId)
{
	bool updated = false;
	if (!patternList.empty())
	{
		for (auto it = patternList.begin(); it != patternList.end(); ++it)
		{
			if (it->_patternData->patternName == patternName)
			{
				updated = (it->CopyBar(srcBarId, destBarId));
				it->_patternData->patternLastStepId = it->getTotalSteps ();
				UpdateUndoList (patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_CopyBar);
				break;
			}
		}
	}
	return updated;
}

bool PatternManager::PatternAddBar(std::string patternName, bool CopyAndCreate, uint16_t FromBarId, uint16_t NumBarToAdd)
{
	bool status = false;
	if (!patternList.empty())
	{
		for (auto it = patternList.begin(); it != patternList.end(); ++it)
		{
			if (it->_patternData->patternName == patternName)
			{
				for (auto i = 0; i < NumBarToAdd; i++)
				{
					status = it->AddBar (CopyAndCreate, FromBarId);
				}
				it->_patternData->patternLastStepId = it->getTotalSteps ();

				for (auto& listener : _patternListeners)
				{
					listener->handleBarCountChange(it->_patternData->numBars, patternName);
				}

				UpdateUndoList (patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_AddBar);
			}
		}
	}
	return status;
}

bool PatternManager::patternInsertBar(std::string patternName, uint16_t postion)
{
	bool updated = false;
	if (!patternList.empty())
	{
		for (auto it = patternList.begin(); it != patternList.end(); ++it)
		{
			if (it->_patternData->patternName == patternName)
			{
				updated = (it->insertBar (postion));
				it->_patternData->patternLastStepId = it->getTotalSteps ();

				for (auto& listener : _patternListeners)
				{
					listener->handleBarCountChange(it->_patternData->numBars, patternName);
				}
				UpdateUndoList(patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_InsertBar);
				break;
			}
		}
	}
	return updated;
}

bool PatternManager::PatternRemoveBar(std::string patternName, uint16_t BarId, uint16_t NumBarToRemove)
{
	bool status = false;
	if (!patternList.empty())
	{
		for (auto it = patternList.begin(); it != patternList.end(); ++it)
		{
			if (it->_patternData->patternName == patternName)
			{
				if (NumBarToRemove)
				{
					for (auto i = 0; i < NumBarToRemove; i++)
					{
						status = (it->RemoveBar (it->getTotalBars () - 1));//Bar ID starts with 0
					}
				}
				else
				{
					status = (it->RemoveBar (BarId));
				}
				it->_patternData->patternLastStepId = it->getTotalSteps ();

				for (auto& listener : _patternListeners)
				{
					listener->handleBarCountChange(it->_patternData->numBars, patternName);
				}

				UpdateUndoList (patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_RemoveStep);
				break;
			}
		}
	}
	return status;
}

bool PatternManager::PatternInsertStep(std::string name,uint16_t bar,uint8_t count,uint8_t position,std::list<ClientPatternStepData> data)
{
    auto* pattern=GetPattern(name);
    if(!pattern) return false;
    auto before=PatternMngrGetPatternData(name);
    if(!pattern->InsertStep(bar,count,position,std::move(data))) return false;
    if(UndoDataList.empty() || UndoDataList.front()._clientPatternData.patternName!=name)
        UpdateUndoList(name,PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT,&before);
    for(auto* listener:_patternListeners)
    {
        listener->handleBarCountChange(pattern->_patternData->numBars,name);
        listener->handleLastStepChange(uint16_t(pattern->_patternData->patternLastStepId),name);
    }
    UpdateUndoList(name,PatternDataUpdateReasonType::PatternDataUpdateReason_InsertStep);
    return true;
}

std::list<PatternDataUpdateReasonType> PatternManager::PatternGetUndoList(void)
{
	std::list<PatternDataUpdateReasonType>  ReasonTypList;

	for (auto& it : UndoDataList)
	{
		if (it.UpdateReasonType != PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT)
			ReasonTypList.push_back(it.UpdateReasonType);
	}
	//ReasonTypList.erase (unique (ReasonTypList.begin (), ReasonTypList.end ()), ReasonTypList.end ());

	return ReasonTypList;
}

void PatternManager::UpdateUndoList(std::string patternName, PatternDataUpdateReasonType _dataUpdateReasonType, ClientPatternData* clientPatternData)
{
    if(_dataUpdateReasonType!=PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT) engine.desktopEdited("pattern:"+patternName);
	PatternUndoData patternUndoData;
	if (clientPatternData != nullptr)
	{
		patternUndoData._clientPatternData = std::move (*clientPatternData);
	}
	else
	{
		patternUndoData._clientPatternData = std::move(PatternMngrGetPatternData (patternName));
	}
	patternUndoData.UpdateReasonType = _dataUpdateReasonType;

	if (_dataUpdateReasonType == PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT)
		UndoDataList.clear();

	if (UndoDataList.size() > MAX_PATTERN_UNDO_COUNT) // NOTE: UndoDataList last param is Default Data
	{
		auto it = UndoDataList.end();
		it--;
		it--;// skip default
		UndoDataList.erase(it);
	}
	//UndoDataList.insert(UndoDataList.begin(), patternUndoData);
	UndoDataList.emplace_front(std::move(patternUndoData));
}

void PatternManager::desktopMarkSaved()
{
    const auto current=!editPatternName.empty()?editPatternName:UndoDataList.empty()?std::string{}:UndoDataList.front()._clientPatternData.patternName;
    if(GetPattern(current)) UpdateUndoList(current,PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT);
    else UndoDataList.clear();
}

PatternDataUpdateReasonType PatternManager::PatternUndo(bool resetToDefault)
{
	if(UndoDataList.empty()) return PatternDataUpdateReasonType::PatternDataUpdateReason_UNKNOWN;
	ClientPatternData _clientPatternData;
	PatternDataUpdateReasonType UpdateReasonType = PatternDataUpdateReasonType::PatternDataUpdateReason_UNKNOWN;
	bool patternRegenerate = false;

	if (resetToDefault)
	{
		PatternUndoData& UndoData = UndoDataList.back();
		_clientPatternData = (UndoData._clientPatternData);
		UpdateReasonType = UndoData.UpdateReasonType;
	}
	else
	{
		if (UndoDataList.size() > 1) // NOTE: UndoDataList last param is Default Data
		{
			UndoDataList.pop_front();
		}

		PatternUndoData& UndoData = UndoDataList.front();
		_clientPatternData = (UndoData._clientPatternData);
		UpdateReasonType = UndoData.UpdateReasonType;
	}

	std::string clientPatternName = _clientPatternData.patternName;

	switch (UpdateReasonType)
	{
	case PatternDataUpdateReasonType::PatternDataUpdateReason_PATTERNDATA:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_direction:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_timeBase:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_patternLastStepId:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_patternName:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_barLoopOn:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_barLoopStartId:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_barLoopEndId:
		PatternMngrSetPatternData((moduleId_t)ModuleName::moduleUndo, editPatternName, std::move(_clientPatternData), UpdateReasonType, true); // update only the data
		break;

	case PatternDataUpdateReasonType::PatternDataUpdateReason_DEFAULT:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_numBars:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_BarData:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_AddBar:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_InsertBar:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_CopyBar:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_StepData:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_InsertStep:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_RemoveStep:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_DeleteNodeData:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_AddNodeData:
	case PatternDataUpdateReasonType::PatternDataUpdateReason_PatternRotate:
		patternRegenerate = true; // recreate with new data
		break;

	default:
		break;
	}

	if (patternRegenerate)
	{
		for (auto& pattern : patternList)
		{
			if (pattern._patternData->patternName == clientPatternName)
			{
				//pattern = CreatePattern(std::move(_clientPatternData));
				UpdatePattern(&pattern, std::move(_clientPatternData));
				for (auto& listener : _patternListeners)
				{
					listener->handleBarCountChange(pattern._patternData->numBars, pattern._patternData->patternName);
					engine.songManager->checkAndApplyNewTimeBaseForScene();
				}
				break;
			}
		}
	}

	if (clientPatternName != editPatternName)
	{
		playPatternName = editPatternName = clientPatternName; // pattern renamed, set the correct name
	}

	return UpdateReasonType;
}
void PatternManager::UpdatePattern(Pattern* pattern, ClientPatternData _clientPatternData)
{
	PatternData patternData = {};
	StepData stepData = {};
	std::list<StepData> StepDataList;
	std::list<std::list<StepData>> StepDataListBars;
	BarData barData = {};
	std::list<BarData> BarDataList;

	patternData.numBars = _clientPatternData.numBars;
	patternData.timeBase = mapUiIndexToTb(_clientPatternData.timeBase);
	patternData.patternName = _clientPatternData.patternName;
	patternData.patternDispName = _clientPatternData.patternDispName;
	patternData.UsedByCount = 0;
	patternData.patternLastStepId = _clientPatternData.patternLastStepId;
	patternData.id = _clientPatternData.id;
	patternData._barLoopOn = _clientPatternData.barLoopOn;
	patternData._barLoopStartId = _clientPatternData.barLoopStartId;
	patternData._barLoopEndId = _clientPatternData.barLoopEndId;

	for (auto& it : _clientPatternData.usedByList)
	{
		UsedByData _UsedByData;
		_UsedByData.isFillPattern = it.isFillPattern;
		_UsedByData.sceneName = it.sceneName;
		_UsedByData.songName = it.songName;
		_UsedByData.trackName = it.trackName;
		patternData.usedByList.push_back(_UsedByData);
	}
	patternData.UsedByCount = (uint32_t)patternData.usedByList.size();

	for (auto& it : _clientPatternData.BarDataList)
	{
		barData = { 0 };
		barData.BarId = static_cast<uint8_t>(it.second.BarId);
		barData.stepCount = it.second.stepCount;
		barData.barRepeatCount = it.second.barRepeatCount;
		barData.barTransposeDefeat = it.second.barTransposeDefeat;
		barData.gBarSyncOn = it.second.gBarSyncOn;

		for (auto& stepit : it.second.StepDataList)
		{
			stepData = { 0 };
			stepData = stepit.second;
			StepDataList.push_back(stepData);
		}
		StepDataListBars.push_back(StepDataList);
		StepDataList.clear();
		BarDataList.push_back(barData);
	}

	pattern->UpdatePattern(patternData, &BarDataList, &StepDataListBars);
}


bool PatternManager::PatternRotate(std::string patternName, uint16_t BarID, uint16_t StepID)
{
	ClientPatternData _clientPatternData, PatternRotatedData;

	UpdateUndoList(patternName, PatternDataUpdateReasonType::PatternDataUpdateReason_PatternRotate);
	//_clientPatternData = PatternMngrGetPatternData (patternName);
	_clientPatternData = UndoDataList.begin()->_clientPatternData; // Copy last saved data from undo list, fast access

	if (_clientPatternData.patternName != patternName) // if not active
		return false;

	// Take copy of all data, then update the required data
	PatternRotatedData = _clientPatternData;

	uint8_t BarIdDst = 0;
	uint8_t BarIdSorce = (uint8_t)BarID;
	uint8_t BarListSize =(uint8_t)_clientPatternData.BarDataList.size();

	//BAR Rotate
	// 1st copy bar data and steplist from Sorce ID : Don't change BAR ID or Step here
	for (; BarIdSorce < BarListSize; BarIdSorce++)
	{
		PatternRotatedData.BarDataList[BarIdDst].StepDataList = _clientPatternData.BarDataList[BarIdSorce].StepDataList;
		PatternRotatedData.BarDataList[BarIdDst].stepCount = _clientPatternData.BarDataList[BarIdSorce].stepCount;
		PatternRotatedData.BarDataList[BarIdDst].barRepeatCount = _clientPatternData.BarDataList[BarIdSorce].barRepeatCount;
		PatternRotatedData.BarDataList[BarIdDst].barTransposeDefeat = _clientPatternData.BarDataList[BarIdSorce].barTransposeDefeat;
		PatternRotatedData.BarDataList[BarIdDst].gBarSyncOn = _clientPatternData.BarDataList[BarIdSorce].gBarSyncOn;
		BarIdDst++;
	}
	// Copy remaining from top BAR, source to Dst
	BarListSize = BarListSize - BarIdDst;
	for (BarIdSorce = 0; BarIdSorce < BarListSize; BarIdSorce++)
	{
		PatternRotatedData.BarDataList[BarIdDst].StepDataList = _clientPatternData.BarDataList[BarIdSorce].StepDataList;
		PatternRotatedData.BarDataList[BarIdDst].stepCount = _clientPatternData.BarDataList[BarIdSorce].stepCount;
		PatternRotatedData.BarDataList[BarIdDst].barRepeatCount = _clientPatternData.BarDataList[BarIdSorce].barRepeatCount;
		PatternRotatedData.BarDataList[BarIdDst].barTransposeDefeat = _clientPatternData.BarDataList[BarIdSorce].barTransposeDefeat;
		PatternRotatedData.BarDataList[BarIdDst].gBarSyncOn = _clientPatternData.BarDataList[BarIdSorce].gBarSyncOn;
		BarIdDst++;
	}
	_clientPatternData = PatternRotatedData; // Bar rotated; Make a work Copy
	//Step Rotate
	{
		uint16_t step = 0, StepSrc = StepID;
		uint8_t bar = 0;
		uint16_t StepsIdForCurrentbar = 0;
		bool OkToContinue = true;

		BarIdSorce = 0;
		BarListSize = (uint8_t)_clientPatternData.BarDataList.size();

		for (bar = 0; bar < BarListSize && OkToContinue; bar++) // Move step UP
		{
			PatternRotatedData.BarDataList[bar].StepDataList.clear();
			for (step = 0; step < MAX_STEPS_IN_PATTERN_BAR && OkToContinue; step++)
			{
				PatternRotatedData.BarDataList[bar].StepDataList[step] = _clientPatternData.BarDataList[BarIdSorce].StepDataList[StepSrc];
				PatternRotatedData.BarDataList[bar].StepDataList[step].stepId = StepsIdForCurrentbar;
				PatternRotatedData.BarDataList[bar].StepDataList[step].BarId = bar;
				StepSrc++;
				StepsIdForCurrentbar++;

				if (StepSrc == MAX_STEPS_IN_PATTERN_BAR)
				{
					BarIdSorce++;
					StepSrc = 0;
					OkToContinue = (BarIdSorce < BarListSize);
				}
			}

		}
		bar--;
		StepSrc = 0; // from the top bar
		if (step != MAX_STEPS_IN_PATTERN_BAR) //rotate the Top step to bottom last bar
		{
			for (/* continue from above loop*/; step < MAX_STEPS_IN_PATTERN_BAR; step++)
			{
				PatternRotatedData.BarDataList[bar].StepDataList[step] = _clientPatternData.BarDataList[0].StepDataList[StepSrc];
				PatternRotatedData.BarDataList[bar].StepDataList[step].stepId = StepsIdForCurrentbar;
				PatternRotatedData.BarDataList[bar].StepDataList[step].BarId = bar;
				StepSrc++;
				StepsIdForCurrentbar++;
			}
		}
	}
	for (auto it = patternList.begin(); it != patternList.end(); ++it) // Remove and recreate
	{
		if (it->_patternData->patternName == PatternRotatedData.patternName)
		{
			//Create and copy new pattern, note that, track using Pattern pointer for referance
			(*it) = CreatePattern(PatternRotatedData);

			// Update Seq manager. So re-calculate the sequence, once this function is called.
			// Use numBar param, as it updates all 3 (bar params, seq order and event updates)
			for (auto& listener : _patternListeners)
			{
				listener->handleBarCountChange(it->_patternData->numBars, patternName);
			}

			break;
		}
	}
	return true;
}
Pattern PatternManager::CreatePattern(ClientPatternData _clientPatternData)
{
	PatternData patternData = {};
	StepData stepData = {};
	NodeData nodeData = {};
	std::list<StepData> StepDataList;
	std::list<std::list<StepData>> StepDataListBars;
	BarData barData = {};
	std::list<BarData> BarDataList;

	patternData.numBars = _clientPatternData.numBars;
	patternData.timeBase = mapUiIndexToTb(_clientPatternData.timeBase);
	patternData.patternName = _clientPatternData.patternName;
	patternData.patternDispName = _clientPatternData.patternDispName;
	patternData.UsedByCount = 0;
	patternData.patternLastStepId = _clientPatternData.patternLastStepId;
	patternData.id = _clientPatternData.id;
	patternData._barLoopOn = _clientPatternData.barLoopOn;
	patternData._barLoopStartId = _clientPatternData.barLoopStartId;
	patternData._barLoopEndId = _clientPatternData.barLoopEndId;

	for (auto& it : _clientPatternData.usedByList)
	{
		UsedByData _UsedByData;
		_UsedByData.isFillPattern = it.isFillPattern;
		_UsedByData.sceneName = it.sceneName;
		_UsedByData.songName = it.songName;
		_UsedByData.trackName = it.trackName;
		patternData.usedByList.push_back(_UsedByData);
	}
	patternData.UsedByCount = (uint32_t)patternData.usedByList.size();

	for (auto& it : _clientPatternData.BarDataList)
	{
		barData = { 0 };
		barData.BarId = static_cast<uint8_t>(it.second.BarId);
		barData.stepCount = it.second.stepCount;
		barData.barRepeatCount = it.second.barRepeatCount;
		barData.barTransposeDefeat = it.second.barTransposeDefeat;
		barData.gBarSyncOn = it.second.gBarSyncOn;

		for (auto& stepit : it.second.StepDataList)
		{
			stepData = { 0 };
			stepData = stepit.second;
			StepDataList.push_back(stepData);
		}
		StepDataListBars.push_back(StepDataList);
		StepDataList.clear();
		BarDataList.push_back(barData);
	}
	return Pattern(patternData, &BarDataList, &StepDataListBars, this);
}


bool PatternManager::copyPattern(std::string SrcpatternName, std::string DstpatternName)
{
	modifyPatternNameIfDuplicate(DstpatternName);
	Pattern tempPattern(*(GetPattern(SrcpatternName)));

	tempPattern._patternData->patternName = DstpatternName;
	tempPattern._patternData->patternDispName = DstpatternName;

	numberOfPatterns++;

	//Remove any track links from newly created pattern
	tempPattern._patternData->UsedByCount = 0;
	tempPattern._patternData->usedByList.clear();

	patternList.push_back(tempPattern);
	SavePattern(&tempPattern, true);
	return true;
}

void PatternManager::movePattern(uint64_t positionData)
{
	bool saveInfo = false;
#if 0
	int32_t curId = getPatternId(editPatternName);
	int32_t newId = positionData;
#else
	uint32_t curId = (positionData & 0x3FF00000000) >> 32;
	uint32_t newId = positionData & 0x3FF;
#endif
	for (auto& pattern : patternList)
	{
		saveInfo = false;
		// If movement is +/- 1 position, LIST sort does not work as required.
		// So in this case which is like swapping, also update the id of the song,
		// which is swapping its position with current song.
		// For other cases when movement is greater than +/- 1 positions,
		// only update the moving song's id, to new id.

		if ((std::abs(int(curId - newId)) == 1) && (pattern.getId() == (int32_t)newId))
		{
			//assign cur song id to the other (adjacent) song
			pattern.setId(curId);
			saveInfo = true;
		}
		else if ((newId < curId) && (pattern.getId() == (int32_t)newId))
		{
			pattern.setId(newId + 1);
			saveInfo = true;
		}
		else if (pattern.getId() == (int32_t)curId)
		{
			pattern.setId(newId);
			saveInfo = true;
		}
		if(saveInfo)
			savePatternInfo (pattern.getName (), *(pattern._patternData));
	}

	sortPatternsById();
	reAssignSequentialIdsForPatternsAndSave();
}

int32_t PatternManager::getPatternId(std::string patternName)
{
	auto pattern = GetPattern(patternName);

	if (pattern == nullptr)
	{
		return -1;
	}

	return pattern->getId();
}

struct sortPatternCompareFunction
{
	bool operator() (Pattern& first, Pattern& second)
	{
		return (first.getId() < second.getId());
	}
};

void PatternManager::sortPatternsById(void)
{
	patternList.sort(sortPatternCompareFunction());
}

void PatternManager::reAssignSequentialIdsForPatternsAndSave(void)
{
	//Re-assign ids starting from 0
	int32_t count = 0;
	for (auto& patternIter : patternList)
	{
		if (patternIter.getId () != count)
		{
			patternIter.setId (count);

			//save to FS
			savePatternInfo (patternIter.getName (), *(patternIter._patternData));
		}
		++count;
	}
}

void PatternManager::savePatternInfo(std::string name, PatternData& data)
{
	engine.fileHandler->updatePatternInfo(name, data);
}
int32_t PatternManager::getPatternCount (void)
{
	return  numberOfPatterns;
}
Pattern* PatternManager::GetPatternFromIndex (int32_t index)
{
	for (auto patternIt = patternList.begin (); patternIt != patternList.end (); ++patternIt)
	{
		if ((*patternIt)._patternData->id == index)
			return &(*patternIt);
	}
	return nullptr;
}
void PatternManager::PatternMngrPowerOnHousekeeping ()
{
#if defined DISABLE_USED_BY_LIST
	for (auto patternIt = patternList.begin (); patternIt != patternList.end (); ++patternIt)
	{
		bool SaveFile = false;
		for (auto it = (*patternIt)._patternData->usedByList.begin (); it != (*patternIt)._patternData->usedByList.end ();)
		{
			bool valid = false;
			valid = engine.trackManager->IsPatternAssigned
						(it->isFillPattern, (*patternIt)._patternData->patternName, it->songName, it->sceneName, it->trackName);
			if (!valid)
			{
				it = (*patternIt)._patternData->usedByList.erase (it);
				SaveFile = true;
			}
			else
				++it;
		}
		if (SaveFile)
		{
			engine.fileHandler->Createpattern ((*patternIt)._patternData->patternName, *patternIt->_patternData);
		}
	}
#endif//DISABLE_USED_BY_LIST
}

void PatternManager::addStepListener (StepChangeListener* listener)
{
	if (listener)
	{
		_stepListeners.insert (listener);
	}
}

void PatternManager::addBarListener(BarChangeListener* listener)
{
	if (listener)
	{
		_barListeners.insert(listener);
	}
}

void PatternManager::addPatternListener(PatternChangeListener* listener)
{
	if (listener)
	{
		_patternListeners.insert(listener);
	}
}

TimeBase PatternManager::mapUiIndexToTb(uint8_t index)
{
	return static_cast<TimeBase>(10 - index);
}

uint8_t PatternManager::mapTbToUiIndex(TimeBase tb)
{
	return 10 - static_cast<uint8_t>(tb);
}

float PatternManager::getTickScalingFactor(TimeBase old, TimeBase next)
{
	//switch to default tb
	float tb4Scaler = 1 / timeBaseScaler[old];

	return tb4Scaler * timeBaseScaler[next];
}

void PatternManager::updateLengthAndOffsetOnTbChange(ClientPatternData* clientPatternData, TimeBase old, TimeBase next)
{
	float scalingFactor = getTickScalingFactor(old, next);

	for (auto& bar : clientPatternData->BarDataList)
	{
		for (auto& step : bar.second.StepDataList)
		{
			for (auto& pitchONodePair : step.second.nodeList)
			{
				std::unordered_map<TickOffset_t, ClientPatternNodeData> tmpOffsetNodeMap;

				for (auto& offsetNodePair : pitchONodePair.second)
				{
					int8_t scaledOffset = static_cast<int8_t>(offsetNodePair.first * scalingFactor);

					tmpOffsetNodeMap[scaledOffset] = offsetNodePair.second;

					tmpOffsetNodeMap[scaledOffset].lenInTicks = static_cast<TickLength_t>(tmpOffsetNodeMap[scaledOffset].lenInTicks * scalingFactor);

					tmpOffsetNodeMap[scaledOffset].offset = static_cast<TickOffset_t>(tmpOffsetNodeMap[scaledOffset].offset * scalingFactor);
				}

				pitchONodePair.second = tmpOffsetNodeMap;
			}
		}
	}
}
