/*
 * Copyright (c) - 2022 - MUSIC Tribe, MY.
 * This file is part of IGNITE.
 *
 * IGNITE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * IGNITE is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with IGNITE. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "Common/Modules/IDevice.h"

using namespace Ignite;

extern std::unique_ptr<IDevice> _PUiVirtualEngine;

namespace ui
{
	////////////////////////////////////////////////
	//   Class: Ignite Track
	////////////////////////////////////////////////
	IgniteTrack::IgniteTrack()
	{
		trackGridPtr = IgniteTrackGrid::GetAppTrackGridInstance(this);
		addChildComponent(trackGridPtr);
		trackGridPtr->setVisible(false);

		trackSwap.makeVisibleAndAddListner(this);
		trackSwap.setClickingTogglesState(false);
		trackSwap.setBounds(766, 10, 132, 44);
		trackSwap.setVisible(false);

		addAndMakeVisible(fwdButton);
		fwdButton.addListener(this);
		fwdButton.setClickingTogglesState(false);
		fwdButton.setDisableImage("btn-next-disabled.png");
		fwdButton.sethoverImages("btn-next-default-hover.png", "btn-next-active-hover.png", "btn-next-disabled-hover.png");

		addAndMakeVisible(backButton);
		backButton.addListener(this);
		backButton.setClickingTogglesState(false);
		backButton.setDisableImage("btn-prev-disabled.png");
		backButton.sethoverImages("btn-prev-default-hover.png", "btn-prev-active-hover.png", "btn-prev-disabled-hover.png");

		scenePtr = IgniteScene::getAppSceneInstance();
		_pChild = AppTrackOpt::GetAppTrackOptionsInstance(this);
		bootup_state = true;
		addChildComponent(_pChild);
		_pChild->setVisible(true);

		trackConfig.makeVisibleAndAddListner(this);
		trackConfig.setBounds(908, 10, 106, 44);
		trackConfig.setVisible(false);

		trackOptCtlPtr = IgniteControlOptions::GetAppOptInstance();
		trackOptCtlPtr->setBounds(xStartOptctl, yStartOptctl, widthOptctl, heightOptctlt);
		trackOptCtlPtr->addOptCtlListener(this);

		trackValuesPtr = trackValuesPtr->GetAppTrackValueInstance();
		addChildComponent(trackValuesPtr);
		trackValuesPtr->setVisible(false);

		trackValueSelectPtr = TrackValueSelection::GetAppTrackValueSelectionInstance();
		addChildComponent(trackValueSelectPtr);
		trackValueSelectPtr->setBounds(210, 0, 604, 480);
		trackValueSelectPtr->setVisible(false);

		addAndMakeVisible(trackView);
		trackView.addListener(this);
		trackView.setToggleState(true, dontSendNotification);
		trackView.setVisible(true);
		trackView.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		trackView.sethoverImages("btn-song-detail-default-hover.png", "btn-song-detail-active-hover.png", "btn-song-detail-default-hover.png");

		addAndMakeVisible(listView);
		listView.addListener(this);
		listView.setToggleState(false, dontSendNotification);
		listView.setVisible(true);
		listView.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		listView.sethoverImages("btn-song-list-default-hover.png", "btn-song-list-active-hover.png", "btn-song-list-default-hover.png");

		viewBox.items.add(FlexItem(trackView).withMargin(1).withFlex(1));
		viewBox.items.add(FlexItem(listView).withMargin(1).withFlex(1));

		RecordModale = IgniteRecordModale::GetAppRecordInstance();
		SongConfig = IgniteSongConfig::getAppSongConfigInstance();
		patternstatusbarptr = PatternStatusBar::GetPatternBarInstance();

		popupPtr = CommonPopUp::getCommonPopupInstance();

		RecordModale->onSelectionChanged = [this](int r)
		{
			if (auto dw = getCurrentlyModalComponent())
			{
				dw->exitModalState(0);
				trackOptCtlPtr->isEditing = false;
			}
			if (r == 1)
			{
				_pChild->handlePattern(0);
				trackGridPtr->updateTrackData((int)IgniteScene::getAppSceneInstance()->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK));
				trackGridPtr->setInterceptsMouseClicks(false, false);
				sendRecordRequestToEngine(1);
			}
			else if (r == 2)
			{
				sendRecordRequestToEngine(1);
				trackGridPtr->setInterceptsMouseClicks(false, false);
			}
			else if (r == 3)
			{
				trackOptCtlPtr->setOperationMode(mt::Ignite::operatioMode::AUDITION_MODE);
				trackOptCtlPtr->updatePlayBtnState();
				trackOptCtlPtr->updateRecBtnState();
				sendRecordRequestToEngine(2);
				trackGridPtr->setInterceptsMouseClicks(false, false);
			}
			else
			{
				trackGridPtr->setInterceptsMouseClicks(true, true);
				enableControlButton(IgniteControlOptions::STOP, true);
				trackOptCtlPtr->setOperationMode(mt::Ignite::operatioMode::IDLE_MODE);
				trackOptCtlPtr->setRecButtonMode(false);
			}
			updateSelectionInfo();
		};
	}


	IgniteTrack::~IgniteTrack()
	{
		cleanup();
	}

	void IgniteTrack::swapTrack(IgniteTrackSwap::swapCtlKey key, int index1, int index2)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "index1: " + String(index1) + "::" + "index2: " + String(index2));
		if (index1 != index2)
		{
			if (key == IgniteTrackSwap::swapCtlKey::SWAP)
			{
				int current_track = (int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK);

				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramActiveTrack, index1);
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackSwapCmd, index2);
				updateCurrentTrackDataFromEngine(index1);
				updateCurrentTrackDataFromEngine(index2);

				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramActiveTrack, current_track);
				scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK, current_track, true);
				refreshTrackUI();
			}
		}
		mainmodeptr = AppMainmode::getMainModeInstance();
		mainmodeptr->trackSwapComp->exitModalState(0);
		mainmodeptr->trackSwapComp->setVisible(false);
	}

	void IgniteTrack::refreshTrackUI()
	{
		if (trackCreated && this->isVisible())
		{
			if ((int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK) < 0)
				scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK, currentTrackBank + 0, false);

			(int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::NUM_TRACK_BANK_INDEX) ? numOfTrackBanks = (int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::NUM_TRACK_BANK_INDEX) : numOfTrackBanks = 1;
			trackGridPtr->updateTrackGridData();
			_pChild->refreshData();
		}
		updateSelectionInfo();
		if (trackDataUpdated && !IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled())
		{
			enableControlButton(IgniteControlOptions::SAVE, true);
			enableControlButton(IgniteControlOptions::UNDO, true);
		}
	}

	void IgniteTrack::addTracks(mt::Ignite::sceneData& data)
	{
		mt::Ignite::trackData tData = { 0, " ", 0, " ", " ", " ", " ", false, false, false, 0 };
		(data.num_of_track_banks) ? numOfTrackBanks = data.num_of_track_banks : numOfTrackBanks = 1;

		std::vector<IgniteParamList> trackParams;
		IgniteParamList params;

		for (auto i = 1; i <= (numOfTrackBanks * 16); i++)
		{
			tData.id = i;
			tData.name = "Track " + String(i);
			data.track_data.push_back(tData);
		}
		currentTrackBank = 0;
		trackCreated = true;
	}


	void IgniteTrack::updateNumOfTracks(mt::Ignite::sceneData& data)
	{
		std::vector<IgniteParamList> trackParams;
		IgniteParamList params;
		mt::Ignite::trackData tData = { 0, " ", 0, " ", " ", " ", " ", false, false, false, 0 };
		int current_track = (int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK);

		if (numOfTrackBanks == data.num_of_track_banks)
			return;

		else if ((numOfTrackBanks > data.num_of_track_banks) && (data.num_of_track_banks > 0))
		{
			for (auto i = 1; i <= ((numOfTrackBanks - (data.num_of_track_banks)) * 16); i++)
			{
				data.track_data.pop_back();
			}
			if (current_track >= data.num_of_track_banks * 16)
			{
				current_track = 0;
			}
			scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK, current_track, false);
			scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK_BANK, 0, false);
			currentTrackBank = 0;
		}
		else if (numOfTrackBanks < data.num_of_track_banks)
		{
			for (auto i = (numOfTrackBanks * 16) + 1; i <= ((data.num_of_track_banks) * 16); i++)
			{
				tData.id = i;
				tData.name = "Track " + String(i);
				data.track_data.push_back(tData);
			}
		}

		if (((int)IgniteSong::getAppSongInstance()->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES) - 1) == num_of_scenes)
		{
			numOfTrackBanks = data.num_of_track_banks;
			num_of_scenes = 0;
			trackCreated = true;
			refreshTrackUI();
		}
		else
			num_of_scenes++;
	}

	void IgniteTrack::updateCurrentTrackDataFromEngine(int index)
	{
		std::vector<IgniteParamList> TrackParamsList;
		IgniteParamList params;
		mt::Ignite::trackData* gTrackRef;

		if (index >= 0)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramActiveTrack, index);
			scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK, index, true);
		}

		TrackParamsList.clear();
		params.param = (paramId_t)TrackParam::paramTrackInstrumentDispName;
		TrackParamsList.push_back(params);
		params.param = (paramId_t)TrackParam::paramTrackOutInstrumentDispName;
		TrackParamsList.push_back(params);
		params.param = (paramId_t)TrackParam::paramTrackPatternDispName;
		TrackParamsList.push_back(params);
		params.param = (paramId_t)TrackParam::paramTrackFillPatternDispName;
		TrackParamsList.push_back(params);
		params.param = (paramId_t)TrackParam::paramTrackSolo;
		TrackParamsList.push_back(params);
		params.param = (paramId_t)TrackParam::paramTrackRecord;
		TrackParamsList.push_back(params);
		params.param = (paramId_t)TrackParam::paramTrackMute;
		TrackParamsList.push_back(params);

		TrackParamsList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, TrackParamsList);

		gTrackRef = scenePtr->getTrackDataRef();

		for (auto& j : TrackParamsList)
		{
			if (j.param == (paramId_t)TrackParam::paramTrackInstrumentDispName)
			{
				gTrackRef->instrument = j.MsgValue.get<std::string>();
			}
			if (j.param == (paramId_t)TrackParam::paramTrackOutInstrumentDispName)
			{
				gTrackRef->outputInstrument = j.MsgValue.get<std::string>();
			}
			else if (j.param == (paramId_t)TrackParam::paramTrackPatternDispName)
			{
				gTrackRef->pattern = j.MsgValue.get<std::string>();
			}
			else if (j.param == (paramId_t)TrackParam::paramTrackFillPatternDispName)
			{
				gTrackRef->fillPattern = j.MsgValue.get<std::string>();
			}
			else if (j.param == (paramId_t)TrackParam::paramTrackSolo)
			{
				gTrackRef->soloMode = j.MsgValue.get<uint8_t>();
			}
			else if (j.param == (paramId_t)TrackParam::paramTrackRecord)
			{
				gTrackRef->recordState = j.MsgValue.get<uint8_t>();
			}
			else if (j.param == (paramId_t)TrackParam::paramTrackMute)
			{
				gTrackRef->trackState = j.MsgValue.get<uint8_t>();
			}
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Track Data, index" + String(index) + "Instrument Name" + String(gTrackRef->instrument) + "Main Pattern Name" + String(gTrackRef->pattern) + "Fill Pattern Name" + String(gTrackRef->fillPattern));
		}
		currentSceneBar = 0;
		activeStepId = 1;
		activeBarId = 1;
		trackGridPtr->updateTrackButtonState();
		_pChild->refreshData();
	}

	void IgniteTrack::updateDataFromEngine()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Track data request from engine");

		if (scenePtr->getSceneData(mt::Ignite::sceneDataIndex::TRACK_DATA_REVALIDATE) || getInstrDataDirtyFlag() || getPatternDataDirtyFlag() || isNumberofTrackChanged() || getTrackMuteDirtyFlag())
		{
			scenePtr->setSceneData(mt::Ignite::sceneDataIndex::TRACK_DATA_REVALIDATE, false, false);
			std::vector<IgniteParamList> TrackParamsList;
			IgniteParamList params;
			mt::Ignite::trackData* gTrackRef;
			int edit_track = (int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK);
			params.param = (paramId_t)TrackParam::paramTrackInfoList;
			TrackParamsList.push_back(params);

			TrackParamsList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, TrackParamsList);

			if (TrackParamsList.size() > 0)
			{
				uint8_t numParam = 8;// number of params for each track - return by TrackParam::paramTrackInfoList
                const int returnedTracks = int(TrackParamsList.size() / numParam);
                for (int tid = 0; tid < std::min(numOfTrackBanks * 16, returnedTracks); ++tid)
				{
					int pos = TrackParamsList[numParam * tid].startPos;
					scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK, pos, true);
					gTrackRef = scenePtr->getTrackDataRef();
					gTrackRef->instrument = TrackParamsList[(numParam * tid) + 1].MsgValue.get<std::string>();
					gTrackRef->outputInstrument = TrackParamsList[(numParam * tid) + 2].MsgValue.get<std::string>();
					gTrackRef->pattern = TrackParamsList[(numParam * tid) + 3].MsgValue.get<std::string>();
					gTrackRef->fillPattern = TrackParamsList[(numParam * tid) + 4].MsgValue.get<std::string>();
					gTrackRef->soloMode = TrackParamsList[(numParam * tid) + 5].MsgValue.get<uint8_t>();
					gTrackRef->recordState = TrackParamsList[(numParam * tid) + 6].MsgValue.get<bool>();
					gTrackRef->trackState = TrackParamsList[(numParam * tid) + 7].MsgValue.get<bool>();
				}
				if (bootup_state)
				{
					edit_track = 0;
					bootup_state = false;
				}
                edit_track = std::clamp(edit_track,0,std::max(0,returnedTracks-1));
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramActiveTrack, edit_track);
				scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK, edit_track, true);
				trackGridPtr->updateTrackButtonState();
				gTrackRef = scenePtr->getTrackDataRef();

				setTrackMuteDirtyFlag(false);
				setTrackSoloDirtyFlag(false);
				setPatternDataDirtyFlag(false);
				noOfTrackChaged(false);
				_pChild->refreshData();
			}
		}
		currentSceneBar = 0;
		activeStepId = 1;
		activeBarId = 1;
	}

	void IgniteTrack::updateTrackDataList()
	{
		if (instrumentDataUpdated)
		{
			auto patternList = _PUiVirtualEngine->EngineGetPatternList(true);
			std::vector<IgniteParamList> instrumentParamList;
			IgniteParamList params;
			params.param = (paramId_t)InstrumentParam::paramInstrumentNameList;
			instrumentParamList.push_back(params);
			instrumentParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentParamList);

			_pChild->updateInstrumentBox(instrumentParamList);
			_pChild->updatePatternBox(patternList);
			_pChild->updateFillPatternBox(patternList);
			instrumentDataUpdated = false;
		}
	}
	void IgniteTrack::updateLables()
	{
		std::vector<IgniteParamList> trackParams;
		IgniteParamList params;

		params.param = (paramId_t)TrackParam::paramUndoListCmd;
		params.startPos = 0;
		trackParams.push_back(params);
		trackParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, trackParams);

		if (trackParams.size() == 1)
		{
			enableControlButton(IgniteControlOptions::SAVE, false);
			enableControlButton(IgniteControlOptions::UNDO, false);
			_pChild->updateTrackEditIndication();
			trackDataUpdated = false;
		}
		_pChild->repaint();
	}

	void IgniteTrack::updateTrackData(uint8_t param, const std::string& value)
	{
		mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef();
		switch ((TrackParam)param)
		{
		case TrackParam::paramTrackName:
			gTrackRef->name = value;
			break;
		}
	}

	void IgniteTrack::updateTrackData(uint8_t param, uint8_t value)
	{
		if ((SceneParam)param == SceneParam::paramSceneBarId)
			currentSceneBar = value;
	}

	void IgniteTrack::setTrackData(mt::Ignite::trackDataIndex item_id, int data, bool is_internal, int currentTrack)
	{
		mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef(currentTrack);
		auto current_track = (int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK);
		auto songComp = IgniteSong::getAppSongInstance();

		if (currentTrack == -1)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramActiveTrack, current_track);
		else
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramActiveTrack, currentTrack);

		switch (item_id)
		{
		case mt::Ignite::trackDataIndex::TRACK_STATE:
			if ((gTrackRef->pattern != " ") && (gTrackRef->outputInstrument != " "))
			{
				gTrackRef->trackState = !data;
				scenePtr->setSceneData(mt::Ignite::sceneDataIndex::TRACKS_ACTIVE, data, false);

				if (gTrackRef->pattern != " ")
				{
					auto pIndex = _pChild->getPatternIndex(gTrackRef->pattern);
					if ((pIndex - 1) >= 0)
					{
						IgnitePattern::getAppPatternInstance()->setPatternIndex(pIndex - 1);
						IgnitePattern::getAppPatternInstance()->setEditPattern(pIndex - 1);
					}
				}
				if (!is_internal)
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackMute, gTrackRef->trackState);
			}
			else
			{
				gTrackRef->trackState = true;
				scenePtr->setSceneData(mt::Ignite::sceneDataIndex::TRACKS_ACTIVE, true, false);
			}

			songComp->isSongSetupComplete();
			break;
		case mt::Ignite::trackDataIndex::TRACK_SOLO_MODE:
			gTrackRef->soloMode = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackSolo, data);
			break;
		case mt::Ignite::trackDataIndex::TRACK_REC_STATE:
			gTrackRef->recordState = data;
			songComp->isSongSetupComplete();
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackRecord, data);
			break;
		default: break;
		}
		if (IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, 0);
	}

	void IgniteTrack::setTrackData(mt::Ignite::trackDataIndex item_id, String data, bool is_internal)
	{
		mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef();
		auto current_track = (int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK);
		auto songComp = IgniteSong::getAppSongInstance();

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramActiveTrack, current_track);

		switch (item_id)
		{
		case mt::Ignite::trackDataIndex::TRACK_NAME:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackName, data.toStdString());
			gTrackRef->name = data;
			break;
		case mt::Ignite::trackDataIndex::TRACK_PATTERN:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackPattern, data.toStdString());
			gTrackRef->pattern = data;
			songComp->isSongSetupComplete();
			break;
		case mt::Ignite::trackDataIndex::TRACK_PFILL:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackFillPattern, data.toStdString());
			gTrackRef->fillPattern = data;

			break;
		case mt::Ignite::trackDataIndex::TRACK_INSTR:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackInstrument, data.toStdString());
			gTrackRef->instrument = data;
			songComp->isSongSetupComplete();
			break;
		case mt::Ignite::trackDataIndex::TRACK_OUT_INSTR:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramTrackOutInstrument, data.toStdString());
			gTrackRef->outputInstrument = data;
			songComp->isSongSetupComplete();
			break;
		default: break;
		}

		if (IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, 0);
	}

	var IgniteTrack::getTrackData(mt::Ignite::trackDataIndex item_id)
	{
		if (trackCreated)
		{
			mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef();

			switch (item_id)
			{
			case mt::Ignite::trackDataIndex::TRACK_ID:
				return gTrackRef->id;
			case mt::Ignite::trackDataIndex::TRACK_PATTERN:
				return gTrackRef->pattern;
			case mt::Ignite::trackDataIndex::TRACK_PFILL:
				return gTrackRef->fillPattern;
			case mt::Ignite::trackDataIndex::TRACK_INSTR:
				return gTrackRef->instrument;
			case mt::Ignite::trackDataIndex::TRACK_OUT_INSTR:
				return gTrackRef->outputInstrument;
			case mt::Ignite::trackDataIndex::TRACK_NAME:
				return gTrackRef->name;
			case mt::Ignite::trackDataIndex::TRACK_STATE:
				return !gTrackRef->trackState;
			case  mt::Ignite::trackDataIndex::TRACK_SOLO_MODE:
				return gTrackRef->soloMode;
			case  mt::Ignite::trackDataIndex::TRACK_REC_STATE:
				return gTrackRef->recordState;
			default:
				return -1;
			}
		}
		return -1;
	}

	void IgniteTrack::cleanup()
	{
	}

	void IgniteTrack::getNextTrackBankAndUpdate(bool direction)
	{
		if (direction)
		{
			currentTrackBank++;
			if (currentTrackBank >= (numOfTrackBanks))
				currentTrackBank = 0;
		}
		else
		{
			currentTrackBank--;
			if (currentTrackBank < 0)
				currentTrackBank = numOfTrackBanks - 1;
		}
		scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK_BANK, currentTrackBank, false);
		trackGridPtr->updateTrackButtonState();
		trackGridPtr->updateTrackGridData();
	}

	void IgniteTrack::getTrackBankAndUpdate(int trackBank)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (trackBank >= 0 && trackBank < numOfTrackBanks)
		{
			currentTrackBank = trackBank;
			scenePtr->setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK_BANK, currentTrackBank, false);
			trackGridPtr->updateTrackButtonState();
			trackGridPtr->updateTrackGridData();
		}
	}

	void IgniteTrack::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		mainmodeptr = AppMainmode::getMainModeInstance();
		if (buttonThatWasClicked == &fwdButton)
		{
			if (fwdButton.getToggleState())
				fwdButton.setToggleState(false, dontSendNotification);
			getNextTrackBankAndUpdate(true);
			mainmodeptr->setFpFillAndSoloLedState();
			mainmodeptr->setFpEditTrackLedState();
			TrackMixer::GetTrackMixerInstance()->getTrackMixerBankAndUpdate(true);

		}
		else if (buttonThatWasClicked == &backButton)
		{
			if (backButton.getToggleState())
				backButton.setToggleState(false, dontSendNotification);
			getNextTrackBankAndUpdate(false);
			mainmodeptr->setFpFillAndSoloLedState();
			mainmodeptr->setFpEditTrackLedState();
			TrackMixer::GetTrackMixerInstance()->getTrackMixerBankAndUpdate(false);
		}
		else if (buttonThatWasClicked == &trackSwap)
		{
			if (getTrackData(mt::Ignite::trackDataIndex::TRACK_STATE))
				mainmodeptr->trackSwapComp->setDefaultEditTrack((int)scenePtr->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK) + 1, 1);
			else
				mainmodeptr->trackSwapComp->setDefaultEditTrack(1, 1);

			mainmodeptr->trackSwapComp->setVisible(true);
			mainmodeptr->trackSwapComp->runModalLoop();
		}
		else if (&trackConfig == buttonThatWasClicked)
		{
			AppMainmode::getMainModeInstance()->changeTabVisibility(mt::Ignite::tabs::MENU, true);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::MENU);
			AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::TRACK_WINDOW);
			if (trackConfig.getToggleState())
				trackConfig.setToggleState(false, dontSendNotification);
		}
		else if (buttonThatWasClicked == &listView)
		{
			changeTrackOptionsVisibility(trackScreenIndex::TRACK_WINDOW);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::TRACK);
		}
		else if (buttonThatWasClicked == &trackView)
		{
			changeTrackOptionsVisibility(trackScreenIndex::OPTION_WINDOW);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::TRACK);
		}
	}

	void IgniteTrack::paint(juce::Graphics& g)
	{
	}

	int IgniteTrack::getViewStatus()
	{
		if (trackView.getToggleState())
			return 1;
		else if (listView.getToggleState())
			return 0;
		else
			return -1;
	}

	void IgniteTrack::resized()
	{
		trackGridPtr->setBounds(getLocalBounds());
		_pChild->setBounds(getLocalBounds());
		trackValuesPtr->setBounds(getLocalBounds());
		if (trackValuesPtr->isVisible())
			viewBox.performLayout(ui::Rectangle(0, 0, 0, 0));
		else
			viewBox.performLayout(ui::Rectangle(xStartViewBtn, yStartViewBtn, widthViewBtn, heightViewBtn));

		fwdButton.setBounds(172, 10, 44, 44);
		backButton.setBounds(120, 10, 44, 44);
	}

	void IgniteTrack::changeTrackOptionsVisibility(trackScreenIndex index)
	{
		mainmodeptr = AppMainmode::getMainModeInstance();
		auto appOptComp = AppOptions::GetOptionsInstance();

		Currentindex = index;
		trackGridPtr->setVisible(false);
		_pChild->setVisible(false);
		backButton.setVisible(false);
		fwdButton.setVisible(false);
		trackValuesPtr->setVisible(false);
		trackConfig.setVisible(false);
		trackSwap.setVisible(false);
		listView.setToggleState(false, dontSendNotification);
		trackView.setToggleState(false, dontSendNotification);
		appOptComp->taboptionChanges(mt::Ignite::tabs::TRACK, (int)trackScreenIndex::TRACK_WINDOW);

		appOptComp->taboptionChanges(mt::Ignite::tabs::TRACK, 0);
		mainmodeptr->setFpFillAndSoloLedState();
		mainmodeptr->setFpEditTrackLedState();

		switch (index)
		{
		case trackScreenIndex::TRACK_WINDOW:
			trackGridPtr->setVisible(true);
			fwdButton.setVisible(true);
			backButton.setVisible(true);
			trackConfig.setVisible(true);
			trackSwap.setVisible(true);
			listView.setToggleState(true, dontSendNotification);
			appOptComp->taboptionChanges(mt::Ignite::tabs::TRACK, (int)trackScreenIndex::TRACK_WINDOW);

			if (isTrackMuteChanged)
				scenePtr->updateTracksData();

			trackGridPtr->updateTrackGridData();
			break;
		case trackScreenIndex::OPTION_WINDOW:
			_pChild->setVisible(true);
			_pChild->refreshData();
			trackView.setToggleState(true, dontSendNotification);
			appOptComp->taboptionChanges(mt::Ignite::tabs::TRACK, (int)trackScreenIndex::OPTION_WINDOW);

			if (isTrackSoloChanged)
				scenePtr->updateTracksData();
			break;
		case trackScreenIndex::INSTR_WINDOW:
			break;
		case trackScreenIndex::TRACKVALUE_WINOW:
			appOptComp->taboptionChanges(mt::Ignite::tabs::TRACK, (int)trackScreenIndex::TRACKVALUE_WINOW);
			appOptComp->changebtnstate(IgniteControlOptions::TRACK_CTL, trackValuesPtr->getInstSoundStatus());
			appOptComp->changebtnstate(IgniteControlOptions::ALT_FN, appOptComp->getBtnState(IgniteControlOptions::TRACK_CTL));
			trackValuesPtr->setVisible(true);
			break;
		default:
			Currentindex = trackScreenIndex::WINOW_INVALID;
			break;
		}
		resized();
	}

	void IgniteTrack::deleteCurrentTrack()
	{
	}

	void IgniteTrack::setCurrentTrackData(int currentTrack)
	{
		mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef(currentTrack);
		setTrackData(mt::Ignite::trackDataIndex::TRACK_NAME, gTrackRef->name, true);
		setTrackData(mt::Ignite::trackDataIndex::TRACK_PATTERN, gTrackRef->pattern, true);
		setTrackData(mt::Ignite::trackDataIndex::TRACK_PFILL, gTrackRef->fillPattern, true);
		setTrackData(mt::Ignite::trackDataIndex::TRACK_INSTR, gTrackRef->instrument, true);
		trackGridPtr->updateTrackGridData();
		if (gTrackRef->instrument != " ")
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, (gTrackRef->instrument).toStdString());
			for (int i = 0; i < 8; i++)
				trackValuesPtr->getdatafromEngine(i);
		}
		else
			for (int i = 0; i < 8; i++)
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_RELEASE + i), 0, false);
		if (gTrackRef->trackState)
		{
			String pattern_name = getTrackData(mt::Ignite::trackDataIndex::TRACK_PATTERN).toString();
			if (pattern_name != " ")
			{
				auto pIndex = _pChild->getPatternIndex(pattern_name);
				if ((pIndex - 1) >= 0)
				{
					IgnitePattern::getAppPatternInstance()->setPatternIndex(_pChild->getPatternIndex(pattern_name) - 1);
					IgnitePattern::getAppPatternInstance()->setEditPattern(_pChild->getPatternIndex(pattern_name) - 1);
				}
			}
		}
	}

	void IgniteTrack::updateSelectionInfo(void)
	{
		String displayMessage1 = " ";
		String displayMessage2 = " ";
		auto remainingSceneBar = 0;

		if (statusBarData.isSongCreated)
		{
			displayMessage1 = "SONG::[" + String(statusBarData.songIndex + 1) + "]";

			if (statusBarData.isSceneCreated)
			{
				auto patternComp = IgnitePattern::getAppPatternInstance();
				mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef();
				opMode = trackOptCtlPtr->getOperationMode();
				auto ptIndex = _pChild->getPatternIndex(gTrackRef->pattern);

				if ((opMode == mt::Ignite::operatioMode::IDLE_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_MODE))
				{
					if ((ptIndex - 1) <= 0)
						ptIndex = 1;

					patternComp->setPatternIndex(ptIndex - 1);
					patternComp->setEditPattern(ptIndex - 1);
					patternComp->setEditTrackPattern(gTrackRef->pattern);
					if(patternComp->isVisible() && patternComp->getTabIndex() != -1)
						patternComp->refreshPatternUI();
				}

				if (gTrackRef->pattern != " ")
					remainingSceneBar = statusBarData.sceneLength - currentSceneBar;

				displayMessage1 = displayMessage1 + "  SCENE::[" + String(statusBarData.editSceneIndex) +
					"]  TEMPO::[" + String(statusBarData.tempo) +
					"]  SWING::[" + String(statusBarData.swing) +
					"]  NEXT SCENE::[" + String(statusBarData.nextSceneIndex) + "]" + (!statusBarData.songadv ? +"[" + String(remainingSceneBar) + "]" : "");
				displayMessage2 = "EDIT TRACK::[" + String(statusBarData.currentTrack + 1) + "]";
				if (gTrackRef->pattern != " ")
				{
					if (bootupState)
					{
						bootupState = false;
						statusBarData.numberOfBars = static_cast<uint8_t>((int)patternComp->getPatternData(mt::Ignite::ptDataIndex::BARNUM) + 1);
						statusBarData.numberOfSteps = static_cast<int16_t>((int)patternComp->getPatternData(mt::Ignite::ptDataIndex::LASTSTEP));
						statusBarData.timeBase = static_cast<uint8_t>((int)patternComp->getPatternData(mt::Ignite::ptDataIndex::TIMEBASE));
					}

					displayMessage2 = displayMessage2 + "[" + String(statusBarData.numberOfBars) +
						"][" + String(statusBarData.numberOfSteps + 1) + "]";
					displayMessage2 = displayMessage2 + "  PATTERN::[" + String(ptIndex) +
						"]  TB::[" + mt::Ignite::tbvalues[statusBarData.timeBase] + "]";
				}
				if (opMode == mt::Ignite::operatioMode::PLAY_MODE)
					displayMessage2 = displayMessage2 + "  BAR:STEP::[" + String(activeBarId) + "][" + String(activeStepId) + "]";
			}
			AppStatusbar::GetAppStatusBarInstance()->setDisplayMessage(displayMessage1, displayMessage2);
		}
	}

	void IgniteTrack::onOptCtlButtonClicked(AppOptCtl::pKey option)
	{
		switch (option) {
		case AppOptCtl::pKey::BACK:
			if (trackDataUpdated)
			{
				trackDataUpdated = false;
				_pChild->updateTrackEditIndication();
			}
			changeTrackOptionsVisibility(trackScreenIndex::TRACK_WINDOW);
			break;
		case AppOptCtl::pKey::SAVE:
			trackDataUpdated = false;
			changeTrackOptionsVisibility(trackScreenIndex::TRACK_WINDOW);
			break;
		case AppOptCtl::pKey::COPY: break;
		case AppOptCtl::pKey::MOVE: break;
		case AppOptCtl::pKey::DEL:
			changeTrackOptionsVisibility(trackScreenIndex::INSTR_WINDOW);
			break;
		}
		setCurrentTrackData();
	}

	void IgniteTrack::sendRecordRequestToEngine(int state)
	{
		auto selected_song = IgniteSong::getAppSongInstance()->getSelected();
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected_song);
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramAuditioningState, false);

		if (state == 1)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramStartRecordCmd, selected_song);
			enableControlButton(IgniteControlOptions::PLAY, true);
			trackOptCtlPtr->recordKeyBlinkCtrl(true);
		}
		else if (state == 2)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramAuditioningState, true);
			enableControlButton(IgniteControlOptions::PLAY, true);
			startTimerHz(2);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_PLAY, fpLEDPattern::EN_LED_PATT_Blink_Slow);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_REC, fpLEDPattern::EN_LED_PATT_Blink_Slow);
			trackOptCtlPtr->recordKeyBlinkCtrl(true);
		}
		else
		{
			stopTimer();
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_PLAY, fpLEDPattern::EN_LED_PATT_ON);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_REC, fpLEDPattern::EN_LED_PATT_ON);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramStopRecordCmd, selected_song);
			trackOptCtlPtr->recordKeyBlinkCtrl(false);
			trackGridPtr->setInterceptsMouseClicks(true, true);
		}
	}

	void IgniteTrack::timerCallback()
	{
		if (opMode == mt::Ignite::operatioMode::AUDITION_MODE)
		{
			if (recButtonBlinkState)
			{
				trackOptCtlPtr->recordKeyBlinkCtrl(false);
				trackOptCtlPtr->playKeyBlinkCtrl(false);
				recButtonBlinkState = false;
			}
			else
			{
				trackOptCtlPtr->recordKeyBlinkCtrl(true);
				trackOptCtlPtr->playKeyBlinkCtrl(true);
				recButtonBlinkState = true;
			}
		}
		else if (opMode == mt::Ignite::operatioMode::PAUSE_MODE)
		{
			if (recButtonBlinkState)
			{
				trackOptCtlPtr->playKeyBlinkCtrl(false);
				recButtonBlinkState = false;
			}
			else
			{
				trackOptCtlPtr->playKeyBlinkCtrl(true);
				recButtonBlinkState = true;
			}
		}
	}

	void IgniteTrack::onOptCtlButtonClicked(IgniteControlOptions::eKey option)
	{
		auto appOptComp = AppOptions::GetOptionsInstance();
		opMode = trackOptCtlPtr->getOperationMode();

		if (option == IgniteControlOptions::PLAY)
		{
			mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef();
			if (gTrackRef->trackState && (gTrackRef->instrument != " ") && (gTrackRef->pattern != " "))
				IgnitePattern::getAppPatternInstance()->setEditTrackPattern(gTrackRef->pattern);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_PLAY, fpLEDPattern::EN_LED_PATT_ON);

			if (opMode == mt::Ignite::operatioMode::PAUSE_MODE)
			{
				startTimerHz(2);
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_PLAY, fpLEDPattern::EN_LED_PATT_Blink_Slow);
			}
		}

		else if (option == IgniteControlOptions::STOP)
		{
			stopTimer();
			activeBarId = 1;
			activeStepId = 1;
			updateSelectionInfo();
			trackGridPtr->setInterceptsMouseClicks(true, true);
			trackOptCtlPtr->recordKeyBlinkCtrl(true);
			trackOptCtlPtr->setRecButtonMode(false);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_PLAY, fpLEDPattern::EN_LED_PATT_ON);
		}

		else if (option == IgniteControlOptions::REC)
		{
			mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef();
			timeHours = 0;
			timeMinutes = 0;
			timeSeconds = 0;

			if ((opMode == mt::Ignite::operatioMode::IDLE_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::AUDITION_MODE))
				sendRecordRequestToEngine(false);
			else
				(gTrackRef->pattern != " ") ? RecordModale->show(true, true) : RecordModale->show(false, true);
		}
		else if (option == IgniteControlOptions::PATTERN)
		{
			AppMainmode::getMainModeInstance()->triggerTabChange(mt::Ignite::tabs::PATTERN);
		}
		else if (option == IgniteControlOptions::INST)
		{
			AppMainmode::getMainModeInstance()->triggerTabChange(mt::Ignite::tabs::MENU);
			AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::INSTR_WINDOW);
		}
		else if (option == IgniteControlOptions::ALT_FN)
		{
			if (trackValuesPtr->isVisible())
			{
				trackValuesPtr->setAltFnStatus(appOptComp->getBtnState(IgniteControlOptions::ALT_FN));
				trackValuesPtr->updateTrackValueScreen();
			}
		}
		else if (option == IgniteControlOptions::TRACK_CTL)
		{
			if (trackValuesPtr->isVisible())
			{
				auto trackCtlState = appOptComp->getBtnState(IgniteControlOptions::TRACK_CTL);
				appOptComp->updateAltFunBtnState();
				trackValuesPtr->setInstSoundStatus(trackCtlState);
				trackValuesPtr->updateTrackValueScreen();
			}
		}
		else if (option == IgniteControlOptions::SAVEALLON)
		{
			trackDataUpdated = false;
		}
		else if (option == IgniteControlOptions::AUTOSAVEON)
		{
			isAutosave = true;
		}
		else if (option == IgniteControlOptions::AUTOSAVEOFF)
		{
			isAutosave = false;
		}
		else
		{
			if (isVisible())
			{
				switch (option) {
				case IgniteControlOptions::SAVE:
					saveTrack();
					break;
				case IgniteControlOptions::DEL:
					break;
				case IgniteControlOptions::UNDO:
					if (trackDataUpdated)
					{
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramActiveTrack, (int)IgniteScene::getAppSceneInstance()->getSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK));
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramUndoCmd, 1);
						Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 50);
						updateLables();
						updateCurrentTrackDataFromEngine(-1);
					}
					break;
				}
			}
		}
	}

	void IgniteTrack::saveTrack()
	{
		String msg;

		(backButton.isVisible()) ? (msg = "Save Tracks edits ") : (msg = "Save <" + scenePtr->getTrackDataRef()->name + "> edits");
		if (popupPtr->isDialogOpen.get() != true)
		{
			popupPtr->close();
			popupPtr->setPopUpPropories("Save ", 604, 290);
			popupPtr->setbtnsize(265, 44);
			popupPtr->setInfoText(msg);
			popupPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
			popupPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
			popupPtr->popupType = POPUP_WITHOKCANCEL;
			popupPtr->isCloseButtonVisible(false);
			popupPtr->show();

			popupPtr->onSelectionChanged = [this](int r)
			{
				if (r == 1)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramSaveTrackCmd, 0);
					trackDataUpdated = false;
					updateLables();
					enableControlButton(IgniteControlOptions::SAVE, false);
					enableControlButton(IgniteControlOptions::UNDO, false);
				}

				if (auto dw = getCurrentlyModalComponent())
					dw->exitModalState(0);
			};
		}
	}

	bool IgniteTrack::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		if (KeyValue == (uint32_t)FrontPannelKeyStatus::paramKeyStatusReleased)
			return true;

		bool handled = false;

		if (popupPtr != nullptr && popupPtr->IsEditing())
			return popupPtr->FPKeyHandler(true, param, KeyValue);

		if ((option_handler_enabled) && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
		{
			handled = PannelKeyOptionsHandler(param, KeyValue);
		}
		else
		{
			switch (Currentindex)
			{
			case trackScreenIndex::TRACK_WINDOW:
				handled = (trackGridPtr->PannelKeyHandler(param, KeyValue));
				break;
			case trackScreenIndex::OPTION_WINDOW:
				handled = _pChild->PannelKeyHandler(param, KeyValue);
				return handled;
				break;
			case trackScreenIndex::TRACKVALUE_WINOW:
				handled = trackValuesPtr->PannelKeyHandler(param, KeyValue);
				return handled;
				break;
			}

			if (!handled || param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
			{
				switch ((FrontPannelParam)param)
				{
				case FrontPannelParam::paramKey_TRACKSEL1:
				case FrontPannelParam::paramKey_TRACKSEL2:
				case FrontPannelParam::paramKey_TRACKSEL3:
				case FrontPannelParam::paramKey_TRACKSEL4:
				case FrontPannelParam::paramKey_TRACKSEL5:
				case FrontPannelParam::paramKey_TRACKSEL6:
				case FrontPannelParam::paramKey_TRACKSEL7:
				case FrontPannelParam::paramKey_TRACKSEL8:
				case FrontPannelParam::paramKey_TRACKSEL9:
				case FrontPannelParam::paramKey_TRACKSEL10:
				case FrontPannelParam::paramKey_TRACKSEL11:
				case FrontPannelParam::paramKey_TRACKSEL12:
				case FrontPannelParam::paramKey_TRACKSEL13:
				case FrontPannelParam::paramKey_TRACKSEL14:
				case FrontPannelParam::paramKey_TRACKSEL15:
				case FrontPannelParam::paramKey_TRACKSEL16://Fallthrough
				case FrontPannelParam::paramKey_TRACKROTARY1:
				case FrontPannelParam::paramKey_TRACKROTARY2:
				case FrontPannelParam::paramKey_TRACKROTARY3:
				case FrontPannelParam::paramKey_TRACKROTARY4:
				case FrontPannelParam::paramKey_TRACKROTARY5:
				case FrontPannelParam::paramKey_TRACKROTARY6:
				case FrontPannelParam::paramKey_TRACKROTARY7:
				case FrontPannelParam::paramKey_TRACKROTARY8:
				case FrontPannelParam::paramKey_TRACKROTARY9:
				case FrontPannelParam::paramKey_TRACKROTARY10:
				case FrontPannelParam::paramKey_TRACKROTARY11:
				case FrontPannelParam::paramKey_TRACKROTARY12:
				case FrontPannelParam::paramKey_TRACKROTARY13:
				case FrontPannelParam::paramKey_TRACKROTARY14:
				case FrontPannelParam::paramKey_TRACKROTARY15:
				case FrontPannelParam::paramKey_TRACKROTARY16://Fallthrough
					handled = (trackGridPtr->PannelKeyHandler(param, KeyValue));
					break;
				case FrontPannelParam::paramKey_RETURN_NAV:
					if (backButton.isVisible())
					{
						handled = true;
						buttonClicked(&backButton);
					}
					break;
				case FrontPannelParam::paramKey_PAGESHIFT:
					option_handler_enabled ? option_handler_enabled = false : option_handler_enabled = true;
					handled = PannelKeyOptionsHandler(param, KeyValue);
					break;
				}
			}
		}
		return handled;
	}

	bool IgniteTrack::isEditing()
	{
		return AppMainmode::getMainModeInstance()->trackSwapComp->isVisible();
	}

	bool IgniteTrack::PannelKeyOptionsHandler(paramId_t param, uint32_t KeyValue)
	{
		bool setfocus = true;

		if (AppMainmode::getMainModeInstance()->trackSwapComp->isVisible())
			return AppMainmode::getMainModeInstance()->trackSwapComp->PannelKeyHandler(param, KeyValue);

		if (KeyFocusedItem == TrackOptionsItemID::NO_FOCUS)
		{
			/*1st key after Page visible */
			if (numOfTrackBanks > 1)
				KeyFocusedItem = TrackOptionsItemID::TRACK_BACK;
			else
				KeyFocusedItem = TrackOptionsItemID::TRACK_SWAP;
		}
		else
		{
			if (isEditing())
			{
				PannelSendKey(true, param, KeyValue);
			}
			else
			{
				PannelSendKey(false, param, KeyValue); // clear last Focus
				if (!option_handler_enabled)
					return true;
				switch ((FrontPannelParam)param)
				{
				case FrontPannelParam::paramKey_RMOVELEFT:
					KeyFocusedItem = (KeyFocusedItem + 1 >= TrackOptionsItemID::FOCUS_MAX ? (TrackOptionsItemID::FOCUS_MAX) : KeyFocusedItem + 1);
					break;
				case FrontPannelParam::paramKey_RMOVERIGHT:
					if (numOfTrackBanks > 1)
						KeyFocusedItem = (KeyFocusedItem - 1 <= TrackOptionsItemID::NO_FOCUS ? (TrackOptionsItemID::NO_FOCUS + 1) : KeyFocusedItem - 1);
					else
						KeyFocusedItem = (KeyFocusedItem - 1 <= TrackOptionsItemID::TRACK_FWD ? (TrackOptionsItemID::TRACK_FWD + 1) : KeyFocusedItem - 1);
					break;
				}
			}
			if (setfocus)
			{
				PannelSendKey(true, param, KeyValue);
			}
		}
		return true;
	}

	void IgniteTrack::PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		switch (TrackOptionsView[KeyFocusedItem])
		{
		case TRACK_FWD:
			fwdButton.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case TRACK_BACK:
			backButton.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case TRACK_SWAP:
			trackSwap.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case TRACK_CONFIG:
			trackConfig.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		default:
			break;
		}
	}
};
