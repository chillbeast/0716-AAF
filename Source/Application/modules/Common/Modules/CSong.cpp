#include <string>
#include "Common/Modules/CSong.h"
#include "common_defs/source/EngineDataStruct.h"

using namespace Ignite;
//class SongManager;

CSong::CSong (int32_t id, std::string name, float tempo, uint8_t swing, int32_t numScenes, SongManager* songmgr/*, bool mode*/)
{
	_songData = new (SongData);
	*_songData = { 0 };

	_songData->songId = id;
	_songData->songName = name;
	_songData->songTempo = tempo;
	_songData->songSwing = swing;
	_songData->numberOfScenes = numScenes;
	_songData->gTrackCount = 16;//TODO take as an input fom UI while creating song
	_songData->applySongSwingToScenes = true;
	_songData->applySongTempoToScenes = true;

	std::string sceneName;
	float sceneTempo = _songData->songTempo;
	uint8_t sceneSwing = _songData->songSwing;
	uint8_t numTracks = _songData->gTrackCount;
	uint64_t trackMute = 0xFFFFFFFFFFFFFFFF;//mute all tracks by default

	editSceneId = -1;
	playSceneId = -1;
	playScene = nullptr;
	editScene = nullptr;
	startSceneId = -1;
	endSceneId = -1;
	jumpSceneId = -1;
	_songManager = songmgr;
	//isFillMode = mode; // default false

	for (int i = 0; i < _songData->numberOfScenes; i++)
	{
		sceneName = _songData->songName + "_Scene_" + std::to_string (i + 1);
		Scene newScene = Scene (i, sceneName, sceneTempo, sceneSwing, numTracks, trackMute, _songManager);
		_songData->sceneList.push_back (newScene);
	}
}

CSong::CSong (const CSong& source)
    : _songData(new SongData(*source._songData)), editSceneId(source.editSceneId),
      playSceneId(source.playSceneId), startSceneId(source.startSceneId), endSceneId(source.endSceneId),
      jumpSceneId(source.jumpSceneId), _songManager(source._songManager), songNameDirtyFlag(source.songNameDirtyFlag)
{
    rebindSelection();
}

CSong& CSong::operator=(const CSong& source)
{
    if (this != &source)
    {
        CSong copy(source);
        std::swap(_songData, copy._songData);
        editSceneId = source.editSceneId;
        playSceneId = source.playSceneId;
        startSceneId = source.startSceneId;
        endSceneId = source.endSceneId;
        jumpSceneId = source.jumpSceneId;
        _songManager = source._songManager;
        songNameDirtyFlag = source.songNameDirtyFlag;
        rebindSelection();
    }
    return *this;
}

// Rebuilding a song after its position changes must preserve all song settings.
CSong::CSong (const SongData songData, SongManager* songMgr, int32_t editId, int32_t playId,
              int32_t startId, int32_t endId, int32_t jumpId)
    : _songData(new SongData(songData)), editSceneId(editId), playSceneId(playId),
      startSceneId(startId), endSceneId(endId), jumpSceneId(jumpId), _songManager(songMgr)
{
    rebindSelection();
}

void CSong::rebindSelection()
{
    editScene = playScene = nullptr;
    for (auto& scene : _songData->sceneList)
    {
        if (scene.getId() == editSceneId) editScene = &scene;
        if (scene.getId() == playSceneId) playScene = &scene;
    }
}

CSong::~CSong ()
{
	delete _songData;
}

void CSong::setTempo (float val)
{
	_songData->songTempo = val;

	// If global flag is true, All scenes tempo should be set to song tempo
	if (_songData->applySongTempoToScenes == true)
	{
		for (auto& scene : _songData->sceneList)
		{
			scene.setTempo (val);
			//Change playback tempo, if Scene is currently playing
			if (_songManager->getActiveSong ()->editScene == _songManager->getActiveSong ()->playScene)
			{
				CMidiClock::getInstance ().setTempo (_songManager->getActiveSong ()->playScene->getTempo ());
			}
		}
	}
}

void CSong::setSwing (uint8_t val)
{
	_songData->songSwing = val;

	// If global flag is true, All scenes swing should be set to song swing
	if (_songData->applySongSwingToScenes == true)
	{
		for (auto& scene : _songData->sceneList)
		{
			scene.setSwing (val);
		}
		//for current playng scene, new swing will take effect during next step
	}
}

void CSong::setNumOfScenes (int32_t val)
{
	_songData->numberOfScenes = val;
}

bool CSong::setEditScene (int32_t index)
{
	bool retval = false;

	if (index >= _songData->numberOfScenes)
	{
		//TODO log debug msg
	}
	else
	{
		if (index == editSceneId)
		{
			retval = true;//already set
		}
		else
		{
			auto start = _songData->sceneList.begin ();
			auto end = _songData->sceneList.end ();

			for (auto i = start; i != end; i++)
			{
				if ((*i)._sceneData->sceneId == (uint32_t)index)
				{
					editSceneId = index;
					editScene = &(*i);
					retval = true;
					break;
				}
			}
		}
	}
	return retval;
}

bool CSong::setPlayScene (int32_t index)
{
	bool retval = false;

	if (index >= _songData->numberOfScenes)
	{
		//TODO log debug msg
	}
	else
	{
		auto start = _songData->sceneList.begin ();
		auto end = _songData->sceneList.end ();
		auto i = start;

		for (i = start; i != end; i++)
		{
			if ((*i)._sceneData->sceneId == (uint32_t)index)
			{
				playSceneId = index;
				playScene = &(*i);
				retval = true;
				break;
			}
		}
	}
	return retval;
}

