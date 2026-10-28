#pragma once
#include <stdint.h>
#include <unordered_map>
#include "anytype.h"
#include "Firmware/Application/Model/MCUInterfaceParams.h"

/* Enable to work with our own 20ms timer, in case we don't want to use juce timer
	Will be useful when we don't want to use Juce audio path and direcly play using platform (Windows/Linux) apis 
	Enabled state works with USE_JUCE_AUDIO disabled */
//#define USE_INTERNAL_20MS_TIMER

/* Experimental
	If disabled Ignite will use direct platform (Windows/Linux) audio framework
	Disabled state works with USE_INTERNAL_20MS_TIMER enabled
	Default state is enabled */
#define USE_JUCE_AUDIO

/* Enable to use, internal audio effects , Attack, Filter, Resonance, Volume, Reverb, Chorus, Eq */
#define USE_INTERNAL_EFFECTS

/* If disabled, Multi track output mixing is disabled
	Only the last track data is output
*/
#define ENABLE_TRACK_MIXING

/* Experimental 
	Enable to use tsf 16 bit rendering, instead of default float rendering */
//#define AUDIO_RENDER_TYPE_SHORT

/* Enable for debugging only - Problems in synthesized audio (internal sound)
   	3 files are created to log data, during play time
	Place the log function calls at required places, in the audio stream. 
	Eg: After tsf synthesis / after effects / after gain changes
   	Clear/Delete files on target, before start, otherwise data will get appended */
//#define LOG_PCM_TO_FILE

// The desktop clock visits tracks synchronously under the engine state lock.
// Queuing raw Track pointers allowed edits to delete tracks before playback used them.
#ifndef _WIN32
#define ENABLE_THREADS_TO_PLAY
#endif

#ifdef LOG_PCM_TO_FILE

#ifdef _WIN32
#define PCM_LOG_FILE_1	"C:\\Ignite_Files\\file1"
#define PCM_LOG_FILE_2	"C:\\Ignite_Files\\file2"
#define PCM_LOG_FILE_3	"C:\\Ignite_Files\\file3"
#elif defined __linux__
#define PCM_LOG_FILE_1	"/home/ignite/file1"
#define PCM_LOG_FILE_2 	"/home/ignite/file2"
#define PCM_LOG_FILE_3 	"/home/ignite/file3"
#endif /* __linux__ */

#endif /* LOG_PCM_TO_FILE */

#define MAX_UNDO_COUNT  50
#define MAX_PATTERN_UNDO_COUNT  16
#define MAX_SETTINGS_UNDO_COUNT  16
#define MAX_SCENE_GBAR_IN_NUM_STEPS 16 //number of steps
#define MAX_SCENE_LEN_IN_NUM_BARS 200 //number of bars
#define MAX_STEPS_IN_PATTERN_BAR	16
#define SCENE_NAME_PREFIX	"SCENE-"
#define SET_PPQN	96
#define TB4_TICKS_PER_STEP 48
#define NOTES_PER_OCTAVE	12
#define NOT_INITIALISED	(-1)
#define DEFAULT_SONG_TEMPO	130
#define DEFAULT_SONG_SWING	0
#define DEFAULT_SCENE_TEMPO	130
#define DEFAULT_SCENE_SWING	0
#define MAX_NUM_OF_BARS_IN_PATTERN	16
#define MAX_NOTE_VELOCITY	127
#define MAX_NOTE	127
#define MAX_LENGTH_OF_A_NOTE_IN_STEPS	95

#define MAX_MIDICONFIG_PORT 35 //Settings Midi Cnfg Max Port Num
#define MAX_CVIO_PORTS 8 //0 - 7
#define MAX_GATE_PORTS 8 //0 - 7
#define MAX_NUM_UDS	10	//UserDefined Scales
#define NUM_UDS_NODES 12	//Number of UserDefined Scales

#define MIDI_MCU_PORT_1 0
#define MIDI_MCU_PORT_2 1
#define MIDI_MCU_PORT_3 2
#define MIDI_MCU_PORT_4 3
#define MIDI_MCU_PORT_5 4

#define ADSR_VOLTAGE_SAMPLE_RATE	500
#define CV_PEAK_VOLTAGE	5

#define ADC_12BIT_RANGE	4096
#define MIDI_CC_VALUE_RANGE	128
#define MIDI_PRESSURE_VALUE_RANGE	128
#define MIDI_PITCH_BEND_RANGE	16384
#define MIDI_VELOCITY_VALUE_RANGE	128

#define FP_LED_GLOW_SLOWBLINK 1
#define FP_LED_GLOW_FASTBLINK 2

#define MIDI_CC_MAX_VALUE	127
#define TIMER_TIMEOUT_UPDATE_TEMPO_MILLISECOND 2000
#define TIMER_TIMEOUT_SLIDER_RESET_MILLISECOND 500

#define ROTARY_LEFT 0
#define ROTARY_RIGHT 1
#define ROTARY_FASTLEFT 4
#define ROTARY_FASTRIGHT 5
#define ROTARY_SUPERFASTLEFT 6
#define ROTARY_SUPERFASTRIGHT 7

#define ROW_HEIGHT 52

#define ROTARY_FASTJUMP 5
#define ROTARY_SUPERFASTJUMP 15

#define LISTVIEW_MAPPED_COMP 5
#define STEPVIEW_MAPPED_COMP 4

#define MAX_DATA_SIZE	100   //Actual data Max size
#define PACKET_SIZE		108   //Size include Headers

#define AUDIO_REVERB 0
#define AUDIO_CHORUS 1
#define AUDIO_EQ_H	 2
#define AUDIO_EQ_L	 3
#define AUDIO_FILTER 4
#define AUDIO_EFFECT_MAX 5

#define AUDITIONING_MAX_PORT 16
#define PRESET_INVALID_ID -1
#define MAX_NUMBER_OF_TVKNOB 24

#define FREQ_UI_UPDATE_PLAYTIME 100
#define PITCH_BEND 4

//---> <START> PC to Device Communication IDs
// < Hearder Ids>
#define PC_SYS_MSG					1     // ID to send/recive System file or System cmd
#define PC_DATA_MSG					2     // ID to send/recive Data files
#define PC_SYS_COMMAND				6

// < Sub Command Ids>
#define PC_SYS_DATA_REQ				1
#define PC_SYS_DATA_CONTINUE		2
#define PC_SYS_DATA_COMPLETE		3
#define PC_SYS_DATA_REPEAT			5
#define PC_SYS_DATA_ACK				6
#define PC_SYS_CONFIRMATION         7
#define PC_SYS_CONFIRMATION_DONE    8

#define PC_SYS_VT_DATA              0xE1
#define PC_SYS_VT_DATA_ACK          0xE2
#define PC_SYS_VT_DATA_COMPLETE     0xE3
#define PC_SYS_VT_DATA_REPEAT       0xE4

// < Sys Execute cmd IDs <PC_SYS_COMMAND>
#define SYSTM_FW_UPDATE				0x01
#define SYSTM_REBOOT				0x02
#define SYSTM_FW_UPDATE_ACK         0x03
#define SYSTM_FW_UPDATE_COMPLETE    0x04
#define SYSTM_FW_UPDATE_DATA_REPEAT 0x05
#define SYSTM_INVALID_FW            0x06
#define SYSTM_FW_UP_TO_DATE         0x07
#define SYSTM_FW_NOT_FOUND          0x08

//---> <End> PC to Device Communication IDs


#define INTERNAL_AUDIO_SAMPLE_RATE	48000
#define INTERNAL_AUDIO_20MS_BLOCK_SIZE	960
#define INTERNAL_AUDIO_CHANNEL_COUNT	1//using TSF mono rendering
#define INTERNAL_AUDIO_SAMPLE_SIZE_BITS	32//using TSF float 32 rendering

#define MAX_POLYPHONIC_NOTES 8

#define IGNITE_MAX_USB_HOST_MIDI_DEVICES	5

#define IGNITE_DIN_MIDI_PORT_PREFIX		"din_midi-"
#define IGNITE_DIN_MIDI_PORT_1_NAME		IGNITE_DIN_MIDI_PORT_PREFIX"1"//0
#define IGNITE_DIN_MIDI_PORT_2_NAME		IGNITE_DIN_MIDI_PORT_PREFIX"2"//1
#define IGNITE_DIN_MIDI_PORT_3_NAME		IGNITE_DIN_MIDI_PORT_PREFIX"3"//2
#define IGNITE_DIN_MIDI_PORT_4_NAME		IGNITE_DIN_MIDI_PORT_PREFIX"4"//3
#define IGNITE_DIN_MIDI_PORT_5_NAME		IGNITE_DIN_MIDI_PORT_PREFIX"5"//4

