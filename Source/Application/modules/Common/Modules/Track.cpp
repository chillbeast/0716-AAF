#include <string>
#include <assert.h>
#include <random>
#include "Common/Modules/Track.h"
#include "Common/Modules/Pattern.h"
#include "Common/Modules/Instrument.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/SoundFontProgram.h"
#include "Firmware/Application/Model/SettingsManager/SettingsSculptModeCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsBRayCnfg.h"
#include "Firmware/Application/Model/SongManager.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"

extern std::unique_ptr<SettingsSculptModeCnfg> _settingsSculptModeCnfg;
//#define SCULPT_WITH_TIME
using namespace Ignite;
//class SongManager;
//class IgniteEngine;

std::map <int8_t, std::string> tvTrackDefaultNameMap = //TODO: Keep data in one place; Instrument has recrsive include issue with all "tv" params; Modelparam.h and EngineDataStruct.h; Track may not has instrument assigned
{
	//0 - 7 External sound
	{(int8_t)0,  "CC#72" },
	{(int8_t)1,  "CC#20" },
	{(int8_t)2,  "CC#71" },
	{(int8_t)3,  "CC#7"  },
	{(int8_t)4,  "CC#91" },
	{(int8_t)5,  "CC#93" },
	{(int8_t)6,  "CC#21" },
	{(int8_t)7,  "CC#22" },
	//8 - 15 Internal sound
	{(int8_t)8,  "Release"    },
	{(int8_t)9,  "Cutoff"     },
	{(int8_t)10, "Resonance"  },
	{(int8_t)11, "Volume"     },
	{(int8_t)12, "Reverb" },
	{(int8_t)13, "Chorus" },
	{(int8_t)14, "EQ High"    },
	{(int8_t)15, "EQ Low"     },

	//16 - 23 Internal sound alt function
	{(int8_t)16,  "Release"    },
	{(int8_t)17,  "Cutoff"     },
	{(int8_t)18, "Resonance"  },
	{(int8_t)19, "Volume"     },
	{(int8_t)20, "Reverb" },
	{(int8_t)21, "Chorus" },
	{(int8_t)22, "EQ High"    },
	{(int8_t)23, "EQ Low"     }
};
std::map <int8_t, TVMidiCCParams> tvTrackDefaultCCMap =
{  //{knob id    {midinum  init, Max, min, value}
	//0-7 External sound
	{(int8_t)0, {(int8_t)72, (int8_t)0, (int8_t)127, (int8_t)0, (float)0.0}},
	{(int8_t)1, {(int8_t)74, (int8_t)0, (int8_t)127, (int8_t)0, (float)0.0}},
	{(int8_t)2, {(int8_t)71, (int8_t)0, (int8_t)127, (int8_t)0, (float)0.0}},
	{(int8_t)3, {(int8_t)7,  (int8_t)0, (int8_t)127, (int8_t)0, (float)0.0}},
	{(int8_t)4, {(int8_t)91, (int8_t)0, (int8_t)127, (int8_t)0, (float)0.0}},
	{(int8_t)5, {(int8_t)93, (int8_t)0, (int8_t)127, (int8_t)0, (float)0.0}},
	{(int8_t)6, {(int8_t)21, (int8_t)0, (int8_t)127, (int8_t)0, (float)0.0}},
	{(int8_t)7, {(int8_t)22, (int8_t)0, (int8_t)127, (int8_t)0, (float)0.0}},
	//8-15 Internal sound
	{(int8_t)8,  {(int8_t)72, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //ADSRRelease
	{(int8_t)9,  {(int8_t)74, (int8_t)0, (int8_t)0, (int8_t)0, (float)20.0}}, //Filtercutoff
	{(int8_t)10, {(int8_t)71, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //Resonance
	{(int8_t)11, {(int8_t)7,  (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //Volume
	{(int8_t)12, {(int8_t)91, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //Reverb
	{(int8_t)13, {(int8_t)93, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //Chorus
	{(int8_t)14, {(int8_t)21, (int8_t)0, (int8_t)0, (int8_t)0, (float)8000.0}}, //EQH(undef)
	{(int8_t)15, {(int8_t)22, (int8_t)0, (int8_t)0, (int8_t)0, (float)20.0}}, //EQL(undef)
	//16-23 Internalsound alt function
	{(int8_t)16, {(int8_t)72, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //ADSRRelease
	{(int8_t)17, {(int8_t)74, (int8_t)0, (int8_t)0, (int8_t)0, (float)12.0}}, //Filtercutoff
	{(int8_t)18, {(int8_t)71, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //Resonance
	{(int8_t)19, {(int8_t)7,  (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //Volume
	{(int8_t)20, {(int8_t)91, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //Reverb
	{(int8_t)21, {(int8_t)93, (int8_t)0, (int8_t)0, (int8_t)0, (float)1.0}}, //Chorus
	{(int8_t)22, {(int8_t)21, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}, //EQH(undef)
	{(int8_t)23, {(int8_t)22, (int8_t)0, (int8_t)0, (int8_t)0, (float)0.0}}  //EQL(undef)
};

//TODO - check performance, if there is issue with number of threads, we can switch to thread pool or queue
//Currently configured to call at every step interval.
//Maximum 64 tracks run at same time
static void seqMgrNotifyAndUpdate(SequenceManager* seqMgr, std::string patternName, int8_t barId, int16_t stepId)
{
	if (seqMgr != nullptr)
	{
		//This is generic update. Will scan the pattern and regenerate the events including both notes and auxes
		PatternBarStepInfo stepAncestory{ patternName, barId, stepId };

		seqMgr->handleRecordChange(stepAncestory);
	}
}

Track::Track (int8_t id, std::string name, bool mute, SongManager* songmgr)
{
	_trackData = new (TrackData);
	*_trackData = { 0 };
	_trackData->trackSolo = false;//default state for new track
	_trackData->trackRecordEnable = false;
	_trackData->trackName = name;
	_trackData->trackId = id;
	_trackData->trackMute = mute;
	_trackData->trackRxInstrument = nullptr;
	_trackData->trackTxInstrument = nullptr;
	_trackData->mainPattern = nullptr;
	_trackData->fillPattern = nullptr;
	_trackData->tvTrackDataCount = 0;
	_songManager = songmgr;
	initTvCcKnobValue();
}

Track::Track (const Track& source) :
	Track (source._trackData->trackId, source._trackData->trackName, source._trackData->trackMute, source._songManager)
{
	this->_trackData->trackSolo = source._trackData->trackSolo;
	this->_trackData->trackRecordEnable = source._trackData->trackRecordEnable;
	this->_trackData->trackMute = source._trackData->trackMute;
	this->_trackData->trackRxInstrument = source._trackData->trackRxInstrument;
	this->_trackData->trackTxInstrument = source._trackData->trackTxInstrument;
	this->_trackData->mainPattern = source._trackData->mainPattern;
	this->_trackData->fillPattern = source._trackData->fillPattern;
	this->_trackData->trackRxInstrumentName = source._trackData->trackRxInstrumentName;
	this->_trackData->trackRxInstrumentDispName = source._trackData->trackRxInstrumentDispName;
	this->_trackData->trackTxInstrumentName = source._trackData->trackTxInstrumentName;
	this->_trackData->trackTxInstrumentDispName = source._trackData->trackTxInstrumentDispName;
	this->_trackData->mainPatternName = source._trackData->mainPatternName;
	this->_trackData->mainPatternDispName = source._trackData->mainPatternDispName;
	this->_trackData->fillPatternName = source._trackData->fillPatternName;
	this->_trackData->fillPatternDispName = source._trackData->fillPatternDispName;
	this->_trackData->tvTrackData = source._trackData->tvTrackData;
	this->_trackData->tvTrackDataCount = source._trackData->tvTrackDataCount;
	this->_pulsesPerStep = source._pulsesPerStep;
	this->_trackPlayStepId = source._trackPlayStepId;
	this->_trackStepIndex = source._trackStepIndex;
	this->_currentSceneBarId = source._currentSceneBarId;
	this->_stepCountForCurrentBar = source._stepCountForCurrentBar;

	this->_patternTimeBase = source._patternTimeBase;
	this->_patternSwing = source._patternSwing;

	this->_rxMidiPort = source._rxMidiPort;
	this->_rxMidiChannel = source._rxMidiChannel;

	this->_txMidiPort = source._txMidiPort;
	this->_txMidiChannel = source._txMidiChannel;

}

Track::~Track ()
{
	delete _trackData;
	tsf_close(static_cast<tsf*>(_trackSf));
}

void Track::setTrackRxInstrument (Instrument* val)
{
	if (val != nullptr)
	{
		InstrumentUpdated = true;
		_trackData->trackRxInstrument = val;
		_trackData->trackRxInstrumentName = val->getName ();
		_trackData->trackRxInstrumentDispName = val->getDispName ();
	}
	else
	{
		_trackData->trackRxInstrumentName = " ";
		_trackData->trackRxInstrumentDispName = " ";
	}
}
void Track::setTrackTxInstrument(Instrument* val)
{
	if (val != nullptr)
	{
		InstrumentOutUpdated = true;
		_trackData->trackTxInstrument = val;
		_trackData->trackTxInstrumentName = val->getName();
		_trackData->trackTxInstrumentDispName = val->getDispName();
	}
	else
	{
		_trackData->trackTxInstrumentName = " ";
		_trackData->trackTxInstrumentDispName = " ";
	}
}
void Track::setTrackMainPattern (Pattern* newPattern)
{
	if (_trackData == nullptr)
	{
		//TODO WARN
		return;
	}

	Pattern* oldPattern = _trackData->mainPattern;

	if (oldPattern != newPattern)
	{
		MainpatternUpdated = true;
		//if song  is in play/pause state
		if ((_songManager->_isSongPlaying == true) || (_songManager->_isSongPaused == true))
		{
			//deInitTrackPlaySetting ();
		}

		_trackData->mainPattern = newPattern;

		if (newPattern != nullptr)
		{
			_trackData->mainPatternName = newPattern->getName ();
			_trackData->mainPatternDispName = newPattern->PatternGetDispName();
		}
		else
		{
			_trackData->mainPatternName = " ";
			_trackData->mainPatternDispName = " ";
		}

		//if song is in pay/pause state
		//update pattern related params - TB
		if ((_songManager->_isSongPlaying == true) || (_songManager->_isSongPaused == true))
		{
			initTrackPlaySetting ();
		}
	}
}

void Track::setTrackFillPattern (Pattern* newPattern)
{
	Pattern* oldPattern = _trackData->fillPattern;

	if (oldPattern != newPattern)
	{
		FillpatternUpdated = true;
		//if song  is in play/pause state
		if ((_songManager->_isSongPlaying == true) || (_songManager->_isSongPaused == true))
		{
			//deInitTrackPlaySetting ();
		}

		_trackData->fillPattern = newPattern;

		if (newPattern != nullptr)
		{
			_trackData->fillPatternName = newPattern->getName ();
			_trackData->fillPatternDispName = newPattern->PatternGetDispName();
		}
		else
		{
			_trackData->fillPatternName = " ";
			_trackData->fillPatternDispName = " ";
		}

		//if song is in pay/pause state
		//update pattern related params - TB
		if ((_songManager->_isSongPlaying == true) || (_songManager->_isSongPaused == true))
		{
			initTrackPlaySetting ();
		}
	}
}

void Track::setTrackName (std::string name)
{
	_trackData->trackName.clear ();
	_trackData->trackName = name;
}



void Track::clearTrackPlayStepId (void)
{
	_trackPlayStepId = NOT_INITIALISED;
}

void Track::setTrackMuteStatus (uint8_t val)
{
	_trackData->trackMute = val; 
}

void Track::setTrackMuteStatusInMuteHoldMode (bool val)
{
	_muteHoldModeTrackMuteState = val;
}

bool Track::getTrackMuteStatusInMuteHoldMode (void)
{
	return _muteHoldModeTrackMuteState;
}

Instrument* Track::getTrackRxInstrument (void)
{
	return _trackData->trackRxInstrument;
}
Instrument* Track::getTrackTxInstrument(void)
{
	return _trackData->trackTxInstrument;
}

Pattern* Track::getTrackFillPattern (void)
{
	return _trackData->fillPattern;
}

Pattern* Track::getTrackMainPattern (void)
{
	return _trackData->mainPattern;
}

//get() has a wrapper to identify the current pattern(main or fill)
//set() is individually handled
Pattern* Track::getTrackPattern (void)
{
	if (shouldPlayFillPattern () == true)
	{
		return this->getTrackFillPattern ();
	}
	else
	{
		return getTrackMainPattern ();
	}
}

std::string Track::getTrackName (void)
{
	return _trackData->trackName;
}

int16_t Track::getTrackTotalSteps (void)
{
	Pattern* pattern = getTrackPattern ();
	if (pattern != nullptr)
	{
		return pattern->getTotalSteps ();
	}
	else
	{
		//TODO WARN
		return 0;
	}
}

int16_t Track::getTrackPlayStepId (void)
{
	return _trackPlayStepId;
}

Step* Track::getTrackPlayStep (void)
{
	Pattern* pattern = getTrackPattern ();
	if (pattern != nullptr)
	{
		uint16_t playStepId = getTrackPlayStepId();

		uint16_t playStepBarId = playStepId / MAX_STEPS_IN_PATTERN_BAR;

		auto barIter = pattern->_patternData->barList.find (playStepBarId);
		if (barIter != pattern->_patternData->barList.end())
		{
			auto stepIter = barIter->second._BarData->stepList.find (playStepId);
			if (stepIter != barIter->second._BarData->stepList.end())
			{
				return &(stepIter->second);
			}
			else
			{
				DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR : Unable to locate play step " + String (playStepId));
			}
		}
		else
		{
			DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR : Unable to locate play step bar " + String (playStepBarId));
		}
	}
	return nullptr;
}




bool Track::getTrackMuteStatus (void)
{
	return _trackData->trackMute;
}

int8_t Track::getTrackId (void)
{
	return _trackData->trackId;
}

void Track::setTrackId (int8_t id)
{
	_trackData->trackId = id;
}

#if 0
init - (play/record)
	_pulsesPerStep = set from pattern
	_trackPulse = 0
	_trackUiTick = 0
	_pulsesPerTick = set from pattern
	_trackBarId = -1
	_trackStepId = -1

pulse_loop(768th div)
	tick loop(32nd div)
		- if first pulse of the step (_trackPulse == 0)
			- moveToNextStep
				- get pattern step
				- check for bar repeat
				- check for bar loop
				- check for gbar sync
			- handle TimeBase Update
			- cv op (cv / envelope)
			- gate op

		- handlePlay
			- get step events for the tick
			- apply Transpose & Fts for note events
			- send events to output

		- handleRecord

	
#endif

#if defined (__linux__)  && defined (TIMER_TEST_PLAY)
void Track::timerStart(int id)
{
	int timerIndex = eventTimerMap[id].index;
	if(timerIndex < TIME_SAMPLES)
	{
		clock_gettime(CLOCK_MONOTONIC_RAW, &(eventTimerMap[id].tp_arr[timerIndex].startTime));
	}
}

void Track::timerStop(int id)
{
	int timerIndex = eventTimerMap[id].index;
	if(timerIndex < TIME_SAMPLES)
	{
		clock_gettime(CLOCK_MONOTONIC_RAW, &(eventTimerMap[id].tp_arr[timerIndex].endTime));
		++(eventTimerMap[id].index);
	}
}

void Track::timerPrint(void)
{
	printf("Mute status Track %d -> %s\n", _trackData->trackId, _trackData->trackMute== true ? "TRUE": "FALSE");
	fflush(stdout);
	if (_trackData->trackMute == false)
	{
		printf("Logging Track %d\n", _trackData->trackId);

		for(int id = 1; id <= 22; ++id)
		{
			std::string fileName = "/home/ignite/" + std::to_string(_trackData->trackId) + "_" + std::to_string(id);

			int fd = open(fileName.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0666);
			if(fd < 0)
			{
				printf("opening file :%s: failed \n", fileName.c_str());
				fflush(stdout);
				continue;
			}

			int max_samples_logged = eventTimerMap[id].index;
			int k = 0;
			long int longest = 0;
			for(k = 0; k < max_samples_logged; ++k)
			{
				struct timespec st = eventTimerMap[id].tp_arr[k].startTime;
				struct timespec en = eventTimerMap[id].tp_arr[k].endTime;

				long int diff_sec = en.tv_sec - st.tv_sec;

				long int diff_nsec = 0;

				if(diff_sec > 0)
				{
					diff_nsec = 999999999 - st.tv_nsec + en.tv_nsec;
				}
				else
				{
					diff_nsec = en.tv_nsec - st.tv_nsec;
				}

				diff_sec = diff_sec > 0 ? (diff_sec -1) : 0;
				long int total_diff = (diff_sec * 1000000000) + diff_nsec;
				if(total_diff > longest)
				{
					longest = total_diff;
				}
				
				char str[12];
				memset(str, 0x00, sizeof(str));
				snprintf(str, sizeof(str) - 3, "%d", total_diff);
				int len = strlen(str);
				str[len] = '\r';
				str[len + 1] = '\n';
				str[len + 2] = '\0';
					
				int ret = write(fd, str, len+2);
				if(ret < (len+2))
				{
					printf("Write error %d\n", ret);
					fflush(stdout);
				}
			}
			printf("Logged %d samples for event %d : max time %ld\n", k, id, longest);
			close(fd);
		}
	}
}

void Track::timerClear(void)
{
	printf("CLearing Track %d timer\n", _trackData->trackId);
	fflush(stdout);
	for(int id = 0; id < 13; ++id)
	{
		for(int k = 0; k < TIME_SAMPLES; ++k)
		{
			struct timespec st = eventTimerMap[id].tp_arr[k].startTime;
			struct timespec en = eventTimerMap[id].tp_arr[k].endTime;

			st.tv_sec = 0;
			st.tv_nsec = 0;

			en.tv_sec = 0;
			en.tv_nsec = 0;
		}
	}
}

#endif /* __linux__ && TIMER_TEST_PLAY */
void Track::pulse(int16_t sceneBarId, int16_t sceneStepId)
{
	if ( (_songManager->_state == SYSTEM_STATE::PLAY) ||
		 ((_songManager->_state == SYSTEM_STATE::LIVE_RECORD) && (_songManager->_isRecording == true))
		)
	{
		if (_pulsesPerStep != 0) //pattern assigned
		{
			TIMER_START(1);
			auto pattern = getTrackPattern();
			TIMER_STOP(1);

			if (pattern)
			{
				TIMER_START(2);
				std::string patternName = pattern->getName();
				TIMER_STOP(2);

				TIMER_START(3);
				advanceTrackPulse();
				TIMER_STOP(3);

				//Take actions only at every tick of a step
				if (_trackPulse % _pulsesPerTick == 0)
				{
					TIMER_START(4);
					handleStepChange(patternName, sceneBarId, sceneStepId);
					TIMER_STOP(4);

					//First pulse of the step
					if (_trackPulse == 0)
					{
						TIMER_START(5);
						getSequenceStep(patternName);
						TIMER_STOP(5);

						TIMER_START(6);
						checkAndHandleTimeBaseUpdate(patternName);
						TIMER_STOP(6);
					}
					//Play output is generated only when track is un-muted
					if ((trackMuted() == false) && (soloModeCheck() == true))
					{
						TIMER_START(7);
						playStepEvents(patternName);
						TIMER_STOP(7);

						TIMER_START(8);
						handleCvGeneration(patternName, false);
						TIMER_STOP(8);

						TIMER_START(9);
						sculptUpdate(pattern);
						TIMER_STOP(9);

						TIMER_START(10);
						recordStepEvents();
						TIMER_STOP(10);
					}
					TIMER_START(11);
					advanceUiTick();
					TIMER_STOP(11);

					TIMER_START(12);
					handleSwing(pattern);
					TIMER_STOP(12);

					TIMER_START(13);
					actionsAtStepBoundary();
					TIMER_STOP(13);
				}//32nd division of step

			}//if (pattern)

		}//if pattern assigned and TB synced with timer

	}//if play or live_record

    completedPulses.fetch_add(1, std::memory_order_release);
#if 0
		handleMidiOutClockGeneration(pulse);
		handleAdsrOut(pulse);
		handleBrayControlForNodeParams (noteIter->second.stepVelocity);
#endif
}

uint8_t Track::getRandomData(uint8_t min, uint8_t max)
{
	if (min > max) {
		std::swap(min, max);
	}
	static std::random_device rDev;
	static std::default_random_engine rEng(rDev());
	std::uniform_int_distribution<unsigned int> dataInRange(min, max);
	return static_cast<uint8_t>(dataInRange(rEng));
}

void Track::sculptUpdate(Pattern* pattern)
{
	if (!_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptButtonOnState)
		return;

	if (_songManager->activeSong->getEditScene()->getEditTrackId() != getTrackId())
		return;

	std::list<ClientPatternStepData> _stepdataList = pattern->PatternGetStepData(_trackBarId);
	ClientPatternStepData* _stepdata = nullptr;
	uint8_t TempValue = 0;

	for (auto& stepIt : _stepdataList)
	{
		if (stepIt.stepId == _trackPlayStepId)
		{
			_stepdata = &stepIt;
			break;
		}
	}

	if (!_stepdata)
		return;

	if (_stepdata->nodeList.empty())
	{
		if (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptMode == SculptMode::randomAbsolute ||
			_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptMode == SculptMode::randomRelative)
		{
			auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeNoteStart;
			auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeNoteEnd;
			TempValue = getRandomData(rangeStart, rangeEnd);
			if (TempValue > MAX_NOTE)
				TempValue /= 2;

			ClientPatternNodeData nodeData;
			nodeData.nodeId = 0;
			nodeData.note = TempValue;
			nodeData.velocity = 60;
			nodeData.lenInSteps = 0;
			nodeData.lenInTicks = 10;
			nodeData.offset = 0;
			nodeData.onState = true;
			nodeData.step_id = _stepdata->stepId;
			_stepdata->nodeList[TempValue][0] = nodeData;
		}
	}

	if (_stepdata->nodeList.empty())
		return;

	for (auto nodeIt = _stepdata->nodeList.begin(); nodeIt != _stepdata->nodeList.end();)
	{
		bool nodeProcessed = false;
		auto& offsetNodeMap = nodeIt->second;

		if (static_cast<int8_t>(_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t>(SculptParameterSelect::velocity))
		{
			switch (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptMode)
			{
			case SculptMode::relative:
				TempValue = offsetNodeMap.begin()->second.velocity + _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRelativeVelocity;
				break;
			case SculptMode::absolute:
				TempValue = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptAbsoluteVelocity;
				break;
			case SculptMode::randomAbsolute:
				TempValue = getRandomData(_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteVelocityStart,
					_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteVelocityEnd);
				break;
			case SculptMode::randomRelative:
				TempValue = getRandomData(_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeVelocityStart,
					_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeVelocityEnd);
				break;
			default:
				break;
			}

			for (auto& noteIT : offsetNodeMap)
			{
				noteIT.second.velocity = (TempValue < MAX_NOTE_VELOCITY ? TempValue : MAX_NOTE_VELOCITY / 2);
			}
		}

		if (static_cast<int8_t>(_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t>(SculptParameterSelect::note))
		{
			switch (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptMode)
			{
			case SculptMode::relative:
				TempValue = offsetNodeMap.begin()->second.note + _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRelativeNote;
				break;
			case SculptMode::absolute:
				TempValue = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptAbsoluteNote;
				break;
			case SculptMode::randomAbsolute:
				TempValue = getRandomData(_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteNoteStart,
					_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteNoteEnd);
				break;
			case SculptMode::randomRelative:
				TempValue = getRandomData(_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeNoteStart,
					_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeNoteEnd);
				break;
			default:
				break;
			}

			if (TempValue > MAX_NOTE)
				TempValue /= 2;

			auto nodeData = std::move(offsetNodeMap);
			nodeIt = _stepdata->nodeList.erase(nodeIt);
			_stepdata->nodeList[TempValue] = std::move(nodeData);

			for (auto& it : _stepdata->nodeList[TempValue])
			{
				it.second.note = TempValue;
			}
			nodeProcessed = true;
		}

		if (!nodeProcessed)
		{
			++nodeIt;
		}
	}

	_songManager->engine.patternManager->PatternMngrSetStepData((moduleId_t)ModuleName::moduleSong, pattern->getName(), _trackBarId, _trackPlayStepId, *_stepdata, false);
}


float Track::cvGetNoteVoltsPerOctave(uint8_t voltSet, uint8_t noteToSend, uint8_t noteRangeStart, uint8_t noteRangeEnd)
{
	float voltageToset = 0.0f;
	if (noteToSend >= noteRangeEnd)
	{
		voltageToset = (float)voltSet;//note is more than MAX set Max volt
	}
	else if (noteToSend < noteRangeStart)
	{
		voltageToset = 0.0f; //note is less than noteMin
	}
	else
	{
		int position = noteToSend - noteRangeStart;
		voltageToset = static_cast<float>((position * 0.08333));//valid note  1/12  1V/Octive
	}
	return voltageToset;
}
float Track::cvGetNoteVoltsHertzPerVolt(uint8_t voltSet, uint8_t noteToSend, uint8_t noteRangeStart, uint8_t noteRangeEnd)
{
	float voltageToset = 0.0f;
	return voltageToset;
}

bool Track::handleCvGeneration(std::string patternName, bool isAllOff)
{
	bool proceed = false;
	//1. Handle change done on play and edit
	//2. Handle change in CV port or change to midi port
	//3. handle play stop/all off --- what to do for pause?
	//4. change in Law
	//5. track mute handle : isAllOff -- used off the track CV
	if ((_txMidiPort >= static_cast<uint8_t>(InstrumentPorts::cvPort1)) && (_txMidiPort <= static_cast<uint8_t>(InstrumentPorts::cvPort8)))
	{
		int8_t cvPortTostop = _trackCVdata.cvPortId - static_cast<uint8_t>(InstrumentPorts::cvPort1);//CV ports are counted from 0 - 7 in CV config module
		proceed = true;
		//Handle change in CV port
		if (_trackCVdata.cvPortId != _txMidiPort)
		{
			_songManager->setCvGateVoltage(-1, 0.0f, cvPortTostop, 0.0f);// no change in CV-Volt only gate off
			_trackCVdata.cvPortId = _txMidiPort;
			//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "  change in CV port to other CV Set voltage 0.0 gate OFF::");
		}
	}
	if ((!proceed || isAllOff ) && (_trackCVdata.cvPortId != (int8_t)InstrumentPorts::MaxMidiPort))
	{
		//Handle change in midi port and play-stop/all off
		int8_t cvPortTostop = _trackCVdata.cvPortId - static_cast<uint8_t>(InstrumentPorts::cvPort1);//CV ports are counted from 0 - 7 in CV config module
		_songManager->setCvGateVoltage(-1, 0.0f, cvPortTostop, 0.0f);// no change in CV-Volt only gate off
		_trackCVdata.cvPortId = (int8_t)InstrumentPorts::MaxMidiPort;
	}

	if (proceed)
	{
		int8_t cvPortForThisTrack = _txMidiPort - static_cast<uint8_t>(InstrumentPorts::cvPort1);//CV ports are counted from 0 - 7 in CV config module
		int8_t noteRangeStart = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[cvPortForThisTrack].noteRangeStart;
		int8_t voltSet = (int8_t)_songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[cvPortForThisTrack].voltage;
		int8_t noteRangeEnd = noteRangeStart + (voltSet * 12);// should with (-1)
		uint8_t noteOn = (uint8_t)Note::noteInvalid;
		//StepEvents get the events for the step
		OffsetMidiEventMap StepEvents = _songManager->engine.sequenceManager->getStepEvents(patternName, _trackBarId, _trackPlayStepId);
		std::vector<MidiEvent> midiEvent;
		auto eventIter = StepEvents.find(_trackUiTick);
		if (eventIter != StepEvents.end())
		{
			midiEvent = eventIter->second;
		}
		if (!_trackCVdata.suppressedNotesOn.empty())
		{
			auto eraseItStart = _trackCVdata.suppressedNotesOn.find(noteRangeStart); // user changed the note start?
			if (eraseItStart != _trackCVdata.suppressedNotesOn.end())
			{
				_trackCVdata.suppressedNotesOn.erase(_trackCVdata.suppressedNotesOn.begin(), eraseItStart);
			}
			if (!_trackCVdata.suppressedNotesOn.empty())
			{
				auto eraseItEnd = _trackCVdata.suppressedNotesOn.find(noteRangeEnd); // user changed the note end?
				if (eraseItEnd != _trackCVdata.suppressedNotesOn.end())
				{
					_trackCVdata.suppressedNotesOn.erase(eraseItEnd, _trackCVdata.suppressedNotesOn.end());
				}
			}
		}

		uint8_t noteToSelect = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[cvPortForThisTrack].cvNoteType;
		switch (noteToSelect)
		{
		case (uint8_t)CvNoteType::LowNoteSet:
		case (uint8_t)CvNoteType::HighNoteSet:
		{
			noteOn = (noteToSelect == (uint8_t)CvNoteType::LowNoteSet ? (uint8_t)Note::noteInvalid : (uint8_t)Note::noteMin);
			for (auto& it : midiEvent)
			{
				if ((it.type & 0xF0) == 0x90)
				{
					if (it.data1 >= noteRangeStart && it.data1 < noteRangeEnd)//in range
					{
						if (noteToSelect == (uint8_t)CvNoteType::LowNoteSet && noteOn > it.data1)
							noteOn = it.data1;
						else if (noteToSelect == (uint8_t)CvNoteType::HighNoteSet && noteOn < it.data1)
							noteOn = it.data1;
						_trackCVdata.suppressedNotesOn[noteOn] = _trackUiTick;
					}
					noteOn = it.data1;//If no note in range this will be used to set CV max or min
				}
				else if ((((it.type & 0xF0) == 0x90) && (it.data2 == 0)) || ((it.type & 0xF0) == 0x80))//Note OFF
				{
					if (it.data1 >= noteRangeStart && it.data1 < noteRangeEnd)//in range
						_trackCVdata.suppressedNotesOn.erase(it.data1);
				}
			}
		}
		break;
		case (uint8_t)CvNoteType::LastNoteSet:
		{
			bool found = false;
			noteOn = (uint8_t)Note::noteInvalid;
			for (auto itStepReverse = StepEvents.rbegin(); itStepReverse != StepEvents.rend(); ++itStepReverse)
			{
				for (auto& it : itStepReverse->second)
				{
					if ((it.type & 0xF0) == 0x90)
					{
						if (it.data1 >= noteRangeStart && it.data1 < noteRangeEnd)//in range
						{
							noteOn = it.data1;
							_trackCVdata.suppressedNotesOn[noteOn] = _trackUiTick;
							found = true;
							break;
						}
						noteOn = it.data1;//If no note in range this will be used to set CV max or min
					}
					else if ((((it.type & 0xF0) == 0x90) && (it.data2 == 0)) || ((it.type & 0xF0) == 0x80))//Note OFF
					{
						if (it.data1 >= noteRangeStart && it.data1 < noteRangeEnd)//in range
							_trackCVdata.suppressedNotesOn.erase(noteOff);
					}
				}
				if (found)//reverse search and find a valid note on
					break;
			}
		}
		break;
		}
		//got all valid note on at _trackCVdata.suppressedNotesOn
		//noteOn - If no note in range this will be used to set CV max or min; not in range but has a note
		if (_trackCVdata.suppressedNotesOn.size())
		{
			//no note on this tick, look on the suppressed Notes
			if (noteToSelect == (uint8_t)CvNoteType::LowNoteSet)
				noteOn = _trackCVdata.suppressedNotesOn.begin()->first;
			else if (noteToSelect == (uint8_t)CvNoteType::HighNoteSet)
				noteOn = _trackCVdata.suppressedNotesOn.rbegin()->first;
			else if (noteToSelect == (uint8_t)CvNoteType::LastNoteSet)
				noteOn = _trackCVdata.suppressedNotesOn.rbegin()->first;
		}
		if (noteOn == (uint8_t)Note::noteInvalid)
		{
			if (_trackCVdata.gatestate)
			{
				//uint8_t gateData = (((cvPortForThisTrack & 0xF0) << 4) | 0); // need?
				_songManager->setCvGateVoltage(-1, 0.0f, cvPortForThisTrack, 0.0f);// no change in CV-Volt only gate off
				_trackCVdata.gatestate = false;//nothing for CV, nothing in suppressedNotesOn
			}
			//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "  Set voltage 0.0 gate OFF::" + String(noteOn));Play time print
		}
		else
		{
			float voltageToset = 0.0f;
			CvLaw law = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[cvPortForThisTrack].law;
			if (law == CvLaw::voltsPerOctave)
			{
				voltageToset = cvGetNoteVoltsPerOctave(voltSet, noteOn, noteRangeStart, noteRangeEnd);
				//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "  Set voltage gate ON::" + String(voltageToset) + String(noteOn)); Play time print
			}
			if (law == CvLaw::hertzPerVolt)
			{
				voltageToset = cvGetNoteVoltsHertzPerVolt(voltSet, noteOn, noteRangeStart, noteRangeEnd);
			}
			// Send to CV module
			_songManager->engine.SendResponse((moduleId_t)ModuleName::moduleTrack, (moduleId_t)ModuleName::moduleUI,
				(paramId_t)((uint8_t)TrackParam::paramCvVoltsPort1 + cvPortForThisTrack),
				voltageToset);

			int8_t GatevoltSet = (int8_t)_songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.gateConfig[cvPortForThisTrack].output;
			_songManager->setCvGateVoltage(cvPortForThisTrack, voltageToset, cvPortForThisTrack, GatevoltSet);
			_trackCVdata.gatestate = true;
		}
	}
	return true;
}

bool Track::getTxInstrumentTransposeDefeatStatus (void)
{
	Instrument* instrumentToPLay = getTrackTxInstrument ();
	if (instrumentToPLay == nullptr)
	{
		return true; //default no transpose defeat
	}

	return instrumentToPLay->getTransposeDefeatStatus ();
}

bool Track::getTxInstrumentFtsDefeatStatus (void)
{
	Instrument* instrumentToPLay = getTrackTxInstrument ();
	if (instrumentToPLay == nullptr)
	{
		return true; //default no FTS defeat
	}

	return instrumentToPLay->getFtsDefeatStatus ();
}


bool Track::getNodeTransposeDefeatStatus (void)
{
	Pattern* patternToPlay = getTrackPattern ();
	if (patternToPlay == nullptr)
	{
		//TODO WARN
		return true; //default no transpose defeat
	}

	Step* playStep = getTrackPlayStep ();
	if (playStep == nullptr)
	{
		return true; //default transpose defeat true
	}

	//TODO add multi node support
	return playStep->getNodeTransposeDefeat ();
}

int8_t Track::getRxInstrumentPort (void)
{
	Instrument* instrumentToPLay = getTrackRxInstrument ();
	if (instrumentToPLay == nullptr)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Track instrument not valid");
		return -1;
	}

	return instrumentToPLay->getMidiPort ();
}

int8_t Track::getRxInstrumentChannel (void)
{
	Instrument* rxInstrument = getTrackRxInstrument ();
	if (rxInstrument == nullptr)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Track instrument not valid");
		return -1;
	}

	return rxInstrument->getMidiChannel ();
}

int8_t Track::getTxInstrumentPort(void)
{
	Instrument* instrumentToPLay = getTrackTxInstrument();
	if (instrumentToPLay == nullptr)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR Track instrument not valid");
		return -1;
	}

	return instrumentToPLay->getMidiPort();
}

int8_t Track::getTxInstrumentChannel(void)
{
	Instrument* instrumentToPlay = getTrackTxInstrument();
	if (instrumentToPlay == nullptr)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR Track instrument not valid");
		return -1;
	}

	return instrumentToPlay->getMidiChannel();
}

//Return direction for all bars
std::vector <PatternDirection> Track::getPatternDirection (void)
{
	Pattern* patternToPlay = getTrackPattern ();
	if (patternToPlay == nullptr)
	{
		assert (0);
	}

	return patternToPlay->getPlayDirection ();
}

bool Track::getRxInstrumentData (void)
{
	Instrument* rxInstrument = getTrackRxInstrument ();
	if (rxInstrument == nullptr)
	{
		return false;
	}
	_rxMidiPort = rxInstrument->getMidiPort ();
	_rxMidiChannel = rxInstrument->getMidiChannel ();

	return true;
}
bool Track::getTxInstrumentData(void)
{
	Instrument* instrumentToPLay = getTrackTxInstrument();
	if (instrumentToPLay == nullptr)
	{
		return false;
	}
	_txMidiPort = instrumentToPLay->getMidiPort();
	_txMidiChannel = instrumentToPLay->getMidiChannel();

	return true;
}

void Track::initTrackPlaySetting (void)
{
	Pattern* patternToPlay = getTrackPattern ();

	if (patternToPlay == nullptr)
	{
		//TODO WARN
		return;
	}

	_patternTimeBase = patternToPlay->getTimeBase();
	if (_patternTimeBase >= TimeBase::Max)
	{
		_patternTimeBase = TimeBase::Base_4; //default to quarter note, if value out of range
	}

	_pulsesPerStep = timeBaseToPps(_patternTimeBase);
	_pulsesPerTick = SET_PPQN / TB4_TICKS_PER_STEP;
	_trackPlayStepId = -1;
	_trackBarId = -1;
	_trackUiTick = 0;

	_newStep = false;
	_trackPulse = -1;
	
	_trackStepIndex = -1;
	_currentSceneBarId = -1;
	_stepCountForCurrentBar = _songManager->engine.sequenceManager->getStepCountForFirstBar(patternToPlay->getName());




	//barLoopStateNRange = std::make_tuple(patternToPlay->getPatternBarLoopStatus(), patternToPlay->getPatternBarLoopStartBarId(), patternToPlay->getPatternBarLoopEndBarId());

	for (auto& bar : patternToPlay->_patternData->barList)
	{
		barRepeatCountMap.emplace(bar.second._BarData->BarId, bar.second.getbarRepeatCount());
	}

	setTrackTxMidiPortAndChannel();

	/*
	_trackStatePlayOrRecord = TrackState::TrackPlayState;

	_patternSwing = 0;

	*/

	_noteOnList.clear();

	//Internal audio - sound font
	sfLoad();

	//Internal audio - effects
	resetEffects();
	effectsInit();

	//Live record is always associated with play
	//Currently it resets the loop tracking variables, for record overwrite control
	//So this is the best place to call.
	//Move else where if requred in future
	initTrackRecordSetting();
}

void Track::initTrackRecordSetting(void)
{
	_recordFirstStep = -1;
	_recordFirstBar = -1;

	Pattern* patternToPlay = getTrackPattern();

	if (patternToPlay != nullptr)
	{
		_stepCountForCurrentBar = _songManager->engine.sequenceManager->getLinearStepCountForFirstBar(patternToPlay->getName());
	}
}

uint16_t Track::timeBaseToPps (TimeBase tb)
{
	uint16_t ret = SET_PPQN;

	switch (tb)
	{
	case TimeBase::Base_32T:
		ret = ((SET_PPQN / 8) * 2) / 3;
		break;

	case TimeBase::Base_32:
		ret = SET_PPQN / 8;
		break;

	case TimeBase::Base_16T:
		ret = ((SET_PPQN / 4) * 2) / 3;
		break;

	case TimeBase::Base_16:
		ret = SET_PPQN / 4;
		break;

	case TimeBase::Base_8T:
		ret = ((SET_PPQN / 2) * 2) / 3;
		break;

	case TimeBase::Base_8:
		ret = SET_PPQN / 2;
		break;

	case TimeBase::Base_4T:
		ret = (SET_PPQN * 2) / 3;
		break;

	case TimeBase::Base_4:
		ret = SET_PPQN;
		break;

	case TimeBase::Base_2T:
		ret = ((SET_PPQN * 2) * 2) / 3;
		break;

	case TimeBase::Base_2:
		ret = SET_PPQN * 2;
		break;

	case TimeBase::Base_1:
		ret = SET_PPQN * 4;
		break;

	default:
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR Inavlid time base");
		ret = SET_PPQN;
		break;
	}

	return ret;
}

TimeBase Track::getTrackPatternTimeBase(void)
{
	return _patternTimeBase;
}

void Track::deInitTrackPlaySetting (void)
{
	std::lock_guard<std::mutex> lock(_NoteListMtx);
	sfReset();
	_trackPulse = -1;
	_playingNoteList.clear();
#if 0
	Instrument* instrumentToPlay = getTrackTxInstrument();
	if (instrumentToPlay == nullptr)
	{
		//TODO WARN
		return;
	}

	//Stop external device from sustaining any notes, which are not turned off
	auto internalSoundMode =  instrumentToPlay->_instrumentData->internalSound;
	auto channel = instrumentToPlay->getMidiChannel();
	auto port = instrumentToPlay->getMidiPort();
	MidiMessage msg;
	msg = SongManager::allNotesOff(static_cast <int> (channel));
	_songManager->sendMidiMessage(msg, port, internalSoundMode);
	msg = SongManager::allSoundOff(static_cast <int> (channel));
	_songManager->sendMidiMessage (msg, port, internalSoundMode);
#endif
		TIMER_PRINT();
		TIMER_CLEAR();
}

void Track::recordStepData(int16_t stepId, int8_t barId, TickOffset_t tick)
{

	bool ret = false;
	std::vector<midiData> data;
	do
	{
		midiData tmp;
		ret = recordData.read(tmp);
		if (ret == true)
		{
			if (isValidNoteEvent (tmp) == true)
			{
				data.push_back (tmp);
				//juce::Logger::outputDebugString ("----------" + String (stepId) + "---------" + String(tick));
				//juce::Logger::outputDebugString ("Accept :" + (String(tmp.statusByte)) + ":" + (String(tmp.dataByte1)) + ":" + (String(tmp.dataByte2)));
			}
		}

	}while(ret == true);


	auto offset = calculateOffsetWithinStep(tick);

	for (auto& noteEvent : data)
	{
		if ( ( (noteEvent.statusByte & 0xF0) == 0x90) && (noteEvent.dataByte2 != 0))//Note ON
		{
			bool onCaptured = false;
			int8_t nodeCount = 0;
			ClientPatternNodeData recordedNodeData = { 0 };
			ClientPatternStepData recordedStepData = { 0 };

			recordedNodeData.offset = offset;
			recordedNodeData.lenInSteps = 1;
			recordedNodeData.lenInTicks = 0;
			recordedNodeData.nodeId = nodeCount;
			recordedNodeData.note = noteEvent.dataByte1;
			recordedNodeData.velocity = noteEvent.dataByte2;
			recordedNodeData.onState = true;
			recordedStepData.nodeList[recordedNodeData.note][recordedNodeData.offset] = recordedNodeData;

			++nodeCount;
			//juce::Logger::outputDebugString ("ON :" + (String (recordedNodeData.note)) + ":" + (String(recordedNodeData.velocity)) + ": D:" + (String(recordedNodeData.delay)));

			//juce::Logger::outputDebugString ("B_"+to_string (bardId) + " : S_" + to_string (stepId) + " : nodes = " + to_string (nodeCount));
			if (nodeCount > 0)
			{
				recordedStepData.BarId = barId;
				recordedStepData.nodeCount = nodeCount;
				recordedStepData.stepId = stepId;

				recordedStepData.enable = true;
				recordedStepData.skipStatus = false;
				recordedStepData.transposeDefeatStatus = false;
				recordedStepData.tieStatus = false;

				Pattern* patternToRecord = getTrackPattern ();
				if (patternToRecord == nullptr)
				{
					DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get Track's rec pattern");
					return;
				}

				auto bar = patternToRecord->_patternData->barList.find (barId);
				if (bar != patternToRecord->_patternData->barList.end())
				{
#if 0
					printf ("NON : Bar %2d\n", bar->second._BarData->BarId);
					DBG ("NON : Bar " + String (bar->second._BarData->BarId));
#endif

					auto step = bar->second._BarData->stepList.find (stepId);

					if (step != bar->second._BarData->stepList.end())
					{
#if 0
						printf ("NON : Step %2d\n", step._stepData->stepId);
						DBG ("NON : Step " + String(step->second._stepData->stepId));
#endif
						step->second.SetStepDataDuringRecord (recordedStepData);
						//Pattern Updated - Node added
						_patternUpdatedByRecord.store(true, std::memory_order_release);
						onCaptured = true;
					}
				}
			}/* if (nodeCount > 0) */
		} /* If NOTE ON */
		else if ( ( ( (noteEvent.statusByte & 0xF0) == 0x90) && (noteEvent.dataByte2 == 0)) || ((noteEvent.statusByte & 0xF0) == 0x80))//Note OFF
		{
			bool pairFound = false;
			uint16_t endStepId = static_cast<uint16_t>(stepId);
			TickOffset_t endTick = tick;

			Pattern* patternToRecord = getTrackPattern ();
			if (patternToRecord == nullptr)
			{
				DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get Track's rec pattern");
				return;
			}

			auto checkBarId = barId;
			uint16_t checkedBarsCount = 0;
			bool looped = false;
			do
			{
				auto barIter = patternToRecord->_patternData->barList.find (checkBarId);
				if (barIter != patternToRecord->_patternData->barList.end())
				{
					//Search as less as possible
					//We record from bar 0 - 15.
					//Normal recoring scenario the note ON would be in current bar or in any previous bar
					//Start from  current bar and then search for the Note ON

#if 0
					printf("NOFF : Bar %2d\n", barIter->second._BarData->BarId);
					DBG ("NOFF : Bar " + String(barIter->second._BarData->BarId));
#endif

					auto stepStartR = barIter->second._BarData->stepList.rbegin();
					auto stepEndR = barIter->second._BarData->stepList.rend();
					for (auto& stepIter = stepStartR; stepIter != stepEndR; ++stepIter)
					{
						//Search as less as possible
						//We record from step 0 - 15
						//Normal recoring scenario the corresponding note ON would be in current step or in any previous step
						//Start from last step and reach the current step and then search for the Note ON

						if (stepIter->second._stepData->stepId > stepId)
						{
							continue;
						}
						auto& step = stepIter->second;
#if 0
						printf("NOFF : Step %2d\n", step._stepData->stepId);
						DBG ("NOFF : Step " + String(step._stepData->stepId));
#endif
						auto note = noteEvent.dataByte1;
						if (step._stepData->nodeList.count(note))
						{
							for (auto& samePitchNodeWithinStep : step._stepData->nodeList[note])
							{
								if (samePitchNodeWithinStep.second.onState == true)
								{

									samePitchNodeWithinStep.second.onState = false;

									uint16_t startStepId = step._stepData->stepId;
									TickOffset_t startTick = samePitchNodeWithinStep.second.offset;
									//juce::Logger::outputDebugString ("Start : " + (to_string (startStepId)) + ":" + (to_string (startTick)));
									//juce::Logger::outputDebugString ("End   : " + (to_string (endStepId)) + ":" + (to_string (endTick)));
									if (startStepId == endStepId)
									{
										samePitchNodeWithinStep.second.lenInSteps = 0;
										samePitchNodeWithinStep.second.lenInTicks = endTick - startTick;
									}
									else
									{
										if (endTick < startTick)
										{
											samePitchNodeWithinStep.second.lenInSteps = (uint8_t)(endStepId - startStepId - 1);
											samePitchNodeWithinStep.second.lenInTicks = endTick + (patternToRecord->getTicksPerStep() - startTick);
										}
										else
										{
											samePitchNodeWithinStep.second.lenInSteps = (uint8_t)(endStepId - startStepId);
											samePitchNodeWithinStep.second.lenInTicks = endTick - startTick;
										}
									}
									//Pattern Updated - Node length
									_patternUpdatedByRecord.store(true, std::memory_order_release);

#if 0
									printf("NOFF : NId %2d  Not %2d Vel %3d Len %2d:%2d Del %2d\n", step._stepData->nodeList[note].note,
										step._stepData->nodeList[note].note, step._stepData->nodeList[note].velocity, step._stepData->nodeList[note].lenInSteps,
										step._stepData->nodeList[note].lenInTicks, step._stepData->nodeList[note].delay);

									DBG("PAT NOFF : NId " + String(step._stepData->nodeList[note].note) + " Not " + String(step._stepData->nodeList[note].note) + " Vel " + String(step._stepData->nodeList[note].velocity)
										+ " Len " + String(step._stepData->nodeList[note].lenInSteps) + ":" + String(step._stepData->nodeList[note].lenInTicks) + " Del " + String(step._stepData->nodeList[note].delay));
#endif
									//juce::Logger::outputDebugString ("PAT OFF :" + (String (tick)) + " : " + (String(note)) + ": S:" + (String(step._stepData->nodeList[note].lenInSteps)) + ": T:" + (String(step._stepData->nodeList[note].lenInTicks)));
									pairFound = true;
									break;

								}/* if (samePitchNodeWithinStep.second.onState == true) */

							}/* for (auto& samePitchNodeWithinStep : step._stepData->nodeList[note]) */

						} /* if ((step._stepData->nodeList.count(note)) */

					} /* Step loop */

					if (pairFound) break; //no need to search further
				}
				else
				{
					DBG (String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR : unable to locate bar" + String(checkBarId));
				}

				// Bar traversal, go to next lower bar
				// Add check (loop exit) to Stop if we reach the same bar (this means we have traversed entire pattern once)
				// Add additional check (loop exit) to loop only once over the pattern
				// Loop is required when overwrite is allowed (Note ON might be at a higher bar number than note OFF)
				// Add additional check (loop exit) to check only max number of bars in pattern.
				checkBarId = checkBarId - 1;
				++checkedBarsCount;
				if (checkBarId < 0)
				{
					if (looped == false)
					{
						looped = true;
						checkBarId = patternToRecord->_patternData->numBars - 1; //after 0th bar, resume search from end bar
					}
					else
					{
						DBG (String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR : NOTE OFF pair not found" +" "+String(noteEvent.dataByte1) + ":" + String(noteEvent.dataByte2));
						break;
					}
				}
				if (checkBarId == barId)
				{
					DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR : NOTE OFF pair not found" + " " + String(noteEvent.dataByte1) + ":" + String(noteEvent.dataByte2));
					break;
				}
			} while (checkedBarsCount <= patternToRecord->_patternData->numBars);
		}

		else if ( ( (noteEvent.statusByte & 0xF0) == (uint8_t)AuxEventType::afterTouchPolyPressure) ||
				  ( (noteEvent.statusByte & 0xF0) == (uint8_t)AuxEventType::controlChange) ||
				  ( (noteEvent.statusByte & 0xF0) == (uint8_t)AuxEventType::programChange) ||
				  ( (noteEvent.statusByte & 0xF0) == (uint8_t)AuxEventType::afterTouchChannelPressure) ||
				  ( (noteEvent.statusByte & 0xF0) == (uint8_t)AuxEventType::pitchBend) )
		{
			Pattern* patternToRecord = getTrackPattern ();
			if (patternToRecord == nullptr)
			{
				DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get Track's rec pattern");
				return;
			}


			auto bar = patternToRecord->_patternData->barList.find (barId);
			if (bar != patternToRecord->_patternData->barList.end())
			{
				auto step = bar->second._BarData->stepList.find (stepId);
				if (step != bar->second._BarData->stepList.end())
				{
					Aux tmp = { true, (uint16_t)noteEvent.statusByte, (int8_t)noteEvent.dataByte1, (int8_t)noteEvent.dataByte2, (int8_t)offset };
					step->second._stepData->auxList.push_back (tmp);
					//Pattern Updated - Aux
					_patternUpdatedByRecord.store(true, std::memory_order_release);
				}
				else
				{
					DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR : Unable to locate step " + String (stepId) + " in bar " + String(barId));
				}
			}
		}
	}
}

void Track::publishTrackBarAndStepInfoToUI(void)
{
	if ((_songManager == nullptr) || (_songManager->activeSong == nullptr) || (_songManager->activeSong->getEditScene () == nullptr))
	{
		return;
	}

	//send info to ui only if this Track is current edit Track
	if (_songManager->activeSong->getEditScene ()->getEditTrackId () == getTrackId ())
	{
		uint16_t data = 0;
		data = _trackBarId;
		data = (data << 8) | _trackPlayStepId;
		_songManager->engine.SendResponse ((moduleId_t)ModuleName::moduleTrack, (moduleId_t)ModuleName::moduleUI, (paramId_t)TrackParam::paramTrackPatternBarAndStepId, data);
	}
}

bool Track::getBarTransposeDefeatStatus (uint8_t barId)
{
	Pattern* pattern = getTrackPattern ();
	if (pattern != nullptr)
	{
		auto barIter = pattern->_patternData->barList.find (barId);
		if (barIter != pattern->_patternData->barList.end())
		{
			return barIter->second._BarData->barTransposeDefeat;
		}
		else
		{
			DBG (String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR Unable to get bar");
		}
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get Track's pattern");
	}

	return true; //default no transpose defeat
}

bool Track::getGbarSyncStatusForBar (int16_t barId)
{
	bool ret = false;
	Pattern* pattern = getTrackPattern ();
	if (pattern != nullptr)
	{
		auto barIter = pattern->_patternData->barList.find (barId);
		if (barIter != pattern->_patternData->barList.end())
		{
			ret =  barIter->second._BarData->gBarSyncOn;
		}
		else
		{
			DBG (String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR Unable to get bar");
		}
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get Track's pattern");
	}
	return ret;
}

void Track::setTrackRecordStatus(bool state)
{
	_trackData->trackRecordEnable = state;
}

bool Track::getTrackRecordStatus(void)
{
	return _trackData->trackRecordEnable;
}

void Track::setTrackSoloStatus (bool state)
{
	_trackData->trackSolo = state;
}

bool Track::getTrackSoloStatus (void)
{
	return _trackData->trackSolo;
}

bool Track::soloModeCheck (void)
{
	bool ret = false;
	bool systemSoloMode = getSystemSoloMode();
	if (systemSoloMode == true)
	{
		if (getTrackSoloStatus () == true)
		{
			ret = true;
		}
		else
		{
			ret = false;
		}
	}
	else
	{
		ret = true;
	}
	return ret;
}

bool Track::trackMuted (void)
{
	bool muted = true;

	// If mute hold mode (next or lock) is enabled, use mute status set for MuteHoldMode
	if (_songManager->getGlobalMuteHoldType () != MuteHoldType::Off)
	{
		muted = getTrackMuteStatusInMuteHoldMode ();
	}
	else
	{
		muted = getTrackMuteStatus ();
	}

	//fillpattern will play if track is muted and fill mode is active
	//so un-mute the track
	if (muted == true)
	{
		auto fillModeTriggered = _songManager->getFillModeTriggerState ();
		auto fillModeType = _songManager->getFillModeType ();

		if ((fillModeTriggered == true) && (fillModeType == static_cast <int8_t> (FillModeType::active)))
		{
			muted = false;
		}
	}

	//track will be muted, if fill is triggered and fill mode is mute
	//so mute the track
	if (muted == false)
	{
		auto fillModeTriggered = _songManager->getFillModeTriggerState ();
		auto fillModeType = _songManager->getFillModeType ();

		if ((fillModeTriggered == true) && (fillModeType == static_cast <int8_t> (FillModeType::mute)))
		{
			muted = true;
		}
	}

	return muted;
}

/*
* --------------------------------------------
* FILL TRIGGERED
* --------------------------------------------
* Regular mute  | Fill Mode   |  PatternToPlay
* --------------------------------------------
*	Muted       | Auto        |   x
*				| Active      |   Fill
*				| Mute        |   x
*				| No Fill     |   x
* --------------------------------------------
* Un-Muted      | Auto        |   Fill
*				| Active      |   Fill
*				| Mute        |   x
*				| No Fill     |   Main
* --------------------------------------------
*/
bool Track::shouldPlayFillPattern (void)
{
	bool ret = false;
	auto trackMutedStatus = trackMuted ();
	auto fillModeTriggered = _songManager->getFillModeTriggerState ();
	auto fillModeType = _songManager->getFillModeType ();

	if (fillModeTriggered == true)
	{
		if (trackMutedStatus == true)// track muted - play fill in active mode only
		{
			if (fillModeType == static_cast <int8_t> (FillModeType::active))
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
		}
		else //track unmuted - play fill in auto or active modes
		{
			if ((fillModeType == static_cast <int8_t> (FillModeType::automatic)) || (fillModeType == static_cast <int8_t> (FillModeType::active)))
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
		}
	}
	else
	{
		ret = false;
	}

	return ret;
}

bool Track::barIdWithinPattern (int16_t barId)
{
	Pattern* pattern = getTrackPattern ();
	if (pattern != nullptr)
	{
		auto barIter = pattern->_patternData->barList.find (barId);
		if (barIter != pattern->_patternData->barList.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR : Unable to get Track's pattern");
		return false;
	}
}

int16_t Track::getLastStepIdOfPattern (void)
{
	Pattern* pattern = getTrackPattern ();
	if (pattern != nullptr)
	{
		auto numBars = pattern->_patternData->numBars;
		auto lastBar = pattern->_patternData->barList.find (numBars - 1);
		if (lastBar != pattern->_patternData->barList.end())
		{
			auto lastStep = lastBar->second._BarData->stepList.rbegin();
			if (lastStep != lastBar->second._BarData->stepList.rend())
			{
				return lastStep->second.getStepId();
			}
			else
			{
				DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR : Unable to get last step of pattern");
				return 0;
			}
		}
		else
		{
			DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR : Unable to get last bar");
			return 0;
		}
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR : Unable to get Track's pattern");
		return 0;
	}
}

int16_t Track::getFirstStepIdOfPattern (void)
{
	Pattern* pattern = getTrackPattern ();
	if (pattern != nullptr)
	{
		auto firstBar = pattern->_patternData->barList.find (0);
		if (firstBar != pattern->_patternData->barList.end())
		{
			auto firstStep = firstBar->second._BarData->stepList.begin();
			if (firstStep != firstBar->second._BarData->stepList.end())
			{
				return firstStep->second.getStepId();
			}
			else
			{
				DBG (String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR : Unable to get first step of pattern");
				return 0;
			}
		}
		else
		{
			DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get first bar");
		}
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get Track's pattern");
	}
	return 0;
}


int16_t Track::getBarIdFromStepId (int16_t stepId)
{
	return stepId / 16;
}

void Track::setTvKnobType (int8_t knob, TrackValueType val)
{
	if (_trackData->tvTrackData.count (knob) != 0)
	{
		_trackData->tvTrackData[knob].paramType = val;

		//Copy instrument PGM CHANG data to track
		if (val == TrackValueType::TVTypePgmChange)
		{
			if (_trackData->trackTxInstrument->_instrumentData->tv.count (knob) != 0)
			{
				auto tv = _trackData->trackTxInstrument->_instrumentData->tv[knob];
				_trackData->tvTrackData[knob].param.tvPgmChange = tv.param.tvPgmChange;
			}
		}
	}
}

//Sets only the value indicated by Knob position
void Track::setTVPgmChangeKnobVal (int8_t knob, TrackValue val)
{
	if (_trackData->tvTrackData.count (knob) != 0)
	{
		if (val.paramType == TrackValueType::TVTypePgmChange)
		{
			if (_trackData->tvTrackData[knob].paramType == TrackValueType::TVTypePgmChange)
			{
				_trackData->tvTrackData[knob].param.tvPgmChange.midiPgmNum = val.param.tvPgmChange.midiPgmNum;
			}
			else
			{
				//assert (0);Commented fro testing, remove comment after UI chnage
			}
		}
	}
}

void Track::initTvCcKnobValue()
{
	for (int8_t i = 0; i < MAX_NUMBER_OF_TVKNOB; ++i)
	{
		TrackValue tmp;
		tmp.knobId = i;
		tmp.name = tvTrackDefaultNameMap[i];
		tmp.paramType = TrackValueType::TVTypeMidiCC;//NOTE : for internal sound UI holding min and max valus
		tmp.param.tvMidiCC.initValue = tvTrackDefaultCCMap[i].initValue;
		tmp.param.tvMidiCC.maxValue = tvTrackDefaultCCMap[i].maxValue;
		tmp.param.tvMidiCC.minValue = tvTrackDefaultCCMap[i].minValue;
		tmp.param.tvMidiCC.num = tvTrackDefaultCCMap[i].num;
		tmp.param.tvMidiCC.value = tvTrackDefaultCCMap[i].value;
		addTvKnobToTrack(i, tmp);
	}
}
//Sets only the value indicated by Knob position
void Track::setTvCcKnobValue (int8_t knob, TrackValue val)
{
	if (_trackData->tvTrackData.count (knob) != 0)
	{
		if (val.paramType == TrackValueType::TVTypeMidiCC)
		{
			if (_trackData->tvTrackData[knob].paramType == TrackValueType::TVTypeMidiCC)
			{
				_trackData->tvTrackData[knob].param.tvMidiCC.value = val.param.tvMidiCC.value;
			}
			else
			{
				assert (0);
			}
		}
	}
}

float Track::getTvCcKnobValue (int8_t knob)
{
	float ret = -1;
	uint8_t knobBank = knob % 8;
	bool alternate = (knob >= 16);
	if (knob > 8 && !alternate && (knobBank == (uint8_t)InternalAudioCC::VOLUME))//master volume
	{
		ret = static_cast<float>(_songManager->getMasterVolume() / 0.01f);
		_trackData->tvTrackData[knob].param.tvMidiCC.value = ret;//just update if the instrumrnt changed
	}
	else if (_trackData->tvTrackData.count (knob) != 0)
	{
		if (_trackData->tvTrackData[knob].paramType == TrackValueType::TVTypeMidiCC)
		{
			ret = _trackData->tvTrackData[knob].param.tvMidiCC.value;
		}
	}
	return ret;
}

int8_t Track::getTvPgmChangeKnobValue (int8_t knob)
{
	int8_t ret = -1;

	if (_trackData->tvTrackData.count (knob) != 0)
	{
		if (_trackData->tvTrackData[knob].paramType == TrackValueType::TVTypePgmChange)
		{
			ret = _trackData->tvTrackData[knob].param.tvPgmChange.midiPgmNum;
		}
	}
	return ret;
}

int8_t Track::getTVPgmChangeLsb (int8_t knobId)
{
	if (_trackData->tvTrackData.count (knobId) != 0)
	{
		auto type = _trackData->tvTrackData[knobId].paramType;

		if (type == TrackValueType::TVTypePgmChange)
		{
			return _trackData->tvTrackData[knobId].param.tvPgmChange.bankLSB;
		}
	}
	return -1;
}

void Track::setTVPgmChangeLsb (int8_t knobId, TrackValue val)
{
	if (_trackData->tvTrackData.count (knobId) != 0)
	{
		auto type = _trackData->tvTrackData[knobId].paramType;

		if (type == TrackValueType::TVTypePgmChange)
		{
			_trackData->tvTrackData[knobId].param.tvPgmChange.bankLSB = val.param.tvPgmChange.bankLSB;
		}
	}
}

int8_t Track::getTVPgmChangeMsb (int8_t knobId)
{
	if (_trackData->tvTrackData.count (knobId) != 0)
	{
		auto type = _trackData->tvTrackData[knobId].paramType;

		if (type == TrackValueType::TVTypePgmChange)
		{
			return _trackData->tvTrackData[knobId].param.tvPgmChange.bankMSB;
		}
	}
	return -1;
}

void Track::setTVPgmChangeMsb (int8_t knobId, TrackValue val)
{
	if (_trackData->tvTrackData.count (knobId) != 0)
	{
		auto type = _trackData->tvTrackData[knobId].paramType;

		if (type == TrackValueType::TVTypePgmChange)
		{
			_trackData->tvTrackData[knobId].param.tvPgmChange.bankMSB = val.param.tvPgmChange.bankMSB;
		}
	}
}

void Track::addTvKnobToTrack (int8_t knob, TrackValue tvData)
{
	if (_trackData->tvTrackData.count (knob) == 0)
	{
		_trackData->tvTrackData.emplace (knob, tvData);
		_trackData->tvTrackDataCount = (uint8_t)_trackData->tvTrackData.size ();
	}
}

void Track::setTvDataFromInstrument (Instrument* instrument)
{
	for (auto& tv : instrument->_instrumentData->tv)
	{
		addTvKnobToTrack (tv.first, tv.second);
	}
}

void Track::deleteTvTrackData (int8_t knob)
{
	if (_trackData->tvTrackData.count (knob) != 0)
	{
		_trackData->tvTrackData.erase (knob);
		_trackData->tvTrackDataCount = (uint8_t)_trackData->tvTrackData.size ();
	}
}

void Track::deleteAllTvTrackData (void)
{
	_trackData->tvTrackData.clear ();
	_trackData->tvTrackDataCount = 0;
}

TrackValue Track::getTv (int8_t knobId)
{
	TrackValue ret; ret.paramType = TrackValueType::TVTypeUnsassigned;

	if (_trackData->tvTrackData.count (knobId) != 0)
	{
		ret = _trackData->tvTrackData[knobId];
	}

	return ret;
}

void Track::sculpt (Step* stepToPlay,
	std::vector<TickOffset_t>& delayList,
	std::vector<TickLength_t>& lengthTicksList,
	std::vector<uint8_t>& lengthStepsList,
	std::vector<uint8_t>& noteList,
	std::vector<uint8_t>& velocityList,
	bool isNextStep,
	bool isEditTrack)
{
		std::vector<NodeData> stepNodeList = stepToPlay->getNodeList();
		for (auto& node : stepNodeList)
		{
			//Sculpt only those notes, which would start playing in this step
			if (((isNextStep == false) && (node.offset >= 0)) || ((isNextStep == true) && (node.offset < 0)))
			{

				//uint8_t lengthStepsSculptedValue = 0;
				//uint8_t lengthTicksSculptedValue = 0;
				//int8_t delaySculptedValue = 0;
				uint8_t velocitySculptedValue = 0;
				if (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptButtonOnState == true && isEditTrack)
				{
					switch (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptMode)
					{
					case SculptMode::relative:
#if defined	SCULPT_WITH_TIME
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::delay))
						{
							delaySculptedValue = node.delay + _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRelativeDelay;
							node.delay = stepToPlay->validateDelay(delaySculptedValue);
							if (isNextStep == false)
							{
								truncateNegativeDelayToZero(node.delay);
							}
							else
							{
								convertNegativeDelayToPositive(node.delay);
							}
						}
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::lengthInSteps))
						{
							lengthStepsSculptedValue = node.lenInSteps + _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRelativeLengthSteps;
							node.lenInSteps = stepToPlay->validateLengthInSteps(lengthStepsSculptedValue);
						}

						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::lengthInTicks))
						{
							lengthTicksSculptedValue = node.lenInTicks + _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRelativeLengthTicks;
							node.lenInTicks = stepToPlay->validateLengthInTicks(lengthTicksSculptedValue);
						}
#endif
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::velocity))
						{
							velocitySculptedValue = node.velocity + _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRelativeVelocity;
							node.velocity = stepToPlay->validateVelocity(velocitySculptedValue);
						}
						break;

					case SculptMode::absolute:
#if defined	SCULPT_WITH_TIME
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::delay))
						{
							delaySculptedValue = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptAbsoluteDelay;
							node.delay = stepToPlay->validateDelay(delaySculptedValue);
							if (isNextStep == false)
							{
								truncateNegativeDelayToZero(node.delay);
							}
							else
							{
								convertNegativeDelayToPositive(node.delay);
							}
						}
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::lengthInSteps))
						{
							lengthStepsSculptedValue = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptAbsoluteLengthSteps;
							node.lenInSteps = stepToPlay->validateLengthInSteps(lengthStepsSculptedValue);
						}

						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::lengthInTicks))
						{
							lengthTicksSculptedValue = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptAbsoluteLengthTicks;
							node.lenInTicks = stepToPlay->validateLengthInTicks(lengthTicksSculptedValue);
						}
#endif
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::velocity))
						{
							velocitySculptedValue = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptAbsoluteVelocity;
							node.velocity = stepToPlay->validateVelocity(velocitySculptedValue);
						}
						break;

					case SculptMode::randomAbsolute:
					{
						std::random_device rDev;
						std::default_random_engine rEng(rDev());
#if defined	SCULPT_WITH_TIME
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::delay))
						{
							auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteDelayStart;
							auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteDelayEnd;
							std::uniform_int_distribution<int> delayRange((rangeStart < rangeEnd ? rangeStart : rangeEnd), (rangeEnd > rangeStart ? rangeEnd : rangeStart));
							delaySculptedValue = (int8_t)delayRange(rEng);
							node.delay = stepToPlay->validateDelay(delaySculptedValue);
							if (isNextStep == false)
							{
								truncateNegativeDelayToZero(node.delay);
							}
							else
							{
								convertNegativeDelayToPositive(node.delay);
							}
						}
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::lengthInSteps))
						{
							auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthStepStart;
							auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthStepEnd;
							std::uniform_int_distribution<int> lengthStepRange((rangeStart < rangeEnd ? rangeStart : rangeEnd), (rangeEnd > rangeStart ? rangeEnd : rangeStart));
							lengthStepsSculptedValue = (uint8_t)lengthStepRange(rEng);
							node.lenInSteps = stepToPlay->validateLengthInSteps(lengthStepsSculptedValue);
						}

						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::lengthInTicks))
						{
							auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthTickStart;
							auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteLengthTickEnd;
							std::uniform_int_distribution<int> lengthTickRange((rangeStart < rangeEnd ? rangeStart : rangeEnd), (rangeEnd > rangeStart ? rangeEnd : rangeStart));
							lengthTicksSculptedValue = (uint8_t)lengthTickRange(rEng);
							node.lenInTicks = stepToPlay->validateLengthInTicks(lengthTicksSculptedValue);
						}
#endif
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::velocity))
						{
							auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteVelocityStart;
							auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomAbsoluteVelocityEnd;
							std::uniform_int_distribution<int> velocityRange((rangeStart < rangeEnd ? rangeStart : rangeEnd), (rangeEnd > rangeStart ? rangeEnd : rangeStart));
							velocitySculptedValue = (uint8_t)velocityRange(rEng);
							node.velocity = stepToPlay->validateVelocity(velocitySculptedValue);
						}
						break;
					}

					case SculptMode::randomRelative:
					{
						std::random_device rDev;
						std::default_random_engine rEng(rDev());
#if defined	SCULPT_WITH_TIME
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::delay))
						{
							auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeDelayStart;
							auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeDelayEnd;
							std::uniform_int_distribution<int> delayRange((rangeStart < rangeEnd ? rangeStart : rangeEnd), (rangeEnd > rangeStart ? rangeEnd : rangeStart));
							delaySculptedValue = node.delay + (int8_t)delayRange(rEng);
							node.delay = stepToPlay->validateDelay(delaySculptedValue);
							if (isNextStep == false)
							{
								truncateNegativeDelayToZero(node.delay);
							}
							else
							{
								convertNegativeDelayToPositive(node.delay);
							}
						}
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::lengthInSteps))
						{
							auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeLengthStepStart;
							auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeLengthStepEnd;
							std::uniform_int_distribution<int> lengthStepRange((rangeStart < rangeEnd ? rangeStart : rangeEnd), (rangeEnd > rangeStart ? rangeEnd : rangeStart));
							lengthStepsSculptedValue = node.lenInSteps + (uint8_t)lengthStepRange(rEng);
							node.lenInSteps = stepToPlay->validateLengthInSteps(lengthStepsSculptedValue);
						}

						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::lengthInTicks))
						{
							auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeLengthTickStart;
							auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeLengthTickEnd;
							std::uniform_int_distribution<int> lengthTickRange((rangeStart < rangeEnd ? rangeStart : rangeEnd), (rangeEnd > rangeStart ? rangeEnd : rangeStart));
							lengthTicksSculptedValue = node.lenInTicks + (uint8_t)lengthTickRange(rEng);
							node.lenInTicks = stepToPlay->validateLengthInTicks(lengthTicksSculptedValue);
						}
#endif
						if (static_cast<int8_t> (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptParameter) & static_cast<int8_t> (SculptParameterSelect::velocity))
						{
							auto rangeStart = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeVelocityStart;
							auto rangeEnd = _settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptRandomRelativeVelocityEnd;
							std::uniform_int_distribution<int> velocityRange((rangeStart < rangeEnd ? rangeStart : rangeEnd), (rangeEnd > rangeStart ? rangeEnd : rangeStart));
							velocitySculptedValue = node.velocity + (uint8_t)velocityRange(rEng);
							node.velocity = stepToPlay->validateVelocity(velocitySculptedValue);
						}
						break;
					}
					}//switch sculpt modes
				}

				 //return sculpted values
				//all current step nodes
				//only nodes with negative delay, from next step
				if ((isNextStep == false) || ((isNextStep == true) && (node.offset < 0)))
				{
					if (isNextStep == false)
					{
						delayList.push_back(node.offset);
					}
					else //moving negative delay step from next step to current step, update delay value to make it part of current step
					{
						TickLength_t max_ticks_per_step = TB4_TICKS_PER_STEP;//default
						auto pattern = stepToPlay->getPattern();
						if (pattern != nullptr)
						{
							max_ticks_per_step = pattern->getTicksPerStep();
						}
						delayList.push_back(max_ticks_per_step + node.offset + 1);
					}
					lengthTicksList.push_back(node.lenInTicks);
					lengthStepsList.push_back(node.lenInSteps);
					noteList.push_back(node.note);
					velocityList.push_back(node.velocity);
				}
			}//if node starts falls in this step
		}//for loop iterate nodes
		//********* TODO stepToPlay->setNodeList need to be redesign
		if (_settingsSculptModeCnfg->_SettingsSculptModeCnfgData.sculptButtonOnState == true && isEditTrack)// update the pattern
			stepToPlay->setNodeList(stepNodeList);
}

