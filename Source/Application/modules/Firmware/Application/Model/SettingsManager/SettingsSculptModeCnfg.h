#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsSculptModeCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsSculptModeCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsSculptModeCnfg (const SettingsSculptModeCnfg&) = delete;
		SettingsSculptModeCnfg& operator= (const SettingsSculptModeCnfg&) = delete;
		SettingsSculptModeCnfg (SettingsSculptModeCnfg&&) = delete;
		SettingsSculptModeCnfg& operator= (SettingsSculptModeCnfg&&) = delete;
		~SettingsSculptModeCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();

		SettingsSculptModeCnfgData _SettingsSculptModeCnfgData;

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		void SetDefaultUndoListData ();
		void SaveData();
		bool isUndilistNeedUpdate = true;
		std::vector<SettingsSculptModeCnfgData> _UndoSettingsSculptModeCnfgDataList;
		int8_t _midiConfigActivePort = -1;
	};
}
