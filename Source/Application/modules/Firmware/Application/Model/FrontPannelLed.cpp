#include "Firmware/Application/Model/FrontPannelLed.h"

using namespace Ignite;

FrontPannelLed::FrontPannelLed(FrontPannelManager* FrontPanneMgr) : FrontPanneMgr(FrontPanneMgr),
FrontPannelLedExecuteThread("FrontPanneLed", 1600, osPriorityHigh)
{
	Init();
}

#define FRONT_PANEL_LED_MAX_WAIT 100 // Max Wait
#define FRONT_PANEL_LED_TIME_WAIT 15 // delay between each action/Led update
#define FRONT_PANEL_RGB_TIME_WAIT 15 // delay between each action
#define FRONT_PANEL_KNOB_LED_TIME_WAIT 15 // delay between each action
void FrontPannelLed::FrontPannelLedExecuteThreadFunction(void)
{
	int wait = 0;
	FrontPannelLedExecuteThread.delayMs(1000);
	while (!CThread::desktopShutdownRequested())
	{
		wait = FRONT_PANEL_LED_MAX_WAIT;
		PatternValueUpdate();
		if (execute())
		{
			wait = (wait - FRONT_PANEL_LED_TIME_WAIT);
			FrontPannelLedExecuteThread.delayMs(FRONT_PANEL_LED_TIME_WAIT);
		}
		if (FrontPanneMgr->isRgbLedDataUpdated())
		{
			wait = (wait - FRONT_PANEL_RGB_TIME_WAIT);
			FrontPannelLedExecuteThread.delayMs(FRONT_PANEL_RGB_TIME_WAIT);
			FrontPanneMgr->sendRGBLedData();
		}
		if (FrontPanneMgr->isKnobLedDataUpdated())
		{
			wait = (wait - FRONT_PANEL_KNOB_LED_TIME_WAIT);
			FrontPannelLedExecuteThread.delayMs(FRONT_PANEL_KNOB_LED_TIME_WAIT);
			FrontPanneMgr->sendKnobLedData();
		}
		FrontPannelLedExecuteThread.delayMs(wait);
	}
}

