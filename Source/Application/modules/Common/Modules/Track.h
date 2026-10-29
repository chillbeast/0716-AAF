#pragma once
#include <stdint.h>
#include <iostream>
#include <iterator>
//#include <list>
#include <unordered_map>
//#include <vector>
#include <string>
#include <mutex>
#include <tuple>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_dsp/juce_dsp.h>

#include "Common/Modules/Pattern.h"
#include "Common/Modules/Instrument.h"
#include "Common/Modules/ISyncListener.h"
#include "Firmware/Application/Model/SongManager.h"

//Effects related
#define FILTER_LOWEQ_INDEX	0
#define FILTER_HIGHEQ_INDEX	1
#define EQFILTER_INDEX_MAX	2

//#define TIMER_TEST_PLAY
//#define TIMER_1
//#define TIMER_2

#if defined(__linux__) && defined(TIMER_TEST_PLAY) 

	#ifdef TIMER_1
		#define TIMER_START(...)	timerStart(__VA_ARGS__)
		#define TIMER_STOP(...)		timerStop(__VA_ARGS__)
		#define TIMER_PRINT(...)	timerPrint(__VA_ARGS__)
		#define TIMER_CLEAR(...)	timerClear(__VA_ARGS__)
	#else /* TIMER_1 */
		#define TIMER_START(...)	
		#define TIMER_STOP(...)		
		#define TIMER_PRINT(...)	
		#define TIMER_CLEAR(...)	
	#endif /* ! TIMER_1 */

	#ifdef TIMER_2
		#define TIMER_START_2(...)	timerStart(__VA_ARGS__)
		#define TIMER_STOP_2(...)	timerStop(__VA_ARGS__)
		#define TIMER_PRINT_2(...)	timerPrint(__VA_ARGS__)
		#define TIMER_CLEAR_2(...)	timerClear(__VA_ARGS__)
	#else /* TIMER_2 */
		#define TIMER_START_2(...)	
		#define TIMER_STOP_2(...)		
		#define TIMER_PRINT_2(...)	
		#define TIMER_CLEAR_2(...)	
	#endif /* ! TIMER_2 */

#else /* __linux__ && TIMER_TEST_PLAY */

	#define TIMER_START(...)	
	#define TIMER_STOP(...)		
	#define TIMER_PRINT(...)	
	#define TIMER_CLEAR(...)	
	#define TIMER_START_2(...)	
	#define TIMER_STOP_2(...)		
	#define TIMER_PRINT_2(...)	
	#define TIMER_CLEAR_2(...)	

#endif /* ! (__linux__ && TIMER_TEST_PLAY) */


namespace Ignite
{
	struct TrackData;
	class SongManager;
	struct BarLoopRange;

	struct playStepData
	{
		bool stepGate = false;
		bool noteOn = false;

		uint32_t onTick = 0;
		uint32_t offTick = 0; // max possible value 586751

		uint8_t stepNote = 0;
		uint8_t stepDelay = 0;
		uint8_t stepLengthTicks = 0;
		uint8_t stepLengthSteps = 0;
		uint8_t stepVelocity = 0;
		//uint8_t stepCount = 0;
		std::vector<Aux> stepAuxList;
	};

	struct midiData
	{
		double timeStamp;
		uint8_t statusByte;
		uint8_t dataByte1;
		uint8_t dataByte2;
	};
	struct TrackCVdata
	{
		bool gatestate = false;
		uint8_t cvPortId = (int8_t)InstrumentPorts::MaxMidiPort;
		std::map<uint8_t, TickOffset_t> suppressedNotesOn; //key as note
	};

	#define MAX_BUFFER_SIZE 1000 // TODO profile max level once track limit is decided for recording
	class RecordBufferLockFree
	{

	private:
		midiData dataBuffer[MAX_BUFFER_SIZE];
		std::atomic <int16_t> writeIndex;
		std::atomic <int16_t> readIndex;

	public:
		RecordBufferLockFree() : writeIndex(0), readIndex(0)
		{
		}

		bool write(midiData data)
		{
			auto nextWriteIndex = (writeIndex.load() + 1) % MAX_BUFFER_SIZE;

			if (nextWriteIndex == readIndex.load())
			{
				return false; //buffer full
			}
			else
			{
				dataBuffer[writeIndex.load()] = data;
				writeIndex.store((const short)nextWriteIndex);
				return true;
			}
		}

		bool read(midiData& data)
		{
			if (readIndex.load() == writeIndex.load())
			{
				return false;
			}
			else
			{
				data = dataBuffer[readIndex.load()];
				readIndex.store((readIndex.load() + 1) % MAX_BUFFER_SIZE);
				return true;
			}
		}
	};

