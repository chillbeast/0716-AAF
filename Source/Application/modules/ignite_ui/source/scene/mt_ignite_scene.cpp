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

using namespace Ignite;

namespace ui
{
	////////////////////////////////////////////////
	//   Class: Ignite Scene
	////////////////////////////////////////////////
	IgniteScene::IgniteScene()
	{
		addNew.makeVisibleAndAddListner(this);
		addNewInList.makeVisibleAndAddListner(this);
		addNewInList.setBounds(Rectangle(xStartAddNewInList, yStartAddNewInList, widthAddNewInList, heightAddNewInList));
		addNew.setColour(juce::TextButton::buttonColourId, Theme::Instance().getColour(neutralGrey1));

		addNew.setBounds(Rectangle(xStartAddNewBtn, yStartAddNewBtn, widthAddNewBtn, heightAddNewBtn));
		addNew.setVisible(true);
		addNew.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		addNew.setColour(juce::TextEditor::outlineColourId, Theme::Instance().getColour(customBlue13));
		addNew.setColour(TextButton::ColourIds::textColourOffId, Theme::Instance().getColour(customBlue13));
		addNew.setColour(TextButton::ColourIds::buttonColourId, Theme::Instance().getColour(customdblue));
		addNewInList.setVisible(false);
		addNewInList.setToggleable(false);
		addNewInList.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());

		addAndMakeVisible(view);
		addAndMakeVisible(container);
		view.setViewedComponent(&container, true);
		view.setBounds(10, 62, widthScreen - 10, heightSongView);