#define IGNITE_USB_D_MIDI_PORT_PREFIX   "f_midi-"
#define IGNITE_USB_D_MIDI_PORT_1_NAME	IGNITE_USB_D_MIDI_PORT_PREFIX"0"//5
#define IGNITE_USB_D_MIDI_PORT_2_NAME	IGNITE_USB_D_MIDI_PORT_PREFIX"1"//6
#define IGNITE_USB_D_MIDI_PORT_3_NAME	IGNITE_USB_D_MIDI_PORT_PREFIX"2"//7
#define IGNITE_USB_D_MIDI_PORT_4_NAME	IGNITE_USB_D_MIDI_PORT_PREFIX"3"//8
#define IGNITE_USB_D_MIDI_PORT_5_NAME	IGNITE_USB_D_MIDI_PORT_PREFIX"4"//9
#define IGNITE_USB_D_MIDI_PORT_6_NAME	IGNITE_USB_D_MIDI_PORT_PREFIX"5"//10

#define IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME	"f_host" //11 //using a constant name internally, connected device name will depend on that device

#define IGNITE_SYNC_PORT_NAME			"sync-port"//12
#define IGNITE_INTERNAL_CLK_NAME		"internal"//13

typedef int16_t TickLength_t; //max value for TB1 will be 192
typedef int16_t	TickOffset_t; //based on pattern's timebase max -96 to +95(for TB1) and min -2 to +1 (for TB 32T)

//Check std::vector<juce::String> portList, before changing this order
//Both should match
enum midiDataIOPorts
{
	dinMidi1 = 0,
	dinMidi2,
	dinMidi3,
	dinMidi4,
	dinMidi5,

	cv1,
	cv2,
	cv3,
	cv4,
	cv5,
	cv6,
	cv7,
	cv8,

	usbDevMidi1,
	usbDevMidi2,
	usbDevMidi3,
	usbDevMidi4,
	usbDevMidi5,
	usbDevMidi6,

	usbHostMidi1,
	usbHostMidi2,
	usbHostMidi3,
	usbHostMidi4,
	usbHostMidi5,

	maxMidiIOPorts
};

//CLK sources and sinks
//Below order of ports is used in for loops and ranges, check before changing 
#define IGNITE_CLK_PORT_DIN_MIDI_PORT_1	  0
#define IGNITE_CLK_PORT_DIN_MIDI_PORT_2	  1
#define IGNITE_CLK_PORT_DIN_MIDI_PORT_3	  2
#define IGNITE_CLK_PORT_DIN_MIDI_PORT_4	  3
#define IGNITE_CLK_PORT_DIN_MIDI_PORT_5	  4
#define IGNITE_CLK_PORT_USB_D_MIDI_PORT_1  5
#define IGNITE_CLK_PORT_USB_D_MIDI_PORT_2  6
#define IGNITE_CLK_PORT_USB_D_MIDI_PORT_3  7
#define IGNITE_CLK_PORT_USB_D_MIDI_PORT_4  8
#define IGNITE_CLK_PORT_USB_D_MIDI_PORT_5  9
#define IGNITE_CLK_PORT_USB_D_MIDI_PORT_6  10
#define IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_1 11
#define IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_2 12
#define IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_3 13
#define IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_4 14
#define IGNITE_CLK_PORT_USB_HOST_MIDI_PORT_5 15
#define IGNITE_CLK_PORT_SYNC_PORT	17
#define IGNITE_CLK_PORT_INTERNAL_CLK  18
#define IGNITE_CLK_PORT_MAX  19

#define PLAY_THREAD_TRACK_SIZE 8

#define SYSMESSAGE_MAX_DAT_SIZE 50

#define FP_KEYPRESS_TO_HOLD 8 //Number of key press input before sending key hold to UI 

#ifdef __linux__

#define SOUND_FONT_FILE	"/app/Ignite.sf2"

#elif defined _WIN32

#define SOUND_FONT_FILE	"C:\\Ignite.sf2"

#elif defined __APPLE__ && defined __MACH__

#define SOUND_FONT_FILE	 "/tmp/Ignite.sf2"

#endif

#define PHY_MOUSE_OVER 0

namespace Ignite
{
	typedef uint8_t paramId_t;
	typedef uint8_t moduleId_t;
	typedef float AudioDataType;

	struct IgniteParamList
	{
		paramId_t param;
		AnyType MsgValue;
		//TODO Change to generic name
		uint32_t startPos; //Used in list of same param type
		uint32_t Count;
	};

	struct SysMessage
	{
		uint8_t Header = 0;
		uint8_t Direction = 0;
		uint8_t Id = 0;
		uint8_t size = 0;
		uint8_t Data[SYSMESSAGE_MAX_DAT_SIZE] = { 0 };
	};

	struct IgniteMessageParam
	{
		moduleId_t moduleDst;
		moduleId_t moduleSrc;
		paramId_t param;
		MsgType messageType = MsgType::msgSetParam;
		AnyType MsgValue;
		bool isPlayMode = false;
		std::vector<IgniteParamList> paramlist;
	};
	enum FileHandlerError
	{
		FILE_ERROR = 0,
		FILE_OK,
		FILE_EXISTS
	};
	enum class SettingsMidiCnfgParam : uint8_t
	{
		paramall = 0,
		paramConfigMidiActivePort,
		paramConfigMidiDestPort,
		paramConfigMidiDestChannel,
		paramConfigMidiThruType,
		paramConfigMidiClkTxMode,
		paramConfigMidiThruFtsOn,
		paramSaveMidiCnfgSetting,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramSettingsMidiCnfgParamMax
	};
	enum class SettingsCvioCnfgParam : uint8_t
	{
		paramall = 0,
		paramActiveCVPort,
		paramActiveCVCalibPort,
		paramCVChannelNumber,
		paramCVLaw,
		paramCVGateType,
		paramCVSlideType,
		paramCVSlideTime,
		paramCVNoteRangeStart,
		paramCVNoteRangeEnd,
		paramCVNotePercent,
		paramCVVelocityPercent,
		paramCVCcNum,
		paramCVCcPercent,
		paramCVPitchBend,
		paramCVVoltage,

		paramActiveGatePort,
		paramGateChannelNum,
		paramGateType,
		paramGateMode,
		paramGateOutput,

		paramEnvAttack,
		paramEnvAttackMin,
		paramEnvAttackMax,
		paramEnvAttackName,
		paramEnvDecay,
		paramEnvDecayMin,
		paramEnvDecayMax,
		paramEnvDecayName,
		paramEnvSustain,
		paramEnvSustainMin,
		paramEnvSustainMax,
		paramEnvSustainName,
		paramEnvRelease,
		paramEnvReleaseMin,
		paramEnvReleaseMax,
		paramEnvReleaseName,
		paramEnvAmount,
		paramEnvAmountMin,
		paramEnvAmountMax,
		paramEnvAmountName,
		paramEnvVelocity,
		paramEnvVelocityMin,
		paramEnvVelocityMax,
		paramEnvVelocityName,
		paramCvNoteType,

		paramCVCalibOffset1,
		paramCVCalibOffset2,
		paramCVCalibOffset3,
		paramCVCalibOffset4,
		paramCVCalibOffset5,
		paramCVCalibOffset6,
		paramCVCalibOffset7,
		paramCVCalibOffset8,
		paramCVCalibOffset9,
		paramCVCalibOffset10,

		paramSaveCvioSetting,
		paramSaveCvioCalibSetting,
		paramFetchCalibData,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramSettingsCvioCnfgParamMax
	};
	enum class SettingsGeneralCnfgParam : uint8_t
	{
		paramall = 0,
		SettingDeviceCnfgUSBParam,
		SettingDeviceCnfgSafeStopParam,
		SettingDeviceCnfgBrightnessParam,
		SettingDeviceCnfgAutoSaveAll,
		SettingInstrumentAutoSaveParam,
		SettingDeviceCnfgSave, //Save All
		paramAuditioningPreset1,
		paramAuditioningPreset2,
		paramAuditioningPreset3,
		paramAuditioningPreset4,
		paramAuditioningPreset5,
		paramAuditioningPreset6,
		paramAuditioningPreset7,
		paramAuditioningPreset8,
		paramAuditioningPreset9,
		paramAuditioningPreset10,
		paramAuditioningPreset11,
		paramAuditioningPreset12,
		paramAuditioningPreset13,
		paramAuditioningPreset14,
		paramAuditioningPreset15,
		paramAuditioningPreset16,
		paramAuditioningPreset17,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramWifiState,
		paramWifiLastConnSsid,
		paramSettingsGeneralCnfgParamMax
	};

