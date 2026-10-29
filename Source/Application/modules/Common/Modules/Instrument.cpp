#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include <memory>
#include "Common/Modules/Instrument.h"
#include "Firmware/Application/Model/ModelParams.h"

using namespace Ignite;
std::map <int8_t, std::string> tvDefaultNameMap =
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
std::map <int8_t, TVMidiCCParams> tvDefaultCCMap =
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
Instrument::Instrument (int32_t id, std::string name, int8_t port, int8_t chan, InstrumentManager* instMgr)
{
	_instrumentData = new (InstrumentData);
	*_instrumentData = { {},{},0, };

	_instrumentData->instrumentName = name;
	_instrumentData->instrumentDispName = name;
	_instrumentData->midiPort = port;
	_instrumentData->midiChannel = chan;
	_instrumentData->defaultNote = (uint8_t)Note::noteC4;
	_instrumentData->id = id;
	_instrumentData->tm.volume = 63;
	_instrumentData->tm.pan = 63;
	_instrumentMgr = instMgr;
	_activeKnobId = -1;

	for (int8_t i = 0; i < MAX_NUMBER_OF_TVKNOB; ++i)
	{
		TrackValue tmp;
		tmp.knobId = i;
		tmp.name = tvDefaultNameMap[i];
		tmp.paramType = TrackValueType::TVTypeMidiCC;//NOTE : for internal sound UI holding min and max valus
		tmp.param.tvMidiCC.initValue = tvDefaultCCMap[i].initValue;
		tmp.param.tvMidiCC.maxValue = tvDefaultCCMap[i].maxValue;
		tmp.param.tvMidiCC.minValue = tvDefaultCCMap[i].minValue;
		tmp.param.tvMidiCC.num = tvDefaultCCMap[i].num;
		tmp.param.tvMidiCC.value = tvDefaultCCMap[i].value;

		//_instrumentData->tv.emplace(i, tmp);
		addTVKnob(i, tmp);
	}
	_instrumentData->tvCount = (uint8_t)_instrumentData->tv.size ();
}

Instrument::Instrument (InstrumentData instrumentData, /*int8_t tvKnob,*/ InstrumentManager* instMgr)
{
	_instrumentData = new (InstrumentData);
	*_instrumentData = { {},{},0, };

	_instrumentData->instrumentName = instrumentData.instrumentName;
	_instrumentData->instrumentDispName = instrumentData.instrumentDispName;
	_instrumentData->midiPort = instrumentData.midiPort;
	_instrumentData->midiChannel = instrumentData.midiChannel;
	_instrumentData->defaultNote = instrumentData.defaultNote;
	_instrumentData->internalSoundPgm = instrumentData.internalSoundPgm;
	_instrumentData->internalSound = instrumentData.internalSound;
	_instrumentData->multiTimbralStatus = instrumentData.multiTimbralStatus;
	_instrumentData->polySpreadStatus = instrumentData.polySpreadStatus;
	_instrumentData->transposeDefeatStatus = instrumentData.transposeDefeatStatus;
	_instrumentData->ftsDefeatStatus = instrumentData.ftsDefeatStatus;
	_instrumentData->editTrackThruStatus = instrumentData.editTrackThruStatus;
	_instrumentData->cc0BankMStatus = instrumentData.cc0BankMStatus;
	_instrumentData->cc0BankLStatus = instrumentData.cc0BankLStatus;
	_instrumentData->displayNoteNum = instrumentData.displayNoteNum;
	_instrumentData->preSendPgmChange = instrumentData.preSendPgmChange;
	_instrumentData->UsedByCount = instrumentData.UsedByCount;
	_instrumentData->usedByList = instrumentData.usedByList;
	_instrumentData->tv = instrumentData.tv;
	_instrumentData->tvCount = instrumentData.tvCount;
	_instrumentData->id = instrumentData.id;
	_instrumentData->tm = instrumentData.tm;
	_instrumentMgr = instMgr;
	_instrumentData->pgmChangeData = instrumentData.pgmChangeData;
    _instrumentData->soundFontPath=instrumentData.soundFontPath;
}

Instrument::Instrument (const Instrument& source)
    : _instrumentMgr(source._instrumentMgr), _instrumentData(new InstrumentData(*source._instrumentData)),
      _activeKnobId(source._activeKnobId)
{
}

