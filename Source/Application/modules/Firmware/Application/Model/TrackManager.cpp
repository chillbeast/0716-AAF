#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include "Firmware/Application/Model/SongManager.h"
#include "Firmware/Application/Model/TrackManager.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Common/Modules/Track.h"

using namespace Ignite;
class Track;
TrackManager::TrackManager(IParamChangeListener* listener, IgniteEngine& model, SongManager* songMgr) : paramlistener(listener), engine(model), SongMgr(songMgr), Module("TrackManager", (moduleId_t)ModuleName::moduleTrack, (unsigned int)SceneParam::paramSceneMax, listener)
{
	AddParameter<uint8_t>((paramId_t)TrackParam::paramActiveTrack, 0, 127, 0);
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackInstrument, "0", "0", "0");
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackPattern, "0", "0", "0");
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackFillPattern, "0", "0", "0");
	AddParameter<uint8_t>((paramId_t)TrackParam::paramTrackMute, 0, 127, 0);
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackName, "0", "0", "0");
	AddParameter<uint8_t>((paramId_t)TrackParam::paramUndoCmd, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)TrackParam::paramSaveTrackCmd, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)TrackParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)TrackParam::paramClearUndoList, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)TrackParam::paramTrackSwapCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)TrackParam::paramTrackPatternBarId, 0, 16, 0);
	AddParameter<uint16_t>((paramId_t)TrackParam::paramTrackPatternBarAndStepId, 0, 0xFFFF, 0);
	AddParameter<bool>((paramId_t)TrackParam::paramTrackSolo, 0, 1, 0);
	AddParameter<bool>((paramId_t)TrackParam::paramTrackRecord, 0, 1, 0);
	AddParameter<uint8_t> ((paramId_t)TrackParam::paramTrackPatternStepId, 0, 255, 0);
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackPatternDispName, "0", "0", "0");
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackFillPatternDispName, "0", "0", "0");
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackInstrumentDispName, "0", "0", "0");
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackOutInstrument, "0", "0", "0");
	AddParameter<std::string>((paramId_t)TrackParam::paramTrackOutInstrumentDispName, "0", "0", "0");
	AddParameter<int8_t>((paramId_t)TrackParam::paramSaveAll, 0, 1, 0);
	AddParameter<float>((paramId_t)TrackParam::paramCvVoltsPort1, 0.0, 10.0, 0.0);// not used directly using Hardware cvControll code remove after all testing
	AddParameter<float>((paramId_t)TrackParam::paramCvVoltsPort2, 0.0, 10.0, 0.0);// not used directly using Hardware cvControll code remove after all testing
	AddParameter<float>((paramId_t)TrackParam::paramCvVoltsPort3, 0.0, 10.0, 0.0);// not used directly using Hardware cvControll code remove after all testing
	AddParameter<float>((paramId_t)TrackParam::paramCvVoltsPort4, 0.0, 10.0, 0.0);// not used directly using Hardware cvControll code remove after all testing
	AddParameter<float>((paramId_t)TrackParam::paramCvVoltsPort5, 0.0, 10.0, 0.0);// not used directly using Hardware cvControll code remove after all testing
	AddParameter<float>((paramId_t)TrackParam::paramCvVoltsPort6, 0.0, 10.0, 0.0);// not used directly using Hardware cvControll code remove after all testing
	AddParameter<float>((paramId_t)TrackParam::paramCvVoltsPort7, 0.0, 10.0, 0.0);// not used directly using Hardware cvControll code remove after all testing
	AddParameter<float>((paramId_t)TrackParam::paramCvVoltsPort8, 0.0, 10.0, 0.0);// not used directly using Hardware cvControll code remove after all testing
	AddParameter<uint8_t>((paramId_t)TrackParam::paramCvGateState, 0, 255, 0);// not used directly using Hardware cvControll code remove after all testing
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleTrack, true);
}
void TrackManager::SetDefaultUndoListData()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};

	for (auto i = (paramId_t)TrackParam::paramall + 1; i < (paramId_t)TrackParam::paramTrackMax; i++)
	{
		if ((i == (paramId_t)TrackParam::paramTrackInfoList) ||
			(i == (paramId_t)TrackParam::paramCvVoltsPort1) ||
			(i == (paramId_t)TrackParam::paramCvVoltsPort2) ||
			(i == (paramId_t)TrackParam::paramCvVoltsPort3) ||
			(i == (paramId_t)TrackParam::paramCvVoltsPort4) ||
			(i == (paramId_t)TrackParam::paramCvVoltsPort5) ||
			(i == (paramId_t)TrackParam::paramCvVoltsPort6) ||
			(i == (paramId_t)TrackParam::paramCvVoltsPort7) ||
			(i == (paramId_t)TrackParam::paramCvVoltsPort8) ||
			(i == (paramId_t)TrackParam::paramCvGateState) ||
			(i == (paramId_t)TrackParam::paramUndoListCmd)
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

	retParam = getParamList((moduleId_t)ModuleName::moduleTrack, paramlist);
	ClearUndoDefaultList();
	for (std::vector<IgniteParamList>::iterator it = retParam.begin(); it != retParam.end() && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList(it->param, it->MsgValue);
	}
}
void TrackManager::setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
	}
}

