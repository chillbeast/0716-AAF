#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsSceneCnfg.h"

using namespace Ignite;

extern std::unordered_map <ScaleList, ScaleParams> ftsList;

SettingsSceneCnfg::SettingsSceneCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsSceneCnfg", (moduleId_t)ModuleName::moduleSettingSceneCnfg, (unsigned int)SettingsSceneCnfgParam::paramSettingsSceneCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsSceneCnfgParam::SettingSceneAutoSaveParam, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSceneCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSceneCnfgParam::SettingSceneCnfgSave, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSceneCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsSceneCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsSceneCnfgParam::SettingSceneTapTempoParam, 3, 10, 3);
	AddParameter<int8_t>((paramId_t)SettingsSceneCnfgParam::settingInsertPosition, static_cast <int8_t> (InsertPosition::listEnd), static_cast <int8_t> (InsertPosition::nextToActiveRow), static_cast <int8_t> (InsertPosition::listEnd));
	AddParameter<int8_t>((paramId_t)SettingsSceneCnfgParam::SettingSceneSoloModeParam, 0, 1, 0);
	AddParameter<int8_t>((paramId_t)SettingsSceneCnfgParam::SettingSceneFillModeParam, 0, 1, 0);
	AddParameter<int8_t>((paramId_t)SettingsSceneCnfgParam::SettingSceneFillActiveCnfgParam, 0, 16, 0);
	AddParameter<int8_t>((paramId_t)SettingsSceneCnfgParam::SettingSceneFillKeyCnfgParam, 0, 16, 0);
	AddParameter<int8_t>((paramId_t)SettingsSceneCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->GetSettingsSceneCnfgDataSetting (&_SettingsSceneCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsSceneCnfgData (_SettingsSceneCnfgData, ""); // create with default data
	}

	SetDefaultUndoListData ();
}
void SettingsSceneCnfg::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsSceneCnfgParam::paramall + 1; i < (paramId_t)SettingsSceneCnfgParam::paramSettingsSceneCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsSceneCnfgParam::SettingSceneCnfgSave) ||
			(i == (paramId_t)SettingsSceneCnfgParam::paramSaveAll) ||
			(i == (paramId_t)SettingsSceneCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsSceneCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsSceneCnfgParam::paramUndoCmd)
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
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingSceneCnfg, true);
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingSceneCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void SettingsSceneCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsSceneCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsSceneCnfgParam)(*it).param)
		{
			case SettingsSceneCnfgParam::SettingSceneAutoSaveParam:
				paramData.param = (paramId_t)SettingsSceneCnfgParam::SettingSceneAutoSaveParam;
				paramData.MsgValue = _SettingsSceneCnfgData.SceneAutoSaveParam;
				retParam.push_back (paramData);
				break;
			case SettingsSceneCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();
				paramData.param = (paramId_t)SettingsSceneCnfgParam::paramUndoListCmd;
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
			case SettingsSceneCnfgParam::SettingSceneTapTempoParam:
				paramData.param = (paramId_t)SettingsSceneCnfgParam::SettingSceneTapTempoParam;
				paramData.MsgValue = _SettingsSceneCnfgData.SceneTapTempoParam;
				retParam.push_back (paramData);
				break;
			case SettingsSceneCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsSceneCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsSceneCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;
			case SettingsSceneCnfgParam::settingInsertPosition:
				paramData.param = (paramId_t)SettingsSceneCnfgParam::settingInsertPosition;
				paramData.MsgValue = _SettingsSceneCnfgData.SceneInsertPosition;
				retParam.push_back(paramData);
				break;
			case SettingsSceneCnfgParam::SettingSceneSoloModeParam:
				paramData.param = (paramId_t)SettingsSceneCnfgParam::SettingSceneSoloModeParam;
				paramData.MsgValue = _SettingsSceneCnfgData.SceneSoloModeParam;
				retParam.push_back(paramData);
				break;

			case SettingsSceneCnfgParam::SettingSceneFillModeParam:
				paramData.param = (paramId_t)SettingsSceneCnfgParam::SettingSceneFillModeParam;
				paramData.MsgValue = _SettingsSceneCnfgData.SceneFillModeParam;
				retParam.push_back(paramData);
				break;

			case SettingsSceneCnfgParam::SettingSceneFillActiveCnfgParam:
				paramData.param = (paramId_t)SettingsSceneCnfgParam::SettingSceneFillActiveCnfgParam;
				paramData.MsgValue = _SettingsSceneCnfgData.SceneFillActiveCnfgParam;
				retParam.push_back(paramData);
				break;

			case SettingsSceneCnfgParam::SettingSceneFillKeyCnfgParam:
				paramData.param = (paramId_t)SettingsSceneCnfgParam::SettingSceneFillKeyCnfgParam;
				paramData.MsgValue = _SettingsSceneCnfgData.SceneFillKeyCnfgParam;
				retParam.push_back(paramData);
				break;
		}
	}

	return  retParam;
}
void SettingsSceneCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsSceneCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsSceneCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsSceneCnfgParam)param->GetParamID () != SettingsSceneCnfgParam::SettingSceneCnfgSave &&
		(SettingsSceneCnfgParam)param->GetParamID () != SettingsSceneCnfgParam::paramSaveAll &&
		(SettingsSceneCnfgParam)param->GetParamID () != SettingsSceneCnfgParam::SettingDataLocation &&
		(SettingsSceneCnfgParam)param->GetParamID () != SettingsSceneCnfgParam::paramUndoListCmd &&
		(SettingsSceneCnfgParam)param->GetParamID () != SettingsSceneCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsSceneCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}

void SettingsSceneCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
	switch ((SettingsSceneCnfgParam)param->GetParamID ())
	{
		case SettingsSceneCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;
		case SettingsSceneCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsSceneCnfgParam::SettingSceneCnfgSave:
			SaveData();

			break;
		case SettingsSceneCnfgParam::SettingDataLocation:
			_SettingsSceneCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsSceneCnfgDataSetting (&_SettingsSceneCnfgData, (_SettingsSceneCnfgData.DataLocation ? getActiveSongName () : ""));
			break;
		case SettingsSceneCnfgParam::SettingSceneAutoSaveParam:
			_SettingsSceneCnfgData.SceneAutoSaveParam = (param->getValue ().get<int8_t> ());
			break;
		case SettingsSceneCnfgParam::SettingSceneTapTempoParam:
			_SettingsSceneCnfgData.SceneTapTempoParam = (param->getValue ().get<int8_t> ());
			break;
		case SettingsSceneCnfgParam::settingInsertPosition:
		{
			auto source = (param->getValue().get<int8_t>());
			if ((source < static_cast <int8_t> (InsertPosition::listEnd)) || (source > static_cast <int8_t> (InsertPosition::nextToActiveRow)))
			{
				source = static_cast <int8_t> (InsertPosition::listEnd);
			}
			_SettingsSceneCnfgData.SceneInsertPosition = source;
			break;
		}
		case SettingsSceneCnfgParam::SettingSceneSoloModeParam:
			_SettingsSceneCnfgData.SceneSoloModeParam = (param->getValue().get<int8_t>());
			break;

		case SettingsSceneCnfgParam::SettingSceneFillModeParam:
			_SettingsSceneCnfgData.SceneFillModeParam = (param->getValue().get<int8_t>());
			break;

		case SettingsSceneCnfgParam::SettingSceneFillActiveCnfgParam:
			_SettingsSceneCnfgData.SceneFillActiveCnfgParam = (param->getValue().get<int8_t>());
			break;

		case SettingsSceneCnfgParam::SettingSceneFillKeyCnfgParam:
			_SettingsSceneCnfgData.SceneFillKeyCnfgParam = (param->getValue().get<int8_t>());
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
void SettingsSceneCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->UpdateSettingsSceneCnfgData(_SettingsSceneCnfgData, (_SettingsSceneCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}
// Explicit desktop save does not change the selected storage location.
bool SettingsSceneCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsSceneCnfgData(_SettingsSceneCnfgData, (_SettingsSceneCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    return true;
}
