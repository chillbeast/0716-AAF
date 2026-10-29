#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsSongCnfg.h"

using namespace Ignite;

SettingsSongCnfg::SettingsSongCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsSongCnfg", (moduleId_t)ModuleName::moduleSettingSongCnfg, (unsigned int)SettingsSongCnfgParam::paramSettingsSongCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsSongCnfgParam::SettingSongAutoSaveParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSongCnfgParam::SettingSongNoteNameDispParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSongCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSongCnfgParam::SettingSongCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSongCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsSongCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsSongCnfgParam::SettingSongTapTempoParam, 3, 10, 3);
	AddParameter<int8_t> ((paramId_t)SettingsSongCnfgParam::settingInsertPosition, static_cast <int8_t> (InsertPosition::listEnd), static_cast <int8_t> (InsertPosition::nextToActiveRow), static_cast <int8_t> (InsertPosition::listEnd));
	AddParameter<int8_t>((paramId_t)SettingsSongCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->GetSettingsSongCnfgDataSetting (&_SettingsSongCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsSongCnfgData (_SettingsSongCnfgData, ""); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingSongCnfg, true);
	SetDefaultUndoListData ();
}
void SettingsSongCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsSongCnfgParam::paramall + 1; i < (paramId_t)SettingsSongCnfgParam::paramSettingsSongCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsSongCnfgParam::SettingSongCnfgSave) ||
			(i == (paramId_t)SettingsSongCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsSongCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsSongCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsSongCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingSongCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsSongCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsSongCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsSongCnfgParam)(*it).param)
		{
			case SettingsSongCnfgParam::SettingSongAutoSaveParam:
				paramData.param = (paramId_t)SettingsSongCnfgParam::SettingSongAutoSaveParam;
				paramData.MsgValue = _SettingsSongCnfgData.SongAutoSaveParam;
				retParam.push_back (paramData);
				break;
			case SettingsSongCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsSongCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsSongCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsSongCnfgParam::SettingSongNoteNameDispParam:
				paramData.param = (paramId_t)SettingsSongCnfgParam::SettingSongNoteNameDispParam;
				paramData.MsgValue = _SettingsSongCnfgData.SongNoteNameDispParam;
				retParam.push_back (paramData);
				break;

			case SettingsSongCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();
				paramData.param = (paramId_t)SettingsSongCnfgParam::paramUndoListCmd;
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

			case SettingsSongCnfgParam::SettingSongTapTempoParam:
				paramData.param = (paramId_t)SettingsSongCnfgParam::SettingSongTapTempoParam;
				paramData.MsgValue = _SettingsSongCnfgData.SongTapTempoParam;
				retParam.push_back (paramData);
				break;

			case SettingsSongCnfgParam::settingInsertPosition:
				paramData.param = (paramId_t)SettingsSongCnfgParam::settingInsertPosition;
				paramData.MsgValue = _SettingsSongCnfgData.SongInsertPosition;
				retParam.push_back(paramData);
				break;
		}
	}

	return  retParam;
}
void SettingsSongCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsSongCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsSongCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsSongCnfgParam)param->GetParamID () != SettingsSongCnfgParam::SettingSongCnfgSave &&
		(SettingsSongCnfgParam)param->GetParamID () != SettingsSongCnfgParam::paramSaveAll &&
		(SettingsSongCnfgParam)param->GetParamID () != SettingsSongCnfgParam::SettingDataLocation &&
		(SettingsSongCnfgParam)param->GetParamID () != SettingsSongCnfgParam::paramUndoListCmd &&
		(SettingsSongCnfgParam)param->GetParamID () != SettingsSongCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsSongCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsSongCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
	switch ((SettingsSongCnfgParam)param->GetParamID ())
	{
		case SettingsSongCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsSongCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsSongCnfgParam::SettingSongCnfgSave:
			_SettingsSongCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
			break;
		case SettingsSongCnfgParam::SettingSongAutoSaveParam:
			_SettingsSongCnfgData.SongAutoSaveParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSongCnfgParam::SettingDataLocation:
			_SettingsSongCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsSongCnfgDataSetting (&_SettingsSongCnfgData, (_SettingsSongCnfgData.DataLocation ? getActiveSongName () : ""));
			break;

		case SettingsSongCnfgParam::SettingSongNoteNameDispParam:
			_SettingsSongCnfgData.SongNoteNameDispParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSongCnfgParam::SettingSongTapTempoParam:
			_SettingsSongCnfgData.SongTapTempoParam = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSongCnfgParam::settingInsertPosition:
		{
			auto source = (param->getValue().get<int8_t>());
			if ((source < static_cast <int8_t> (InsertPosition::listEnd)) || (source > static_cast <int8_t> (InsertPosition::nextToActiveRow)))
			{
				source = static_cast <int8_t> (InsertPosition::listEnd);
			}
			_SettingsSongCnfgData.SongInsertPosition = source;
			break;
		}
			
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
int8_t SettingsSongCnfg::getSongTapTempo ()
{
		return(_SettingsSongCnfgData.SongTapTempoParam);
}
void SettingsSongCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsSongCnfgData(_SettingsSongCnfgData, (_SettingsSongCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}
// Explicit desktop save does not change the selected storage location.
bool SettingsSongCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsSongCnfgData(_SettingsSongCnfgData, (_SettingsSongCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
