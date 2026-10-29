#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsSongCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsSongCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsSongCnfg (const SettingsSongCnfg&) = delete;
		SettingsSongCnfg& operator= (const SettingsSongCnfg&) = delete;
		SettingsSongCnfg (SettingsSongCnfg&&) = delete;
		SettingsSongCnfg& operator= (SettingsSongCnfg&&) = delete;
		~SettingsSongCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();
		int8_t getSongTapTempo ();
		SettingsSongCnfgData _SettingsSongCnfgData;

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		void SetDefaultUndoListData ();
		void SaveData();
		bool isUndilistNeedUpdate = true;
		int8_t _midiConfigActivePort = -1;
	};
}