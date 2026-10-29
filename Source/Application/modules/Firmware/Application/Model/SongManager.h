#pragma once
#include <stdint.h>
#include <string>
#include <mutex>
#include <map>
#include <unordered_map>
#include <assert.h>
#include <random>
#include <algorithm>
#include <atomic>
#include <vector>
#include <tuple>
#include <utility>

#include "Common/Os/CrossPlatform/CThread.h"
#include "Common/Modules/CSong.h"
#include "Common/Modules/ISyncListener.h"
//#include <juce_audio_basics/juce_audio_basics.h>
//#include <juce_audio_devices/juce_audio_devices.h>
//#include <juce_dsp/juce_dsp.h>
//#include "MidiTransport.h"
#include "Firmware/Application/Model/ModelParams.h"
#include "CSoundEngine.h"
#include "module.h"
#include "Common/Modules/Scene.h"
#include "common_defs/source/EngineDataStruct.h"
//#include "Firmware/Application/Model/EngineFileHandler.h"
#include "idevice.h"
#include "InstrumentManager.h"
#include "PatternManager.h"
#include "FixedAudioBlockAdapter.h"

#ifdef __linux__

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#endif



// Enable TEST_ENABLE_SAMPLE to load and test with sample file :
// ***NOTE**: this required to enable "juce_audio_formats" in Cmakelist under Application\apps\ignite
//#define TEST_ENABLE_SAMPLE

namespace Ignite
{
	class IgniteEngine;
	class CSong;
	class Scene;
	class AudioEffectsUtils;
	struct SongData;
	using namespace juce;

	class IgniteMidi : MidiInputCallback
	{
	public:
		IgniteMidi() {};
		~IgniteMidi() = default;

		void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message);
		std::shared_ptr <MidiInput> inputInterface = nullptr;
		std::shared_ptr <MidiOutput> outputInterface = nullptr;