	class Track : public ISyncListener
	{
	public:
		explicit Track (int8_t id, std::string name, bool mute, SongManager* songmgr);
		Track (const Track&);
		Track& operator= (const Track&) = delete;
		~Track ();

		void setTrackRxInstrument (Instrument* val);
		void setTrackTxInstrument (Instrument* val);
		void setTrackMainPattern (Pattern* newPattern);
		void setTrackFillPattern (Pattern* val);
		void setTrackName (std::string name);

		void setTrackMuteStatus (uint8_t val);
		void setTrackId (int8_t id);//to be used only by reAssignSequentialIdsForTracks

		Instrument* getTrackRxInstrument (void);
		Instrument* getTrackTxInstrument(void);
		Pattern* getTrackPattern (void);
		std::string getTrackName (void);
		int16_t getTrackTotalSteps (void);
		Step* getTrackPlayStep (void);


		int16_t getTrackPlayStepId (void);
		bool getTrackMuteStatus (void);
		Pattern* getTrackFillPattern (void);
		Pattern* getTrackMainPattern (void);
		int8_t getTrackId (void);
		void pulse(int16_t sceneBarId, int16_t sceneStepId);
		void unloadDesktopSoundFont();
		void prepareDesktopMonitor();

		bool getRxInstrumentData (void);
		bool getTxInstrumentData (void);
		void advanceTrackPlayStep (void);
		std::vector <PatternDirection>  getPatternDirection (void);
		void initTrackPlaySetting (void);
		void deInitTrackPlaySetting (void);
		TimeBase getTrackPatternTimeBase(void);

		void checkForTimeBaseChange (void);
		bool getNodeTransposeDefeatStatus (void);
		float cvGetNoteVoltsPerOctave(uint8_t voltSet, uint8_t noteToSend, uint8_t noteRangeStart, uint8_t noteRangeEnd);
		float cvGetNoteVoltsHertzPerVolt(uint8_t voltSet, uint8_t noteToSend, uint8_t noteRangeStart, uint8_t noteRangeEnd);
		bool handleCvGeneration(std::string patternName, bool isAllOff);
		bool getTxInstrumentTransposeDefeatStatus (void);
		bool getTxInstrumentFtsDefeatStatus (void);
		int8_t getRxInstrumentPort(void);
		int8_t getRxInstrumentChannel (void);
		int8_t getTxInstrumentPort(void);
		int8_t getTxInstrumentChannel(void);

		int getRandomData(int min, int max);
		void  sculptUpdate(Pattern* pattern);

		void recordStepData (int16_t stepId, int8_t bardId, TickOffset_t tick);
		bool isValidNoteEvent (midiData data);

		void publishTrackBarAndStepInfoToUI (void);

		bool getBarTransposeDefeatStatus (uint8_t barId);


		bool getGbarSyncStatusForBar (int16_t barId);

		void setTrackRecordStatus(bool state);
		bool getTrackRecordStatus(void);

		void setTrackSoloStatus (bool state);
		bool getTrackSoloStatus (void);
		bool soloModeCheck (void);
		int16_t getBarIdFromStepId (int16_t stepId);

		bool trackMuted (void);
		void setTrackMuteStatusInMuteHoldMode (bool val);
		bool getTrackMuteStatusInMuteHoldMode (void);

		bool shouldPlayFillPattern (void);

		bool barIdWithinPattern (int16_t bardId);
		int16_t getFirstStepIdOfPattern (void);
		int16_t getLastStepIdOfPattern (void);

		void addTvKnobToTrack (int8_t knob, TrackValue tvData);
		void setTvKnobType (int8_t knob, TrackValueType val);
		void setTVPgmChangeKnobVal (int8_t knob, TrackValue val);
		void initTvCcKnobValue();
		void setTvCcKnobValue (int8_t knob, TrackValue val);
		float getTvCcKnobValue (int8_t knob);
		int8_t getTvPgmChangeKnobValue (int8_t knob);
		int8_t getTVPgmChangeLsb (int8_t knobId);
		void setTVPgmChangeLsb (int8_t knobId, TrackValue val);
		int8_t getTVPgmChangeMsb (int8_t knobId);
		void setTVPgmChangeMsb (int8_t knobId, TrackValue val);
		void deleteTvTrackData (int8_t knob);
		void deleteAllTvTrackData (void);
		TrackValue getTv (int8_t knobId);
		void setTvDataFromInstrument (Instrument* instrument);

