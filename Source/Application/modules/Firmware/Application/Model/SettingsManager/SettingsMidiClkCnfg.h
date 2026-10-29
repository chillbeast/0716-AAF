#pragma once
#include <stdint.h>
#include <string>
#include "module.h"
#include "Common/Modules/CSong.h"
#include "Firmware/Application/Model/ModelParams.h"

namespace Ignite
{
	class IgniteEngine;
	class SettingsMidiClkCnfg : public Module
	{
	public:
        bool saveForDesktop();
		SettingsMidiClkCnfg (IParamChangeListener* listener, IgniteEngine& model);
		SettingsMidiClkCnfg (const SettingsMidiClkCnfg&) = delete;
		SettingsMidiClkCnfg& operator= (const SettingsMidiClkCnfg&) = delete;
		SettingsMidiClkCnfg (SettingsMidiClkCnfg&&) = delete;
		SettingsMidiClkCnfg& operator= (SettingsMidiClkCnfg&&) = delete;
		~SettingsMidiClkCnfg () = default;

		void init(void);

		void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam (moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		std::string getActiveSongName ();

		AnyType getParamSingle (SettingsMidiClkCnfgParam param);
		void informClkSrcToSongManager(void);
		void informClkOutPpqnChangeToSongManager(void);
		void informClkOutPortChangeToSongManager(void);

		bool isClkInFromDinMidiPort(void);
		bool isClkInFromUsbMidiPort(void);
		bool isClkInFromUsbDeviceMidiPort(void);
		bool isClkInFromUsbHostMidiPort(void);
		bool isClockInternal(void);
		bool isClkInFromSyncPort(void);
		bool isClkOutEnabledOnSyncPort(void);
		bool isClkOutEnabledOnDin1Port(void);
		bool isClkOutEnabledOnDin2Port(void);
		bool isClkOutEnabledOnDin3Port(void);
		bool isClkOutEnabledOnDin4Port(void);
		bool isClkOutEnabledOnDin5Port(void);
		bool isClkOutEnabledOnUsbD1Port(void);
		bool isClkOutEnabledOnUsbD2Port(void);
		bool isClkOutEnabledOnUsbD3Port(void);
		bool isClkOutEnabledOnUsbD4Port(void);
		bool isClkOutEnabledOnUsbD5Port(void);
		bool isClkOutEnabledOnUsbD6Port(void);
		bool isClkOutEnabledOnUsbH1Port(void);
		bool isClkOutEnabledOnUsbH2Port(void);
		bool isClkOutEnabledOnUsbH3Port(void);
		bool isClkOutEnabledOnUsbH4Port(void);
		bool isClkOutEnabledOnUsbH5Port(void);
		bool isClkOutEnabledOnUsbH6Port(void);



		SettingsMidiClkCnfgData _settingsMidiClkCnfgData;

	private:
		IgniteEngine& engine;
		IParamChangeListener* paramlistener;
		int8_t _midiConfigActivePort = -1;
		bool isUndilistNeedUpdate = true;
		void setDefaultUndoListData ();
		void SaveData();
	};
}
