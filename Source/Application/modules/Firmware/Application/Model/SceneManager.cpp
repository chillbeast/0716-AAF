#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Firmware/Application/Model/SongManager.h"
#include "Firmware/Application/Model/SceneManager.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Common/Modules/Track.h"

using namespace Ignite;
class Track;

SceneManager::SceneManager(IParamChangeListener* listener, IgniteEngine& model, SongManager* songMgr) : paramlistener(listener), engine(model), SongMgr(songMgr), Module("SceneManager", (moduleId_t)ModuleName::moduleScene, (unsigned int)SceneParam::paramSceneMax, listener)
{
	AddParameter<int32_t>((paramId_t)SceneParam::paramAddSceneCmd, 0, 127, 0);// no arg
	AddParameter<int32_t>((paramId_t)SceneParam::paramAddTotalScenesCmd, 0, 1000, 0);// total scene count as arg
	AddParameter<int32_t>((paramId_t)SceneParam::paramDeleteSceneCmd, 0, 1000, 0);
	AddParameter<int32_t>((paramId_t)SceneParam::paramActiveScene, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)SceneParam::paramSceneSwing, 0, 16, 0);
	AddParameter<float>((paramId_t)SceneParam::paramSceneTempo, 20.0, 360.0, 130.0);
	AddParameter<std::string>((paramId_t)SceneParam::paramSceneName, "0", "0", "0");
	AddParameter<int32_t>((paramId_t)SceneParam::paramCurrentPlaySceneId, 0, 1000, 0);
	AddParameter<int32_t>((paramId_t)SceneParam::paramNextPlaySceneId, 0, 1000, 0);
	AddParameter<uint8_t>((paramId_t)SceneParam::paramSaveSceneCmd, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)SceneParam::paramUndoCmd, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)SceneParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int32_t>((paramId_t)SceneParam::paramStartScene, 0, 1000, 0);
	AddParameter<int32_t>((paramId_t)SceneParam::paramEndScene, 0, 1000, 0);
	AddParameter<uint8_t>((paramId_t)SceneParam::paramSceneAdvanceMode, 0, 1, 0);
	AddParameter<int32_t>((paramId_t)SceneParam::paramSceneCount, 0, 1000, 0);
	AddParameter<std::string>((paramId_t)SceneParam::paramCurrentPlaySceneName, "0", "0", "0");
	AddParameter<std::string>((paramId_t)SceneParam::paramNextPlaySceneName, "0", "0", "0");
	AddParameter<int8_t>((paramId_t)SceneParam::paramSceneGBar, 1, 16, 16);
	AddParameter<int16_t>((paramId_t)SceneParam::paramSceneLength, 0, 200, 1);
	AddParameter<int8_t>((paramId_t)SceneParam::paramSceneMuteHold, 0, 2, 0);
	AddParameter<int8_t>((paramId_t)SceneParam::paramSceneFTSRootNote, (int8_t)FTSRootNote::NoteMin, (int8_t)FTSRootNote::NoteMax, 0);
	AddParameter<int8_t>((paramId_t)SceneParam::paramSceneFTSScale, (int8_t)ScaleList::ScaleMin, (int8_t)ScaleList::ScaleMax, 0);
	AddParameter<int8_t>((paramId_t)SceneParam::paramSceneTransposeValue, -12, 12, 0);
	AddParameter<int8_t>((paramId_t)SceneParam::paramSceneTransposeFTSPriority, 0, 1, 0);
	AddParameter<bool>((paramId_t)SceneParam::paramManualPlayNextSceneCmd, false, true, false);
	AddParameter<int32_t>((paramId_t)SceneParam::paramManualPlayNewSceneCmd, 0, 1000, 0);
	AddParameter<int8_t>((paramId_t)SceneParam::paramClearUndoList, 0, 127, 0);
	AddParameter<uint64_t>((paramId_t)SceneParam::paramSceneTrackMuteStatus, 0, 0xFFFFFFFFFFFFFFFF, 0);
	AddParameter<int8_t>((paramId_t)SceneParam::paramCurrentPlaySceneUnmutedTracks, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SceneParam::paramNextPlaySceneUnmutedTracks, 0, 127, 0);
	AddParameter<int16_t>((paramId_t)SceneParam::paramSceneBarId, 0, 200, 1);
	AddParameter<std::string>((paramId_t)SceneParam::paramSceneCopy, "0", "0", "0");
	AddParameter<int32_t>((paramId_t)SceneParam::paramSceneMove, 0, 1000, 0);
	AddParameter<int8_t>((paramId_t)SceneParam::paramSaveAll, 0, 1, 0);
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleScene, true);
}

