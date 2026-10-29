#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsTrackCnfg.h"

using namespace Ignite;

SettingsTrackCnfg::SettingsTrackCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsTrackCnfg", (moduleId_t)ModuleName::moduleSettingsTrackCnfg, (unsigned int)SettingsTrackCnfgParam::paramSettingsTrackCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsTrackCnfgParam::SettingsTrackAutoSaveParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsTrackCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsTrackCnfgParam::SettingsTrackCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsTrackCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsTrackCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsTrackCnfgParam::paramSaveAll, 0, 1, 0);
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsTrackCnfg, true);
	if (!engine.fileHandler->GetSettingsTrackCnfgDataSetting (&_SettingsTrackCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsTrackCnfgData (_SettingsTrackCnfgData, ""); // create with default data
	}
	SetDefaultUndoListData ();
}
void SettingsTrackCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsTrackCnfgParam::paramall + 1; i < (paramId_t)SettingsTrackCnfgParam::paramSettingsTrackCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsTrackCnfgParam::SettingsTrackCnfgSave) ||
			(i == (paramId_t)SettingsTrackCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsTrackCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsTrackCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsTrackCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsTrackCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsTrackCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsTrackCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsTrackCnfgParam)(*it).param)
		{
			case SettingsTrackCnfgParam::SettingsTrackAutoSaveParam:
				paramData.param = (paramId_t)SettingsTrackCnfgParam::SettingsTrackAutoSaveParam;
				paramData.MsgValue = _SettingsTrackCnfgData.TrackAutoSaveParam;
				retParam.push_back (paramData);
				break;

			case SettingsTrackCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsTrackCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsTrackCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsTrackCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();
				paramData.param = (paramId_t)SettingsTrackCnfgParam::paramUndoListCmd;
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
		}
	}

	return  retParam;
}
void SettingsTrackCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsTrackCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsTrackCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsTrackCnfgParam)param->GetParamID () != SettingsTrackCnfgParam::SettingsTrackCnfgSave &&
		(SettingsTrackCnfgParam)param->GetParamID () != SettingsTrackCnfgParam::paramSaveAll &&
		(SettingsTrackCnfgParam)param->GetParamID () != SettingsTrackCnfgParam::SettingDataLocation &&
		(SettingsTrackCnfgParam)param->GetParamID () != SettingsTrackCnfgParam::paramUndoListCmd &&
		(SettingsTrackCnfgParam)param->GetParamID () != SettingsTrackCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsTrackCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsTrackCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
	switch ((SettingsTrackCnfgParam)param->GetParamID ())
	{
		case SettingsTrackCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsTrackCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsTrackCnfgParam::SettingsTrackCnfgSave:
			SaveData();
			break;
		case SettingsTrackCnfgParam::SettingsTrackAutoSaveParam:
			_SettingsTrackCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			_SettingsTrackCnfgData.TrackAutoSaveParam = (param->getValue ().get<int8_t> ());
			break;
		case SettingsTrackCnfgParam::SettingDataLocation:
			_SettingsTrackCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsTrackCnfgDataSetting (&_SettingsTrackCnfgData, (_SettingsTrackCnfgData.DataLocation ? getActiveSongName () : ""));
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
void SettingsTrackCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsTrackCnfgData(_SettingsTrackCnfgData, (_SettingsTrackCnfgData.DataLocation && engine.songManager->activeSong ? engine.songManager->activeSong->getName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}
// Explicit desktop save does not change the selected storage location.
bool SettingsTrackCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsTrackCnfgData(_SettingsTrackCnfgData, (_SettingsTrackCnfgData.DataLocation && engine.songManager->activeSong ? engine.songManager->activeSong->getName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
