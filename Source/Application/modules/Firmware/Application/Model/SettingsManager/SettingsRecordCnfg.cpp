#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsRecordCnfg.h"

using namespace Ignite;

SettingsRecordCnfg::SettingsRecordCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsRecordCnfg", (moduleId_t)ModuleName::moduleSettingsRecordCnfg, (unsigned int)SettingsRecordCnfgParam::paramSettingsRecordCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingsRecordCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordQuantizationParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordMthruParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordFTSParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordAutoAddBarParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordBarOverWriteParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordNumOfBarsParam, 0, 16, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordNoteEventParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordMidiCCEventParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordAfterTouchEventParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordPitchBendEventParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordPolyPEventParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingRecordPgmChangeEventParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsRecordCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsRecordCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsRecordCnfgParam::paramSaveAll, 0, 1, 0);


	if (!engine.fileHandler->GetSettingsRecordCnfgDataSetting (&_SettingsRecordCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsRecordCnfgData (_SettingsRecordCnfgData, ""); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsRecordCnfg, true);
	SetDefaultUndoListData ();
}
void SettingsRecordCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsRecordCnfgParam::paramall + 1; i < (paramId_t)SettingsRecordCnfgParam::paramSettingsRecordCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsRecordCnfgParam::SettingsRecordCnfgSave) ||
			(i == (paramId_t)SettingsRecordCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsRecordCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsRecordCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsRecordCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsRecordCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsRecordCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsRecordCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsRecordCnfgParam)(*it).param)
		{
			case SettingsRecordCnfgParam::SettingRecordQuantizationParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordQuantizationParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordQuantizationParam;
				retParam.push_back (paramData);
				break;
			case SettingsRecordCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsRecordCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordMthruParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordMthruParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordMthryParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordFTSParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordFTSParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordFTSParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordAutoAddBarParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordAutoAddBarParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordAutoAddBarParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordBarOverWriteParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordBarOverWriteParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordBarOverWriteParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordNumOfBarsParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordNumOfBarsParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordNumOfBarsParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordNoteEventParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordNoteEventParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordNoteEventParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordMidiCCEventParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordMidiCCEventParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordMidiCCEventParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordAfterTouchEventParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordAfterTouchEventParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordAfterTouchEventParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordPitchBendEventParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordPitchBendEventParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordPitchBendEventParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordPolyPEventParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordPolyPEventParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordPolyPEventParam;
				retParam.push_back (paramData);
				break;

			case SettingsRecordCnfgParam::SettingRecordPgmChangeEventParam:
				paramData.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordPgmChangeEventParam;
				paramData.MsgValue = _SettingsRecordCnfgData.RecordPgmChangeEventParam;
				retParam.push_back (paramData);
				break;
			case SettingsRecordCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();
				paramData.param = (paramId_t)SettingsRecordCnfgParam::paramUndoListCmd;
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
void SettingsRecordCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsRecordCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsRecordCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsRecordCnfgParam)param->GetParamID () != SettingsRecordCnfgParam::SettingsRecordCnfgSave &&
		(SettingsRecordCnfgParam)param->GetParamID () != SettingsRecordCnfgParam::paramSaveAll &&
		(SettingsRecordCnfgParam)param->GetParamID () != SettingsRecordCnfgParam::SettingDataLocation &&
		(SettingsRecordCnfgParam)param->GetParamID () != SettingsRecordCnfgParam::paramUndoListCmd &&
		(SettingsRecordCnfgParam)param->GetParamID () != SettingsRecordCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsRecordCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsRecordCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
	switch ((SettingsRecordCnfgParam)param->GetParamID ())
	{
		case SettingsRecordCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsRecordCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsRecordCnfgParam::SettingsRecordCnfgSave:
			_SettingsRecordCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
			break;
		case SettingsRecordCnfgParam::SettingDataLocation:
			_SettingsRecordCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsRecordCnfgDataSetting (&_SettingsRecordCnfgData, (_SettingsRecordCnfgData.DataLocation ? getActiveSongName () : ""));
			break;

		case SettingsRecordCnfgParam::SettingRecordQuantizationParam:
			_SettingsRecordCnfgData.RecordQuantizationParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordMthruParam:
			_SettingsRecordCnfgData.RecordMthryParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordFTSParam:
			_SettingsRecordCnfgData.RecordFTSParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordAutoAddBarParam:
			_SettingsRecordCnfgData.RecordAutoAddBarParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordBarOverWriteParam:
			_SettingsRecordCnfgData.RecordBarOverWriteParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordNumOfBarsParam:
			_SettingsRecordCnfgData.RecordNumOfBarsParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordNoteEventParam:
			_SettingsRecordCnfgData.RecordNoteEventParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordMidiCCEventParam:
			_SettingsRecordCnfgData.RecordMidiCCEventParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordAfterTouchEventParam:
			_SettingsRecordCnfgData.RecordAfterTouchEventParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordPitchBendEventParam:
			_SettingsRecordCnfgData.RecordPitchBendEventParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordPolyPEventParam:
			_SettingsRecordCnfgData.RecordPolyPEventParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsRecordCnfgParam::SettingRecordPgmChangeEventParam:
			_SettingsRecordCnfgData.RecordPgmChangeEventParam = (param->getValue ().get<int8_t> ());
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
void SettingsRecordCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsRecordCnfgData(_SettingsRecordCnfgData, (_SettingsRecordCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;

}
// Explicit desktop save does not change the selected storage location.
bool SettingsRecordCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsRecordCnfgData(_SettingsRecordCnfgData, (_SettingsRecordCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