void SceneManager::setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}

void SceneManager::SetDefaultUndoListData()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};

	for (auto i = (paramId_t)SceneParam::paramall + 1; i < (paramId_t)SceneParam::paramSceneMax; i++)
	{
		if ((i == (paramId_t)SceneParam::paramSceneNameList) ||
			(i == (paramId_t)SceneParam::paramUndoListCmd) ||
			(i == (paramId_t)SceneParam::paramSceneBarId) ||
			(i == (paramId_t)SceneParam::paramSceneCopy)
			)
		{
			//ignore command params, used in get param getParamList
		}
		else
		{
			paramlistdata.param = (paramId_t)i;
			paramlist.push_back(paramlistdata);
		}
	}

	retParam = getParamList((moduleId_t)ModuleName::moduleScene, paramlist);
	ClearUndoDefaultList();
	for (std::vector<IgniteParamList>::iterator it = retParam.begin(); it != retParam.end() && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList(it->param, it->MsgValue);
	}
}

void SceneManager::SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	//commands not added to Undo list, and no notification
	ParamHandler(moduleSrc, FindParameter(param), false);
}

void SceneManager::onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(SceneParam)param->GetParamID() != SceneParam::paramUndoCmd &&
		(SceneParam)param->GetParamID() != SceneParam::paramActiveScene &&
		(SceneParam)param->GetParamID() != SceneParam::paramNextPlaySceneId &&
		(SceneParam)param->GetParamID() != SceneParam::paramCurrentPlaySceneId &&
		(SceneParam)param->GetParamID() != SceneParam::paramAddSceneCmd &&
		(SceneParam)param->GetParamID() != SceneParam::paramSceneBarId
		)
	{
		AddToUndoList(param);
	}
	ParamHandler(moduleSrc, param, true);
}

