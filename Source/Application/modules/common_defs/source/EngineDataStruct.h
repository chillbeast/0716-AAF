#pragma once
#include <stdint.h>
#include <iostream>
#include <iterator>
#include <list>
#include <atomic>
#include <tuple>
#include <map>
#include "Common/Modules/Instrument.h"
#include "Common/Modules/Track.h"
#include "Common/Modules/Scene.h"

namespace Ignite
{
	class Track;
	class Scene;
	struct Aux;

	struct UsedByData
	{
		bool isFillPattern = false;
		std::string songName = " ";
		std::string sceneName = " ";
		std::string trackName = " ";
	};

	struct TVMidiCCParams
	{
		int8_t num; //0 - 127, -1 is not set
		int8_t initValue; // 0 - 127, -1 is not set
		int8_t maxValue;
		int8_t minValue;
		float value;
	};

	struct PgmChangeParams
	{
		int8_t bankLSB; //0 - 127
		int8_t bankMSB; //0 -127
		int8_t midiPgmNum; //0 - 127
	};

	struct TVScaleVelocityByPercentParams
	{
		int16_t percentage = 0; // 0 - 255%
		int8_t refVelocity = 0; // 0 - 127
	};

	struct TVScaleLengthByPercent
	{
		int16_t percentage =0; // 0 - 255%
	};

	struct TVTransposeNoteByPercent
	{
		int16_t percentage = 0; // 0 - 255%
		int8_t refNote = 0; //0 - 127
	};

	struct TVTransposeNoteBySemitone
	{
		int8_t semiTone = 0; // -12 - +12
	};

	struct TVTransposeNoteByOctave
	{
		int8_t octave = 0; //-4 - +4
	};

	struct TVFTS
	{
		FTSRootNote rootNote = FTSRootNote::NoteMin;
		ScaleList scale = ScaleList::ScaleMin;
	};

	//int8_t reichValue;//-8 - +8
	struct TrackValue
	{
		int8_t knobId;//0 - 7
		std::string name;
		TrackValueType paramType;
		TrackValueScope savedAt;

		//bool valueModified;//decide when to send it to instrument, apart from start of song/scene

		union
		{
			TVMidiCCParams tvMidiCC;
			PgmChangeParams tvPgmChange;
			/*
			TVScaleVelocityByPercentParams tvScaleVelocityByPercent;
			TVScaleLengthByPercent tvScaleLengthByPercent;
			TVTransposeNoteByPercent tvTranposeNoteByPercent;
			TVTransposeNoteBySemitone tvTransposeNoteBySemitone;
			TVTransposeNoteByOctave tvTransposeNoteByOctave;
			TVFTS tvFTS;
			*/
		} param;
	};

	struct TrackMixer
	{
		int8_t volume = 0;
		int8_t pan = 0;
	};

	struct InstrumentData
	{
		std::string instrumentName = " ";
		std::string instrumentDispName = " ";
		int8_t midiPort = 0;
		int8_t midiChannel = 0;
		uint8_t defaultNote = 0;
		int internalSoundPgm = 0;
		bool internalSound = true;
		bool multiTimbralStatus = false;
		bool polySpreadStatus = false;
		bool transposeDefeatStatus = false;
		bool ftsDefeatStatus = false;
		bool editTrackThruStatus = false;
		bool cc0BankMStatus = false;
		bool cc0BankLStatus = false;
		bool displayNoteNum = false;
		bool preSendPgmChange = false;
		uint32_t UsedByCount = 0;
		uint8_t tvCount = 0;
		std::list <UsedByData> usedByList;
		std::unordered_map <int8_t, TrackValue> tv;//knb_id, knob_data
		TrackMixer tm;
		int32_t id =0 ;
		PgmChangeParams pgmChangeData;
        std::string soundFontPath; // Empty uses the desktop default bank.
	};