void TrackManager::SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	//commands not added to Undo list, and no notification
	ParamHandler(moduleSrc, FindParameter(param), false);
}
void TrackManager::onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(TrackParam)param->GetParamID() != TrackParam::paramUndoCmd &&
		(TrackParam)param->GetParamID() != TrackParam::paramSaveTrackCmd &&
		(TrackParam)param->GetParamID() != TrackParam::paramSaveAll &&
		(TrackParam)param->GetParamID() != TrackParam::paramUndoListCmd &&
		(TrackParam)param->GetParamID() != TrackParam::paramTrackInfoList &&
		(TrackParam)param->GetParamID() != TrackParam::paramClearUndoList &&
		(TrackParam)param->GetParamID() != TrackParam::paramActiveTrack &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvVoltsPort1 &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvVoltsPort2 &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvVoltsPort3 &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvVoltsPort4 &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvVoltsPort5 &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvVoltsPort6 &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvVoltsPort7 &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvVoltsPort8 &&
		(TrackParam)param->GetParamID() != TrackParam::paramCvGateState &&
		(TrackParam)param->GetParamID() != TrackParam::paramTrackSwapCmd)
	{
		AddToUndoList(param);
	}
	ParamHandler(moduleSrc, param, true);
}
void TrackManager::ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate && SongMgr->getActiveSong() != nullptr)
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData();
	}
	if (moduleSrc != (moduleId_t)ModuleName::moduleTrack)
	{
		switch ((TrackParam)param->GetParamID())
		{

			case TrackParam::paramActiveTrack:
				TrackManager::setSongEditTrack(param->getValue().get<uint8_t>());
				break;
			case TrackParam::paramTrackInstrument:
				TrackManager::setTrackRxInstrument((std::string)(param->getValue().get<std::string>().c_str()));
				break;
			case TrackParam::paramTrackOutInstrument:
				TrackManager::setTrackTxInstrument((std::string)(param->getValue().get<std::string>().c_str()));
				break;
			case TrackParam::paramTrackPattern:
				TrackManager::setTrackMainPattern((std::string)(param->getValue().get<std::string>().c_str()));
				break;
			case TrackParam::paramTrackMute:
				TrackManager::setSongTrackMuteStatus(param->getValue().get<uint8_t>());
				break;
			case TrackParam::paramTrackName:
				TrackManager::setSceneTrackName((std::string)(param->getValue().get<std::string>().c_str()));
				break;
			case TrackParam::paramUndoCmd:
				doParamUndo();
				break;
			case TrackParam::paramSaveAll:
				ClearUndoList();//Save Song will save all tracks
				break;
			
			case TrackParam::paramSaveTrackCmd:
				SaveData();
				break;
			case TrackParam::paramTrackFillPattern:
				TrackManager::setTrackFillPattern((std::string)(param->getValue().get<std::string>().c_str()));
				break;
			case TrackParam::paramClearUndoList:
				if (param->getValue().get<int8_t>() == 0)
					ResetToDefaultData();
				ClearUndoList();
				break;

			case TrackParam::paramTrackSwapCmd:
				TrackManager::swapTrack(param->getValue().get<int8_t>());
				break;

			case TrackParam::paramTrackSolo:
				TrackManager::setSoloStateOnEditTrack(param->getValue().get<int8_t>());
				break;

			case TrackParam::paramTrackRecord:
				TrackManager::setRecordStateOnEditTrack(param->getValue().get<int8_t>());
				break;

			default:
				assert(0);
				break;
		}
	}
	if (engine.getAutoSaveStatus() || engine.settingsTrackCnfg->_SettingsTrackCnfgData.TrackAutoSaveParam)
	{
		SaveData();
	}
}
void TrackManager::SaveData()
{
	TrackManager::saveTrack();
	ClearUndoList();
	isUndilistNeedUpdate = true;
}