Instrument& Instrument::operator=(const Instrument& source)
{
    if (this != &source)
    {
        auto data = std::make_unique<InstrumentData>(*source._instrumentData);
        delete _instrumentData;
        _instrumentData = data.release();
        _activeKnobId = source._activeKnobId;
        _instrumentMgr = source._instrumentMgr;
    }
    return *this;
}

Instrument::~Instrument ()
{
	delete _instrumentData;
}

void Instrument::setMidiPort (int8_t val)
{
	//Validate the input
	if ((val >= static_cast<uint8_t> (InstrumentPorts::midiPort1)) && (val < static_cast<uint8_t> (InstrumentPorts::MaxMidiPort)))
	{
		_instrumentData->midiPort = val;
	}
	else
	{
		assert(0);
	}
}

void Instrument::setMidiChannel (int8_t val)
{
	_instrumentData->midiChannel = val;
}

void Instrument::setName (std::string name)
{
	_instrumentData->instrumentDispName = name;
}

int8_t Instrument::getMidiPort (void) const
{
	return _instrumentData->midiPort;
}

int8_t Instrument::getMidiChannel (void) const
{
	return _instrumentData->midiChannel;
}

std::string Instrument::getName (void) const
{
	return _instrumentData->instrumentName;
}
std::string Instrument::getDispName(void) const
{
	return _instrumentData->instrumentDispName;
}
void Instrument::setDefaultNote (uint8_t val)
{
	_instrumentData->defaultNote = val;
}
void Instrument::setInternalSoundPgm (int val)
{
	_instrumentData->internalSoundPgm = val;
}
void Instrument::setInternalSound (bool val)
{
	_instrumentData->internalSound = val;
}
void Instrument::setMultiTimbralStatus (bool val)
{
	_instrumentData->multiTimbralStatus = val;
}
void Instrument::setPolySpreadStatus (bool val)
{
	_instrumentData->polySpreadStatus = val;
}
void Instrument::setTransposeDefeatStatus (bool val)
{
	_instrumentData->transposeDefeatStatus = val;
}
void Instrument::setFtsDefeatStatus (bool val)
{
	_instrumentData->ftsDefeatStatus = val;
}
void Instrument::setEditTrackThruStatus (bool val)
{
	_instrumentData->editTrackThruStatus = val;
}
void Instrument::setCc0BankMStatus (bool val)
{
	_instrumentData->cc0BankMStatus = val;
}
void Instrument::setCc0BankLStatus (bool val)
{
	_instrumentData->cc0BankLStatus = val;
}
void Instrument::setDisplayNoteNum (bool val)
{
	_instrumentData->displayNoteNum = val;
}
void Instrument::setPreSendPgmChange (bool val)
{
	_instrumentData->preSendPgmChange = val;
}
uint8_t Instrument::getDefaultNote ()
{
	return(_instrumentData->defaultNote);
}
int Instrument::getInternalSoundPgm ()
{
	return(_instrumentData->internalSoundPgm);
}
bool Instrument::getInternalSound ()
{
	return(_instrumentData->internalSound);
}
bool Instrument::getMultiTimbralStatus ()
{
	return(_instrumentData->multiTimbralStatus);
}
bool Instrument::getPolySpreadStatus ()
{
	return(_instrumentData->polySpreadStatus);
}
bool Instrument::getTransposeDefeatStatus ()
{
	return(_instrumentData->transposeDefeatStatus);
}
bool Instrument::getFtsDefeatStatus ()
{
	return(_instrumentData->ftsDefeatStatus);
}
bool Instrument::getEditTrackThruStatus ()
{
	return(_instrumentData->editTrackThruStatus);
}
bool Instrument::getCc0BankMStatus ()
{
	return(_instrumentData->cc0BankMStatus);
}
bool Instrument::getCc0BankLStatus ()
{
	return(_instrumentData->cc0BankLStatus);
}
bool Instrument::getDisplayNoteNum ()
{
	return(_instrumentData->displayNoteNum);
}
bool Instrument::getPreSendPgmChange ()
{
	return(_instrumentData->preSendPgmChange);
}
bool Instrument::UpdateUsedByList (bool isFillPattern, std::string SongName, std::string SceneName, std::string TrackName)
{
#if defined DISABLE_USED_BY_LIST
	auto it = std::find_if (_instrumentData->usedByList.begin (), _instrumentData->usedByList.end (), [&](const UsedByData usedByData)
		{
			if ((usedByData.isFillPattern == isFillPattern) &&
				(usedByData.songName == SongName) &&
				(usedByData.sceneName == SceneName) &&
				(usedByData.trackName == TrackName))
				return (true);
			else
				return (false);
		});

	if (it == _instrumentData->usedByList.end ())
	{
		UsedByData usedByData;
		usedByData.isFillPattern = isFillPattern;
		usedByData.songName = SongName;
		usedByData.sceneName = SceneName;
		usedByData.trackName = TrackName;
		_instrumentData->usedByList.push_back (usedByData);
		_instrumentData->UsedByCount++;
		return true;
	}
	return false;
#else
	return true;
#endif//DISABLE_USED_BY_LIST
}
bool Instrument::RemoveFromUsedByList (bool isFillPattern, std::string SongName, std::string SceneName, std::string TrackName)
{
#if defined DISABLE_USED_BY_LIST
	auto it = std::find_if (_instrumentData->usedByList.begin (), _instrumentData->usedByList.end (), [&](const UsedByData usedByData)
		{
			if ((usedByData.isFillPattern == isFillPattern) &&
				(usedByData.songName == SongName) &&
				(usedByData.sceneName == SceneName) &&
				(usedByData.trackName == TrackName))
				return (true);
			else
				return (false);
		});

	if (it != _instrumentData->usedByList.end ())
	{
		_instrumentData->usedByList.erase (it);
		_instrumentData->UsedByCount--;
	}
	saveInstrumentUsedByList ();
#endif//DISABLE_USED_BY_LIST
	return true;
}

