#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsMidiCnfg.h"

using namespace Ignite;

SettingsMidiCnfg::SettingsMidiCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsMidiCnfg", (moduleId_t)ModuleName::moduleMidiCnfg, (unsigned int)SettingsMidiCnfgParam::paramSettingsMidiCnfgParamMax, listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::paramConfigMidiActivePort, 0, MAX_MIDICONFIG_PORT, -1);//TODO finalise the total count
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestPort, 0, MAX_MIDICONFIG_PORT, -1);
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestChannel, 0, 15, -1);
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruType, 0, 5, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::paramConfigMidiClkTxMode, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruFtsOn, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::paramSaveMidiCnfgSetting, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsMidiCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsMidiCnfgParam::paramSaveAll, 0, 1, 0);

	std::list <MidiConfig> _midiCnfg = engine.fileHandler->GetMidiCnfgSetting ("");
	if (!_midiCnfg.empty ())
	{
		for (auto it = _midiCnfg.begin (); it != _midiCnfg.end (); ++it)
		{
			MidiConfig _midiCnfgData = { 0, };
			_midiCnfgData.sourcePort = it->sourcePort;
			_midiCnfgData.type = it->type;
			_midiCnfgData.thruFTSEnabled = it->thruFTSEnabled;
			_midiCnfgData.destPort = it->destPort;
			_midiCnfgData.destChannel = it->destChannel;
			_midiCnfgData.midiClkTxEnabled = it->midiClkTxEnabled;
			_settingsMidiCnfgData.MidiCnfg.insert ({ _midiCnfgData.sourcePort, _midiCnfgData });
		}
	}
	else // create with default data
	{
		for (int8_t i = 0; i < MAX_MIDICONFIG_PORT; ++i)
		{
			MidiConfig _midiCnfgData = { 1, SoftThruType::off, false, 0, 1, false };
			_midiCnfgData.sourcePort = i;
			_settingsMidiCnfgData.MidiCnfg.insert ({ i, _midiCnfgData });
		}

		engine.fileHandler->UpdateMidiCnfgSetting (_settingsMidiCnfgData, "");
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleMidiCnfg, true);
	_UndoSettingsMidiCnfgDataList.insert (_UndoSettingsMidiCnfgDataList.begin (), _settingsMidiCnfgData);
}
void SettingsMidiCnfg::GetMidiCnfg ()
{
	std::list <MidiConfig> _midiCnfg = engine.fileHandler->GetMidiCnfgSetting ((_settingsMidiCnfgData.DataLocation ? getActiveSongName () : ""));
	if (_midiCnfg.empty ())
	{
		engine.fileHandler->UpdateMidiCnfgSetting (_settingsMidiCnfgData, (_settingsMidiCnfgData.DataLocation ? getActiveSongName () : ""));
	}
	else
		_settingsMidiCnfgData.MidiCnfg.clear ();

	for (auto it = _midiCnfg.begin (); it != _midiCnfg.end (); ++it)
	{
		MidiConfig _midiCnfgData = { 0, };
		_midiCnfgData.sourcePort = it->sourcePort;
		_midiCnfgData.type = it->type;
		_midiCnfgData.thruFTSEnabled = it->thruFTSEnabled;
		_midiCnfgData.destPort = it->destPort;
		_midiCnfgData.destChannel = it->destChannel;
		_midiCnfgData.midiClkTxEnabled = it->midiClkTxEnabled;
		_settingsMidiCnfgData.MidiCnfg.insert ({ _midiCnfgData.sourcePort, _midiCnfgData });
	}
	_UndoSettingsMidiCnfgDataList.clear();
	_UndoSettingsMidiCnfgDataList.insert(_UndoSettingsMidiCnfgDataList.begin(), _settingsMidiCnfgData);
}
void SettingsMidiCnfg::SetDefaultUndoListData ()
{
	//Not used
}
void SettingsMidiCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter ((*it).param)->setValue (moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsMidiCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsMidiCnfgParam)(*it).param)
		{
			case SettingsMidiCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsMidiCnfgParam::SettingDataLocation;
				paramData.MsgValue = _settingsMidiCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsMidiCnfgParam::paramConfigMidiActivePort:
				paramData.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiActivePort;
				paramData.MsgValue = _midiConfigActivePort;
				paramData.startPos = _midiConfigActivePort;
				retParam.push_back (paramData);
				break;

			case SettingsMidiCnfgParam::paramConfigMidiClkTxMode:
				paramData.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiClkTxMode;
				paramData.MsgValue = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].midiClkTxEnabled;
				paramData.startPos = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].sourcePort;
				retParam.push_back (paramData);
				break;

			case SettingsMidiCnfgParam::paramConfigMidiThruType:
				paramData.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruType;
				paramData.MsgValue = (int8_t)_settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].type;
				paramData.startPos = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].sourcePort;
				retParam.push_back (paramData);
				break;

			case SettingsMidiCnfgParam::paramConfigMidiDestPort:
				paramData.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestPort;
				paramData.MsgValue = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].destPort;
				paramData.startPos = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].sourcePort;
				retParam.push_back (paramData);
				break;

			case SettingsMidiCnfgParam::paramConfigMidiDestChannel:
				paramData.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestChannel;
				paramData.MsgValue = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].destChannel;
				paramData.startPos = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].sourcePort;
				retParam.push_back (paramData);
				break;

			case SettingsMidiCnfgParam::paramConfigMidiThruFtsOn:
				paramData.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruFtsOn;
				paramData.MsgValue = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].thruFTSEnabled;
				paramData.startPos = _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].sourcePort;
				retParam.push_back (paramData);
				break;
			case SettingsMidiCnfgParam::paramUndoListCmd:
			{
				paramData.param = (paramId_t)SettingsMidiCnfgParam::paramUndoListCmd;
				paramData.Count = (uint32_t)_UndoSettingsMidiCnfgDataList.size () - 1; //set the number of params
				retParam.push_back (paramData);

				if (_UndoSettingsMidiCnfgDataList.size () > 1) // 1 for default
					retParam.push_back (paramData);// intentionally added to indicate undo available

				break;
			}
		}
	}

	return  retParam;
}
void SettingsMidiCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}
void SettingsMidiCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsMidiCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsMidiCnfgParam)param->GetParamID () != SettingsMidiCnfgParam::paramSaveMidiCnfgSetting &&
		(SettingsMidiCnfgParam)param->GetParamID () != SettingsMidiCnfgParam::paramSaveAll &&
		(SettingsMidiCnfgParam)param->GetParamID () != SettingsMidiCnfgParam::SettingDataLocation &&
		(SettingsMidiCnfgParam)param->GetParamID () != SettingsMidiCnfgParam::paramUndoListCmd &&
		(SettingsMidiCnfgParam)param->GetParamID () != SettingsMidiCnfgParam::paramConfigMidiActivePort &&
		(SettingsMidiCnfgParam)param->GetParamID () != SettingsMidiCnfgParam::paramUndoCmd
		)
	{
		engine.onParameterEdited(param);
        if (_UndoSettingsMidiCnfgDataList.size () > MAX_SETTINGS_UNDO_COUNT) // NOTE: UndoDataList last param is Default Data
		{
			auto it = _UndoSettingsMidiCnfgDataList.end ();
			it--;
			it--;// skip default
			_UndoSettingsMidiCnfgDataList.erase (it);
		}
		_UndoSettingsMidiCnfgDataList.insert (_UndoSettingsMidiCnfgDataList.begin (), _settingsMidiCnfgData);
	}
	ParamHandler (moduleSrc, param, true);
}
std::string SettingsMidiCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
void SettingsMidiCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	switch ((SettingsMidiCnfgParam)param->GetParamID ())
	{
		case SettingsMidiCnfgParam::paramUndoCmd:
            if(_UndoSettingsMidiCnfgDataList.size()>1)
            {
                _settingsMidiCnfgData=_UndoSettingsMidiCnfgDataList.front();
                _UndoSettingsMidiCnfgDataList.erase(_UndoSettingsMidiCnfgDataList.begin());
            }
            break;

		case SettingsMidiCnfgParam::SettingDataLocation:
		{
			int8_t DataLocation = param->getValue().get<int8_t>();
			if (DataLocation != _settingsMidiCnfgData.DataLocation)//update only if location change
			{
				_settingsMidiCnfgData.DataLocation = DataLocation;
					GetMidiCnfg();
			}
		}
			break;

		case SettingsMidiCnfgParam::paramConfigMidiActivePort:
			_midiConfigActivePort = param->getValue ().get<int8_t> ();
			break;

		case SettingsMidiCnfgParam::paramConfigMidiClkTxMode:
			_settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].midiClkTxEnabled = param->getValue ().get<int8_t> ();
			break;

		case SettingsMidiCnfgParam::paramConfigMidiThruType:
			_settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].type = (SoftThruType)param->getValue ().get<int8_t> ();
			break;

		case SettingsMidiCnfgParam::paramConfigMidiDestPort:
			_settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].destPort = param->getValue ().get<int8_t> ();
			break;

		case SettingsMidiCnfgParam::paramConfigMidiDestChannel:
			_settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].destChannel = param->getValue ().get<int8_t> ();
			break;

		case SettingsMidiCnfgParam::paramConfigMidiThruFtsOn:
			_settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].thruFTSEnabled = param->getValue ().get<int8_t> ();
			break;

		case SettingsMidiCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (_UndoSettingsMidiCnfgDataList.empty())
				break;
			//fallthrough