	enum class SettingsSongCnfgParam : uint8_t
	{
		paramall = 0,
		SettingSongAutoSaveParam,
		SettingSongNoteNameDispParam,
		SettingSongCnfgSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		SettingSongTapTempoParam,
		settingInsertPosition,
		paramSaveAll,
		paramSettingsSongCnfgParamMax
	};
	enum class SettingsSceneCnfgParam : uint8_t
	{
		paramall = 0,
		SettingSceneAutoSaveParam,
		SettingSceneCnfgSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		SettingSceneTapTempoParam,
		settingInsertPosition,
		SettingSceneSoloModeParam,
		SettingSceneFillModeParam,
		SettingSceneFillActiveCnfgParam,
		SettingSceneFillKeyCnfgParam,
		paramSaveAll,
		paramSettingsSceneCnfgParamMax
	};
	enum class SettingsUdsCnfgParam : uint8_t
	{
		paramall = 0,
		SettingDataLocation,
		SettingsUdsCnfgSaveParam,
		paramUndoCmd,
		paramUndoListCmd,
		paramClearUndoList,
		paramAddUds,
		paramActiveUds,
		paramUdsName,
		paramUdsNameList,
		paramRemoveUds,
		paramUdsNote1,
		paramUdsNote2,
		paramUdsNote3,
		paramUdsNote4,
		paramUdsNote5,
		paramUdsNote6,
		paramUdsNote7,
		paramUdsNote8,
		paramUdsNote9,
		paramUdsNote10,
		paramUdsNote11,
		paramUdsNote12,
		paramUdsConfigDone,
		paramUdsCount,
		paramSaveAll,
		paramSettingsUdsCnfgParamMax
	};
	enum class SettingsTrackCnfgParam : uint8_t
	{
		paramall = 0,
		SettingsTrackAutoSaveParam,
		SettingsTrackCnfgSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramSettingsTrackCnfgParamMax
	};
	enum class SettingsTrackInstrumentMixerParam : uint8_t
	{
		paramall = 0,
		paramTMActiveTrack,
		paramTMPan,
		paramTMVolume,
		paramUndoCmd,
		paramUndoListCmd,
		paramSettingsTrackInstrumentMixerCnfgInit,
		paramSettingsTrackInstrumentMixerCnfgSave,
		paramSettingsTrackInstrumentMixerCnfgCancel,
		paramSaveAll,
		paramSettingsTrackInstrumentMixerParamMax
	};
	enum class SettingsPatternCnfgParam : uint8_t
	{
		paramall = 0,
		SettingPatternAutoSaveParam,
		SettingsPatternCnfgSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramPatternBarFollow,
		paramPatternDefaultNote,
		paramInsertPosition,
		paramSaveAll,
		paramSettingsPatternCnfgParamMax
	};
	enum class SettingsRecordCnfgParam : uint8_t
	{
		paramall = 0,
		SettingRecordQuantizationParam,
		SettingRecordMthruParam,
		SettingRecordFTSParam,
		SettingRecordAutoAddBarParam,
		SettingRecordBarOverWriteParam,
		SettingRecordNumOfBarsParam,
		SettingRecordNoteEventParam,
		SettingRecordMidiCCEventParam,
		SettingRecordAfterTouchEventParam,
		SettingRecordPitchBendEventParam,
		SettingRecordPolyPEventParam,
		SettingRecordPgmChangeEventParam,
		SettingsRecordCnfgSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramSettingsRecordCnfgParamMax
	};
	enum class SettingsBRayCnfgParam : uint8_t
	{
		paramall = 0,
		paramBrayPowerStatus,
		paramBrayAdcValue,
		paramBrayControlledParam,
		paramBrayCcNum,
		paramBRayCnfgSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramSettingsBrayCnfgMax
	};
	enum class SettingsRemoteControlCnfgParam : uint8_t
	{
		paramall = 0,
		SettingsRemoteControlStatus,
		SettingsRemoteControlLatchParam,
		SettingsRemoteControlMidiPort,
		SettingsRemoteControlMidiChannel,
		SettingsRemoteControlSceneChangeParam,
		SettingsRemoteControlFontChangeParam,
		SettingRemoteControlCnfgSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramSettingsRemoteControlCnfgParamMax
	};

	enum class SettingsMidiMachineControlCnfgParam : uint8_t
	{
		paramall = 0,
		paramMidiMachineControlReceiveStatus,
		paramMidiMachineControlSendStatus,
		paramMidiMachineControlInputPort,
		paramMidiMachineControlOutputPort,
		paramMidiMachineControlDeviceId,
		paramMidiMachineControlDeviceIdAll,
		SettingMidiMachineControlCnfgSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramSettingsMidiMachineControlCnfgParamMax
	};

	enum class SettingsSculptModeCnfgParam : uint8_t
	{
		paramall = 0,

		sculptMode,

		sculptRelativeVelocity,
		/*sculptRelativeLengthSteps,
		sculptRelativeLengthTicks,*/
		//sculptRelativeDelay,
		sculptRelativeNote,

		sculptAbsoluteVelocity,
		/*sculptAbsoluteLengthSteps,
		sculptAbsoluteLengthTicks,*/
		//sculptAbsoluteDelay,
		sculptAbsoluteNote,

		sculptRandomRelativeVelocityStart,
		sculptRandomRelativeVelocityEnd,
		/*sculptRandomRelativeLengthStepStart,
		sculptRandomRelativeLengthStepEnd,
		sculptRandomRelativeLengthTickStart,
		sculptRandomRelativeLengthTickEnd,*/
		//sculptRandomRelativeDelayStart,
		//sculptRandomRelativeDelayEnd,
		sculptRandomRelativeNoteStart,
		sculptRandomRelativeNoteEnd,

		sculptRandomAbsoluteVelocityStart,
		sculptRandomAbsoluteVelocityEnd,
		/*sculptRandomAbsoluteLengthStepStart,
		sculptRandomAbsoluteLengthStepEnd,
		sculptRandomAbsoluteLengthTickStart,
		sculptRandomAbsoluteLengthTickEnd,*/
		/*sculptRandomAbsoluteDelayStart,
		sculptRandomAbsoluteDelayEnd,*/
		sculptRandomAbsoluteNoteStart,
		sculptRandomAbsoluteNoteEnd,

		settingSculptConfigSave,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,

		sculptButtonOnState,
		sculptParameterSelection,

		paramSaveAll,
		paramSettingsSculptModeCnfgParamMax
	};
	enum class SettingsMidiClkCnfgParam : uint8_t
	{
		paramall = 0,
		clkInPort,
		inputPpqn,
		outputPpqn,
		saveCmd,
		triggerForIn,
		triggerForOut,
		clkOutPort,
		SettingDataLocation,
		paramUndoCmd,
		paramUndoListCmd,
		paramSaveAll,
		paramSettingsMidiClkCnfgParamMax
	};
	
	enum class SettingsMidiClkCnfgInputClkRate : int8_t
	{
		inClkRate_1PP16 = 0,
		inClkRate_1PP8 = 1,
		inClkRate_1PPQ = 2,
		inClkRate_1PP2Q = 3,
		inClkRate_1PP4Q = 4,
		inClkRate_2PPQ = 5,
		inClkRate_24PPQ = 6,
		inClkRate_48PPQ = 7,
		inClkRateMax	= 7,
	};
	enum class SettingsMidiClkCnfgOutputClkRate : int8_t
	{
		outClkRate_1PP16	= 0,
		outClkRate_1PP8		= 1,
		outClkRate_1PPQ		= 2,
		outClkRate_1PP2Q	= 3,
		outClkRate_1PP4Q	= 4,
		outClkRate_2PPQ		= 5,
		outClkRate_24PPQ	= 6,
		outClkRate_48PPQ	= 7,
		outClkRateMax		= 7,
	};

	struct ClientPatternNodeData
	{
		uint16_t nodeId;// ID may same, delay will be different for the same note
		uint8_t note;
		uint8_t velocity;
		uint8_t lenInSteps;
		TickLength_t lenInTicks; 
		TickOffset_t offset;
		bool onState;
		uint16_t step_id;
		uint8_t noteCount = 0;//UI using it for copy/paste

		bool operator <(const ClientPatternNodeData& rhs) const
		{
			return this->nodeId < rhs.nodeId;
		}
	};

	struct Aux
	{
		bool enabled;
		uint16_t type;
		int8_t data1;
		int8_t data2;
		TickOffset_t offset;

		bool operator==(const Aux& other) const
		{
			return (type == other.type) && (data1 == other.data1);
		}
	};

	struct ClientPatternStepData
	{
		uint16_t BarId;
		uint16_t stepId;
		uint8_t nodeCount;
		std::vector<Aux> auxList;
		bool enable;
		bool skipStatus;
		bool transposeDefeatStatus;
		bool tieStatus;
		//nodeList : for 1st map, key as note : 2nd map key is offset/delay (can be -ive)
		std::unordered_map <uint8_t, std::unordered_map<TickOffset_t, ClientPatternNodeData>> nodeList;
	};

	enum class PatternDirection : uint8_t
	{
		forward,
		reverseA,
		reverseB,
		alternate,
		pendulum,
		random,
		brownian,
		eitherway,
		max
	};

	typedef struct ClientPatternStepData StepData;
	typedef struct ClientPatternNodeData NodeData;
	struct ClienPatterntBarData
	{
		uint16_t BarId = 0;
		uint16_t stepCount = 0;
		uint8_t  barRepeatCount = 0;
		bool barTransposeDefeat = false;
		bool gBarSyncOn = false;
		uint8_t direction = static_cast<uint8_t>(PatternDirection::forward);
		//int16_t barLastStep;
		std::unordered_map <uint16_t, ClientPatternStepData> StepDataList;
	};

