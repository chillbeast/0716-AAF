#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsCvioCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsCvioCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsCvioCnfg (const SettingsCvioCnfg&) = delete;
		SettingsCvioCnfg& operator= (const SettingsCvioCnfg&) = delete;
		SettingsCvioCnfg (SettingsCvioCnfg&&) = delete;
		SettingsCvioCnfg& operator= (SettingsCvioCnfg&&) = delete;
		~SettingsCvioCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification, bool isPlayMode);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;

		int8_t getActiveCVPort (void);

		int8_t getVChannelNumber (int8_t CVPort);
		int8_t getVLaw (int8_t CVPort);
		int8_t getVGateType (int8_t CVPort);
		int8_t getVSlideType (int8_t CVPort);
		int32_t getVSlideTime (int8_t CVPort);
		int8_t getVNoteRangeStart (int8_t CVPort);
		int8_t getVNoteRangeEnd (int8_t CVPort);
		int8_t getVNotePercent (int8_t CVPort);
		int8_t getVVelocityPercent (int8_t CVPort);
		int8_t getVCcNum (int8_t CVPort);
		int8_t getVCcPercent (int8_t CVPort);
		int16_t getVPitchBend (int8_t CVPort);
		int8_t getVVoltage(int8_t CVPort);
		int8_t getEnvAttack (int8_t CVPort);
		int8_t getEnvDecay (int8_t CVPort);
		int8_t getEnvSustain (int8_t CVPort);
		int8_t getEnvRelease (int8_t CVPort);
		int8_t getEnvAmount (int8_t CVPort);
		int8_t getEnvVelocity (int8_t CVPort);

		int8_t getActiveGatePort (void);
		int8_t getGateChannelNum (int8_t GatePort);
		int8_t getGateType (int8_t GatePort);

		SettingsCvioCnfgData _SettingsCvioCnfgData;
		Array <CalibrationData> * _calibOffsetData;
		std::vector<SettingsCvioCnfgData> _UndoSettingsCvioCnfgDataList;
		std::string getActiveSongName ();

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		bool isUndilistNeedUpdate = true;
		void SetDefaultUndoListData ();
		void getCvioCnfgData ();
		void SaveData();

		int8_t _activeCVPort = -1;
		int8_t _activeCVCalibPort = -1;
		int8_t _activeGatePort = -1;
	};
}