bool CSong::setStartSceneId (int32_t index)
{
	bool retval = false;

	if (index >= _songData->numberOfScenes)
	{
		//TODO log debug msg
	}
	else
	{
		// loop clearing
		if (index == -1)
		{
			startSceneId = -1;
			retval = true;
		}
		else if (index == startSceneId)
		{
			retval = true; //already set
		}
		else
		{
			auto start = _songData->sceneList.begin ();
			auto end = _songData->sceneList.end ();
			auto i = start;

			for (i = start; i != end; i++)
			{
				if ((*i)._sceneData->sceneId == (uint32_t)index)
				{
					startSceneId = index;
					retval = true;
					break;
				}
			}
		}
	}
	return retval;
}

bool CSong::setEndSceneId (int32_t index)
{
	bool retval = false;

	if (index >= _songData->numberOfScenes)
	{
		//TODO log debug msg
	}
	else
	{
		// loop clearing
		if (index == -1)
		{
			endSceneId = -1;
			retval = true;
		}
		else if (index == endSceneId)
		{
			retval = true; //already set
		}
		else
		{
			auto start = _songData->sceneList.begin ();
			auto end = _songData->sceneList.end ();
			auto i = start;

			for (i = start; i != end; i++)
			{
				if ((*i)._sceneData->sceneId == (uint32_t)index)
				{
					endSceneId = index;
					retval = true;
					break;
				}
			}
		}
	}
	return retval;
}

void CSong::setName (std::string name)
{
	_songData->songName.clear ();
	_songData->songName = name;
	songNameDirtyFlag = true;
}

float CSong::getTempo (void) const
{
	return _songData->songTempo;
}

uint8_t CSong::getSwing (void) const
{
	return _songData->songSwing;
}

int32_t CSong::getNumOfScenes (void) const
{
	return _songData->numberOfScenes;
}

std::string CSong::getName (void)
{
	return _songData->songName;
}

int32_t CSong::getEditSceneId (void)
{
	return editSceneId;
}

int32_t CSong::getPlaySceneId (void)
{
	return playSceneId;
}

Scene* CSong::getPlayScene (void)
{
	return playScene;
}

void CSong::addScene (Scene& newScene, bool sortTracks, bool reAssignTrackIds)
{
	if (sortTracks == true)
	{
		//sort tracks within scene as per their id
		sortTracksInScene (newScene);
	}

	if (reAssignTrackIds == true)
	{
		//re-assign sequentail ids to all tracks in the list
		reAssignSequentialIdsForTracks (newScene);
	}

	_songData->sceneList.push_back (newScene);
	++(_songData->numberOfScenes);

	//updateSongInfoList();
}

int32_t CSong::getStartSceneId (void)
{
	return startSceneId;
}

int32_t CSong::getEndSceneId (void)
{
	return endSceneId;
}

Scene* CSong::getEditScene (void)
{
	return editScene;
}

struct sortTrackCompareFunction
{
	bool operator()(Track& first, Track& second)
	{
		return (first.getTrackId () < second.getTrackId ());
	}
};

void CSong::sortTracksInScene (Scene& scene)
{
	scene._sceneData->trackList.sort (sortTrackCompareFunction ());
}

void CSong::reAssignSequentialIdsForTracks (Scene& scene)
{
	auto trackListStart = scene._sceneData->trackList.begin ();
	auto trackListEnd = scene._sceneData->trackList.end ();

	//Re-assign ids starting from 0
	int8_t count = 0;

	for (auto& trackIter = trackListStart; trackIter != trackListEnd; ++trackIter)
	{
		(*trackIter).setTrackId (count);
		++count;
	}
}
int32_t CSong::getId (void)
{
	return _songData->songId;
}

void CSong::setId (int32_t id)
{
	_songData->songId = id;
}
/*
void CSong::setFillMode(bool mode)
{
	//isFillMode = mode;
}

bool CSong::getFillMode(void)
{
//    return isFillMode;
}
*/

bool CSong::getApplySongTempoStatus (void)
{
	return _songData->applySongTempoToScenes;
}

bool CSong::getApplySongSwingStatus (void)
{
	return _songData->applySongSwingToScenes;
}

void CSong::setApplySongTempoStatus (bool state)
{
	_songData->applySongTempoToScenes = state;

	// All scenes tempo should be set to song tempo
	if (state == true)
	{
		auto songTempo = getTempo ();

		setTempo(songTempo);
	}
}

void CSong::setApplySongSwingStatus (bool state)
{
	_songData->applySongSwingToScenes = state;

	// All scenes swing should be set to song swing
	if (state == true)
	{
		auto songSwing = getSwing ();

		setSwing(songSwing);
	}
}

/** songInfoList is the current list of open songs(id and name)	, managed by SongManager
 *  Update any changes to individual song, to the SongInfolist.
 */
void CSong::updateSongInfoList (void)
{
	auto thisSongId = this->getId ();
	auto songInfo = _songManager->songInfoList.find (thisSongId);
	if (songInfo != _songManager->songInfoList.end ())
	{

		songInfo->second = this->_songData->songName;
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "Song info not in list");
	}
}