void Instrument::addTVKnob (int8_t knobId, TrackValue tvData)
{
	//check if already exist
	if (_instrumentData->tv.count (knobId) == 0)
	{
		_instrumentData->tv.insert ({ knobId, tvData });
		_instrumentData->tvCount = (uint8_t)_instrumentData->tv.size();

		/*
		//add track specific value part entry
		if ( (_instrumentMgr->engine.songManager != nullptr) &&
			(_instrumentMgr->engine.songManager->activeSong != nullptr) &&
			(_instrumentMgr->engine.songManager->activeSong->editScene != nullptr) &&
			(_instrumentMgr->engine.songManager->activeSong->editScene->editTrack != nullptr))
		{
			_instrumentMgr->engine.songManager->activeSong->editScene->editTrack->addTvKnobToTrack (knobId, tvData);
		}
		*/
	}
	else
	{
		assert (0);
	}
}

void Instrument::deleteTVKnob (int8_t knobId)
{
	if (_instrumentData->tv.count (knobId) == 1)
	{
		_instrumentData->tv.erase (knobId);
		_instrumentData->tvCount = (uint8_t)_instrumentData->tv.size ();

		//delete track specific value part entry
		_instrumentMgr->engine.songManager->activeSong->editScene->editTrack->deleteTvTrackData (_activeKnobId);
	}
	else
	{
		assert (0);
	}
}

void Instrument::setTVActiveKnob (int8_t knobId)
{
	_activeKnobId = knobId;
}

int8_t Instrument::getTVActiveKnob (void)
{
	return _activeKnobId;
}

void Instrument::setTVParamType (TrackValueType type)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		if (_instrumentData->tv.at (_activeKnobId).paramType != type)
		{
			// Tv type (cc or pgmchange) is set and saved at both instrument knob data and track knob data
			// Avoids resetting knob level pgm change values (msb, lsb pgm num), to instrument's pgm change default value;
			// when setTVParamType is called multiple times with pgm change as type.
			// Only for the first time (ie type changes from cc tp pgm change) we load values from
			// instrument definition pgm change fields, to knob's pgm change field.
			// Then onwards as long as the knob remains in pgm change mode, only user input updates the values (msb, lsb, num)
			_instrumentData->tv.at (_activeKnobId).paramType = type;

			if (type == TrackValueType::TVTypePgmChange)
			{
				loadInstrumentLevelPgmChangeData (_activeKnobId);
			}

			//copy to track tv
			if ((_instrumentMgr->engine.songManager != nullptr) &&
				(_instrumentMgr->engine.songManager->activeSong != nullptr) &&
				(_instrumentMgr->engine.songManager->activeSong->editScene != nullptr) &&
				(_instrumentMgr->engine.songManager->activeSong->editScene->editTrack != nullptr))
			{
				auto editTrack = _instrumentMgr->engine.songManager->activeSong->editScene->editTrack;
				auto editTrackTxInstrument = editTrack->getTrackTxInstrument ();
				if (this->getId () == editTrackTxInstrument->getId ())
				{
					editTrack->setTvKnobType (_activeKnobId, type);
				}

			}
		}
	}
	else
	{
		assert (0);
	}
}

