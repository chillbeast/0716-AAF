#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsBRayCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsBRayCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsBRayCnfg (const SettingsBRayCnfg&) = delete;
		SettingsBRayCnfg& operator= (const SettingsBRayCnfg&) = delete;
		SettingsBRayCnfg (SettingsBRayCnfg&&) = delete;
		SettingsBRayCnfg& operator= (SettingsBRayCnfg&&) = delete;
		~SettingsBRayCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();
		void SaveData();

		SettingsBRayCnfgData _settingsBRayCnfgData;
		uint16_t brayAdcValue = 0; //12 bit adc present in MCU GD32F407
		//int8_t brayVelocity = -1;

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		
		int8_t _midiConfigActivePort = -1;
		bool isUndilistNeedUpdate = true;
		void SetDefaultUndoListData ();
	};
}