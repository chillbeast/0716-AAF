#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsMidiClkCnfg.h"

using namespace Ignite;

SettingsMidiClkCnfg::SettingsMidiClkCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsMidiClkCnfg", (moduleId_t)ModuleName::moduleSettingsMidiClkCnfg, static_cast <unsigned int> (SettingsMidiClkCnfgParam::paramSettingsMidiClkCnfgParamMax), listener)
{
	AddParameter<int8_t> ((paramId_t)SettingsMidiClkCnfgParam::clkInPort, 0, IGNITE_CLK_PORT_MAX, 0);
	AddParameter<uint32_t>((paramId_t)SettingsMidiClkCnfgParam::clkOutPort, 0, 0x3FFFF, 0);//refer midiClkOutPorts for range info
	AddParameter<int8_t> ((paramId_t)SettingsMidiClkCnfgParam::inputPpqn, 0, static_cast <int8_t> (SettingsMidiClkCnfgInputClkRate::inClkRateMax), static_cast <int8_t> (SettingsMidiClkCnfgInputClkRate::inClkRate_24PPQ));
	AddParameter<int8_t> ((paramId_t)SettingsMidiClkCnfgParam::outputPpqn, 0, static_cast <int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRateMax), static_cast <int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRate_24PPQ));
	AddParameter<int8_t>((paramId_t)SettingsMidiClkCnfgParam::triggerForIn, 0, 8, 0);
	AddParameter<int8_t>((paramId_t)SettingsMidiClkCnfgParam::triggerForOut, 0, 8, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiClkCnfgParam::saveCmd, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiClkCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsMidiClkCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsMidiClkCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsMidiClkCnfgParam::paramSaveAll, 0, 1, 0);

	
	_settingsMidiClkCnfgData.midiClkOutPorts = 0;

	if (!engine.fileHandler->getSettingsMidiClkCnfgDataSetting(&_settingsMidiClkCnfgData, ""))
	{
		engine.fileHandler->updateSettingsMidiClkCnfgData(_settingsMidiClkCnfgData, ""); // create with default data
	}

	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSettingsMidiClkCnfg, true);
	setDefaultUndoListData ();
}

void SettingsMidiClkCnfg::setDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)SettingsMidiClkCnfgParam::paramall + 1; i < (paramId_t)SettingsMidiClkCnfgParam::paramSettingsMidiClkCnfgParamMax; i++)
	{
		if ((i == (paramId_t)SettingsMidiClkCnfgParam::saveCmd) ||
			(i == (paramId_t)SettingsMidiClkCnfgParam::SettingDataLocation) ||
			(i == (paramId_t)SettingsMidiClkCnfgParam::paramUndoListCmd) ||
			(i == (paramId_t)SettingsMidiClkCnfgParam::paramUndoCmd)
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
	retParam = getParamList ((moduleId_t)ModuleName::moduleSettingsMidiClkCnfg, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}

void SettingsMidiClkCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsMidiClkCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsMidiClkCnfgParam)(*it).param)
		{
			case SettingsMidiClkCnfgParam::clkInPort:
				paramData.param = (paramId_t)SettingsMidiClkCnfgParam::clkInPort;
				paramData.MsgValue = _settingsMidiClkCnfgData.midiClkInPort;
				retParam.push_back (paramData);
				break;

			case SettingsMidiClkCnfgParam::clkOutPort:
				paramData.param = (*it).param;
				paramData.MsgValue = _settingsMidiClkCnfgData.midiClkOutPorts;
				retParam.push_back(paramData);
			break;

			case SettingsMidiClkCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsMidiClkCnfgParam::SettingDataLocation;
				paramData.MsgValue = _settingsMidiClkCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsMidiClkCnfgParam::inputPpqn:
				paramData.param = (paramId_t)SettingsMidiClkCnfgParam::inputPpqn;
				paramData.MsgValue = (int8_t)_settingsMidiClkCnfgData.MidiClkInputClkRate;
				retParam.push_back (paramData);
				break;

			case SettingsMidiClkCnfgParam::outputPpqn:
				paramData.param = (paramId_t)SettingsMidiClkCnfgParam::outputPpqn;
				paramData.MsgValue = (int8_t)_settingsMidiClkCnfgData.MidiClkOutPutClkRate;
				retParam.push_back (paramData);
				break;

			case SettingsMidiClkCnfgParam::triggerForIn:
				paramData.param = (paramId_t)SettingsMidiClkCnfgParam::triggerForIn;
				paramData.MsgValue = (int8_t)_settingsMidiClkCnfgData.SyncTriggerForIn;
				retParam.push_back(paramData);
				break;

			case SettingsMidiClkCnfgParam::triggerForOut:
				paramData.param = (paramId_t)SettingsMidiClkCnfgParam::triggerForOut;
				paramData.MsgValue = (int8_t)_settingsMidiClkCnfgData.SyncTriggerForOut;
				retParam.push_back(paramData);
				break;

			case SettingsMidiClkCnfgParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();

				paramData.param = (paramId_t)SettingsMidiClkCnfgParam::paramUndoListCmd;
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

void SettingsMidiClkCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}

void SettingsMidiClkCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}

void SettingsMidiClkCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsMidiClkCnfgParam)param->GetParamID () != SettingsMidiClkCnfgParam::saveCmd &&
		(SettingsMidiClkCnfgParam)param->GetParamID () != SettingsMidiClkCnfgParam::paramSaveAll &&
		(SettingsMidiClkCnfgParam)param->GetParamID () != SettingsMidiClkCnfgParam::SettingDataLocation &&
		(SettingsMidiClkCnfgParam)param->GetParamID () != SettingsMidiClkCnfgParam::paramUndoListCmd &&
		(SettingsMidiClkCnfgParam)param->GetParamID () != SettingsMidiClkCnfgParam::paramUndoCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}

std::string SettingsMidiClkCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}

void SettingsMidiClkCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate)
	{
		isUndilistNeedUpdate = false;
		setDefaultUndoListData ();
	}

	switch ((SettingsMidiClkCnfgParam)param->GetParamID ())
	{
		case SettingsMidiClkCnfgParam::paramUndoCmd:
			doParamUndo ();
			break;

		case SettingsMidiClkCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (GetUndoParamsInList().empty())
				break;
			//fallthrough
#endif

		case SettingsMidiClkCnfgParam::saveCmd:
			_settingsMidiClkCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			SaveData();
			break;

		case SettingsMidiClkCnfgParam::SettingDataLocation:
			_settingsMidiClkCnfgData.DataLocation = (param->getValue ().get<int8_t> ());
			engine.fileHandler->getSettingsMidiClkCnfgDataSetting (&_settingsMidiClkCnfgData, (_settingsMidiClkCnfgData.DataLocation ? getActiveSongName () : ""));
			break;

		case SettingsMidiClkCnfgParam::inputPpqn:
			_settingsMidiClkCnfgData.MidiClkInputClkRate = (param->getValue ().get<int8_t> ());
			break;

		case SettingsMidiClkCnfgParam::outputPpqn:
			_settingsMidiClkCnfgData.MidiClkOutPutClkRate = (param->getValue ().get<int8_t> ());
			informClkOutPpqnChangeToSongManager();
			break;

		case SettingsMidiClkCnfgParam::clkInPort:
			_settingsMidiClkCnfgData.midiClkInPort = (param->getValue().get<int8_t>());
			informClkSrcToSongManager();
			break;

		case SettingsMidiClkCnfgParam::clkOutPort:
			_settingsMidiClkCnfgData.midiClkOutPorts = (param->getValue().get<uint32_t>());
			informClkOutPortChangeToSongManager();

		break;

		case SettingsMidiClkCnfgParam::triggerForIn:
			_settingsMidiClkCnfgData.SyncTriggerForIn = (param->getValue().get<int8_t>());
			break;
		case SettingsMidiClkCnfgParam::triggerForOut:
			_settingsMidiClkCnfgData.SyncTriggerForOut = (param->getValue().get<int8_t>());
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
void SettingsMidiClkCnfg::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if(engine.fileHandler->updateSettingsMidiClkCnfgData(_settingsMidiClkCnfgData, (_settingsMidiClkCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	ClearUndoList();
	isUndilistNeedUpdate = true;
}
AnyType SettingsMidiClkCnfg::getParamSingle(SettingsMidiClkCnfgParam param)
{
	AnyType ret = 0;

	switch ((SettingsMidiClkCnfgParam)param)
	{
	case SettingsMidiClkCnfgParam::clkInPort:
		ret =  _settingsMidiClkCnfgData.midiClkInPort;
		break;

	case SettingsMidiClkCnfgParam::SettingDataLocation:
		ret =  _settingsMidiClkCnfgData.DataLocation;
		break;

	case SettingsMidiClkCnfgParam::inputPpqn:
		ret = _settingsMidiClkCnfgData.MidiClkInputClkRate;
		break;

	case SettingsMidiClkCnfgParam::outputPpqn:
		ret = _settingsMidiClkCnfgData.MidiClkOutPutClkRate;
		break;

	case SettingsMidiClkCnfgParam::clkOutPort:
		ret = _settingsMidiClkCnfgData.midiClkOutPorts;
		break;

	case SettingsMidiClkCnfgParam::triggerForIn:
		ret = _settingsMidiClkCnfgData.SyncTriggerForIn;
		break;

	case SettingsMidiClkCnfgParam::triggerForOut:
		ret = _settingsMidiClkCnfgData.SyncTriggerForOut;
		break;

	default:
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR Unknown param requested");
		break;
	}

	return ret;
}

/*
 * Initialise songmanger settings, if clock source is external
 */
void SettingsMidiClkCnfg::informClkSrcToSongManager(void)
{
	if (isClkInFromDinMidiPort() || isClkInFromUsbMidiPort())
	{
		//going to clock slave mode, set-up bpm calcuator for DIN MIDI and USB MIDI
		engine.songManager->initMidiClkCalculator();
	}
	else if (isClockInternal())
	{
		//going to clock master mode, stop any BPM calculation from external midi
		engine.songManager->deInitMidiClkCalculator();
	}
	//no action for sync clock here
}

/*
* Any init settings which need to be done
* which has dependancy on other modules
*/
void SettingsMidiClkCnfg::init(void)
{
	//inform song manager about clk src
	informClkSrcToSongManager();
}

bool SettingsMidiClkCnfg::isClkInFromDinMidiPort(void)
{
	if ( (_settingsMidiClkCnfgData.midiClkInPort >= IGNITE_CLK_PORT_DIN_MIDI_PORT_1) && (_settingsMidiClkCnfgData.midiClkInPort <= IGNITE_CLK_PORT_DIN_MIDI_PORT_5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkInFromUsbMidiPort(void)
{
	//covers 6 device midi ports and  host midi port
	if ( isClkInFromUsbDeviceMidiPort() || isClkInFromUsbHostMidiPort())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClockInternal(void)
{
	if (_settingsMidiClkCnfgData.midiClkInPort == IGNITE_CLK_PORT_INTERNAL_CLK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkInFromSyncPort(void)
{
	if (_settingsMidiClkCnfgData.midiClkInPort == IGNITE_CLK_PORT_SYNC_PORT)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SettingsMidiClkCnfg::informClkOutPpqnChangeToSongManager(void)
{
	engine.songManager->handleClkOutPpqnChange();
}

void SettingsMidiClkCnfg::informClkOutPortChangeToSongManager(void)
{
	engine.songManager->handleClkOutPortChange();
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnSyncPort (void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_SYNC_PORT))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnDin1Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_DIN_MIDI_PORT_1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnDin2Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_DIN_MIDI_PORT_2))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnDin3Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_DIN_MIDI_PORT_3))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnDin4Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_DIN_MIDI_PORT_4))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnDin5Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_DIN_MIDI_PORT_5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbD1Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_D_MIDI_PORT_1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbD2Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_D_MIDI_PORT_2))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbD3Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_D_MIDI_PORT_3))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbD4Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_D_MIDI_PORT_4))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbD5Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_D_MIDI_PORT_5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbD6Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_D_MIDI_PORT_6))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbH1Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbH2Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_2))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbH3Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_3))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbH4Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_4))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkOutEnabledOnUsbH5Port(void)
{
	if (_settingsMidiClkCnfgData.midiClkOutPorts & (1 << IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkInFromUsbDeviceMidiPort(void)
{
	if ((_settingsMidiClkCnfgData.midiClkInPort >= IGNITE_CLK_PORT_USB_D_MIDI_PORT_1) && (_settingsMidiClkCnfgData.midiClkInPort <= IGNITE_CLK_PORT_USB_D_MIDI_PORT_6))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SettingsMidiClkCnfg::isClkInFromUsbHostMidiPort(void)
{
	if ((_settingsMidiClkCnfgData.midiClkInPort >= IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_1) && (_settingsMidiClkCnfgData.midiClkInPort < (IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_1 + IGNITE_MAX_USB_HOST_MIDI_DEVICES)))
	{
		return true;
	}
	else
	{
		return false;
	}
}


// Explicit desktop save does not change the selected storage location.
bool SettingsMidiClkCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->updateSettingsMidiClkCnfgData(_settingsMidiClkCnfgData, (_settingsMidiClkCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    setDefaultUndoListData();
    ClearUndoList();
    return true;
}