	struct ClientUsedByData
	{
		bool isFillPattern;
		std::string songName;
		std::string sceneName;
		std::string trackName;
	};
	enum class PatternDataUpdateReasonType : uint8_t
	{
		PatternDataUpdateReason_UNKNOWN,
		PatternDataUpdateReason_DEFAULT,
		PatternDataUpdateReason_PATTERNDATA,
		PatternDataUpdateReason_numBars,
		PatternDataUpdateReason_direction,
		PatternDataUpdateReason_timeBase,
		PatternDataUpdateReason_patternLastStepId,
		PatternDataUpdateReason_patternName,
		PatternDataUpdateReason_barLoopOn,
		PatternDataUpdateReason_barLoopStartId,
		PatternDataUpdateReason_barLoopEndId,
		PatternDataUpdateReason_BarData,
		PatternDataUpdateReason_AddBar,
		PatternDataUpdateReason_InsertBar,
		PatternDataUpdateReason_CopyBar,
		PatternDataUpdateReason_StepData,
		PatternDataUpdateReason_InsertStep,
		PatternDataUpdateReason_RemoveStep,
		PatternDataUpdateReason_NodeData,
		PatternDataUpdateReason_DeleteNodeData,
		PatternDataUpdateReason_AddNodeData,
		PatternDataUpdateReason_PatternRotate,
		paramSaveAll,
		PatternDataUpdateReason_Max
	};

	struct PatternDataUpdateLocation
	{
		int8_t barId;
		int8_t stepId;
		int8_t noteId;
		int8_t auxId;
	};

	struct ClientPatternData
	{
		uint8_t numBars;
		uint8_t timeBase;
		int16_t patternLastStepId;
		int32_t id;
		std::string patternName;
		std::string patternDispName;
		bool barLoopOn;
		int8_t barLoopStartId;
		int8_t barLoopEndId;
		std::unordered_map <uint8_t, ClienPatterntBarData> BarDataList;
		std::list <ClientUsedByData> usedByList;
	};

	enum class ModuleName : moduleId_t
	{
		moduleEngState = 0,
		moduleSong = 1,
		moduleScene = 2,
		moduleTrack = 3,
		modulePattern = 4,
		moduleInstrument = 5,
		moduleUI = 6,
		moduleFrontPannel = 7,
		moduleUndo = 8,
		moduleMidiCnfg = 9,
		moduleCvioCnfg = 10,
		moduleSettingGnrlCnfg = 11,
		moduleSettingSongCnfg = 12,
		moduleSettingSceneCnfg = 13,
		moduleSettingsTrackCnfg = 14,
		moduleSettingsTrackInstrumentMixer = 15,
		moduleSettingsPatternCnfg = 16,
		moduleSettingsRecordCnfg = 17,
		moduleSettingsBRayCnfg = 18,
		moduleSettingsRemoteControlCnfg = 19,
		moduleSettingsSculptModeCnfg = 20,
		moduleSettingsUdsCnfg = 21,
		moduleSettingsMidiClkCnfg = 22,
		moduleSettingsTransposeControlCnfg = 23,
		moduleSettingsMidiMachineControlCnfg = 24,
		moduleSequenceManager = 25,
		moduleMax
	};

	enum class SongParam : paramId_t
	{
		paramall = 0,
		paramAddSongCmd = 1,
		paramDeleteSongCmd = 2,
		paramActiveSong = 3,
		paramPlaySongCmd = 4,
		paramPauseSongCmd = 5,
		paramSongTempo = 6,
		paramSongSwing = 7,
		paramSongName = 8,
		paramSaveSongCmd = 9,
		paramUndoCmd = 10,
		paramSongNameList = 11,
		paramUndoListCmd = 12,
		paramClearUndoList = 13,
		paramStopSongCmd = 14,
		//paramFillMode = 15,
		paramStartRecordCmd = 15,
		paramStopRecordCmd = 16,
		paramMoveSong = 17,
		paramCopySong = 18,
		paramRecordEventFilter = 19,
		paramTrackCountCmd = 20,
		paramTrackSoloMode = 21,
		paramFillModeSwitchOn = 22,
		paramFillModeType = 23,
		paramFillModeRelease = 24,
		paramApplySongTempoToScenes = 25,
		paramApplySongSwingToScenes = 26,
		paramReStartSongCmd = 27,
		paramFtsNameList = 28,

		ParamTestNumCreateSong = 29,
		ParamTestSetNumScene = 30,
		ParamTestSetNumTrack = 31,
		ParamTestSetNumPattern = 32,
		ParamTestSetNumPatternBars = 33,
		ParamTestSetNumInstrument = 34,
		ParamDeleteAllSong = 35,
		ParamDeleteAllSongInstPattern = 36,
		ParamEngineReady = 37,
		paramSyncIn = 38,
		paramSyncOut = 39,
		ParamEngineCreateDataBackup,
		ParamEngineRestoreDataBackup,

		paramActiveTrackTxInstrumentAvailability,
		paramAuditioningState,

		paramUsbHostMidiChangeNotification,
		paramUsbHostMidiDeviceNameList,

		paramSaveAll,
		paramSystemState,
		paramSongMax
	};

	enum class SceneParam : paramId_t
	{
		paramall = 0,
		paramAddSceneCmd = 1,
		paramAddTotalScenesCmd = 2,
		paramDeleteSceneCmd = 3,
		paramActiveScene = 4,
		paramSceneSwing = 5,
		paramSceneTempo = 6,
		paramSceneName = 7,
		paramCurrentPlaySceneId = 8,
		paramNextPlaySceneId = 9,
		paramSaveSceneCmd = 10,
		paramUndoCmd = 11,
		paramUndoListCmd = 12,
		paramStartScene = 13,
		paramEndScene = 14,
		paramSceneAdvanceMode = 15,
		paramSceneCount = 16,
		paramCurrentPlaySceneName = 17,
		paramNextPlaySceneName = 18,
		paramSceneGBar = 19,
		paramSceneLength = 20,
		paramSceneMuteHold = 21,
		paramSceneFTSRootNote = 22,
		paramSceneFTSScale = 23,
		paramSceneTransposeValue = 24,
		paramSceneTransposeFTSPriority = 25,
		paramManualPlayNextSceneCmd = 26,
		paramManualPlayNewSceneCmd = 27,
		paramSceneNameList = 28,
		paramClearUndoList = 29,
		paramSceneTrackMuteStatus = 30,
		paramCurrentPlaySceneUnmutedTracks = 31,
		paramNextPlaySceneUnmutedTracks = 32,
		paramSceneBarId = 33,
		paramSceneCopy = 34,
		paramSceneMove = 35,
		paramSaveAll,
		paramSceneMax
	};

	enum class TrackParam : paramId_t
	{
		paramall = 0,
		paramActiveTrack = 1,
		paramTrackInstrument = 2,
		paramTrackPattern = 3,
		paramTrackMute = 4,
		paramTrackName = 5,
		paramUndoCmd = 6,
		paramSaveTrackCmd = 7,
		paramUndoListCmd = 8,
		paramTrackInfoList = 9,
		paramTrackFillPattern = 10,
		paramClearUndoList = 11,
		paramTrackSwapCmd = 12,
		paramTrackSolo = 13,
		paramTrackPatternBarId = 14,
		paramTrackPatternStepId = 15,
		paramTrackPatternDispName = 16,
		paramTrackFillPatternDispName = 17,
		paramTrackInstrumentDispName = 18,
		paramTrackRecord = 19,
		paramTrackPatternBarAndStepId,
		paramTrackOutInstrument,
		paramTrackOutInstrumentDispName,
		paramSaveAll,
		paramCvVoltsPort1,
		paramCvVoltsPort2,
		paramCvVoltsPort3,
		paramCvVoltsPort4,
		paramCvVoltsPort5,
		paramCvVoltsPort6,
		paramCvVoltsPort7,
		paramCvVoltsPort8,
		paramCvGateState,
		paramTrackMax
	};

	enum class InstrumentParam : uint8_t
	{
		paramall = 0,
		paramInstrumentAdd = 1,
		paramSetActiveInstrument = 2,
		paramInstrumentChannel = 3,
		paramInstrumentPort = 4,
		paramInstrumentRemove = 5,
		paramInstrumentName = 6,
		paramUndoCmd = 7,
		paramInstrumentNameList = 8,
		paramInstrumentSave = 9,
		paramInternalSoundPgm = 10,
		paramInternalSound = 11,
		paramMultiTimbralStatus = 12,
		paramPolySpreadStatus = 13,
		paramTransposeDefeatStatus = 14,
		paramFtsDefeatStatus = 15,
		paramEditTrackThruStatus = 16,
		paramCc0BankMStatus = 17,
		paramCc0BankLStatus = 18,
		paramDisplayNoteNum = 19,
		paramPreSendPgmChange = 20,
		paramUndoListCmd = 21,
		paramDefaultNote = 22,
		paramClearUndoList = 23,
		paramUsedByListCmd = 24,
		paramUsedBySongList = 25,

		paramTVActiveKnobCmd = 26,
		paramTVName = 27,
		paramTVType = 28,

		paramTVMidiCCNum = 29,
		paramTVMidiCCValue = 30,
		paramTVMidiCCInitValue = 31,
		paramTVMidiCCMinValue = 32,
		paramTVMidiCCMaxValue = 33,

		paramTVMidiPgmNum = 34,
		paramTVMidiPgmBankLsb = 35,
		paramTVMidiPgmBankMsb = 36,

