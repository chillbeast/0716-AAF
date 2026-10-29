#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SettingsManager/SettingsCvioCnfg.h"

using namespace Ignite;

SettingsCvioCnfg::SettingsCvioCnfg (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model),
Module ("SettingsCvioCnfg", (moduleId_t)ModuleName::moduleCvioCnfg, (unsigned int)SettingsCvioCnfgParam::paramSettingsCvioCnfgParamMax, listener)
{
	AddParameter<int8_t>((paramId_t)SettingsCvioCnfgParam::paramActiveCVPort, 0, 7, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramActiveCVCalibPort, 0, 7, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVChannelNumber, 0, 15, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVLaw, 0, 2, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVGateType, 0, 1, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVSlideType, 0, 2, -1);
	AddParameter<int32_t> ((paramId_t)SettingsCvioCnfgParam::paramCVSlideTime, 0, 10000, -1);//range TBD
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVNoteRangeStart, 0, 127, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVNoteRangeEnd, 0, 127, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVNotePercent, 0, 100, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVVelocityPercent, 0, 100, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVCcNum, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramCVCcPercent, 0, 100, 0);
	AddParameter<int16_t> ((paramId_t)SettingsCvioCnfgParam::paramCVPitchBend, -8192, 8191, 0);
	AddParameter<int8_t>((paramId_t)SettingsCvioCnfgParam::paramCVVoltage, 5, 12, 5);

	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramActiveGatePort, 0, 7, -1);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramGateChannelNum, 0, 15, -1);
	AddParameter<int8_t>((paramId_t)SettingsCvioCnfgParam::paramGateType, 0, 1, 0);
	AddParameter<int8_t>((paramId_t)SettingsCvioCnfgParam::paramGateMode, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramGateOutput, 0, 1, 0);

	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvAttack, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvAttackMin, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvAttackMax, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)SettingsCvioCnfgParam::paramEnvAttackName, "0", "0", "0");
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvDecay, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvDecayMin, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvDecayMax, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)SettingsCvioCnfgParam::paramEnvDecayName, "0", "0", "0");
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvSustain, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvSustainMin, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvSustainMax, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)SettingsCvioCnfgParam::paramEnvSustainName, "0", "0", "0");
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvRelease, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvReleaseMin, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvReleaseMax, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)SettingsCvioCnfgParam::paramEnvReleaseName, "0", "0", "0");
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvAmount, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvAmountMin, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvAmountMax, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)SettingsCvioCnfgParam::paramEnvAmountName, "0", "0", "0");
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvVelocity, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvVelocityMin, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramEnvVelocityMax, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)SettingsCvioCnfgParam::paramEnvVelocityName, "0", "0", "0");
	AddParameter<uint8_t> ((paramId_t)SettingsCvioCnfgParam::paramCvNoteType, 0, 1, 0);

	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset1, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset2, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset3, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset4, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset5, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset6, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset7, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset8, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset9, -5.0, 5.0, 0.0);
	AddParameter<float>((paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset10, -5.0, 5.0, 0.0);

	AddParameter<int8_t>((paramId_t)SettingsCvioCnfgParam::paramSaveCvioSetting, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramSaveCvioCalibSetting, 0, 1, 0);
	AddParameter<int8_t>((paramId_t)SettingsCvioCnfgParam::paramFetchCalibData, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::SettingDataLocation, 0, 1, 0);
	AddParameter<int8_t> ((paramId_t)SettingsCvioCnfgParam::paramUndoCmd, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)SettingsCvioCnfgParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SettingsCvioCnfgParam::paramSaveAll, 0, 1, 0);

	SettingsCvioCnfgData settingscvioData = engine.fileHandler->GetCvioCnfgSetting("");
	_calibOffsetData = engine.fileHandler->GetCalibData();

	if (!settingscvioData.cvConfig.empty () && !settingscvioData.gateConfig.empty ())
	{
		for (int8_t i = 0; i < MAX_CVIO_PORTS; ++i)
		{
			_SettingsCvioCnfgData.cvConfig.insert ({ settingscvioData.cvConfig[i].portNum, settingscvioData.cvConfig[i] });
		}
		for (int8_t i = 0; i < MAX_GATE_PORTS; ++i)
		{
			_SettingsCvioCnfgData.gateConfig.insert ({ settingscvioData.gateConfig[i].portNum, settingscvioData.gateConfig[i] });
		}
	}
	else // create with default data
	{
		for (int8_t i = 0; i < MAX_CVIO_PORTS; ++i)
		{
			CV _cv;
			_cv.portNum = i;
			_SettingsCvioCnfgData.cvConfig.insert ({ i, _cv });
		}
		for (int8_t i = 0; i < MAX_GATE_PORTS; ++i)
		{
			Gate _gate;
			_gate.portNum = i;
			_SettingsCvioCnfgData.gateConfig.insert ({ i, _gate });
		}

		engine.fileHandler->UpdateCvioCnfgSetting (_SettingsCvioCnfgData, "");
	}
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleCvioCnfg, true);
	_UndoSettingsCvioCnfgDataList.insert (_UndoSettingsCvioCnfgDataList.begin (), _SettingsCvioCnfgData);
}
void SettingsCvioCnfg::getCvioCnfgData ()
{
	SettingsCvioCnfgData settingscvioData = engine.fileHandler->GetCvioCnfgSetting ((_SettingsCvioCnfgData.DataLocation ? getActiveSongName () : ""));
	if (settingscvioData.cvConfig.empty ())
	{
		engine.fileHandler->UpdateCvioCnfgSetting (_SettingsCvioCnfgData, (_SettingsCvioCnfgData.DataLocation ? getActiveSongName () : ""));
	}
	else if (!settingscvioData.cvConfig.empty () && !settingscvioData.gateConfig.empty ())
	{
		_SettingsCvioCnfgData.cvConfig.clear ();
		_SettingsCvioCnfgData.gateConfig.clear ();
		for (int8_t i = 0; i < MAX_CVIO_PORTS; ++i)
		{
			_SettingsCvioCnfgData.cvConfig.insert ({ settingscvioData.cvConfig[i].portNum, settingscvioData.cvConfig[i] });
		}
		for (int8_t i = 0; i < MAX_GATE_PORTS; ++i)
		{
			_SettingsCvioCnfgData.gateConfig.insert ({ settingscvioData.gateConfig[i].portNum, settingscvioData.gateConfig[i] });
		}
	}
}
void SettingsCvioCnfg::SetDefaultUndoListData ()
{
	//Not used
}
void SettingsCvioCnfg::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter ((*it).param)->setValue (moduleSrc, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> SettingsCvioCnfg::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData = { 0 };

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((SettingsCvioCnfgParam)(*it).param)
		{
			case SettingsCvioCnfgParam::SettingDataLocation:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::SettingDataLocation;
				paramData.MsgValue = _SettingsCvioCnfgData.DataLocation;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVChannelNumber:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVChannelNumber;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].channelNum;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVLaw:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVLaw;
				paramData.MsgValue = static_cast <int8_t> (_SettingsCvioCnfgData.cvConfig[_activeCVPort].law);
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVGateType:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVGateType;
				paramData.MsgValue = static_cast <int8_t> (_SettingsCvioCnfgData.cvConfig[_activeCVPort].gateType);
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVSlideType:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVSlideType;
				paramData.MsgValue = static_cast <int8_t> (_SettingsCvioCnfgData.cvConfig[_activeCVPort].slideType);
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVSlideTime:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVSlideTime;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].slideTime;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVNoteRangeStart:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVNoteRangeStart;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].noteRangeStart;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVNoteRangeEnd:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVNoteRangeEnd;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].noteRangeEnd;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVNotePercent:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVNotePercent;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].notePercent;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVVelocityPercent:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVVelocityPercent;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].velocityPercent;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCcNum:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCcNum;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].ccNum;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCcPercent:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCcPercent;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].ccPercent;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVPitchBend:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVPitchBend;
				paramData.MsgValue = (int16_t)_SettingsCvioCnfgData.cvConfig[_activeCVPort].pitchBend;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVVoltage:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVVoltage;
				paramData.MsgValue = static_cast <int8_t> (_SettingsCvioCnfgData.cvConfig[_activeCVPort].voltage);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramEnvAttack:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvAttack;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttack;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvAttackMin:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvAttackMin;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttackMin;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvAttackMax:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvAttackMax;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttackMax;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvAttackName:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvAttackName;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttackName;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvDecay:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvDecay;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecay;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvDecayMin:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvDecayMin;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecayMin;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvDecayMax:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvDecayMax;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecayMax;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvDecayName:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvDecayName;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecayName;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvSustain:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvSustain;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustain;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvSustainMin:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvSustainMin;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustainMin;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvSustainMax:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvSustainMax;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustainMax;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvSustainName:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvSustainName;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustainName;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvRelease:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvRelease;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvRelease;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvReleaseMin:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvReleaseMin;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvReleaseMin;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvReleaseMax:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvReleaseMax;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvReleaseMax;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvReleaseName:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvReleaseName;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvReleaseName;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvAmount:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvAmount;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmount;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvAmountMin:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvAmountMin;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmountMin;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvAmountMax:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvAmountMax;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmountMax;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvAmountName:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvAmountName;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmountName;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvVelocity:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvVelocity;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocity;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvVelocityMin:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvVelocityMin;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocityMin;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvVelocityMax:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvVelocityMax;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocityMax;
				retParam.push_back (paramData);
				break;
			case SettingsCvioCnfgParam::paramEnvVelocityName:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramEnvVelocityName;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocityName;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramGateChannelNum:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramGateChannelNum;
				paramData.MsgValue = _SettingsCvioCnfgData.gateConfig[_activeGatePort].channelNum;
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramGateType:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramGateType;
				paramData.MsgValue = static_cast <int8_t> (_SettingsCvioCnfgData.gateConfig[_activeGatePort].type);
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramGateMode:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramGateMode;
				paramData.MsgValue = static_cast <int8_t> (_SettingsCvioCnfgData.gateConfig[_activeGatePort].mode);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramGateOutput:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramGateOutput;
				paramData.MsgValue = static_cast <int8_t> (_SettingsCvioCnfgData.gateConfig[_activeGatePort].output);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCvNoteType:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCvNoteType;
				paramData.MsgValue = _SettingsCvioCnfgData.cvConfig[_activeCVPort].cvNoteType;
				juce::Logger::outputDebugString ("CVNoteType sent to  UI :" + (std::to_string (_SettingsCvioCnfgData.cvConfig[_activeCVPort].cvNoteType)));
				retParam.push_back (paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset1:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset1;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[0]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset2:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset2;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[1]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset3:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset3;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[2]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset4:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset4;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[3]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset5:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset5;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[4]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset6:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset6;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[5]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset7:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset7;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[6]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset8:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset8;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[7]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset9:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset9;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[8]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramCVCalibOffset10:
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramCVCalibOffset10;
				paramData.MsgValue = static_cast <float> (_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[9]);
				retParam.push_back(paramData);
				break;

			case SettingsCvioCnfgParam::paramUndoListCmd:
			{
				paramData.param = (paramId_t)SettingsCvioCnfgParam::paramUndoListCmd;
				paramData.Count = (uint32_t)_UndoSettingsCvioCnfgDataList.size () - 1; //set the number of params
				retParam.push_back (paramData);

				if (_UndoSettingsCvioCnfgDataList.size () > 1) // 1 for default
					retParam.push_back (paramData);// intentionally added to indicate undo available

				break;
			}
		}
	}
	return  retParam;
}
void SettingsCvioCnfg::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false, true);
}
void SettingsCvioCnfg::getParam (moduleId_t moduleSrc, paramId_t param)
{
	//this funcation not used; use get param list
}
void SettingsCvioCnfg::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	ParamHandler (moduleSrc, param, true, isPlayMode);
}
void SettingsCvioCnfg::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification, bool isPlayMode)
{
	switch ((SettingsCvioCnfgParam)param->GetParamID ())
	{
		case SettingsCvioCnfgParam::paramUndoCmd:
			if (_UndoSettingsCvioCnfgDataList.size () > 1)
			{
				engine.onParameterEdited(param);
				_UndoSettingsCvioCnfgDataList.erase (_UndoSettingsCvioCnfgDataList.begin ());
				_SettingsCvioCnfgData = _UndoSettingsCvioCnfgDataList.front();
			}
			break;
		case SettingsCvioCnfgParam::paramActiveCVPort:
			_activeCVPort = param->getValue ().get<uint8_t> ();
			if ((_activeCVPort < 0) || (_activeCVPort >= MAX_CVIO_PORTS))
			{
				_activeCVPort = 0; // If Invalid, port set to 0
			}
			break;
		case SettingsCvioCnfgParam::paramActiveCVCalibPort:
			_activeCVCalibPort = param->getValue().get<uint8_t>();
			if ((_activeCVCalibPort < 0) || (_activeCVCalibPort >= MAX_CVIO_PORTS))
			{
				_activeCVCalibPort = 0; // If Invalid, port set to 0
			}
			break;
		case SettingsCvioCnfgParam::SettingDataLocation:
			_SettingsCvioCnfgData.DataLocation = param->getValue ().get<uint8_t> ();
			getCvioCnfgData ();
			break;

		case SettingsCvioCnfgParam::paramCVChannelNumber:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].channelNum = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVLaw:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].law = static_cast <CvLaw> (param->getValue ().get<int8_t> ());
			break;

		case SettingsCvioCnfgParam::paramCVGateType:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].gateType = static_cast <CvGateType> (param->getValue ().get<int8_t> ());
			break;

		case SettingsCvioCnfgParam::paramCVSlideType:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].slideType = static_cast <CvSlideType> (param->getValue ().get<int8_t> ());
			break;

		case SettingsCvioCnfgParam::paramCVSlideTime:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].slideTime = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVNoteRangeStart:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].noteRangeStart = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVNoteRangeEnd:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].noteRangeEnd = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVNotePercent:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].notePercent = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVVelocityPercent:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].velocityPercent = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVCcNum:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].ccNum = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVCcPercent:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].ccPercent = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVPitchBend:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].pitchBend = param->getValue ().get<int16_t> ();
			break;

		case SettingsCvioCnfgParam::paramCVVoltage:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].voltage = static_cast <CVPeakVoltage> (param->getValue().get<int8_t>());
			break;

		case SettingsCvioCnfgParam::paramEnvAttack:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttack = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvAttackMin:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttackMin = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvAttackMax:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttackMax = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvAttackName:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttackName = (std::string)(param->getValue ().get<std::string> ().c_str ());
			break;
		case SettingsCvioCnfgParam::paramEnvDecay:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecay = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvDecayMin:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecayMin = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvDecayMax:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecayMax = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvDecayName:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecayName = (std::string)(param->getValue ().get<std::string> ().c_str ());
			break;
		case SettingsCvioCnfgParam::paramEnvSustain:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustain = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvSustainMin:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustainMin = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvSustainMax:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustainMax = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvSustainName:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustainName = (std::string)(param->getValue ().get<std::string> ().c_str ());
			break;
		case SettingsCvioCnfgParam::paramEnvRelease:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvRelease = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvReleaseMin:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvReleaseMin = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvReleaseMax:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvReleaseMax = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvReleaseName:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvReleaseName = (std::string)(param->getValue ().get<std::string> ().c_str ());
			break;
		case SettingsCvioCnfgParam::paramEnvAmount:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmount = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvAmountMin:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmountMin = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvAmountMax:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmountMax = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvAmountName:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmountName = (std::string)(param->getValue ().get<std::string> ().c_str ());
			break;
		case SettingsCvioCnfgParam::paramEnvVelocity:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocity = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvVelocityMin:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocityMin = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvVelocityMax:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocityMax = param->getValue ().get<int8_t> ();
			break;
		case SettingsCvioCnfgParam::paramEnvVelocityName:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocityName = (std::string)(param->getValue ().get<std::string> ().c_str ());
			break;
		case SettingsCvioCnfgParam::paramCvNoteType:
			_SettingsCvioCnfgData.cvConfig[_activeCVPort].cvNoteType = param->getValue ().get<uint8_t> ();
			juce::Logger::outputDebugString ("CVNoteType received from UI :" + (std::to_string (_SettingsCvioCnfgData.cvConfig[_activeCVPort].cvNoteType)));
			break;

		case SettingsCvioCnfgParam::paramActiveGatePort:
			_activeGatePort = param->getValue ().get<uint8_t> ();
			if (_activeGatePort < 0 || _activeGatePort >= MAX_GATE_PORTS)
				_activeGatePort = 0; // If Invalid, port set to 0
			break;

		case SettingsCvioCnfgParam::paramGateChannelNum:
			_SettingsCvioCnfgData.gateConfig[_activeGatePort].channelNum = param->getValue ().get<int8_t> ();
			break;

		case SettingsCvioCnfgParam::paramGateType:
			_SettingsCvioCnfgData.gateConfig[_activeGatePort].type = static_cast <GateType> (param->getValue ().get<int8_t> ());
			break;

		case SettingsCvioCnfgParam::paramGateMode:
			_SettingsCvioCnfgData.gateConfig[_activeGatePort].mode = static_cast <CvGateType> (param->getValue().get<int8_t>());
			break;

		case SettingsCvioCnfgParam::paramGateOutput:
			_SettingsCvioCnfgData.gateConfig[_activeGatePort].output = static_cast <gatePeakVoltage> (param->getValue().get<int8_t>());
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset1:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[0] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset2:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[1] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset3:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[2] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset4:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[3] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset5:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[4] = param->getValue().get<float>();

		case SettingsCvioCnfgParam::paramCVCalibOffset6:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[5] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset7:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[6] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset8:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[7] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset9:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[8] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramCVCalibOffset10:
			_calibOffsetData->getReference(_activeCVCalibPort).calibOffset[9] = param->getValue().get<float>();
			break;

		case SettingsCvioCnfgParam::paramSaveAll:
#ifdef _WIN32
            saveForDesktop(); break;
#else

			if (!_UndoSettingsCvioCnfgDataList.empty())
				SaveData();
			break;
			//fallthrough
#endif

		case SettingsCvioCnfgParam::paramSaveCvioSetting:
			_SettingsCvioCnfgData.DataLocation = param->getValue().get<int8_t>();
			SaveData();
			break;

		case SettingsCvioCnfgParam::paramSaveCvioCalibSetting:
			engine.fileHandler->UpdateCalibData(_calibOffsetData);
			break;

		case SettingsCvioCnfgParam::paramFetchCalibData:
			_calibOffsetData = engine.fileHandler->GetCalibData();
			break;

		default:
			assert (0);
			break;
	}
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramSaveAll &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramSaveCvioSetting &&
		(SettingsCvioCnfgParam)param->GetParamID () != SettingsCvioCnfgParam::paramSaveCvioCalibSetting &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::SettingDataLocation &&
		(SettingsCvioCnfgParam)param->GetParamID () != SettingsCvioCnfgParam::paramFetchCalibData &&
		(SettingsCvioCnfgParam)param->GetParamID () != SettingsCvioCnfgParam::paramUndoListCmd &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramActiveCVPort &&
		(SettingsCvioCnfgParam)param->GetParamID () != SettingsCvioCnfgParam::paramActiveCVCalibPort &&
		(SettingsCvioCnfgParam)param->GetParamID () != SettingsCvioCnfgParam::paramActiveGatePort &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset1 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset2 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset3 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset4 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset5 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset6 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset7 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset8 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset9 &&
		(SettingsCvioCnfgParam)param->GetParamID() != SettingsCvioCnfgParam::paramCVCalibOffset10 &&
		(SettingsCvioCnfgParam)param->GetParamID () != SettingsCvioCnfgParam::paramUndoCmd
		)
	{
		engine.onParameterEdited(param);
		if (_UndoSettingsCvioCnfgDataList.size () > MAX_SETTINGS_UNDO_COUNT) // NOTE: UndoDataList last param is Default Data
		{
			auto it = _UndoSettingsCvioCnfgDataList.end ();
			it--;
			it--;// skip default
			_UndoSettingsCvioCnfgDataList.erase (it);
		}
		_UndoSettingsCvioCnfgDataList.insert (_UndoSettingsCvioCnfgDataList.begin (), _SettingsCvioCnfgData);
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
void SettingsCvioCnfg::SaveData()
{
	if (_UndoSettingsCvioCnfgDataList.empty())
		return;
	if(engine.fileHandler->UpdateCvioCnfgSetting(_SettingsCvioCnfgData, (_SettingsCvioCnfgData.DataLocation ? getActiveSongName() : "")) != FILE_OK) return;
	_UndoSettingsCvioCnfgDataList.clear();
	_UndoSettingsCvioCnfgDataList.insert(_UndoSettingsCvioCnfgDataList.begin(), _SettingsCvioCnfgData);
	engine.fileHandler->UpdateCalibData(_calibOffsetData);
}
int8_t SettingsCvioCnfg::getActiveCVPort (void)
{
	return _activeCVPort;
}

int8_t SettingsCvioCnfg::getVChannelNumber (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].channelNum;
}