		std::string deviceName;
	};

	class SongManager : public Module, public juce::AudioIODeviceCallback, public juce::AudioSource,
		public IInstrumentListener, private Timer, public juce::Component
	{
		struct PendingMidiMsg {
			MidiMessage msg;
			uint8_t midiport;
			bool isInternalSound;
		};
	public:
		SongManager(IParamChangeListener* listener, IgniteEngine& model);
		SongManager(const SongManager&) = default;
		SongManager& operator= (const SongManager&) = delete;
		SongManager(SongManager&&) = delete;
		SongManager& operator= (SongManager&&) = delete;

		~SongManager();
		void shutdownDesktop();

		void init(void);

#ifdef TEST_ENABLE_SAMPLE
		static void readAudioFromFile();
#endif
		void addSong(CSong& newSong, bool sortScenes, bool reAssignSceneIds);
		void createAndAddSong(void); //call at create song page, then call set param for any parameer which are set by user
		void createSongWithRandomData (uint8_t NumOfSongs, uint8_t NumOfScene, uint8_t NumOfTracks, uint8_t numInstrument, uint8_t numPattern, uint8_t numPatternBars);
		void clearAllSongs (void);
		void clearAllSongsInstrumentPattern (void);
		void removeSong(void);
		//void insertSong (int32_t songid, CSong& newSong);
		void modifySongNameIfDuplicate(std::string& name);

		int32_t getSongCount(void) const;
		float getTempo(void);
		int8_t getSwing(void);
		std::string getName(void);
		//CSong* getSong (uint8_t index);
		//bool getFillMode(void);

		void setActiveSong (int32_t index);
		void setTempo(float val);
		void setSwing(uint8_t val);
		void setName(std::string name);

		void playSong(int32 sceneId);
		void pauseSong(void);
		void noteOn (uint8_t note, uint8_t velocity, uint8_t midiPort, int8_t midiChannel);
		void noteOff (uint8_t note, uint8_t velocity, uint8_t midiPort, int8_t midiChannel);
		bool applyFtsAndTranspose(uint8_t& note, bool isTransposeDefeat, bool isFtsDefeat);
		uint8_t getPlaySceneNumTracks(void);
		Scene* getPlayScene(void);
		void advancePlayStep(void);
		void advancePlayScene(void);
		//void setFillMode(bool);
		void setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode)override;
		void onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		void getParam(moduleId_t moduleSrc, paramId_t param)override;

		bool saveSongData(void);
		bool saveSong(void);
		bool saveSong(CSong& song);
		bool populateSongFromFileSystem(int32_t id);
		void SetDefaultUndoListData();
		void resetToDefaultParamValues (std::vector<SongParam> paramList);
		void initPlayScene(int32_t sceneId);

		std::vector<int8_t> generateScaleNotes(int8_t rootNote, int8_t scale);
		int8_t scaleNote(int8_t note);
		int8_t transposeNote(int8_t note);
		void publishSceneInfoToUi(void);
		CSong* getActiveSong(void);
		int32_t identifyNextPlayScene(void);
		void sortScenesInSong(CSong& song);
		void reAssignSequentialIdsForScenes(CSong& song);
		void reAssignSequentialIdsForScenesAndSave(CSong& song);
		void reAssignSequentialIdsForSongsAndSave (void);
		int32_t reAssignSequentialIdsForSongsAndSave (int32_t monitorIndex, bool IsDirectionUp);
		void pulse(uint16_t pulse);
		void clearLastPlayedScene(int32_t SceneId);
		void deInitSongPlaySettings(void);
		void stopSong();
		void sendTrackValuesAtSongLevel(void);
		void sendTrackValuesAtSceneLevel(void);
		void initRecord(void);
		void startRecord(const MidiMessage& message);
		void stopRecord(void);
		void saveSongInfo(SongData& songInfo);
		void saveSongInfo (int32_t id, std::string songName);
		void populateSongInfoListFromFileSystem(void);
		std::string getSongNameFromId(int32_t id);
		std::vector<std::string> getSongNameListSorted(int32_t start, int32_t count);
		void modifyTrackCount(int8_t newTrackCount);
		bool moveSong(int32_t curId, int32_t newId);
		bool moveActiveSong(int32_t curId, int32_t newId);
		bool moveNonActiveSong(int32_t curId, int32_t newId);
		bool copySong(std::string songName);
		void publishSceneBarInfoToUi(void);
		void setRecordEventFilter(uint16_t val);
		uint16_t getRecordEventFilter(void);
		void setRecordTrigger(bool val);
		TimeBase getSceneLargestTimeBase(Scene& scene);
		void setSceneTimeBaseTick (void);
		void checkAndApplyNewTimeBaseForScene (void);

		//bool getSceneBarChangedStatus(void);
		//void clearSceneBarChangedStatus(void);
		void sendMidiMessage (MidiMessage msg, int8_t midiport, bool isInternalSound, Track* track);
		void handleTVMidiMessageOnRecord(MidiMessage msg, int8_t midiport, bool isInternalSound, Track* track);
		//void setTrackSoloMode(bool state);//moved to scene settings
		//bool getTrackSoloMode(void);//moved to scene settings
		uint64_t getSceneAllTrackMuteStatus(int32_t sceneId);
		void setGlobalMuteHold(MuteHoldType val, int32_t sceneId);
		MuteHoldType getGlobalMuteHoldType(void);
		Scene* getSceneWithId(int32_t sceneId);
		Scene* findTheFirstSceneWithMuteHoldLock(void);
		void setFillModeSwitchOnState(bool state);
		void setFillModeType(int8_t type);
		void setFillModeRelease(int8_t releasetype);
		bool getFillModeSwitchOnState(void);
		int8_t getFillModeType(void);
		int8_t getFillModeRelease(void);
		bool getFillModeTriggerState(void);
		int32_t getCurrentPlaySceneId(void);
		int16_t getCurrentPlayingGBarId(void);
		void setGlobalSwing(bool state);
		void setGlobalTempo(bool state);
		bool getGlobalSwingFlag(void);
		bool getGlobalTempoFlag(void);

		void handleIncomingMcuMidiMessage (String msgPortName, const MidiMessage& message);
		void handleMcuSpecialKey(uint8_t KeyId, uint8_t value);
		void handleMidiRemoteControlMessage (const MidiMessage& message);
		void handleInMidiMessage (const MidiMessage& message, const std::string portName, bool isSendMidiThru = true);
		bool midiRemoteControlEnabled (String msgPortName);
		bool midiTransposeControlEnabled (String msgPortName);
		bool midiMachineControlEnabled (String msgPortName);
		void handleMidiMachineControlMessage (const MidiMessage& message);

		std::string getPortNameFromId (int8_t id);
		int8_t getPortIdFromName (std::string name);

		int32_t getSavedActiveSongId();

		void handleMidiTransposeControlMessage (const MidiMessage& message);

		void sendMidiMessageInternal (MidiMessage msg, Track* track);
		void ResetTrackUsedByList (int32_t activeSongId, bool isDelete);

		std::vector <Instrument*> getInstrumentsFromPortName (std::string portName);
		int32_t getSongIdFromName (std::string songName);
		int32_t getSceneIdFromName (std::string sceneName);
		uint8_t getTrackIdFromName (std::string trackName);

		template <typename Type>
		static Type setBounds (Type lowerLimit,
			Type upperLimit,
			Type valueToConstrain) noexcept
		{
			assert (lowerLimit <= upperLimit); // if these are in the wrong order, results are unpredictable..

			return valueToConstrain < lowerLimit ? lowerLimit
				: (upperLimit < valueToConstrain ? upperLimit
					: valueToConstrain);
		}

		// Midi packet creation functions
		static uint8_t initialByte (const int type, const int channel) noexcept;
		static MidiMessage controllerEvent (const int channel, const int controllerType, const int value) noexcept;
		static MidiMessage programChange (const int channel, const int programNumber) noexcept;
		static MidiMessage noteOn (const int channel, const int noteNumber, const uint8 velocity) noexcept;
		static MidiMessage noteOff (const int channel, const int noteNumber, uint8 velocity) noexcept;
		static MidiMessage allNotesOff (const int channel) noexcept;
		static MidiMessage allSoundOff (const int channel) noexcept;
		static MidiMessage pitchWheel (const int channel, const int position) noexcept;
		static MidiMessage pitchWheel (const int channel, const int data1, const int data2) noexcept;
		static MidiMessage channelPressureChange (const int channel, const int pressure) noexcept;
		static MidiMessage aftertouchChange (const int channel, const int noteNum, const int aftertouchValue) noexcept;

		void getRecordParams (void);

		void openMidiInterfaces (void);

		bool getActiveTrackTxInstrumentAvailability(void);

		void setUpPlatformAudio (void);

		virtual void audioDeviceAboutToStart (AudioIODevice* device);
		virtual void audioDeviceStopped();
		void startJuceAudioCallback (void);
		void stopJuceAudioCallback (void);
		std::atomic<uint64_t> desktopAudioCallbacks{0};
		std::atomic<float> desktopAudioPeak{0};
		std::atomic<uint64_t> desktopSourceSamples{0}, desktopOutputSamples{0};
		std::atomic<double> desktopHostSampleRate{0};
		void prepareToPlay(int, double) override;
		void releaseResources() override;
		void getNextAudioBlock(const juce::AudioSourceChannelInfo&) override;

		virtual void handleInternalPgmChange (int32_t instrId, int programId)override;
		virtual void handleMidiSettingChange(int32_t instrId)override;