		paramTVSeqQuantPercent = 37,
		paramTVSeqVelocityPercent = 38,
		paramTVSeqVelocityRef = 39,
		paramTVSeqLengthPercent = 40,
		paramTVSeqNotePercent = 41,
		paramTVSeqNoteRef = 42,
		paramTVSeqTransposeSemiTone = 43,
		paramTVSeqTransposeOctave = 44,
		paramTVSeqFtsScale = 45,
		paramTVSeqFtsRootNote = 46,
		paramTVSeqReichValue = 47,
		paramTVSaveScope = 48,
		paramAddTrackValueCmd = 49,
		paramDeleteTrackValueCmd = 50,
		paramInstrumentCopy = 51,
		paramInstrumentMoveCmd = 52,

		paramTMActiveTrack = 53,
		paramTMPan = 54,
		paramTMVolume = 55,

		paramCountTrackValueCmd = 56,
		paramTVKnobPresenceQuery = 57,
		paramInstrumentDispName = 58,
		paramUsedBySceneList = 59,
		paramUsedByTrackList = 60,

		paramInstrumentPgmBankLsb,
		paramInstrumentPgmBankMsb,
		paramInstrumentPgmNum,

		paramTVKnobEnable,
		ParamDeleteAllInstrument,
		paramTrackEffectMuteUnmute,
		paramSaveAll,
		paramInstrumentMax
	};

	enum class PatternParam : uint8_t
	{
		paramall = 0,
		paramAddPattern,
		paramSavePatternCmd,
		paramPatternName,
		paramRemovePattern,
		paramUndoCmd,
		paramPatternUpdateNotif,
		setActivePattern,
		paramClearUndoList,
		paramMovePatternCmd,
		ParamDeleteAllPattern,
		ParamSelectedStep,
		paramSaveAll,
		paramPatternMax
	};

	enum class FrontPannelParam : uint8_t
	{
		paramall = 0,
		paramKey_ROTARYROW = 0,
		paramKey_SAVE = 1,
		paramKey_LAST = 2,
		paramKey_PAGESHIFT = 3,
		paramKey_REC = 4,
		paramKey_BACKFORWARD = 5,
		paramKey_STOP = 6,
		paramKey_PLAY = 7,
		paramKey_SCULPT = 8,
		paramKey_DELETE = 9,
		paramKey_INSERT = 10,
		paramKey_ROTARYBAR = 11,
		paramKey_SONG = 12,
		paramKey_SCENE = 13,
		paramKey_TRACK = 14,
		paramKey_NEXT_TIE = 15,
		paramKey_FILL_X = 16,
		paramKey_SOLO_SKIP = 17,
		paramKey_GANG = 18,
		paramKey_PATTERN = 19,
		paramKey_TEMPO = 20,
		paramKey_INTERNALSOUNDS = 21,
		paramKey_USB_STOP = 22,
		paramKey_MENU = 23,
		paramKey_BRAY = 24,
		paramKey_RETURN_NAV = 25,
		paramKey_COPY_NAV = 26,
		paramKey_RSELECT = 27,
		paramKey_MINUS_NAV = 28,
		paramKey_PLUS_NAV = 29,
		paramKey_TRACKSEL1 = 30, //RGB Key --> button + led + below * BLUE LED *
		paramKey_TRACKSEL2 = 31,
		paramKey_TRACKSEL3 = 32,
		paramKey_TRACKSEL4 = 33,
		paramKey_TRACKSEL5 = 34,
		paramKey_TRACKSEL6 = 35,
		paramKey_TRACKSEL7 = 36,
		paramKey_TRACKSEL8 = 37,
		paramKey_TRACKSEL9 = 38,
		paramKey_TRACKSEL10 = 39,
		paramKey_TRACKSEL11 = 40,
		paramKey_TRACKSEL12 = 41,
		paramKey_TRACKSEL13 = 42,
		paramKey_TRACKSEL14 = 43,
		paramKey_TRACKSEL15 = 44,
		paramKey_TRACKSEL16 = 45, // RGB Key <<-- End + below * BLUE LED * ; RGB define below param_RGB_DISP1
		paramKey_TRACKROTARY1 = 46, // Rotary push Key --> button + led
		paramKey_TRACKROTARY2 = 47,
		paramKey_TRACKROTARY3 = 48,
		paramKey_TRACKROTARY4 = 49,
		paramKey_TRACKROTARY5 = 50,
		paramKey_TRACKROTARY6 = 51,
		paramKey_TRACKROTARY7 = 52,
		paramKey_TRACKROTARY8 = 53,
		paramKey_TRACKROTARY9 = 54,
		paramKey_TRACKROTARY10 = 55,
		paramKey_TRACKROTARY11 = 56,
		paramKey_TRACKROTARY12 = 57,
		paramKey_TRACKROTARY13 = 58,
		paramKey_TRACKROTARY14 = 59,
		paramKey_TRACKROTARY15 = 60,
		paramKey_TRACKROTARY16 = 61, //Rotary push Key <-- button + led
		paramKey_ROTARY_NAV = 62,
		paramKey_RELEASE = 63,
		paramKey_FILTER = 64,
		paramKey_RESONANCE = 65,
		paramKey_VOLUME = 66,
		paramKey_REVERBE = 67,
		paramKey_CHORUS = 68,
		paramKey_EQHI = 69,
		paramKey_EQLO = 70,
		paramRot_ROW = 71,	 //Rotary rotate --> Right/Left + led
		paramRot_PADROT1 = 72, // Rotary rotate --> Right/Left + led
		paramRot_PADROT2 = 73,
		paramRot_PADROT3 = 74,
		paramRot_PADROT4 = 75,
		paramRot_PADROT5 = 76,
		paramRot_PADROT6 = 77,
		paramRot_PADROT7 = 78,
		paramRot_PADROT8 = 79,
		paramRot_PADROT9 = 80,
		paramRot_PADROT10 = 81,
		paramRot_PADROT11 = 82,
		paramRot_PADROT12 = 83,
		paramRot_PADROT13 = 84,
		paramRot_PADROT14 = 85,
		paramRot_PADROT15 = 86,
		paramRot_PADROT16 = 87, // Rotary rotate --> Right/Left + led
		paramRot_BAR = 88,		// Rotary rotate-- > Right / Left + led
		paramKey_Internals = 89,// --> Below has no mapping on MCU/Front pannel internel Key mapping
		paramKey_SHIFT = 90,
		paramKey_MouseClick = 91,
		paramKey_ENTER_NAV = 92,
		paramKey_RDBLSELECT = 93,
		paramKey_RMOVELEFT = 94,
		paramKey_RMOVERIGHT = 95,
		paramKey_SHIFT_NEXT_TIE = 96,
		paramKey_SHIFT_FILL_X = 97,
		paramKey_SHIFT_SOLO_SKIP = 98,
		paramKey_End_KeyMapping = 99,
		paramKey_TempoValue = paramKey_End_KeyMapping,// <---------- internel Key mapping
		param_RGB_DISP1 = 100, // Only to set color -- for key event use paramKey_TRACKSEL
		param_RGB_DISP2 = 101,
		param_RGB_DISP3 = 102,
		param_RGB_DISP4 = 103,
		param_RGB_DISP5 = 104,
		param_RGB_DISP6 = 105,
		param_RGB_DISP7 = 106,
		param_RGB_DISP8 = 107,
		param_RGB_DISP9 = 108,
		param_RGB_DISP10 = 109,
		param_RGB_DISP11 = 110,
		param_RGB_DISP12 = 111,
		param_RGB_DISP13 = 112,
		param_RGB_DISP14 = 113,
		param_RGB_DISP15 = 114,
		param_RGB_DISP16 = 115,
		paramKey_LCD_Brightness = 116,// --- > System Events
		paramKey_FPLED_Brightness = 117,
		paramKey_PowerKey_Event = 118,
		paramKey_USBStop = 119,
		paramKey_getSerialNumber = 120,
		paramKey_getFirmwareVersion = 121,
		paramKey_getFirmwareBuildInfo = 122,
		paramKey_Firmware_Update = 123,
		paramKey_Valid_Firmware_check = 124,
		paramKey_EmmcGetTotalSize = 125,
		paramKey_EmmcGetFreeSize = 126,
		paramKey_UsbGetConnectionStatus = 127,
		paramKey_UsbGetTotalSize = 128,
		paramKey_UsbGetFreeSize = 129,
		paramKey_RamGetTotalSize = 130,
		paramKey_RamGetFreeSize = 131,
		paramKey_FactoryReset = 132,//<--- System Events
		paramKey_HwTestMode = 133,
		paramKey_KeyDataReady = 134,
		paramKey_FwUpgrade = 135,
		paramKey_DataCorrupted = 136,
		paramKey_McuSleepMode = 137,
		paramKey_decryptFirmwareImage = 138,
		paramKey_Firmware_Update_Source = 139,
		paramKey_ExtractFirmwareImage = 140,
		param_SyncOut = 141,
		param_SyncIn = 142,
		paramKey_DevStatus = 143,
		paramKey_RebootDevice = 144,
		param_removeFirmwarePacakge = 145,
		param_startPlayTribeUsbHandler = 146,
		param_SOMFwUpdate = 147,
		param_MCU_FWVERSION = 148,
		paramSaveAll,
		paramFrontPannelMax,
		param_startPlayTribeWifiHandler,
		param_stopPlayTribeWifiHandler
	};
	enum class FrontPannelKeyStatus : uint8_t
	{
		paramKeyStatusIdle = 0,
		paramKeyStatusPressed = 1,
		paramKeyStatusReleased = 2,
		paramKeyStatusOnHold = 3,
		paramKeyStatusInvalid
	};

