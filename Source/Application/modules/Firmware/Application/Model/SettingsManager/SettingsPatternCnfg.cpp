#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsPatternCnfg.h"

using namespace Ignite;

SettingsPatternCnfg::SettingsPatternCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsPatternCnfg", (moduleId_t)ModuleName::moduleSettingsPatternCnfg, (unsigned int)SettingsPatternCnfgParam::paramSettingsPatternCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsPatternCnfgParam::SettingPatternAutoSaveParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsPatternCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsPatternCnfgParam::SettingsPatternCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsPatternCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsPatternCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<bool> ((paramId_t)SettingsPatternCnfgParam::paramPatternBarFollow, 0, 1, 0);
	AddParameter<int16_t> ((paramId_t)SettingsPatternCnfgParam::paramPatternDefaultNote, static_cast<int16_t>(Note::noteC0), static_cast<int16_t>(Note::noteG10), static_cast<int16_t>(Note::noteC4));
	AddParameter<int8_t>((paramId_t)SettingsPatternCnfgParam::paramInsertPosition, static_cast <int8_t> (InsertPosition::listEnd), static_cast <int8_t> (InsertPosition::nextToActiveRow), static_cast <int8_t> (InsertPosition::listEnd));
	AddParameter<int8_t>((paramId_t)SettingsPatternCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->GetSettingsPatternCnfgDataSetting (&_SettingsPatternCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsPatternCnfgData (_SettingsPatternCnfgData, ""); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsPatternCnfg, true);
	SetDefaultUndoListData ();
}
void SettingsPatternCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsPatternCnfgParam::paramall + 1; i < (paramId_t)SettingsPatternCnfgParam::paramSettingsPatternCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsPatternCnfgParam::SettingsPatternCnfgSave) ||
			(i == (paramId_t)SettingsPatternCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsPatternCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsPatternCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsPatternCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsPatternCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsPatternCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsPatternCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsPatternCnfgParam)(*it).param)
		{
			case SettingsPatternCnfgParam::SettingPatternAutoSaveParam:
				paramData.param = (paramId_t)SettingsPatternCnfgParam::SettingPatternAutoSaveParam;
				paramData.MsgValue = _SettingsPatternCnfgData.PatternAutoSaveParam;
				retParam.push_back (paramData);
				break;

			case SettingsPatternCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsPatternCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsPatternCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsPatternCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();
				paramData.param = (paramId_t)SettingsPatternCnfgParam::paramUndoListCmd;
				paramData.Count = (uint32_t)undoList.size (); //set the number of params
				retParam.push_back (paramData);
				for (auto itr = undoList.begin (); itr < undoList.end (); ++itr)
				{
					paramData.param = *itr;
					paramData.Count = Count;
					Count++;
					retParam.push_back (paramData);
				}
				break;
			}

			case SettingsPatternCnfgParam::paramPatternBarFollow:
				paramData.param = (paramId_t)SettingsPatternCnfgParam::paramPatternBarFollow;
				paramData.MsgValue = _SettingsPatternCnfgData.PatternBarFollowState;
				retParam.push_back (paramData);
				break;

			case SettingsPatternCnfgParam::paramPatternDefaultNote:
				paramData.param = (paramId_t)SettingsPatternCnfgParam::paramPatternDefaultNote;
				paramData.MsgValue = _SettingsPatternCnfgData.patternDefaultNote;
				retParam.push_back (paramData);
				break;
			case SettingsPatternCnfgParam::paramInsertPosition:
				paramData.param = (paramId_t)SettingsPatternCnfgParam::paramInsertPosition;
				paramData.MsgValue = _SettingsPatternCnfgData.patternInsertPosition;
				retParam.push_back(paramData);
				break;
		}
	}

	return  retParam;
}
void SettingsPatternCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsPatternCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsPatternCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsPatternCnfgParam)param->GetParamID () != SettingsPatternCnfgParam::SettingsPatternCnfgSave &&
		(SettingsPatternCnfgParam)param->GetParamID () != SettingsPatternCnfgParam::paramSaveAll &&
		(SettingsPatternCnfgParam)param->GetParamID () != SettingsPatternCnfgParam::SettingDataLocation &&
		(SettingsPatternCnfgParam)param->GetParamID () != SettingsPatternCnfgParam::paramUndoListCmd &&
		(SettingsPatternCnfgParam)param->GetParamID () != SettingsPatternCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsPatternCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsPatternCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
	switch ((SettingsPatternCnfgParam)param->GetParamID ())
	{
		case SettingsPatternCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsPatternCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsPatternCnfgParam::SettingsPatternCnfgSave:
			_SettingsPatternCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
			break;
		case SettingsPatternCnfgParam::SettingPatternAutoSaveParam:
			_SettingsPatternCnfgData.PatternAutoSaveParam = (param->getValue ().get<int8_t> ());
			break;
		case SettingsPatternCnfgParam::SettingDataLocation:
			_SettingsPatternCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsPatternCnfgDataSetting (&_SettingsPatternCnfgData, (_SettingsPatternCnfgData.DataLocation ? getActiveSongName () : ""));
			break;

		case SettingsPatternCnfgParam::paramPatternBarFollow:
			_SettingsPatternCnfgData.PatternBarFollowState = param->getValue ().get<bool> ();
			break;

		case SettingsPatternCnfgParam::paramPatternDefaultNote:
			_SettingsPatternCnfgData.patternDefaultNote = param->getValue ().get<int16> ();
			break;

		case SettingsPatternCnfgParam::paramInsertPosition:
			_SettingsPatternCnfgData.patternInsertPosition = param->getValue().get<int8_t>();
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
void SettingsPatternCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsPatternCnfgData(_SettingsPatternCnfgData, (_SettingsPatternCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}
// Explicit desktop save does not change the selected storage location.
bool SettingsPatternCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsPatternCnfgData(_SettingsPatternCnfgData, (_SettingsPatternCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
