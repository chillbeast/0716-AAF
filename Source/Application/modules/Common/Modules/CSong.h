#pragma once
#include <stdint.h>
#include <vector>
#include <string>

#include "Scene.h"
#include "Firmware/Application/Model/SongManager.h"


namespace Ignite
{
	struct SongData;

	class CSong
	{
	public:
		explicit CSong (int32_t id, std::string name, float tempo, uint8_t swing, int32_t numscenes, SongManager* songmgr/*,bool mode=false*/);
		CSong (const CSong& source);
		CSong (const SongData songData, SongManager* songMgr, int32_t editSceneId, int32_t playSceneId, int32_t startSceneId, int32_t endSceneId, int32_t jumpSceneId);
		CSong& operator= (const CSong& source);
		~CSong ();

		void setTempo (float val);
		void setSwing (uint8_t val);
		void setNumOfScenes (int32_t val);
		void setName (std::string name);
		void setId (int32_t id);//to be used only by reAssignSequentialIdsForSongs*()
		bool setEditScene (int32_t index);
		bool setPlayScene (int32_t index);
		bool setEndSceneId (int32_t index);
		bool setStartSceneId (int32_t index);
		//void setFillMode(bool mode);

		float getTempo (void) const;
		uint8_t getSwing (void) const;
		int32_t getNumOfScenes (void) const;
		std::string getName (void);
		int32_t getId (void);

		int32_t getEditSceneId (void);
		int32_t getPlaySceneId (void);
		Scene* getPlayScene (void);
		int32_t getStartSceneId (void);
		int32_t getEndSceneId (void);
		//bool getFillMode(void);
		void addScene (Scene& newScene, bool sortTracks, bool reAssignTrackIds);
		Scene* getEditScene (void);

		void sortTracksInScene (Scene& sceneToSort);
		void reAssignSequentialIdsForTracks (Scene& scene);
		bool getApplySongTempoStatus (void);
		bool getApplySongSwingStatus (void);
		void setApplySongTempoStatus (bool state);
		void setApplySongSwingStatus (bool state);

		void updateSongInfoList (void);

		SongData* _songData;

		int32_t editSceneId = -1;
		int32_t playSceneId = -1;

		Scene* editScene = nullptr;
		Scene* playScene = nullptr;

		int32_t startSceneId = -1;
		int32_t endSceneId = -1;
		int32_t jumpSceneId = -1;

		//bool isFillMode = false;

		SongManager* _songManager = nullptr;

		bool songNameDirtyFlag = false;
	private:
		void rebindSelection();
	};
}