void SceneManager::ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate && SongMgr->getActiveSong() != nullptr)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData();
	}

	if (moduleSrc != (moduleId_t)ModuleName::moduleScene)
	{
		switch ((SceneParam)param->GetParamID())
		{
			case SceneParam::paramAddSceneCmd:
				SceneManager::createAndAddScene();
				break;

			case SceneParam::paramAddTotalScenesCmd:
				SceneManager::addMultipleScenes(param->getValue().get<int32_t>());
				break;

			case SceneParam::paramDeleteSceneCmd:
				SceneManager::removeScene(param->getValue().get<int32_t>());
				break;

			case SceneParam::paramActiveScene:
				SceneManager::setSongEditScene(param->getValue().get<int32_t>());
				break;

			case SceneParam::paramSceneSwing:
				SceneManager::setSceneSwing(param->getValue().get<uint8_t>());
				break;

			case SceneParam::paramSceneTempo:
				SceneManager::setSceneTempo(param->getValue().get<float>());
				break;

			case SceneParam::paramSceneName:
				SceneManager::setSceneName((std::string)(param->getValue().get<std::string>().c_str()));
				break;

			case SceneParam::paramSaveAll:
                // Song Save All commits the complete song and clears scene undo only on success.
				break;
			case SceneParam::paramSaveSceneCmd:
				SaveData();
				break;

			case SceneParam::paramUndoCmd:
				doParamUndo();
				break;

			case SceneParam::paramStartScene:
				SceneManager::setStartScene(param->getValue().get<int32_t>());
				break;

			case SceneParam::paramEndScene:
				SceneManager::setEndScene(param->getValue().get<int32_t>());
				break;

			case SceneParam::paramSceneAdvanceMode:
				SceneManager::setSceneAdvanceMode(param->getValue().get<uint8_t>());
				break;

			case SceneParam::paramSceneGBar:
				SceneManager::setSceneGbar(param->getValue().get<int8_t>());
				break;

			case SceneParam::paramSceneLength:
				SceneManager::setSceneLength(param->getValue().get<int16_t>());
				break;

			case SceneParam::paramSceneFTSRootNote:
				SceneManager::setSceneFTSRootNote(param->getValue().get<int8_t>());
				break;

			case SceneParam::paramSceneFTSScale:
				SceneManager::setSceneFTSScale(param->getValue().get<int8_t>());
				break;

			case SceneParam::paramSceneTransposeValue:
				SceneManager::setSceneTranspose(param->getValue().get<int8_t>());
				break;

			case SceneParam::paramSceneTransposeFTSPriority:
				SceneManager::setSceneTransposeFTSPriority(param->getValue().get<int8_t>());
				break;

			case SceneParam::paramManualPlayNextSceneCmd:
				SceneManager::manualPlayNextScene();
				break;

			case SceneParam::paramManualPlayNewSceneCmd:
				SceneManager::manualPlayNewScene(param->getValue().get<int8_t>());
				break;

			case SceneParam::paramSceneCopy:
				SceneManager::copyScene((std::string)(param->getValue().get<std::string>().c_str()));
				break;

			case SceneParam::paramSceneMove:
				SceneManager::moveScene(param->getValue().get<int32_t>());
				break;

			case SceneParam::paramClearUndoList:
				if (param->getValue().get<int8_t>() == 0)
					ResetToDefaultData();
				ClearUndoList();
				break;

			case SceneParam::paramSceneMuteHold:
				SceneManager::setMuteHold(param->getValue().get<int8_t>());
				break;
		}

	}
	if (engine.getAutoSaveStatus())
	{
		SaveData();
	}
	if (SendNotification)
	{
		/* Send notification to the subscribed modules via engine */
		paramlistener->onParameterChanged(moduleSrc, param);
	}
}
void SceneManager::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if (SceneManager::saveScene())
    {
        markSaved();
    }

}
std::vector<IgniteParamList> SceneManager::getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData;
	//loop to fill all data and return the value
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		switch ((SceneParam)(*it).param)
		{
			case SceneParam::paramSceneName: //return active scene name
			{
				//read value from active scene
				std::string val = SceneManager::getSceneName();
				//fill return structure
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = val;
				retParam.push_back(tempParam);
				break;
			}
			case SceneParam::paramSceneCount:
			{
				int32_t val = SceneManager::getNumOfScenes();
				//fill return structure
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = val;
				retParam.push_back(tempParam);
				break;
			}
			case SceneParam::paramSceneSwing: //return active scene's swing value
			{
				//read value from active scene
				uint8_t val = SceneManager::getSceneSwing();
				//fill return structure
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = val;
				retParam.push_back(tempParam);
				break;
			}
			case SceneParam::paramSceneTempo: //return active scene's tempo value
			{
				float val = SceneManager::getSceneTempo();
				//fill return structure
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = val;
				retParam.push_back(tempParam);
				break;
			}
			case SceneParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList();
				paramData.param = (paramId_t)SceneParam::paramUndoListCmd;
				paramData.Count = (uint32_t)undoList.size(); //set the number of params
				retParam.push_back(paramData);
				for (auto itr = undoList.begin(); itr < undoList.end(); ++itr)
				{
					paramData.param = *itr;
					paramData.Count = Count;
					Count++;
					retParam.push_back(paramData);
				}
				break;
			}
			case SceneParam::paramSceneNameList://get the scene names from active song
				if (SongMgr->getActiveSong() != nullptr)
				{
					int32_t index = 0;
					struct IgniteParamList tempParam;
					for (auto nameIter = SongMgr->getActiveSong()->_songData->sceneList.begin();
						nameIter != SongMgr->getActiveSong()->_songData->sceneList.end(); ++nameIter)
					{
						tempParam.param = (*it).param;
						tempParam.MsgValue = (*nameIter).getName();
						tempParam.startPos = index;
						retParam.push_back(tempParam);
						++index;
					}
				}
				break;
			case SceneParam::paramSceneTrackMuteStatus://get the track mute status from active song's edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					//UI uses 1 as unmuted, flip the bits before sending
					tempParam.MsgValue = (SongMgr->getActiveSong()->editScene->getSceneTrackMuteStatus());
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case SceneParam::paramSceneGBar://get for current edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.MsgValue = SongMgr->getActiveSong()->editScene->getStepsPerBar();
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case SceneParam::paramSceneLength://get for current edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.MsgValue = SongMgr->getActiveSong()->editScene->getBarCount();
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case SceneParam::paramSceneAdvanceMode://get for current edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.MsgValue = SongMgr->getActiveSong()->editScene->getSceneAdvanceMode();
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case SceneParam::paramSceneFTSRootNote://get for current edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.MsgValue = SongMgr->getActiveSong()->editScene->getFTSRootNote();
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case SceneParam::paramSceneFTSScale://get for current edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.MsgValue = SongMgr->getActiveSong()->editScene->getFTSScale();
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case SceneParam::paramSceneTransposeValue://get for current edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.MsgValue = SongMgr->getActiveSong()->editScene->getTransposeValue();
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case SceneParam::paramSceneTransposeFTSPriority://get for current edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.MsgValue = SongMgr->getActiveSong()->editScene->getTransposeFTSPriority();
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case SceneParam::paramSceneMuteHold://get for current edit scene
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.MsgValue = (int8_t)SongMgr->getActiveSong()->editScene->getSceneMuteHold();
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			default:
			{
				break;
			}
		}
	}
	return retParam;
}

