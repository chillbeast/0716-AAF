#include <string>
#include "common_defs/source/EngineDataStruct.h"
#include "Common/Modules/Scene.h"

using namespace Ignite;
//class SongManager;

Scene::Scene (int32_t id, std::string name, float tempo, int8_t swing, uint8_t numtracks, uint64_t trackMute, SongManager* songManager)
{
	_sceneData = new (SceneData);
	*_sceneData = { 0 };

	_sceneData->sceneId = id;
	_sceneData->sceneName = name;
	_sceneData->sceneTempo = tempo;
	_sceneData->sceneSwing = swing;
	_sceneData->numberOfTracks = numtracks;
	_sceneData->trackMuteStatus = trackMute;
	_sceneData->transposeAfterFTS = false;
	_sceneData->gBarLenInSteps = 16;
	_sceneData->numBars = 1;
	_sceneData->mode = (uint8_t)SceneAdvanceMode::SceneAdvanceAuto;
	_sceneData->scaleFTS = (int8_t)ScaleList::ScaleAllNotes;
	_sceneData->noteFTS = (int8_t)FTSRootNote::NoteC;
	_sceneData->transposeValue = 0;

	editTrackId = -1;

	editTrack = nullptr;

	gStepId = -1;
	gBarId = -1;
	_songManager = songManager;

	std::string trackName;

	for (int8_t i = 0; i < numtracks; i++)
	{
		trackName = _sceneData->sceneName + "_Track_" + std::to_string (i + 1);
		bool mute = ((_sceneData->trackMuteStatus >> i) & 0x1) == 1 ? true : false;
		Track newTrack = Track (i, trackName, mute, _songManager);
		_sceneData->trackList.push_back (newTrack);
	}
}

Scene::Scene (const Scene& source)
    : _sceneData(new SceneData(*source._sceneData)), editTrackId(source.editTrackId),
      gStepId(source.gStepId), gBarId(source.gBarId), _songManager(source._songManager),
      _sceneMuteHold(source._sceneMuteHold)
{
    rebindSelection();
}

Scene& Scene::operator=(const Scene& source)
{
    if (this != &source)
    {
        Scene copy(source);
        std::swap(_sceneData, copy._sceneData);
        editTrackId = source.editTrackId;
        gStepId = source.gStepId;
        gBarId = source.gBarId;
        _songManager = source._songManager;
        _sceneMuteHold = source._sceneMuteHold;
        rebindSelection();
    }
    return *this;
}

void Scene::rebindSelection()
{
    editTrack = nullptr;
    for (auto& track : _sceneData->trackList)
        if (track.getTrackId() == editTrackId) editTrack = &track;
}

Scene::~Scene ()
{
	delete _sceneData;
}

void Scene::setTempo (float val)
{
	if (_sceneData == nullptr)
	{
		//TODO WARN
		return;
	}
	_sceneData->sceneTempo = val;
}

void Scene::setSwing (int8_t val)
{
	_sceneData->sceneSwing = val;
}

void Scene::setNumOfTracks (int8_t val)
{
	_sceneData->numberOfTracks = val;
}

void Scene::addTrack (Track& newTrack)
{
	_sceneData->trackList.push_back (newTrack);
	++(_sceneData->numberOfTracks);
}

void Scene::addTrack (int8_t val)
{
	//default set to 16.
	//If user adds more, add it
	if (val > _sceneData->numberOfTracks)
	{
		std::string trackName;
		bool mute = true;//mute by default

		for (int8_t i = _sceneData->numberOfTracks; i < val; i++)
		{
			trackName = _sceneData->sceneName + "_Track_" + std::to_string (i + 1);
			_sceneData->trackList.push_back (Track (i, trackName, mute, _songManager));
		}
		_sceneData->numberOfTracks = val;
	}
	else
	{
		//TODO log debug msg
	}
}

bool Scene::setEditTrack (int8_t index)
{
	bool retval = false;

	if (index >= _sceneData->numberOfTracks)
	{
		//TODO log debug msg
	}
	else
	{
		if (index == editTrackId)
		{
			retval = true;
		}
		else
		{
			auto start = _sceneData->trackList.begin ();
			auto end = _sceneData->trackList.end ();

			for (auto i = start; i != end; i++)
			{
				if ((*i)._trackData->trackId == index)
				{
					editTrack = &(*i);
					editTrackId = index;
					retval = true;
					break;
				}
			}
		}
	}
	return retval;
}

void Scene::setName (std::string name)
{
	_sceneData->sceneName.clear ();
	_sceneData->sceneName = name;
}

void Scene::setSceneAdvanceMode (uint8_t val)
{
	_sceneData->mode = val;
}

float Scene::getTempo (void) const
{
	return _sceneData->sceneTempo;
}

int8_t Scene::getSwing (void) const
{
	return _sceneData->sceneSwing;
}

int8_t Scene::getNumOfTracks (void) const
{
	return _sceneData->numberOfTracks;
}

std::string Scene::getName (void)
{
	return _sceneData->sceneName;
}

int32_t Scene::getId (void)
{
	return _sceneData->sceneId;
}

int8_t Scene::getEditTrackId (void)
{
	return editTrackId;
}

void Scene::setStepsPerBar (int8_t val)
{
	if (val > MAX_SCENE_GBAR_IN_NUM_STEPS)
	{
		val = MAX_SCENE_GBAR_IN_NUM_STEPS;
	}
	_sceneData->gBarLenInSteps = val;
}

