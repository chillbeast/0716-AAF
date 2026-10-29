#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsMidiMachineControlCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsMidiMachineControlCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsMidiMachineControlCnfg (const SettingsMidiMachineControlCnfg&) = delete;
		SettingsMidiMachineControlCnfg& operator= (const SettingsMidiMachineControlCnfg&) = delete;
		SettingsMidiMachineControlCnfg (SettingsMidiMachineControlCnfg&&) = delete;
		SettingsMidiMachineControlCnfg& operator= (SettingsMidiMachineControlCnfg&&) = delete;
		~SettingsMidiMachineControlCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();

		SettingsMidiMachineControlCnfgData _SettingsMidiMachineControlCnfgData;
	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		void SetDefaultUndoListData ();
		void SaveData();
		bool isUndilistNeedUpdate = true;

		int8_t _midiConfigActivePort = -1;
	};
}