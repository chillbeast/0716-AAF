#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsMidiMachineControlCnfg.h"

using namespace Ignite;

SettingsMidiMachineControlCnfg::SettingsMidiMachineControlCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsMidiMessageControlCnfg", (moduleId_t)ModuleName::moduleSettingsMidiMachineControlCnfg, (unsigned int)SettingsMidiMachineControlCnfgParam::paramSettingsMidiMachineControlCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlReceiveStatus, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlSendStatus, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlInputPort, 0, 4, 0); // TODO: Consider USB MIDI ports
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlOutputPort, 0, 4, 0); // TODO: Consider USB MIDI ports
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlDeviceId, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlDeviceIdAll, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::SettingMidiMachineControlCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsMidiMachineControlCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsMidiMachineControlCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->GetSettingsMidiMachineControlCnfgDataSetting (&_SettingsMidiMachineControlCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsMidiMachineControlCnfgData (_SettingsMidiMachineControlCnfgData, ""); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsMidiMachineControlCnfg, true);
	SetDefaultUndoListData ();
}
void SettingsMidiMachineControlCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsMidiMachineControlCnfgParam::paramall + 1; i < (paramId_t)SettingsMidiMachineControlCnfgParam::paramSettingsMidiMachineControlCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsMidiMachineControlCnfgParam::SettingMidiMachineControlCnfgSave) ||
			(i == (paramId_t)SettingsMidiMachineControlCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsMidiMachineControlCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsMidiMachineControlCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsMidiMachineControlCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsMidiMachineControlCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsMidiMachineControlCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter ((*it).param)->setValue (moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsMidiMachineControlCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsMidiMachineControlCnfgParam)(*it).param)
		{
			case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlReceiveStatus:
				paramData.param = (paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlReceiveStatus;
				paramData.MsgValue = _SettingsMidiMachineControlCnfgData.mmcReceiveStatus;
				retParam.push_back (paramData);
				break;

			case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlSendStatus:
				paramData.param = (paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlSendStatus;
				paramData.MsgValue = _SettingsMidiMachineControlCnfgData.mmcSendStatus;
				retParam.push_back (paramData);
				break;

			case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlInputPort:
				paramData.param = (paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlInputPort;
				paramData.MsgValue = _SettingsMidiMachineControlCnfgData.mmcInputPort;
				retParam.push_back (paramData);
				break;

			case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlOutputPort:
				paramData.param = (paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlOutputPort;
				paramData.MsgValue = _SettingsMidiMachineControlCnfgData.mmcOutputPort;
				retParam.push_back (paramData);
				break;

			case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlDeviceId:
				paramData.param = (paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlDeviceId;
				paramData.MsgValue = _SettingsMidiMachineControlCnfgData.mmcDeviceId;
				retParam.push_back (paramData);
				break;

			case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlDeviceIdAll:
				paramData.param = (paramId_t)SettingsMidiMachineControlCnfgParam::paramMidiMachineControlDeviceIdAll;
				paramData.MsgValue = _SettingsMidiMachineControlCnfgData.mmcDeviceIdAll;
				retParam.push_back (paramData);
				break;

			case SettingsMidiMachineControlCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsMidiMachineControlCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsMidiMachineControlCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsMidiMachineControlCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();
				paramData.param = (paramId_t)SettingsMidiMachineControlCnfgParam::paramUndoListCmd;
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
void SettingsMidiMachineControlCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsMidiMachineControlCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsMidiMachineControlCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsMidiMachineControlCnfgParam)param->GetParamID () != SettingsMidiMachineControlCnfgParam::SettingMidiMachineControlCnfgSave &&
		(SettingsMidiMachineControlCnfgParam)param->GetParamID () != SettingsMidiMachineControlCnfgParam::paramSaveAll &&
		(SettingsMidiMachineControlCnfgParam)param->GetParamID () != SettingsMidiMachineControlCnfgParam::SettingDataLocation &&
		(SettingsMidiMachineControlCnfgParam)param->GetParamID () != SettingsMidiMachineControlCnfgParam::paramUndoListCmd &&
		(SettingsMidiMachineControlCnfgParam)param->GetParamID () != SettingsMidiMachineControlCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsMidiMachineControlCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsMidiMachineControlCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
	switch ((SettingsMidiMachineControlCnfgParam)param->GetParamID ())
	{
		case SettingsMidiMachineControlCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsMidiMachineControlCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsMidiMachineControlCnfgParam::SettingMidiMachineControlCnfgSave:
			_SettingsMidiMachineControlCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
			break;
		case SettingsMidiMachineControlCnfgParam::SettingDataLocation:
			_SettingsMidiMachineControlCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsMidiMachineControlCnfgDataSetting (&_SettingsMidiMachineControlCnfgData, (_SettingsMidiMachineControlCnfgData.DataLocation ? getActiveSongName () : ""));
			break;
		case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlReceiveStatus:
			_SettingsMidiMachineControlCnfgData.mmcReceiveStatus = (param->getValue ().get<int8_t> ());
			break;
		case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlSendStatus:
			_SettingsMidiMachineControlCnfgData.mmcSendStatus = (param->getValue ().get<int8_t> ());
			break;
		case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlInputPort:
			_SettingsMidiMachineControlCnfgData.mmcInputPort = (param->getValue ().get<int8_t> ());
			break;
		case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlOutputPort:
			_SettingsMidiMachineControlCnfgData.mmcOutputPort = (param->getValue ().get<int8_t> ());
			break;
		case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlDeviceId:
			_SettingsMidiMachineControlCnfgData.mmcDeviceId = (param->getValue ().get<int8_t> ());
			break;
		case SettingsMidiMachineControlCnfgParam::paramMidiMachineControlDeviceIdAll:
			_SettingsMidiMachineControlCnfgData.mmcDeviceIdAll = (param->getValue ().get<int8_t> ());
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
void SettingsMidiMachineControlCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsMidiMachineControlCnfgData(_SettingsMidiMachineControlCnfgData, (_SettingsMidiMachineControlCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}

// Explicit desktop save does not change the selected storage location.
bool SettingsMidiMachineControlCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsMidiMachineControlCnfgData(_SettingsMidiMachineControlCnfgData, (_SettingsMidiMachineControlCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