	enum class PatternTimeBase : uint8_t
	{
		t1 = 1,
		t2 = 2,
		t4 = 3,
		t8 = 4,
		t16 = 5,
		t32 = 6,
		t64 = 7,
		triplet2 = 8,
		triplet4 = 9,
		triplet8 = 10,
		triplet16 = 11,
		triplet32 = 12,
		triplet64 = 13,
		polyrythm = 14,
		patternTimeBaseMax
	};

	enum class Note : uint8_t
	{
		noteMin = 0,
		noteC0 = noteMin,
		noteCSharp0 = 1,
		noteD0 = 2,
		noteDSharp0 = 3,
		noteE0 = 4,
		noteF0 = 5,
		noteFSharp0 = 6,
		noteG0 = 7,
		noteGSharp0 = 8,
		noteA0 = 9,
		noteASharp0 = 10,
		noteB0 = 11,
		noteC1 = 12,
		noteCSharp1 = 13,
		noteD1 = 14,
		noteDSharp1 = 15,
		noteE1 = 16,
		noteF1 = 17,
		noteFSharp1 = 18,
		noteG1 = 19,
		noteGSharp1 = 20,
		noteA1 = 21,
		noteASharp1 = 22,
		noteB1 = 23,
		noteC2 = 24,
		noteCSharp2 = 25,
		noteD2 = 26,
		noteDSharp2 = 27,
		noteE2 = 28,
		noteF2 = 29,
		noteFSharp2 = 30,
		noteG2 = 31,
		noteGSharp2 = 32,
		noteA2 = 33,
		noteASharp2 = 34,
		noteB2 = 35,
		noteC3 = 36,
		noteCSharp3 = 37,
		noteD3 = 38,
		noteDSharp3 = 39,
		noteE3 = 40,
		noteF3 = 41,
		noteFSharp3 = 42,
		noteG3 = 43,
		noteGSharp3 = 44,
		noteA3 = 45,
		noteASharp3 = 46,
		noteB3 = 47,
		noteC4 = 48,
		noteCSharp4 = 49,
		noteD4 = 50,
		noteDSharp4 = 51,
		noteE4 = 52,
		noteF4 = 53,
		noteFSharp4 = 54,
		noteG4 = 55,
		noteGSharp4 = 56,
		noteA4 = 57,
		noteASharp4 = 58,
		noteB4 = 59,
		noteC5 = 60,
		noteCSharp5 = 61,
		noteD5 = 62,
		noteDSharp5 = 63,
		noteE5 = 64,
		noteF5 = 65,
		noteFSharp5 = 66,
		noteG5 = 67,
		noteGSharp5 = 68,
		noteA5 = 69,
		noteASharp5 = 70,
		noteB5 = 71,
		noteC6 = 72,
		noteCSharp6 = 73,
		noteD6 = 74,
		noteDSharp6 = 75,
		noteE6 = 76,
		noteF6 = 77,
		noteFSharp6 = 78,
		noteG6 = 79,
		noteGSharp6 = 80,
		noteA6 = 81,
		noteASharp6 = 82,
		noteB6 = 83,
		noteC7 = 84,
		noteCSharp7 = 85,
		noteD7 = 86,
		noteDSharp7 = 87,
		noteE7 = 88,
		noteF7 = 89,
		noteFSharp7 = 90,
		noteG7 = 91,
		noteGSharp7 = 92,
		noteA7 = 93,
		noteASharp7 = 94,
		noteB7 = 95,
		noteC8 = 96,
		noteCSharp8 = 97,
		noteD8 = 98,
		noteDSharp8 = 99,
		noteE8 = 100,
		noteF8 = 101,
		noteFSharp8 = 102,
		noteG8 = 103,
		noteGSharp8 = 104,
		noteA8 = 105,
		noteASharp8 = 106,
		noteB8 = 107,
		noteC9 = 108,
		noteCSharp9 = 109,
		noteD9 = 110,
		noteDSharp9 = 111,
		noteE9 = 112,
		noteF9 = 113,
		noteFSharp9 = 114,
		noteG9 = 115,
		noteGSharp9 = 116,
		noteA9 = 117,
		noteASharp9 = 118,
		noteB9 = 119,
		noteC10 = 120,
		noteCSharp10 = 121,
		noteD10 = 122,
		noteDSharp10 = 123,
		noteE10 = 124,
		noteF10 = 125,
		noteFSharp10 = 126,
		noteG10 = 127,
		noteMax = noteG10,
		noteInvalid
	};

	enum class PlayState : uint8_t
	{
		Stopped = 0,
		Paused,
		Playing,
	};

	enum class PatternMode : bool
	{
		Edit = 0,
		Play = 1
	};

	enum class PlayControl : uint8_t
	{
		Stop = 0,
		Pause,
		Play,
	};

	enum class MuteControl : uint8_t
	{
		Muted = 0,
		Unmuted = 1,
	};

	enum class SceneAdvanceMode : uint8_t
	{
		SceneAdvanceAuto,
		SceneAdvanceManual,
	};

	enum class FTSRootNote : int8_t
	{
		NoteMin = (int8_t)Note::noteC0,
		NoteC = NoteMin,
		NoteCSharp,
		NoteD,
		NoteDSharp,
		NoteE,
		NoteF,
		NoteFSharp,
		NoteG,
		NoteGSharp,
		NoteA,
		NoteASharp,
		NoteB,
		NoteMax = NoteB
	};
	enum class ScaleList : int8_t
	{
		ScaleMin,
		ScaleAllNotes,
		ScaleMajor,
		ScaleAeolian,
		ScaleHarmonicMinor,
		ScaleMelodicMinor,
		ScaleBlues1,
		ScaleMinorPentatonic,
		ScaleMixoLydian,
		ScaleLydianDiminished,
		ScaleBlues2,
		ScaleAugmented,
		ScaleArabian,
		ScaleBalinese,
		ScaleHungarian1,
		ScaleOriental,
		ScaleIndian1,
		ScaleChinese,
		ScaleHungarian2,
		ScaleJapanese1,
		ScaleJapanese2,
		ScaleJapanese3,
		ScaleJapanese4,
		ScalePersian,
		ScaleDiminished,
		ScaleWholeNote,
		ScaleMongolian,
		ScaleAlgerian,
		ScaleByzantine,
		ScaleAncientChinese,
		ScaleIndian2,
		ScalePrometheus,
		ScaleUD1,
		ScaleUD2,
		ScaleUD3,
		ScaleUD4,
		ScaleUD5,
		ScaleUD6,
		ScaleUD7,
		ScaleUD8,
		ScaleUD9,
		ScaleUD10,
		ScaleMax
	};

	enum class TransposeFTSPriority : uint8_t
	{
		TransposePreFTS = 0,
		TransposePostFTS = 1
	};

	enum class TimeBase
	{
		//Base_64T,
		//Base_64,
		Base_32T = 0,
		Base_32,
		Base_16T,
		Base_16,
		Base_8T,
		Base_8,
		Base_4T,
		Base_4,
		Base_2T,
		Base_2,
		Base_1,
		Max
	};

	enum class TrackValueSaveType : uint8_t
	{
		TVSaveActiveToScene = 0,
		TVSaveActiveToSong = 1,
		TVSaveAllToScene = 2,
		TVSaveAllToSong = 3,
	};

	enum class TrackValueScope : uint8_t
	{
		TVAtSceneLevel = 0,
		TVAtSongLevel = 1,
	};

	enum class TrackValueType : uint8_t
	{
		TVTypeUnsassigned = 0,
		TVTypeMidiCC = 1,
		//TVTypeAltMidiCC = 2,// may not be the right place, check ui side also before re-adding
		TVTypePgmChange = 2,

		/*
		TVTypeTransposeNoteByPercent = 3,
		TVTypeTransposeNoteBySemitone = 4,
		TVTypeTransposeNoteByOctave = 5,
		TVTypeScaleVelocityByPercent = 6,
		TVTypeScaleLengthByPercent = 7,
		TVTypeFts = 8,
		TVTypeReich = 9,
		*/
	};

	enum class TrackMixerType : int8_t
	{
		Pan = 0,
		Volume = 1,
	};

	enum class MidiParam : paramId_t
	{
	};

	enum class TrackState : int8_t
	{
		TrackIdleState = 0,
		TrackPlayState = 1,
		TrackRecordState = 2,
	};

	enum class RecordEvents : uint16_t
	{
		None = 0,
		Note = 1,
		PitchBend = 2,
		AfterTouchCP = 4,
		AfterTouchPP = 8,
		ProgramChange = 16,
		MidiCC = 32
	};