void SceneManager::getParam(moduleId_t moduleSrc, paramId_t param)
{
	switch ((SceneParam)param)
	{
		case SceneParam::paramSceneName:
		{
			std::string name = SceneManager::getSceneName();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, name);
			break;
		}
		case SceneParam::paramSceneSwing:
		{
			int8_t val = SceneManager::getSceneSwing();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneTempo:
		{
			float val = SceneManager::getSceneTempo();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneCount:
		{
			int32_t val = SceneManager::getNumOfScenes();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneGBar:
		{
			int8_t val = SceneManager::getSceneGbar();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneLength:
		{
			int16_t val = SceneManager::getSceneLength();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneFTSRootNote:
		{
			int8_t val = SceneManager::getSceneFTSRootNote();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneFTSScale:
		{
			int8_t val = SceneManager::getSceneFTSScale();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneTransposeValue:
		{
			int8_t val = SceneManager::getSceneTranspose();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneTransposeFTSPriority:
		{
			int8_t val = SceneManager::getSceneTransposeFTSPriority();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneAdvanceMode:
		{
			int8_t val = SceneManager::getSceneAdvanceMode();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		case SceneParam::paramSceneMuteHold:
		{
			int8_t val = SceneManager::getMuteHold();
			engine.SendResponse((moduleId_t)ModuleName::moduleScene, moduleSrc, param, val);
			break;
		}
		default:
			//TODO WARN
			break;
	}
}

void SceneManager::createAndAddScene(void)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		//TODO add warn
		return;
	}

	float tempo;
	if (SongMgr->getActiveSong()->getApplySongTempoStatus() == true)
	{
		tempo = SongMgr->getActiveSong()->_songData->songTempo;
	}
	else
	{
		tempo = DEFAULT_SCENE_TEMPO;
	}

	int8_t swing;
	if (SongMgr->getActiveSong()->getApplySongSwingStatus() == true)
	{
		swing = SongMgr->getActiveSong()->_songData->songSwing;
	}
	else
	{
		swing = DEFAULT_SCENE_SWING;
	}

	uint8_t numTracks = SongMgr->getActiveSong()->_songData->gTrackCount;
	uint64_t trackMute = 0xFFFFFFFFFFFFFFFF; //mute all tracks by default

	std::string sceneName;

	int32_t numScenes = SongMgr->getActiveSong()->getNumOfScenes();

	sceneName = SCENE_NAME_PREFIX + std::to_string(numScenes + 1);

	modifySceneNameIfDuplicate(SongMgr->getActiveSong()->_songData->sceneList, sceneName);

	Scene tempScene(numScenes, sceneName, tempo, swing, numTracks, trackMute, SongMgr);
	SongMgr->getActiveSong()->addScene(tempScene, false, false);
	saveScene(*(SongMgr->getActiveSong()), tempScene);
	SongMgr->saveSongInfo(*(SongMgr->getActiveSong()->_songData));
}
void SceneManager::modifySceneNameIfDuplicate(std::list<Scene>& sceneList, std::string& name)
{
	auto start = sceneList.begin();
	auto end = sceneList.end();
	int32_t count = 0;

	for (auto& sceneIter = start; sceneIter != end; ++sceneIter)
	{
		if ((*sceneIter).getName() == name)
		{
			name = name + "_new";
		}
		++count;
	}
}
void SceneManager::addMultipleScenes(int32_t val)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		return;
	}

	int32_t curNumOfScenes = SongMgr->getActiveSong()->getNumOfScenes();

	if (val > curNumOfScenes)
	{
		uint8_t newScenesToBeAdded = (uint8_t)(val - curNumOfScenes);

		for (int i = 0; i < newScenesToBeAdded; i++)
		{
			createAndAddScene();
		}
	}
}
void SceneManager::removeScene(int32_t sceneId)//id starts from 0
{
	bool deinitRequired = false;
	if (SongMgr->getActiveSong() == nullptr)
	{
		//TODO warn
		return;
	}

	//DON'T delete, If current play scene is selected for deletion
	if ((SongMgr->getActiveSong()->playScene != nullptr) && (SongMgr->getActiveSong()->playScene->getId() == sceneId)
		/* && (CMidiSync::getInstance().isPlaying() == true)*/)
	{
		deinitRequired = true;
	}

	auto sceneIterStart = SongMgr->getActiveSong()->_songData->sceneList.begin();
	auto sceneIterEnd = SongMgr->getActiveSong()->_songData->sceneList.end();

	for (auto& sceneIter = sceneIterStart; sceneIter != sceneIterEnd; ++sceneIter)
	{
		if ((*sceneIter).getId() == sceneId)
		{
			if (deinitRequired)
			{
				(*sceneIter).deInitScenePlaySettings();
				SongMgr->getActiveSong()->playScene = nullptr;
			}

			releaseSceneResources (sceneId, true);

			//delete in filesystem
			engine.get_fileHandler()->DeleteScene(SongMgr->getActiveSong()->getName(), sceneIter->getName());

			//delete in engine scene list
			//If current edit scene is selected for deletion
			if ((SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->getId() == sceneId))
			{
				SongMgr->getActiveSong()->editScene = nullptr;
				SongMgr->getActiveSong()->editSceneId = -1;
			}
			SongMgr->getActiveSong()->_songData->sceneList.erase(sceneIter);

			int32_t numScenes = SongMgr->getActiveSong()->getNumOfScenes();
			SongMgr->getActiveSong()->setNumOfScenes(numScenes - 1);

			//re-order the scene ids and save
			reAssignSequentialIdsForScenes(*SongMgr->getActiveSong());
			//saveSong(); -- TODO  - save only user save
			break;
		}
	}
}

void SceneManager::reAssignSequentialIdsForScenes(CSong& song)
{
	auto sceneStart = song._songData->sceneList.begin();
	auto sceneEnd = song._songData->sceneList.end();

	//Re-assign ids starting from 0
	int32_t count = 0;
	for (auto& sceneIter = sceneStart; sceneIter != sceneEnd; ++sceneIter)
	{
		if ((*sceneIter).getId() != count)
		{
			(*sceneIter).setId(count);
		}
		++count;
	}
}

void SceneManager::reAssignSequentialIdsForScenesAndSave(CSong& song)
{
	auto sceneStart = song._songData->sceneList.begin();
	auto sceneEnd = song._songData->sceneList.end();

	//Re-assign ids starting from 0
	int32_t count = 0;
	for (auto& sceneIter = sceneStart; sceneIter != sceneEnd; ++sceneIter)
	{
		if ((*sceneIter).getId() != count)
		{
			(*sceneIter).setId(count);
		}

		saveSceneInfo(*(sceneIter->_sceneData));

		++count;
	}
}

void SceneManager::setSongEditScene(int32_t sceneId)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		//TODO warn
		return;
	}

	if (sceneId >= SongMgr->getActiveSong()->getNumOfScenes())
		//TODO log warn message
		return;

	if (SongMgr->getActiveSong()->getEditSceneId() != sceneId)
	{
		SongMgr->getActiveSong()->setEditScene(sceneId);
		isUndilistNeedUpdate = true;
		ClearUndoList();
	}
}
void SceneManager::setSceneSwing(uint8_t val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->editScene->setSwing(val);
}
void SceneManager::setSceneTempo(float val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}

	SongMgr->getActiveSong()->editScene->setTempo(val);
	//Change playback tempo, if Scene is currently playing
	if (SongMgr->getActiveSong()->editScene == SongMgr->getActiveSong()->playScene)
	{
		CMidiClock::getInstance().setTempo(SongMgr->getActiveSong()->playScene->getTempo());
		SongMgr->sendMidiClk();
	}
}
void SceneManager::setSceneName(std::string name)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}

	//File system update
	engine.get_fileHandler()->RenameScene(SongMgr->getActiveSong()->getName(), SongMgr->getActiveSong()->editScene->getName(), name);

	SongMgr->getActiveSong()->editScene->setName(name);
}
bool SceneManager::saveScene(CSong& song, Scene& scene)
{
    return engine.get_fileHandler()->CreateScene(song.getName(), *scene._sceneData) == FILE_OK;
}
void SceneManager::setStartScene(int32_t index)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		//TODO warn
		return;
	}

	SongMgr->getActiveSong()->setStartSceneId(index);
}