	class Pattern;
	struct TrackData
	{
		bool trackSolo = false;
		bool trackRecordEnable = false;
		bool trackMute = false;
		uint8_t trackId = 0;
		std::string trackName = " ";
		std::string trackRxInstrumentName = " ";
		std::string trackRxInstrumentDispName = " ";
		std::string mainPatternName = " ";
		std::string mainPatternDispName = " ";
		std::string fillPatternName = " ";
		std::string fillPatternDispName = " ";
		Instrument* trackRxInstrument = nullptr;
		Instrument* trackTxInstrument = nullptr;
		std::string trackTxInstrumentName = " ";
		std::string trackTxInstrumentDispName = " ";
		Pattern* mainPattern = nullptr;
		Pattern* fillPattern = nullptr;
		uint8_t tvTrackDataCount = 0;
		std::unordered_map<int8_t, TrackValue> tvTrackData; //knobid, midi cc/pgm change data struct
	};

	struct SceneData
	{
		bool transposeAfterFTS = false;
		int8_t sceneSwing = DEFAULT_SONG_SWING;
		int8_t gBarLenInSteps = 16; //num of Steps
		uint16_t numBars = 0; //len
		uint8_t mode = 0; //man, auto(def) 
		uint8_t scaleFTS = 0;
		uint8_t noteFTS = 0;
		int8_t transposeValue = 0; //semitones
		int8_t numberOfTracks = 0;
		uint32_t sceneId =0 ;
		uint64_t trackMuteStatus = 0x0;
		float sceneTempo = DEFAULT_SONG_TEMPO;
		std::string sceneName = " ";
		std::list <Track> trackList;
	};

	struct SongData
	{
		uint8_t gTrackCount = 0;
		uint8_t songSwing = DEFAULT_SONG_SWING;
		int32_t numberOfScenes = 0;
		int32_t songId = 0;
		float songTempo = DEFAULT_SONG_TEMPO;
		bool isFillMode = false;
		bool applySongTempoToScenes = false;
		bool applySongSwingToScenes = false;
		std::string songName = " ";
		std::list <Scene> sceneList;
	};
	struct SettingsMidiCnfgData
	{
		std::unordered_map <int8_t, MidiConfig>  MidiCnfg;
		int8_t DataLocation = 0;
	};

	struct SettingsCvioCnfgData
	{
		std::unordered_map <int8_t, CV> cvConfig;
		std::unordered_map <int8_t, Gate> gateConfig;
		int8_t DataLocation = 0;
	};

	struct CalibrationData
	{
		std::array<float, 10> calibOffset = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		int8_t DataLocation = 0;
	};

	struct SettingsGeneralCnfgData
	{
		int8_t DeviceCnfgUSBParam = 0;
		int8_t DeviceCnfgSafeStopParam = 0;
		int8_t DeviceCnfgBrightnessParam = 80;
		bool   DeviceCnfgAutoSaveAll = false;
		bool   InstrumentAutoSave = false;
		bool   WifiState = false;
		std::string LastWifiConSsid = " ";
		uint8_t auditioningMaxPort = AUDITIONING_MAX_PORT;
		int auditioningPortPreset[AUDITIONING_MAX_PORT] = { -1, };

	};

	struct SettingsSongCnfgData
	{
		int8_t SongAutoSaveParam = 0;
		int8_t SongNoteNameDispParam = 0;
		int8_t SongSoloModeParam = 0;
		int8_t SongFillModeParam = 0;
		int8_t SongFillActiveCnfgParam = 0;
		int8_t SongFillKeyCnfgParam = 0;
		int8_t SongTapTempoParam = 3;
		int8_t DataLocation = 0;
		int8_t syncSource = 0;
		int8_t syncSourcePort = 0;
		int8_t SongInsertPosition = 0;
	};