		void sculpt (Step* stepToPlay,
			std::vector<TickOffset_t>& delayList,
			std::vector<TickLength_t>& lengthTicksList,
			std::vector<uint8_t>& lengthStepsList,
			std::vector<uint8_t>& noteList,
			std::vector<uint8_t>& velocityList,
			bool isNextStep,
			bool isEditTrack);
		void truncateNegativeDelayToZero (TickOffset_t& delay);
		void convertNegativeDelayToPositive (TickOffset_t&  delay);

		void handleMidiOutClockGeneration (uint16_t tick);
		std::vector <uint16_t> getTicksAtWhichClockIsSend (int8_t rate);

		bool cvAdsrOutEnabledForThisTrack (void);
		bool cvEnabledForThisTrack(void);
		void generateCvVoltages(void);
		void generateAdsrVoltages (uint8_t velocity, uint16_t lengthTicks);
		void handleAdsrOut (uint16_t tick);

		uint16_t uiTickToEnginePulse (TickLength_t uiTick);
		uint16_t enginePulseToUiTick (uint16_t enginePulse);

		void handleBrayControlForNodeParams (uint8_t& velocity);
		void handleBrayControlForStepAuxParams (AuxEventType type, int8_t& data1, int8_t& data2);
		void SaveDfaultUsedbyInfo (bool update);
		void ResetUsedbyInfo (std::string SongName, std::string SceneName, bool ForceRemove);

		uint16_t timeBaseToPps(TimeBase tb);
		TickOffset_t calculateOffsetWithinStep(TickOffset_t tick);
		void setStepReSelectedStep(uint8_t stepId);

		template <typename T>
		void boundCheck (T& value, T min, T max)
		{
			if (value < min)
			{
				value = min;
			}
			else if (value > max)
			{
				value = max;
			}
		}

		template <typename T>
		void normalise (T& value, T inputRange, T expectedRange)
		{
			value = (value * expectedRange) / inputRange;
		}

		void handleTransposeControlFromExternalMidi (void);

		void clearTrackPlayStepId (void);

		void sfInit(void);
		void sfReset(void);
		void sfLoad(void);
		void sfNoteOn(uint8_t channel, uint8_t key, float velocity);
		void sfNoteOff(uint8_t channel, uint8_t key);
		void sfHandleMidi(const juce::MidiMessage& message);
		void sfSynthesizeAudio(void);
		void sfSetProgram(void);
		void sfChangeProgram(void);

		void setInternalAudioEffects(uint8_t knobID, float value, bool isAlternate);
		void effectsInit(void);
		void reverbInit(void);
		void chorusInit(void);
		void eqInit(void);
		void filterInit(void);
		void gainInit(void);
		void resetEffects(void);

		void initStepRecord(void);
		void initTrackRecordSetting(void);
		void stepRecordNote(const MidiMessage& message);
		void liveRecord(const MidiMessage& message);
		void AuditioningTrackInit(void);
		void AuditioningTrackSetPreset(int presetindex);
		void AuditioningTrackClear(void);

		void setTrackRxMidiPortAndChannel(void);
		void setTrackTxMidiPortAndChannel(void);

		bool getSystemSoloMode(void);
		int8_t getSceneSwing(void);
		void getSequenceStep(std::string patternName);
		void checkAndHandleTimeBaseUpdate(std::string patternName);
		void playStepEvents(std::string patternName);
		void recordStepEvents(void);
		void handleSwing(Pattern* pattern);
		void advanceUiTick(void);
		void advanceTrackPulse(void);
		void handleStepChange(std::string patternName, int16_t sceneBarId, int16_t sceneStepId);
		void handleStepChangeWithout_Gbar_BarRepeat_BarLoop(std::string patternName);
		void recordTimeSequenceManagerUpdate(void);
		void checkAndHandlePatternLengthChange(void);
		void publishInfoToUI(void);

		int getTsfPresetFromIgniteIndex(int index);

		bool isSceneBarChange(int16_t sceneBarId);
		bool isGBarSyncOn(int16_t barId);
		bool isTrackBarEndReached(int16_t stepIndexWithinBar);
		void actionsAtStepBoundary(void);
		void actionsAtBarBoundary(std::string patternName, int8_t previousTrackBarId);

		void updatePlayingNoteList (MidiMessage msg);
		void stopPlayingNotes (void);

		TrackData* _trackData;
		std::atomic<uint64_t> completedPulses{0};
		SongManager* _songManager = nullptr;

		Instrument* SavedInstrument;
		Instrument* SavedOutInstrument;
		Pattern* SavedMainpattern;
		Pattern* SavedFillpattern;
		bool DefaultDataSaved = false;
		bool InstrumentUpdated = false;
		bool InstrumentOutUpdated = false;
		bool MainpatternUpdated = false;
		bool FillpatternUpdated = false;

