#pragma once
#include <iostream>
#include <fstream> 
#include <stdint.h>
#include <string>
#include <mutex>
#include <atomic>
#include <array>
#include <condition_variable>
#include "module.h"
#include "Common/Os/CrossPlatform/CThread.h"
#include "Common/Modules/ISyncListener.h"
#include "Firmware/Application/Model/MCUInterfaceParams.h"
#include "hardware/source/PlayTribeConnectionManager.h"

#define FRONT_PANEL_KEY_MAX 160
#define FRONT_PANEL_THREAD_INIT_WAIT 2000
#define FRONT_PANEL_THREAD_WAIT_1_SEC 1000
#define FRONT_PANEL_READ_THREAD_INIT_WAIT 100
#define MCU_RX_BUFF_MAX_SIZE 15360
#define PC_RX_BUFF_MAX_SIZE 4096

#define FRONT_PANEL_PC_THREAD_INIT_WAIT 2000

#ifdef _WIN64
//#define FP_TEST_KEYBRD              // uncomment Enable/Disable Key Test code
//#define FP_REMOTE_CLIENT_RECIVE_WIN // Recive from script or device based on IP
#elif __APPLE__
//
#else //linux --- Remote Controll/Debug
//#define FP_SIM_LINUX_REMOTE_SERVER // This config Device as server, This must be enabled to use below
//#define FP_SIM_LINUX_REMOTE_SEND	 // Device will send the MCU inputs to connected client - MCU means Front pannel + MIDI
//#define FP_REMOTE_CLIENT_RECIVE	 // Device will Recive the MCU inputs from connected client

#define FP_UART_LINUX // Enable iMX - MCU Communication port
#define PLAYTRIBE    //Enable Pc Application related code
#endif

#define TEST_DELAY_ID 0xFF
#define TEST_DELAY_TIME 2000
#define MCU_KEY_TEMP_KEY_ID 0xE0
namespace Ignite
{
	class IgniteEngine;
	class FrontPannelUtility;
	class FrontPannelLed;
	class USBFunctionFS;

#if defined FP_TEST_KEYBRD
	struct KeyCodeValue
	{
		paramId_t KeyParam;
		uint8_t KeyValue = 1;
	};
#endif//FP_TEST_KEYBRD

	/* FrontPanneQData
	*  CommandID(8b)  Direction(8b) ID(8b)		SIZE(8b)		Data(160b)
	*	-------------------------------------------------------------------------
	*	MIDI			IN			PORT ID			x		Raw Data to send on Midi port
	*	MIDI			OUT			PORT ID			x		Raw Data recived from Midi port
	*	LED				IN			LED - ID		x		Value
	*	LED				OUT			LED - ID		x		Value
	*	ENCODER			IN			Key - ID		x		8 bit Fast / Slow + 8 bit FW / REV + 16 bit Key Value
	*	B - Ray			IN			Key - ID		x		8 bit Fast / Slow + 8 bit FW / REV + 16 bit Key Value
	*	Key				IN			Key - ID		x		Key Press / Release
	*	Controll		IN							x
	*	Controll		OUT							x
	*/

	class FrontPannelManager : public Module, public ISyncListener, public juce::Component, private Timer, public juce::Thread
	{
	public:
		FrontPannelManager(IParamChangeListener* listener, IgniteEngine& model);
		FrontPannelManager(const FrontPannelManager&) = default;
		FrontPannelManager& operator= (const FrontPannelManager&) = delete;
		FrontPannelManager(FrontPannelManager&&) = delete;
		FrontPannelManager& operator= (FrontPannelManager&&) = delete;
		~FrontPannelManager() = default;