bool SceneManager::saveScene(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return false;
	}
	auto activeSong = SongMgr->getActiveSong ();
	auto editScene = SongMgr->getActiveSong ()->editScene;

    if (engine.get_fileHandler()->CreateScene(activeSong->getName(), *editScene->_sceneData) != FILE_OK) return false;
    const auto fileSystemSceneName = GetDefaultDataFromList((paramId_t)SceneParam::paramSceneName).get<std::string>();
    if (fileSystemSceneName != editScene->getName())
        engine.get_fileHandler()->DeleteScene(activeSong->getName(), fileSystemSceneName);


	std::list <Pattern*> PatternList;
	std::list <Instrument*> InstrumentList;

	auto& trackList = editScene->_sceneData->trackList;
	for (auto& track : trackList)
	{
		PatternList.push_back(track.getTrackMainPattern());
		PatternList.push_back(track.getTrackFillPattern());
		InstrumentList.push_back(track.getTrackRxInstrument());
		InstrumentList.push_back(track.getTrackTxInstrument());
	}
	PatternList.unique();
	InstrumentList.unique();

	for (auto it : PatternList)
	{
		if (it != NULL)
			it->savePatternUsedByList();
	}
	for (auto it : InstrumentList)
	{
		if (it != NULL)
			it->saveInstrumentUsedByList();
	}
    return true;
}

