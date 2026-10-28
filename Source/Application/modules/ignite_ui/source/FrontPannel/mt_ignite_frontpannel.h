#pragma once

using namespace Ignite;
namespace ui
{
	class FrountPannel : public juce::Component, public ApplicationCommandManager
	{
	public:
		FrountPannel();
		~FrountPannel();

		void EnginePublish(moduleId_t module, moduleId_t moduleSrc, paramId_t param, const AnyType& value, bool isResponse);
		void simulatorInput(FrontPannelParam param, uint8_t value);
		float getTapTempoValue() { return tapTempoValue; };

		static FrountPannel* GetFrountPannelInstance()
		{
			static FrountPannel* FrountPannelInstance;
			if (NULL == FrountPannelInstance)
			{
				FrountPannelInstance = new FrountPannel();
			}
			return FrountPannelInstance;
		}

		bool getShiftKeyStatus() {
			return isShiftKeyActive;
		};

		FrontPannelKeyStatus getDeleteKeyStatus() {
			return((FrontPannelKeyStatus)KeyStatus[(uint8_t)FrontPannelParam::paramKey_DELETE]);
		};

		FrontPannelKeyStatus getInsertKeyStatus() {
			return((FrontPannelKeyStatus)KeyStatus[(uint8_t)FrontPannelParam::paramKey_INSERT]);
		};

		FrontPannelKeyStatus getBarKeyStatus() {
			return((FrontPannelKeyStatus)KeyStatus[(uint8_t)FrontPannelParam::paramKey_ROTARYBAR]);
		};
		bool isMultiKeyPressed() const {
			return multiKeyPressed;
		};
		FrontPannelKeyStatus getKeyStatus(paramId_t param) const {
			if ((uint8_t)param < (uint8_t)FrontPannelParam::paramKey_End_KeyMapping)
				return((FrontPannelKeyStatus)KeyStatus[(uint8_t)param]);
			else
				return(FrontPannelKeyStatus::paramKeyStatusIdle);
		};
	private:
		bool isShiftKeyActive = false;
		CommonPopUp* popupPtr = nullptr;
		float tapTempoValue = 0.0f;
		CommonInfoWindow* _pInfoWindow = nullptr;
		bool HandleKeyIndication(AppMainmode* AppMainmodeRef, paramId_t param, uint8_t KeyValue);
		bool Handle_NavigationKey(AppMainmode* AppMainmodeRef, paramId_t param, uint8_t KeyValue);
		ApplicationCommandManager* myCommandManager;
		void handleCommandMessage(int cmdId);
		bool multiKeyPressed = false;
		uint8_t KeyStatus[(uint8_t)FrontPannelParam::paramKey_End_KeyMapping];
	};
}
