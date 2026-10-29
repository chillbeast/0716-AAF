#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsUdsCnfg.h"

using namespace Ignite;

SettingsUdsCnfg::SettingsUdsCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsUdsCnfg", (moduleId_t)ModuleName::moduleSettingsUdsCnfg, (unsigned int)SettingsUdsCnfgParam::paramSettingsUdsCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::SettingsUdsCnfgSaveParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsUdsCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsUdsCnfgParam::paramClearUndoList, 0, 127, 0);
	AddParameter<bool> ((paramId_t)SettingsUdsCnfgParam::paramAddUds, true, true, true);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramActiveUds, 0, MAX_NUM_UDS - 1, 0);
	AddParameter<std::string> ((int8_t)SettingsUdsCnfgParam::paramUdsName, "0", "0", "0");
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNameList, 0, MAX_NUM_UDS - 1, 0);
	AddParameter<bool> ((paramId_t)SettingsUdsCnfgParam::paramRemoveUds, true, true, true);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote1, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote2, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote3, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote4, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote5, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote6, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote7, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote8, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote9, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote10, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote11, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsNote12, -1, 11, -1);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsConfigDone, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsUdsCnfgParam::paramUdsCount, 0, MAX_NUM_UDS, 0);
	AddParameter<int8_t>((paramId_t)SettingsUdsCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->GetSettingsUdsCnfgDataSetting (&_settingsUdsCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsUdsCnfgData (_settingsUdsCnfgData, ""); // create with default data
	}
	else //some data present in file system
	{
		//populate user defined scales into global map
		if (_settingsUdsCnfgData.udsCount > 0)
		{
			for (auto udsIndex = 0; udsIndex < _settingsUdsCnfgData.udsCount; ++udsIndex)
			{
				if (_settingsUdsCnfgData.udsConfig[udsIndex].udsNoteList.size () > 0)
				{
					auto noteIntervalList = generateNoteIntervalList (_settingsUdsCnfgData.udsConfig[udsIndex].udsNoteList);
					updateUdsToGlobalScaleList ((int8_t)udsIndex, noteIntervalList, _settingsUdsCnfgData.udsConfig[udsIndex].udsName);
				}
			}
		}
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsUdsCnfg, true);
	SetDefaultUndoListData ();
}
void SettingsUdsCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsUdsCnfgParam::paramall + 1; i < (paramId_t)SettingsUdsCnfgParam::paramSettingsUdsCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsUdsCnfgParam::SettingsUdsCnfgSaveParam) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsUdsCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramClearUndoList) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramActiveUds) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramUndoCmd) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramRemoveUds) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramUdsCount) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramUdsConfigDone) ||
			(i == (paramId_t)SettingsUdsCnfgParam::paramUdsNameList)
			)
		{
			//ignore command params, used in get param getParamList
		}
		else
		{
			AddToUndoDefaultList ((paramId_t)i, FindParameter ((paramId_t)i)->getValue ());
			paramlistdata.param = (paramId_t)i;
			paramlist.push_back (paramlistdata);
		}
	}
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsUdsCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsUdsCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsUdsCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsUdsCnfgParam)(*it).param)
		{
			case SettingsUdsCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::SettingDataLocation;
				paramData.MsgValue = _settingsUdsCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;
			case SettingsUdsCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();

				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUndoListCmd;
				paramData.Count = (uint32_t)undoList.size (); //set the number of params
				retParam.push_back (paramData);

				for (auto itlist = undoList.begin (); itlist < undoList.end (); ++itlist)
				{
					paramData.param = *itlist;
					paramData.Count = Count;
					Count++;
					retParam.push_back (paramData);
				}
			}
			break;
			case SettingsUdsCnfgParam::paramActiveUds:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramActiveUds;
				paramData.MsgValue = activeUds;
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsName:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsName;
				paramData.MsgValue = getUdsName();
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNameList:
			{
				for (auto udsIndex = 0; udsIndex < _settingsUdsCnfgData.udsCount; ++udsIndex)
				{
					paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNameList;
					paramData.MsgValue = _settingsUdsCnfgData.udsConfig[udsIndex].udsName;
					paramData.Count = udsIndex;
					retParam.push_back (paramData);
				}
			}
			break;
			case SettingsUdsCnfgParam::paramUdsCount:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsCount;
				paramData.MsgValue = _settingsUdsCnfgData.udsCount;
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote1:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote1;
				paramData.MsgValue = getUdsNote(0);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote2:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote2;
				paramData.MsgValue = getUdsNote (1);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote3:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote3;
				paramData.MsgValue = getUdsNote (2);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote4:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote4;
				paramData.MsgValue = getUdsNote (3);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote5:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote5;
				paramData.MsgValue = getUdsNote (4);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote6:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote6;
				paramData.MsgValue = getUdsNote (5);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote7:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote7;
				paramData.MsgValue = getUdsNote (6);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote8:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote8;
				paramData.MsgValue = getUdsNote (7);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote9:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote9;
				paramData.MsgValue = getUdsNote (8);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote10:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote10;
				paramData.MsgValue = getUdsNote (9);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote11:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote11;
				paramData.MsgValue = getUdsNote (10);
				retParam.push_back(paramData);
				break;
			case SettingsUdsCnfgParam::paramUdsNote12:
				paramData.param = (paramId_t)SettingsUdsCnfgParam::paramUdsNote12;
				paramData.MsgValue = getUdsNote (11);
				retParam.push_back (paramData);
				break;
		}
	}

	return  retParam;
}
void SettingsUdsCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsUdsCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}

void SettingsUdsCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::SettingsUdsCnfgSaveParam &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::paramSaveAll &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::SettingDataLocation &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::paramUndoListCmd &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::paramClearUndoList &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::paramActiveUds &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::paramRemoveUds &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::paramUdsCount &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::paramUdsConfigDone &&
		(SettingsUdsCnfgParam)param->GetParamID () != SettingsUdsCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsUdsCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsUdsCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}

	switch ((SettingsUdsCnfgParam)param->GetParamID ())
	{
		case SettingsUdsCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsUdsCnfgParam::paramClearUndoList:
			if (param->getValue().get<int8_t>() == 0)
				ResetToDefaultData();
			ClearUndoList ();
			//engine.fileHandler->GetSettingsUdsCnfgDataSetting (&_settingsUdsCnfgData, ""); // Load saved data
			break;
		case SettingsUdsCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsUdsCnfgParam::SettingsUdsCnfgSaveParam:
			_settingsUdsCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
            break;
		case SettingsUdsCnfgParam::SettingDataLocation:
			_settingsUdsCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsUdsCnfgDataSetting (&_settingsUdsCnfgData, (_settingsUdsCnfgData.DataLocation ? getActiveSongName () : ""));
			break;
		case SettingsUdsCnfgParam::paramActiveUds:
			setActiveUds (param->getValue().get<int8_t>());
			break;
		case SettingsUdsCnfgParam::paramAddUds:
			if (_settingsUdsCnfgData.udsCount < MAX_NUM_UDS)
			{
				++(_settingsUdsCnfgData.udsCount);
				UDS temp;
				for (auto i = 0; i < NUM_UDS_NODES; i++)
					temp.udsNoteList.push_back (-1); // init with invalide data
				temp.udsName = ("UDS" + std::to_string (_settingsUdsCnfgData.udsCount));
				_settingsUdsCnfgData.udsConfig.push_back (temp);
				engine.fileHandler->UpdateSettingsUdsCnfgData (_settingsUdsCnfgData, (_settingsUdsCnfgData.DataLocation ? getActiveSongName () : ""));
			}
			break;

		case SettingsUdsCnfgParam::paramUdsName:
			setUdsName (param->getValue ().get<std::string> ());
			break;

		case SettingsUdsCnfgParam::paramRemoveUds:
			if (_settingsUdsCnfgData.udsCount)
			{
				int index = 0;
				for (auto it = _settingsUdsCnfgData.udsConfig.begin (); it != _settingsUdsCnfgData.udsConfig.end (); it++)
				{
					if (index == activeUds)
					{
						_settingsUdsCnfgData.udsConfig.erase (it);
						break;
					}
						
					index++;
				}
				_settingsUdsCnfgData.udsCount--;
				engine.fileHandler->UpdateSettingsUdsCnfgData (_settingsUdsCnfgData, (_settingsUdsCnfgData.DataLocation ? getActiveSongName () : ""));
			}
			break;
		case SettingsUdsCnfgParam::paramUdsNote1:
			setUdsNote (0, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote2:
			setUdsNote (1, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote3:
			setUdsNote (2, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote4:
			setUdsNote (3, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote5:
			setUdsNote (4, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote6:
			setUdsNote (5, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote7:
			setUdsNote (6, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote8:
			setUdsNote (7, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote9:
			setUdsNote (8, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote10:
			setUdsNote (9, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote11:
			setUdsNote (10, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsNote12:
			setUdsNote (11, param->getValue ().get<int8_t> ());
			break;
		case SettingsUdsCnfgParam::paramUdsConfigDone:
			int8_t udsConfigDone;
			udsConfigDone = (param->getValue().get<int8_t>());

			if (udsConfigDone == 1)
			{
				if (_settingsUdsCnfgData.udsConfig[activeUds].udsNoteList.size () > 0)
				{
					auto intervalList = generateNoteIntervalList (_settingsUdsCnfgData.udsConfig[activeUds].udsNoteList);
					updateUdsToGlobalScaleList (activeUds, intervalList, _settingsUdsCnfgData.udsConfig[activeUds].udsName);
				}
			}
			break;
		default:
			assert (0);
			break;
	}
	if (engine.getAutoSaveStatus())
	{
		SaveData();
	}
	if (SendNotification)
	{
		/* Send notification to the subscribed modules via engine */
		paramlistener->onParameterChanged (moduleSrc, param);
	}
}
void SettingsUdsCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsUdsCnfgData(_settingsUdsCnfgData, (_settingsUdsCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}
void SettingsUdsCnfg::setActiveUds (int8_t index)
{
	if ((index >= 0) && (index < MAX_NUM_UDS))
	{
		isUndilistNeedUpdate = (activeUds != index);
		activeUds = index;
	}
	else
	{
		assert(0);
	}
}

int8_t SettingsUdsCnfg::getActiveUds(void)
{
	return activeUds;
}

std::string SettingsUdsCnfg::getUdsName(void)
{
	auto activeUdsIndex = activeUds;
	if ((activeUdsIndex >= 0) && (activeUdsIndex < _settingsUdsCnfgData.udsConfig.size ()))
	{
		return _settingsUdsCnfgData.udsConfig[activeUdsIndex].udsName;
	}
	else
	{
		return "INVALID";
	}
}

void SettingsUdsCnfg::setUdsName (std::string name)
{
	auto activeUdsIndex = activeUds;
	if ((activeUdsIndex >= 0) && (activeUdsIndex < _settingsUdsCnfgData.udsConfig.size ()))
	{
		_settingsUdsCnfgData.udsConfig[activeUds].udsName = name;
		updateUdsNameToGlobalScaleList (activeUdsIndex, name);
	}
	else
	{
		assert(0);
	}
}

int8_t SettingsUdsCnfg::getUdsCount(void)
{
	return _settingsUdsCnfgData.udsCount;
}

int8_t SettingsUdsCnfg::getUdsNote (int8_t index)
{
	auto activeUdsIndex = activeUds;
	if ((activeUdsIndex >= 0) && (activeUdsIndex < _settingsUdsCnfgData.udsConfig.size ()) && (_settingsUdsCnfgData.udsConfig[activeUdsIndex].udsNoteList.size () > index))
	{
		return _settingsUdsCnfgData.udsConfig[activeUdsIndex].udsNoteList[index];
	}
	else
	{
		return -1;
	}
}

void SettingsUdsCnfg::setUdsNote (int8_t notePosition, int8_t note)
{
	auto activeUdsIndex = activeUds;

	if ((activeUdsIndex >= 0) && (activeUdsIndex < _settingsUdsCnfgData.udsConfig.size ()))
	{
		if (_settingsUdsCnfgData.udsConfig[activeUdsIndex].udsNoteList.size () >= notePosition)
		{
			_settingsUdsCnfgData.udsConfig[activeUdsIndex].udsNoteList[notePosition] = note;
		}
		else
		{
			_settingsUdsCnfgData.udsConfig[activeUdsIndex].udsNoteList.push_back (note);
		}
	}
	else
	{
		assert(0);
	}
}

std::vector <int8_t> SettingsUdsCnfg::generateNoteIntervalList (std::vector <int8_t> noteList)
{
	int8_t noteInterval = -1;
	std::vector <int8_t> noteIntervalList;

	noteIntervalList.clear ();

	auto currentNote = noteList[0];

	auto noteIter = noteList.begin ();
	auto noteIterEnd = noteList.end ();

	//start loop from second note in the list
	for (std::advance (noteIter, 1); noteIter != noteIterEnd; ++noteIter)
	{
		auto nextNote = *noteIter;

		if (nextNote == -1)
		{
			break;
		}

		if (currentNote > nextNote)
		{
			noteInterval = static_cast<int8_t>(FTSRootNote::NoteMax) - currentNote + nextNote + 1;
		}
		else
		{
			noteInterval = nextNote - currentNote;
		}
		noteIntervalList.push_back(noteInterval);
		currentNote = nextNote;
	}
	return noteIntervalList;
}

void SettingsUdsCnfg::updateUdsToGlobalScaleList (int8_t index, std::vector <int8_t> intervalList, std::string name)
{
	updateUdsNameToGlobalScaleList (index, name);
	updateUdsNotesToGlobalScaleList (index, intervalList);
}

void SettingsUdsCnfg::updateUdsNameToGlobalScaleList (int8_t index, std::string name)
{
	//index runs from 0 to 9
	//in global map, start adding from offset ScaleList::ScaleUD1
	auto position = static_cast <ScaleList> (static_cast<int8_t> (ScaleList::ScaleUD1) + index);

	ftsList[position].name = name;
}

void SettingsUdsCnfg::updateUdsNotesToGlobalScaleList (int8_t index, std::vector <int8_t> intervalList)
{
	//index runs from 0 to 9
	//in global map, start adding from offset ScaleList::ScaleUD1
	auto position = static_cast <ScaleList> (static_cast<int8_t> (ScaleList::ScaleUD1) + index);

	ftsList[position].scaleIntervalList = intervalList;
}

// Explicit desktop save does not change the selected storage location.
bool SettingsUdsCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsUdsCnfgData(_settingsUdsCnfgData, (_settingsUdsCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
