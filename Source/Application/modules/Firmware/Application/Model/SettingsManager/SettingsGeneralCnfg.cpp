#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsGeneralCnfg.h"

using namespace Ignite;

SettingsGeneralCnfg::SettingsGeneralCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsGeneralCnfg", (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (unsigned int)SettingsGeneralCnfgParam::paramSettingsGeneralCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgUSBParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSafeStopParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgBrightnessParam, 0, 100, 0);
	AddParameter<bool> ((paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgAutoSaveAll, false, true, false);
	AddParameter<bool> ((paramId_t)SettingsGeneralCnfgParam::SettingInstrumentAutoSaveParam, false, true, false);
	AddParameter<int8_t> ((paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsGeneralCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsGeneralCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset1, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset2, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset3, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset4, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset5, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset6, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset7, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset8, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset9, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset10, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int> ((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset11, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int>((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset12, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int>((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset13, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int>((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset14, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int>((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset15, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int>((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset16, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int>((paramId_t)SettingsGeneralCnfgParam::paramAuditioningPreset17, -1, 5000, PRESET_INVALID_ID);
	AddParameter<int8_t>((paramId_t)SettingsGeneralCnfgParam::paramSaveAll, 0, 1, 0);
	AddParameter<bool>((paramId_t)SettingsGeneralCnfgParam::paramWifiState, false, true, false);
	AddParameter<std::string>((paramId_t)SettingsGeneralCnfgParam::paramWifiLastConnSsid, " ", " ", " ");

	for (uint8_t i = 0; i < AUDITIONING_MAX_PORT; i++)
		_SettingsGeneralCnfgData.auditioningPortPreset[i] = PRESET_INVALID_ID;

	if (!engine.fileHandler->GetGeneralCnfgDataSetting (&_SettingsGeneralCnfgData))
	{
		engine.fileHandler->UpdateSettingsGeneralCnfgData (_SettingsGeneralCnfgData); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingGnrlCnfg, true);
	SetDefaultUndoListData ();
	engine.setAutoSaveAllModules(_SettingsGeneralCnfgData.DeviceCnfgAutoSaveAll);
}
int SettingsGeneralCnfg::getAuditioningPortPreset(uint8_t portID)
{
	return (_SettingsGeneralCnfgData.auditioningPortPreset[portID]);
}
void SettingsGeneralCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsGeneralCnfgParam::paramall + 1; i < (paramId_t)SettingsGeneralCnfgParam::paramSettingsGeneralCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSave) ||
			(i == (paramId_t)SettingsGeneralCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsGeneralCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsGeneralCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingGnrlCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsGeneralCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsGeneralCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsGeneralCnfgParam)(*it).param)
		{
		case SettingsGeneralCnfgParam::SettingDeviceCnfgUSBParam:
			paramData.param = (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgUSBParam;
			paramData.MsgValue = _SettingsGeneralCnfgData.DeviceCnfgUSBParam;
			retParam.push_back(paramData);
			break;

		case SettingsGeneralCnfgParam::SettingDeviceCnfgSafeStopParam:
			paramData.param = (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSafeStopParam;
			paramData.MsgValue = _SettingsGeneralCnfgData.DeviceCnfgSafeStopParam;
			retParam.push_back(paramData);
			break;

		case SettingsGeneralCnfgParam::SettingDeviceCnfgBrightnessParam:
			paramData.param = (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgBrightnessParam;
			paramData.MsgValue = _SettingsGeneralCnfgData.DeviceCnfgBrightnessParam;
			retParam.push_back(paramData);
			break;
		case SettingsGeneralCnfgParam::SettingDeviceCnfgAutoSaveAll:
			paramData.param = (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgAutoSaveAll;
			paramData.MsgValue = _SettingsGeneralCnfgData.DeviceCnfgAutoSaveAll;
			retParam.push_back(paramData);
			break;
		case SettingsGeneralCnfgParam::SettingInstrumentAutoSaveParam:
			paramData.param = (paramId_t)SettingsGeneralCnfgParam::SettingInstrumentAutoSaveParam;
			paramData.MsgValue = _SettingsGeneralCnfgData.InstrumentAutoSave;
			retParam.push_back(paramData);
			break;
		case SettingsGeneralCnfgParam::paramUndoListCmd:
		{
			uint8_t Count = 1;
			paramData = { 0 };
			std::vector<paramId_t> undoList = GetUndoParamsInList();
			paramData.param = (paramId_t)SettingsGeneralCnfgParam::paramUndoListCmd;
			paramData.Count = (uint32_t)undoList.size(); //set the number of params
			retParam.push_back(paramData);
			for (auto itr = undoList.begin(); itr < undoList.end(); ++itr)
			{
				paramData.param = *itr;
				paramData.Count = Count;
				Count++;
				retParam.push_back(paramData);
			}
			break;
		}
		case SettingsGeneralCnfgParam::paramWifiState:
			paramData.param = (paramId_t)SettingsGeneralCnfgParam::SettingInstrumentAutoSaveParam;
			paramData.MsgValue = _SettingsGeneralCnfgData.WifiState;
			retParam.push_back(paramData);
			break;
		case SettingsGeneralCnfgParam::paramWifiLastConnSsid:
			paramData.param = (paramId_t)SettingsGeneralCnfgParam::SettingInstrumentAutoSaveParam;
			paramData.MsgValue = _SettingsGeneralCnfgData.LastWifiConSsid;
			retParam.push_back(paramData);
			break;
		case SettingsGeneralCnfgParam::paramAuditioningPreset1:
		case SettingsGeneralCnfgParam::paramAuditioningPreset2:
		case SettingsGeneralCnfgParam::paramAuditioningPreset3:
		case SettingsGeneralCnfgParam::paramAuditioningPreset4:
		case SettingsGeneralCnfgParam::paramAuditioningPreset5:
		case SettingsGeneralCnfgParam::paramAuditioningPreset6:
		case SettingsGeneralCnfgParam::paramAuditioningPreset7:
		case SettingsGeneralCnfgParam::paramAuditioningPreset8:
		case SettingsGeneralCnfgParam::paramAuditioningPreset9:
		case SettingsGeneralCnfgParam::paramAuditioningPreset10:
		case SettingsGeneralCnfgParam::paramAuditioningPreset11:
		case SettingsGeneralCnfgParam::paramAuditioningPreset12:
		case SettingsGeneralCnfgParam::paramAuditioningPreset13:
		case SettingsGeneralCnfgParam::paramAuditioningPreset14:
		case SettingsGeneralCnfgParam::paramAuditioningPreset15:
		case SettingsGeneralCnfgParam::paramAuditioningPreset16:
		case SettingsGeneralCnfgParam::paramAuditioningPreset17:
		{
			paramData.param = (*it).param;
			uint8_t port = ((*it).param - (uint8_t)SettingsGeneralCnfgParam::paramAuditioningPreset1);
			paramData.MsgValue = getAuditioningPortPreset(port);
			retParam.push_back(paramData);
		}
			break;
		}
	}
	return  retParam;
}
void SettingsGeneralCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsGeneralCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsGeneralCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsGeneralCnfgParam)param->GetParamID () != SettingsGeneralCnfgParam::SettingDeviceCnfgSave &&
		(SettingsGeneralCnfgParam)param->GetParamID () != SettingsGeneralCnfgParam::paramSaveAll &&
		(SettingsGeneralCnfgParam)param->GetParamID () != SettingsGeneralCnfgParam::paramUndoListCmd &&
		(SettingsGeneralCnfgParam)param->GetParamID () != SettingsGeneralCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
void SettingsGeneralCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
	switch ((SettingsGeneralCnfgParam)param->GetParamID ())
	{
		case SettingsGeneralCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsGeneralCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsGeneralCnfgParam::SettingDeviceCnfgSave:
			if (engine.fileHandler->UpdateSettingsGeneralCnfgData (_SettingsGeneralCnfgData) != FILE_OK) break;
			ClearUndoList ();
			isUndilistNeedUpdate = true;
			break;

		case SettingsGeneralCnfgParam::SettingDeviceCnfgUSBParam:
			_SettingsGeneralCnfgData.DeviceCnfgUSBParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsGeneralCnfgParam::paramWifiState:
			_SettingsGeneralCnfgData.WifiState = (param->getValue().get<bool>());
			break;

		case SettingsGeneralCnfgParam::paramWifiLastConnSsid:
			_SettingsGeneralCnfgData.LastWifiConSsid = (param->getValue().get<std::string>());
			break;

		case SettingsGeneralCnfgParam::paramAuditioningPreset1:
		case SettingsGeneralCnfgParam::paramAuditioningPreset2:
		case SettingsGeneralCnfgParam::paramAuditioningPreset3:
		case SettingsGeneralCnfgParam::paramAuditioningPreset4:
		case SettingsGeneralCnfgParam::paramAuditioningPreset5:
		case SettingsGeneralCnfgParam::paramAuditioningPreset6:
		case SettingsGeneralCnfgParam::paramAuditioningPreset7:
		case SettingsGeneralCnfgParam::paramAuditioningPreset8:
		case SettingsGeneralCnfgParam::paramAuditioningPreset9:
		case SettingsGeneralCnfgParam::paramAuditioningPreset10:
		case SettingsGeneralCnfgParam::paramAuditioningPreset11:
		case SettingsGeneralCnfgParam::paramAuditioningPreset12:
		case SettingsGeneralCnfgParam::paramAuditioningPreset13:
		case SettingsGeneralCnfgParam::paramAuditioningPreset14:
		case SettingsGeneralCnfgParam::paramAuditioningPreset15:
		case SettingsGeneralCnfgParam::paramAuditioningPreset16:
		case SettingsGeneralCnfgParam::paramAuditioningPreset17:
		{
			uint8_t port = ((uint8_t)param->GetParamID() - (uint8_t)SettingsGeneralCnfgParam::paramAuditioningPreset1);
			_SettingsGeneralCnfgData.auditioningPortPreset[port] = (param->getValue().get<int>());
			engine.songManager->setAuditioningPreset(port, _SettingsGeneralCnfgData.auditioningPortPreset[port]);
		}
			break;

		case SettingsGeneralCnfgParam::SettingDeviceCnfgSafeStopParam:
			_SettingsGeneralCnfgData.DeviceCnfgSafeStopParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsGeneralCnfgParam::SettingDeviceCnfgBrightnessParam:
			_SettingsGeneralCnfgData.DeviceCnfgBrightnessParam = (param->getValue ().get<int8_t> ());
			break;
		case SettingsGeneralCnfgParam::SettingDeviceCnfgAutoSaveAll:
			_SettingsGeneralCnfgData.DeviceCnfgAutoSaveAll = (param->getValue ().get<bool> ());
			engine.setAutoSaveAllModules(_SettingsGeneralCnfgData.DeviceCnfgAutoSaveAll);
			break;
		case SettingsGeneralCnfgParam::SettingInstrumentAutoSaveParam:
			_SettingsGeneralCnfgData.InstrumentAutoSave = (param->getValue ().get<bool> ());
			break;
		default:
			assert (0);
			break;
	}
	//todo - need to save ?
	{
		if (engine.fileHandler->UpdateSettingsGeneralCnfgData(_SettingsGeneralCnfgData) == FILE_OK)
		{
			ClearUndoList();
			isUndilistNeedUpdate = true;
		}
	}
	if (SendNotification)
	{
		/* Send notification to the subscribed modules via engine */
		paramlistener->onParameterChanged (moduleSrc, param);
	}
}
// Explicit desktop save does not change the selected storage location.
bool SettingsGeneralCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsGeneralCnfgData(_SettingsGeneralCnfgData);
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