void Instrument::setTVEffectStatus(bool status)
{
	if (_instrumentData->tv.count(_activeKnobId) != 0)
	{
		if (status)
		{
			//Instrument level - update here for sending output midi
			sendTVMidiMessage(_instrumentData->tv[_activeKnobId], _activeKnobId);
		}
		else
		{
			TrackValue dummytv;
			dummytv = _instrumentData->tv[_activeKnobId];
			//Instrument level - update here for sending output midi
			dummytv.param.tvMidiCC.value = 0;
			sendTVMidiMessage(dummytv, _activeKnobId);
		}
	}
	else
	{
		assert(0);
	}
}

int8_t Instrument::getTVParamType (void)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		return static_cast <int8_t> (_instrumentData->tv.at (_activeKnobId).paramType);
	}
	else
	{
		return -1;
	}
}

void Instrument::setTVMidiCCNum (int8_t num)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		_instrumentData->tv.at (_activeKnobId).param.tvMidiCC.num = num;
		_instrumentData->tv.at (_activeKnobId).paramType = TrackValueType::TVTypeMidiCC;
	}
	else
	{
		assert (0);

	}
}

int8_t Instrument::getTVMidiCCNum (void)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		return _instrumentData->tv.at (_activeKnobId).param.tvMidiCC.num;
	}
	else
	{
		return -1;
	}
}

void Instrument::setTVName (std::string name)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		_instrumentData->tv.at (_activeKnobId).name = name;
	}
	else
	{
		assert (0);
	}
}

std::string Instrument::getTVName (void)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		return _instrumentData->tv.at (_activeKnobId).name;
	}
	else
	{
		return "Invalid";
	}
}

void Instrument::setTVMidiCCValue (float val)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		//Track level
		TrackValue tmp;
		tmp.paramType = TrackValueType::TVTypeMidiCC;
		tmp.param.tvMidiCC.value = val;
		_instrumentMgr->engine.songManager->activeSong->editScene->editTrack->setTvCcKnobValue (_activeKnobId, tmp);

		//Instrument level - update here for sending output midi
		_instrumentData->tv[_activeKnobId].param.tvMidiCC.value = val;
		sendTVMidiMessage (_instrumentData->tv[_activeKnobId], _activeKnobId);
	}
	else
	{
		assert (0);
	}
}

float Instrument::getTVMidiCCValue (void)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		return _instrumentMgr->engine.songManager->activeSong->editScene->editTrack->getTvCcKnobValue (_activeKnobId);
	}
	else
	{
		return -1;
	}
}

void Instrument::setTVMidiCCInitValue (int8_t val)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		_instrumentData->tv.at (_activeKnobId).param.tvMidiCC.initValue = val;
	}
	else
	{
		assert (0);
	}
}

int8_t Instrument::getTVMidiCCInitValue (void)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		return _instrumentData->tv.at (_activeKnobId).param.tvMidiCC.initValue;
	}
	else
	{
		return -1;
	}
}

void Instrument::setTVMidiCCMinValue (int8_t val)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		_instrumentData->tv.at (_activeKnobId).param.tvMidiCC.minValue = val;
	}
	else
	{
		assert (0);
	}
}

int8_t Instrument::getTVMidiCCMinValue (void)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		return _instrumentData->tv.at (_activeKnobId).param.tvMidiCC.minValue;
	}
	else
	{
		return -1;
	}
}

void Instrument::setTVMidiCCMaxValue (int8_t val)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		_instrumentData->tv.at (_activeKnobId).param.tvMidiCC.maxValue = val;
	}
	else
	{
		assert (0);
	}
}