	struct SettingsSceneCnfgData
	{
		int8_t SceneAutoSaveParam = 0;
		int8_t SceneTapTempoParam = 3;
		int8_t DataLocation = 0;
		std::vector <UDS> udsConfig;
		int8_t activeUds = 0;
		int8_t udsCount = 0;
		int8_t SceneInsertPosition = 0;
		int8_t SceneSoloModeParam = 0;
		int8_t SceneFillModeParam = 0;
		int8_t SceneFillActiveCnfgParam = 0;
		int8_t SceneFillKeyCnfgParam = 0;
	};
	struct SettingsTrackCnfgData
	{
		int8_t TrackAutoSaveParam = 0;
		int8_t DataLocation = 0;
	};
	struct SettingsTrackInstrumentMixerData
	{
		uint8_t MixerTrack = 0;
		uint8_t MixerPanValue = 0;
		uint8_t MixerVolumeValue = 0;
		int8_t DataLocation = 0;
	};

	struct SettingsPatternCnfgData
	{
		int8_t PatternAutoSaveParam = 0;
		bool PatternBarFollowState = true;
		int8_t DataLocation = 0;
		int16_t patternDefaultNote = 0;
		int8_t patternInsertPosition = 0;
	};

	struct SettingsRecordCnfgData
	{
		int8_t RecordQuantizationParam = 0;
		int8_t RecordMthryParam = 0;
		int8_t RecordFTSParam = 0;
		int8_t RecordAutoAddBarParam = 0;
		int8_t RecordBarOverWriteParam = 0;
		int8_t RecordNumOfBarsParam = 16;
		int8_t RecordNoteEventParam = 1;
		int8_t RecordMidiCCEventParam = 1;
		int8_t RecordAfterTouchEventParam = 1;
		int8_t RecordPitchBendEventParam = 1;
		int8_t RecordPolyPEventParam = 1;
		int8_t RecordPgmChangeEventParam = 1;
		int8_t DataLocation = 0;
	};
	struct SettingsBRayCnfgData
	{
		int8_t brayPowerStatus = 0;
		int8_t brayCcNum = -1;
		BRayControllSetting brayControlledParam = BRayControllSetting::none;
		int8_t DataLocation = 0;
	};
	struct SettingsRemoteControlCnfgData
	{
		int8_t RemoteControlStatus = 0;
		int8_t RemoteControlLatch = 0;
		int8_t RemoteControlMidiPort = 0;
		int8_t RemoteControlMidiChannel = 0;
		int8_t RemoteControlSceneChange = 0;
		int8_t RemoteControlFontChange = 0;
		int8_t DataLocation = 0;
	};
	struct SettingsMidiMachineControlCnfgData
	{
		int8_t mmcReceiveStatus = 0;
		int8_t mmcSendStatus = 0;
		int8_t mmcDeviceId = 0;
		int8_t mmcDeviceIdAll = 0;
		int8_t mmcInputPort = 0;
		int8_t mmcOutputPort = 0;
		int8_t DataLocation = 0;
	};
	struct SettingsUdsCnfgData
	{
		int8_t DataLocation = 0;
		std::vector <UDS> udsConfig;
		int8_t udsCount = 0;
	};
	struct SettingsMidiClkCnfgData
	{
		int8_t midiClkInPort = IGNITE_CLK_PORT_INTERNAL_CLK; //default to internal clock
		uint32_t midiClkOutPorts; //bit map for the 5 din midi + 6 usb dev midi + 6 usb host midi + 1 sync out
		int8_t MidiClkInputClkRate = (int8_t)SettingsMidiClkCnfgInputClkRate::inClkRate_48PPQ;
		int8_t MidiClkOutPutClkRate = (int8_t)SettingsMidiClkCnfgOutputClkRate::outClkRate_48PPQ;
		int8_t SyncTriggerForIn = 0;
		int8_t SyncTriggerForOut = 0;
		int8_t DataLocation = 0;
	};

