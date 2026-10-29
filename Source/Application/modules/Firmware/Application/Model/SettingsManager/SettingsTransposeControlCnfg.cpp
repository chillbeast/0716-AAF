#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsTransposeControlCnfg.h"

using namespace Ignite;

SettingsTransposeControlCnfg::SettingsTransposeControlCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsTransposeControlCnfg", (moduleId_t)ModuleName::moduleSettingsTransposeControlCnfg, (unsigned int)SettingsTransposeControlCnfgParam::paramTransposeControlCnfgParamMax, listener)
{
	AddParameter<bool> ((paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlStatus, false, true, false);
	AddParameter<bool> ((paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlLatch, false, true, false);
	AddParameter<int8_t> ((paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlMidiPort, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlMidiChannel, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlCenterPitch, 0, 127, 64);
	AddParameter<int8_t> ((paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsTransposeControlCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsTransposeControlCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsTransposeControlCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->getSettingsTransposeControlCnfgData (&_transposeControlCnfgData, ""))
	{
		engine.fileHandler->updateSettingsTransposeControlCnfgData (&_transposeControlCnfgData, ""); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsTransposeControlCnfg, true);
	SetDefaultUndoListData ();
}

void SettingsTransposeControlCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsTransposeControlCnfgParam::paramall + 1; i < (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlCnfgSave) ||
			(i == (paramId_t)SettingsTransposeControlCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlDataLocation) ||
			(i == (paramId_t)SettingsTransposeControlCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsTransposeControlCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsTransposeControlCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}

void SettingsTransposeControlCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}

std::vector<IgniteParamList> SettingsTransposeControlCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsTransposeControlCnfgParam)(*it).param)
		{
			case SettingsTransposeControlCnfgParam::paramTransposeControlStatus:
				paramData.param = (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlStatus;
				paramData.MsgValue = _transposeControlCnfgData.transposeControlStatus;
				retParam.push_back (paramData);
				break;

			case SettingsTransposeControlCnfgParam::paramTransposeControlDataLocation:
				paramData.param = (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlDataLocation;
				paramData.MsgValue = _transposeControlCnfgData.dataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsTransposeControlCnfgParam::paramTransposeControlLatch:
				paramData.param = (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlLatch;
				paramData.MsgValue = _transposeControlCnfgData.transposeControlLatch;
				retParam.push_back (paramData);
				break;

			case SettingsTransposeControlCnfgParam::paramTransposeControlMidiPort:
				paramData.param = (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlMidiPort;
				paramData.MsgValue = _transposeControlCnfgData.transposeControlMidiPort;
				retParam.push_back (paramData);
				break;

			case SettingsTransposeControlCnfgParam::paramTransposeControlMidiChannel:
				paramData.param = (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlMidiChannel;
				paramData.MsgValue = _transposeControlCnfgData.transposeControlMidiChannel;
				retParam.push_back (paramData);
				break;

			case SettingsTransposeControlCnfgParam::paramTransposeControlCenterPitch:
				paramData.param = (paramId_t)SettingsTransposeControlCnfgParam::paramTransposeControlCenterPitch;
				paramData.MsgValue = _transposeControlCnfgData.transposeControlCenterPitch;
				retParam.push_back (paramData);
				break;

			case SettingsTransposeControlCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();
				paramData.param = (paramId_t)SettingsTransposeControlCnfgParam::paramUndoListCmd;
				paramData.Count = (uint32_t)undoList.size (); //set the number of params
				retParam.push_back (paramData);
				for (auto iter = undoList.begin (); iter < undoList.end (); ++iter)
				{
					paramData.param = *iter;
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

void SettingsTransposeControlCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}

void SettingsTransposeControlCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}

void SettingsTransposeControlCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsTransposeControlCnfgParam)param->GetParamID () != SettingsTransposeControlCnfgParam::paramTransposeControlCnfgSave &&
		(SettingsTransposeControlCnfgParam)param->GetParamID () != SettingsTransposeControlCnfgParam::paramSaveAll &&
		(SettingsTransposeControlCnfgParam)param->GetParamID () != SettingsTransposeControlCnfgParam::paramTransposeControlDataLocation &&
		(SettingsTransposeControlCnfgParam)param->GetParamID () != SettingsTransposeControlCnfgParam::paramUndoListCmd &&
		(SettingsTransposeControlCnfgParam)param->GetParamID () != SettingsTransposeControlCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}

std::string SettingsTransposeControlCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
	{
		return (engine.songManager->activeSong->getName ());
	}
	return("");
}

void SettingsTransposeControlCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}

	switch ((SettingsTransposeControlCnfgParam)param->GetParamID ())
	{
		case SettingsTransposeControlCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;

		case SettingsTransposeControlCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsTransposeControlCnfgParam::paramTransposeControlCnfgSave:
			_transposeControlCnfgData.dataLocation = (param->getValue ().get<int8_t> ());

			break;

		case SettingsTransposeControlCnfgParam::paramTransposeControlDataLocation:
			_transposeControlCnfgData.dataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->getSettingsTransposeControlCnfgData (&_transposeControlCnfgData, (_transposeControlCnfgData.dataLocation ? getActiveSongName () : ""));
			break;

		case SettingsTransposeControlCnfgParam::paramTransposeControlStatus:
			_transposeControlCnfgData.transposeControlStatus = (param->getValue ().get<int8_t> ());
			break;

		case SettingsTransposeControlCnfgParam::paramTransposeControlLatch:
			_transposeControlCnfgData.transposeControlLatch = (param->getValue ().get<int8_t> ());
			break;

		case SettingsTransposeControlCnfgParam::paramTransposeControlMidiPort:
			_transposeControlCnfgData.transposeControlMidiPort = (param->getValue ().get<int8_t> ());
			break;

		case SettingsTransposeControlCnfgParam::paramTransposeControlMidiChannel:
			_transposeControlCnfgData.transposeControlMidiChannel = (param->getValue ().get<int8_t> ());
			break;

		case SettingsTransposeControlCnfgParam::paramTransposeControlCenterPitch:
			_transposeControlCnfgData.transposeControlCenterPitch = (param->getValue ().get<int8_t> ());
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
void SettingsTransposeControlCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->updateSettingsTransposeControlCnfgData(&_transposeControlCnfgData, (_transposeControlCnfgData.dataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}
// Explicit desktop save does not change the selected storage location.
bool SettingsTransposeControlCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->updateSettingsTransposeControlCnfgData(&_transposeControlCnfgData, (_transposeControlCnfgData.dataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