#if (JUCE_VERSION>=8) && (JUCE_BUILDNUMBER>=0)
		void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int deviceInputChannels,
			float* const* outputChannelData, int deviceOutputChannels,
			int numSamples, const juce::AudioIODeviceCallbackContext& /*context*/);
#else
		void audioDeviceIOCallbackWithContext(const float** inputChannelData, int numInputChannels,
			float** outputChannelData, int numOutputChannels,
			int numSamples, const juce::AudioIODeviceCallbackContext& context);
#endif

#ifdef _WIN32
#ifdef USE_INTERNAL_20MS_TIMER
		static void startWindowsAudioDirect (void);
		static void playWindowsAudioDirect (AudioDataType* audioBuffer, int sizeInBytes);
		static void stopWindowsAudioDirect (void);
#endif
#endif

#ifdef LOG_PCM_TO_FILE
		void openLogFiles (void);
		void closeLogFiles (void);

		void openLogFile_1 (void);
		void openLogFile_2 (void);
		void openLogFile_3 (void);

		void logToFile_1 (AudioDataType* buffer, int sizeInBytes);
		void logToFile_2 (AudioDataType* buffer, int sizeInBytes);
		static void logToFile_3 (SongManager *context, AudioDataType* buffer, int sizeInBytes);//used in timer cb
#endif
		void setMasterVolume(float volume);
		float getMasterVolume();

#ifdef USE_INTERNAL_20MS_TIMER
		void start20msAudioProcessingTimer (void);
		static void directPlatformPlay (AudioDataType* TwentyMsBuffer, int numBytes);//used in timer cb
		void stopAudioProcessingTimer (void);
		void readFromBuffer (AudioDataType* data);
		static void writeToBuffer (SongManager *songManagerP, AudioDataType* data);//used in timer cb
