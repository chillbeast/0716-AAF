#pragma once
#include <stdint.h>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include "Common/Modules/Track.h"
#include "Common/Modules/ISyncListener.h"
#include "Firmware/Application/Model/SongManager.h"

namespace Ignite
{
	struct SceneData;
	class Track;
	class SongManager;

	class Scene :public ISyncListener
	{
	public:
		explicit Scene (int32_t id, std::string name, float tempo, int8_t swing, uint8_t numtracks, uint64_t trackMute, SongManager* songManager);
		Scene (const Scene&);
		Scene& operator= (const Scene& source);
		~Scene ();

		void setTempo (float val);
		void setSwing (int8_t val);
		void setNumOfTracks (int8_t val);
		void addTrack (int8_t val);
		void addTrack (Track& newTrack);
		void setName (std::string name);
		bool setEditTrack (int8_t index);
		void setSceneAdvanceMode (uint8_t mode);
		void setStepsPerBar (int8_t val);
		void setBarCount (uint16_t val);
		void setFTSRootNote (int8_t val);
		void setFTSScale (int8_t val);
		void setTransposeValue (int8_t val);
		void setTransposeFTSPriority (int8_t val);
		void setSceneTrackMuteStatus (uint64_t status);
		void setId (int32_t id);//to be used only by reAssignSequentialIdsForScenes*()
		void deInitScenePlaySettings (void);

		float getTempo (void) const;
		int8_t getSwing (void) const;
		int8_t getNumOfTracks (void) const;
		std::string getName (void);
		int32_t getId (void);
		int8_t getEditTrackId (void);
		int8_t getStepsPerBar (void);
		int16_t getBarCount (void);
		int8_t getFTSRootNote (void);
		int8_t getFTSScale (void);
		int8_t getTransposeValue (void);
		int8_t getTransposeFTSPriority (void);
		int8_t getSceneAdvanceMode (void);
		uint64_t getSceneTrackMuteStatus (void);
		void deleteTrack (int8_t newTrackCount, std::string songName);
		void modifyTrackCount (int8_t newCount, std::string songName);
		MuteHoldType getSceneMuteHold (void);
		void setSceneMuteHold (MuteHoldType val);

		void stopPlayingNotes (void);

		SceneData* _sceneData;

		int8_t editTrackId;

		Track* editTrack = nullptr;

		int16_t gStepId = 0;
		int16_t gBarId = 0;
		SongManager* _songManager = nullptr;

		MuteHoldType _sceneMuteHold = MuteHoldType::Off;
	private:
		void rebindSelection();
	};
}
