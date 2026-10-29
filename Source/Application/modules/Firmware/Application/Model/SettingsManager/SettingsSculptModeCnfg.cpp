#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsSculptModeCnfg.h"

using namespace Ignite;

SettingsSculptModeCnfg::SettingsSculptModeCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsSculptModeCnfg", (moduleId_t)ModuleName::moduleSettingsSculptModeCnfg, (unsigned int)SettingsSculptModeCnfgParam::paramSettingsSculptModeCnfgParamMax, listener)
{
	AddParameter<bool> ((paramId_t)SettingsSculptModeCnfgParam::sculptButtonOnState, false, true, false);
	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptMode, (int8_t)SculptMode::none, (int8_t)SculptMode::randomAbsolute, (int8_t)SculptMode::absolute);
	AddParameter<uint8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptParameterSelection, 0, 0xF, 1);

	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRelativeVelocity, -127, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRelativeLengthSteps, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRelativeLengthTicks, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsSculptModeCnfgParam::sculptRelativeNote, -127, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRelativeDelay, 0, 127, 0);

	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteVelocity, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteLengthSteps, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteLengthTicks, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteNote, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteDelay, 0, 127, 0);

	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeVelocityStart, -127, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeVelocityEnd, -127, 127, 127);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeLengthStepStart, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeLengthStepEnd, 0, 127, 127);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeLengthTickStart, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeLengthTickEnd, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeNoteStart, -127, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeNoteEnd, -127, 127, 127);
	//AddParameter<int8_t>((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeDelayStart, -16, 15, 0);
	//AddParameter<int8_t>((paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeDelayEnd, -16, 15, 0);

	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteVelocityStart, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteVelocityEnd, 0, 127, 127);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthStepStart, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthStepEnd, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthTickStart, 0, 127, 0);
	//AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthTickEnd, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteNoteStart, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteNoteEnd, 0, 127, 127);
	//AddParameter<int8_t>((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteDelayStart, -16, 15, 0);
	//AddParameter<int8_t>((paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteDelayEnd, -16, 15, 0);

	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::settingSculptConfigSave, 0, 1, 0);


	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::SettingDataLocation, 0, 1, 0);	AddParameter<int8_t> ((paramId_t)SettingsSculptModeCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsSculptModeCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsSculptModeCnfgParam::paramSaveAll, 0, 1, 0);

	if (!engine.fileHandler->GetSettingsSculptModeCnfgDataSetting (&_SettingsSculptModeCnfgData, ""))
	{
		engine.fileHandler->UpdateSettingsSculptModeCnfgData (_SettingsSculptModeCnfgData, ""); // create with default data
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsSculptModeCnfg, true);
	_UndoSettingsSculptModeCnfgDataList.insert (_UndoSettingsSculptModeCnfgDataList.begin (), _SettingsSculptModeCnfgData);
}

void SettingsSculptModeCnfg::SetDefaultUndoListData ()
{
 // Not used
}

void SettingsSculptModeCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}