void TrackManager::swapTrack(int8_t otherTrackId)
{
	// TODO check if cirklon permits it during play
	auto startIter = SongMgr->activeSong->editScene->_sceneData->trackList.begin();
	auto endIter = SongMgr->activeSong->editScene->_sceneData->trackList.end();

	auto& trackIter = startIter;
	for (; trackIter != endIter; trackIter++)
	{
		if (trackIter->getTrackId() == otherTrackId)
		{
			auto temp = SongMgr->activeSong->editScene->editTrack->_trackData;
			SongMgr->activeSong->editScene->editTrack->_trackData = trackIter->_trackData;
			trackIter->_trackData = temp;
			SongMgr->activeSong->reAssignSequentialIdsForTracks(*(SongMgr->activeSong->editScene));
			break;
		}
	}
}

std::vector<IgniteParamList> TrackManager::getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData;
	//loop to fill all data and return the value
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		switch ((TrackParam)(*it).param)
		{

			case TrackParam::paramTrackInfoList://get the (1)track_id, (2)track_instrument, (3)track_pattern, (4)track_fill_pattern & (5)track_solo_state, for edit track
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr))
				{
					struct IgniteParamList tempParam;
					for (auto trackIter = SongMgr->getActiveSong()->editScene->_sceneData->trackList.begin(); 
						trackIter != SongMgr->getActiveSong()->editScene->_sceneData->trackList.end(); ++trackIter)
					{
						tempParam.param = (*it).param;
						tempParam.MsgValue = (*trackIter).getTrackName();
						tempParam.startPos = (*trackIter)._trackData->trackId;
						retParam.push_back(tempParam);

						tempParam.param = (paramId_t)TrackParam::paramTrackInstrumentDispName;

						if ((*trackIter)._trackData->trackRxInstrumentName.empty() || (*trackIter)._trackData->trackRxInstrumentName == " ")
							tempParam.MsgValue = " ";
						else
							tempParam.MsgValue = (*trackIter)._trackData->trackRxInstrument->getDispName();

						tempParam.startPos = (*trackIter)._trackData->trackId;
						retParam.push_back (tempParam);

						tempParam.param = (paramId_t)TrackParam::paramTrackOutInstrumentDispName;

						if ((*trackIter)._trackData->trackTxInstrumentName.empty() || (*trackIter)._trackData->trackTxInstrumentName == " " || !(*trackIter)._trackData->trackTxInstrument)
							tempParam.MsgValue = " ";
						else
							tempParam.MsgValue = (*trackIter)._trackData->trackTxInstrument->getDispName();

						tempParam.startPos = (*trackIter)._trackData->trackId;
						retParam.push_back(tempParam);

						tempParam.param = (paramId_t)TrackParam::paramTrackPatternDispName;

						if ((*trackIter)._trackData->mainPatternName.empty() || (*trackIter)._trackData->mainPatternName == " " || !(*trackIter)._trackData->mainPattern)
							tempParam.MsgValue = " ";
						else
							tempParam.MsgValue = (*trackIter)._trackData->mainPattern->PatternGetDispName();

						tempParam.startPos = (*trackIter)._trackData->trackId;
						retParam.push_back (tempParam);

						tempParam.param = (paramId_t)TrackParam::paramTrackFillPatternDispName;

						if ((*trackIter)._trackData->fillPatternName.empty() || (*trackIter)._trackData->fillPatternName == " " || !(*trackIter)._trackData->fillPattern)
							tempParam.MsgValue = " ";
						else
							tempParam.MsgValue = (*trackIter)._trackData->fillPattern->PatternGetDispName();

						tempParam.startPos = (*trackIter)._trackData->trackId;
						retParam.push_back (tempParam);

						tempParam.param = (paramId_t)TrackParam::paramTrackSolo;
						tempParam.MsgValue = (*trackIter)._trackData->trackSolo;
						tempParam.startPos = (*trackIter)._trackData->trackId;
						retParam.push_back (tempParam);

						tempParam.param = (paramId_t)TrackParam::paramTrackRecord;
						tempParam.MsgValue = (*trackIter)._trackData->trackRecordEnable;
						tempParam.startPos = (*trackIter)._trackData->trackId;
						retParam.push_back(tempParam);

						tempParam.param = (paramId_t)TrackParam::paramTrackMute;
						tempParam.MsgValue = (*trackIter)._trackData->trackMute;
						tempParam.startPos = (*trackIter)._trackData->trackId;
						retParam.push_back(tempParam);
					}
				}
				break;
			case TrackParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList();
				paramData.param = (paramId_t)TrackParam::paramUndoListCmd;
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
			case TrackParam::paramTrackInstrument:
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					tempParam.MsgValue = (std::string)(SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackRxInstrumentName.empty() ?
						" " : SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackRxInstrumentName);
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case TrackParam::paramTrackInstrumentDispName:
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					if (SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackRxInstrumentName.empty() || SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackRxInstrumentName == " ")
						tempParam.MsgValue = std::string(" ");
					else
						tempParam.MsgValue = (std::string)SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackRxInstrument->getDispName();

					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case TrackParam::paramTrackOutInstrument:
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					tempParam.MsgValue = (std::string)(SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackTxInstrumentName.empty() ?
						" " : SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackTxInstrumentName);
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case TrackParam::paramTrackOutInstrumentDispName:
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					if (SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackTxInstrumentName.empty() || SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackTxInstrumentName == " ")
						tempParam.MsgValue = std::string(" ");
					else
						tempParam.MsgValue = (std::string)SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackTxInstrument->getDispName();

					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;
			case TrackParam::paramTrackPattern:
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					tempParam.MsgValue = (std::string)(SongMgr->getActiveSong()->editScene->editTrack->_trackData->mainPatternName.empty() ? " " :
						SongMgr->getActiveSong()->editScene->editTrack->_trackData->mainPatternName);
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;

			case TrackParam::paramTrackFillPattern:
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					tempParam.MsgValue = (std::string)(SongMgr->getActiveSong()->editScene->editTrack->_trackData->fillPatternName.empty() ?
						" " : SongMgr->getActiveSong()->editScene->editTrack->_trackData->fillPatternName);
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;

			case TrackParam::paramTrackPatternDispName:
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					if (SongMgr->getActiveSong()->editScene->editTrack->_trackData->mainPatternName.empty() || SongMgr->getActiveSong()->editScene->editTrack->_trackData->mainPatternName == " ")
						tempParam.MsgValue = std::string(" ");
					else
						tempParam.MsgValue = (std::string)SongMgr->getActiveSong()->editScene->editTrack->_trackData->mainPattern->PatternGetDispName();

					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;

			case TrackParam::paramTrackFillPatternDispName:
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					if (SongMgr->getActiveSong()->editScene->editTrack->_trackData->fillPatternName.empty() || SongMgr->getActiveSong()->editScene->editTrack->_trackData->fillPatternName == " ")
						tempParam.MsgValue = std::string(" ");
					else
						tempParam.MsgValue = (std::string)SongMgr->getActiveSong()->editScene->editTrack->_trackData->fillPattern->PatternGetDispName();

					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
				break;

			case TrackParam::paramTrackSolo:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getSoloStateOnEditTrack();
				tempParam.startPos = 0;//not used
				retParam.push_back(tempParam);
			}
			break;
			case TrackParam::paramTrackMute:
			{
				if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					tempParam.MsgValue = SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackMute;
					tempParam.startPos = 0;//not used
					retParam.push_back(tempParam);
				}
			}
			break;
			case TrackParam::paramTrackRecord:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getRecordStateOnEditTrack();
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
void TrackManager::getParam(moduleId_t moduleSrc, paramId_t param)
{
	switch ((TrackParam)param)
	{
		case TrackParam::paramTrackInstrument:
			if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
			{
				std::string name = SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackRxInstrumentName;
				engine.SendResponse((moduleId_t)ModuleName::moduleTrack, moduleSrc, param, name);
			}
			break;
		case TrackParam::paramTrackOutInstrument:
			if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
			{
				std::string name = SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackTxInstrumentName;
				engine.SendResponse((moduleId_t)ModuleName::moduleTrack, moduleSrc, param, name);
			}
			break;
		case TrackParam::paramTrackPattern:
			if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
			{
				std::string name = SongMgr->getActiveSong()->editScene->editTrack->_trackData->mainPatternName;
				engine.SendResponse((moduleId_t)ModuleName::moduleTrack, moduleSrc, param, name);
			}
			break;
		case TrackParam::paramTrackFillPattern:
			if ((SongMgr->getActiveSong() != nullptr) && (SongMgr->getActiveSong()->editScene != nullptr) && (SongMgr->getActiveSong()->editScene->editTrack != nullptr))
			{
				std::string name = SongMgr->getActiveSong()->editScene->editTrack->_trackData->fillPatternName;
				engine.SendResponse((moduleId_t)ModuleName::moduleTrack, moduleSrc, param, name);
			}
			break;
		default:
			//TODO WARN
			break;
	}
}
void TrackManager::setSongEditTrack(uint8_t trackId)
{
	if (SongMgr->getActiveSong() == nullptr)
	{
		//TODO warn
		return;
	}
	if (trackId >= SongMgr->getActiveSong()->editScene->getNumOfTracks())
	{
		//TODO log warn message
		return;
	}
	int8_t TrackId = SongMgr->getActiveSong()->editScene->getEditTrackId();
	if ((TrackId != -1) && (TrackId != trackId))
	{
		SongMgr->getActiveSong()->editScene->setEditTrack(trackId);
		SongMgr->getActiveSong ()->editScene->editTrack->SaveDfaultUsedbyInfo (false);
		isUndilistNeedUpdate = true;
		ClearUndoList();
	}
	else if (TrackId == -1)
	{
		SongMgr->getActiveSong()->editScene->setEditTrack(trackId);
		isUndilistNeedUpdate = true;
	}
}