void Track::truncateNegativeDelayToZero (TickOffset_t& delay)
{
	// if start point falls in the previous step. we cannot play this note
	// and if we let negative delay accumulate due to sculpting
	// all notes will have starting point before 0, nothing will play
	if (delay < 0)
	{
		delay = 0;
	}
}

void Track::convertNegativeDelayToPositive (TickOffset_t& delay)
{
	//Notes from next step (with negative delay)
	//Represent them as notes of current step with positive delay
	if (delay < 0)
	{
		TickLength_t maxTicksPerStep = TB4_TICKS_PER_STEP;
		auto pattern = getTrackPattern();
		if (pattern != nullptr)
		{
			maxTicksPerStep = pattern->getTicksPerStep();
		}
		delay = maxTicksPerStep + delay;
	}
}

void Track::handleMidiOutClockGeneration (uint16_t tick)
{
	//get configured rate
	auto clockFrequency = _songManager->engine.settingsMidiClkCnfg->_settingsMidiClkCnfgData.MidiClkOutPutClkRate;

	//decide which tick needs to be send
	auto tickList = getTicksAtWhichClockIsSend(clockFrequency);

	//send if tick matches
	for (auto triggerTick : tickList)
	{
		if (triggerTick == tick)
		{
			//TODO send midi clock
		}
	}
}