void SceneManager::saveSceneInfo(SceneData& sceneData)
{
	engine.get_fileHandler()->UpdateSceneInfo(SongMgr->getActiveSong()->getName(), sceneData);
}

void SceneManager::setEndScene(int32_t index)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->setEndSceneId(index);
}
void SceneManager::setSceneAdvanceMode(uint8_t val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->editScene->setSceneAdvanceMode(val);
}
void SceneManager::setSceneGbar(int8_t val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->editScene->setStepsPerBar(val);
}
void SceneManager::setSceneLength(int16_t val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->editScene->setBarCount(val);
}
void SceneManager::setSceneFTSRootNote(int8_t index)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->editScene->setFTSRootNote(index);
}
void SceneManager::setSceneFTSScale(int8_t index)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->editScene->setFTSScale(index);
}
void SceneManager::setSceneTranspose(int8_t val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}

	SongMgr->getActiveSong()->editScene->setTransposeValue(val);
}
void SceneManager::setSceneTransposeFTSPriority(int8_t val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->editScene->setTransposeFTSPriority(val);
}
void SceneManager::manualPlayNextScene(void)
{
	//Manual Scene change - last manual action takes effect - not both(Next and Jump)
	if (SongMgr->manualJumpToNewScene == true)
	{
		SongMgr->manualJumpToNewScene = false;
		SongMgr->getActiveSong()->jumpSceneId = -1;
	}
	SongMgr->manualSwitchToNextScene = true;
}
void SceneManager::manualPlayNewScene(int32_t sceneId)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		return;
	}
	if ((sceneId < 0) || (sceneId >= SongMgr->getActiveSong()->getNumOfScenes()))
	{
		return;
	}

	SongMgr->getActiveSong()->jumpSceneId = sceneId;

	//Manual Scene change - last manual action takes effect - not both(Next and Jump)
	if (SongMgr->manualSwitchToNextScene == true)
	{
		SongMgr->manualSwitchToNextScene = false;
	}
	SongMgr->manualJumpToNewScene = true;
}
std::string SceneManager::getSceneName(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return "invalid";
	}
	return SongMgr->getActiveSong()->editScene->getName();
}
int8_t SceneManager::getSceneSwing()
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return -1;
	}

	return SongMgr->getActiveSong()->editScene->getSwing();
}
float SceneManager::getSceneTempo()
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return -1;
	}

	return SongMgr->getActiveSong()->editScene->getTempo();
}
int32_t SceneManager::getNumOfScenes(void)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		return -1;
	}

	return SongMgr->getActiveSong()->getNumOfScenes();
}
int8_t SceneManager::getSceneGbar(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return -1;
	}
	return SongMgr->getActiveSong()->editScene->getStepsPerBar();
}
int16_t SceneManager::getSceneLength(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return -1;
	}
	return SongMgr->getActiveSong()->editScene->getBarCount();
}
int8_t SceneManager::getSceneFTSRootNote(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return -1;
	}
	return SongMgr->getActiveSong()->editScene->getFTSRootNote();
}
int8_t SceneManager::getSceneFTSScale(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return -1;
	}
	return SongMgr->getActiveSong()->editScene->getFTSScale();
}
int8_t SceneManager::getSceneTranspose(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return 0;
	}
	return SongMgr->getActiveSong()->editScene->getTransposeValue();
}
int8_t SceneManager::getSceneTransposeFTSPriority(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return -1;
	}
	return SongMgr->getActiveSong()->editScene->getTransposeFTSPriority();
}
int8_t SceneManager::getSceneAdvanceMode(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return -1;
	}
	return SongMgr->getActiveSong()->editScene->getSceneAdvanceMode();
}

