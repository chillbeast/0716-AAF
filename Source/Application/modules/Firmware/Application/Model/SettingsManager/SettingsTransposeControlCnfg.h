#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsTransposeControlCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsTransposeControlCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsTransposeControlCnfg (const SettingsTransposeControlCnfg&) = delete;
		SettingsTransposeControlCnfg& operator= (const SettingsTransposeControlCnfg&) = delete;
		SettingsTransposeControlCnfg (SettingsTransposeControlCnfg&&) = delete;
		SettingsTransposeControlCnfg& operator= (SettingsTransposeControlCnfg&&) = delete;
		~SettingsTransposeControlCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();

		TransposeControlCnfgData _transposeControlCnfgData;
	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		void SetDefaultUndoListData ();
		void SaveData();
		bool isUndilistNeedUpdate = true;
	};
}