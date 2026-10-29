#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsMidiCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsMidiCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsMidiCnfg (const SettingsMidiCnfg&) = delete;
		SettingsMidiCnfg& operator= (const SettingsMidiCnfg&) = delete;
		SettingsMidiCnfg (SettingsMidiCnfg&&) = delete;
		SettingsMidiCnfg& operator= (SettingsMidiCnfg&&) = delete;
		~SettingsMidiCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;

		int8_t getMidiActivePort (void);
		int8_t getMidiClkTxMode (void);
		int8_t getMidiThruType (void);
		int8_t getMidiDestPort (void);
		int8_t getMidiDestChannel (void);
		int8_t getMidiThruFtsOn (void);
		std::string getActiveSongName ();
		MidiConfig getMidiConfigForPortId(int8_t portId);
	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		bool isUndilistNeedUpdate = true;
		void SetDefaultUndoListData ();
		void GetMidiCnfg ();
		SettingsMidiCnfgData _settingsMidiCnfgData;
		std::vector<SettingsMidiCnfgData> _UndoSettingsMidiCnfgDataList;
		int8_t _midiConfigActivePort = -1;
		void SaveData();
	};
}