int8_t SceneManager::getMuteHold(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return -1;
	}
	return (int8_t)SongMgr->getActiveSong()->editScene->getSceneMuteHold();
}

bool SceneManager::copyScene(std::string sceneName)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		//TODO add warn
		return false;
	}

	Scene tmpScene(*(SongMgr->getActiveSong()->getEditScene()));
	tmpScene.setId((int32_t)SongMgr->getActiveSong()->_songData->sceneList.size());
	modifySceneNameIfDuplicate(SongMgr->getActiveSong()->_songData->sceneList, sceneName);
	tmpScene.setName(sceneName);
	SongMgr->getActiveSong()->addScene(tmpScene, false, false);
	SongMgr->getActiveSong()->setNumOfScenes((int32_t)SongMgr->getActiveSong()->_songData->sceneList.size());
	saveScene(*(SongMgr->getActiveSong()), tmpScene);

	return true;
}

void SceneManager::moveScene(int32_t newId)
{
	if ((SongMgr == nullptr) || (SongMgr->getActiveSong() == nullptr))
	{
		//TODO WARN
		return;
	}

	auto curId = SongMgr->getActiveSong()->getEditSceneId();

	for (auto& scene : SongMgr->getActiveSong()->_songData->sceneList)
	{
		// If movement is +/- 1 position, LIST sort does not work as required.
		// So in this case which is like swapping, also update the id of the scene,
		// which is swapping its position with current scene.
		// For other cases when movement is greater than +/- 1 positions,
		// only update the moving scene's id, to new id.

		if ((std::abs(curId - newId) == 1) && (scene.getId() == newId))
		{
			//assign cur scene id to the other (adjacent) scene
			scene.setId(curId);
		}
		else if ((newId < curId) && (scene.getId() == newId))
		{
			scene.setId(newId + 1);
		}
		else if (scene.getId() == curId)
		{
			scene.setId(newId);
		}
	}

	SongMgr->sortScenesInSong(*(SongMgr->activeSong));
	reAssignSequentialIdsForScenesAndSave(*(SongMgr->activeSong));
}

void SceneManager::setMuteHold(int8_t val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		return;
	}
	SongMgr->getActiveSong()->editScene->setSceneMuteHold(static_cast <MuteHoldType> (val));
}
void SceneManager::releaseSceneResources (int32_t sceneId, bool isDelete)
{
	std::string songName = SongMgr->getActiveSong ()->getName();
	Scene* scene = SongMgr->getSceneWithId (sceneId);
	std::string sceneName = scene->getName ();
	auto& trackList = scene->_sceneData->trackList;

#ifdef ENABLE_PARALLEL_EXE
	std::for_each (
		std::execution::par_unseq,
		trackList.begin (),
		trackList.end (),
		[this, songName, sceneName, isDelete](auto&& track)
		{
			track.ResetUsedbyInfo (songName, sceneName, isDelete);
		});
#else
	for (auto& track : trackList)
	{
		track.ResetUsedbyInfo (songName, sceneName, isDelete);
	}
#endif
	engine.trackManager->saveTrack ();
}

std::string SceneManager::getSceneNameFromSceneId (int32_t id)
{
	auto& sceneList = SongMgr->getActiveSong ()->_songData->sceneList;

	for (auto& scene : sceneList)
	{
		if (scene.getId () == id)
		{
			return scene.getName();
		}
	}
	return nullptr;
}