void Scene::setBarCount (uint16_t val)
{
	if (val > MAX_SCENE_LEN_IN_NUM_BARS)
	{
		val = MAX_SCENE_LEN_IN_NUM_BARS;
	}
	_sceneData->numBars = val;
}

void Scene::setFTSRootNote (int8_t val)
{
	//default to C, if out of range
	if ((val < (int8_t)FTSRootNote::NoteMin) || (val > (int8_t)FTSRootNote::NoteMax))
	{
		val = (int8_t)FTSRootNote::NoteC;
	}
	_sceneData->noteFTS = val;
}

void Scene::setFTSScale (int8_t val)
{
	//default to major scale, if value out of range
	if ((val <= (int8_t)ScaleList::ScaleMin) || (val >= (int8_t)ScaleList::ScaleMax))
	{
		val = (int8_t)ScaleList::ScaleAllNotes;
	}
	_sceneData->scaleFTS = val;
}

void Scene::setTransposeValue (int8_t val)
{
	//disable transpose, if vaue is out of range
	if ((val < -12) || (val > 12))
	{
		val = 0;
	}
	_sceneData->transposeValue = val;
}

void Scene::setTransposeFTSPriority (int8_t val)
{
	if ((val != (int8_t)TransposeFTSPriority::TransposePreFTS) && (val != (int8_t)TransposeFTSPriority::TransposePostFTS))
	{
		return;
	}
	_sceneData->transposeAfterFTS = (bool)(val == (int8_t)TransposeFTSPriority::TransposePostFTS ? true : false);
}

int8_t Scene::getStepsPerBar (void)
{
	return _sceneData->gBarLenInSteps;
}

int16_t Scene::getBarCount (void)//max200
{
	return _sceneData->numBars;
}

int8_t Scene::getFTSRootNote (void)
{
	return _sceneData->noteFTS;
}

int8_t Scene::getFTSScale (void)
{
	return _sceneData->scaleFTS;
}

int8_t Scene::getTransposeValue (void)
{
	return _sceneData->transposeValue;
}

int8_t Scene::getTransposeFTSPriority (void)
{
	return _sceneData->transposeAfterFTS;
}

int8_t Scene::getSceneAdvanceMode (void)
{
	return _sceneData->mode;
}

void Scene::setSceneTrackMuteStatus (uint64_t status)
{
	_sceneData->trackMuteStatus = status;
}

uint64_t Scene::getSceneTrackMuteStatus (void)
{
	return _sceneData->trackMuteStatus;
}

void Scene::setId (int32_t id)
{
	_sceneData->sceneId = id;
}

void Scene::deleteTrack (int8_t newTrackCount, std::string songName)
{
	if (newTrackCount <= 0)
	{
		_sceneData->trackList.clear ();
		_sceneData->numberOfTracks = newTrackCount;
	}
	else
	{
		auto trackStart = _sceneData->trackList.begin ();
		auto trackEnd = _sceneData->trackList.end ();

		std::advance (trackStart, newTrackCount);
		auto TmptrackStart = trackStart;

		while (TmptrackStart != trackEnd) // update used by info for each track
		{
			if(!TmptrackStart->_trackData->fillPatternName.empty() && TmptrackStart->_trackData->fillPatternName != " ")
				TmptrackStart->_trackData->fillPattern->RemoveFromUsedByList(true, songName, _sceneData->sceneName, TmptrackStart->_trackData->trackName);
			if (!TmptrackStart->_trackData->mainPatternName.empty() && TmptrackStart->_trackData->mainPatternName != " ")
				TmptrackStart->_trackData->mainPattern->RemoveFromUsedByList(false, songName, _sceneData->sceneName, TmptrackStart->_trackData->trackName);
			if (!TmptrackStart->_trackData->trackRxInstrumentName.empty() && TmptrackStart->_trackData->trackRxInstrumentName != " ")
				TmptrackStart->_trackData->trackRxInstrument->RemoveFromUsedByList(false, songName, _sceneData->sceneName, TmptrackStart->_trackData->trackName);
			TmptrackStart++;
		}
		_sceneData->trackList.erase (trackStart, trackEnd);
		_sceneData->numberOfTracks = newTrackCount;
		setEditTrack(newTrackCount - 1);
		// Clear MUTE status
		uint64_t tempTrackMuteStatus = _sceneData->trackMuteStatus;
		tempTrackMuteStatus = tempTrackMuteStatus & (~((uint64_t)0xFFFFFFFFFFFFFFFF << newTrackCount));
		_sceneData->trackMuteStatus = tempTrackMuteStatus | (((uint64_t)0xFFFFFFFFFFFFFFFF << newTrackCount));
	}
}

void Scene::modifyTrackCount (int8_t newCount, std::string songName)
{
	if (newCount < _sceneData->numberOfTracks)
	{
		deleteTrack (newCount, songName);
	}
	else if (newCount > _sceneData->numberOfTracks)
	{
		addTrack (newCount);
	}
}

void Scene::deInitScenePlaySettings (void)
{
	for (auto& trackIter : _sceneData->trackList)
	{
		trackIter.deInitTrackPlaySetting ();
	}
}

void Scene::setSceneMuteHold (MuteHoldType val)
{
	_sceneMuteHold = val;
}

MuteHoldType Scene::getSceneMuteHold (void)
{
	return _sceneMuteHold;
}

/* stop all notes which are currently playing from all tracks, of this scene */
void Scene::stopPlayingNotes(void)
{
	for (auto& track : _sceneData->trackList)
	{
		track.stopPlayingNotes();
	}
}
