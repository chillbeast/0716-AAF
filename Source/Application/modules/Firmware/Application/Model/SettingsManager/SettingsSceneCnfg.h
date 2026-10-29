#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsSceneCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsSceneCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsSceneCnfg (const SettingsSceneCnfg&) = delete;
		SettingsSceneCnfg& operator= (const SettingsSceneCnfg&) = delete;
		SettingsSceneCnfg (SettingsSceneCnfg&&) = delete;
		SettingsSceneCnfg& operator= (SettingsSceneCnfg&&) = delete;
		~SettingsSceneCnfg() = default;

		void setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam(moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName();

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		void SetDefaultUndoListData ();
		void SaveData();
		bool isUndilistNeedUpdate = true;
		SettingsSceneCnfgData _SettingsSceneCnfgData;
		int8_t _midiConfigActivePort = -1;
	};
}