std::vector <uint16_t> Track::getTicksAtWhichClockIsSend (int8_t rate)
{
	std::vector <uint16_t> tickList;
	tickList.clear();

	uint16_t triggerTickInterval = 0;

	switch(rate)
	{
		case static_cast<int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRate_2PPQ):
			triggerTickInterval = SET_PPQN / 2;
			break;

		case static_cast<int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRate_24PPQ):
			triggerTickInterval = SET_PPQN / 24;
			break;

		case static_cast<int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRate_48PPQ):
			triggerTickInterval = SET_PPQN / 48;
			break;

		case static_cast<int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRate_1PP2Q):
			triggerTickInterval = SET_PPQN * 2;
			break;

		case static_cast<int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRate_1PP4Q):
			triggerTickInterval = SET_PPQN * 4;
			break;

		case static_cast<int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRate_1PP8):
			triggerTickInterval = SET_PPQN / 2;
			break;

		case static_cast<int8_t> (SettingsMidiClkCnfgOutputClkRate::outClkRate_1PP16):
			triggerTickInterval = SET_PPQN / 4;
			break;
	}

	int32_t stepTicks = _pulsesPerStep; //from timebase selected
	do
	{
		uint16_t count = 0;//first trigger at begining of step and subsequent ticks at equal intervals, till end of step
		stepTicks = stepTicks - triggerTickInterval;
		tickList.push_back (count * triggerTickInterval);
		++count;
	} while (stepTicks > 0);

	return tickList;
}