bool FrontPannelLed::Init()
{
	for (uint8_t i = 0; i < LED_MAX_NUM; i++) // init LEDS :LED_MAX_NUM ? // TODO remove if not required
	{
		LedData[i].Com = 0;
		LedData[i].Row = 0;
		LedData[i].incCom = false;
		LedData[i].dspId = DISP_CHIP_ID_1;
		LedData[i].value = 0;
		LedData[i].count = 0;
		LedData[i].pattern = 0;
		LedData[i].tCount = 0;
	}
	//Total count = 120 LEDs Single Color, Chip ID  1 & 2;
	// Count will indicate LEDs under same ID
	LedData[EN_LED_ALL].Com = 0;
	LedData[EN_LED_ALL].Row = 0;
	LedData[EN_LED_ALL].incCom = false;
	LedData[EN_LED_ALL].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_ALL].value = 0;
	LedData[EN_LED_ALL].count = 0;
	LedData[EN_LED_ALL].pattern = 0;
	LedData[EN_LED_ALL].tCount = 0;
	LedData[EN_LED_ALL].ledParamKeyId = 0;

	LedData[EN_LED_USB_STOP].Com = 0;
	LedData[EN_LED_USB_STOP].Row = 0;
	LedData[EN_LED_USB_STOP].incCom = true;
	LedData[EN_LED_USB_STOP].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_USB_STOP].value = 0;
	LedData[EN_LED_USB_STOP].count = 2;
	LedData[EN_LED_USB_STOP].pattern = 0;
	LedData[EN_LED_USB_STOP].tCount = 0;
	LedData[EN_LED_USB_STOP].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_USB_STOP;

	LedData[EN_LED_MENU].Com = 0;
	LedData[EN_LED_MENU].Row = 4;
	LedData[EN_LED_MENU].incCom = false;
	LedData[EN_LED_MENU].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_MENU].value = 0;
	LedData[EN_LED_MENU].count = 0;
	LedData[EN_LED_MENU].pattern = 0;
	LedData[EN_LED_MENU].tCount = 0;
	LedData[EN_LED_MENU].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_MENU;

	LedData[EN_LED_BRAY].Com = 0;
	LedData[EN_LED_BRAY].Row = 6;
	LedData[EN_LED_BRAY].incCom = false;
	LedData[EN_LED_BRAY].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_BRAY].value = 0;
	LedData[EN_LED_BRAY].count = 0;
	LedData[EN_LED_BRAY].pattern = 0;
	LedData[EN_LED_BRAY].tCount = 0;
	LedData[EN_LED_BRAY].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_BRAY;

	LedData[EN_LED_RETURN_NAV].Com = 0;
	LedData[EN_LED_RETURN_NAV].Row = 8;
	LedData[EN_LED_RETURN_NAV].incCom = false;
	LedData[EN_LED_RETURN_NAV].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_RETURN_NAV].value = 0;
	LedData[EN_LED_RETURN_NAV].count = 0;
	LedData[EN_LED_RETURN_NAV].pattern = 0;
	LedData[EN_LED_RETURN_NAV].tCount = 0;
	LedData[EN_LED_RETURN_NAV].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_RETURN_NAV;

	LedData[EN_LED_COPY_NAV].Com = 0;
	LedData[EN_LED_COPY_NAV].Row = 10;
	LedData[EN_LED_COPY_NAV].incCom = false;
	LedData[EN_LED_COPY_NAV].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_COPY_NAV].value = 0;
	LedData[EN_LED_COPY_NAV].count = 0;
	LedData[EN_LED_COPY_NAV].pattern = 0;
	LedData[EN_LED_COPY_NAV].tCount = 0;
	LedData[EN_LED_COPY_NAV].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_COPY_NAV;

	LedData[EN_LED_MINUS_NAV].Com = 0;
	LedData[EN_LED_MINUS_NAV].Row = 12;
	LedData[EN_LED_MINUS_NAV].incCom = false;
	LedData[EN_LED_MINUS_NAV].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_MINUS_NAV].value = 0;
	LedData[EN_LED_MINUS_NAV].count = 0;
	LedData[EN_LED_MINUS_NAV].pattern = 0;
	LedData[EN_LED_MINUS_NAV].tCount = 0;
	LedData[EN_LED_MINUS_NAV].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_MINUS_NAV;

	LedData[EN_LED_PLUS_NAV].Com = 0;
	LedData[EN_LED_PLUS_NAV].Row = 14;
	LedData[EN_LED_PLUS_NAV].incCom = false;
	LedData[EN_LED_PLUS_NAV].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_PLUS_NAV].value = 0;
	LedData[EN_LED_PLUS_NAV].count = 0;
	LedData[EN_LED_PLUS_NAV].pattern = 0;
	LedData[EN_LED_PLUS_NAV].tCount = 0;
	LedData[EN_LED_PLUS_NAV].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_PLUS_NAV;

	LedData[EN_LED_ROTARYROW].Com = 1;
	LedData[EN_LED_ROTARYROW].Row = 0;
	LedData[EN_LED_ROTARYROW].incCom = true;
	LedData[EN_LED_ROTARYROW].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_ROTARYROW].value = 0;
	LedData[EN_LED_ROTARYROW].count = 4;
	LedData[EN_LED_ROTARYROW].pattern = 0;
	LedData[EN_LED_ROTARYROW].tCount = 0;
	LedData[EN_LED_ROTARYROW].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_ROTARYROW;

	LedData[EN_LED_REC].Com = 2;
	LedData[EN_LED_REC].Row = 0;
	LedData[EN_LED_REC].incCom = false;
	LedData[EN_LED_REC].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_REC].value = 0;
	LedData[EN_LED_REC].count = 0;
	LedData[EN_LED_REC].pattern = 0;
	LedData[EN_LED_REC].tCount = 0;
	LedData[EN_LED_REC].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_REC;

	LedData[EN_LED_BACKFORWARD].Com = 2;
	LedData[EN_LED_BACKFORWARD].Row = 2;
	LedData[EN_LED_BACKFORWARD].incCom = false;
	LedData[EN_LED_BACKFORWARD].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_BACKFORWARD].value = 0;
	LedData[EN_LED_BACKFORWARD].count = 0;
	LedData[EN_LED_BACKFORWARD].pattern = 0;
	LedData[EN_LED_BACKFORWARD].tCount = 0;
	LedData[EN_LED_BACKFORWARD].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_BACKFORWARD;

	LedData[EN_LED_STOP].Com = 2;
	LedData[EN_LED_STOP].Row = 4;
	LedData[EN_LED_STOP].incCom = false;
	LedData[EN_LED_STOP].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_STOP].value = 0;
	LedData[EN_LED_STOP].count = 0;
	LedData[EN_LED_STOP].pattern = 0;
	LedData[EN_LED_STOP].tCount = 0;
	LedData[EN_LED_STOP].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_STOP;

	LedData[EN_LED_PLAY].Com = 2;
	LedData[EN_LED_PLAY].Row = 6;
	LedData[EN_LED_PLAY].incCom = false;
	LedData[EN_LED_PLAY].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_PLAY].value = 0;
	LedData[EN_LED_PLAY].count = 0;
	LedData[EN_LED_PLAY].pattern = 0;
	LedData[EN_LED_PLAY].tCount = 0;
	LedData[EN_LED_PLAY].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_PLAY;

	LedData[EN_LED_NEXT_TIE].Com = 2;
	LedData[EN_LED_NEXT_TIE].Row = 8;
	LedData[EN_LED_NEXT_TIE].incCom = false;
	LedData[EN_LED_NEXT_TIE].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_NEXT_TIE].value = 0;
	LedData[EN_LED_NEXT_TIE].count = 0;
	LedData[EN_LED_NEXT_TIE].pattern = 0;
	LedData[EN_LED_NEXT_TIE].tCount = 0;
	LedData[EN_LED_NEXT_TIE].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_NEXT_TIE;

	LedData[EN_LED_FILL_X].Com = 2;
	LedData[EN_LED_FILL_X].Row = 10;
	LedData[EN_LED_FILL_X].incCom = false;
	LedData[EN_LED_FILL_X].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_FILL_X].value = 0;
	LedData[EN_LED_FILL_X].count = 0;
	LedData[EN_LED_FILL_X].pattern = 0;
	LedData[EN_LED_FILL_X].tCount = 0;
	LedData[EN_LED_FILL_X].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_FILL_X;

	LedData[EN_LED_SOLO_SKIP].Com = 2;
	LedData[EN_LED_SOLO_SKIP].Row = 12;
	LedData[EN_LED_SOLO_SKIP].incCom = false;
	LedData[EN_LED_SOLO_SKIP].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_SOLO_SKIP].value = 0;
	LedData[EN_LED_SOLO_SKIP].count = 0;
	LedData[EN_LED_SOLO_SKIP].pattern = 0;
	LedData[EN_LED_SOLO_SKIP].tCount = 0;
	LedData[EN_LED_SOLO_SKIP].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_SOLO_SKIP;

	LedData[EN_LED_GANG].Com = 2;
	LedData[EN_LED_GANG].Row = 14;
	LedData[EN_LED_GANG].incCom = false;
	LedData[EN_LED_GANG].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_GANG].value = 0;
	LedData[EN_LED_GANG].count = 0;
	LedData[EN_LED_GANG].pattern = 0;
	LedData[EN_LED_GANG].tCount = 0;
	LedData[EN_LED_GANG].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_GANG;

	LedData[EN_LED_SAVE].Com = 3;
	LedData[EN_LED_SAVE].Row = 0;
	LedData[EN_LED_SAVE].incCom = false;
	LedData[EN_LED_SAVE].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_SAVE].value = 0;
	LedData[EN_LED_SAVE].count = 0;
	LedData[EN_LED_SAVE].pattern = 0;
	LedData[EN_LED_SAVE].tCount = 0;
	LedData[EN_LED_SAVE].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_SAVE;

	LedData[EN_LED_LAST].Com = 3;
	LedData[EN_LED_LAST].Row = 2;
	LedData[EN_LED_LAST].incCom = false;
	LedData[EN_LED_LAST].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_LAST].value = 0;
	LedData[EN_LED_LAST].count = 0;
	LedData[EN_LED_LAST].pattern = 0;
	LedData[EN_LED_LAST].tCount = 0;
	LedData[EN_LED_LAST].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_LAST;

	LedData[EN_LED_SHIFT].Com = 3;
	LedData[EN_LED_SHIFT].Row = 4;
	LedData[EN_LED_SHIFT].incCom = false;
	LedData[EN_LED_SHIFT].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_SHIFT].value = 0;
	LedData[EN_LED_SHIFT].count = 0;
	LedData[EN_LED_SHIFT].pattern = 0;
	LedData[EN_LED_SHIFT].tCount = 0;
	LedData[EN_LED_SHIFT].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_SHIFT;

	LedData[EN_LED_SONG].Com = 3;
	LedData[EN_LED_SONG].Row = 6;
	LedData[EN_LED_SONG].incCom = false;
	LedData[EN_LED_SONG].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_SONG].value = 0;
	LedData[EN_LED_SONG].count = 0;
	LedData[EN_LED_SONG].pattern = 0;
	LedData[EN_LED_SONG].tCount = 0;
	LedData[EN_LED_SONG].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_SONG;

	LedData[EN_LED_SCENE].Com = 3;
	LedData[EN_LED_SCENE].Row = 8;
	LedData[EN_LED_SCENE].incCom = false;
	LedData[EN_LED_SCENE].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_SCENE].value = 0;
	LedData[EN_LED_SCENE].count = 0;
	LedData[EN_LED_SCENE].pattern = 0;
	LedData[EN_LED_SCENE].tCount = 0;
	LedData[EN_LED_SCENE].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_SCENE;

	LedData[EN_LED_TRACK].Com = 3;
	LedData[EN_LED_TRACK].Row = 10;
	LedData[EN_LED_TRACK].incCom = false;
	LedData[EN_LED_TRACK].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACK].value = 0;
	LedData[EN_LED_TRACK].count = 0;
	LedData[EN_LED_TRACK].pattern = 0;
	LedData[EN_LED_TRACK].tCount = 0;
	LedData[EN_LED_TRACK].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACK;

	LedData[EN_LED_SCULPT].Com = 3;
	LedData[EN_LED_SCULPT].Row = 12;
	LedData[EN_LED_SCULPT].incCom = false;
	LedData[EN_LED_SCULPT].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_SCULPT].value = 0;
	LedData[EN_LED_SCULPT].count = 0;
	LedData[EN_LED_SCULPT].pattern = 0;
	LedData[EN_LED_SCULPT].tCount = 0;
	LedData[EN_LED_SCULPT].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_SCULPT;

	LedData[EN_LED_DELETE].Com = 3;
	LedData[EN_LED_DELETE].Row = 14;
	LedData[EN_LED_DELETE].incCom = false;
	LedData[EN_LED_DELETE].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_DELETE].value = 0;
	LedData[EN_LED_DELETE].count = 0;
	LedData[EN_LED_DELETE].pattern = 0;
	LedData[EN_LED_DELETE].tCount = 0;
	LedData[EN_LED_DELETE].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_DELETE;

	LedData[EN_LED_INSERT].Com = 4;
	LedData[EN_LED_INSERT].Row = 0;
	LedData[EN_LED_INSERT].incCom = false;
	LedData[EN_LED_INSERT].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_INSERT].value = 0;
	LedData[EN_LED_INSERT].count = 0;
	LedData[EN_LED_INSERT].pattern = 0;
	LedData[EN_LED_INSERT].tCount = 0;
	LedData[EN_LED_INSERT].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_INSERT;

	LedData[EN_LED_PATTERN].Com = 4;
	LedData[EN_LED_PATTERN].Row = 2;
	LedData[EN_LED_PATTERN].incCom = false;
	LedData[EN_LED_PATTERN].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_PATTERN].value = 0;
	LedData[EN_LED_PATTERN].count = 0;
	LedData[EN_LED_PATTERN].pattern = 0;
	LedData[EN_LED_PATTERN].tCount = 0;
	LedData[EN_LED_PATTERN].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_PATTERN;

	LedData[EN_LED_TEMPO].Com = 4;
	LedData[EN_LED_TEMPO].Row = 4;
	LedData[EN_LED_TEMPO].incCom = false;
	LedData[EN_LED_TEMPO].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TEMPO].value = 0;
	LedData[EN_LED_TEMPO].count = 0;
	LedData[EN_LED_TEMPO].pattern = 0;
	LedData[EN_LED_TEMPO].tCount = 0;
	LedData[EN_LED_TEMPO].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TEMPO;

	LedData[EN_LED_INTERNALSOUNDS].Com = 4;
	LedData[EN_LED_INTERNALSOUNDS].Row = 6;
	LedData[EN_LED_INTERNALSOUNDS].incCom = false;
	LedData[EN_LED_INTERNALSOUNDS].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_INTERNALSOUNDS].value = 0;
	LedData[EN_LED_INTERNALSOUNDS].count = 0;
	LedData[EN_LED_INTERNALSOUNDS].pattern = 0;
	LedData[EN_LED_INTERNALSOUNDS].tCount = 0;
	LedData[EN_LED_INTERNALSOUNDS].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_INTERNALSOUNDS;

	LedData[EN_LED_ROTARYBAR].Com = 4;
	LedData[EN_LED_ROTARYBAR].Row = 8;
	LedData[EN_LED_ROTARYBAR].incCom = true;
	LedData[EN_LED_ROTARYBAR].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_ROTARYBAR].value = 0;
	LedData[EN_LED_ROTARYBAR].count = 3;
	LedData[EN_LED_ROTARYBAR].pattern = 0;
	LedData[EN_LED_ROTARYBAR].tCount = 0;
	LedData[EN_LED_ROTARYBAR].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_ROTARYBAR;

	LedData[EN_LED_TRACKROTARY1].Com = 5;
	LedData[EN_LED_TRACKROTARY1].Row = 0;
	LedData[EN_LED_TRACKROTARY1].incCom = true;
	LedData[EN_LED_TRACKROTARY1].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY1].value = 0;
	LedData[EN_LED_TRACKROTARY1].count = 3;
	LedData[EN_LED_TRACKROTARY1].pattern = 0;
	LedData[EN_LED_TRACKROTARY1].tCount = 0;
	LedData[EN_LED_TRACKROTARY1].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY1;

	LedData[EN_LED_TRACKROTARY2].Com = 5;
	LedData[EN_LED_TRACKROTARY2].Row = 8;
	LedData[EN_LED_TRACKROTARY2].incCom = true;
	LedData[EN_LED_TRACKROTARY2].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY2].value = 0;
	LedData[EN_LED_TRACKROTARY2].count = 3;
	LedData[EN_LED_TRACKROTARY2].pattern = 0;
	LedData[EN_LED_TRACKROTARY2].tCount = 0;
	LedData[EN_LED_TRACKROTARY2].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY2;

	LedData[EN_LED_TRACKROTARY3].Com = 6;
	LedData[EN_LED_TRACKROTARY3].Row = 0;
	LedData[EN_LED_TRACKROTARY3].incCom = true;
	LedData[EN_LED_TRACKROTARY3].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY3].value = 0;
	LedData[EN_LED_TRACKROTARY3].count = 3;
	LedData[EN_LED_TRACKROTARY3].pattern = 0;
	LedData[EN_LED_TRACKROTARY3].tCount = 0;
	LedData[EN_LED_TRACKROTARY3].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY3;

	LedData[EN_LED_TRACKROTARY4].Com = 6;
	LedData[EN_LED_TRACKROTARY4].Row = 8;
	LedData[EN_LED_TRACKROTARY4].incCom = true;
	LedData[EN_LED_TRACKROTARY4].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY4].value = 0;
	LedData[EN_LED_TRACKROTARY4].count = 3;
	LedData[EN_LED_TRACKROTARY4].pattern = 0;
	LedData[EN_LED_TRACKROTARY4].tCount = 0;
	LedData[EN_LED_TRACKROTARY4].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY4;

	LedData[EN_LED_TRACKROTARY5].Com = 7;
	LedData[EN_LED_TRACKROTARY5].Row = 0;
	LedData[EN_LED_TRACKROTARY5].incCom = true;
	LedData[EN_LED_TRACKROTARY5].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY5].value = 0;
	LedData[EN_LED_TRACKROTARY5].count = 3;
	LedData[EN_LED_TRACKROTARY5].pattern = 0;
	LedData[EN_LED_TRACKROTARY5].tCount = 0;
	LedData[EN_LED_TRACKROTARY5].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY5;

	LedData[EN_LED_TRACKROTARY6].Com = 7;
	LedData[EN_LED_TRACKROTARY6].Row = 8;
	LedData[EN_LED_TRACKROTARY6].incCom = true;
	LedData[EN_LED_TRACKROTARY6].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY6].value = 0;
	LedData[EN_LED_TRACKROTARY6].count = 3;
	LedData[EN_LED_TRACKROTARY6].pattern = 0;
	LedData[EN_LED_TRACKROTARY6].tCount = 0;
	LedData[EN_LED_TRACKROTARY6].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY6;

	LedData[EN_LED_TRACKROTARY7].Com = 0;
	LedData[EN_LED_TRACKROTARY7].Row = 1;
	LedData[EN_LED_TRACKROTARY7].incCom = true;
	LedData[EN_LED_TRACKROTARY7].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY7].value = 0;
	LedData[EN_LED_TRACKROTARY7].count = 3;
	LedData[EN_LED_TRACKROTARY7].pattern = 0;
	LedData[EN_LED_TRACKROTARY7].tCount = 0;
	LedData[EN_LED_TRACKROTARY7].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY7;

	LedData[EN_LED_TRACKROTARY8].Com = 0;
	LedData[EN_LED_TRACKROTARY8].Row = 9;
	LedData[EN_LED_TRACKROTARY8].incCom = true;
	LedData[EN_LED_TRACKROTARY8].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY8].value = 0;
	LedData[EN_LED_TRACKROTARY8].count = 3;
	LedData[EN_LED_TRACKROTARY8].pattern = 0;
	LedData[EN_LED_TRACKROTARY8].tCount = 0;
	LedData[EN_LED_TRACKROTARY8].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY8;

	LedData[EN_LED_TRACKROTARY9].Com = 1;
	LedData[EN_LED_TRACKROTARY9].Row = 1;
	LedData[EN_LED_TRACKROTARY9].incCom = true;
	LedData[EN_LED_TRACKROTARY9].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY9].value = 0;
	LedData[EN_LED_TRACKROTARY9].count = 3;
	LedData[EN_LED_TRACKROTARY9].pattern = 0;
	LedData[EN_LED_TRACKROTARY9].tCount = 0;
	LedData[EN_LED_TRACKROTARY9].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY9;

	LedData[EN_LED_TRACKROTARY10].Com = 1;
	LedData[EN_LED_TRACKROTARY10].Row = 9;
	LedData[EN_LED_TRACKROTARY10].incCom = true;
	LedData[EN_LED_TRACKROTARY10].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY10].value = 0;
	LedData[EN_LED_TRACKROTARY10].count = 3;
	LedData[EN_LED_TRACKROTARY10].pattern = 0;
	LedData[EN_LED_TRACKROTARY10].tCount = 0;
	LedData[EN_LED_TRACKROTARY10].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY10;

	LedData[EN_LED_TRACKROTARY11].Com = 2;
	LedData[EN_LED_TRACKROTARY11].Row = 1;
	LedData[EN_LED_TRACKROTARY11].incCom = true;
	LedData[EN_LED_TRACKROTARY11].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY11].value = 0;
	LedData[EN_LED_TRACKROTARY11].count = 3;
	LedData[EN_LED_TRACKROTARY11].pattern = 0;
	LedData[EN_LED_TRACKROTARY11].tCount = 0;
	LedData[EN_LED_TRACKROTARY11].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY11;

	LedData[EN_LED_TRACKROTARY12].Com = 2;
	LedData[EN_LED_TRACKROTARY12].Row = 9;
	LedData[EN_LED_TRACKROTARY12].incCom = true;
	LedData[EN_LED_TRACKROTARY12].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY12].value = 0;
	LedData[EN_LED_TRACKROTARY12].count = 3;
	LedData[EN_LED_TRACKROTARY12].pattern = 0;
	LedData[EN_LED_TRACKROTARY12].tCount = 0;
	LedData[EN_LED_TRACKROTARY12].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY12;

	LedData[EN_LED_TRACKROTARY13].Com = 3;
	LedData[EN_LED_TRACKROTARY13].Row = 1;
	LedData[EN_LED_TRACKROTARY13].incCom = true;
	LedData[EN_LED_TRACKROTARY13].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY13].value = 0;
	LedData[EN_LED_TRACKROTARY13].count = 3;
	LedData[EN_LED_TRACKROTARY13].pattern = 0;
	LedData[EN_LED_TRACKROTARY13].tCount = 0;
	LedData[EN_LED_TRACKROTARY13].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY13;

	LedData[EN_LED_TRACKROTARY14].Com = 3;
	LedData[EN_LED_TRACKROTARY14].Row = 9;
	LedData[EN_LED_TRACKROTARY14].incCom = true;
	LedData[EN_LED_TRACKROTARY14].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY14].value = 0;
	LedData[EN_LED_TRACKROTARY14].count = 4;
	LedData[EN_LED_TRACKROTARY14].pattern = 0;
	LedData[EN_LED_TRACKROTARY14].tCount = 0;
	LedData[EN_LED_TRACKROTARY14].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY14;

	LedData[EN_LED_TRACKROTARY15].Com = 4;
	LedData[EN_LED_TRACKROTARY15].Row = 1;
	LedData[EN_LED_TRACKROTARY15].incCom = true;
	LedData[EN_LED_TRACKROTARY15].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY15].value = 0;
	LedData[EN_LED_TRACKROTARY15].count = 3;
	LedData[EN_LED_TRACKROTARY15].pattern = 0;
	LedData[EN_LED_TRACKROTARY15].tCount = 0;
	LedData[EN_LED_TRACKROTARY15].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY15;

	LedData[EN_LED_TRACKROTARY16].Com = 4;
	LedData[EN_LED_TRACKROTARY16].Row = 9;
	LedData[EN_LED_TRACKROTARY16].incCom = true;
	LedData[EN_LED_TRACKROTARY16].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKROTARY16].value = 0;
	LedData[EN_LED_TRACKROTARY16].count = 3;
	LedData[EN_LED_TRACKROTARY16].pattern = 0;
	LedData[EN_LED_TRACKROTARY16].tCount = 0;
	LedData[EN_LED_TRACKROTARY16].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKROTARY16;

	LedData[EN_LED_TRACKSEL1].Com = 5;
	LedData[EN_LED_TRACKSEL1].Row = 1;
	LedData[EN_LED_TRACKSEL1].incCom = false;
	LedData[EN_LED_TRACKSEL1].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL1].value = 0;
	LedData[EN_LED_TRACKSEL1].count = 0;
	LedData[EN_LED_TRACKSEL1].pattern = 0;
	LedData[EN_LED_TRACKSEL1].tCount = 0;
	LedData[EN_LED_TRACKSEL1].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL1;

	LedData[EN_LED_TRACKSEL2].Com = 5;
	LedData[EN_LED_TRACKSEL2].Row = 3;
	LedData[EN_LED_TRACKSEL2].incCom = false;
	LedData[EN_LED_TRACKSEL2].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL2].value = 0;
	LedData[EN_LED_TRACKSEL2].count = 0;
	LedData[EN_LED_TRACKSEL2].pattern = 0;
	LedData[EN_LED_TRACKSEL2].tCount = 0;
	LedData[EN_LED_TRACKSEL2].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL2;

	LedData[EN_LED_TRACKSEL3].Com = 5;
	LedData[EN_LED_TRACKSEL3].Row = 5;
	LedData[EN_LED_TRACKSEL3].incCom = false;
	LedData[EN_LED_TRACKSEL3].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL3].value = 0;
	LedData[EN_LED_TRACKSEL3].count = 0;
	LedData[EN_LED_TRACKSEL3].pattern = 0;
	LedData[EN_LED_TRACKSEL3].tCount = 0;
	LedData[EN_LED_TRACKSEL3].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL3;

	LedData[EN_LED_TRACKSEL4].Com = 5;
	LedData[EN_LED_TRACKSEL4].Row = 7;
	LedData[EN_LED_TRACKSEL4].incCom = false;
	LedData[EN_LED_TRACKSEL4].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL4].value = 0;
	LedData[EN_LED_TRACKSEL4].count = 0;
	LedData[EN_LED_TRACKSEL4].pattern = 0;
	LedData[EN_LED_TRACKSEL4].tCount = 0;
	LedData[EN_LED_TRACKSEL4].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL4;

	LedData[EN_LED_TRACKSEL5].Com = 5;
	LedData[EN_LED_TRACKSEL5].Row = 9;
	LedData[EN_LED_TRACKSEL5].incCom = false;
	LedData[EN_LED_TRACKSEL5].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL5].value = 0;
	LedData[EN_LED_TRACKSEL5].count = 0;
	LedData[EN_LED_TRACKSEL5].pattern = 0;
	LedData[EN_LED_TRACKSEL5].tCount = 0;
	LedData[EN_LED_TRACKSEL5].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL5;

	LedData[EN_LED_TRACKSEL6].Com = 5;
	LedData[EN_LED_TRACKSEL6].Row = 11;
	LedData[EN_LED_TRACKSEL6].incCom = false;
	LedData[EN_LED_TRACKSEL6].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL6].value = 0;
	LedData[EN_LED_TRACKSEL6].count = 0;
	LedData[EN_LED_TRACKSEL6].pattern = 0;
	LedData[EN_LED_TRACKSEL6].tCount = 0;
	LedData[EN_LED_TRACKSEL6].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL6;

	LedData[EN_LED_TRACKSEL7].Com = 5;
	LedData[EN_LED_TRACKSEL7].Row = 13;
	LedData[EN_LED_TRACKSEL7].incCom = false;
	LedData[EN_LED_TRACKSEL7].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL7].value = 0;
	LedData[EN_LED_TRACKSEL7].count = 0;
	LedData[EN_LED_TRACKSEL7].pattern = 0;
	LedData[EN_LED_TRACKSEL7].tCount = 0;
	LedData[EN_LED_TRACKSEL7].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL7;

	LedData[EN_LED_TRACKSEL8].Com = 5;
	LedData[EN_LED_TRACKSEL8].Row = 15;
	LedData[EN_LED_TRACKSEL8].incCom = false;
	LedData[EN_LED_TRACKSEL8].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL8].value = 0;
	LedData[EN_LED_TRACKSEL8].count = 0;
	LedData[EN_LED_TRACKSEL8].pattern = 0;
	LedData[EN_LED_TRACKSEL8].tCount = 0;
	LedData[EN_LED_TRACKSEL8].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL8;

	LedData[EN_LED_TRACKSEL9].Com = 6;
	LedData[EN_LED_TRACKSEL9].Row = 1;
	LedData[EN_LED_TRACKSEL9].incCom = false;
	LedData[EN_LED_TRACKSEL9].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL9].value = 0;
	LedData[EN_LED_TRACKSEL9].count = 0;
	LedData[EN_LED_TRACKSEL9].pattern = 0;
	LedData[EN_LED_TRACKSEL9].tCount = 0;
	LedData[EN_LED_TRACKSEL9].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL9;

	LedData[EN_LED_TRACKSEL10].Com = 6;
	LedData[EN_LED_TRACKSEL10].Row = 3;
	LedData[EN_LED_TRACKSEL10].incCom = false;
	LedData[EN_LED_TRACKSEL10].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL10].value = 0;
	LedData[EN_LED_TRACKSEL10].count = 0;
	LedData[EN_LED_TRACKSEL10].pattern = 0;
	LedData[EN_LED_TRACKSEL10].tCount = 0;
	LedData[EN_LED_TRACKSEL10].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL10;

	LedData[EN_LED_TRACKSEL11].Com = 6;
	LedData[EN_LED_TRACKSEL11].Row = 5;
	LedData[EN_LED_TRACKSEL11].incCom = false;
	LedData[EN_LED_TRACKSEL11].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL11].value = 0;
	LedData[EN_LED_TRACKSEL11].count = 0;
	LedData[EN_LED_TRACKSEL11].pattern = 0;
	LedData[EN_LED_TRACKSEL11].tCount = 0;
	LedData[EN_LED_TRACKSEL11].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL11;

	LedData[EN_LED_TRACKSEL12].Com = 6;
	LedData[EN_LED_TRACKSEL12].Row = 7;
	LedData[EN_LED_TRACKSEL12].incCom = false;
	LedData[EN_LED_TRACKSEL12].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL12].value = 0;
	LedData[EN_LED_TRACKSEL12].count = 0;
	LedData[EN_LED_TRACKSEL12].pattern = 0;
	LedData[EN_LED_TRACKSEL12].tCount = 0;
	LedData[EN_LED_TRACKSEL12].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL12;

	LedData[EN_LED_TRACKSEL13].Com = 6;
	LedData[EN_LED_TRACKSEL13].Row = 9;
	LedData[EN_LED_TRACKSEL13].incCom = false;
	LedData[EN_LED_TRACKSEL13].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL13].value = 0;
	LedData[EN_LED_TRACKSEL13].count = 0;
	LedData[EN_LED_TRACKSEL13].pattern = 0;
	LedData[EN_LED_TRACKSEL13].tCount = 0;
	LedData[EN_LED_TRACKSEL13].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL13;

	LedData[EN_LED_TRACKSEL14].Com = 6;
	LedData[EN_LED_TRACKSEL14].Row = 11;
	LedData[EN_LED_TRACKSEL14].incCom = false;
	LedData[EN_LED_TRACKSEL14].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL14].value = 0;
	LedData[EN_LED_TRACKSEL14].count = 0;
	LedData[EN_LED_TRACKSEL14].pattern = 0;
	LedData[EN_LED_TRACKSEL14].tCount = 0;
	LedData[EN_LED_TRACKSEL14].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL14;

	LedData[EN_LED_TRACKSEL15].Com = 6;
	LedData[EN_LED_TRACKSEL15].Row = 13;
	LedData[EN_LED_TRACKSEL15].incCom = false;
	LedData[EN_LED_TRACKSEL15].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL15].value = 0;
	LedData[EN_LED_TRACKSEL15].count = 0;
	LedData[EN_LED_TRACKSEL15].pattern = 0;
	LedData[EN_LED_TRACKSEL15].tCount = 0;
	LedData[EN_LED_TRACKSEL15].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL15;

	LedData[EN_LED_TRACKSEL16].Com = 6;
	LedData[EN_LED_TRACKSEL16].Row = 15;
	LedData[EN_LED_TRACKSEL16].incCom = false;
	LedData[EN_LED_TRACKSEL16].dspId = DISP_CHIP_ID_1;
	LedData[EN_LED_TRACKSEL16].value = 0;
	LedData[EN_LED_TRACKSEL16].count = 0;
	LedData[EN_LED_TRACKSEL16].pattern = 0;
	LedData[EN_LED_TRACKSEL16].tCount = 0;
	LedData[EN_LED_TRACKSEL16].ledParamKeyId  =(uint8_t)FrontPannelParam::paramKey_TRACKSEL16;

	LedData[EN_LED_ROTARY_RELEASE].Com = 0;
	LedData[EN_LED_ROTARY_RELEASE].Row = 0;
	LedData[EN_LED_ROTARY_RELEASE].incCom = false;
	LedData[EN_LED_ROTARY_RELEASE].dspId = DISP_CHIP_ID_2;
	LedData[EN_LED_ROTARY_RELEASE].value = 0;
	LedData[EN_LED_ROTARY_RELEASE].count = 15;
	LedData[EN_LED_ROTARY_RELEASE].pattern = 0;
	LedData[EN_LED_ROTARY_RELEASE].tCount = 0;
	LedData[EN_LED_ROTARY_RELEASE].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_RELEASE;

	LedData[EN_LED_ROTARY_FILTER].Com = 0;
	LedData[EN_LED_ROTARY_FILTER].Row = 1;
	LedData[EN_LED_ROTARY_FILTER].incCom = false;
	LedData[EN_LED_ROTARY_FILTER].dspId = DISP_CHIP_ID_2;
	LedData[EN_LED_ROTARY_FILTER].value = 0;
	LedData[EN_LED_ROTARY_FILTER].count = 15;
	LedData[EN_LED_ROTARY_FILTER].pattern = 0;
	LedData[EN_LED_ROTARY_FILTER].tCount = 0;
	LedData[EN_LED_ROTARY_FILTER].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_FILTER;

	LedData[EN_LED_ROTARY_RESONANCE].Com = 0;
	LedData[EN_LED_ROTARY_RESONANCE].Row = 2;
	LedData[EN_LED_ROTARY_RESONANCE].incCom = false;
	LedData[EN_LED_ROTARY_RESONANCE].dspId = DISP_CHIP_ID_2;
	LedData[EN_LED_ROTARY_RESONANCE].value = 0;
	LedData[EN_LED_ROTARY_RESONANCE].count = 15;
	LedData[EN_LED_ROTARY_RESONANCE].pattern = 0;
	LedData[EN_LED_ROTARY_RESONANCE].tCount = 0;
	LedData[EN_LED_ROTARY_RESONANCE].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_RESONANCE;

	LedData[EN_LED_ROTARY_VOLUME].Com = 0;
	LedData[EN_LED_ROTARY_VOLUME].Row = 3;
	LedData[EN_LED_ROTARY_VOLUME].incCom = false;
	LedData[EN_LED_ROTARY_VOLUME].dspId = DISP_CHIP_ID_2;
	LedData[EN_LED_ROTARY_VOLUME].value = 0;
	LedData[EN_LED_ROTARY_VOLUME].count = 15;
	LedData[EN_LED_ROTARY_VOLUME].pattern = 0;
	LedData[EN_LED_ROTARY_VOLUME].tCount = 0;
	LedData[EN_LED_ROTARY_VOLUME].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_VOLUME;

	LedData[EN_LED_ROTARY_CHORUS_AMT].Com = 0;
	LedData[EN_LED_ROTARY_CHORUS_AMT].Row = 4;
	LedData[EN_LED_ROTARY_CHORUS_AMT].incCom = false;
	LedData[EN_LED_ROTARY_CHORUS_AMT].dspId = DISP_CHIP_ID_2;
	LedData[EN_LED_ROTARY_CHORUS_AMT].value = 0;
	LedData[EN_LED_ROTARY_CHORUS_AMT].count = 15;
	LedData[EN_LED_ROTARY_CHORUS_AMT].pattern = 0;
	LedData[EN_LED_ROTARY_CHORUS_AMT].tCount = 0;
	LedData[EN_LED_ROTARY_CHORUS_AMT].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_REVERBE;

	LedData[EN_LED_ROTARY_REVERB_AMT].Com = 0;
	LedData[EN_LED_ROTARY_REVERB_AMT].Row = 5;
	LedData[EN_LED_ROTARY_REVERB_AMT].incCom = false;
	LedData[EN_LED_ROTARY_REVERB_AMT].dspId = DISP_CHIP_ID_2;
	LedData[EN_LED_ROTARY_REVERB_AMT].value = 0;
	LedData[EN_LED_ROTARY_REVERB_AMT].count = 15;
	LedData[EN_LED_ROTARY_REVERB_AMT].pattern = 0;
	LedData[EN_LED_ROTARY_REVERB_AMT].tCount = 0;
	LedData[EN_LED_ROTARY_REVERB_AMT].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_CHORUS;

	LedData[EN_LED_ROTARY_EQ_HIGH].Com = 0;
	LedData[EN_LED_ROTARY_EQ_HIGH].Row = 6;
	LedData[EN_LED_ROTARY_EQ_HIGH].incCom = false;
	LedData[EN_LED_ROTARY_EQ_HIGH].dspId = DISP_CHIP_ID_2;
	LedData[EN_LED_ROTARY_EQ_HIGH].value = 0;
	LedData[EN_LED_ROTARY_EQ_HIGH].count = 15;
	LedData[EN_LED_ROTARY_EQ_HIGH].pattern = 0;
	LedData[EN_LED_ROTARY_EQ_HIGH].tCount = 0;
	LedData[EN_LED_ROTARY_EQ_HIGH].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_EQHI;

	LedData[EN_LED_ROTARY_EQ_LOW].Com = 0;
	LedData[EN_LED_ROTARY_EQ_LOW].Row = 7;
	LedData[EN_LED_ROTARY_EQ_LOW].incCom = false;
	LedData[EN_LED_ROTARY_EQ_LOW].dspId = DISP_CHIP_ID_2;
	LedData[EN_LED_ROTARY_EQ_LOW].value = 0;
	LedData[EN_LED_ROTARY_EQ_LOW].count = 15;
	LedData[EN_LED_ROTARY_EQ_LOW].pattern = 0;
	LedData[EN_LED_ROTARY_EQ_LOW].tCount = 0;
	LedData[EN_LED_ROTARY_EQ_LOW].ledParamKeyId = (uint8_t)FrontPannelParam::paramKey_EQLO;

	//PatternMapping(LedTestAllPowerOn, LED_SINGLE_COLOR_DSP_1_MAX); // TODO need power on seq ?
	//execute();