#endif

		void clearSceneTimeBaseTick(void);
		void renderTracksAndMix(AudioDataType* buffer);
        void renderDesktopStereo(AudioDataType* planarBuffer);
        bool setDesktopVoiceLimit(int voices);
        int desktopVoiceLimit = 32;

		void handleSyncIn(float frequencyHz);
		float mapConfigPpqnToPpqn(int8_t configPpqn);
		int8_t getChannelFromMidiMessage(MidiMessage msg);
		void handleMidiClkIn(float freqHz);
		void setTempoFromFrequency(float freqHz);
		void initMidiClkCalculator(void);
		void deInitMidiClkCalculator(void);
		void startMidiClockCalculation(void);
		//MidiOutput* getUsbMidiOutInterface(int8_t portId);
		void initAuditioningPreset();
		void setAuditioningPreset(uint8_t portID, int presetindex);

		void setSystemState(KEY_PRESSED key);
		SYSTEM_STATE getSystemState(void);
		void setStepReSelectedStep(uint8_t stepId);
		Track* getPlayTrackFromId(uint8_t trackId);
		void handleMidiThru(const MidiMessage& message, const std::string portName);
		std::multimap <int8_t, uint8_t> playSceneTrackPortMap;
		bool UpdateplaySceneTrackPortMap();//updates playSceneTrackPortMap

		void sendMidiClk(void);
		void handleClkOutPpqnChange(void);
		void handleClkOutPortChange(void);
		void stopMidiClk(void);
		void sendSyncClkPulses(void);
		void sendMidiClkPackets(void);
		void sendTransportControlMsg(MidiMessage msg);
		void sendMidiExternal(MidiMessage msg, uint8_t port);

		std::mutex cvset_mutex;  // Lock for the CV set
		void setCvGateVoltage(int cv_channel, float cvVolt, int gate_channel, float gateVolt, bool allcvOff = false, bool allGateOff = false);


#ifdef __linux__
		static void handle_signal(int sig);
		void checkAndOpenMidiHostDevice (void);
		static void *handleUsbHostMidiDevices (void *arg);
		
#endif
		std::vector <std::string> getUsbHostMidiDeviceList(void);
		std::vector <std::string> findNewlyAddedUsbMidiDevices(std::vector <std::string> extDevList);
		std::vector <std::string> findRecentlyRemovedUsbMidiDevices(std::vector <std::string> extDevList);
		void initUsbMidiDevPorts(std::vector <std::string> devNameList);
		void deinitUsbMidiDevPorts(std::vector <std::string> devNameList);
		void openUsbMidiPort(uint8_t port);
		void closeUsbMidiPort(uint8_t port);

		bool isDinMidiPort(uint8_t port);
		bool isUsbMidiPort(uint8_t port);
		bool isUsbDeviceMidiPort(uint8_t port);
		bool isUsbHostMidiPort(uint8_t port);
		std::map<int8_t, std::string> getUsbHostPortMidiDeviceNames(void);
		void publishUsbHostMidiChangeNotificationToUI(void);

#ifdef __linux__
		void startPulseProcessingTimers(void);
		void stopPulseProcessingThreads(void);