void TrackManager::setTrackRxInstrument(std::string name)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr) ||
		(SongMgr->getActiveSong()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return;
	}
	Instrument* CurrentInstrument = SongMgr->getActiveSong()->editScene->editTrack->getTrackRxInstrument();
	if (CurrentInstrument != nullptr)
	{
		//TODO - un-necessary set  will happen, if user changes the name and calls this function
		//switch to sending id from UI
		if (CurrentInstrument->getName () == name)
		{
			return;
		}
		CurrentInstrument->RemoveFromUsedByList(false, SongMgr->getActiveSong()->getName(), SongMgr->getActiveSong()->editScene->getName(),
			SongMgr->getActiveSong()->editScene->editTrack->getTrackName());
	}

	Instrument* instru = _PInstrumentMgr->getInstrument(name);
	if (instru != nullptr)
	{
		SongMgr->getActiveSong()->editScene->editTrack->setTrackRxInstrument(instru);
		instru->UpdateUsedByList (false, SongMgr->getActiveSong()->getName(), SongMgr->getActiveSong()->editScene->getName(),
			SongMgr->getActiveSong()->editScene->editTrack->getTrackName());
	}
	else
	{
		SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackRxInstrumentName = " ";
		SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackRxInstrument = nullptr;
	}
	//TODO: remove below not required for IN instrument
	////clean track values saved, from previous instrument
	//SongMgr->getActiveSong ()->editScene->editTrack->deleteAllTvTrackData ();

	//if (instru != nullptr)
	//{
	//	SongMgr->getActiveSong ()->editScene->editTrack->setTvDataFromInstrument (instru);
	//}
}