#endif

		case SettingsMidiCnfgParam::paramSaveMidiCnfgSetting:
			_settingsMidiCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
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
void SettingsMidiCnfg::SaveData()
{
	if (_UndoSettingsMidiCnfgDataList.empty())
		return;
	if(engine.fileHandler->UpdateMidiCnfgSetting(_settingsMidiCnfgData, (_settingsMidiCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	_UndoSettingsMidiCnfgDataList.clear();
	_UndoSettingsMidiCnfgDataList.insert(_UndoSettingsMidiCnfgDataList.begin(), _settingsMidiCnfgData);
}
int8_t SettingsMidiCnfg::getMidiActivePort (void)
{
	return _midiConfigActivePort;
}

int8_t SettingsMidiCnfg::getMidiClkTxMode (void)
{
	return _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].midiClkTxEnabled;
}

int8_t SettingsMidiCnfg::getMidiThruType (void)
{
	return static_cast <int8_t> (_settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].type);
}

int8_t SettingsMidiCnfg::getMidiDestPort (void)
{
	return _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].destPort;
}

int8_t SettingsMidiCnfg::getMidiDestChannel (void)
{
	return _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].destChannel;
}

int8_t SettingsMidiCnfg::getMidiThruFtsOn (void)
{
	return _settingsMidiCnfgData.MidiCnfg[_midiConfigActivePort].thruFTSEnabled;
}

MidiConfig SettingsMidiCnfg::getMidiConfigForPortId(int8_t portId)
{
	if (!_settingsMidiCnfgData.MidiCnfg.empty())
	{
		return(_settingsMidiCnfgData.MidiCnfg.at(portId));
	}
	MidiConfig tmp;
	return tmp;
}

// Explicit desktop save does not change the selected storage location.
bool SettingsMidiCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateMidiCnfgSetting(_settingsMidiCnfgData, (_settingsMidiCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    _UndoSettingsMidiCnfgDataList.clear(); _UndoSettingsMidiCnfgDataList.push_back(_settingsMidiCnfgData);
    return true;
}
