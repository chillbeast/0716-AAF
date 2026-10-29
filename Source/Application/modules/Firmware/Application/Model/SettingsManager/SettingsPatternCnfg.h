#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsPatternCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsPatternCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsPatternCnfg (const SettingsPatternCnfg&) = delete;
		SettingsPatternCnfg& operator= (const SettingsPatternCnfg&) = delete;
		SettingsPatternCnfg (SettingsPatternCnfg&&) = delete;
		SettingsPatternCnfg& operator= (SettingsPatternCnfg&&) = delete;
		~SettingsPatternCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();

		SettingsPatternCnfgData _SettingsPatternCnfgData;

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		bool isUndilistNeedUpdate = true;
		void SetDefaultUndoListData ();
		void SaveData();
		int8_t _midiConfigActivePort = -1;
	};
}