	enum class SculptMode : int8_t
	{
		none = 0,
		relative,
		absolute,
		randomRelative,
		randomAbsolute,
	};

	enum class SculptParameterSelect : int8_t
	{
		none = 0,
		velocity = 1,
		/*lengthInSteps = 2,
		lengthInTicks = 4,*/
		note = 2,
		//delay = 8,
		allParams = 0xF
	};

	struct SettingsSculptModeCnfgData
	{
		bool sculptButtonOnState = false;
		SculptMode sculptMode = SculptMode::absolute;
		SculptParameterSelect sculptParameter = SculptParameterSelect::velocity;

		int8_t sculptRelativeVelocity = 0;
		/*int8_t sculptRelativeLengthSteps = 0;
		int8_t sculptRelativeLengthTicks = 0;*/
		int8_t sculptRelativeNote = 0;
		//int8_t sculptRelativeDelay = 0;

		int8_t sculptAbsoluteVelocity = 0;
		/*int8_t sculptAbsoluteLengthSteps = 0;
		int8_t sculptAbsoluteLengthTicks = 0;*/
		int8_t sculptAbsoluteNote = 0;
		//int8_t sculptAbsoluteDelay = 0;

		int8_t sculptRandomRelativeVelocityStart = 0;
		int8_t sculptRandomRelativeVelocityEnd = 127;
		/*int8_t sculptRandomRelativeLengthStepStart = 0;
		int8_t sculptRandomRelativeLengthStepEnd = 0;
		int8_t sculptRandomRelativeLengthTickStart = 0;
		int8_t sculptRandomRelativeLengthTickEnd = 0;*/
		int8_t sculptRandomRelativeNoteStart = 0;
		int8_t sculptRandomRelativeNoteEnd = 127;
		//int8_t sculptRandomRelativeDelayStart = 0;
		//int8_t sculptRandomRelativeDelayEnd = 0;

		int8_t sculptRandomAbsoluteVelocityStart = 0;
		int8_t sculptRandomAbsoluteVelocityEnd = 127;
		/*int8_t sculptRandomAbsoluteLengthStepStart = 0;
		int8_t sculptRandomAbsoluteLengthStepEnd = 0;
		int8_t sculptRandomAbsoluteLengthTickStart = 0;
		int8_t sculptRandomAbsoluteLengthTickEnd = 0;*/
		int8_t sculptRandomAbsoluteNoteStart = 0;
		int8_t sculptRandomAbsoluteNoteEnd = 127;
		//int8_t sculptRandomAbsoluteDelayStart = 0;
		//int8_t sculptRandomAbsoluteDelayEnd = 0;
		int8_t DataLocation = 0;
	};


	struct TransposeControlCnfgData
	{
		bool transposeControlStatus = false;
		bool transposeControlLatch = false;
		int8_t transposeControlMidiPort = 0;
		int8_t transposeControlMidiChannel = 0;
		int8_t transposeControlCenterPitch = 64;
		int8_t dataLocation = 0;

		//internal data, not shared with UI
		int8_t inputNote = transposeControlCenterPitch;
		int8_t transposeDelta = 0;
		std::atomic <bool> tranposeUpdatedFlag;
	};

	//Used for data sharing between SeqManager and Track
	struct PatternBarStepInfo
	{
		std::string patternName;
		int8_t barId;
		int16_t stepId;
	};

	//Used for data sharing between SeqManager and Track
	struct PatternBarInfo
	{
		std::string patternName;
		int8_t barId;
		uint8_t repeatCount;
		bool gBarSyncState;
	};

	struct BarLoopRange
	{
		int8_t startBarId;
		int8_t endBarId;
	};

	//Used for data sharing between SeqManager and Track
	struct PatternInfo
	{
		std::string name;
		TimeBase tb;
		std::tuple <bool, BarLoopRange> barloop;
		std::map <int8_t, PatternBarInfo> barInfoList;
	};
}/* namespace Ignite */
