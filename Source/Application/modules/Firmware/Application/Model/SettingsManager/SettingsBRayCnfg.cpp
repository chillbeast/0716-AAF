#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsBRayCnfg.h"

using namespace Ignite;

SettingsBRayCnfg::SettingsBRayCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsBRayCnfg", (moduleId_t)ModuleName::moduleSettingsBRayCnfg, (unsigned int)SettingsBRayCnfgParam::paramSettingsBrayCnfgMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsBRayCnfgParam::paramBrayPowerStatus, 0, 1, 0);
	AddParameter<uint16_t> ((paramId_t)SettingsBRayCnfgParam::paramBrayAdcValue, 0, 0xFFFF, 0);
	AddParameter<int8_t> ((paramId_t)SettingsBRayCnfgParam::paramBrayControlledParam, 0, (int8_t)BRayControllSetting::brayControlSettingMax, 0);
	AddParameter<int8_t> ((paramId_t)SettingsBRayCnfgParam::paramBrayCcNum, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsBRayCnfgParam::paramBRayCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsBRayCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsBRayCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsBRayCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsBRayCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->GetSettingsBRayCnfgDataSetting (&_settingsBRayCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsBRayCnfgData (_settingsBRayCnfgData, ""); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsBRayCnfg, true);
	SetDefaultUndoListData ();
}
void SettingsBRayCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsBRayCnfgParam::paramall + 1; i < (paramId_t)SettingsBRayCnfgParam::paramSettingsBrayCnfgMax; i++)
	{
		if ((i == (paramId_t)SettingsBRayCnfgParam::paramBRayCnfgSave) ||
			(i == (paramId_t)SettingsBRayCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsBRayCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsBRayCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsBRayCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsBRayCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsBRayCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsBRayCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsBRayCnfgParam)(*it).param)
		{
			case SettingsBRayCnfgParam::paramBrayPowerStatus:
				paramData.param = (paramId_t)SettingsBRayCnfgParam::paramBrayPowerStatus;
				paramData.MsgValue = _settingsBRayCnfgData.brayPowerStatus;
				retParam.push_back (paramData);
				break;

			case SettingsBRayCnfgParam::paramBrayAdcValue:
				paramData.param = (paramId_t)SettingsBRayCnfgParam::paramBrayAdcValue;
				paramData.MsgValue = brayAdcValue;
				retParam.push_back (paramData);
				break;

			case SettingsBRayCnfgParam::paramBrayCcNum:
				paramData.param = (paramId_t)SettingsBRayCnfgParam::paramBrayCcNum;
				paramData.MsgValue = _settingsBRayCnfgData.brayCcNum;
				retParam.push_back (paramData);
				break;

			case SettingsBRayCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsBRayCnfgParam::SettingDataLocation;
				paramData.MsgValue = _settingsBRayCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;
			case SettingsBRayCnfgParam::paramBrayControlledParam:
				paramData.param = (paramId_t)SettingsBRayCnfgParam::paramBrayControlledParam;
				paramData.MsgValue = (int8_t)_settingsBRayCnfgData.brayControlledParam;
				retParam.push_back (paramData);
				break;
			case SettingsBRayCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();

				paramData.param = (paramId_t)SettingsBRayCnfgParam::paramUndoListCmd;
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
		}
	}

	return  retParam;
}
void SettingsBRayCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsBRayCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}

void SettingsBRayCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsBRayCnfgParam)param->GetParamID () != SettingsBRayCnfgParam::paramBRayCnfgSave &&
		(SettingsBRayCnfgParam)param->GetParamID () != SettingsBRayCnfgParam::paramSaveAll &&
		(SettingsBRayCnfgParam)param->GetParamID () != SettingsBRayCnfgParam::SettingDataLocation &&
		(SettingsBRayCnfgParam)param->GetParamID () != SettingsBRayCnfgParam::paramUndoListCmd &&
		(SettingsBRayCnfgParam)param->GetParamID () != SettingsBRayCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsBRayCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsBRayCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}

	switch ((SettingsBRayCnfgParam)param->GetParamID ())
	{
		case SettingsBRayCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		
		case SettingsBRayCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsBRayCnfgParam::paramBRayCnfgSave:
			_settingsBRayCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
			break;
		
		case SettingsBRayCnfgParam::SettingDataLocation:
			_settingsBRayCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsBRayCnfgDataSetting (&_settingsBRayCnfgData, (_settingsBRayCnfgData.DataLocation ? getActiveSongName () : ""));
			break;
		
		case SettingsBRayCnfgParam::paramBrayPowerStatus:
			_settingsBRayCnfgData.brayPowerStatus = (param->getValue ().get<int8_t> ());
			break;
		
		case SettingsBRayCnfgParam::paramBrayControlledParam:
			_settingsBRayCnfgData.brayControlledParam = (BRayControllSetting)(param->getValue ().get<int8_t> ());
			break;
		
		case SettingsBRayCnfgParam::paramBrayAdcValue:
			//Set from MCU
			//brayAdcValue = param->getValue ().get<uint16_t> ();
			break;

		case SettingsBRayCnfgParam::paramBrayCcNum:
			_settingsBRayCnfgData.brayCcNum = param->getValue ().get<int8_t> ();
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
void SettingsBRayCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsBRayCnfgData(_settingsBRayCnfgData, (_settingsBRayCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}

// Explicit desktop save does not change the selected storage location.
bool SettingsBRayCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsBRayCnfgData(_settingsBRayCnfgData, (_settingsBRayCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