std::vector<IgniteParamList> SettingsSculptModeCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsSculptModeCnfgParam)(*it).param)
		{
			case SettingsSculptModeCnfgParam::sculptMode:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptMode;
				paramData.MsgValue = static_cast<int8_t> (_SettingsSculptModeCnfgData.sculptMode);
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRelativeVelocity:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRelativeVelocity;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRelativeVelocity;
				retParam.push_back (paramData);
				break;
			case SettingsSculptModeCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsSculptModeCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			/*case SettingsSculptModeCnfgParam::sculptRelativeLengthSteps:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRelativeLengthSteps;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRelativeLengthSteps;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRelativeLengthTicks:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRelativeLengthTicks;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRelativeLengthTicks;
				retParam.push_back (paramData);
				break;*/

			case SettingsSculptModeCnfgParam::sculptRelativeNote:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRelativeNote;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRelativeNote;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptAbsoluteVelocity:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteVelocity;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptAbsoluteVelocity;
				retParam.push_back (paramData);
				break;

			/*case SettingsSculptModeCnfgParam::sculptAbsoluteLengthSteps:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteLengthSteps;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptAbsoluteLengthSteps;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptAbsoluteLengthTicks:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteLengthTicks;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptAbsoluteLengthTicks;
				retParam.push_back (paramData);
				break;*/

			case SettingsSculptModeCnfgParam::sculptAbsoluteNote:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptAbsoluteNote;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptAbsoluteNote;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomRelativeVelocityStart:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeVelocityStart;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomRelativeVelocityStart;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomRelativeVelocityEnd:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeVelocityEnd;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomRelativeVelocityEnd;
				retParam.push_back (paramData);
				break;

			/*case SettingsSculptModeCnfgParam::sculptRandomRelativeLengthStepStart:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeLengthStepStart;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomRelativeLengthStepStart;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomRelativeLengthStepEnd:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeLengthStepEnd;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomRelativeLengthStepEnd;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomRelativeLengthTickStart:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeLengthTickStart;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomRelativeLengthTickStart;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomRelativeLengthTickEnd:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeLengthTickEnd;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomRelativeLengthTickEnd;
				retParam.push_back (paramData);
				break;*/

			case SettingsSculptModeCnfgParam::sculptRandomRelativeNoteStart:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeNoteStart;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomRelativeNoteStart;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomRelativeNoteEnd:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomRelativeNoteEnd;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomRelativeNoteEnd;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomAbsoluteVelocityStart:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteVelocityStart;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomAbsoluteVelocityStart;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomAbsoluteVelocityEnd:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteVelocityEnd;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomAbsoluteVelocityEnd;
				retParam.push_back (paramData);
				break;

			/*case SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthStepStart:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthStepStart;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthStepStart;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthStepEnd:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthStepEnd;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthStepEnd;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthTickStart:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthTickStart;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthTickStart;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthTickEnd:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthTickEnd;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthTickEnd;
				retParam.push_back (paramData);
				break;*/

			case SettingsSculptModeCnfgParam::sculptRandomAbsoluteNoteStart:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteNoteStart;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomAbsoluteNoteStart;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptRandomAbsoluteNoteEnd:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptRandomAbsoluteNoteEnd;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptRandomAbsoluteNoteEnd;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::paramUndoListCmd:
			{
				paramData.param = (paramId_t)(paramId_t)SettingsSculptModeCnfgParam::paramUndoListCmd;
				paramData.Count = (uint32_t)_UndoSettingsSculptModeCnfgDataList.size () - 1; //set the number of params
				retParam.push_back (paramData);

				if (_UndoSettingsSculptModeCnfgDataList.size () > 1) // 1 for default
					retParam.push_back (paramData);// intentionally added to indicate undo available

				break;
			}

			case SettingsSculptModeCnfgParam::sculptButtonOnState:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptButtonOnState;
				paramData.MsgValue = _SettingsSculptModeCnfgData.sculptButtonOnState;
				retParam.push_back (paramData);
				break;

			case SettingsSculptModeCnfgParam::sculptParameterSelection:
				paramData.param = (paramId_t)SettingsSculptModeCnfgParam::sculptParameterSelection;
				paramData.MsgValue = static_cast <uint8_t> (_SettingsSculptModeCnfgData.sculptParameter);
				retParam.push_back (paramData);
				break;
		}
	}

	return  retParam;
}

void SettingsSculptModeCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}

void SettingsSculptModeCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}

void SettingsSculptModeCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsSculptModeCnfgParam)param->GetParamID () != SettingsSculptModeCnfgParam::settingSculptConfigSave &&
		(SettingsSculptModeCnfgParam)param->GetParamID () != SettingsSculptModeCnfgParam::paramSaveAll &&
		(SettingsSculptModeCnfgParam)param->GetParamID () != SettingsSculptModeCnfgParam::SettingDataLocation &&
		(SettingsSculptModeCnfgParam)param->GetParamID () != SettingsSculptModeCnfgParam::paramUndoListCmd &&
		(SettingsSculptModeCnfgParam)param->GetParamID () != SettingsSculptModeCnfgParam::paramUndoCmd
		)
	{
		engine.onParameterEdited(param);
        if (_UndoSettingsSculptModeCnfgDataList.size () > MAX_SETTINGS_UNDO_COUNT) // NOTE: UndoDataList last param is Default Data
		{
			auto it = _UndoSettingsSculptModeCnfgDataList.end ();
			it--;
			it--;// skip default
			_UndoSettingsSculptModeCnfgDataList.erase (it);
		}
		_UndoSettingsSculptModeCnfgDataList.insert (_UndoSettingsSculptModeCnfgDataList.begin (), _SettingsSculptModeCnfgData);
	}
	ParamHandler (moduleSrc, param, true);
}

