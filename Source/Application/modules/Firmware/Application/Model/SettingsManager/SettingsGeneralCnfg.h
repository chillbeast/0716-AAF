#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsGeneralCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsGeneralCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsGeneralCnfg (const SettingsGeneralCnfg&) = delete;
		SettingsGeneralCnfg& operator= (const SettingsGeneralCnfg&) = delete;
		SettingsGeneralCnfg (SettingsGeneralCnfg&&) = delete;
		SettingsGeneralCnfg& operator= (SettingsGeneralCnfg&&) = delete;
		~SettingsGeneralCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		int getAuditioningPortPreset(uint8_t portID);
		SettingsGeneralCnfgData _SettingsGeneralCnfgData;

	private:
		//void SaveData();
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		bool isUndilistNeedUpdate = true;
		void SetDefaultUndoListData ();
		int8_t _midiConfigActivePort = -1;
	};
}