void TrackManager::setTrackTxInstrument(std::string name)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr) ||
		(SongMgr->getActiveSong()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return;
	}
	Instrument* CurrentInstrument = SongMgr->getActiveSong()->editScene->editTrack->getTrackTxInstrument();
	if (CurrentInstrument != nullptr)
	{
		//TODO - un-necessary set  will happen, if user changes the name and calls this function
		//switch to sending id from UI
		if (CurrentInstrument->getName() == name)
		{
			return;
		}
		CurrentInstrument->RemoveFromUsedByList(false, SongMgr->getActiveSong()->getName(), SongMgr->getActiveSong()->editScene->getName(),
			SongMgr->getActiveSong()->editScene->editTrack->getTrackName());
	}

	Instrument* instru = _PInstrumentMgr->getInstrument(name);
	if (instru != nullptr)
	{
		SongMgr->getActiveSong()->editScene->editTrack->setTrackTxInstrument(instru);
		instru->UpdateUsedByList(false, SongMgr->getActiveSong()->getName(), SongMgr->getActiveSong()->editScene->getName(),
			SongMgr->getActiveSong()->editScene->editTrack->getTrackName());
	}
	else
	{
		SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackTxInstrumentName = " ";
		SongMgr->getActiveSong()->editScene->editTrack->_trackData->trackTxInstrument = nullptr;
	}

	//clean track values saved, from previous instrument
	SongMgr->getActiveSong()->editScene->editTrack->deleteAllTvTrackData();

	if (instru != nullptr)
	{
		SongMgr->getActiveSong()->editScene->editTrack->setTvDataFromInstrument(instru);
	}
}