std::string SettingsSculptModeCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsSculptModeCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	switch ((SettingsSculptModeCnfgParam)param->GetParamID ())
	{
		case SettingsSculptModeCnfgParam::SettingDataLocation:
			_SettingsSculptModeCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->GetSettingsSculptModeCnfgDataSetting (&_SettingsSculptModeCnfgData, (_SettingsSculptModeCnfgData.DataLocation ? getActiveSongName () : ""));
			break;
		case SettingsSculptModeCnfgParam::paramUndoCmd:
            if(_UndoSettingsSculptModeCnfgDataList.size()>1)
            {
                _SettingsSculptModeCnfgData=_UndoSettingsSculptModeCnfgDataList.front();
                _UndoSettingsSculptModeCnfgDataList.erase(_UndoSettingsSculptModeCnfgDataList.begin());
            }
            break;

		case SettingsSculptModeCnfgParam::sculptMode:
			_SettingsSculptModeCnfgData.sculptMode = static_cast <SculptMode>(param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRelativeVelocity:
			_SettingsSculptModeCnfgData.sculptRelativeVelocity = (param->getValue ().get<int8_t> ());
			break;

		/*case SettingsSculptModeCnfgParam::sculptRelativeLengthSteps:
			_SettingsSculptModeCnfgData.sculptRelativeLengthSteps = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRelativeLengthTicks:
			_SettingsSculptModeCnfgData.sculptRelativeLengthTicks = (param->getValue ().get<int8_t> ());
			break;*/

		case SettingsSculptModeCnfgParam::sculptRelativeNote:
			_SettingsSculptModeCnfgData.sculptRelativeNote = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptAbsoluteVelocity:
			_SettingsSculptModeCnfgData.sculptAbsoluteVelocity = (param->getValue ().get<int8_t> ());
			break;

		/*case SettingsSculptModeCnfgParam::sculptAbsoluteLengthSteps:
			_SettingsSculptModeCnfgData.sculptAbsoluteLengthSteps = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptAbsoluteLengthTicks:
			_SettingsSculptModeCnfgData.sculptAbsoluteLengthTicks = (param->getValue ().get<int8_t> ());
			break;*/

		case SettingsSculptModeCnfgParam::sculptAbsoluteNote:
			_SettingsSculptModeCnfgData.sculptAbsoluteNote = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomRelativeVelocityStart:
			_SettingsSculptModeCnfgData.sculptRandomRelativeVelocityStart = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomRelativeVelocityEnd:
			_SettingsSculptModeCnfgData.sculptRandomRelativeVelocityEnd = (param->getValue ().get<int8_t> ());
			break;

		/*case SettingsSculptModeCnfgParam::sculptRandomRelativeLengthStepStart:
			_SettingsSculptModeCnfgData.sculptRandomRelativeLengthStepStart = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomRelativeLengthStepEnd:
			_SettingsSculptModeCnfgData.sculptRandomRelativeLengthStepEnd = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomRelativeLengthTickStart:
			_SettingsSculptModeCnfgData.sculptRandomRelativeLengthTickStart = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomRelativeLengthTickEnd:
			_SettingsSculptModeCnfgData.sculptRandomRelativeLengthTickEnd = (param->getValue ().get<int8_t> ());
			break;*/

		case SettingsSculptModeCnfgParam::sculptRandomRelativeNoteStart:
			_SettingsSculptModeCnfgData.sculptRandomRelativeNoteStart = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomRelativeNoteEnd:
			_SettingsSculptModeCnfgData.sculptRandomRelativeNoteEnd = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomAbsoluteVelocityStart:
			_SettingsSculptModeCnfgData.sculptRandomAbsoluteVelocityStart = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomAbsoluteVelocityEnd:
			_SettingsSculptModeCnfgData.sculptRandomAbsoluteVelocityEnd = (param->getValue ().get<int8_t> ());
			break;

		/*case SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthStepStart:
			_SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthStepStart = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthStepEnd:
			_SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthStepEnd = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthTickStart:
			_SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthTickStart = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomAbsoluteLengthTickEnd:
			_SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthTickEnd = (param->getValue ().get<int8_t> ());
			break;*/

		case SettingsSculptModeCnfgParam::sculptRandomAbsoluteNoteStart:
			_SettingsSculptModeCnfgData.sculptRandomAbsoluteNoteStart = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::sculptRandomAbsoluteNoteEnd:
			_SettingsSculptModeCnfgData.sculptRandomAbsoluteNoteEnd = (param->getValue ().get<int8_t> ());
			break;

		case SettingsSculptModeCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (_UndoSettingsSculptModeCnfgDataList.empty())
				break;
			//fallthrough
#endif

		case SettingsSculptModeCnfgParam::settingSculptConfigSave:
			_SettingsSculptModeCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
			break;

		case SettingsSculptModeCnfgParam::sculptButtonOnState:
			_SettingsSculptModeCnfgData.sculptButtonOnState = param->getValue ().get<bool> ();
			break;

		case SettingsSculptModeCnfgParam::sculptParameterSelection:
			_SettingsSculptModeCnfgData.sculptParameter = static_cast<SculptParameterSelect> (param->getValue ().get<uint8_t> ());
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
void SettingsSculptModeCnfg::SaveData()
{
	if (_UndoSettingsSculptModeCnfgDataList.empty())
		return;
	if(engine.fileHandler->UpdateSettingsSculptModeCnfgData(_SettingsSculptModeCnfgData, (_SettingsSculptModeCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	_UndoSettingsSculptModeCnfgDataList.clear();
	_UndoSettingsSculptModeCnfgDataList.insert(_UndoSettingsSculptModeCnfgDataList.begin(), _SettingsSculptModeCnfgData);
}


// Explicit desktop save does not change the selected storage location.
bool SettingsSculptModeCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateSettingsSculptModeCnfgData(_SettingsSculptModeCnfgData, (_SettingsSculptModeCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    _UndoSettingsSculptModeCnfgDataList.clear(); _UndoSettingsSculptModeCnfgDataList.push_back(_SettingsSculptModeCnfgData);
    return true;
}
