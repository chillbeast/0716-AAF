#pragma once
#include <stdint.h>
#include <string>
#include "Common/Modules/CSong.h"
#include "Common/Modules/ISyncListener.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
//#include "MidiTransport.h"
#include "Firmware/Application/Model/ModelParams.h"
#include "CSoundEngine.h"
#include "module.h"
#include "Common/Modules/Scene.h"

namespace Ignite
{
	class IgniteEngine;
	class CSong;
	class Scene;
	using namespace juce;
	class SceneManager : public Module, public ISyncListener
	{
	public:
		SceneManager(IParamChangeListener* listener, IgniteEngine& model, SongManager* songMgr);
		SceneManager(const SceneManager&) = default;
		SceneManager& operator= (const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator= (SceneManager&&) = delete;
		~SceneManager() = default;

		void setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode);
		std::vector<IgniteParamList> getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist) override;
		void SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode);
		void getParam(moduleId_t moduleSrc, paramId_t param) override;
		void onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);

		void manualPlayNewScene (int32_t sceneId);
		void markSaved()
		{
			isUndilistNeedUpdate = false;
			SetDefaultUndoListData();
			ClearUndoList();
		}

	private:
		IgniteEngine& engine;
		SongManager* SongMgr = nullptr;
		IParamChangeListener* paramlistener;
		bool isUndilistNeedUpdate = true;

		void SetDefaultUndoListData();
		void createAndAddScene(void);
		void modifySceneNameIfDuplicate(std::list<Scene>& sceneList, std::string& name);
		void addMultipleScenes(int32_t val);
		void removeScene(int32_t sceneId);
		void reAssignSequentialIdsForScenes(CSong& song);
		void setSongEditScene(int32_t sceneId);
		void setSceneSwing(uint8_t val);
		void setSceneTempo(float val);
		void setSceneName(std::string name);
		bool saveScene(CSong& song, Scene& scene);
		void setStartScene(int32_t index);
		bool saveScene(void);
		void setEndScene(int32_t index);
		void setSceneAdvanceMode(uint8_t val);
		void setSceneGbar(int8_t val);
		void setSceneLength(int16_t val);
		void setSceneFTSRootNote(int8_t index);
		void setSceneFTSScale(int8_t index);
		void setSceneTranspose(int8_t val);
		void setSceneTransposeFTSPriority(int8_t val);
		void manualPlayNextScene(void);
		std::string getSceneName(void);
		int8_t getSceneSwing();
		float getSceneTempo();
		int32_t getNumOfScenes(void);
		int8_t getSceneGbar(void);
		int16_t getSceneLength(void);
		int8_t getSceneFTSRootNote(void);
		int8_t getSceneFTSScale(void);
		int8_t getSceneTranspose(void);
		int8_t getSceneTransposeFTSPriority(void);
		int8_t getSceneAdvanceMode(void);
		int8_t getMuteHold(void);
		bool copyScene(std::string sceneName);
		void moveScene(int32_t newId);
		void reAssignSequentialIdsForScenesAndSave(CSong& song);
		void saveSceneInfo(SceneData& sceneData);
		void setMuteHold(int8_t val);
		void releaseSceneResources (int32_t sceneId, bool isDelete);
		std::string getSceneNameFromSceneId (int32_t id);
		void SaveData();
	};
}