int8_t Instrument::getTVMidiCCMaxValue (void)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		return _instrumentData->tv.at (_activeKnobId).param.tvMidiCC.maxValue;
	}
	else
	{
		return -1;
	}
}

void Instrument::setTVMidiPgmNumber (int8_t val, TvLocation location)
{
	if (location == TvLocation::InstrumentLevel)
	{
		_instrumentData->pgmChangeData.midiPgmNum = val;

		//Bank and sound selection is end to conncted device
		TrackValue tv;
		tv.paramType = TrackValueType ::TVTypePgmChange;
		tv.param.tvPgmChange.bankLSB = _instrumentData->pgmChangeData.bankLSB;
		tv.param.tvPgmChange.bankMSB = _instrumentData->pgmChangeData.bankMSB;
		tv.param.tvPgmChange.midiPgmNum = _instrumentData->pgmChangeData.midiPgmNum;

		if (isValidPgmChangeData (tv) == true)
		{
			sendTVMidiMessage (tv, 0);
		}
	}
	else if (location == TvLocation::TrackLevel)//Track level
	{
		//Set at track level
		TrackValue tmp;
		tmp.paramType = TrackValueType::TVTypePgmChange;
		tmp.param.tvPgmChange.midiPgmNum = val;

		_instrumentMgr->engine.songManager->activeSong->editScene->editTrack->setTVPgmChangeKnobVal (_activeKnobId, tmp);

		//Bank and sound selection is send to connected device
		auto tv = _instrumentMgr->engine.songManager->activeSong->editScene->editTrack->getTv (_activeKnobId);
		if (isValidPgmChangeData(tv) == true)
		{
			sendTVMidiMessage (tv, 0);
		}
	}
}

int8_t Instrument::getTVMidiPgmNumber (TvLocation location)
{
	int8_t ret = -1;

	if (location == TvLocation::TrackLevel)
	{
		if ((_instrumentMgr->engine.songManager != nullptr) &&
			(_instrumentMgr->engine.songManager->activeSong != nullptr) &&
			(_instrumentMgr->engine.songManager->activeSong->editScene != nullptr) &&
			(_instrumentMgr->engine.songManager->activeSong->editScene->editTrack != nullptr))
		{
			ret = _instrumentMgr->engine.songManager->activeSong->editScene->editTrack->getTvPgmChangeKnobValue (_activeKnobId);
		}
	}
	else if (location == TvLocation::InstrumentLevel)
	{
		ret = _instrumentData->pgmChangeData.midiPgmNum;
	}

	return ret;
}

void Instrument::setTVMidiPgmBankLsb (int8_t val, TvLocation location)
{
	if (location == TvLocation::InstrumentLevel)
	{
		_instrumentData->pgmChangeData.bankLSB = val;
	}
	else if (location == TvLocation::TrackLevel)
{
		TrackValue tmp;
		tmp.paramType = TrackValueType::TVTypePgmChange;
		tmp.param.tvPgmChange.bankLSB = val;
		_instrumentMgr->engine.songManager->activeSong->editScene->editTrack->setTVPgmChangeLsb (_activeKnobId, tmp);
	}
}

int8_t Instrument::getTVMidiPgmBankLsb (TvLocation location)
{
	int8_t ret = -1;

	if (location == TvLocation::TrackLevel)
	{
		if ((_instrumentMgr->engine.songManager->activeSong->editScene != nullptr) &&
			(_instrumentMgr->engine.songManager->activeSong->editScene->editTrack != nullptr))
		{
			ret = _instrumentMgr->engine.songManager->activeSong->editScene->editTrack->getTVPgmChangeLsb (_activeKnobId);
		}
	}
	else if (location == TvLocation::InstrumentLevel)
	{
			ret = _instrumentData->pgmChangeData.bankLSB;
	}
	return ret;
}

void Instrument::setTVMidiPgmBankMsb (int8_t val, TvLocation location)
{
	if (location == TvLocation::InstrumentLevel)
	{
		_instrumentData->pgmChangeData.bankMSB = val;
	}
	else if (location == TvLocation::TrackLevel)
	{
		TrackValue tmp;
		tmp.paramType = TrackValueType::TVTypePgmChange;
		tmp.param.tvPgmChange.bankMSB = val;
		_instrumentMgr->engine.songManager->activeSong->editScene->editTrack->setTVPgmChangeMsb (_activeKnobId, tmp);
	}
}