void TrackManager::setTrackMainPattern(std::string name)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr)
		|| (SongMgr->getActiveSong()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return;
	}
	Pattern* Currentpattern = SongMgr->getActiveSong()->editScene->editTrack->getTrackMainPattern ();
	if (Currentpattern != nullptr)
	{
		Currentpattern->RemoveFromUsedByList(false, SongMgr->getActiveSong()->getName(),
			SongMgr->getActiveSong()->editScene->getName(), SongMgr->getActiveSong()->editScene->editTrack->getTrackName());
	}
	Pattern* pattern = _PPatternMgr->GetPattern(name);
	if (pattern != nullptr)
	{
		SongMgr->getActiveSong()->editScene->editTrack->setTrackMainPattern(pattern);
		pattern->UpdateUsedByList(false, SongMgr->getActiveSong()->getName(),
			SongMgr->getActiveSong()->editScene->getName(), SongMgr->getActiveSong()->editScene->editTrack->getTrackName());

		engine.sequenceManager->updateTrackPattern(pattern);
	}
	else
	{
		SongMgr->getActiveSong()->editScene->editTrack->setTrackMainPattern(nullptr);
		SongMgr->getActiveSong()->editScene->editTrack->_trackData->mainPatternName = " ";
	}
}
void TrackManager::setSongTrackMuteStatus(uint8_t val)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr)
		|| (SongMgr->getActiveSong()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return;
	}

	SongMgr->getActiveSong()->editScene->editTrack->setTrackMuteStatus(val);

	//update scene's overall track mute bit field
	uint64_t tempTrackMuteStatus = SongMgr->getActiveSong()->editScene->getSceneTrackMuteStatus();
	int8_t tempTrackId = SongMgr->getActiveSong()->editScene->editTrack->getTrackId();
	//clear and set
	tempTrackMuteStatus &= ~((uint64_t)1 << tempTrackId);
	tempTrackMuteStatus |= (((val == 0 ? (uint64_t)0 : (uint64_t)1)) << tempTrackId);
	SongMgr->getActiveSong()->editScene->setSceneTrackMuteStatus(tempTrackMuteStatus);
}
void TrackManager::setSceneTrackName(std::string name)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr) ||
		(SongMgr->getActiveSong()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return;
	}

	//uint8_t sceneId = activeSong->getEditSceneId();
	//uint8_t trackId = activeSong->sceneList[sceneId].getEditTrackId();

	SongMgr->getActiveSong()->editScene->editTrack->setTrackName(name);
}
void TrackManager::saveTrack()
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr))
	{
		//TODO warn
		return;
	}


	//filesystem save, all track Saved in a single file to save boot time and save memory in HW
	// CreateTracksWithList atomically replaces the previous track file.

	std::vector<TrackData> TrackDataList;
	for (auto it = SongMgr->getActiveSong ()->editScene->_sceneData->trackList.begin (); it != SongMgr->getActiveSong ()->editScene->_sceneData->trackList.end (); ++it)
	{
		if (it->_trackData != nullptr)
		{
			TrackDataList.push_back (*(it->_trackData));
		}
		else
		{
			break;
		}
	}
	engine.get_fileHandler ()->CreateTracksWithList (SongMgr->getActiveSong ()->getName (), SongMgr->getActiveSong ()->editScene->getName (), TrackDataList);

	//If this track is using a pattern,
	//save the pattern's used-by-list, to include this track
	if (SongMgr->getActiveSong ()->editScene->editTrack != nullptr)
	{
		auto trackMainPattern = SongMgr->getActiveSong ()->editScene->editTrack->getTrackMainPattern ();
		if (trackMainPattern != nullptr)
		{
			trackMainPattern->savePatternUsedByList ();
		}

		auto trackFillPattern = SongMgr->getActiveSong ()->editScene->editTrack->getTrackFillPattern ();
		if (trackFillPattern != nullptr)
		{
			trackFillPattern->savePatternUsedByList ();
		}


		//If this track is using an instrument,
		//1. save the instrument's used-by-list, to include this track
		//2. save the TV part handled by Instrument
		auto trackRxInstrument = SongMgr->getActiveSong ()->editScene->editTrack->getTrackRxInstrument ();
		if (trackRxInstrument != nullptr)
		{
			trackRxInstrument->saveInstrumentUsedByList ();
			//trackRxInstrument->saveInstrumentTV ();
		}

		auto trackTxInstrument = SongMgr->getActiveSong()->editScene->editTrack->getTrackTxInstrument();
		if (trackTxInstrument != nullptr)
		{
			trackTxInstrument->saveInstrumentUsedByList();
			trackTxInstrument->saveInstrumentTV();
		}
		SongMgr->getActiveSong ()->editScene->editTrack->SaveDfaultUsedbyInfo (true);
	}
}