	enum class AuxEventType : uint8_t
	{
		//voice control
		afterTouchPolyPressure = 0xA0,
		controlChange = 0xB0,
		programChange = 0xC0,
		afterTouchChannelPressure = 0xD0,
		pitchBend = 0xE0,

/* Enable selectevely when we start using
		//mode control
		resetAllControllers = 6,
		localControl = 7,
		allNoteOff = 8,
		omniModeOff = 9,
		omniModeOn = 10,
		polyphonicModeOff = 11,
		polyphonicModeOn = 12,

		//system control
		timingClock = 13,
		startSequence = 14,
		continueSequence = 15,
		stopSequence = 16,
		activeSensing = 17,
		systemReset = 18,

		//Special CCs
		programChangeBankSelectMSB,
		programChangeBankSelectLSB,

		rpnSelectLSB,
		rpnSelectMSB,

		nrpnSelectLSB,
		nrpnSelectMSB,

		pnSetValueLSB,
		pnSetValueMSB,

		pnIncrementValue,
		pnDecrementValue,
*/
	};

	enum class SongMenuSettings : uint8_t
	{
		autoSave,
		accidentals,
		soloMode,
		fillModeActive,
		fillModeAuto,
		fillModeMute,
		fillModeOff,
		fillReleaseKey,
		fillReleaseBar,
		fillReleaseScene,
	};

	enum class BRayControllSetting : int8_t
	{
		none = 0,
		ccControl,
		pitchBendControl,
		afterTouchControl,
		velocityControl,
		brayControlSettingMax,
	};
}

enum class MuteHoldType : int8_t
{
	Off = 0,
	Next = 1,
	Lock = 2
};

enum class FillModeType : int8_t
{
	automatic = 0,
	active = 1,
	mute = 2,
	noFill = 3
};

enum class FillReleaseMode : int8_t
{
	key = 0,
	bar = 1,
	scene = 2
};

enum class CvLaw : int8_t
{
	voltsPerOctave = 0,
	hertzPerVolt = 1,
	envelope = 2,
};

enum class CvGateType : int8_t
{
	normal = 0,
	reTrigger = 1,
};

enum class CvSlideType : int8_t
{
	off = 0,
	linear = 1,
	exponential = 2,
};

enum class GateType : int8_t
{
	normal = 0,
	sTrigger = 1,
};

enum class CVPeakVoltage : int8_t
{
	volt5 = 5,
	volt6 = 6,
	volt7 = 7,
	volt8 = 8,
	volt9 = 9,
	volt10 = 10,
	volt11 = 11,
	volt12 = 12,
};

enum class gatePeakVoltage : int8_t
{
	volt5 = 5,
	volt6 = 10,
};

struct CV
{
	int8_t portNum = 0;
	int8_t channelNum = 0;

	CvLaw law = CvLaw::hertzPerVolt;

	CvGateType gateType = CvGateType::normal;

	CvSlideType slideType = CvSlideType::off;
	int32_t slideTime = 0;

	int8_t noteRangeStart = 0;
	int8_t noteRangeEnd = 0;
	int8_t notePercent = 0;

	int8_t velocityPercent = 0;

	int8_t ccNum = 0;;//add in ui
	int8_t ccPercent = 0;

	int16_t pitchBend = 0;
	CVPeakVoltage voltage = CVPeakVoltage::volt5;

	int8_t EnvAttack = 0;
	int8_t EnvAttackMin = 0;
	int8_t EnvAttackMax = 0;
	std::string EnvAttackName;
	int8_t EnvDecay = 0;
	int8_t EnvDecayMin = 0;
	int8_t EnvDecayMax = 0;
	std::string EnvDecayName;
	int8_t EnvSustain = 0;
	int8_t EnvSustainMin = 0;
	int8_t EnvSustainMax = 0;
	std::string EnvSustainName;
	int8_t EnvRelease = 0;
	int8_t EnvReleaseMin = 0;
	int8_t EnvReleaseMax = 0;
	std::string EnvReleaseName;
	int8_t EnvAmount = 0;
	int8_t EnvAmountMin = 0;
	int8_t EnvAmountMax = 0;
	std::string EnvAmountName;
	int8_t EnvVelocity = 0;
	int8_t EnvVelocityMin = 0;
	int8_t EnvVelocityMax = 0;
	uint8_t cvNoteType = 0;
	std::string EnvVelocityName;
};

struct Gate
{
	int8_t portNum = 0;
	int8_t channelNum = 0;//add in ui
	CvGateType mode = CvGateType::normal;
	GateType type = GateType::normal;
	gatePeakVoltage output = gatePeakVoltage::volt5;
};

enum class SoftThruType : int8_t
{
	off = 0,
	loopBack = 1,
	//editTrack = 2,
	differentPortSameChannel = 2,
	differentChannelDifferentPort = 3,
	//internalSound = 5,
};

struct MidiConfig
{
	int8_t sourcePort = 0;
	SoftThruType type = SoftThruType::off;
	int8_t thruFTSEnabled = false;
	int8_t destPort = 0;
	int8_t destChannel = 0;
	int8_t midiClkTxEnabled = false;
};

struct UDS
{
	std::vector <int8_t> udsNoteList;
	std::string udsName;
};

struct mainInfoData
{
	int32_t activeSongId = 0;
	float masterVolume   = 0.7f;
};

//same order maintained in UI
enum class InstrumentPorts: uint8_t
{
	midiPort1 = 0,
	midiPort2,
	midiPort3,
	midiPort4,
	midiPort5,
	cvPort1,
	cvPort2,
	cvPort3,
	cvPort4,
	cvPort5,
	cvPort6,
	cvPort7,
	cvPort8,
	usbMidiPort1,
	usbMidiPort2,
	usbMidiPort3,
	usbMidiPort4,
	usbMidiPort5,
	usbMidiPort6,
	//usbHostMidiPort,

	MaxMidiPort = 127,
};

enum class SettingsTransposeControlCnfgParam : uint8_t
{
	paramall = 0,
	paramTransposeControlStatus,
	paramTransposeControlLatch,
	paramTransposeControlMidiPort,
	paramTransposeControlMidiChannel,
	paramTransposeControlCenterPitch,
	paramTransposeControlCnfgSave,
	paramTransposeControlDataLocation,
	paramUndoCmd,
	paramUndoListCmd,
	paramSaveAll,
	paramTransposeControlCnfgParamMax
};

enum class SyncSource : int8_t
{
	internal = 0,
	inputClock,
	midi,
	usbMidi,
};

enum class InsertPosition : int8_t
{
	listEnd = 0,
	nextToActiveRow
};

enum class MidiCCMsgID : int {
	BankSelect = 0,
	ModulationWheel = 1,
	BreathController = 2,
	FootPedal = 4,
	Portamento_GlideTime = 5,
	DataEntry = 6,
	MIDIVolume = 7,
	StereoBalance = 8,
	PanPosition_Balance = 10,
	ExpressionPedal = 11,
	EffectController1 = 12,
	EffectController2 = 13,
	Slider_GeneralPurposeController1 = 16,
	Slider_GeneralPurposeController2 = 17,
	Slider_GeneralPurposeController3 = 18,
	Slider_GeneralPurposeController4 = 19,
	BankSelect_LSB = 32,
	ModulationWheel_LSB = 33,
	BreathController_LSB = 34,
	FootPedal_LSB = 36,
	Portamento_GlideTime_LSB = 37,
	DataEntry_LSB = 38,
	Volume = 39,
	Balance = 40,
	PanPosition = 42,
	Expression = 43,
	EffectControl1 = 44,
	EffectControl2 = 45,
	SustainPedalOn_Off = 64,
	Portamento_GlideOn_OffSwitch = 65,
	SostenutoOn_OffSwitch = 66,
	SoftPedalOn_OffSwitch = 67,
	LegatoOn_OffSwitch = 68,
	HoldPedal2 = 69,
	SoundController1 = 70,
	SoundController2_FilterResonance = 71,
	SoundController3_AmpEnvelopeDecay = 72,
	SoundController4_AmpEnvelopeAttack = 73,
	SoundController5_FilterCutoff = 74,
	SoundController6 = 75,
	SoundController7 = 76,
	SoundController8 = 77,
	SoundController9 = 78,
	SoundController10 = 79,
	GeneralPurpose1 = 80,
	GeneralPurpose2 = 81,
	GeneralPurpose3 = 82,
	GeneralPurpose4 = 83,
	Effect1Amount = 91,
	Effect2Amount = 92,
	Effect3Amount = 93,
	Effect4Amount = 94,
	Effect5Amount = 95,
	DataBoundIncrement = 96,
	DataBoundDecrement = 97,
	NRPNLSB = 98,
	NRPNMSB = 99,
	RPNLSB = 100,
	RPNMSB = 101,
	ChannelMute_SoundOff = 120,
	ResetAllControllers = 121,
	LocalKeyboardOn_OffSwitch = 122,
	AllMIDINotesOFF = 123,
	OMNIModeOFF = 124,
	OMNIModeON = 125,
	MonoMode = 126,
	PolyMode = 127,
};