#if defined DEBUG_PRINT_PARAM_NAME
	// Todo Remove this -- only for testing
	{
		tmpParam[0] = "ROTARYROW";
		tmpParam[1] = "SHIFT";
		tmpParam[2] = "LAST";
		tmpParam[3] = "COPY_NAV";
		tmpParam[4] = "REC";
		tmpParam[5] = "BACKFORWARD";
		tmpParam[6] = "STOP";
		tmpParam[7] = "PLAY";
		tmpParam[8] = "SCULPT";
		tmpParam[9] = "DELETE";
		tmpParam[10] = "INSERT";
		tmpParam[11] = "ROTARYBAR";
		tmpParam[12] = "SONG";
		tmpParam[13] = "SCENE";
		tmpParam[14] = "TRACK";
		tmpParam[15] = "NEXT_TIE";
		tmpParam[16] = "FILL_X";
		tmpParam[17] = "SOLO_SKIP";
		tmpParam[18] = "GANG";
		tmpParam[19] = "PATTERN";
		tmpParam[20] = "TEMPO";
		tmpParam[21] = "INTERNALSOUNDS";
		tmpParam[22] = "USB_STOP";
		tmpParam[23] = "MENU";
		tmpParam[24] = "BRAY";
		tmpParam[25] = "RETURN_NAV";
		tmpParam[26] = "SAVE";
		tmpParam[27] = "RSELECT";
		tmpParam[28] = "MINUS_NAV";
		tmpParam[29] = "PLUS_NAV";
		tmpParam[30] = "TRACKSEL1";
		tmpParam[31] = "TRACKSEL2";
		tmpParam[32] = "TRACKSEL3";
		tmpParam[33] = "TRACKSEL4";
		tmpParam[34] = "TRACKSEL5";
		tmpParam[35] = "TRACKSEL6";
		tmpParam[36] = "TRACKSEL7";
		tmpParam[37] = "TRACKSEL8";
		tmpParam[38] = "TRACKSEL9";
		tmpParam[39] = "TRACKSEL10";
		tmpParam[40] = "TRACKSEL11";
		tmpParam[41] = "TRACKSEL12";
		tmpParam[42] = "TRACKSEL13";
		tmpParam[43] = "TRACKSEL14";
		tmpParam[44] = "TRACKSEL15";
		tmpParam[45] = "TRACKSEL16";
		tmpParam[46] = "TRACKROTARY1";
		tmpParam[47] = "TRACKROTARY2";
		tmpParam[48] = "TRACKROTARY3";
		tmpParam[49] = "TRACKROTARY4";
		tmpParam[50] = "TRACKROTARY5";
		tmpParam[51] = "TRACKROTARY6";
		tmpParam[52] = "TRACKROTARY7";
		tmpParam[53] = "TRACKROTARY8";
		tmpParam[54] = "TRACKROTARY9";
		tmpParam[55] = "TRACKROTARY10";
		tmpParam[56] = "TRACKROTARY11";
		tmpParam[57] = "TRACKROTARY12";
		tmpParam[58] = "TRACKROTARY13";
		tmpParam[59] = "TRACKROTARY14";
		tmpParam[60] = "TRACKROTARY15";
		tmpParam[61] = "TRACKROTARY16";
		tmpParam[62] = "ROTARY_NAV";
		tmpParam[63] = "RELEASE";
		tmpParam[64] = "FILTER";
		tmpParam[65] = "RESONANCE";
		tmpParam[66] = "VOLUME";
		tmpParam[67] = "REVERBE";
		tmpParam[68] = "CHORUS";
		tmpParam[69] = "EQHI";
		tmpParam[70] = "EQLO";
		tmpParam[71] = "paramRot_ROW";
		tmpParam[72] = "paramRot_PADROT1";
		tmpParam[73] = "paramRot_PADROT2";
		tmpParam[74] = "paramRot_PADROT3";
		tmpParam[75] = "paramRot_PADROT4";
		tmpParam[76] = "paramRot_PADROT5";
		tmpParam[77] = "paramRot_PADROT6";
		tmpParam[78] = "paramRot_PADROT7";
		tmpParam[79] = "paramRot_PADROT8";
		tmpParam[80] = "paramRot_PADROT9";
		tmpParam[81] = "paramRot_PADROT10";
		tmpParam[82] = "paramRot_PADROT11";
		tmpParam[83] = "paramRot_PADROT12";
		tmpParam[84] = "paramRot_PADROT13";
		tmpParam[85] = "paramRot_PADROT14";
		tmpParam[86] = "paramRot_PADROT15";
		tmpParam[87] = "paramRot_PADROT16";
		tmpParam[88] = "paramRot_BAR";
		tmpParam[89] = "Internals";
		tmpParam[90] = "PAGESHIFT";
		tmpParam[91] = "MouseClick";
		tmpParam[92] = "ENTER_NAV";
		tmpParam[93] = "RDBLSELECT";
		tmpParam[94] = "RMOVELEFT";
		tmpParam[95] = "RMOVERIGHT";
		tmpParam[96] = "SHIFT_NEXT_TIE";
		tmpParam[97] = "SHIFT_FILL_X";
		tmpParam[98] = "SHIFT_SOLO_SKIP";
		tmpParam[99] = "LCD_Brightness";
		tmpParam[100] = "FPLED_Brightness";
		tmpParam[101] = "PowerKey_Event";
		tmpParam[102] = "USBStop";
	}
