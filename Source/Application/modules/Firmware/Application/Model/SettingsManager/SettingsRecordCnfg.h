#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsRecordCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsRecordCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsRecordCnfg (const SettingsRecordCnfg&) = delete;
		SettingsRecordCnfg& operator= (const SettingsRecordCnfg&) = delete;
		SettingsRecordCnfg (SettingsRecordCnfg&&) = delete;
		SettingsRecordCnfg& operator= (SettingsRecordCnfg&&) = delete;
		~SettingsRecordCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		void SetDefaultUndoListData ();
		void SaveData();
		bool isUndilistNeedUpdate = true;
		SettingsRecordCnfgData _SettingsRecordCnfgData;
		int8_t _midiConfigActivePort = -1;
	};
}