int8_t SettingsCvioCnfg::getVLaw (int8_t CVPort)
{
	return static_cast <int8_t> (_SettingsCvioCnfgData.cvConfig[_activeCVPort].law);
}

int8_t SettingsCvioCnfg::getVGateType (int8_t CVPort)
{
	return static_cast <int8_t> (_SettingsCvioCnfgData.cvConfig[_activeCVPort].gateType);
}

int8_t SettingsCvioCnfg::getVSlideType (int8_t CVPort)
{
	return static_cast <int8_t> (_SettingsCvioCnfgData.cvConfig[_activeCVPort].slideType);
}

int32_t SettingsCvioCnfg::getVSlideTime (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].slideTime;
}

int8_t SettingsCvioCnfg::getVNoteRangeStart (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].noteRangeStart;
}

int8_t SettingsCvioCnfg::getVNoteRangeEnd (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].noteRangeEnd;
}

int8_t SettingsCvioCnfg::getVNotePercent (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].notePercent;
}

int8_t SettingsCvioCnfg::getVVelocityPercent (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].velocityPercent;
}

int8_t SettingsCvioCnfg::getVCcNum (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].ccNum;
}

int8_t SettingsCvioCnfg::getVCcPercent (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].ccPercent;
}

int16_t SettingsCvioCnfg::getVPitchBend (int8_t CVPort)
{
	return _SettingsCvioCnfgData.cvConfig[_activeCVPort].pitchBend;
}