#endif //DEBUG_PRINT_PARAM_NAME

	FrontPannelLedExecuteThreadFunctionCB = [](void* context) { static_cast<FrontPannelLed*> (context)->FrontPannelLedExecuteThreadFunction(); };

	return true;
}
//Copy the input pattern to LedData; which will be executed by execute function
bool FrontPannelLed::PatternMapping(uint8_t* InPatternArray, uint8_t size)
{
	for (uint8_t i = 0; i < size; i++)
	{
		LedData[LedPosition_Dsp1[i]].pattern = InPatternArray[i]; // update pattern to LedPosition
		LedData[LedPosition_Dsp1[i]].value = 0;
		LedData[LedPosition_Dsp1[i]].tCount = 0;
	}
	return true;
}
// Update the Value depends on Pattern; Handle base Pattern or Individual Pattern
// Will be called periodically for housekeeping.
bool FrontPannelLed::PatternValueUpdate()
{
	if (!executeDone)
		return false;

	uint8_t tmp = 0;

	for (uint8_t  i = EN_LED_MENU; i < LED_MAX_NUM; i++)
	{
		switch (LedData[i].pattern)
		{
		case EN_LED_PATT_None:
		case EN_LED_PATT_OFF: //Fall through
			LedData[i].value = LED_POWER_OFF;
			LedData[i].tCount = 0;
			break;

		case EN_LED_PATT_ON:
			LedData[i].value = LED_POWER_ON;
			LedData[i].tCount = 0;
			break;

		case EN_LED_PATT_Blink_Slow:
		case EN_LED_PATT_Blink_Fast:
			if (LedData[i].tCount == 0)
				LedData[i].value = ((LedData[i].value == LED_POWER_OFF) ? LED_POWER_ON : LED_POWER_OFF);

			tmp = ((LedData[i].pattern == EN_LED_PATT_Blink_Slow) ? LED_BLINK_SLOW_COUNT : LED_BLINK_FAST_COUNT);
			if (LedData[i].tCount >= tmp)
			{
				LedData[i].value = ((LedData[i].value == LED_POWER_OFF) ? LED_POWER_ON : LED_POWER_OFF);
				LedData[i].tCount = 0;
			}
			break;
		case EN_LED_PATT_Blink3_On:
		case EN_LED_PATT_Blink3_Off:
			if(LedData[i].tCount == 0)
				LedData[i].value = ((LedData[i].value == LED_POWER_OFF) ? LED_POWER_ON : LED_POWER_OFF);

			if (LedData[i].tCount == LED_BLINK_COUNT_1 ||
				LedData[i].tCount == LED_BLINK_COUNT_2 ||
				LedData[i].tCount == LED_BLINK_COUNT_3 ||
				LedData[i].tCount == LED_BLINK_COUNT_4 ||
				LedData[i].tCount == LED_BLINK_COUNT_5
				)
			{
				LedData[i].value = ((LedData[i].value == LED_POWER_OFF) ? LED_POWER_ON : LED_POWER_OFF);
			}
			if (LedData[i].tCount >= LED_BLINK_COUNT_6)
			{
				LedData[i].pattern = ((LedData[i].pattern == EN_LED_PATT_Blink3_On) ? EN_LED_PATT_ON : EN_LED_PATT_OFF);
				LedData[i].tCount = 0;
			}
			break;
		case EN_LED_PATT_Blink5_On:
		case EN_LED_PATT_Blink5_Off:
			if (LedData[i].tCount == 0)
				LedData[i].value = ((LedData[i].value == LED_POWER_OFF) ? LED_POWER_ON : LED_POWER_OFF);

			if (LedData[i].tCount == LED_BLINK_COUNT_1 ||
				LedData[i].tCount == LED_BLINK_COUNT_2 ||
				LedData[i].tCount == LED_BLINK_COUNT_3 ||
				LedData[i].tCount == LED_BLINK_COUNT_4 ||
				LedData[i].tCount == LED_BLINK_COUNT_5 ||
				LedData[i].tCount == LED_BLINK_COUNT_6 ||
				LedData[i].tCount == LED_BLINK_COUNT_7 ||
				LedData[i].tCount == LED_BLINK_COUNT_8
				)
			{
				LedData[i].value = ((LedData[i].value == LED_POWER_OFF) ? LED_POWER_ON : LED_POWER_OFF);
			}
			if (LedData[i].tCount >= LED_BLINK_COUNT_9)
			{
				LedData[i].pattern = ((LedData[i].pattern == EN_LED_PATT_Blink5_On) ? EN_LED_PATT_ON : EN_LED_PATT_OFF);
				LedData[i].tCount = 0;
			}
			break;
		}
	}
	executeDone = false;
	return true;
}