int8_t Instrument::getTVMidiPgmBankMsb (TvLocation location)
{
	int8_t ret = -1;

	if (location == TvLocation::TrackLevel)
	{
		if ((_instrumentMgr->engine.songManager->activeSong->editScene != nullptr) &&
			(_instrumentMgr->engine.songManager->activeSong->editScene->editTrack != nullptr))
		{
			ret = _instrumentMgr->engine.songManager->activeSong->editScene->editTrack->getTVPgmChangeMsb (_activeKnobId);
		}
	}
	else if (location == TvLocation::InstrumentLevel)
	{
		ret = _instrumentData->pgmChangeData.bankMSB;
	}

	return ret;
}

void Instrument::setTrackValueScope (uint8_t saveMode)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
	{
		switch (saveMode)
		{
			case (int)TrackValueSaveType::TVSaveActiveToScene:
				_instrumentData->tv.at (_activeKnobId).savedAt = TrackValueScope::TVAtSceneLevel;
				break;

			case (int)TrackValueSaveType::TVSaveActiveToSong:
				_instrumentData->tv.at (_activeKnobId).savedAt = TrackValueScope::TVAtSongLevel;
				break;

			case (int)TrackValueSaveType::TVSaveAllToScene:
				break;

			case (int)TrackValueSaveType::TVSaveAllToSong:
				break;

			default:
				assert (0);
				break;
		}
	}
	else
	{
		assert (0);
	}
}

uint8_t Instrument::getTrackValueScope (void)
{
	if (_instrumentData->tv.count (_activeKnobId) != 0)
		return (uint8_t)_instrumentData->tv.at (_activeKnobId).savedAt;
	else
		return 0;
}

void Instrument::setId (int32_t val)
{
	_instrumentData->id = val;
}

int32_t Instrument::getId (void)
{
	return _instrumentData->id;
}

// Midi cc can be  set to multiple knobs, each with diffrent cc number,
// so using vector to get all midi ccs.
// All others are params are unique, only one knob should control these parameters
bool Instrument::getTrackValueMidiCC (std::vector <TVMidiCCParams>& ccData)
{
	bool ret = false;
	auto trackValueList = _instrumentData->tv;

	for (auto tv : trackValueList)
	{
		if ((tv.second.paramType == TrackValueType::TVTypeMidiCC))
		{
			ccData.push_back (tv.second.param.tvMidiCC);
			break;
		}
	}

	return ret;
}

void Instrument::sendTVMidiMessage (TrackValue tvToSend, int8_t knobId)
{
	MidiMessage msg;
	//int8_t knobBank = knobId / 8;
	auto track = _instrumentMgr->engine.songManager->activeSong->editScene->editTrack;
	if (knobId < 8)//CC value knobID bank 0
	{
		switch (tvToSend.paramType)
		{
		case TrackValueType::TVTypeMidiCC:
			msg = SongManager::controllerEvent(getMidiChannel(), tvToSend.param.tvMidiCC.num, (int)tvToSend.param.tvMidiCC.value);
			_instrumentMgr->engine.songManager->sendMidiMessage (msg, getMidiPort (), getInternalSound (), track);
			_instrumentMgr->engine.songManager->handleTVMidiMessageOnRecord(msg, getMidiPort (), getInternalSound (), track);
			break;

		case TrackValueType::TVTypePgmChange:
			// select bank, if specified by user
			if ((tvToSend.param.tvPgmChange.bankLSB != -1) && (tvToSend.param.tvPgmChange.bankMSB != -1))
			{
				msg = SongManager::controllerEvent(getMidiChannel(), 0x00, (int)tvToSend.param.tvPgmChange.bankMSB);
				_instrumentMgr->engine.songManager->sendMidiMessage (msg, getMidiPort (), getInternalSound (), track);

				msg = SongManager::controllerEvent(getMidiChannel(), 0x20, (int)tvToSend.param.tvPgmChange.bankLSB);
				_instrumentMgr->engine.songManager->sendMidiMessage (msg, getMidiPort (), getInternalSound (), track);
			}
			msg = SongManager::programChange(getMidiChannel(), (int)tvToSend.param.tvPgmChange.midiPgmNum);
			_instrumentMgr->engine.songManager->sendMidiMessage (msg, getMidiPort (), getInternalSound (), track);
			break;

		default:
			assert(0);
			break;
		}
	}
	else if(getInternalSound())
	{
		bool alternate = (knobId >= 16);
		track->setInternalAudioEffects(knobId, tvToSend.param.tvMidiCC.value, alternate);
	}
}