		view.setScrollBarsShown(true, false, true, false);
		view.setScrollBarThickness(5);
		view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);
		container.setVisible(false);
		view.setVisible(false);

		_pEmptySceneComp = new class AppDnp();
		addChildComponent(_pEmptySceneComp);
		_pEmptySceneComp->setBounds(xStartEmptyComp, yStartEmptyComp, widthEmptyComp, heightEmptyComp);
		_pEmptySceneComp->setVisible(true);
		_pEmptySceneComp->setDisplayMessage(String(" "));
		_pEmptySceneComp->setBackgrounImage(juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("img-empty-state-scene.png")));

		setBufferedToImage(true);
		curretSceneRow = -1;
		_pSceneView = AppSceneView::getAppSceneViewInstance(this);
		addChildComponent(_pSceneView);
		_pSceneView->setBounds(xStartSongView1, yStartSongView1, widthSongView1, heightSongView1);

		addAndMakeVisible(sceneView);
		sceneView.addListener(this);
		sceneView.setToggleState(true, dontSendNotification);
		sceneView.setVisible(true);
		sceneView.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		sceneView.sethoverImages("btn-song-detail-default-hover.png", "btn-song-detail-active-hover.png", "btn-song-detail-default-hover.png");

		addAndMakeVisible(listView);
		listView.addListener(this);
		listView.setToggleState(false, dontSendNotification);
		listView.setVisible(true);
		listView.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		listView.sethoverImages("btn-song-list-default-hover.png", "btn-song-list-active-hover.png", "btn-song-list-default-hover.png");

		viewBox.items.add(FlexItem(sceneView).withMargin(1).withFlex(1));
		viewBox.items.add(FlexItem(listView).withMargin(1).withFlex(1));

		_pSongPtr = IgniteSong::getAppSongInstance();
		_pAppOptPtr = AppOptions::GetOptionsInstance();

		_pSceneOptCtl = IgniteControlOptions::GetAppOptInstance();
		_pSceneOptCtl->setBounds(xStartOptctl, yStartOptctl, widthOptctl, heightOptctlt);
		_pSceneOptCtl->addOptCtlListener(this);

		_pSceneLoopComp = new IgniteLoopControl(&sceneList);
		addChildComponent(_pSceneLoopComp);
		_pSceneLoopComp->setBounds(129, 64, 766, 350);
		_pSceneLoopComp->setVisible(false);
		_pSceneLoopComp->setTitle("Scene Loop");
		_pSceneBarRef = SceneStatusBar::GetSceneBarInstance();
		refreshSceneUI();
		_pPopUpPtr = CommonPopUp::getCommonPopupInstance();
		getAssets();

		_pSceneLoopComp->onButtonPressed = [this](IgniteLoopControl::loopKey key, int loopStartIndex, int loopEndIndex)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;
				setLoopControl(key, loopStartIndex - 1, loopEndIndex - 1);
			};

		_pSceneCopyComp = new IgniteCopy();
		addChildComponent(_pSceneCopyComp);
		_pSceneCopyComp->setBounds(xStartLoopCmp, yStartLoopCmp, widthCopy, heightCopy);
		_pSceneCopyComp->setVisible(false);

		_pSceneCopyComp->onButtonPressed = [this](IgniteCopy::copyCtlKey key, String copyName)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;

				copyScene(key, copyName);
			};
	}

	IgniteScene::~IgniteScene()
	{
		cleanup();
	}

	void IgniteScene::copyScene(IgniteCopy::copyCtlKey key, String copyName)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " New Name : " + copyName);

		if (copyName.isNotEmpty())
		{
			if (key == IgniteCopy::copyCtlKey::COPY)
			{
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneCopy, copyName.toStdString());
				_pSongPtr->updateDataFromEngine();
			}
		}
		_pSceneCopyComp->exitModalState(0);
		_pSceneCopyComp->setVisible(false);
	}


	int IgniteScene::getSelected(void)
	{
		return _pSongPtr->getSongData(mt::Ignite::songsDataIndex::SELECTED_SCENE);
	}

	void IgniteScene::setLoopControl(IgniteLoopControl::loopKey key, int loopStartIndex, int loopEndIndex)
	{
		auto selected_index = loopStartIndex;
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " loopStartIndex : " + String(loopStartIndex) + " loopEndIndex : " + String(loopEndIndex));

		_pRowPtr = _pSongPtr->getSceneList();
		if (_pRowPtr == NULL)
			return;

		/* Clear the exitsing loop */
		for (auto& i : *_pRowPtr)
		{
			if (songIndex == i->getSongIndex())
				i->setloop(false);
		}
		if (loopStartIndex > loopEndIndex)
		{
			selected_index = loopStartIndex;
			loopStartIndex = loopStartIndex ^ loopEndIndex;
			loopEndIndex = loopStartIndex ^ loopEndIndex;
			loopStartIndex = loopStartIndex ^ loopEndIndex;
		}

		if ((key == IgniteLoopControl::loopKey::APPLY) || (key == IgniteLoopControl::loopKey::CANCEL))
		{
			int index = 0;
			for (auto& i : *_pRowPtr)
			{
				if (songIndex == i->getSongIndex())
				{
					if ((index >= loopStartIndex) && (index <= loopEndIndex))
						i->setloop(true);
					index++;
				}
			}
		}
		else if (key == IgniteLoopControl::loopKey::CLEAR)
		{
			loopStartIndex = loopEndIndex = -1;
			for (auto& i : *_pRowPtr)
			{
				if (songIndex == i->getSongIndex())
					i->setloop(false);
			}
		}
		_pSongPtr->setSongData(mt::Ignite::songsDataIndex::LOOP_SCENE_START, loopStartIndex, false);
		_pSongPtr->setSongData(mt::Ignite::songsDataIndex::LOOP_SCENE_END, loopEndIndex, false);
		_pSongPtr->setSongData(mt::Ignite::songsDataIndex::CURRENT_SCENE, loopStartIndex, true);
		_pSongPtr->setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, loopStartIndex + 1, true);

		setSelected(selected_index);
		_pSceneLoopComp->exitModalState(0);
		_pSceneLoopComp->setVisible(false);
	}

	void IgniteScene::updateTracksData()
	{
		auto index = getSelected();
		std::vector<IgniteParamList> sceneParams;
		IgniteParamList paramsList;
		auto activeTracks = 0;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		_pSongPtr->setSongData(mt::Ignite::songsDataIndex::SELECTED_SCENE, index, false);
		mt::Ignite::sceneData* scDataPtr = _pSongPtr->getSceneData(index);
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, index);

		sceneParams.clear();
		paramsList.param = (paramId_t)SceneParam::paramSceneTrackMuteStatus;
		sceneParams.push_back(paramsList);
		sceneParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, sceneParams);

		if (sceneParams.size() > 0)
		{
			for (auto i = 0; i < (scDataPtr->num_of_track_banks * 16); i++)
			{
				scDataPtr->track_data[i].trackState = (sceneParams[0].MsgValue.get<std::uint64_t>() >> i) & 0x1;
				if (!scDataPtr->track_data[i].trackState)
					activeTracks++;
			}
			scDataPtr->activeTracks = activeTracks;
			IgniteTrack::GetAppTrackInstance()->updateDataFromEngine();
		}
	}

	void IgniteScene::setSelected(int index)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		_pRowPtr = (juce::OwnedArray <ui::IgniteSceneRow>*)songDataPtr->list;
		auto opMode = _pSceneOptCtl->getOperationMode();

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " Scene index: " + String(index));

		if (index != songDataPtr->selectedScene)
			setSceneData(mt::Ignite::sceneDataIndex::TRACK_DATA_REVALIDATE, true, true);


		if ((opMode != mt::Ignite::operatioMode::PLAY_MODE) || (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE))
		{
			if (isPositiveAndBelow(index, _pRowPtr->size()))
			{
				if (index != songDataPtr->selectedScene)
				{
					enableControlButton(IgniteControlOptions::ALL, false);
					_pSongPtr->setSongData(mt::Ignite::songsDataIndex::SELECTED_SCENE, index, false);
				}
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, index);
				requestDataFromEngine(songDataPtr->selectedScene);
			}

			// Highlist selected row.
			for (auto& i : *_pRowPtr)
				i->highlight(false);

			(*_pRowPtr)[songDataPtr->selectedScene]->highlight(true);

			const auto rowPosition = index * ROW_HEIGHT;
			const auto viewHeight = view.getViewArea().getHeight();
			const auto viewPosY = view.getViewPositionY();
			const auto maxVisiblePosition = viewPosY + viewHeight - ROW_HEIGHT;

			if (rowPosition + ROW_HEIGHT > maxVisiblePosition)
				view.setViewPosition(view.getViewPositionX(), maxVisiblePosition);
			else if (rowPosition <= viewPosY)
				view.setViewPosition(view.getViewPositionX(), viewPosY - viewHeight + ROW_HEIGHT);

			// Refresh track UI if scene was created
			if (songDataPtr->sceneCreated)
				IgniteTrack::GetAppTrackInstance()->refreshTrackUI();

			// Update scene data
			auto* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];
			statusBarData.swing = scDataPtr->swing;
			statusBarData.tempo = scDataPtr->tempo;
			statusBarData.sceneLength = scDataPtr->length;
			statusBarData.editSceneIndex = songDataPtr->selectedScene + 1;

			SceneStatusBar::GetSceneBarInstance()->onNumofBarchangedinScene(scDataPtr->length);
			_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
		}
		else
		{
			// TODO: Handle next scene change
			//_pSongPtr->setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, index + 1, false);
		}
		if (songDataPtr->sceneCreated)
			IgniteTrack::GetAppTrackInstance()->refreshTrackUI();
		if (this->isVisible())
		{
			if (_pRowPtr->size() > 1)
				enableControlButton(IgniteControlOptions::LOOP, true);

			if (songDataPtr->sceneCreated && ((opMode != mt::Ignite::operatioMode::PLAY_MODE) || (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE)))
			{
				if (!getViewStatus())
				{
					enableControlButton(IgniteControlOptions::DEL, true);
					enableControlButton(IgniteControlOptions::COPY, true);
					if (_pRowPtr->size() > 1)
					{
						enableControlButton(IgniteControlOptions::MOVE, true);
					}
				}
			}
			bool enableSaveUndo = (sceneDataUpdated && !IgniteSceneConfig::getAppSceneConfigInstance()->isAutoSaveEnabled());
			enableControlButton(IgniteControlOptions::SAVE, enableSaveUndo);
			enableControlButton(IgniteControlOptions::UNDO, enableSaveUndo);
		}
		setFpSceneLedState();
		IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
	}

	void IgniteScene::setCurrentPlayingScene(int index)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " New Index: " + String(index));

		auto songDataPtr = _pSongPtr->getSongDataPtr();
		_pRowPtr = (juce::OwnedArray <ui::IgniteSceneRow>*)songDataPtr->list;

		////_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
		//_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());
		//sceneDataUpdated = false;
		//updateLables();

		// Highlist selected row.
		for (auto& i : *_pRowPtr)
			i->highlight(false);

		(*_pRowPtr)[index]->highlight(true);
		requestDataFromEngine(index);
		if (songDataPtr->sceneCreated)
			IgniteTrack::GetAppTrackInstance()->refreshTrackUI();

		_pSceneView->refreshData(index, &songDataPtr->scene_data[index]);
	}

	void IgniteScene::cleanup()
	{
		container.removeAllChildren();
	}

	String IgniteScene::getSceneName()
	{
		std::vector<IgniteParamList> sceneNameList;
		IgniteParamList params;

		sceneNameList.clear();
		params.param = (paramId_t)SceneParam::paramSceneNameList;
		sceneNameList.push_back(params);
		sceneNameList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, sceneNameList);

		if (sceneNameList.size())
			return sceneNameList[sceneNameList.size() - 1].MsgValue.get<std::string>();
		else
			return "";
	}

	void IgniteScene::populate(mt::Ignite::sceneData* scDataPtr, bool isSceneUpdateNeeded, String def_name)
	{
		IgniteSceneRow* row;
		auto songDataPtr = _pSongPtr->getSongDataPtr();

		if (def_name == "")
		{
			String name = getSceneName();

			if (name != "")
				scDataPtr->name = getSceneName();
		}
		else
			scDataPtr->name = def_name;

		row = new IgniteSceneRow(scDataPtr->serialNum, *scDataPtr, this);
		row->setSongIndex(_pSongPtr->getSelected());
		if ((juce::OwnedArray <ui::IgniteSceneRow>*)songDataPtr->list == NULL)
		{
			_pRowPtr = new juce::OwnedArray<ui::IgniteSceneRow>;
			_pSongPtr->addToSceneList(_pRowPtr);
		}
		_pRowPtr = (juce::OwnedArray <ui::IgniteSceneRow>*)songDataPtr->list;
		_pRowPtr->add(row);
		curretSceneRow++;
		container.addAndMakeVisible(row);

		if (isSceneUpdateNeeded)
		{
			refreshSceneUI();
		}
	}

	void IgniteScene::refreshSceneUI()
	{
		int index = 0;

		if (this->isVisible())
		{
			cleanup();
			auto songDataPtr = _pSongPtr->getSongDataPtr();

			if (statusBarData.isSceneCreated)
			{
				songIndex = _pSongPtr->getSelected();
				_pRowPtr = (juce::OwnedArray <ui::IgniteSceneRow>*)songDataPtr->list;
				if (_pRowPtr != NULL)
				{
					if (addNew.isVisible())
						addNew.setVisible(false);

					for (auto& i : *_pRowPtr)
					{
						if (songIndex == i->getSongIndex())
						{
							i->setBounds(0, ((heightSongList + 8) * index++), widthSongList, heightSongList);

							i->onMouseDown = [this](IgniteSceneRow* Row)
								{
									showPopup((Row->getSerialNumber()));
								};
							container.addAndMakeVisible(i);
						}
					}
					_pAppOptPtr->showOptions(true);
					_pEmptySceneComp->setVisible(false);
					_pSceneView->clearTapTempoData();
				}
			}
			else
			{
				addNew.setVisible(true);
				addNewInList.setVisible(false);
				_pEmptySceneComp->setVisible(true);
				_pSceneView->setVisible(false);
				_pAppOptPtr->showOptions(false);
				sceneView.setVisible(false);
				listView.setVisible(false);
			}

			// Set overall view container size....
			container.setSize(widthScreen - 20, ((index + 1) * (heightSongContainer + 8)));

			if (statusBarData.isSceneCreated && (_pRowPtr != NULL))
			{
				setSelected(songDataPtr->selectedScene);
				AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::TRACK, true);
			}
		}
	}

	void IgniteScene::setDefaultSceneData(mt::Ignite::sceneData* scDataPtr, bool updateDefultData)
	{
		scDataPtr->swing = (uint8_t)defSwing;
		scDataPtr->tempo = (float)defTempo;
		scDataPtr->ftsRootNoteIndex = 0;
		scDataPtr->ftsScaleIndex = 1;
		scDataPtr->muteHoldIndex = 0;
		scDataPtr->gbar = defGbar;
		scDataPtr->length = defLength;
		scDataPtr->xpose = defXpose;
		scDataPtr->songadv = 0;
		scDataPtr->xfts = false;
		(_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX)) ? scDataPtr->num_of_track_banks = (int)_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX) : scDataPtr->num_of_track_banks = 1;
		scDataPtr->current_track = 0;
		scDataPtr->current_track_bank = 0;
		scDataPtr->activeTracks = 0;
		_pRowPtr = _pSongPtr->getSceneList();
	}

	void IgniteScene::requestDataFromEngine(int index)
	{
		std::vector<IgniteParamList> sceneParams;
		IgniteParamList paramsList;
		auto activeTracks = 0;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " Scene index: " + String(index));

		_pSongPtr->setSongData(mt::Ignite::songsDataIndex::SELECTED_SCENE, index, false);
		mt::Ignite::sceneData* scDataPtr = _pSongPtr->getSceneData(getSelected());
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, index);

		sceneParams.clear();
		paramsList.param = (paramId_t)SceneParam::paramSceneName;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneTempo;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneSwing;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneAdvanceMode;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneGBar;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneFTSRootNote;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneFTSScale;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneLength;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneTransposeValue;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneMuteHold;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneTransposeFTSPriority;
		sceneParams.push_back(paramsList);
		paramsList.param = (paramId_t)SceneParam::paramSceneTrackMuteStatus;
		sceneParams.push_back(paramsList);
		sceneParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, sceneParams);

		for (auto& params : sceneParams)
		{
			switch (params.param)
			{
			case (paramId_t)SceneParam::paramSceneName:
				scDataPtr->name = params.MsgValue.get< std::string >();
				break;
			case (paramId_t)SceneParam::paramSceneTempo:
				scDataPtr->tempo = statusBarData.tempo = params.MsgValue.get< std::float_t >();
				break;
			case (paramId_t)SceneParam::paramSceneSwing:
				scDataPtr->swing = statusBarData.swing = params.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)SceneParam::paramSceneAdvanceMode:
				scDataPtr->songadv = statusBarData.songadv = params.MsgValue.get< std::int8_t >();
				/*if (IgniteControlOptions::GetAppOptInstance()->getOperationMode() == mt::Ignite::operatioMode::PLAY_MODE)
				{
					if (scDataPtr->songadv == 1)
					{
						_pSongPtr->setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, getSelected(), true);
						_pSongPtr->setSongData(mt::Ignite::songsDataIndex::CURRENT_SCENE, getSelected(), true);
					}
					else
					{
						if ((int)_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES) > getSelected() + 1)
						{
							_pSongPtr->setSongData(mt::Ignite::songsDataIndex::CURRENT_SCENE, getSelected(), true);
							_pSongPtr->setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, getSelected() + 1, true);
						}
						else
						{
							_pSongPtr->setSongData(mt::Ignite::songsDataIndex::CURRENT_SCENE, getSelected(), true);
							_pSongPtr->setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, 0, true);
						}
					}
				}*/
				break;
			case (paramId_t)SceneParam::paramSceneGBar:
				scDataPtr->gbar = params.MsgValue.get< std::int8_t >();
				break;
			case (paramId_t)SceneParam::paramSceneFTSRootNote:
				scDataPtr->ftsRootNoteIndex = params.MsgValue.get< std::int8_t >();
				break;
			case (paramId_t)SceneParam::paramSceneFTSScale:
				scDataPtr->ftsScaleIndex = params.MsgValue.get< std::int8_t >();
				break;
			case (paramId_t)SceneParam::paramSceneLength:
				scDataPtr->length =  static_cast<int>(params.MsgValue.get< std::int16_t >());
				statusBarData.sceneLength = scDataPtr->length;
				break;
			case (paramId_t)SceneParam::paramSceneTransposeValue:
				scDataPtr->xpose = (int)params.MsgValue.get< std::int8_t >();
				break;
			case (paramId_t)SceneParam::paramSceneMuteHold:
				scDataPtr->muteHoldIndex = params.MsgValue.get< std::int8_t >();
				break;
			case (paramId_t)SceneParam::paramSceneTransposeFTSPriority:
				scDataPtr->xfts = params.MsgValue.get< std::int8_t >();
				break;
			case (paramId_t)SceneParam::paramSceneTrackMuteStatus:
				const int totalTracks = scDataPtr->num_of_track_banks * 16;
				std::uint64_t msgValue = params.MsgValue.get<std::uint64_t>();

				for (auto i = 0; i < totalTracks; i++)
				{
					bool trackState = (msgValue >> i) & 0x1;
					scDataPtr->track_data[i].trackState = trackState;
					if (!trackState)
						activeTracks++;
				}
				scDataPtr->activeTracks = activeTracks;
				/*auto opMode = _pSceneOptCtl->getOperationMode();
				if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE) || (IgniteTrack::GetAppTrackInstance()->isVisible()))*/
					IgniteTrack::GetAppTrackInstance()->updateDataFromEngine();
				break;
			}
		}
	}

	void IgniteScene::addScene(bool isSceneUpdateNeeded, String name)
	{
		mt::Ignite::sceneData scData;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "Scene Name: " + name + " isSceneUpdateNeeded " + String((int)isSceneUpdateNeeded));

		setDefaultSceneData(&scData, isSceneUpdateNeeded);
		scData.name = name;
		IgniteTrack::GetAppTrackInstance()->addTracks(scData);
		auto data = _pSongPtr->addSceneData(scData);
		if (!addNewInList.isVisible())
		{
			_pSceneView->setVisible(true);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::SCENE);
		}
		else
			listView.setToggleState(true, dontSendNotification);

		addNew.setVisible(false);
		populate(data, isSceneUpdateNeeded, name);
		_pEmptySceneComp->setVisible(false);
		_pAppOptPtr->showOptions(true);
		listView.setVisible(true);
		sceneView.setVisible(true);
		enableViewBox(true);
		setFpSceneLedState();
	}

	void IgniteScene::enableViewBox(bool state)
	{
		if (state)
			viewBox.performLayout(ui::Rectangle(xStartViewBtn, yStartViewBtn, widthViewBtn, heightViewBtn));
		else
			viewBox.performLayout(ui::Rectangle(0, 0, 0, 0));
	}

	void IgniteScene::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (((buttonThatWasClicked == &addNew) || (buttonThatWasClicked == &addNewInList)) && (_pSongPtr->isSongCreated()))
		{
			auto tmpSelected = getSelected();

			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramAddSceneCmd, 0);
			//	_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, _pSongPtr->getSelected());
			if (IgniteSceneConfig::getAppSceneConfigInstance()->getSceneInsertPosition() == mt::Ignite::sceneInsertPosition[0])
			{
				addScene(true, getSceneName());
			}
			else if (IgniteSceneConfig::getAppSceneConfigInstance()->getSceneInsertPosition() == mt::Ignite::sceneInsertPosition[1])
			{
				addScene(true, getSceneName());
				setSelected((int)_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES) - 1);
				moveScene(tmpSelected + 1);
				setSelected(tmpSelected);
			}

			if (addNewInList.getToggleState())
				addNewInList.setToggleState(false, dontSendNotification);
		}
		else if (buttonThatWasClicked == &listView)
		{
			view.setVisible(true);
			changeSceneOptionsVisibility(false);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::SCENE);
		}
		else if (buttonThatWasClicked == &sceneView)
		{
			view.setVisible(false);
			changeSceneOptionsVisibility(true);
			//AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::SCENE);
		}
	}

	void IgniteScene::updateSceneEditIndication(bool state)
	{
		auto opMode = _pSceneOptCtl->getOperationMode();
		if ((opMode != mt::Ignite::operatioMode::PLAY_MODE) || (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE))
		{
			sceneDataUpdated = state;
			enableControlButton(IgniteControlOptions::SAVE, state);
			enableControlButton(IgniteControlOptions::UNDO, state);
			(*_pSongPtr->getSceneList())[getSelected()]->updateEditIndication();
		}
		else
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());
	}

	void IgniteScene::showPopup(int index)
	{
		if (index == (int)_pSongPtr->getSongData(mt::Ignite::songsDataIndex::SELECTED_SCENE) || (!sceneDataUpdated))
		{
			setSelected(index);
			currentSceneBank = index / 16;
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
			setFpSceneLedState();
			return;
		}
		prevSelected = index;
		if (_pPopUpPtr->isDialogOpen.get() != true)
		{
			_pPopUpPtr->close();
			_pPopUpPtr->setPopUpPropories("Switch Scene", 604, 290);
			_pPopUpPtr->setbtnsize(184, 44);
			_pPopUpPtr->addImageButton("Cancel", "btn-cancel-174x44-default-hover.png", "btn-cancel-174x44-default.png", 3);
			_pPopUpPtr->addImageButton("Confirm Switch", "btn-confirmswitch-174x44-default.png", "btn-confirmswitch-174x44-default-hover.png", 1);
			_pPopUpPtr->addImageButton("Save and Switch", "btn-saveswitch-174x44-default.png", "btn-saveswitch-174x44-default-hover.png", 2);
			_pPopUpPtr->popupType = POPUP_CHANGEWARNING;
			_pPopUpPtr->isCloseButtonVisible(false);
			_pPopUpPtr->show();
			_pPopUpPtr->setInfoText(String(getSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME).toString()) + " active scene unsaved data will be lost");
			_pPopUpPtr->onSelectionChanged = [this](int r)
				{
					if (r == 1)
					{
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramClearUndoList, getSelected());
						sceneDataUpdated = false;
						updateLables();
						setSelected(prevSelected);
						currentSceneBank = prevSelected / 16;
						AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
						setFpSceneLedState();
					}
					if (r == 2)
					{
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());
						sceneDataUpdated = false;
						updateLables();
						setSelected(prevSelected);
						currentSceneBank = prevSelected / 16;
						AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
						setFpSceneLedState();
					}

					if (r == 3)
						sceneRowHighLightIndex = prevSelected - 1;

					if (auto dw = getCurrentlyModalComponent())
						dw->exitModalState(0);
				};
		}


	}

	void IgniteScene::getAssets()
	{
		imageAddNewInlistDefault= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newscene-default.png"));
		imageAddNewInlistActive= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newscene-active.png"));
		imageAddNewInlistHover= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newscene-hover.png"));

		imageSceneDetailViewDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-default.png"));
		imageSceneDetailViewActive = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-active.png"));
		imageSceneDetailViewHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-default-hover.png"));

		imageSceneListViewDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-default.png"));
		imageSceneListViewActive = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-active.png"));
		imageSceneListViewHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-default-hover.png"));
		
		imageAddNewDefault	= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newscene-default.png"));
		imageAddNewActive	= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newscene-active.png"));
		imageAddNewHover	= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newscene-hover.png"));
		addNewInList.setImage(imageAddNewInlistDefault, imageAddNewInlistActive, imageAddNewInlistHover);
		sceneView.setImage(imageSceneDetailViewDefault, imageSceneDetailViewActive, imageSceneDetailViewHover);
		listView.setImage(imageSceneListViewDefault, imageSceneListViewActive, imageSceneListViewHover);

	}
	void IgniteScene::paint(juce::Graphics& g)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();

		auto opMode = _pSceneOptCtl->getOperationMode();
		if ((songDataPtr->sceneCreated && ((opMode != mt::Ignite::operatioMode::PLAY_MODE) || (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE))))
			_pSceneView->refreshData(songDataPtr->selectedScene, &songDataPtr->scene_data[songDataPtr->selectedScene]);

		if (addNew.isVisible())
		{
			g.drawImage(addNew.getToggleState() ? imageAddNewActive : addNew.isMouseOver() ? imageAddNewHover : imageAddNewDefault, ui::Rectangle<float>{ xStartAddNewBtn, yStartAddNewBtn, widthAddNewBtn, heightAddNewBtn }, RectanglePlacement::centred);
		}
		if (addNewInList.isVisible())
		{
			if (addNewInList.getToggleState())
				addNewInList.setToggleState(false, dontSendNotification);
		}
	}

	void IgniteScene::resized()
	{
	}

	int IgniteScene::getViewStatus()
	{
		if (sceneView.getToggleState())
			return 1;
		else if (listView.getToggleState())
			return 0;
		else
			return -1;
	}

	void IgniteScene::changeSceneOptionsVisibility(bool bShouldShow)
	{
		bool sceneCreated = statusBarData.isSceneCreated;
		mainmodePtr = AppMainmode::getMainModeInstance();

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " bShouldShow " + String((int)bShouldShow));

		if (sceneCreated) {
			listView.setToggleState(false, dontSendNotification);
			sceneView.setToggleState(false, dontSendNotification);
		}

		listView.setVisible(sceneCreated);
		sceneView.setVisible(sceneCreated);
		enableViewBox(sceneCreated);


		if (!bShouldShow)
		{
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Scene View:  List View");

			optionHandlerEnabled = SCENE_LIST;
			_pRowPtr = _pSongPtr->getSceneList();
			view.setVisible(true);
			container.setVisible(true);
			if (!addNew.isVisible())
				addNewInList.setVisible(true);
			_pSceneView->setVisible(false);
			listView.setToggleState(true, dontSendNotification);
			enableViewBox(true);
			mainmodePtr->resetPadLedsOnViewChange();
			mainmodePtr->setFpEditTrackLedState();
			setFpSceneLedState();
			if (isVisible())
				_pAppOptPtr->taboptionChanges(mt::Ignite::tabs::SCENE, getViewStatus());

			if (_pRowPtr != NULL)
			{
				for (auto eachRow : *_pRowPtr)
				{
					eachRow->setVisible(!bShouldShow);
				}
			}
			refreshSceneUI();
		}
		else
		{
			optionHandlerEnabled = SCENE_DETAIL;
			if (sceneCreated)
			{
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Scene View:  Detailed View");

				_pSceneView->setVisible(true);
				addNewInList.setVisible(false);
				view.setVisible(false);
				container.setVisible(false);
				sceneView.setToggleState(true, dontSendNotification);
				enableViewBox(true);
				moveEnabled = false;
			}
			else
			{
				_pEmptySceneComp->setVisible(true);
				addNew.setVisible(true);
				addNewInList.setVisible(false);
				if (isVisible())
					_pAppOptPtr->taboptionChanges(mt::Ignite::tabs::SCENE, getViewStatus());
			}
			enableControlButton(IgniteControlOptions::UNDO, false);
			refreshSceneUI();
			mainmodePtr->resetPadLedsOnViewChange();
			mainmodePtr->setFpFillAndSoloLedState();
			mainmodePtr->setFpEditTrackLedState();
		}
		auto opMode = _pSceneOptCtl->getOperationMode();
		if (bShouldShow && ((opMode != mt::Ignite::operatioMode::PLAY_MODE) || (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE)))
		{
			enableControlButton(IgniteControlOptions::DEL, false);
			enableControlButton(IgniteControlOptions::COPY, false);
			enableControlButton(IgniteControlOptions::MOVE, false);
		}
	}

	void IgniteScene::updateSceneData(uint8_t param, uint8_t value)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();

		if (songDataPtr == nullptr || !songDataPtr->sceneCreated ||
            !juce::isPositiveAndBelow(songDataPtr->selectedScene, songDataPtr->scene_data.size()))
			return;

		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " param: " + String(param) + " value: " + String(value));

		switch (param)
		{
		case (uint8_t)SceneParam::paramSceneSwing:
			scDataPtr->swing = statusBarData.swing = value;
			break;
		case (uint8_t)SceneParam::paramSceneBarId:
			_pSceneBarRef->setActiveSceneBar(value);
			break;
		}
		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::updateSceneData(uint8_t param, int8_t value)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();

		if (songDataPtr == nullptr || !songDataPtr->sceneCreated ||
            !juce::isPositiveAndBelow(songDataPtr->selectedScene, songDataPtr->scene_data.size()))
			return;

		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " param: " + String(param) + " value: " + String(value));

		switch (param)
		{
		case (uint8_t)SceneParam::paramSceneAdvanceMode:
			scDataPtr->songadv = statusBarData.songadv = value;
			break;
		case (uint8_t)SceneParam::paramSceneGBar:
			scDataPtr->gbar = value;
			break;
		case  (uint8_t)SceneParam::paramSceneFTSRootNote:
			scDataPtr->ftsRootNoteIndex = value;
			break;
		case  (uint8_t)SceneParam::paramSceneFTSScale:
			scDataPtr->ftsScaleIndex = value;
			break;
		case  (uint8_t)SceneParam::paramSceneTransposeValue:
			scDataPtr->xpose = value;
			break;
		case  (uint8_t)SceneParam::paramSceneMuteHold:
			scDataPtr->muteHoldIndex = value;
			break;
		case  (uint8_t)SceneParam::paramSceneTransposeFTSPriority:
			scDataPtr->xfts = value;
			break;
		case (uint8_t)SceneParam::paramSceneBarId:
			_pSceneBarRef->setActiveSceneBar(value);
			break;
		}
		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::updateSceneData(uint8_t param, int16_t value)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		if (songDataPtr == nullptr || !songDataPtr->sceneCreated ||
            !juce::isPositiveAndBelow(songDataPtr->selectedScene, songDataPtr->scene_data.size()))
			return;

		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " param: " + String(param) + " value: " + String(value));

		switch (param)
		{
		case (uint8_t)SceneParam::paramSceneLength:
			scDataPtr->length = value;
			statusBarData.sceneLength = static_cast<int>(scDataPtr->length);
			break;
		}
		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::updateSceneData(uint8_t param, const uint32_t value)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		if (songDataPtr == nullptr || !songDataPtr->sceneCreated ||
            !juce::isPositiveAndBelow(songDataPtr->selectedScene, songDataPtr->scene_data.size()))
			return;
	}

	void IgniteScene::updateSceneData(uint8_t param, const float value)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		if (songDataPtr == nullptr || !songDataPtr->sceneCreated ||
            !juce::isPositiveAndBelow(songDataPtr->selectedScene, songDataPtr->scene_data.size()))
			return;

		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " param: " + String(param) + " value: " + String(value));

		switch (param)
		{
		case (uint8_t)SceneParam::paramSceneTempo:
			scDataPtr->tempo = statusBarData.tempo = value;
			break;
		}
		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::updateSceneData(uint8_t param, const std::string& value)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		if (songDataPtr == nullptr || !songDataPtr->sceneCreated ||
            !juce::isPositiveAndBelow(songDataPtr->selectedScene, songDataPtr->scene_data.size()))
			return;

		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " param: " + String(param) + " value: " + String(value));

		switch (param)
		{
		case (uint8_t)SceneParam::paramSceneName:
			scDataPtr->name = value;
			break;
		}
		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::setSceneData(mt::Ignite::sceneDataIndex item_id, int8_t data, bool is_internal)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
		switch (item_id)
		{
		case mt::Ignite::sceneDataIndex::XPOSE:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneTransposeValue, data);
			scDataPtr->xpose = data;
			break;
		}

		if (IgniteSceneConfig::getAppSceneConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());

		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::setSceneData(mt::Ignite::sceneDataIndex item_id, uint8_t data, bool is_internal)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
		switch (item_id)
		{
		case mt::Ignite::sceneDataIndex::SCENE_SWING:
			scDataPtr->swing = statusBarData.swing = data;
			IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneSwing, data);
			break;
		case mt::Ignite::sceneDataIndex::SONGADV:
			scDataPtr->songadv = statusBarData.songadv = data;
			IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneAdvanceMode, (int8_t)data);
			break;
		case mt::Ignite::sceneDataIndex::GBAR:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneGBar, (int8_t)data);
			scDataPtr->gbar = data;
			break;
		case mt::Ignite::sceneDataIndex::FTSROOTNOTE:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneFTSRootNote, (int8_t)data);
			scDataPtr->ftsRootNoteIndex = data;
			break;
		case mt::Ignite::sceneDataIndex::FTSSCALE:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneFTSScale, (int8_t)data);
			scDataPtr->ftsScaleIndex = data;
			break;
		case mt::Ignite::sceneDataIndex::MUTEHOLD:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneMuteHold, (int8_t)data);
			scDataPtr->muteHoldIndex = data;
			break;
		case mt::Ignite::sceneDataIndex::XFTS:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneTransposeFTSPriority, (int8_t)data);
			scDataPtr->xfts = data;
			break;
		default: break;
		}

		if (IgniteSceneConfig::getAppSceneConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());

		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::setSceneData(mt::Ignite::sceneDataIndex item_id, int data, bool is_internal)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
		switch (item_id)
		{
		case mt::Ignite::sceneDataIndex::LENGHT:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneLength, (int16_t)data);
			statusBarData.sceneLength = data;
			scDataPtr->length = static_cast<int16_t>(statusBarData.sceneLength);
			IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
			break;
		case mt::Ignite::sceneDataIndex::XPOSE:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneTransposeValue, (int8_t)data);
			scDataPtr->xpose = (int8_t)data;
			break;
		case mt::Ignite::sceneDataIndex::CURRENT_TRACK:
			scDataPtr->current_track = statusBarData.currentTrack = data;
			IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
			if(!is_internal)
				IgniteSong::getAppSongInstance()->isSongSetupComplete();
			break;
		case mt::Ignite::sceneDataIndex::CURRENT_TRACK_BANK:
			scDataPtr->current_track_bank = data;
			break;
		case mt::Ignite::sceneDataIndex::NUM_TRACK_BANK_INDEX:
			scDataPtr->num_of_track_banks = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramTrackCountCmd, (uint8_t)((data + 1) * 16));
			break;
		case mt::Ignite::sceneDataIndex::TRACKS_ACTIVE:
			data ? (scDataPtr->activeTracks += 1) : (scDataPtr->activeTracks -= 1);
			break;
		case mt::Ignite::sceneDataIndex::TRACK_DATA_REVALIDATE:
			scDataPtr->trackDataLoded = data;
			break;
		default: break;
		}

		if (IgniteSceneConfig::getAppSceneConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());

		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::setSceneData(mt::Ignite::sceneDataIndex item_id, float data, bool is_internal, bool isPlayModeData)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal) + " isPlayModeData: " + String((int)isPlayModeData));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
		switch (item_id)
		{
		case mt::Ignite::sceneDataIndex::SCENE_TEMPO:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneTempo, data, isPlayModeData);
			scDataPtr->tempo = statusBarData.tempo = data;
			IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
			break;
		default: break;
		}

		if (IgniteSceneConfig::getAppSceneConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());

		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	void IgniteScene::setSceneData(mt::Ignite::sceneDataIndex item_id, String data, bool is_internal)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
		switch (item_id)
		{
		case mt::Ignite::sceneDataIndex::SCENE_NAME:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneName, data.toStdString());
			scDataPtr->name = data;
			break;
		default: break;
		}

		if (IgniteSceneConfig::getAppSceneConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());

		_pRowPtr = (juce::OwnedArray <ui::IgniteSceneRow>*)songDataPtr->list;
		(*_pRowPtr)[songDataPtr->selectedScene]->refreshSceneRowUI();
		_pSceneView->refreshData(songDataPtr->selectedScene, scDataPtr);
	}

	var IgniteScene::getSceneData(mt::Ignite::sceneDataIndex item_id)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		auto* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];

		if (songDataPtr->num_scenes > 0)
		{
			switch (item_id)
			{
			case mt::Ignite::sceneDataIndex::SCENE_SWING:
				return scDataPtr->swing;
			case mt::Ignite::sceneDataIndex::SCENE_TEMPO:
				return scDataPtr->tempo;
			case mt::Ignite::sceneDataIndex::SONGADV:
				return scDataPtr->songadv;
			case mt::Ignite::sceneDataIndex::FTSROOTNOTE:
				return scDataPtr->ftsRootNoteIndex;
			case mt::Ignite::sceneDataIndex::FTSSCALE:
				return scDataPtr->ftsScaleIndex;
			case mt::Ignite::sceneDataIndex::XFTS:
				return scDataPtr->xfts;
			case mt::Ignite::sceneDataIndex::MUTEHOLD:
				return scDataPtr->muteHoldIndex;
			case mt::Ignite::sceneDataIndex::SCENE_NAME:
				return scDataPtr->name;
			case mt::Ignite::sceneDataIndex::GBAR:
				return scDataPtr->gbar;
			case mt::Ignite::sceneDataIndex::LENGHT:
				return scDataPtr->length;
			case mt::Ignite::sceneDataIndex::XPOSE:
				return scDataPtr->xpose;
			case mt::Ignite::sceneDataIndex::NUM_TRACK_BANK_INDEX:
				return scDataPtr->num_of_track_banks;
			case mt::Ignite::sceneDataIndex::CURRENT_TRACK:
				return scDataPtr->current_track;
			case mt::Ignite::sceneDataIndex::CURRENT_TRACK_BANK:
				return scDataPtr->current_track_bank;
			case mt::Ignite::sceneDataIndex::TRACKS_ACTIVE:
				return scDataPtr->activeTracks;
			case mt::Ignite::sceneDataIndex::TRACK_DATA_REVALIDATE:
				return scDataPtr->trackDataLoded;
			default: return -1;
			}
		}
		return -1;
	}

	mt::Ignite::trackData* IgniteScene::getTrackDataRef(int current_track)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		if (songDataPtr->sceneCreated)
		{
			mt::Ignite::sceneData* scDataPtr = &songDataPtr->scene_data[songDataPtr->selectedScene];
			if (current_track == -1)
				return &(scDataPtr->track_data[scDataPtr->current_track]);
			else
				return &(scDataPtr->track_data[current_track]);
		}
		return nullptr;
	}

	String IgniteScene::getSceneListData(mt::Ignite::sceneDataIndex index)
	{
		return _pSceneView->getListData(index);
	}

	void IgniteScene::updateSceneList()
	{
		mt::Ignite::sceneData* sceneData;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		sceneList.clear();
		sceneList.push_back("No Scene Selected");
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		for (auto j = 0; j < (int)_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES); j++)
		{
			sceneData = &songDataPtr->scene_data[j];
			sceneList.push_back(sceneData->name);
		}
	}

	void IgniteScene::deleteAllScenes()
	{
		if ((int)_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES) > 0)
		{
			int selected_sceneIndex = getSelected();

			DBG(String(__FUNCTION__) + "::" + String(__LINE__));
			if (_pPopUpPtr->isDialogOpen.get() != true)
			{
				_pPopUpPtr->close();
				_pPopUpPtr->setPopUpPropories(" Delete all Scenes of " + _pSongPtr->getSongData(mt::Ignite::songsDataIndex::SONG_NAME).toString(), 604, 290);
				_pPopUpPtr->setbtnsize(265, 44);
				_pPopUpPtr->setInfoText(" Are you sure you want to delete all scenes of " + _pSongPtr->getSongData(mt::Ignite::songsDataIndex::SONG_NAME).toString());
				_pPopUpPtr->addImageButton("Cancel", "btn-secondary-cancel-267x44-default.png", "btn-secondary-cancel-267x44-default-hover.png", 2);
				_pPopUpPtr->addImageButton("delete", "btn-delete-267x44-default.png", "btn-delete-267x44-default-hover.png", 1);
				_pPopUpPtr->popupType = POPUP_WIPEDATA;
				_pPopUpPtr->isCloseButtonVisible(false);
				_pPopUpPtr->show();
				_pPopUpPtr->onSelectionChanged = [this, &selected_sceneIndex](int r)
					{
						if (r == 1)
						{
							auto num_of_scenes = (int)_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES);
							while (num_of_scenes)
							{
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramDeleteSceneCmd, getSelected());
								num_of_scenes--;
							}

							statusBarData.isSceneCreated = false;
							if (auto dw = getCurrentlyModalComponent())
								dw->exitModalState(0);

							_pSongPtr->updateDataFromEngine();
							_pSongPtr->removeScenesFromList();
							addNewInList.setVisible(false);
							changeSceneOptionsVisibility(true);
						}
						else
						{
							if (auto dw = getCurrentlyModalComponent())
								dw->exitModalState(0);
						}
					};
			}
		}
		enableControlButton(IgniteControlOptions::PLAY, false);
		enableControlButton(IgniteControlOptions::REC, false);
	}

	void IgniteScene::deleteCurrentScene()
	{
		//Send request to Engine for deleting scene
		container.removeChildComponent(getSelected());
		_pRowPtr = _pSongPtr->getSceneList();
		_pRowPtr->remove(getSelected());
		container.setSize(220, (_pRowPtr->size() * 44));
	}

	void IgniteScene::updateLables()
	{
		std::vector<IgniteParamList> sceneParams;
		IgniteParamList params;

		params.param = (paramId_t)SceneParam::paramUndoListCmd;
		params.startPos = 0;
		sceneParams.push_back(params);
		sceneParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, sceneParams);

		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::SCENE_TEMPO, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::SCENE_SWING, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::GBAR, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::XPOSE, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::LENGHT, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::SONGADV, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::MUTEHOLD, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::FTSROOTNOTE, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::FTSSCALE, false);
		//_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::XFTS, false);

		//for (auto& j : sceneParams)
		//{
		//	if (j.param == (paramId_t)SceneParam::paramSceneTempo)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::SCENE_TEMPO, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneSwing)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::SCENE_SWING, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneGBar)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::GBAR, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneTransposeValue)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::XPOSE, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneLength)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::LENGHT, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneAdvanceMode)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::SONGADV, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneMuteHold)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::MUTEHOLD, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneFTSRootNote)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::FTSROOTNOTE, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneFTSScale)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::FTSSCALE, true);
		//	if (j.param == (paramId_t)SceneParam::paramSceneTransposeFTSPriority)
		//		_pSceneView->updateLableText(mt::Ignite::sceneDataIndex::XFTS, true);
		//	/* TODO: Add other parameters once supported by engine*/
		//}
		if (sceneParams.size() == 1)
			updateSceneEditIndication(false);

		_pSceneView->repaint();
	}

	void IgniteScene::onOptCtlButtonClicked(IgniteControlOptions::eKey option)
	{
		auto songDataPtr = _pSongPtr->getSongDataPtr();
		if ((songDataPtr == nullptr) || (!songDataPtr->sceneCreated))
			return;

		if (option == IgniteControlOptions::SAVEALLON)
		{
			sceneDataUpdated = false;
			(*_pRowPtr)[getSelected()]->updateEditIndication();
		}
		else if (option == IgniteControlOptions::AUTOSAVEON)
		{
			isAutosave = true;
		}
		else if (option == IgniteControlOptions::AUTOSAVEOFF)
		{
			isAutosave = false;
		}
		if (isVisible())
		{
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " key: " + String(option));
			auto opMode = _pSceneOptCtl->getOperationMode();

			switch (option) {
			case IgniteControlOptions::PLAY:
				if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
				{
					addNewInList.setEnabled(false);
					enableControlButton(IgniteControlOptions::DEL, false);
					enableControlButton(IgniteControlOptions::COPY, false);
					enableControlButton(IgniteControlOptions::MOVE, false);
				}
				else
				{
					addNewInList.setEnabled(true);
					enableControlButton(IgniteControlOptions::DEL, true);
					enableControlButton(IgniteControlOptions::COPY, true);
					enableControlButton(IgniteControlOptions::MOVE, true);
				}
				break;
			case IgniteControlOptions::STOP:
				addNewInList.setEnabled(true);
				enableControlButton(IgniteControlOptions::DEL, true);
				enableControlButton(IgniteControlOptions::COPY, true);
				enableControlButton(IgniteControlOptions::MOVE, true);
				break;
			case IgniteControlOptions::SAVE:
				saveScene();
				break;
			case IgniteControlOptions::LOOP:
				_pRowPtr = (juce::OwnedArray <ui::IgniteSceneRow>*)songDataPtr->list;
				if (songDataPtr->sceneCreated && (_pRowPtr != NULL))
				{
					updateSceneList();
					_pSceneLoopComp->setVisible(true);
					//_pSceneLoopComp->runModalLoop();
				}
				break;
			case IgniteControlOptions::DEL:
				deleteScene();
				break;
			case IgniteControlOptions::UNDO:
				if (sceneDataUpdated)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramUndoCmd, getSelected());
					Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 50);
					updateLables();
					setSelected(getSelected());
				}
				break;
			case IgniteControlOptions::COPY:
				_pSceneCopyComp->setLabelText(getSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME).toString());
				_pSceneCopyComp->setVisible(true);
				_pSceneCopyComp->runModalLoop();
				break;
			case IgniteControlOptions::MOVE:
				_pAppOptPtr->optionMove.getToggleState() ? moveEnabled = true : moveEnabled = false;
				moveEnabled ? view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all) : view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::never);
				_pAppOptPtr->enableMoveoptions(mt::Ignite::tabs::SCENE, 0, moveEnabled);
				break;
			case IgniteControlOptions::MOVETOP:
				moveScene(0);
				break;
			case IgniteControlOptions::MOVEBTM:
				moveScene(songDataPtr->num_scenes);
				break;
			case IgniteControlOptions::MOVEDOWN:
				if ((getSelected() + 1) < songDataPtr->num_scenes)
					moveScene(getSelected() + 1);
				break;
			case IgniteControlOptions::MOVEUP:
				if ((getSelected() - 1) >= 0)
					moveScene(getSelected() - 1);
				break;
			case IgniteControlOptions::CONFIG:
				AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::MENU);
				AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::SCENE_WINDOW);
				break;
			}
		}
	}

	void IgniteScene::deleteScene()
	{
		int selected_sceneIndex = getSelected();

		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		if (_pPopUpPtr->isDialogOpen.get() != true)
		{
			_pPopUpPtr->close();
			_pPopUpPtr->setPopUpPropories("Delete " + String(getSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME).toString()), 604, 290);
			_pPopUpPtr->setbtnsize(265, 44);
			_pPopUpPtr->setInfoText("Are you sure you want to delete " + String(getSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME).toString()));
			_pPopUpPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
			_pPopUpPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
			_pPopUpPtr->popupType = POPUP_WITHOKCANCEL;
			_pPopUpPtr->isCloseButtonVisible(false);
			_pPopUpPtr->show();
			_pPopUpPtr->onSelectionChanged = [this, selected_sceneIndex](int r)
				{
					if (r == 1)
					{
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramDeleteSceneCmd, getSelected());

						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);

						statusBarData.isSceneCreated = false;
						_pSongPtr->updateDataFromEngine();
						if ((int)(_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES)) > 0)
						{
							if (selected_sceneIndex == 0)
								setSelected(selected_sceneIndex);
							else
								setSelected(selected_sceneIndex - 1);
						}
						else
						{
							IgniteTrack::GetAppTrackInstance()->trackCreated = false;
							enableViewBox(false);
							changeSceneOptionsVisibility(true);
							addNewInList.setVisible(false);
							_pSongPtr->setSongData(mt::Ignite::songsDataIndex::SCENE_CREATED, false, true);
							enableControlButton(IgniteControlOptions::STOP, false);
							enableControlButton(IgniteControlOptions::PLAY, false);
							enableControlButton(IgniteControlOptions::REC, false);
						}

						if ((int)_pSongPtr->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES) >= 2)
							enableControlButton(IgniteControlOptions::LOOP, true);
						else
							enableControlButton(IgniteControlOptions::LOOP, false);

						refreshSceneUI();
						AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
						setFpSceneLedState();
					}
					else
					{
						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);
					}
				};
		}
	}

	void IgniteScene::saveScene()
	{
		if (_pPopUpPtr->isDialogOpen.get() != true)
		{
			_pPopUpPtr->close();
			_pPopUpPtr->setPopUpPropories("Save ", 604, 290);

			DBG(String(__FUNCTION__) + "::" + String(__LINE__));

			_pPopUpPtr->setbtnsize(265, 44);
			_pPopUpPtr->setInfoText(" Save " + String(getSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME).toString()) + " edits ?");
			_pPopUpPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
			_pPopUpPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
			_pPopUpPtr->popupType = POPUP_WITHOKCANCEL;
			_pPopUpPtr->isCloseButtonVisible(false);
			_pPopUpPtr->show();
			_pPopUpPtr->onSelectionChanged = [this](int r)
				{
					if (r == 1)
					{
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, getSelected());
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, getSelected());
						sceneDataUpdated = false;
						_pSceneView->repaint();
					}
					updateLables();
					if (auto dw = getCurrentlyModalComponent())
						dw->exitModalState(0);
				};
		}
	}

	bool IgniteScene::pannelKeyHandler(paramId_t param, uint32_t keyValue)
	{
		if (!_pSongPtr->getSongData(mt::Ignite::songsDataIndex::SCENE_CREATED) && (param != (paramId_t)FrontPannelParam::paramKey_INSERT))
			return false;

		if ((keyValue == (uint32_t)FrontPannelKeyStatus::paramKeyStatusReleased) && (param != (paramId_t)FrontPannelParam::paramKey_ROTARYBAR))
			return true;

		bool keyHandled = false;
		itemParam = param;
		itemKeyValue = keyValue;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "param: " + String(param) + " value: " + String(keyValue));

		if (_pSceneCopyComp->isVisible())
			return _pSceneCopyComp->PannelKeyHandler(param, keyValue);

		if (_pSceneLoopComp->isVisible())
			return _pSceneLoopComp->PannelKeyHandler(param, keyValue);

		if (_pPopUpPtr != nullptr && _pPopUpPtr->IsEditing() && _pPopUpPtr->popupType != POPUP_WIPEDATA)
			return _pPopUpPtr->FPKeyHandler(true, param, keyValue);

		if (param == (paramId_t)FrontPannelParam::paramKey_NEXT_TIE && _pSceneOptCtl->getOperationMode() == mt::Ignite::operatioMode::IDLE_MODE)
		{
			if (!_pRowPtr->isEmpty())
			{
				(*_pRowPtr)[sceneRowHighLightIndex]->SetFocus(false, false);
				this->postCommandMessage(4);

				if (sceneRowHighLightIndex + 1 >= _pRowPtr->size())
					sceneRowHighLightIndex = 0;
				else
					sceneRowHighLightIndex = jmin<int>(jmax<int>((sceneRowHighLightIndex + 1), 0), (_pRowPtr->size() - 1));

				setSceneRowHighLightIndex(true, 1);

				(*_pRowPtr)[sceneRowHighLightIndex]->SetFocus(true, false);
				repaint();
			}
			return true;
		}

		if ((optionHandlerEnabled == SCENE_OPTION) && !addNew.isVisible() && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
		{
			keyHandled = pannelKeyOptionsHandler(param, keyValue);
		}
		else if ((FrontPannelParam)param == FrontPannelParam::paramKey_COPY_NAV)
		{
			this->postCommandMessage(9);

		}
		else if ((optionHandlerEnabled == SCENE_DETAIL) && !addNew.isVisible() && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT && (param != (paramId_t)FrontPannelParam::paramKey_SCENE))
		{
			keyHandled = _pSceneView->pannelKeyHandler(param, keyValue);
		}
		else
		{
			if (view.isVisible())
			{
				if (_pSceneLoopComp->isShowing())
				{
					_pSceneLoopComp->PannelKeyHandler(param, keyValue);
				}

				else
				{
					if (addNew.isVisible())
					{
						if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT || ((FrontPannelParam)param == FrontPannelParam::paramKey_INSERT))
						{
							this->postCommandMessage(1);
							_pSceneView->pannelKeyHandler(param, keyValue);
						}
					}
					else
					{
						if (!_pRowPtr->isEmpty() && (optionHandlerEnabled == SCENE_LIST))
						{
							if (param == (paramId_t)FrontPannelParam::paramKey_INSERT)
							{
								this->postCommandMessage(2);
								return true;
							}
							if (param == (paramId_t)FrontPannelParam::paramKey_ROTARYBAR)
							{
								if (FrountPannel::GetFrountPannelInstance()->getKeyStatus((paramId_t)FrontPannelParam::paramKey_ROTARYBAR) == FrontPannelKeyStatus::paramKeyStatusPressed)
								{
									AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
									for (int sceneBank = 0; sceneBank <= (gSongs[_pSongPtr->getSelected()].scene_data.size() / 16); sceneBank++)
									{
										_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + sceneBank), fpLEDPattern::EN_RGB_LED_GREEN);
									}
								}
								else if (FrountPannel::GetFrountPannelInstance()->getKeyStatus((paramId_t)FrontPannelParam::paramKey_ROTARYBAR) == FrontPannelKeyStatus::paramKeyStatusReleased)
								{
									setFpSceneLedState();
								}
								return true;
							}
							setFpSceneLedState();
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
							case FrontPannelParam::paramKey_TRACKSEL16:
								if (FrountPannel::GetFrountPannelInstance()->getBarKeyStatus() == FrontPannelKeyStatus::paramKeyStatusOnHold)
								{
									auto scenesInBank = 0;
									currentSceneBank = (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
									int index = 0;
									AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
									for (auto scene : gSongs[_pSongPtr->getSelected()].scene_data)
									{
										if (scene.current_scene_bank == currentSceneBank)
										{
											if (index == selected)
												_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + scenesInBank), fpLEDPattern::EN_RGB_LED_LBLUE);
											else
												_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + scenesInBank), fpLEDPattern::EN_RGB_LED_LPURPLE);
											scenesInBank++;
										}
										if (scenesInBank >= 16) break;
										index++;
									}
								}
								else if (FrountPannel::GetFrountPannelInstance()->getDeleteKeyStatus() == FrontPannelKeyStatus::paramKeyStatusOnHold)
								{
									auto selectedScene = (currentSceneBank * 16) + (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
									if (gSongs[_pSongPtr->getSelected()].scene_data.size() > selectedScene)
									{
										selected = selectedScene;
										setSelected(selected);
										_pSceneOptCtl->PannelKeyHandler((paramId_t)FrontPannelParam::paramKey_DELETE, keyValue);
									}
									else
										return true;//nothing to do
								}
								else
								{
									auto selectedScene = (currentSceneBank * 16) + (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
									if (gSongs[_pSongPtr->getSelected()].scene_data.size() > selectedScene)
									{
										if (selected == selectedScene)
										{
											changeSceneOptionsVisibility(false);
											AppOptions::GetOptionsInstance()->taboptionChanges(mt::Ignite::tabs::SCENE, 0);
											changeSceneOptionsVisibility(true);
										}
										else
										{
											(*_pRowPtr)[sceneRowHighLightIndex]->SetFocus(false, false);
											int indexToMove = abs(selected - selectedScene);
											bool isInc = (selected > selectedScene);
											selected = selectedScene;
											sceneRowHighLightIndex = selectedScene;
											setSceneRowHighLightIndex(isInc, indexToMove);
											setSelected(selected);
											(*_pRowPtr)[sceneRowHighLightIndex]->SetFocus(true, false);
										}
									}
									else
										return true;//nothing to do
								}
								view.setViewPosition(view.getViewPositionX(), selected > 7 ? (selected - 3) * ROW_HEIGHT : 0);
								break;

							}
							keyHandled = true;
							(*_pRowPtr)[sceneRowHighLightIndex]->SetFocus(false, false);
							addNewInList.setState(Button::ButtonState::buttonNormal);
							switch ((FrontPannelParam)param)
							{
							case FrontPannelParam::paramKey_RMOVELEFT:
								if ((*_pRowPtr)[sceneRowHighLightIndex]->isEditing())
									this->postCommandMessage(4);
								else
								{
									if (sceneRowHighLightIndex + 1 >= _pRowPtr->size() && addNewInList.isVisible())
									{
										addNewInList.setState(Button::ButtonState::buttonOver);
										sceneRowHighLightIndex = (_pRowPtr->size() - 1);
										addNewInListHiglited = true;
									}
									else
									{
										sceneRowHighLightIndex = jmin<int>(jmax<int>((sceneRowHighLightIndex + 1), 0), (_pRowPtr->size() - 1));
									}
									setSceneRowHighLightIndex(true, 1);

									if (((sceneRowHighLightIndex + 3) * ROW_HEIGHT >= view.getViewPositionY()) && sceneRowHighLightIndex > 3)
										view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() + ROW_HEIGHT);
								}
								break;
							case FrontPannelParam::paramKey_RMOVERIGHT:
								if ((*_pRowPtr)[sceneRowHighLightIndex]->isEditing())
									this->postCommandMessage(4);
								else
								{
									if (addNewInListHiglited)
									{
										addNewInList.setState(Button::ButtonState::buttonNormal);
										addNewInListHiglited = false;
										sceneRowHighLightIndex = (_pRowPtr->size() - 1);
									}
									else
										sceneRowHighLightIndex = jmin<int>(jmax<int>((sceneRowHighLightIndex - 1), 0), (_pRowPtr->size() - 1));

									setSceneRowHighLightIndex(false, 1);
									if (((sceneRowHighLightIndex - 3) * ROW_HEIGHT < view.getViewPositionY()))
										view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() - ROW_HEIGHT);
								}
								break;
							case FrontPannelParam::paramKey_RDBLSELECT:
							case FrontPannelParam::paramKey_RSELECT:
								if (addNewInListHiglited)
									this->postCommandMessage(2); //Add new in list Button
								else if (sceneRowHighLightIndex == _pRowPtr->size())
									(*_pRowPtr)[sceneRowHighLightIndex]->postCommandMessage(1);
								else
									this->postCommandMessage(4);
								break;
							case FrontPannelParam::paramKey_PAGESHIFT:
								if (optionHandlerEnabled == SCENE_LIST)
								{
									optionHandlerEnabled = SCENE_DETAIL;
									addNewInList.setState(Button::ButtonState::buttonOver);
									keyHandled = _pSceneView->pannelKeyHandler(param, keyValue);
									repaint();
								}
								break;
							case FrontPannelParam::paramKey_RETURN_NAV:
								if ((*_pRowPtr)[sceneRowHighLightIndex]->isEditing())
									this->postCommandMessage(4);
								break;
							}

							if ((optionHandlerEnabled == SCENE_LIST) && !addNewInListHiglited)
								(*_pRowPtr)[sceneRowHighLightIndex]->SetFocus(true, false);
							repaint();
						}
						else if (!_pRowPtr->isEmpty() && (optionHandlerEnabled == SCENE_OPTION))
						{
							if (param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
							{
								if (optionHandlerEnabled == SCENE_OPTION)
								{
									optionHandlerEnabled = SCENE_LIST;
									param = (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT;
									pannelKeyOptionsHandler(param, keyValue);
									keyHandled = pannelKeyHandler(param, keyValue);
								}
								else
									keyHandled = pannelKeyOptionsHandler(param, keyValue);
							}
						}
						else if (!_pRowPtr->isEmpty() && (optionHandlerEnabled == SCENE_DETAIL))
						{
							if (param == (paramId_t)FrontPannelParam::paramKey_SCENE)
							{
								optionHandlerEnabled = SCENE_LIST;
							}
							if (param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
							{
								if (optionHandlerEnabled == SCENE_DETAIL)
								{
									optionHandlerEnabled = SCENE_OPTION;
									_pSceneView->pannelKeyHandler(param, keyValue);
									keyHandled = pannelKeyOptionsHandler(param, keyValue);
								}
								else
									keyHandled = _pSceneView->pannelKeyHandler(param, keyValue);
							}
						}
					}
				}
			}
			else
			{
				if (((FrontPannelParam)param == FrontPannelParam::paramKey_RSELECT) || ((FrontPannelParam)param == FrontPannelParam::paramKey_INSERT))
					this->postCommandMessage(1); //Add new Button
				keyHandled = true;
			}
		}
		return keyHandled;
	}
	void IgniteScene::setSceneRowHighLightIndex(bool isInc, int indexBy)
	{
		if (isInc)
		{
			if ((sceneRowHighLightIndex) * (52 * indexBy) > (view.getViewArea().getHeight() - (52 * indexBy)))
				view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() + (52* indexBy));
		}
		else
		{
			if ((sceneRowHighLightIndex - 1) * (52 * indexBy) <= (view.getViewArea().getHeight()))
				view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() - (52 * indexBy));
		}
	}

	int IgniteScene::setNextCurrentTrack(int selectedTrack)
	{
		mt::Ignite::sceneData* scDataPtr = _pSongPtr->getSceneData(getSelected());

		for (auto& trackNum : scDataPtr->track_data)
		{
			if (trackNum.trackState)
			{
				setSceneData(mt::Ignite::sceneDataIndex::CURRENT_TRACK, trackNum.id - 1, false);
				return trackNum.id - 1;
			}
		}
		return selectedTrack;
	}

	bool IgniteScene::pannelKeyOptionsHandler(paramId_t param, uint32_t keyValue)
	{
		bool setfocus = true;
		bool moveBtnState = _pAppOptPtr->getBtnState(IgniteControlOptions::eKey::MOVEUP);

		if (keyFocusedItem == OptionsViewItemID::NO_FOCUS)
		{
			/*1st key after Page visible */
			if (moveBtnState)
				keyFocusedItem = OptionsViewItemID::SCENE_MOVE_DOWN;
			else
				keyFocusedItem = OptionsViewItemID::SCENE_MOVE;
		}
		else
		{
			pannelSendKey(false, param, keyValue); // clear last Focus
			if (optionHandlerEnabled != SCENE_OPTION)
				return true;
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				keyFocusedItem = (keyFocusedItem + 1 >= OptionsViewItemID::FOCUS_MAX ? (OptionsViewItemID::FOCUS_MAX) : keyFocusedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				if (moveBtnState)
					keyFocusedItem = (keyFocusedItem - 1 <= OptionsViewItemID::NO_FOCUS ? (OptionsViewItemID::NO_FOCUS + 1) : keyFocusedItem - 1);
				else
					keyFocusedItem = (keyFocusedItem - 1 <= OptionsViewItemID::SCENE_MOVE_LAST ? (OptionsViewItemID::SCENE_MOVE_LAST + 1) : keyFocusedItem - 1);
				break;
			}
		}
		if (setfocus)
		{
			pannelSendKey(true, param, keyValue);
		}
		return true;
	}

	void IgniteScene::pannelSendKey(bool isFocuse, paramId_t param, uint32_t keyValue)
	{
		itemFocus = isFocuse;
		switch (optionsView[keyFocusedItem])
		{
		case SCENE_MOVE_DOWN:
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEDOWN, isFocuse, param, keyValue);
			break;
		case SCENE_MOVE_UP:
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEUP, isFocuse, param, keyValue);
			break;
		case SCENE_MOVE_TOP:
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVETOP, isFocuse, param, keyValue);
			break;
		case SCENE_MOVE_LAST:
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEBTM, isFocuse, param, keyValue);
			break;
		case SCENE_MOVE:
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVE, isFocuse, param, keyValue);
			break;
		case SCENE_UNDO:
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::UNDO, isFocuse, param, keyValue);
			break;
		case SCENE_COPY:
			if (!isFocuse)
				_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::COPY, isFocuse, param, keyValue);
			else
				this->postCommandMessage(3);
			break;
		case SCENE_CONFIG:
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::CONFIG, isFocuse, param, keyValue);
			break;
		/*case SCENE_LOOP:
			if (!isFocuse)
				_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::LOOP, isFocuse, param, keyValue);
			else
				this->postCommandMessage(5);
			break;*/
		default:
			break;
		}
	}

	void IgniteScene::handleCommandMessage(int id)
	{
		if (id == 1)
			buttonClicked(&addNew);
		else if (id == 2)
			buttonClicked(&addNewInList);
		else if (id == 3)
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::COPY, itemFocus, itemParam, itemKeyValue);
		else if (id == 4)
			(*_pRowPtr)[sceneRowHighLightIndex]->pannelKeyHandler(itemParam, itemKeyValue);
		else if (id == 5)
			_pSceneOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::LOOP, itemFocus, itemParam, itemKeyValue);
		else if (id == 8)
			updateTracksData();
		else if (id == 9)
		{
			_pSceneCopyComp->setLabelText(getSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME).toString());
			_pSceneCopyComp->setVisible(true);
			_pSceneCopyComp->runModalLoop();
		}
	}

	void IgniteScene::moveScene(juce::Point<float> position)
	{
		int index = static_cast<int>(((getSelected() * (layout::row.getHeight())) + position.getY()) / (layout::row.getHeight()));

		if ((index == getSelected()))
			return;

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneMove, index);
		_pSongPtr->updateDataFromEngine();
		setSelected(index);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "dest_index: " + String(index));
	}

	void IgniteScene::moveScene(int dest_index)
	{
		if ((dest_index == getSelected()))
			return;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "dest_index: " + String(dest_index));

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSceneMove, dest_index);
		_pSongPtr->updateDataFromEngine();
		if ((_pSongPtr->getSceneList()->size() == dest_index) && (dest_index > 0))
			dest_index--;
		setSelected(dest_index);
	}

	void IgniteScene::setFpSceneLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		auto currentBank = 0;
		auto sceneStart = 0;
		auto& gSongRef = gSongs.getReference(_pSongPtr->getSelected());

		for (int scene = 0; scene < gSongRef.scene_data.size(); scene++)
		{
			if ((scene == 0) || (scene % 16) != 0)
			{
				gSongRef.scene_data[scene].current_scene_bank = currentBank;
			}
			else
			{
				currentBank = currentBank + 1;
				gSongRef.scene_data[scene].current_scene_bank = currentBank;
			}

			if (gSongRef.scene_data[scene].current_scene_bank == currentSceneBank)
			{
				if (scene == getSelected())
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + sceneStart), fpLEDPattern::EN_RGB_LED_LBLUE);
				else
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + sceneStart), fpLEDPattern::EN_RGB_LED_LPURPLE);

				sceneStart++;

				if (sceneStart >= 16)
					break;
			}
		}
	}

	////////////////////////////////////////////////
	//   Class: Ignite Scene row
	////////////////////////////////////////////////
	IgniteSceneRow::IgniteSceneRow(int SerialNumber, mt::Ignite::sceneData& Data, IgniteScene* scenePtr) : Scenedata(Data)
	{
		_pScene = scenePtr;
		name.makeVisibleAndAddListner(this);
		name.setEditable(false, true, false);
		name.setDisplayTitle(false);
		serialNumber = String(SerialNumber + 1);
		name.setText(String(serialNumber) + ": " + Scenedata.name, juce::dontSendNotification);
		name.setJustificationType(Justification::centred);
		srNo.setText(String(SerialNumber), juce::dontSendNotification);
		name.addMouseListener(this, false);
		name.setLableFont(font_10);

		sceneState.makeVisible(this);
		sceneState.setDisplayTitle(false);
		sceneState.setJustificationType(juce::Justification::centred);
		sceneState.setTextFont(font_12);

		box.items.add(FlexItem(sceneState).withMargin(0).withFlex(2));
		box.items.add(FlexItem(name).withMargin(0).withFlex(5));
		getAssets();
		name.onValueChanged = [this](String value)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;
				_pScene->setSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME, value, false);

				if (!IgniteSceneConfig::getAppSceneConfigInstance()->isAutoSaveEnabled())
					_pScene->updateSceneEditIndication(true);

				IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
				updateEditIndication();
			};

		name.ValidateDataOnChange = [this](String value)
			{
				Component::BailOutChecker checker(this);
				if (checker.shouldBailOut())
					return KEY_PAD_DATA_VALIDATE_OK;

				if (_pScene->getSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME) != value)
				{
					//check for duplicate
					for (auto& i : gSongs[IgniteSong::getAppSongInstance()->getSelected()].scene_data)
					{
						if (i.name == value)
							return KEY_PAD_DATA_DUPLICATE_ERROR; // Name already present
					}
				}
				return KEY_PAD_DATA_VALIDATE_OK;
			};
	}


	IgniteSceneRow::~IgniteSceneRow()
	{}

	int IgniteSceneRow::getSongIndex(void)
	{
		return songIndex;
	}

	void IgniteSceneRow::setSongIndex(int index)
	{
		songIndex = index;
	}

	void IgniteSceneRow::highlight(bool State)
	{
		State ? sceneState.setText("Active", dontSendNotification) : sceneState.setText("InActive", dontSendNotification);
		isHighlighted = State;
		repaint();
	}

	int IgniteSceneRow::getSerialNumber()
	{
		return srNo.getText().getIntValue();
	}

	void IgniteSceneRow::mouseDown(const MouseEvent& event)
	{
	}

	void IgniteSceneRow::mouseUp(const MouseEvent& event)
	{
		if (rowIsBeingDraggedOver)
		{
			if (_pScene->getSceneMoveState())
			{
				_pScene->moveScene(event.position);
			}
			rowIsBeingDraggedOver = false;
		}
		else if (onMouseDown)
			onMouseDown(this);
	}

	void IgniteSceneRow::mouseDrag(const MouseEvent& event)
	{
		juce::DragAndDropContainer* dragC = juce::DragAndDropContainer::findParentDragContainerFor(this);
		if (dragC)
		{
			if (isHighlighted)
			{
				if (!dragC->isDragAndDropActive())
				{
					if (_pScene->getSceneMoveState())
						dragC->startDragging("TargetSource", this);
					rowIsBeingDraggedOver = true;
				}
			}
		}
	}

	void IgniteSceneRow::updateEditIndication()
	{
		String scneneNameString = _pScene->getSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME);

		scneneNameString = serialNumber + ": " + scneneNameString;

		if (_pScene->getSceneDataUpdateStatus() && !_pScene->isGlobalAutosaveEnabled())
			scneneNameString = scneneNameString + "*";

		name.setText(scneneNameString, juce::dontSendNotification);
	}

	void IgniteSceneRow::refreshSceneRowUI()
	{
		updateEditIndication();
		srNo.setText(String(getSerialNumber()), juce::dontSendNotification);
		sceneState.setText("Active", dontSendNotification);
	}

	void IgniteSceneRow::labelTextChanged(Label* labelThatHasChanged)
	{
		if (labelThatHasChanged == &name)
		{
			_pScene->setSceneData(mt::Ignite::sceneDataIndex::SCENE_NAME, name.getText(), false);
			_pScene->setSceneDataUpdateStatus(true);
			updateEditIndication();
		}
	}

	void IgniteSceneRow::getAssets()
	{
		imageSceneListActiveHover= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-list-item-active-hover.png"));
		imageSceneListActive= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-list-item-active.png"));
		imageSceneListInactiveDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-list-item-inactive.png"));
		imageSceneListInactiveHover= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-list-item-inactive-hover.png"));
	}

	void IgniteSceneRow::paint(juce::Graphics& g)
	{
		auto color = ui::Theme::Instance().getColour(neutralGrey4);
		Image dropImage;
		String s;

		if (isloopEnabled)
			g.drawImage(isMouseOver ? imageSceneListActiveHover: imageSceneListActive, getLocalBounds().toFloat(), RectanglePlacement::centred);
		
		if (isHighlighted)
			g.drawImage(isMouseOver ? imageSceneListActiveHover : imageSceneListActive, getLocalBounds().toFloat(), RectanglePlacement::centred);
		else
			g.drawImage(isMouseOver ? imageSceneListInactiveHover : imageSceneListInactiveDefault, getLocalBounds().toFloat(), RectanglePlacement::centred);
	}

	void IgniteSceneRow::setColor(int colourID, Colour colour)
	{
		if (colourID == ui::Label::backgroundColourId)
		{
			name.setBackgroundColour(colour);
		}
		else if (colourID == ui::Label::textColourId)
		{
			name.setTextColour(colour);
		}
		else if (colourID == ui::Label::outlineColourId)
		{
			name.setColour(ui::Label::outlineColourId, colour);
		}
	}
	void IgniteSceneRow::resized()
	{
		box.performLayout(getLocalBounds());
	}

	bool IgniteSceneRow::pannelKeyHandler(paramId_t param, uint32_t keyValue)
	{
		if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT || param == (paramId_t)FrontPannelParam::paramKey_NEXT_TIE)
		{
			onMouseDown(this);
		}
		else if (param == (paramId_t)FrontPannelParam::paramKey_RDBLSELECT)
		{
			name.editLable();
			name.PannelKeyHandler(param, keyValue);
		}
		return true;
	}
	void IgniteSceneRow::SetFocus(bool hasFocus, bool activeHasFocus)
	{
		if (hasFocus)
			isMouseOver = true;
		else
			isMouseOver = false;

		if (activeHasFocus)
		{
			name.setBackgroundColour(juce::Colour(0x2086BF));
			sceneState.setBackgroundColour(juce::Colour(0x28AAEE));
		}
		repaint();
	};

	void IgniteSceneRow::mouseMove(const MouseEvent& event)
	{
		isMouseOver = true;
		repaint();
	}

	void IgniteSceneRow::mouseExit(const MouseEvent& event)
	{
		isMouseOver = false;
		repaint();
	}
};