// Update the Value depends on Pattern; Handles only for All Leds and Generic pattern
bool FrontPannelLed::PatternGenericHandler(uint8_t PatternVal)
{
	switch (PatternVal)
	{
	case EN_LED_PATT_PowerON:
		//		PatternMapping (LedTestAllPowerOn, LED_SINGLE_COLOR_DSP_1_MAX);
		break;
	case EN_LED_PATT_ON_ALL:
		PatternMapping(LedTestAllPowerOn, LED_SINGLE_COLOR_DSP_1_MAX);
		break;
	case EN_LED_PATT_OFF_ALL:
		PatternMapping(LedAllPowerOFF, LED_SINGLE_COLOR_DSP_1_MAX);
		break;
	case EN_LED_PATT_SongPage:
		//			PatternMapping (LedAllPowerOFF, LED_SINGLE_COLOR_DSP_1_MAX);
		break;
	}
	return true;
}


bool FrontPannelLed::HandleLedMessageFromUI(uint8_t ledParamId, uint8_t LedPattern, uint8_t LedColor)
{
	if (ledParamId == EN_LED_ALL)
	{
		// Set predefine Pattern
		PatternGenericHandler(LedPattern);
	}
	for (int i = 0; i < EN_MX_LED; i++)
	{
		if (LedData[i].ledParamKeyId == ledParamId)
		{
			LedData[i].pattern = LedPattern;
			LedData[i].value = 0;
			LedData[i].tCount = 0;
#if defined DEBUG_PRINT_PARAM_NAME
			DBG(String(__FUNCTION__) + "---> UI Set Param  Name :" + String(tmpParam[ledParamId]) + " value " + String(LedData[i].pattern));
#else
			//DBG(String(__FUNCTION__) + " UI Set Param  ID :" + String(ledParamId) + " value " + String(LedData[i].pattern));
#endif
		}
	}
	return true;
}