int8_t Instrument::getTrackMixerVolume (void)
{
	return _instrumentData->tm.volume;
}

void Instrument::setTrackMixerVolume (int8_t value)
{
	_instrumentData->tm.volume = value;
	sendTMMidiMessage (_instrumentData->tm, TrackMixerType::Volume);
}

int8_t Instrument::getTrackMixerPan (void)
{
	return _instrumentData->tm.pan;
}

void Instrument::setTrackMixerPan (int8_t value)
{
	_instrumentData->tm.pan = value;
	sendTMMidiMessage (_instrumentData->tm, TrackMixerType::Pan);
}

void Instrument::sendTMMidiMessage (TrackMixer tmToSend, TrackMixerType type)
{
	MidiMessage msg;

	switch (type)
	{
		case TrackMixerType::Pan:
			msg = SongManager::controllerEvent (getMidiChannel (), 0xA, tmToSend.pan);
			break;

		case TrackMixerType::Volume:
			msg = SongManager::controllerEvent (getMidiChannel (), 0x7, tmToSend.volume);
			break;

		default:
			assert (0);
			break;
	}
	auto track = _instrumentMgr->engine.songManager->activeSong->editScene->editTrack;
	_instrumentMgr->engine.songManager->sendMidiMessage (msg, getMidiPort (), getInternalSound (), track);
}

void Instrument::saveInstrumentUsedByList (void)
{
#if defined DISABLE_USED_BY_LIST
	auto instData  = _instrumentMgr->engine.fileHandler->GetInstrumentData (_instrumentData->instrumentName);

	//take updated used-by-list
	instData.UsedByCount = _instrumentData->UsedByCount;
	instData.usedByList.clear();
	instData.usedByList = _instrumentData->usedByList;

	//TODO : remove below DeleteInstrument after HW test, this may create race condition when multiple req in place by used by for the same file
	//_instrumentMgr->engine.fileHandler->DeleteInstrument(_instrumentData->instrumentName);
	_instrumentMgr->engine.fileHandler->saveInstrument(_instrumentData->instrumentName, instData);
#endif//DISABLE_USED_BY_LIST
}

uint8_t Instrument::getTrackValueCount (void)
{
	return _instrumentData->tvCount;
}

bool Instrument::getTrackValuePresenceStatus (int8_t knobId)
{
	if (_instrumentData->tv.count (knobId) == 0)
	{//does not exist
		return false;
	}
	else//exists
	{
		return true;
	}
}

void Instrument::saveInstrumentTV (void)
{
	auto instData = _instrumentMgr->engine.fileHandler->GetInstrumentData (_instrumentData->instrumentName);

	//take updated used-by-list
	instData.tvCount = _instrumentData->tvCount;
	instData.tv.clear ();
	instData.tv = _instrumentData->tv;

	_instrumentMgr->engine.fileHandler->DeleteInstrument (_instrumentData->instrumentName);
	_instrumentMgr->engine.fileHandler->saveInstrument (_instrumentData->instrumentName, instData);
}

bool Instrument::isValidPgmChangeData (TrackValue tv)
{
	if ((tv.paramType == TrackValueType::TVTypePgmChange) &&
		(tv.param.tvPgmChange.bankLSB != -1) &&
		(tv.param.tvPgmChange.bankMSB != -1) &&
		(tv.param.tvPgmChange.midiPgmNum != -1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Instrument::loadInstrumentLevelPgmChangeData (int8_t knobId)
{
	_instrumentData->tv[knobId].param.tvPgmChange.bankLSB = _instrumentData->pgmChangeData.bankLSB;
	_instrumentData->tv[knobId].param.tvPgmChange.bankMSB = _instrumentData->pgmChangeData.bankMSB;
	_instrumentData->tv[knobId].param.tvPgmChange.midiPgmNum = _instrumentData->pgmChangeData.midiPgmNum;
}