		void setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode);
		std::vector<IgniteParamList> getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist) override;
		void SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode);
		void getParam(moduleId_t moduleSrc, paramId_t param) override;
		void onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);

		void FrontPannelStartThreads(void);
		void shutdownDesktop();
		uint8_t simulatorLed(uint8_t id) const { return id < simulatorLeds.size() ? simulatorLeds[id].load() : 0; }
		std::array<std::atomic<uint8_t>, 116> simulatorLeds{};
		void handleCommandMessage(int id);
		void sendMidiMessageExternal(MidiMessage msg, uint8_t midiport);
		SongManager* getSongManager();
		IgniteEngine& engine;
		uint8_t McuLedMapping[100] = { 0 };
		void updateLedData(uint8_t leddata[], uint16_t arraySize, uint8_t dispID);
		void sendRGBLedData();
		void sendKnobLedData();
		bool isRgbLedDataUpdated() { return RgbLedDataUpdated; };
		bool isKnobLedDataUpdated() { return KnobLedDataUpdated; };
		void HandleSystemMsg(uint8_t id, uint8_t value);
		void timerCallback() override;
		void HandleFwUpgrade(uint8_t id);
		uint8_t openHexFile(std::ifstream* fileptr, const char* file_path);
		uint8 getDataFromHexFile(uint8* data, uint8_t bytes);	
		void rebootDevice();
		DEV_STATUS getDevOpStatus() { return fwUpdateStatus;  }
		bool isShiftKeyActive() const { return ShiftKeyPressed; };
		DeviceInfo fillDeviceStatusInfo();
		void SendSyncOutFreq(uint16_t freq);
		bool sendAsMcuMsg(std::vector<uint8_t> data);//to rute the midimsg recived from USB port to MCU port --- only for testing

	private:
		std::mutex midiSendMutx;
		bool midiSendDone = true;
		std::condition_variable midiSendWait;
		uint16_t KEY_DATA[FRONT_PANEL_KEY_MAX] = { 0 };

		std::atomic<uint32_t> McuRxRead{ 0 };
		std::atomic<uint32_t> McuRxWrite{ 0 };
		uint8_t UartMcuRxBuffer[MCU_RX_BUFF_MAX_SIZE];
		uint8_t UartPCRxBuffer[PC_RX_BUFF_MAX_SIZE];
		std::atomic<int> clientSocket{ -1 };

		uint8_t RGB_Led_Data[RGB_LED_COUNT] = { 0 };//RGB - Data
		uint8_t CV_Led_Data[CV_KNOB_COUNT] = { 0 };//RGB - Data
		bool RgbLedDataUpdated = false;
		bool KnobLedDataUpdated = false;
		bool isHwTestOn = false;
		uint8_t HBState = 0;
		uint8_t HBMissingCount = 0;
		uint8_t McuSoftReset = 0;
		int testPortNum = 0;
		int intTotalSize = 0;
		int intFreeSize = 0;
		int usbTotalSize = 0;
		int usbFreeSize = 0;
		int ramTotalSize = 0;
		int ramFreeSize = 0;
		int fw_ver_index = -1;
		std::string firmwareBuildDate = " ";
		std::string imxFirmwareVersion = " ";
		std::string gdFirmwareVersion = " ";
		std::string newFirmwareBuildDate = " ";
		std::string newImxFirmwareVersion = " ";
		std::string newGdFirmwareVersion = " ";
		std::string serialNumber = " ";
		std::ifstream fileptr;
		std::string fwFileName = "";
		std::string fwFileFullpath = "";
		std::string encryptedFwFileFullpath = "";
		std::string decrptionDir = "/tmp/decrypted_image/";
		std::string extractDir = "/tmp/fwupdate";
		SwUdateSource_t fw_source;
		std::string decrptKey = "PJSDSOMIGNITEMTBE";
		std::string fwHeaderFile = "Ignite_header.img";
		DEV_STATUS fwUpdateStatus = DEV_OPERATION_FAILED;

		//void FrontPannelDecodeMessage(void);
		uint8_t FrontPannelGetDataAt(uint32_t pos, bool incRxPos);
		std::unordered_map<uint8_t, std::chrono::time_point<std::chrono::system_clock>> messageTimestamps;
		inline int uart_write_with_retry(int ttyMcuFileHandler, const void* data, size_t size);
		FrontPannelUtility* FpUtility;
		FrontPannelLed* FpLedHandler;
		IParamChangeListener* paramlistener;
		CThread FrontPannelExternalReadThread;
		CThread FrontPannelExternalWriteThread;
		CThread FrontPannelInternalProcessThread;
		CThread FrontPannelLinuxWatchThread;
		ThreadFunction FrontPannelExternalReadThreadFunctionCB;
		ThreadFunction FrontPannelExternalWriteThreadFunctionCB;
		ThreadFunction FrontPannelInternalProcessThreadFunctionCB;
		ThreadFunction FrontPannelLinuxWatchThreadFunctionCB;
		CMsgQueue<SysMessage> FrontPanneExternalReadQ;
		CMsgQueue<SysMessage> FrontPanneExternalWriteQ;

		CThread FrontPanneEthernetWatchThread;
		ThreadFunction FrontPanneEthernetWatchThreadFunctionCB;
		void FrontPanneEthernetWatchThreadFunction(void);

		void FrontPannelExternalReadThreadFunction(void);
		void FrontPannelExternalWriteThreadFunction(void);
		void FrontPannelInternalProcessThreadFunction(void);
		void FrontPannelLinuxWatchThreadFunction(void);	
		bool isVirtualPort(uint8_t midiport);
		bool isPortAvailable(uint8_t midiport);
		int8_t isFirmwareImagePresent();
		FwStatus_t isValidFirmware();
		bool extractImage();
		int compareSWVersion(String v1, String v2);
		bool getFWUpdatePkgFirmwareVersion();

		void sendLedMessageToMCU(uint8_t LediD, uint8_t LedNum, uint8_t LedColor);
		void setMcuLEDBrightness(uint8_t value);
		void setLCDDisplayBrightness(uint8_t value);
		void stopUsb(uint8_t value);
		void startFirmwareUpdate();
		bool startFactoryReset();
		int8_t decryptFirmwareImage();
		bool getVersionfromFile(std::string filename);
		void removeHeaderFromImage(std::string fwfilepath);
		void removeFirmwareFile();
		void HandleSpecialKey();
		std::vector<std::string> getSWUFiles(const std::string& folderPath, const std::string& imageFormat);
		
		void RequestSyncIn();
		USBFunctionFS* usbFunctionFS;
		PlayTribeConnectionManager* playTribeConMngr;

		void getMemoryStatus();
		void getIntStorageStatus();
		void getUsbStorageStatus();
		bool isUSBStorageConnected();
		void getFirmwareInfo();
		void getSerialNumber();
		void SaveData() { return; };
		void run() override;

		std::unique_ptr <MidiInput> FPMidiinputInterface;
		bool multiKeyPressed = false;
		bool ShiftKeyPressed = false;
		uint16_t errorSkipCount = 0;

		std::unordered_map<uint8_t, int> KeypressCount; // Counter for consecutive presses
		uint8_t KeyStatus[(uint8_t)FrontPannelParam::paramKey_End_KeyMapping];
		std::vector<IgniteParamList> Keyparamlist;
		std::mutex keyReadmutex;
		bool KeyReady = false;
		std::condition_variable KeyReady_cv;


		std::unordered_map <uint8_t, uint8_t> McuToUiMap {
			{(uint8_t)FrontPannelMcuMap::McuMapKey_ROTARYROW, (uint8_t)FrontPannelParam::paramKey_ROTARYROW },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SHIFT,  (uint8_t)FrontPannelParam::paramKey_PAGESHIFT },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_LAST,  (uint8_t)FrontPannelParam::paramKey_LAST },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SAVE,  (uint8_t)FrontPannelParam::paramKey_SAVE },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_REC,  (uint8_t)FrontPannelParam::paramKey_REC },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_BACKFORWARD,  (uint8_t)FrontPannelParam::paramKey_BACKFORWARD },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_STOP,  (uint8_t)FrontPannelParam::paramKey_STOP },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_PLAY,  (uint8_t)FrontPannelParam::paramKey_PLAY },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SCULPT,  (uint8_t)FrontPannelParam::paramKey_SCULPT },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_DELETE,  (uint8_t)FrontPannelParam::paramKey_DELETE },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_INSERT,  (uint8_t)FrontPannelParam::paramKey_INSERT },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_ROTARYBAR,  (uint8_t)FrontPannelParam::paramKey_ROTARYBAR },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SONG,  (uint8_t)FrontPannelParam::paramKey_SONG },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SCENE,  (uint8_t)FrontPannelParam::paramKey_SCENE },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACK,  (uint8_t)FrontPannelParam::paramKey_TRACK },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_NEXT_TIE,  (uint8_t)FrontPannelParam::paramKey_NEXT_TIE },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_FILL_X,  (uint8_t)FrontPannelParam::paramKey_FILL_X },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SOLO_SKIP,  (uint8_t)FrontPannelParam::paramKey_SOLO_SKIP },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_GANG,  (uint8_t)FrontPannelParam::paramKey_GANG },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_PATTERN,  (uint8_t)FrontPannelParam::paramKey_PATTERN },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TEMPO,  (uint8_t)FrontPannelParam::paramKey_TEMPO },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_INTERNALSOUNDS,  (uint8_t)FrontPannelParam::paramKey_INTERNALSOUNDS },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_USB_STOP,  (uint8_t)FrontPannelParam::paramKey_USB_STOP },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_MENU,  (uint8_t)FrontPannelParam::paramKey_MENU },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_BRAY,  (uint8_t)FrontPannelParam::paramKey_BRAY },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RETURN_NAV,  (uint8_t)FrontPannelParam::paramKey_RETURN_NAV },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_COPY_NAV,  (uint8_t)FrontPannelParam::paramKey_COPY_NAV },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RSELECT,  (uint8_t)FrontPannelParam::paramKey_RSELECT },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_MINUS_NAV,  (uint8_t)FrontPannelParam::paramKey_MINUS_NAV },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_PLUS_NAV,  (uint8_t)FrontPannelParam::paramKey_PLUS_NAV },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL1,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL1 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL2,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL2 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL3,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL3 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL4,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL4 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL5,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL5 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL6,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL6 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL7,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL7 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL8,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL8 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL9,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL9 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL10,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL10 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL11,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL11 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL12,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL12 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL13,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL13 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL14,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL14 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL15,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL15 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKSEL16,  (uint8_t)FrontPannelParam::paramKey_TRACKSEL16 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY1,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY1 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY2,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY2 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY3,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY3 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY4,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY4 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY5,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY5 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY6,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY6 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY7,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY7 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY8,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY8 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY9,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY9 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY10,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY10 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY11,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY11 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY12,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY12 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY13,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY13 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY14,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY14 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY15,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY15 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TRACKROTARY16,  (uint8_t)FrontPannelParam::paramKey_TRACKROTARY16 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_ROTARY_NAV,  (uint8_t)FrontPannelParam::paramKey_ROTARY_NAV },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RELEASE,  (uint8_t)FrontPannelParam::paramKey_RELEASE },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_FILTER,  (uint8_t)FrontPannelParam::paramKey_FILTER },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RESONANCE,  (uint8_t)FrontPannelParam::paramKey_RESONANCE },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_VOLUME,  (uint8_t)FrontPannelParam::paramKey_VOLUME },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_REVERBE,  (uint8_t)FrontPannelParam::paramKey_REVERBE },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_CHORUS,  (uint8_t)FrontPannelParam::paramKey_CHORUS },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_EQHI,  (uint8_t)FrontPannelParam::paramKey_EQHI },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_EQLO,  (uint8_t)FrontPannelParam::paramKey_EQLO },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_ROW,  (uint8_t)FrontPannelParam::paramRot_ROW },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT1,  (uint8_t)FrontPannelParam::paramRot_PADROT1 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT2,  (uint8_t)FrontPannelParam::paramRot_PADROT2 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT3,  (uint8_t)FrontPannelParam::paramRot_PADROT3 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT4,  (uint8_t)FrontPannelParam::paramRot_PADROT4 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT5,  (uint8_t)FrontPannelParam::paramRot_PADROT5 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT6,  (uint8_t)FrontPannelParam::paramRot_PADROT6 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT7,  (uint8_t)FrontPannelParam::paramRot_PADROT7 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT8,  (uint8_t)FrontPannelParam::paramRot_PADROT8 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT9,  (uint8_t)FrontPannelParam::paramRot_PADROT9 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT10,  (uint8_t)FrontPannelParam::paramRot_PADROT10 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT11,  (uint8_t)FrontPannelParam::paramRot_PADROT11 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT12,  (uint8_t)FrontPannelParam::paramRot_PADROT12 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT13,  (uint8_t)FrontPannelParam::paramRot_PADROT13 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT14,  (uint8_t)FrontPannelParam::paramRot_PADROT14 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT15,  (uint8_t)FrontPannelParam::paramRot_PADROT15 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_PADROT16,  (uint8_t)FrontPannelParam::paramRot_PADROT16 },
			{ (uint8_t)FrontPannelMcuMap::McuMapRot_BAR,  (uint8_t)FrontPannelParam::paramRot_BAR },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_Internals,  (uint8_t)FrontPannelParam::paramKey_Internals },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_PAGESHIFT,  (uint8_t)FrontPannelParam::paramKey_PAGESHIFT },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_MouseClick,  (uint8_t)FrontPannelParam::paramKey_MouseClick },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_ENTER_NAV,  (uint8_t)FrontPannelParam::paramKey_ENTER_NAV },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RDBLSELECT,  (uint8_t)FrontPannelParam::paramKey_RDBLSELECT },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RMOVELEFT,  (uint8_t)FrontPannelParam::paramKey_RMOVELEFT },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RMOVERIGHT,  (uint8_t)FrontPannelParam::paramKey_RMOVERIGHT },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SHIFT_NEXT_TIE,  (uint8_t)FrontPannelParam::paramKey_SHIFT_NEXT_TIE },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SHIFT_FILL_X,  (uint8_t)FrontPannelParam::paramKey_SHIFT_FILL_X },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_SHIFT_SOLO_SKIP,  (uint8_t)FrontPannelParam::paramKey_SHIFT_SOLO_SKIP },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_TempoValue,  (uint8_t)FrontPannelParam::paramKey_TempoValue },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP1,  (uint8_t)FrontPannelParam::param_RGB_DISP1 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP2,  (uint8_t)FrontPannelParam::param_RGB_DISP2 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP3,  (uint8_t)FrontPannelParam::param_RGB_DISP3 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP4,  (uint8_t)FrontPannelParam::param_RGB_DISP4 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP5,  (uint8_t)FrontPannelParam::param_RGB_DISP5 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP6,  (uint8_t)FrontPannelParam::param_RGB_DISP6 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP7,  (uint8_t)FrontPannelParam::param_RGB_DISP7 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP8,  (uint8_t)FrontPannelParam::param_RGB_DISP8 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP9,  (uint8_t)FrontPannelParam::param_RGB_DISP9 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP10,  (uint8_t)FrontPannelParam::param_RGB_DISP10 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP11,  (uint8_t)FrontPannelParam::param_RGB_DISP11 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP12,  (uint8_t)FrontPannelParam::param_RGB_DISP12 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP13,  (uint8_t)FrontPannelParam::param_RGB_DISP13 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP14,  (uint8_t)FrontPannelParam::param_RGB_DISP14 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP15,  (uint8_t)FrontPannelParam::param_RGB_DISP15 },
			{ (uint8_t)FrontPannelMcuMap::McuMap_RGB_DISP16,  (uint8_t)FrontPannelParam::param_RGB_DISP16 },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_LCD_Brightness,  (uint8_t)FrontPannelParam::paramKey_LCD_Brightness },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_FPLED_Brightness,  (uint8_t)FrontPannelParam::paramKey_FPLED_Brightness },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_PowerKey_Event,  (uint8_t)FrontPannelParam::paramKey_PowerKey_Event },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_USBStop,  (uint8_t)FrontPannelParam::paramKey_USBStop },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_getSerialNumber,  (uint8_t)FrontPannelParam::paramKey_getSerialNumber },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_getFirmwareVersion,  (uint8_t)FrontPannelParam::paramKey_getFirmwareVersion },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_getFirmwareBuildInfo,  (uint8_t)FrontPannelParam::paramKey_getFirmwareBuildInfo },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_Firmware_Update,  (uint8_t)FrontPannelParam::paramKey_Firmware_Update },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_Valid_Firmware_check,  (uint8_t)FrontPannelParam::paramKey_Valid_Firmware_check },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_EmmcGetTotalSize,  (uint8_t)FrontPannelParam::paramKey_EmmcGetTotalSize },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_EmmcGetFreeSize,  (uint8_t)FrontPannelParam::paramKey_EmmcGetFreeSize },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_UsbGetConnectionStatus,  (uint8_t)FrontPannelParam::paramKey_UsbGetConnectionStatus },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_UsbGetTotalSize,  (uint8_t)FrontPannelParam::paramKey_UsbGetTotalSize },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_UsbGetFreeSize,  (uint8_t)FrontPannelParam::paramKey_UsbGetFreeSize },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RamGetTotalSize,  (uint8_t)FrontPannelParam::paramKey_RamGetTotalSize },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_RamGetFreeSize,  (uint8_t)FrontPannelParam::paramKey_RamGetFreeSize },
			{ (uint8_t)FrontPannelMcuMap::McuMapKey_FactoryReset,  (uint8_t)FrontPannelParam::paramKey_FactoryReset },
			{ (uint8_t)FrontPannelMcuMap::McuMapFrontPannelMax,  (uint8_t)FrontPannelParam::paramFrontPannelMax }
		};
		uint16_t SyncOutMap[288] = { 65535, 62535, 22935, 17335, 14035, 11735, 10035, 8735 , 7700 , 7035 , 6430 , 5890 , 5450 , 5050 , 4720 , 4405 , 4155 , 3925 , 3509 , 3436 ,
 3330 , 3165 , 2925 , 2808 , 2698 , 2643 , 2569 , 2498 , 2336 , 2282 , 2217 , 2143 , 2099 , 1992 , 1962 , 1919 , 1868 , 1828 , 1793 , 1716 ,
 1680 , 1652 , 1609 , 1585 , 1537 , 1498 , 1468 , 1447 , 1426 , 1399 , 1357 , 1330 , 1312 , 1291 , 1281 , 1245 , 1217 , 1195 , 1176 , 1165 ,
 1149 , 1126 , 1104 , 1088 , 1074 , 1061 , 1047 , 1040 , 1011 , 998  , 985  , 974  , 957  , 942  , 930  , 920  , 910  , 900  , 889  , 873  ,
 862  , 853  , 846  , 837  , 822  , 812  , 805  , 797  , 789  , 778  , 767  , 761  , 754  , 748  , 741  , 731  , 722  , 716  , 709  , 702  ,
 689  , 689  , 681  , 675  , 669  , 664  , 656  , 649  , 644  , 639  , 634  , 629  , 623  , 616  , 611  , 605  , 601  , 597  , 591  , 585  ,
 581  , 577  , 573  , 568  , 562  , 558  , 554  , 550  , 547  , 541  , 536  , 533  , 529  , 526  , 523  , 519  , 513  , 510  , 507  , 504  ,
 500  , 495  , 492  , 489  , 486  , 483  , 480  , 475  , 472  , 470  , 467  , 465  , 460  , 457  , 454  , 452  , 450  , 446  , 442  , 440  ,
 438  , 436  , 433  , 430  , 427  , 425  , 422  , 420  , 418  , 415  , 412  , 410  , 408  , 406  , 402  , 400  , 397  , 395  , 394  , 393  ,
 389  , 386  , 385  , 384  , 382  , 380  , 377  , 374  , 373  , 371  , 369  , 367  , 365  , 363  , 361  , 360  , 358  , 356  , 354  , 352  ,
 351  , 349  , 348  , 346  , 344  , 342  , 341  , 339  , 337  , 335  , 334  , 333  , 331  , 330  , 329  , 326  , 325  , 323  , 322  , 321  ,
 319  , 317  , 316  , 315  , 314  , 312  , 310  , 309  , 308  , 307  , 306  , 304  , 303  , 301  , 300  , 299  , 298  , 296  , 295  , 294  ,
 293  , 292  , 290  , 289  , 288  , 287  , 286  , 285  , 283  , 282  , 281  , 280  , 279  , 278  , 276  , 276  , 275  , 274  , 273  , 271  ,
 270  , 269  , 268  , 268  , 267  , 266  , 264  , 263  , 262  , 262  , 261  , 260  , 258  , 257  , 257  , 256  , 255  , 254  , 253  , 252  ,
 251  , 251  , 250  , 249  , 248  , 247  , 246  , 245 };
#if defined FP_UART_LINUX
		int ttyMcuFileHandler;
		int ttyUSBHostPortHandler;
		std::chrono::steady_clock::time_point lastKeyPress;
		std::atomic<bool> isScheduled;
		int debounceDurationMs = 1000;
#endif //FP_UART_LINUX
	};
}
