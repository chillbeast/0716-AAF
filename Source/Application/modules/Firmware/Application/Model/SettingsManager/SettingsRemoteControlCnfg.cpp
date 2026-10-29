#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsRemoteControlCnfg.h"

using namespace Ignite;

SettingsRemoteControlCnfg::SettingsRemoteControlCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsRemoteControlCnfg", (moduleId_t)ModuleName::moduleSettingsRemoteControlCnfg, (unsigned int)SettingsRemoteControlCnfgParam::paramSettingsRemoteControlCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlStatus, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlLatchParam, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlMidiPort, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlMidiChannel, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlSceneChangeParam, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlFontChangeParam, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::SettingRemoteControlCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRemoteControlCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsRemoteControlCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsRemoteControlCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->GetSettingsRemoteControlCnfgDataSetting (&_SettingsRemoteControlCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsRemoteControlCnfgData (_SettingsRemoteControlCnfgData, ""); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsRemoteControlCnfg, true);
	SetDefaultUndoListData ();
}
void SettingsRemoteControlCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsRemoteControlCnfgParam::paramall + 1; i < (paramId_t)SettingsRemoteControlCnfgParam::paramSettingsRemoteControlCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsRemoteControlCnfgParam::SettingRemoteControlCnfgSave) ||
			(i == (paramId_t)SettingsRemoteControlCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsRemoteControlCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsRemoteControlCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsRemoteControlCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsRemoteControlCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsRemoteControlCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsRemoteControlCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsRemoteControlCnfgParam)(*it).param)
		{
			case SettingsRemoteControlCnfgParam::SettingsRemoteControlStatus:
				paramData.param = (paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlStatus;
				paramData.MsgValue = _SettingsRemoteControlCnfgData.RemoteControlStatus;
				retParam.push_back (paramData);
				break;

			case SettingsRemoteControlCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsRemoteControlCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsRemoteControlCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;
			case SettingsRemoteControlCnfgParam::SettingsRemoteControlLatchParam:
				paramData.param = (paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlLatchParam;
				paramData.MsgValue = _SettingsRemoteControlCnfgData.RemoteControlLatch;
				retParam.push_back (paramData);
				break;
			case SettingsRemoteControlCnfgParam::SettingsRemoteControlMidiPort:
				paramData.param = (paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlMidiPort;
				paramData.MsgValue = _SettingsRemoteControlCnfgData.RemoteControlMidiPort;
				retParam.push_back (paramData);
				break;
			case SettingsRemoteControlCnfgParam::SettingsRemoteControlMidiChannel:
				paramData.param = (paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlMidiChannel;
				paramData.MsgValue = _SettingsRemoteControlCnfgData.RemoteControlMidiChannel;
				retParam.push_back (paramData);
				break;
			case SettingsRemoteControlCnfgParam::SettingsRemoteControlSceneChangeParam:
				paramData.param = (paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlSceneChangeParam;
				paramData.MsgValue = _SettingsRemoteControlCnfgData.RemoteControlSceneChange;
				retParam.push_back (paramData);
				break;
			case SettingsRemoteControlCnfgParam::SettingsRemoteControlFontChangeParam:
				paramData.param = (paramId_t)SettingsRemoteControlCnfgParam::SettingsRemoteControlFontChangeParam;
				paramData.MsgValue = _SettingsRemoteControlCnfgData.RemoteControlFontChange;
				retParam.push_back (paramData);
				break;
			case SettingsRemoteControlCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();
				paramData.param = (paramId_t)SettingsRemoteControlCnfgParam::paramUndoListCmd;
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
void SettingsRemoteControlCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsRemoteControlCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsRemoteControlCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsRemoteControlCnfgParam)param->GetParamID () != SettingsRemoteControlCnfgParam::SettingRemoteControlCnfgSave &&
		(SettingsRemoteControlCnfgParam)param->GetParamID () != SettingsRemoteControlCnfgParam::paramSaveAll &&
		(SettingsRemoteControlCnfgParam)param->GetParamID () != SettingsRemoteControlCnfgParam::SettingDataLocation &&
		(SettingsRemoteControlCnfgParam)param->GetParamID () != SettingsRemoteControlCnfgParam::paramUndoListCmd &&
		(SettingsRemoteControlCnfgParam)param->GetParamID () != SettingsRemoteControlCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsRemoteControlCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsRemoteControlCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
	switch ((SettingsRemoteControlCnfgParam)param->GetParamID ())
	{
		case SettingsRemoteControlCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsRemoteControlCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsRemoteControlCnfgParam::SettingRemoteControlCnfgSave:
			_SettingsRemoteControlCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
			break;
		case SettingsRemoteControlCnfgParam::SettingDataLocation:
			_SettingsRemoteControlCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsRemoteControlCnfgDataSetting (&_SettingsRemoteControlCnfgData, (_SettingsRemoteControlCnfgData.DataLocation ? getActiveSongName () : ""));
			break;
		case SettingsRemoteControlCnfgParam::SettingsRemoteControlStatus:
			_SettingsRemoteControlCnfgData.RemoteControlStatus = (param->getValue ().get<int8_t> ());
			break;
		case SettingsRemoteControlCnfgParam::SettingsRemoteControlLatchParam:
			_SettingsRemoteControlCnfgData.RemoteControlLatch = (param->getValue ().get<int8_t> ());
			break;
		case SettingsRemoteControlCnfgParam::SettingsRemoteControlMidiPort:
			_SettingsRemoteControlCnfgData.RemoteControlMidiPort = (param->getValue ().get<int8_t> ());
			break;
		case SettingsRemoteControlCnfgParam::SettingsRemoteControlMidiChannel:
			_SettingsRemoteControlCnfgData.RemoteControlMidiChannel = (param->getValue ().get<int8_t> ());
			break;
		case SettingsRemoteControlCnfgParam::SettingsRemoteControlSceneChangeParam:
			_SettingsRemoteControlCnfgData.RemoteControlSceneChange = (param->getValue ().get<int8_t> ());
			break;
		case SettingsRemoteControlCnfgParam::SettingsRemoteControlFontChangeParam:
			_SettingsRemoteControlCnfgData.RemoteControlFontChange = (param->getValue ().get<int8_t> ());
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
void SettingsRemoteControlCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsRemoteControlCnfgData(_SettingsRemoteControlCnfgData, (_SettingsRemoteControlCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;

}
// Explicit desktop save does not change the selected storage location.
bool SettingsRemoteControlCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsRemoteControlCnfgData(_SettingsRemoteControlCnfgData, (_SettingsRemoteControlCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