bool FrontPannelLed::execute()
{
	bool proceed = false;
	uint8_t ComDataDsp1[LED_REG_BANKS_DISP_1] = { 0, };   //8 com 16 row [0-7][8-15] - // Each Disp has 16 bit of ROW data
	uint8_t DataToWriteDsp2[LED_REG_BANKS_DISP_2] = { 0, };
	for (uint8_t i = 0; i < EN_MX_LED; i++) // Single Color LED
	{
		if (LedData[i].dspId == DISP_CHIP_ID_1)
		{
			if (LedData[i].value)
			{
				ComDataDsp1[LedData[i].Row] |= (0x1 << LedData[i].Com); // Colum/COM data will be writen
				if (LedData[i].incCom && LedData[i].count)// Multi count LEDs
				{
					for (uint8_t j = 1; j <= LedData[i].count; j++)
					{
						ComDataDsp1[LedData[i].Row + (2*j)] |= (0x1 << LedData[i].Com); // increment Colum/COM data and update
					}
				}
			}
			LedData[i].tCount++;
#if defined DEBUG_PRINT_PARAM_NAME
			DBG(String(__FUNCTION__) + " MCU LED ON - Name :" + String(tmpParam[LedData[i].ledParamKeyId]) + "  Value " + String(LedData[i].value));
#endif
		}
		else if (LedData[i].dspId == DISP_CHIP_ID_2)
		{
			if (LedData[i].value)
			{
				DataToWriteDsp2[LedData[i].Row] = LedData[i].value;
			}
		}
	}

	for (auto i = 0; (i < LED_REG_BANKS_DISP_1 && !proceed); i++) // Prevent sending same data again and again
		proceed = (DataToDsp1[i] != ComDataDsp1[i]);

	if (proceed)
	{
		std::copy(ComDataDsp1, ComDataDsp1 + LED_REG_BANKS_DISP_1, DataToDsp1);
		FrontPanneMgr->updateLedData(ComDataDsp1, LED_REG_BANKS_DISP_1, 1); //OUT will have 1 OR 0 for All LED position in DISP_1
	}
	executeDone = true;
	return proceed;
}

bool FrontPannelLed::startLedExecuteThread()
{
	if (!isLedThreadStarted)
	{
		FrontPannelLedExecuteThread.startThread(FrontPannelLedExecuteThreadFunctionCB, this);
		isLedThreadStarted = true;
	}
	return true;
}