void TrackManager::setTrackFillPattern (std::string name)
{
	if ((SongMgr->getActiveSong () == nullptr) || (SongMgr->getActiveSong ()->editScene == nullptr) ||
		(SongMgr->getActiveSong ()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return;
	}

	//clear used-by-list of existing fill pattern
	Pattern* currentFillpattern = SongMgr->getActiveSong ()->editScene->editTrack->getTrackFillPattern ();
	if (currentFillpattern != nullptr)
	{
		currentFillpattern->RemoveFromUsedByList (true, SongMgr->getActiveSong ()->getName (),
			SongMgr->getActiveSong ()->editScene->getName (), SongMgr->getActiveSong ()->editScene->editTrack->getTrackName ());
	}

	//update used-by-list of new fill pattern
	Pattern* pattern = _PPatternMgr->GetPattern (name);
	if (pattern != nullptr)
	{
		SongMgr->getActiveSong ()->editScene->editTrack->setTrackFillPattern (pattern);
		pattern->UpdateUsedByList (true, SongMgr->getActiveSong ()->getName (),
			SongMgr->getActiveSong ()->editScene->getName (), SongMgr->getActiveSong ()->editScene->editTrack->getTrackName ());

		engine.sequenceManager->updateTrackPattern(pattern);
	}
	else
	{
		SongMgr->getActiveSong ()->editScene->editTrack->setTrackFillPattern (nullptr);
		SongMgr->getActiveSong ()->editScene->editTrack->_trackData->fillPatternName = " ";
	}
}

void TrackManager::setRecordStateOnEditTrack(bool state)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr) ||
		(SongMgr->getActiveSong()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong()->editScene->editTrack->setTrackRecordStatus(state);
}