#endif
		float getTickDivider(TimeBase tb);
		uint16_t getPulsesPerStep(TimeBase tb);

		bool runPulseProcessingThreads = false;
		std::multimap <int32_t, std::string> songInfoList;

		std::unique_ptr<CSong> activeSong{ nullptr };
		//std::chrono::time_point<std::chrono::high_resolution_clock> _timeStamp;
		bool _isSongPaused = false;//pause/resume
		bool _isSongPlaying = false;//play/stop

		std::unordered_map <int8_t, int8_t>  recordNoteOnList;

		bool _isRecording = false;
		IgniteEngine& engine;
		uint16_t _recordEventFilter = (uint16_t)RecordEvents::Note;
		bool _recordTrigger = false;
		int16_t _scenePulsesPerStep = 0;
		bool manualJumpToNewScene = false;
		bool manualSwitchToNextScene = false;

		bool _sceneChangedFlag = false;
		//bool _trackSoloModeOn = false;//moved to scene settings
		MuteHoldType _gSceneMuteHold = MuteHoldType::Off;
		int16_t _gMuteHoldLockedSceneId = -1;

		bool _fillModeSwitchOn = false;
		int8_t _fillModeType = static_cast<int8_t> (FillModeType::active);
		int8_t _fillModeReleaseType = static_cast<int8_t> (FillReleaseMode::key);
		//bool _fillModeOn = false;
		int16_t _fillModeBarId = 0;
		int32_t _fillModeSceneId = 0;

		// Data protection
		std::mutex _sceneBarChange;

		uint8_t _stopCount = 0;
		std::atomic <uint8_t> _reStartCount = 0;
		std::atomic<bool> _pulseHandlingInProgress{ false };

		//Externl midi clock, freq calc timer
		std::atomic<uint32_t> _midiClockCounter { 0 };
		bool _midiClockCalculationStarted = false;
		bool _midiClkSlaveModeActive = false;
		bool _midiClkRxInProgress = false;

		std::unordered_map <uint8_t, int8_t> _recordSetting;//record config, value

		TimeBase _sceneTimeBase = TimeBase::Base_32T;

		float _masterVolume = 0.7f;

		SYSTEM_STATE _state = SYSTEM_STATE::IDLE;

		bool _clkOutTimerRunning = false;
		std::chrono::microseconds _clockOutDurationUs;

#ifdef __linux__
		pthread_t usbHostMidiDeviceThreadId;
		pthread_attr_t usbHostMidiDeviceThreadAttr;
		static bool usbHostMidiDevConnectSigReceived;
		static bool usbHostMidiDevDisconnectSigReceived;
#endif

	private:
		bool isUndilistNeedUpdate = true;
		bool isSustainPedalOn = false;
		bool isInternelKeyHeld = false;
		std::unordered_map<std::string, Pattern*> patternRecMap;
		IParamChangeListener* paramlistener;
		std::map <int8_t, IgniteMidi>  midiDevList;
		std::list<PendingMidiMsg> PendingMidiMsgList;
		int8_t InternalChannelPgm[20] = { -1, };//0-16 channels

		int32_t activeSongId = -1;
		int32_t savedActiveSongId = -1;
		int32_t numberOfSongs = -1;

		uint8_t TestCreateNumScene = 5;
		uint8_t TestCreateNumTrack = 16;
		uint8_t TestCreateNumInst = 5;
		uint8_t TestCreateNumPattern = 5;
		uint8_t TestCreateNumPatternBars = 16;

		bool _isAudioCallbackEnabled = false;
		bool _isAudioTimerEnabled = false;
		bool _isAuditioningOn = false;
		bool _handleMidiIn = false;
		int InternalTimeIntervalMs = 5000;
		std::unique_ptr<Track> AuditioningTrack[AUDITIONING_MAX_PORT]{ nullptr };
		bool isAuditioningTrackEnabled[AUDITIONING_MAX_PORT];

//		void SendPendingMidiMsg (void);
//		void handleMidiOnSongExit (void);
		void handleRunningSongOnSwitch (void);
		void releaseSongResources (int32_t activeSongId, bool isDelete);
		void SaveUsedbyLisrtDataForActiveSong();
		void GetEngineDataBackup(uint8_t DataFolderId);
		void RestoreEngineDataBackup(uint8_t DataFolderId);

		void setAuditioningState(bool state);
		void timerCallback() override;
		void HandleMidiInAuditioning(uint8_t portID, const MidiMessage msg);
		void SaveData();
#ifdef _WIN32
		unsigned int audioGentimerId;
#elif defined __linux__
		timer_t audioGentimerId;
#endif
#ifdef ENABLE_THREADS_TO_PLAY
#ifdef __linux__
		pthread_t TrackPlayThread_0;
		pthread_t TrackPlayThread_1;
		pthread_t TrackPlayThread_2;
		pthread_t TrackPlayThread_3;
		pthread_t TrackPlayThread_4;
		pthread_t TrackPlayThread_5;
		pthread_t TrackPlayThread_6;
		pthread_t TrackPlayThread_7;

		static void* TrackPlayThreadFunction_0(void*);
		static void* TrackPlayThreadFunction_1(void*);
		static void* TrackPlayThreadFunction_2(void*);
		static void* TrackPlayThreadFunction_3(void*);
		static void* TrackPlayThreadFunction_4(void*);
		static void* TrackPlayThreadFunction_5(void*);
		static void* TrackPlayThreadFunction_6(void*);
		static void* TrackPlayThreadFunction_7(void*);