bool Track::cvAdsrOutEnabledForThisTrack (void)
{
	if ((_txMidiPort >= static_cast<uint8_t>(InstrumentPorts::cvPort1)) && (_txMidiPort <= static_cast<uint8_t>(InstrumentPorts::cvPort8)))
	{
		//normalise CV port number in the range 0 - MAX_CVIO_PORTS
		auto cvPortForThisTrack = _txMidiPort - static_cast<uint8_t>(InstrumentPorts::cvPort1);//CV ports are counted from 0 - 7 in CV config module

		//Calculate envelope, only if envelope mode is selected under LAW.
		if (_songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[(int8_t)cvPortForThisTrack].law == CvLaw::envelope)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool Track::cvEnabledForThisTrack(void)
{
	bool ret = false;

	return ret;
}


void Track::generateAdsrVoltages (uint8_t velocity, uint16_t lengthTicks)
{
	//normalise CV port number in the range 0 - MAX_CVIO_PORTS
	auto cvPortForThisTrack = _txMidiPort - static_cast<uint8_t>(InstrumentPorts::cvPort1);//CV ports are counted from 0 - 7 in CV config module

	//Calculate envelope if envelope mode is selected under LAW.
	if (_songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[(int8_t)cvPortForThisTrack].law == CvLaw::envelope)
	{
		uint8_t attackTimePercent = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[(int8_t)cvPortForThisTrack].EnvAttack;
		uint8_t decayTimePercent = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[(int8_t)cvPortForThisTrack].EnvDecay;
		uint8_t sustainVoltagePercent = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[(int8_t)cvPortForThisTrack].EnvSustain;
		uint8_t releaseTimePercent = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[(int8_t)cvPortForThisTrack].EnvRelease;
		uint8_t amountPercent = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[(int8_t)cvPortForThisTrack].EnvAmount;
		uint8_t velocityPercent = _songManager->engine.settingsCvioCnfg->_SettingsCvioCnfgData.cvConfig[(int8_t)cvPortForThisTrack].EnvVelocity;

		auto tempo = _songManager->activeSong->getPlayScene ()->getTempo ();

		Adsr stepEnvelope =  Adsr(attackTimePercent, decayTimePercent, sustainVoltagePercent, releaseTimePercent, amountPercent, velocityPercent,
			CV_PEAK_VOLTAGE, _pulsesPerStep, ADSR_VOLTAGE_SAMPLE_RATE, SET_PPQN, (uint8_t)tempo, velocity, lengthTicks);

		adsrEnvelopeValues = stepEnvelope.createEnvelope();
	}
}

void Track::handleAdsrOut (uint16_t tick)
{
	//check if adsr is enabled
	if ( cvAdsrOutEnabledForThisTrack () == true)
	{
		double sampleIntervalMSec = static_cast <double> (1000) / ADSR_VOLTAGE_SAMPLE_RATE;

		auto tempo = _songManager->activeSong->getPlayScene ()->getTempo ();

		double tickDurationMSec = static_cast <double> (60 * 1000) / (tempo * _pulsesPerStep);

		uint16_t numTicksPerSampleInterval = (uint16_t)(sampleIntervalMSec / tickDurationMSec);

		// sampleIntervalMsec should be greater than tick duration,
		// other wise we will have zero ticks per sample interval
		// TODO : choose a lower sample rate, which would have
		// enought voltage samples per second, but the interval should be greater than tick duration

		if ((numTicksPerSampleInterval != 0) && ((tick % numTicksPerSampleInterval) == 0))
		{
			//TODO SEND ADSR values to CV port
			//values in adsrEnvelopeValues
		}
	}
}

uint16_t Track::uiTickToEnginePulse(TickLength_t uiTick)
{
	TickLength_t maxTicksPerStep = TB4_TICKS_PER_STEP;
	auto pattern = getTrackPattern();
	if (pattern != nullptr)
	{
		maxTicksPerStep = pattern->getTicksPerStep();
	}
	return (_pulsesPerStep / maxTicksPerStep) * uiTick;
}

void Track::handleBrayControlForNodeParams (uint8_t& velocity)
{
	if (_songManager->engine.settingsBrayCnfg->_settingsBRayCnfgData.brayPowerStatus)
	{
		switch (_songManager->engine.settingsBrayCnfg->_settingsBRayCnfgData.brayControlledParam)
		{
			case BRayControllSetting::velocityControl:
			{
				auto adcValue = _songManager->engine.settingsBrayCnfg->brayAdcValue;
				normalise <uint16_t> (adcValue, ADC_12BIT_RANGE, MIDI_VELOCITY_VALUE_RANGE);
				velocity = static_cast <uint8_t> (adcValue);
				break;
			}

			//add any new controls here

			default:
				//nothing to do
				break;
		}
	}
}

void Track::handleBrayControlForStepAuxParams (AuxEventType type, int8_t& data1, int8_t& data2)
{
	if (_songManager->engine.settingsBrayCnfg->_settingsBRayCnfgData.brayPowerStatus)
	{
		switch (_songManager->engine.settingsBrayCnfg->_settingsBRayCnfgData.brayControlledParam)//current param controlled by BRAY
		{
			case BRayControllSetting::afterTouchControl://polyphonic key pressure
				if (type == AuxEventType::afterTouchPolyPressure)
				{
					auto adcValue = _songManager->engine.settingsBrayCnfg->brayAdcValue;
					normalise <uint16_t> (adcValue, ADC_12BIT_RANGE, MIDI_PRESSURE_VALUE_RANGE);
					data2 = static_cast <int8_t> (adcValue);
				}
				break;

			case BRayControllSetting::ccControl:
				if (type == AuxEventType::controlChange)
				{
					auto controlledCcNum = _songManager->engine.settingsBrayCnfg->_settingsBRayCnfgData.brayCcNum;
					if (controlledCcNum == data1) //check if CC num matches
					{
						auto adcValue = _songManager->engine.settingsBrayCnfg->brayAdcValue;
						normalise <uint16_t> (adcValue, ADC_12BIT_RANGE, MIDI_CC_VALUE_RANGE);
						data2 = static_cast <int8_t> (adcValue);
					}
				}
				break;

			case BRayControllSetting::pitchBendControl:
				if (type == AuxEventType::pitchBend)
				{
					auto adcValue = _songManager->engine.settingsBrayCnfg->brayAdcValue;
					normalise <uint16_t> (adcValue, ADC_12BIT_RANGE, MIDI_PITCH_BEND_RANGE);
					data1 = static_cast <int8_t> (((adcValue >> 8) & 0x7F));//MSB
					data2 = static_cast <int8_t> (adcValue & 0x7F);//LSB
				}
				break;
		}
	}
}

uint16_t Track::enginePulseToUiTick (uint16_t enginePulse)
{
	TickLength_t maxTicksPerStep = TB4_TICKS_PER_STEP;
	auto pattern = getTrackPattern();
	if (pattern != nullptr)
	{
		maxTicksPerStep = pattern->getTicksPerStep();
	}
	return (maxTicksPerStep * enginePulse) / _pulsesPerStep;
}

// Setting it to edit scene does not make much sense, as tranpose changes would be audible when playing only.
// So setting this to current play scene, rather current edit scene.
// Changes would be updated every step
void Track::handleTransposeControlFromExternalMidi (void)
{
	if (_songManager->engine.settingsTransposeControlCnfg->_transposeControlCnfgData.transposeControlStatus == true)
	{
		if (_songManager->engine.settingsTransposeControlCnfg->_transposeControlCnfgData.tranposeUpdatedFlag.load() == true)
		{
			//reset
			_songManager->engine.settingsTransposeControlCnfg->_transposeControlCnfgData.tranposeUpdatedFlag.store (false);

			auto transposeDelta = _songManager->engine.settingsTransposeControlCnfg->_transposeControlCnfgData.transposeDelta;

			if (transposeDelta != 0)
			{
				if ((_songManager->activeSong != nullptr) && (_songManager->activeSong->playScene != nullptr))
				{
					//TODO : Feature can be modifed in the future, to set the absolute value, rather than adding delta
					// Then the user will be provided with the option to choose relative or absolute, from UI.
					// And here we will check the user decision and set accordingly.
					auto currentTransposeValue = _songManager->activeSong->playScene->getTransposeValue ();
					auto newTransposeValue = currentTransposeValue + transposeDelta;
					_songManager->activeSong->playScene->setTransposeValue ((int8_t)newTransposeValue); //bound check will be done in this function
				}
			}
		}
	}
}
void Track::SaveDfaultUsedbyInfo (bool update)
{
	if (!DefaultDataSaved || update)
	{
		SavedInstrument = _trackData->trackRxInstrument;
		SavedOutInstrument = _trackData->trackTxInstrument;
		SavedFillpattern = _trackData->fillPattern;
		SavedMainpattern = _trackData->mainPattern;
		DefaultDataSaved = true;
		InstrumentUpdated = false;
		InstrumentOutUpdated = false;
		MainpatternUpdated = false;
		FillpatternUpdated = false;
	}
}
/* To reset used by list
*  Function to handle - user assign pattern/instrument to a track save/reload/assign different data
*  do undo, switch to different song/scene, rename the song/scene, delete etc...
*  Used by info needs to be handled in all use case
*/
void Track::ResetUsedbyInfo (std::string SongName, std::string SceneName, bool ForceRemove)
{
	if (InstrumentUpdated || ForceRemove) // ForceRemove - check and remove the entry, not required to reset old data if forced
	{
		if(_trackData->trackRxInstrument)
			_trackData->trackRxInstrument->RemoveFromUsedByList (false, SongName, SceneName, _trackData->trackName);
		if(SavedInstrument && !ForceRemove)
			SavedInstrument->UpdateUsedByList (false, SongName, SceneName, _trackData->trackName);
	}
	if (InstrumentOutUpdated || ForceRemove) // ForceRemove - check and remove the entry, not required to reset old data if forced
	{
		if (_trackData->trackTxInstrument)
			_trackData->trackTxInstrument->RemoveFromUsedByList(false, SongName, SceneName, _trackData->trackName);
		if (SavedOutInstrument && !ForceRemove)
			SavedOutInstrument->UpdateUsedByList(false, SongName, SceneName, _trackData->trackName);
	}
	if (FillpatternUpdated || ForceRemove)
	{
		if (_trackData->fillPattern)
		{
			_trackData->fillPattern->RemoveFromUsedByList(true, SongName, SceneName, _trackData->trackName);
			if (ForceRemove)
				_trackData->fillPattern->savePatternUsedByList();
		}
		if(SavedFillpattern && !ForceRemove)
			SavedFillpattern->UpdateUsedByList (true, SongName, SceneName, _trackData->trackName);// restore defaults
	}
	if (MainpatternUpdated || ForceRemove)
	{
		if (_trackData->mainPattern)
		{
			_trackData->mainPattern->RemoveFromUsedByList(false, SongName, SceneName, _trackData->trackName);
			if (ForceRemove)
				_trackData->mainPattern->savePatternUsedByList();
		}
		if(SavedMainpattern && !ForceRemove)
			SavedMainpattern->UpdateUsedByList (false, SongName, SceneName, _trackData->trackName);// restore defaults
	}
	SaveDfaultUsedbyInfo (true);
}

/** Check if the midi packet received is valid or not
 * Validates channel, midi msg type
 */
bool Track::isValidNoteEvent (midiData data)
{
	auto trackChannel = getRxInstrumentChannel ();//In Instrument
	if (trackChannel != -1)
	{
		auto midiDataChannel = data.statusByte & 0xF;

		// Process midi data meant for this track only
		if (trackChannel == midiDataChannel)
		{
			if ( ( ( (data.statusByte & 0xF0) == 0x90) && (data.dataByte2 == 0)) || ( (data.statusByte & 0xF0) == 0x80))//Note OFF
			{
				//for off, check if on is present
				if (_noteOnList.count (data.dataByte1) > 0)
				{
					_noteOnList.erase (data.dataByte1);
					return true;
				}
				else
				{
					return false;
				}
			}
			else if ( ( (data.statusByte & 0xF0) == 0x90) && (data.dataByte2 != 0))//Note ON
			{
				//for on,  check if on is already present
				if (_noteOnList.count (data.dataByte1) > 0)
				{
					DBG("IN : Note On rej, already present : " + String(data.dataByte1) + " : " + String(data.dataByte2));
					return false;
				}
				else
				{
					auto ret = _noteOnList.insert (data.dataByte1);
					if(ret.second != true)
					{
						DBG("IN : Valid NON, capture failed");
					}
					//DBG ("Valid ON packet : Track id : " + String (_trackData->trackId) + " CH : " + String (trackChannel) + " Packet Ch : " + String (midiDataChannel));

					return true;
				}
			}
			else if ( ( (data.statusByte & 0xF0) == (uint8_t)AuxEventType::afterTouchPolyPressure) ||
					  ( (data.statusByte & 0xF0) == (uint8_t)AuxEventType::controlChange) ||
				      ( (data.statusByte & 0xF0) == (uint8_t)AuxEventType::programChange) ||
				      ( (data.statusByte & 0xF0) == (uint8_t)AuxEventType::afterTouchChannelPressure) ||
				      ( (data.statusByte & 0xF0) == (uint8_t)AuxEventType::pitchBend))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		// midi packet channel does not match with track's midi channel
		else
		{
			//DBG ("IN : Invalid packet : Track id : " + String(_trackData->trackId) + " CH : " + String (trackChannel) + " Packet Ch : " + String (midiDataChannel));
			//Ignore this packet
			return false;
		}
	}
	//did not get track's midi channel info
	else
	{
		return false;
	}
}

void Track::AuditioningTrackInit(void)
{
	resetEffects();
	effectsInit();
	_noteOnList.clear();
	if (_trackSf == nullptr)
		_trackSf = tsf_copy(static_cast<tsf*>(_songManager->get_tsf_instance()));
	/* TSF init */
	//_trackSf = tsf_load_filename(SOUND_FONT_FILE);
	tsf* sf = static_cast<tsf*>(_trackSf);
	if (sf == nullptr)
		return;
	tsf_set_output(sf, TSF_MONO, INTERNAL_AUDIO_SAMPLE_RATE, 0); //sample rate
	tsf_set_max_voices(sf, 8);// 8 note polyphony
	tsf_channel_set_volume(sf, 0, 0.9f);//set to 0.9 and apply Master volume later
	tsf_channel_set_presetindex(static_cast<tsf*>(_trackSf), 0, 0);
}
void Track::AuditioningTrackSetPreset(int presetindex)
{
	if (_trackSf == nullptr)
		return;
	tsf_channel_set_presetindex(static_cast<tsf*>(_trackSf), 0, presetindex);
}
void Track::AuditioningTrackClear(void)
{
	_noteOnList.clear();
	if (_trackSf == nullptr)
		return;
	tsf_channel_note_off_all(static_cast<tsf*>(_trackSf), 0);
}

void Track::sfInit(void)
{
	tsf* sf = static_cast<tsf*>(_trackSf);
	if (sf == nullptr)
		return;
	tsf_set_output(sf, TSF_MONO, INTERNAL_AUDIO_SAMPLE_RATE, 0); //sample rate
	tsf_set_max_voices(sf, 8);// 8 note polyphony
	tsf_channel_set_volume(sf, 0, 0.9f);//set to 0.9 and apply Master volume later
	sfSetProgram();
}

void Track::sfReset(void)
{
	if (_trackSf != nullptr)
	{
		//release tsf resources
		tsf_reset(static_cast<tsf*>(_trackSf));
	}
}

void Track::sfLoad(void)
{
	//tsf set up
	if(_trackSf == nullptr)
		_trackSf = tsf_copy(static_cast<tsf*>(_songManager->get_tsf_instance()));
	//_trackSf = tsf_load_filename(SOUND_FONT_FILE);

	sfInit();
	//soundFontProgramCount = tsf_get_presetcount(trackSf);
}

void Track::sfNoteOn(uint8_t channel, uint8_t key, float velocity)
{
	if (_trackSf == nullptr || !desktopPresetValid)
		return;
	tsf_channel_note_on(static_cast<tsf*>(_trackSf), channel, key, velocity);
}

void Track::sfNoteOff(uint8_t channel, uint8_t key)
{
	if (_trackSf == nullptr)
		return;
	tsf_channel_note_off(static_cast<tsf*>(_trackSf), channel, key);
}

void Track::sfSynthesizeAudio(void)
{
    float* desktopBuffer = _twentyMsBuffer;
    audioBuff = juce::AudioBuffer<float>(&desktopBuffer, 1, INTERNAL_AUDIO_20MS_BLOCK_SIZE);
	if (_trackSf == nullptr)
	{
		std::fill_n(_twentyMsBuffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE, 0.0f);
		return;
	}
#ifdef USE_INTERNAL_EFFECTS
	float* TwentyMsBufferPtr = _twentyMsBuffer;
	audioBuff = juce::AudioBuffer<float>(&TwentyMsBufferPtr, 1, INTERNAL_AUDIO_20MS_BLOCK_SIZE);
	juce::dsp::AudioBlock<float> outputBlock(audioBuff);
	dsp::ProcessContextReplacing<float> buffer(outputBlock);
#endif
	if (_trackSf != nullptr)
	{

#ifdef AUDIO_RENDER_TYPE_SHORT

		short tmpBuffer[INTERNAL_AUDIO_20MS_BLOCK_SIZE] = { 0 };
		tsf_render_short(static_cast<tsf*>(_trackSf), tmpBuffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE, 0);

		/* WORK AROUND FOR NOISE ISSUE : 
		* Using short rendering shows no crackling noise after render.
		* So use render_short and then convert back to float(AudioDataType) as the remaning chain expects float data -> effects and juce */
		for (int i = 0; i < INTERNAL_AUDIO_20MS_BLOCK_SIZE; ++i)
		{
			_twentyMsBuffer[i] = (float)tmpBuffer[i] / 32768;
		}

#else

		tsf_render_float(static_cast<tsf*>(_trackSf), _twentyMsBuffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE, 0);

#endif /* !AUDIO_RENDER_TYPE_SHORT */

#ifdef LOG_PCM_TO_FILE
		// Log raw synthesized data here
#endif

	}
#ifdef USE_INTERNAL_EFFECTS
	for (auto index = 0; index < AUDIO_EFFECT_MAX; index++)
	{
		switch (audioEffectsOrder[index])// execute the effects in the order user requested
		{
		case AUDIO_REVERB:
		{
			if (theReverbParameters.width != 0)
			{
				igniteReverb.processMono(_twentyMsBuffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE);
			}
		}
		break;

		case AUDIO_CHORUS:
			if (chorusStatus[1] == true)
				_Chorus[0].process(buffer);
			break;
		case AUDIO_EQ_L:
		{
			if (EQisEnabled[FILTER_HIGHEQ_INDEX])
				eqFilter[FILTER_HIGHEQ_INDEX].process(buffer);
		}
		break;
		case AUDIO_EQ_H:
		{
			if (EQisEnabled[FILTER_LOWEQ_INDEX])
				eqFilter[FILTER_LOWEQ_INDEX].process(buffer);
		}
		break;
		case AUDIO_FILTER:
		{
			filter.process(buffer);
		}
		break;
		default:
			break;
		}
	}
#ifdef LOG_PCM_TO_FILE
//Log data after effets are applied
#endif

	//Volume affected by the filter gain and other params .. Volume applied after all effects
	trackGain.process(buffer);
#endif /* USE_INTERNAL_EFFECTS */

}

void Track::sfSetProgram(void)
{
	if (_trackSf == nullptr)
		return;
	if (_trackData->trackTxInstrument)
	{
		int preset = 0;
		int bank = 0;
		int index = _trackData->trackTxInstrument->getInternalSoundPgm();

		//0 - 127 is for regular presets
		//128 - 137 is fro drum presets
        if (SoundFontProgram::isCustom(index))
        {
            bank = SoundFontProgram::bank(index);
            preset = SoundFontProgram::program(index);
        }
        else if (index > 127)
		{
			bank = 128; //drum bank
			preset = getTsfPresetFromIgniteIndex(index);
		}
		else
		{
			preset = index;
		}
		desktopPresetValid = tsf_channel_set_bank_preset(static_cast<tsf*>(_trackSf), 0, bank, preset) != 0;
	}
}

void Track::sfChangeProgram(void)
{
	if (_trackSf == nullptr)
		return;
	tsf_channel_note_off_all(static_cast<tsf*>(_trackSf), 0);
	sfSetProgram();
}


void Track::setInternalAudioEffects(uint8_t knobID, float value, bool isAlternate)
{
#ifdef USE_INTERNAL_EFFECTS
	uint8_t knobBank = knobID % 8;
	switch (knobBank)
	{
	case (uint8_t)InternalAudioCC::RELEASE:
		/*if(isAlternate)
			tsf_set_noteAttackTimeMs(value);
		else*/
		tsf_set_noteReleaseTimeMs(static_cast<int>(value));
		break;

	case (uint8_t)InternalAudioCC::FILTER_CUTOFF:
		if (isAlternate)
		{
			if (value == 12)
				filter.setMode(dsp::LadderFilter<float>::Mode::LPF12);
			else
				filter.setMode(dsp::LadderFilter<float>::Mode::LPF24);
		}
		else if (value != 0)
		{
			if (value <= 22)
			{
				filter.setEnabled(false);
			}
			else
			{
				filter.setEnabled(true);
				filter.setCutoffFrequencyHz(value);
			}
		}
		break;

	case (uint8_t)InternalAudioCC::RESONANCE:
		filter.setResonance(value * 0.01f);
		break;

	case (uint8_t)InternalAudioCC::VOLUME:
		if (isAlternate)
			trackGain.setGainLinear(value * 0.01f);
		else
			_songManager->setMasterVolume(value * 0.01f);
		break;

	case (uint8_t)InternalAudioCC::REVERB:
		if (isAlternate)
			theReverbParameters.dryLevel = (value * 0.01f);
		else
			theReverbParameters.width = (value * 0.01f);
		break;

	case (uint8_t)InternalAudioCC::CHORUS:
		if (value == 0)
		{
			chorusStatus[1] = false;
		}
		else
		{
			chorusStatus[1] = true;
			if (isAlternate)
				_Chorus[0].setDepth(value * 0.01f);
			else
				if (value >= 100)
					_Chorus[0].setRate(99);
				else
					_Chorus[0].setRate(value);
		}
		break;

		//case (uint8_t)InternalAudioCC::EQ_LOW:
	case (uint8_t)InternalAudioCC::EQ_HIGH:
		if (isAlternate)
		{
			if (value)
				EQisEnabled[FILTER_HIGHEQ_INDEX] = true;
			else
				EQisEnabled[FILTER_HIGHEQ_INDEX] = false;

			EQcutoffGain[FILTER_HIGHEQ_INDEX] = (value + 0.00001f);
			eqFilter[FILTER_HIGHEQ_INDEX].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(INTERNAL_AUDIO_SAMPLE_RATE, EQcutoffFreq[FILTER_HIGHEQ_INDEX], 1, EQcutoffGain[FILTER_HIGHEQ_INDEX]);
		}
		else
		{
			if (value)
				EQisEnabled[FILTER_HIGHEQ_INDEX] = true;
			else
				EQisEnabled[FILTER_HIGHEQ_INDEX] = false;

			if (value)
			{
				EQcutoffFreq[FILTER_HIGHEQ_INDEX] = (value);
				if (EQcutoffGain[FILTER_HIGHEQ_INDEX])
					eqFilter[FILTER_HIGHEQ_INDEX].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(INTERNAL_AUDIO_SAMPLE_RATE, EQcutoffFreq[FILTER_HIGHEQ_INDEX], 1, EQcutoffGain[FILTER_HIGHEQ_INDEX]);
			}
		}
		EQisEnabled[FILTER_HIGHEQ_INDEX] = (EQcutoffGain[FILTER_HIGHEQ_INDEX] > 0.0001f);//added multiple check here for EQisEnabled to avoid complex checks for each ticks when playing
		break;

		//case (uint8_t)InternalAudioCC::EQ_HIGH:
	case (uint8_t)InternalAudioCC::EQ_LOW:
		if (isAlternate)
		{
			if (value)
				EQisEnabled[FILTER_LOWEQ_INDEX] = true;
			else
				EQisEnabled[FILTER_LOWEQ_INDEX] = false;

			EQcutoffGain[FILTER_LOWEQ_INDEX] = (value + 0.00001f);
			eqFilter[FILTER_LOWEQ_INDEX].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(INTERNAL_AUDIO_SAMPLE_RATE, EQcutoffFreq[FILTER_LOWEQ_INDEX], 1, EQcutoffGain[FILTER_LOWEQ_INDEX]);
		}
		else
		{
			if (value)
				EQisEnabled[FILTER_LOWEQ_INDEX] = true;
			else
				EQisEnabled[FILTER_LOWEQ_INDEX] = false;

			if (value)
			{
				EQcutoffFreq[FILTER_LOWEQ_INDEX] = value;
				if (EQcutoffGain[FILTER_LOWEQ_INDEX])
					eqFilter[FILTER_LOWEQ_INDEX].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(INTERNAL_AUDIO_SAMPLE_RATE, EQcutoffFreq[FILTER_LOWEQ_INDEX], 1, EQcutoffGain[FILTER_LOWEQ_INDEX]);
			}
		}
		EQisEnabled[FILTER_LOWEQ_INDEX] = (EQcutoffGain[FILTER_LOWEQ_INDEX] > 0.0001f);//added multiple check here for EQisEnabled to avoid complex checks for each ticks when playing
		break;

	default:
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN knobID for Internal Not valid: " + String(knobID));
		break;

	}
#endif //USE_INTERNAL_EFFECTS
}

void Track::resetEffects(void)
{
#ifdef TEST_ENABLE_SAMPLE
	readaudiofile = 1;
	IndexReadaudiofile = 0;
#endif
	igniteReverb.reset();//clear old data when stoped/paused
	eqFilter[FILTER_LOWEQ_INDEX].reset();
	eqFilter[FILTER_HIGHEQ_INDEX].reset();
	_Chorus[0].reset();
	igniteReverb.reset();
}

void Track::effectsInit(void)
{
	reverbInit();
	chorusInit();
	eqInit();
	filterInit();
	gainInit();
}

void Track::reverbInit(void)
{
	igniteReverb.setSampleRate(INTERNAL_AUDIO_SAMPLE_RATE);//TODO : settings can be done from UI for each track?
	theReverbParameters.dryLevel = 0.4f;
	theReverbParameters.wetLevel = 0.33f;
	theReverbParameters.roomSize = 1.0f;
	theReverbParameters.damping = 0.5f;
	theReverbParameters.width = 0.0f;
	igniteReverb.setParameters(theReverbParameters);
}

void Track::chorusInit(void)
{
	_Chorus[0].prepare({ INTERNAL_AUDIO_SAMPLE_RATE, INTERNAL_AUDIO_20MS_BLOCK_SIZE, 1 });
	_Chorus[0].setRate(0.2f); // Set chorus rate
	_Chorus[0].setDepth(0.0f); // Set chorus depth
	_Chorus[0].setMix(0.5f); // Set chorus Dry/wet Mix)
}

void Track::eqInit(void)
{
	eqFilter[FILTER_HIGHEQ_INDEX].prepare({ INTERNAL_AUDIO_SAMPLE_RATE, INTERNAL_AUDIO_20MS_BLOCK_SIZE, 1 });
	//0.001 as 0 is not allowed, this code only for init
	eqFilter[FILTER_HIGHEQ_INDEX].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(INTERNAL_AUDIO_SAMPLE_RATE, EQcutoffFreq[FILTER_HIGHEQ_INDEX], 1, EQcutoffGain[FILTER_HIGHEQ_INDEX]);
	eqFilter[FILTER_LOWEQ_INDEX].prepare({ INTERNAL_AUDIO_SAMPLE_RATE, INTERNAL_AUDIO_20MS_BLOCK_SIZE, 1 });
	eqFilter[FILTER_LOWEQ_INDEX].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(INTERNAL_AUDIO_SAMPLE_RATE, EQcutoffFreq[FILTER_LOWEQ_INDEX], 1, EQcutoffGain[FILTER_LOWEQ_INDEX]);
}

void Track::filterInit(void)
{
	filter.prepare({ INTERNAL_AUDIO_SAMPLE_RATE, INTERNAL_AUDIO_20MS_BLOCK_SIZE, 1 });
	filter.setMode(dsp::LadderFilter<float>::Mode::LPF12);
	filter.setCutoffFrequencyHz(filtercutoffFreq);
	filter.setResonance(filterResonance);
	filter.setEnabled(false);
	filter.setDrive(1);
}

void Track::gainInit(void)
{
	trackGain.prepare({ INTERNAL_AUDIO_SAMPLE_RATE, INTERNAL_AUDIO_20MS_BLOCK_SIZE, 1 });
	trackGain.setGainLinear(0.7f);
}


void Track::initStepRecord(void)
{
	if (_songManager->_state == SYSTEM_STATE::STEP_RECORD)
	{
		//Start from the first step
		_trackBarId = 0;
		_trackPlayStepId = 0;
		stepRecordNoteOnCount = 0;
		_trackStepIndex = 0;

		setTrackRxMidiPortAndChannel();
	}
}

void Track::stepRecordNote(const MidiMessage& message)
{
    if (message.getChannel() != _rxMidiChannel + 1 || getTrackPattern() == nullptr) return;
	uint8_t status; uint8_t data1; uint8_t data2;

	/* We need note-on message only, to identify the the mote and velocity
	* Since it is assigned to current step only, we are not using the note-off to calculate length.
	* Length is currently fixed at half step length
	*/
	auto pattern = getTrackPattern();
	auto patternName = pattern->getName();
	if (message.isNoteOn())
	{
		stepRecordNoteOnCount++;
		auto data = message.getRawData();

		status = data[0]; data1 = data[1]; data2 = data[2];

		if (_songManager->_state == SYSTEM_STATE::STEP_RECORD)
		{
			//assign note to current step
			//len - set to half step
			//chords not suported - if required in future use timestamps or timers, to group notes
			auto msgChannel = status & 0x0F;

			//check if it belongs to the track
			if (msgChannel == _rxMidiChannel)
			{
				auto bar = pattern->_patternData->barList.find(_trackBarId);
				if (bar != pattern->_patternData->barList.end())
				{
					auto step = bar->second._BarData->stepList.find(_trackPlayStepId);
					if (step != bar->second._BarData->stepList.end())
					{
						//defaulting to half step length and zero offset
						//update based on user feedback
						ClientPatternNodeData tmp;
						tmp.nodeId = static_cast<uint16_t>(data1);
						tmp.note = data1;
						tmp.velocity = data2;
						tmp.lenInSteps = 0;
                        tmp.lenInTicks = std::max<int>(1, pattern->getTicksPerStep() / 2);
                        tmp.step_id = _trackPlayStepId;
                        tmp.onState = true;
						tmp.offset = 0;

						//if pattern already has same note in the current step, remove it
						if (step->second._stepData->nodeList.count(static_cast<uint8_t>(tmp.nodeId)) != 0)
						{
							return;
						}
						//step->second._stepData->nodeList[tmp.note].emplace(tmp.delay, tmp);
						step->second._stepData->nodeList[tmp.note].emplace(
							std::piecewise_construct,
							std::forward_as_tuple(tmp.offset),
							std::forward_as_tuple(tmp)
						);
						step->second._stepData->nodeCount = static_cast<uint8_t>(step->second._stepData->nodeList.size());
					}
				}

				//inform ui
				publishTrackBarAndStepInfoToUI();

				//update sequence after every step
				PatternBarStepInfo stepAncestory{ patternName, _trackBarId, _trackPlayStepId };
				_songManager->engine.sequenceManager->handleRecordChange(stepAncestory);

			}//msg channel check
		}//step record mode check
	}//note on check
	else if (message.isNoteOff() || stepRecordNoteOnCount > 127)//safty check for error case
	{
		if (stepRecordNoteOnCount == 0)//if note off recived first, user may start recording with key already pressed
			return;

		stepRecordNoteOnCount--;
		if (stepRecordNoteOnCount == 0 || stepRecordNoteOnCount > 127)
		{
			//move to next step
			int8_t barId;
			int16_t stepId;
			int16_t barStepCount;
			handleStepChangeWithout_Gbar_BarRepeat_BarLoop(patternName);
			std::tie(barId, stepId, barStepCount) = _songManager->engine.sequenceManager->getLinearNextStepOfBar(_trackBarId, _trackStepIndex, patternName);
			_trackBarId = barId;
			_trackPlayStepId = stepId;
			_stepCountForCurrentBar = barStepCount;
			stepRecordNoteOnCount = 0;
		}
	}
	else//midi cc messages
	{
		auto ccData = message.getRawData();
		Aux tmp = { true, static_cast<uint16_t>(ccData[0]), static_cast<int8_t>(ccData[1]), static_cast<int8_t>(ccData[2]), 0 };

		auto bar = pattern->_patternData->barList.find(_trackBarId);
		if (bar != pattern->_patternData->barList.end())
		{
			auto step = bar->second._BarData->stepList.find(_trackPlayStepId);
			if (step != bar->second._BarData->stepList.end())
			{
				step->second._stepData->auxList.push_back(tmp);
			}
		}
	}
}
void Track::setStepReSelectedStep(uint8_t stepId)
{
	_trackBarId = (stepId >>4) & 0x0F;
	_trackPlayStepId = stepId & 0x0F;
}

/* Decides the event(note/aux) offset within a step, during record
 * Considers user setting -> Quantisation 0n/off
 * Quantised recording -> all events start at, start of the step
 * Un-Qunatised recording -> events will be placed at one of the intervals(ticks) within the step, based on the time of arrival.
 */
TickOffset_t Track::calculateOffsetWithinStep(TickOffset_t tick)
{
	TickOffset_t offset = 0;
	auto quantiseRecord = _songManager->_recordSetting.find((paramId_t)SettingsRecordCnfgParam::SettingRecordQuantizationParam);
	if ( (quantiseRecord != _songManager->_recordSetting.end()) &&  (quantiseRecord->second == 1))
	{
		offset = 0;
	}
	else
	{
		offset = tick;
	}

	return offset;
}

void Track::liveRecord(const MidiMessage& message)
{
	auto data = message.getRawData();
	midiData tmp = { message.getTimeStamp(), data[0], uint8_t(message.getRawDataSize() > 1 ? data[1] : 0), uint8_t(message.getRawDataSize() > 2 ? data[2] : 0) };

	auto ret = recordData.write(tmp);
	if (ret == false)
	{
		assert(0);
	}
}

void Track::setTrackRxMidiPortAndChannel(void)
{
	Instrument* rxInstrument = getTrackRxInstrument();
	if (rxInstrument == nullptr)
	{
		return;
	}

	_rxMidiPort = rxInstrument->getMidiPort();
	_rxMidiChannel = rxInstrument->getMidiChannel();
}
void Track::setTrackTxMidiPortAndChannel(void)
{
	Instrument* instrumentToPlay = getTrackTxInstrument();
	if (instrumentToPlay == nullptr)
	{
		return;
	}

	_txMidiPort = instrumentToPlay->getMidiPort();
	_txMidiChannel = instrumentToPlay->getMidiChannel();
}

bool Track::getSystemSoloMode(void)
{
	IgniteParamList param;
	std::vector<IgniteParamList> paramList, ret;

	param.param = (paramId_t)SettingsSceneCnfgParam::SettingSceneSoloModeParam;
	paramList.push_back(param);

	ret = _songManager->engine.settingsSceneCnfg->getParamList((moduleId_t)ModuleName::moduleTrack, paramList);

	if (ret.size() == 1)
	{
		if (ret[0].MsgValue.get<int8_t>())
		{
			return true;
		}
		else
		{
			return false;
		}	
	}
	else
	{
		return false;
	}
}

int8_t Track::getSceneSwing(void)
{
	if (_songManager && _songManager->getPlayScene())
	{
		return _songManager->getPlayScene()->getSwing();
	}
	else
	{
		return 0;
	}
}

void Track::getSequenceStep(std::string patternName)
{
	int8_t barId;
	int16_t stepId;
	int16_t totalSteps;
	if (_songManager->_state == SYSTEM_STATE::LIVE_RECORD)
	{
		//always use linear ascending steps in record - pattern direction is ignored
		std::tie(barId, stepId, std::ignore) = _songManager->engine.sequenceManager->getLinearNextStepOfBar(_trackBarId, _trackStepIndex, patternName);

		if ((barId != -1) && (stepId != -1))
		{
			//update the play step id, before publish
			_trackBarId = barId;//during init it will have to be updated from -1
			_trackPlayStepId = stepId;

		}//if ((barId != -1) && (stepId != -1))
		else
		{
			//no valid next step is not found
			//it will currently stay at the last valid step
			//If required, add any action like play/record STOP here
		}

		// While recording
		// if baroverwrite is NOT allowed and
		// if all bars are exhausted, stop recording
		// This way any track can stop the record as soon as its data is about to get overwritten
		if ((_trackBarId == _recordFirstBar) && (_trackPlayStepId == _recordFirstStep))
		{
			auto barOverWrite = _songManager->_recordSetting.find((paramId_t)SettingsRecordCnfgParam::SettingRecordBarOverWriteParam);
			if (barOverWrite->second == false)
			{
				_songManager->stopRecord();
				return;
			}
		}
	}
	else // if (_songManager->_state == SYSTEM_STATE::LIVE_RECORD)
	{
		std::tie(barId, stepId, totalSteps) = _songManager->engine.sequenceManager->getNextStepOfBar(_trackBarId, _trackStepIndex, patternName);

		//Valid step not found
		//stay at current step
		if ((barId != -1) && (stepId != -1))
		{
			_stepCountForCurrentBar = totalSteps;

			//update the play step id, before publish
			_trackBarId = barId;//during init it will have to be updated from -1
			_trackPlayStepId = stepId;

		}//if ((barId != -1) && (stepId != -1))
		else
		{
			//no valid next step is not found
			//it will currently stay at the last valid step
			//If required, add any action like play/record STOP here
		}

	}// if (_songManager->_state == SYSTEM_STATE::LIVE_RECORD) else
}


void Track::checkAndHandleTimeBaseUpdate(std::string patternName)
{
	TimeBase tb = _songManager->engine.sequenceManager->getPatternTimeBase(patternName);
	_pulsesPerStep = timeBaseToPps(tb);

	_pulsesPerTick = SET_PPQN / TB4_TICKS_PER_STEP;
}


void Track::playStepEvents(std::string patternName)
{
	//Time to get the event list
	MidiEventVector eventMap;
	TIMER_START_2(14);
	if (_songManager->_state == SYSTEM_STATE::LIVE_RECORD)
	{
		eventMap = _songManager->engine.sequenceManager->getLinearStepEventsAtOffset(patternName, _trackBarId, _trackPlayStepId, _trackUiTick);
	}
	else
	{
		eventMap = _songManager->engine.sequenceManager->getStepEventsAtOffset(patternName, _trackBarId, _trackPlayStepId, _trackUiTick);
	}
	TIMER_STOP_2(14);

	for (auto& event : eventMap)
	{
		//TIMER_START_3(23);
		//juce code does not have a generic handler, it is specific about type and lengths
		//use appropriate juce overloads
		MidiMessage msg;

		if (((event.type & 0xF0) == 0xC0) || ((event.type & 0xF0) == 0xD0))
		{
			TIMER_START_2(15);
			msg = MidiMessage{ _songManager->initialByte(event.type, _txMidiChannel), event.data1 };
			TIMER_STOP_2(15);
		}
		else
		{
			TIMER_START_2(16);
			//Transpose and FTS
			bool transposeDefeatStatus = getNodeTransposeDefeatStatus() || getTxInstrumentTransposeDefeatStatus() || getBarTransposeDefeatStatus(_trackBarId);
			TIMER_STOP_2(16);

			TIMER_START_2(17);
			bool ftsDefeatStatus = getTxInstrumentFtsDefeatStatus();
			TIMER_STOP_2(17);

			TIMER_START_2(18);
			_songManager->applyFtsAndTranspose(event.data1, transposeDefeatStatus, ftsDefeatStatus);
			TIMER_STOP_2(18);

			TIMER_START_2(19);
			msg = MidiMessage{ _songManager->initialByte(event.type, _txMidiChannel), event.data1, event.data2 };
			TIMER_STOP_2(19);

			TIMER_START_2(20);
			if (msg.isNoteOnOrOff())
			{
				updatePlayingNoteList(msg);
			}
			TIMER_STOP_2(20);
		}

		TIMER_START_2(21);
		Instrument* instrumentToPLay = getTrackTxInstrument();
		TIMER_STOP_2(21);

		TIMER_START_2(22);
		if (instrumentToPLay != nullptr)
		{
			_songManager->sendMidiMessage(msg, _txMidiPort, getTrackTxInstrument()->getInternalSound(), this);
		}
		TIMER_STOP_2(22);
		//TIMER_STOP_3(23);//per event
	}
}

void Track::recordStepEvents(void)
{
	//RECORD ACTION - LIVE & LIVE TRIGGERED
	if (_songManager->_state == LIVE_RECORD)
	{
		if (_recordFirstBar == -1)//set only once at start
		{
			_recordFirstBar = _trackBarId;
			_recordFirstStep = _trackPlayStepId;
		}

		if (_trackData->trackRecordEnable)
		{
			recordStepData(_trackPlayStepId, _trackBarId, _trackUiTick);
		}
	}
}

void Track::advanceUiTick(void)
{
	++_trackUiTick;
	if (_trackUiTick >= (_pulsesPerStep / _pulsesPerTick))
	{
		_trackUiTick = 0;
	}
}

void Track::handleSwing (Pattern *pattern)
{
	auto swing = getSceneSwing();
	if (swing > 0)
	{
		if (_trackUiTick == (pattern->getTicksPerStep() - swing))
		{
			if (_trackStepIndex % 2 == 0)//every second step is moved 
			{
				_newStep = true;
				_trackPulse = -1;
				_trackUiTick = 0;
			}
		}
	}
}

void Track::advanceTrackPulse(void)
{
	++_trackPulse;
	
	//Step change
	if ((_trackPulse >= _pulsesPerStep) || (_trackPulse == 0)) //end of step || begining of step
	{
		_trackPulse = 0;
		_newStep = true;
	}
}

bool Track::isTrackBarEndReached(int16_t stepIndexWithinBar)
{
	if (stepIndexWithinBar >= (_stepCountForCurrentBar - 1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Track::handleStepChange(std::string patternName, int16_t sceneBarId, int16_t sceneStepId)
{
	if (_newStep)
	{
		bool newBar = false;
		int8_t prevTrackBarId = -1;

		//DBG("Track Step CH -> Scene Bar Id : " + String(sceneBarId) + " : Scene Step Id : " + String(sceneStepId));
		if (isSceneBarChange(sceneBarId) && (_songManager->_state != SYSTEM_STATE::LIVE_RECORD))
		{
			//DBG("SCENE BAR CHANGE");
			_currentSceneBarId = sceneBarId;
			
			if (isGBarSyncOn(_trackBarId))
			{
				prevTrackBarId = _trackBarId;
				_trackBarId++;
				_trackStepIndex = 0;
				newBar = true;
				//DBG("SBC : GSYN ON : Scene + TRACK BAR FORCE CHANGE");
			}
			else
			{
				if (isTrackBarEndReached(_trackStepIndex))
				{
					//DBG("SBC : GSYN OFF : Scene + TRACK BAR END CHANGE");
					prevTrackBarId = _trackBarId;
					_trackBarId++;
					_trackStepIndex = 0;
					newBar = true;
				}
				else
				{
					//DBG("SBC : GSYN OFF : Scene + TRACK STEP INC");
					_trackStepIndex++;
				}
			}
		}
		else
		{
			//DBG("NO SCENE BAR CHANGE");
			if (isTrackBarEndReached(_trackStepIndex))
			{
				//DBG("NSBC : End of track");
				if (isGBarSyncOn(_trackBarId) && (_songManager->_state != SYSTEM_STATE::LIVE_RECORD))
				{
					//DBG("NSBC : GSYN ON : RE-START TRACK BAR");
					prevTrackBarId = _trackBarId;
					_trackStepIndex = 0;
					newBar = true;
				}
				else
				{
					//DBG("NSBC : GSYN OFF : SWITCH TO NEXT TRACK BAR");
					prevTrackBarId = _trackBarId;
					_trackBarId++;
					_trackStepIndex = 0;
					newBar = true;
				}
			}
			else
			{
				//DBG("NSBC/NTBC : TRACK STEP INC");
				_trackStepIndex++;
			}
		}

		if (newBar)
		{
			newBar = false;//reset
			actionsAtBarBoundary(patternName, prevTrackBarId);
		}

		checkAndHandlePatternLengthChange();

		//DBG("Track Step CH -> Scene Bar Id : " + String(_currentSceneBarId) + " : Track Bar id : " + String(_trackBarId) + " : Track Step Index : " + String(_trackStepIndex));		
	}//new step
}

void Track::handleStepChangeWithout_Gbar_BarRepeat_BarLoop(std::string patternName)
{		
	if (isTrackBarEndReached(_trackStepIndex))
	{
		_trackBarId++;
		_trackStepIndex = 0;
	}
	else
	{
		_trackStepIndex++;
	}	

	checkAndHandlePatternLengthChange();
}

void Track::recordTimeSequenceManagerUpdate(void)
{
	//update sequnce (at every step interval), if live record is in progress
	if (_songManager->_state == SYSTEM_STATE::LIVE_RECORD)
	{
		if (_patternUpdatedByRecord.load(std::memory_order_acquire))
		{
			_patternUpdatedByRecord.store(false, std::memory_order_release);

			auto pattern = getTrackPattern();

			if (pattern)
			{
				std::string patternName = pattern->getName();

				//delegate to a background thread, to avoid loading the playback
#ifdef _WIN32
                auto lock = _songManager->engine.lockDesktopState();
                seqMgrNotifyAndUpdate(_songManager->engine.sequenceManager, patternName, _trackBarId, _trackPlayStepId);
#else
				std::thread seqMgrNotifyAndUpdateThread(seqMgrNotifyAndUpdate, _songManager->engine.sequenceManager, patternName, _trackBarId, _trackPlayStepId);
				seqMgrNotifyAndUpdateThread.detach();
#endif
			}

		}
	}
}

void Track::checkAndHandlePatternLengthChange(void)
{
	auto pattern = getTrackPattern();
	if (pattern)
	{
		auto patternName = pattern->getName();

		//Check if pattern was modified to a new length (smaller), so that it excludes current play step or bar
		//reset to start
		auto  patternBarCount = pattern->getTotalBars();
		auto  patternLastStep = pattern->getLastStepId();


		if ((_trackBarId >= patternBarCount) || (_trackPlayStepId >= patternLastStep))
		{
			_trackBarId = 0;
			_trackPlayStepId = 0;
		}
	}//if(pattern)
}

void Track::publishInfoToUI(void)
{
	auto now = std::chrono::system_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUiPublish).count() > FREQ_UI_UPDATE_PLAYTIME)
	{
		lastUiPublish = now;
	}
	else
		return;

	auto pattern = getTrackPattern();
	if (pattern)
	{
		auto  patternBarCount = pattern->getTotalBars();
		auto  patternLastStep = pattern->getLastStepId();

		//Get latest pattern data and Check if pattern was modified to a new length (smaller), so that it excludes current play step or bar
		//Don't send step id / bar id to UI, as it would be out of range at UI's updated pattern.
		//Next step both engine and UI would be in sync
		
		if ((patternBarCount > _trackBarId) && (patternLastStep >= _trackPlayStepId) && (_trackBarId >=0) && (_trackPlayStepId >= 0))
		{
			//DBG("TRACK ->  BAR : " + String(_trackBarId) + " : STEP " + String(_trackPlayStepId));// - play time print don't enbale unless absolutely needed
			publishTrackBarAndStepInfoToUI();
		}
	}//if(pattern)
}

int Track::getTsfPresetFromIgniteIndex(int index)
{
	int preset = 0;

	switch (index)
	{
	case StandardDrum:
		preset = tsfStandardDrum;
		break;

	case RoomDrum:
		preset = tsfRoomDrum;
		break;

	case PowerDrum:
		preset = tsfPowerDrum;
		break;

	case ElectronicDrum:
		preset = tsfElectronicDrum;
		break;

	case TR808Drum:
		preset = tsfTR808Drum;
		break;

	case JazzDrum:
		preset = tsfJazzDrum;
		break;

	case BrushDrum:
		preset = tsfBrushDrum;
		break;

	case OrchestraDrum:
		preset = tsfOrchestraDrum;
		break;

	case SFXDrum:
		preset = tsfSFXDrum;
		break;

	case CM63_32SetDrum:
		preset = tsfCM63_32SetDrum;
		break;

	default:
		//defaulting to 0
		break;
	}

	return preset;
}

bool Track::isSceneBarChange(int16_t sceneBarId)
{
	if (_currentSceneBarId != sceneBarId)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// TODO Move this to seq manger to avoid race(if any)
bool Track::isGBarSyncOn(int16_t barId)
{
	auto pattern = getTrackPattern();
	if (pattern != nullptr)
	{
		auto iter = pattern->_patternData->barList.find(barId);
		if (iter != pattern->_patternData->barList.end())
		{
			return iter->second._BarData->gBarSyncOn;
		}
	}

	return false;
}

void Track::actionsAtStepBoundary(void)
{
	//limit to step boundary
	if (_newStep)
	{
		_newStep = false;
		recordTimeSequenceManagerUpdate();

		publishInfoToUI();
	}
}

/* Keep track of all notes in the current track, which are currently ON.
 * Used at abrupt end secnarios to send associated note off
 */
void Track::updatePlayingNoteList(MidiMessage msg)
{
	std::lock_guard<std::mutex> lock(_NoteListMtx);
	if (msg.isNoteOn())
	{
		_playingNoteList.emplace(static_cast<uint8_t>((msg.getRawData())[1]), static_cast<uint8_t>(( (msg.getRawData())[0]) & 0xF));
	}
	else
	{
		_playingNoteList.erase(static_cast<uint8_t>((msg.getRawData())[1]));
	}
}

/* Used at track de-init
 * Send note off for the notes in the list
 * Clear the list
 */
void Track::stopPlayingNotes(void)
{
	std::lock_guard<std::mutex> lock(_NoteListMtx);
	auto playlist = _playingNoteList;
	_playingNoteList.clear();
	Instrument* instrumentToPLay = getTrackTxInstrument();
	if (instrumentToPLay != nullptr)
	{
		for (auto& noteAndChannel : playlist)
		{
			auto msg = _songManager->noteOff(noteAndChannel.second, noteAndChannel.first, 0);
			_songManager->sendMidiMessage(msg, _txMidiPort, getTrackTxInstrument()->getInternalSound(), this);
		}
	}

}

/* At bar change, check for
1 . Bar Repeat
2 . Bar Loop
*/
void Track::actionsAtBarBoundary(std::string patternName, int8_t previousTrackBarId)
{
	if (_songManager->_state != SYSTEM_STATE::LIVE_RECORD)
	{
		PatternInfo patternInfo = _songManager->engine.sequenceManager->getUpdatedInfo(patternName);

		auto currentBar = patternInfo.barInfoList.find(previousTrackBarId);
		if (currentBar != patternInfo.barInfoList.end())
		{
			//check if bar repeat is there
			auto patternBarRepeatCount = currentBar->second.repeatCount;

			if (patternBarRepeatCount > 0)
			{
				if (_trackBarRepeatCount < patternBarRepeatCount)
				{
					++_trackBarRepeatCount;

					//current bar repeating from step 1
					_trackBarId = previousTrackBarId;
					_trackStepIndex = 0;
				}
				else //bar already repeated for required count - proceed to next
				{
					//reset
					_trackBarRepeatCount = 0;

					//check for bar loop
					if (std::get<0>(patternInfo.barloop) == true)
					{
						//current bar = last bar of the loop
						if (previousTrackBarId == std::get<1>(patternInfo.barloop).endBarId)
						{
							//continue playing from FIRST bar of the loop.
							_trackBarId = std::get<1>(patternInfo.barloop).startBarId;
							_trackStepIndex = 0;

						}
						else // current bar not last bar of the loop
						{
							// continue playing NEXT bar - bar and step already set
						}
					}
					else //no bar loop
					{
						// continue playing NEXT bar - bar and step already set
					}
				}
			}// (patternBarRepeatCount > 0)
			else
			{
				//check for bar loop
				if (std::get<0>(patternInfo.barloop) == true)
				{
					//current bar = last bar of the loop
					if (previousTrackBarId == std::get<1>(patternInfo.barloop).endBarId)
					{
						//continue playing from FIRST bar of the loop.
						_trackBarId = std::get<1>(patternInfo.barloop).startBarId;
						_trackStepIndex = 0;

					}
					else // current bar not last bar of the loop
					{
						// continue playing NEXT bar - bar and step already set
					}
				}
				else //no bar loop
				{
					// continue playing NEXT bar - bar and step already set
				}
			}

		}//(currentBar != patternInfo.barInfoList.end())

	}//(_songManager->_state != SYSTEM_STATE::LIVE_RECORD)

}

void Track::unloadDesktopSoundFont()
{
    tsf_close(static_cast<tsf*>(_trackSf));
    _trackSf = nullptr;
}
void Track::prepareDesktopMonitor()
{
    if (_trackSf == nullptr)
    {
        resetEffects();
        effectsInit();
        sfLoad();
    }
}
