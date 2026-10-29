#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsUdsCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsUdsCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsUdsCnfg (const SettingsUdsCnfg&) = delete;
		SettingsUdsCnfg& operator= (const SettingsUdsCnfg&) = delete;
		SettingsUdsCnfg (SettingsUdsCnfg&&) = delete;
		SettingsUdsCnfg& operator= (SettingsUdsCnfg&&) = delete;
		~SettingsUdsCnfg () = default;

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();
		int8_t getActiveUds(void);
		void setActiveUds (int8_t index);
		int8_t getUdsCount(void);
		std::string getUdsName(void);
		void setUdsName (std::string name);
		int8_t getUdsNote (int8_t index);
		void setUdsNote (int8_t index, int8_t note);
		std::vector <int8_t> generateNoteIntervalList (std::vector <int8_t> noteList);
		void updateUdsToGlobalScaleList (int8_t index, std::vector <int8_t> intervalList, std::string name);
		void updateUdsNotesToGlobalScaleList (int8_t index, std::vector <int8_t> intervalList);
		void updateUdsNameToGlobalScaleList (int8_t index, std::string name);

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		SettingsUdsCnfgData _settingsUdsCnfgData;
		int8_t _midiConfigActivePort = -1;
		bool isUndilistNeedUpdate = true;
		void SetDefaultUndoListData ();
		void SaveData();
		int8_t activeUds = 0;
	};
}