#else /* __linux__ */
		CThread TrackPlayThread_0;
		CThread TrackPlayThread_1;
		CThread TrackPlayThread_2;
		CThread TrackPlayThread_3;
		CThread TrackPlayThread_4;
		CThread TrackPlayThread_5;
		CThread TrackPlayThread_6;
		CThread TrackPlayThread_7;

		void TrackPlayThreadFunction_0(void);
		void TrackPlayThreadFunction_1(void);
		void TrackPlayThreadFunction_2(void);
		void TrackPlayThreadFunction_3(void);
		void TrackPlayThreadFunction_4(void);
		void TrackPlayThreadFunction_5(void);
		void TrackPlayThreadFunction_6(void);
		void TrackPlayThreadFunction_7(void);
		ThreadFunction TrackPlayThreadFunctionCB[PLAY_THREAD_TRACK_SIZE];
#endif /* else __linux__ */
		CMsgQueue<std::list <TrackAndPosition>> TrackPlayQ_0;
		CMsgQueue<std::list <TrackAndPosition>> TrackPlayQ_1;
		CMsgQueue<std::list <TrackAndPosition>> TrackPlayQ_2;
		CMsgQueue<std::list <TrackAndPosition>> TrackPlayQ_3;
		CMsgQueue<std::list <TrackAndPosition>> TrackPlayQ_4;
		CMsgQueue<std::list <TrackAndPosition>> TrackPlayQ_5;
		CMsgQueue<std::list <TrackAndPosition>> TrackPlayQ_6;
		CMsgQueue<std::list <TrackAndPosition>> TrackPlayQ_7;
#endif
	public:
#ifdef LOG_PCM_TO_FILE

#ifdef _WIN32
		void *fd_1;
		void *fd_2;
		void *fd_3;
#elif defined __linux__
		int fd_1;
		int fd_2;
		int fd_3;
#endif /* __linux__ */

#endif /* LOG_PCM_TO_FILE */
		void handleCommandMessage(int id);
		juce::AudioDeviceManager *dm = nullptr;
		void* get_tsf_instance();
		struct SoundFontPreset { int bank, program, index; std::string name; };
		std::vector<SoundFontPreset> getDesktopSoundFontPresets();
		bool loadDesktopSoundFont(const std::string& path, std::string& error);
        bool browseDesktopSoundFont(const std::string& path,std::string& error);
        std::shared_ptr<void> acquireDesktopSoundFont(const std::string& path,std::string& error);
        std::shared_ptr<void> soundFontForInstrument(Instrument* instrument,std::string& error);
        std::string soundFontPathForInstrument(Instrument* instrument) const;
        void desktopInstrumentFontChanged(const std::string& name);
        bool assignDesktopSoundFont(const std::string& instrument,const std::string& path,int program,std::string& error);
		std::string desktopPresetName(int program,const std::string& instrument = {});
		std::string desktopSoundFontPath;
        std::string desktopBrowserFontPath;
		static juce::File desktopSettingsFile();
		void receiveDesktopMidi(uint8_t port, const MidiMessage& message);
		void desktopPanic();
		bool desktopPreviewPreset(int presetIndex);
		void desktopPreviewNoteOff();
		void desktopStopPreview();
		bool desktopMonitorEnabled = true;
		std::function<bool(const MidiMessage&, uint8_t)> desktopMidiOutput;
		std::atomic<uint64_t> desktopMidiReceived{0};
		void renderDesktopMonitor(AudioDataType* buffer);
	//	juce::dsp::ProcessorChain<juce::Reverb, juce::dsp::Chorus<float>, dsp::IIR::Filter<float>, dsp::IIR::Filter<float>, juce::dsp::LadderFilter<float>, juce::dsp::Gain<float>> processorChain;

	private:
		int soundFontProgramCount;
		FixedAudioBlockAdapter<INTERNAL_AUDIO_20MS_BLOCK_SIZE,2> audioBlockAdapter;
		juce::ResamplingAudioSource audioResampler{this, false, 2};
		void* _songTSf = nullptr;
        std::shared_ptr<void> desktopDefaultFont,desktopBrowserFont;
        std::map<std::string,std::weak_ptr<void>> desktopFontCache;
		void init_tsf();
		static SongManager *instance;

	public:
		static void setInstance(SongManager* inst);
	};
}