		int8_t _patternSwing = 0;
		int8_t _rxMidiPort = 0;
		int8_t _rxMidiChannel = -1;

		int8_t _txMidiPort = 0;
		int8_t _txMidiChannel = -1;

		bool _newStep = false;
		int8_t _trackBarRepeatCount = 0;
		int16_t _trackPulse = 0;
		int16_t _trackUiTick = 0;//for ppqn 48, max ui tick will be 192
		uint16_t stepRecordNoteOnCount = 0;
		int16_t _pulsesPerTick = 0;
		uint16_t _pulsesPerStep = 0;// Update based on pattern's TB
		int16_t _trackPlayStepId = 0;// Keep track of step progression
		int16_t _recordFirstStep = -1;//for checking loop
		int8_t _trackBarId = 0;//Keep track of bar progression
		int16_t _recordFirstBar = -1;//for checking loop
		TimeBase _patternTimeBase = TimeBase::Base_1;// update from pattern

		int16_t _trackStepIndex = 0;
		int16_t _currentSceneBarId = 0;
		int16_t _stepCountForCurrentBar = 0;//fetch from seqmanager at every bar change

		std::map <int8_t, uint8_t> barRepeatCountMap;
		//std::tuple <bool, BarLoopRange> barLoopStateNRange;

		//1 buffer for each track
		RecordBufferLockFree recordData;



		bool _barTransposeDefeat = 0;
		std::mutex _NoteListMtx;
		std::chrono::time_point<std::chrono::system_clock> lastUiPublish;

		bool _muteHoldModeTrackMuteState = true;

		std::vector <double> adsrEnvelopeValues;

		std::unordered_set <uint8_t> _noteOnList;
		std::unordered_map <uint8_t, uint8_t> _playingNoteList;

		void* _trackSf = nullptr;
		bool desktopPresetValid = true;
		AudioDataType _twentyMsBuffer[INTERNAL_AUDIO_20MS_BLOCK_SIZE] = { 0 };
		juce::AudioBuffer<float> audioBuff;

		//Effects - InternalAudio
		uint8_t audioEffectsOrder[AUDIO_EFFECT_MAX]{ AUDIO_REVERB ,AUDIO_CHORUS, AUDIO_EQ_L, AUDIO_EQ_H, AUDIO_FILTER };
		juce::Reverb igniteReverb;
		juce::Reverb::Parameters theReverbParameters;
		juce::dsp::Chorus<float> _Chorus[1];//TODO: Array can be mapped to Tracks
		bool chorusStatus[1] = { false };//To be applied?

		juce::dsp::IIR::Filter<float> eqFilter[EQFILTER_INDEX_MAX];
		float EQcutoffFreq[EQFILTER_INDEX_MAX] = { 20.0f, 20000.0f };
		float EQcutoffGain[EQFILTER_INDEX_MAX] = { 0.0001f, 0.0001f };
		bool EQisEnabled[EQFILTER_INDEX_MAX] = { false, false };
		juce::dsp::LadderFilter<float> filter;
		float filtercutoffFreq = 20000.0f;
		float filterResonance = 0.1f;
		juce::dsp::Gain<float> trackGain;

		std::atomic_bool _patternUpdatedByRecord{ false };
		TrackCVdata _trackCVdata;


#ifdef TIMER_TEST_PLAY

		#define TIME_SAMPLES 9999

		typedef struct timePoint
		{
			struct timespec startTime;
			struct timespec endTime;
		};
	
		struct event_time
		{
			int index;
			struct timePoint tp_arr[TIME_SAMPLES];
		};
		
		std::map <int, struct event_time> eventTimerMap;

		int timerIndex = 0;

		void timerStart(int id);

		void timerStop(int id);
		
		void timerPrint(void);

		void timerClear(void);
		bool timerCleared = false;

#endif /* TIMER_TEST_PLAY */
	};