enum class CvNoteType : uint8_t
{
	LowNoteSet = 0,
	HighNoteSet = 1,
	LastNoteSet = 2,
};

//Track Values
// program change is configured at 2 Instrument level and at Track Level also
enum class TvLocation : int8_t
{
	InstrumentLevel,
	TrackLevel,
};

struct ScaleParams
{
	std::string name;
	std::vector<int8_t> scaleIntervalList;
};


enum class SongInfoListUpdateParam : int8_t
{
	Name,
	Scene,
};

enum class EngineDataFolderId : uint8_t
{
	AllDataInternalStorage = 0,
	SongsInternalStorage,
	PatternInternalStorage,
	InstrumentInternalStorage,
	SettingsInternalStorage,

	AllDataExternalStorage,
	SongsExternalStorage,
	PatternExternalStorage,
	InstrumentExternalStorage,
	SettingsExternalStorage,
	MaxValue,
};

enum class InternalAudioCC : uint8_t
{
	RELEASE = 0,
	FILTER_CUTOFF,
	RESONANCE,
	VOLUME,
	REVERB,
	CHORUS,
	EQ_LOW,
	EQ_HIGH,
};

enum class SwUdateSource_t
{
	PC,
	INTERNAL,
	USBDISK,
	OTA,
	NONE
};

enum class FwStatus_t
{
	INVALID_FW = -3,
	FW_NOT_PRESENT = -2,
	OLD_FW = -1,
	SAME_FW = 0,
	NEW_FW = 1,
};

enum class CalibStep
{
	INVALID_CALIB_STEP = -1,
	CV_CALIB_STEP1,
	CV_CALIB_STEP2,
	CV_CALIB_STEP3,
	CV_CALIB_STEP4,
	CV_CALIB_STEP5,
	CV_CALIB_STEP6,
	CV_CALIB_STEP7,
	CV_CALIB_STEP8,
	CV_CALIB_STEP9,
	CV_CALIB_STEP10,
};

const float FilterFreqMap[128] = { 20.0f, 23.00f, 24.5625f, 25.5f, 26.75f, 37.7344f, 45.2813f, 52.8282f, 60.375f,
							69.75f, 79.8438f, 90.2344f, 99.5625f, 107.8594f, 116.1563f, 124.4532f, 134.5f, 146.8907f, 160.0313f,
							173.375f, 187.1875f, 201.7969f, 216.5625f, 232.1563f, 247.875f, 264.0625f, 278.6875f, 293.2032f, 308.4375f,
							323.5313f, 339.375f, 358.9219f, 379.5f, 401.1563f, 422.875f, 445.1563f, 459.5625f, 467.7032f, 475.5938f,
							483.2344f, 490.625f, 497.7657f, 518.4375f, 552.2813f, 587.125f, 622.9688f, 659.8125f, 697.6563f, 727.5f,
							749.5469f, 771.875f, 794.4844f, 816.5625f, 839.7188f, 869.9063f, 901.4844f, 934.5f, 967.2188f, 1000.5f,
							1048.1719f, 1105.3125f, 1163.7657f, 1223.5313f, 1283.625f, 1319.0f, 1354.8438f, 1391.1563f, 1427.9375f, 1464.125f,
							1501.8282f, 1546.5625f, 1600.8282f, 1656.0f, 1709.7969f, 1755.1875f, 1801.1719f, 1847.75f, 1894.9219f, 1943.9063f,
							2055.2344f, 2170.0f, 2285.7188f, 2403.625f, 2509.4532f, 2614.5f, 2721.3282f, 2829.9375f, 2940.3282f, 3052.5f,
							3166.4532f, 3282.1875f, 3413.9219f, 3566.4375f, 3721.4532f, 3977.375f, 4237.8907f, 4503.0f, 4772.7032f, 5062.3125f,
							5378.4844f, 5701.5625f, 6028.4375f, 6406.875f, 6791.5625f, 7193.875f, 7612.5f, 8037.7813f, 8469.7188f, 8984.25f,
							9510.25f, 10049.5313f, 10602.2344f, 11163.25f, 11734.3438f, 12406.4063f, 13099.2188f, 13934.5f, 14880.9375f, 15930.0f,
							16734.375f, 17000.0f/*17373.75*/, 17749.1875f, 18128.4375f, 18511.5f, 18898.375f, 19289.0625f, 19683.5625f, 20000.0f };

struct MemoryStatus {
	char total[15];
	char available[15];
	char used[15];
};

struct InternalStorageStatus {
	char totalSpace[15];
	char availableSpace[15];
	char usedSpace[15];
};

struct USBStorageStatus {
	char totalSpace[25];
	char availableSpace[25];
};

struct DeviceInfo {
	char serialNumber[32];
	char firmwareVersion[16];
	char firmwareBuildDate[16];
	struct MemoryStatus memoryStatus;
	struct InternalStorageStatus internalStorageStatus;
	struct USBStorageStatus usbStorageStatus;
};

typedef enum
{
	U_INIT,
	U_DOWNLOAD,
	U_DONE,
	U_SUCCESS,
	U_FAILED,
	U_TIMEOUT
} USB_STATUS;

typedef enum
{
	INVALID_STATE = -10,
	DEV_OPERATION_FAILED = -3,
	FW_UPDATE_IMX_FAILED,
	FW_UPDATE_MCU_FAILED,
	DEV_OP_ONGOING,
	FW_UPDATE_ONGOING = 1,
	FACTORY_RESET_ONGOING,
	FW_UPDATE_COMPLETED,
	FACTORY_RESET_COMPLETED,
	PLAYTRIBE_OP_STARTED,
	PLAYTRIBE_OP_COMPLETED,
} DEV_STATUS;

typedef struct usb_msg
{
	USB_STATUS command_status;
	size_t     totalfilesize;
	size_t     downloadfilesize;
	double     percentage;

} usbmsg_t;

typedef std::function<void(usbmsg_t&)>      UsbStatus;

enum class PatternChangeParameter : uint8_t
{
	//PATTERN LEVEL
	initPattern,
	numBars, /* uint8_t */
	direction, /* enum */
	timeBase, /*  enum */
	patternLastStep, /* uint16_t */
	firstStep, /* uint16_t */
	barLoopStatus, /* bool */
	barLoopStart, /* uint8_t */
	barLoopEnd, /* uint8_t */

	//BAR LEVEL
	barStepCount, /* uint8_t */
	barRepeatCount, /* uint8_t */
	barGbarSyncState, /* bool */
	barTransposeDefeat, /* bool */
	barTranspose, /* uint8_t */

	//STEP LEVEL
	stepSkipState, /* bool */
	stepTieState, /* bool */
	stepTransposeDefeatState, /* bool */
	stepOnState, /* bool */

	nodeOffset, /* uint8_t */
	nodeLength, /* uint8_t */

	addRemoveNote,
	addRemoveAux,

	addRecordEvents,
};

enum MidiEventType
{
	invalid,
	noteOff=0x80,
	noteOn=0x90,
	keyPressure=0xA0,
	controlChange=0xB0,
	programChange=0xC0,
	channelPressure=0xD0,
	pitchBend=0xE0
};

enum PatternEventType
{
	note,
	command,
};


struct MidiEvent
{
	MidiEventType type;//note on/note off/ cc / pgm ch / cha pres / key pres /pitch b
	uint8_t data1;
	uint8_t data2;

	MidiEvent() : type(MidiEventType::invalid), data1(0), data2(0) {}

	MidiEvent(MidiEventType type, uint8_t data1, uint8_t data2)
		: type(type), data1(data1), data2(data2) {}
	
	MidiEvent(const MidiEvent& other)
		: type(other.type), data1(other.data1), data2(other.data2) {}
	
};

struct NoteInfo
{

};

struct AuxInfo
{

};

enum SYSTEM_STATE
{
	IDLE,
	PLAY,
	STEP_RECORD,
	LIVE_RECORD,
	PAUSE,
	RECORD_PAUSE,
};

enum KEY_PRESSED
{
	PLAY_KEY,
	RECORD_KEY,
	STOP_KEY,
	PAUSE_KEY,
	RECORD_STOP_KEY,
};

enum IgniteDrumPresetIndex
{
	StandardDrum = 128,
	RoomDrum,
	PowerDrum,
	ElectronicDrum,
	TR808Drum,
	JazzDrum,
	BrushDrum,
	OrchestraDrum,
	SFXDrum,
	CM63_32SetDrum,
};

enum tsfDrumPresetNumber
{
	tsfStandardDrum = 0,
	tsfRoomDrum = 8,
	tsfPowerDrum = 16,
	tsfElectronicDrum = 24,
	tsfTR808Drum = 25,
	tsfJazzDrum = 32,
	tsfBrushDrum = 40,
	tsfOrchestraDrum = 48,
	tsfSFXDrum = 56,
	tsfCM63_32SetDrum = 127,
};

struct TrackAndPosition
{
	void* trackPtr;
	int16_t sceneBarId;
	int16_t sceneStepId;
};

struct VariableMinMax
{
	int16_t min = 0;
	int16_t max = 0;
};