int8_t SettingsCvioCnfg::getVVoltage(int8_t CVPort)
{
	return static_cast <int8_t> (_SettingsCvioCnfgData.cvConfig[_activeCVPort].voltage);
}

int8_t SettingsCvioCnfg::getEnvAttack (int8_t CVPort)
{
	return static_cast<int8_t>(_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAttack);
}
int8_t SettingsCvioCnfg::getEnvDecay (int8_t CVPort)
{
	return static_cast<int8_t>(_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvDecay);
}
int8_t SettingsCvioCnfg::getEnvSustain (int8_t CVPort)
{
	return static_cast<int8_t>(_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvSustain);
}
int8_t SettingsCvioCnfg::getEnvRelease (int8_t CVPort)
{
	return static_cast<int8_t>(_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvRelease);
}
int8_t SettingsCvioCnfg::getEnvAmount (int8_t CVPort)
{
	return static_cast<int8_t>(_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvAmount);
}
int8_t SettingsCvioCnfg::getEnvVelocity (int8_t CVPort)
{
	return static_cast<int8_t>(_SettingsCvioCnfgData.cvConfig[_activeCVPort].EnvVelocity);
}

int8_t SettingsCvioCnfg::getActiveGatePort (void)
{
	return _activeGatePort;
}

int8_t SettingsCvioCnfg::getGateChannelNum (int8_t GatePort)
{
	return _SettingsCvioCnfgData.gateConfig[_activeGatePort].channelNum;
}

int8_t SettingsCvioCnfg::getGateType (int8_t GatePort)
{
	return static_cast <int8_t> (_SettingsCvioCnfgData.gateConfig[_activeGatePort].type);
}

std::string SettingsCvioCnfg::getActiveSongName ()
{
	if (engine.songManager->activeSong != nullptr)
		return (engine.songManager->activeSong->getName ());

	return("");
}
// Explicit desktop save does not change the selected storage location.
bool SettingsCvioCnfg::saveForDesktop()
{
    const auto result=engine.fileHandler->UpdateCvioCnfgSetting(_SettingsCvioCnfgData, (_SettingsCvioCnfgData.DataLocation ? getActiveSongName() : ""));
    if(result!=FILE_OK) {engine.desktopChanges.writeFailed(); return false;}
    SetDefaultUndoListData();
    ClearUndoList();
    _UndoSettingsCvioCnfgDataList.clear(); _UndoSettingsCvioCnfgDataList.push_back(_SettingsCvioCnfgData);
    return true;
}