	class Adsr
	{
	public:
		Adsr (uint8_t attackTimePercent, uint8_t decayTimePercent, uint8_t sustainVoltagePercent, uint8_t releaseTimePercent, uint8_t amountPercent, uint8_t velocityPercent,
			int8_t maxVolt, uint16_t noteDurationTicks, uint16_t voltageSamplesRate, uint16_t ppqn, uint16_t tempo, uint8_t velocity, uint16_t noteOnLengthTicks)
		{
			//Bound check all percent values
			boundCheck <uint8_t> (attackTimePercent, 0, 100);
			boundCheck <uint8_t> (decayTimePercent, 0, 100);
			boundCheck <uint8_t> (sustainVoltagePercent, 0, 100);
			boundCheck <uint8_t> (releaseTimePercent, 0, 100);
			boundCheck <uint8_t> (amountPercent, 0, 100);
			boundCheck <uint8_t> (velocityPercent, 0, 100);


			//attack and decay time are controlled by user, normalise if total is more than on time of note
			if ((attackTimePercent + decayTimePercent) > 100)
			{
				auto totalTime = attackTimePercent + decayTimePercent;
				//only integer part of calculated value is considered, as it provides good resolution
				//For finer resolution take decimal part also (convert all % values to float)
				attackTimePercent = static_cast <uint8_t> (attackTimePercent * (static_cast <double> (100) / totalTime));
				decayTimePercent = static_cast <uint8_t> (decayTimePercent * (static_cast <double> (100) / totalTime));
			}
			//Max limit of 5000 is arbitrary value, modify if we require higher sample rate
			boundCheck <uint16_t> (voltageSamplesRate, 500, 5000);

			//Assuming envelope to be a positive side curve
			//If negative values are required, modify bound check with negative max
			minVoltage = 0;
			maxVoltage = ((amountPercent /100) * maxVolt) + ((velocityPercent/100) * velocity * maxVolt);
			boundCheck <int8_t> (maxVoltage, minVoltage, maxVolt);


			sustainVoltage = maxVoltage * (static_cast <double> (sustainVoltagePercent)/100);

			//Attack, Decay and suatain will happen during the ON phase.
			//Release will happen during the OFF phase(once note is released)
			double stepDurationSec = static_cast <double> (60) / tempo;
			double noteDurationSec = (static_cast <double> (noteDurationTicks) / (ppqn * 4)) * stepDurationSec;
			double onTimeSec = (static_cast <double> (noteOnLengthTicks) / (ppqn * 4)) * stepDurationSec;
			double offTimeSec = noteDurationSec - onTimeSec;

			uint16_t onTimeSamples = static_cast <uint16_t> (onTimeSec * voltageSamplesRate);
			uint16_t offTimeSamples = static_cast <uint16_t> (offTimeSec * voltageSamplesRate);

			attackTimeSamples = static_cast <uint16_t> (static_cast <double> (attackTimePercent) / 100 * onTimeSamples);
			decayTimeSamples = static_cast <uint16_t> (static_cast <double> (decayTimePercent) / 100 * onTimeSamples);
			releaseTimeSamples = static_cast <uint16_t> (static_cast <double> (releaseTimePercent) / 100 * offTimeSamples);
			sustainTimeSamples = onTimeSamples - (attackTimeSamples + decayTimeSamples);
		}

		template <typename T>
		void boundCheck (T& value, T min, T max)
		{
			if (value > max)
			{
				value = max;
			}
			else if (value < min)
			{
				value = min;
			}
		}

		std::vector <double> createEnvelope ()
		{
			std::vector <double> outputVoltageList;

			// Attack phase
			double linearVoltageDelta = static_cast <double> (maxVoltage) / attackTimeSamples;
			double voltage = 0.0;

			for (int i = 0; i < attackTimeSamples; ++i)
			{
				voltage += linearVoltageDelta;
				if (voltage > maxVoltage)
				{
					voltage = maxVoltage;
				}
				outputVoltageList.push_back (voltage);
			}

			// Decay phase
			linearVoltageDelta = static_cast <double> (maxVoltage - sustainVoltage) / decayTimeSamples;

			for (int i = 0; i < decayTimeSamples; ++i)
			{
				voltage -= linearVoltageDelta;
				if (voltage < minVoltage)
				{
					voltage = minVoltage;
				}
				outputVoltageList.push_back (voltage);
			}

			// Sustain phase
			for (int i = 0; i < sustainTimeSamples; ++i)
			{
				outputVoltageList.push_back (sustainVoltage);
			}

			// Release phase
			//Assuming envelope volatge reaches zero at end of the step
			linearVoltageDelta = (sustainVoltage - 0) / releaseTimeSamples;

			for (int i = 0; i < releaseTimeSamples; ++i)
			{
				voltage -= linearVoltageDelta;
				if (voltage < minVoltage)
				{
					voltage = minVoltage;
				}
				outputVoltageList.push_back (voltage);
			}

			return outputVoltageList;
		}

	private:
		uint16_t attackTimeSamples;
		uint16_t decayTimeSamples;
		uint16_t releaseTimeSamples;
		uint16_t sustainTimeSamples;

		int8_t maxVoltage;
		int8_t minVoltage;
		double sustainVoltage;
	};
}