bool TrackManager::getRecordStateOnEditTrack(void)
{
	if ((SongMgr->getActiveSong() == nullptr) || (SongMgr->getActiveSong()->editScene == nullptr) ||
		(SongMgr->getActiveSong()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return false;
	}
	return SongMgr->getActiveSong()->editScene->editTrack->getTrackRecordStatus();
}

void TrackManager::setSoloStateOnEditTrack (bool state)
{
	if ((SongMgr->getActiveSong () == nullptr) || (SongMgr->getActiveSong ()->editScene == nullptr) ||
		(SongMgr->getActiveSong ()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return;
	}
	SongMgr->getActiveSong ()->editScene->editTrack->setTrackSoloStatus (state);
}

bool TrackManager::getSoloStateOnEditTrack (void)
{
	if ((SongMgr->getActiveSong () == nullptr) || (SongMgr->getActiveSong ()->editScene == nullptr) ||
		(SongMgr->getActiveSong ()->editScene->editTrack == nullptr))
	{
		//TODO warn
		return false;
	}
	return SongMgr->getActiveSong ()->editScene->editTrack->getTrackSoloStatus ();
}
bool TrackManager::IsPatternAssigned (bool isFillPattern, std::string PatternName, std::string SongName, std::string SceneName, std::string TrackName)
{
	std::vector<TrackData> TrackDataList = engine.fileHandler->GetTrackDataList (SongName, SceneName, 64); // 64 - Max track, reads dummy data
	bool found = false;
	for (auto &trackInfo : TrackDataList)
	{
		if (trackInfo.trackName == TrackName)
		{
			if (isFillPattern)
				found = (trackInfo.fillPatternName == PatternName);
			else
				found = (trackInfo.mainPatternName == PatternName);

			if (found)
				return found;
		}
	}
	return found;
}
bool TrackManager::IsInstrumentAssigned (std::string instrumentName, std::string SongName, std::string SceneName, std::string TrackName)
{
	std::vector<TrackData> TrackDataList = engine.fileHandler->GetTrackDataList (SongName, SceneName, 64); // 64 - Max track, reads dummy data
	for (auto& trackInfo : TrackDataList)
	{
		if (trackInfo.trackName == TrackName)
		{
			if ((trackInfo.trackRxInstrumentName == instrumentName) || (trackInfo.trackTxInstrumentName == instrumentName))
				return true;
		}
	}
	return false;
}
