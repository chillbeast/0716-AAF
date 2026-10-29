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

class AppOptCtl;
namespace ui
{
	mt::Ignite::StatusParams statusBarData;

	juce::Array<mt::Ignite::songData> gSongs;


	////////////////////////////////////////////////
	//   Class: Ignite Song
	////////////////////////////////////////////////
	IgniteSong::IgniteSong()
	{
		addAndMakeVisible(view);
		addAndMakeVisible(container);

		addNew.makeVisibleAndAddListner(this);
		addNewInList.makeVisibleAndAddListner(this);
		addNew.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		addNew.setColour(juce::TextEditor::outlineColourId, Theme::Instance().getColour(customBlue13));
		addNew.setColour(TextButton::ColourIds::textColourOffId, Theme::Instance().getColour(customBlue13));
		addNew.setColour(TextButton::ColourIds::buttonColourId, Theme::Instance().getColour(customdblue));
		//addNewInList.setColour(juce::TextButton::buttonColourId, Theme::Instance().getColour(neutralGrey1));
		addNew.setBounds(Rectangle(xStartAddNewBtn, yStartAddNewBtn, widthAddNewBtn, heightAddNewBtn));
		addNewInList.setBounds(Rectangle(xStartAddNewInList, yStartAddNewInList, widthAddNewInList, heightAddNewInList));
		addNew.setVisible(true);
		addNewInList.setToggleable(false);
		addNewInList.setVisible(false);
		addNewInList.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());

		_pEmptySongComp = new class AppDnp();
		addChildComponent(_pEmptySongComp);
		_pEmptySongComp->setBounds(xStartEmptyComp, yStartEmptyComp, 1004, heightEmptyComp);
		_pEmptySongComp->setVisible(true);
		_pEmptySongComp->setDisplayMessage(String(" "));

		view.setViewedComponent(&container, true);
		view.setBounds(10, 62, widthScreen - 10, heightSongView);
		view.setScrollBarsShown(true, false, true, true);
		view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);
		container.setVisible(false);
		view.setVisible(false);

		addAndMakeVisible(songsView);
		songsView.addListener(this);
		songsView.setToggleState(true, dontSendNotification);
		songsView.setVisible(true);
		songsView.setLookAndFeel(ImageButtonLookandFeel::getLnf());


		addAndMakeVisible(listView);
		listView.addListener(this);
		listView.setToggleState(false, dontSendNotification);
		listView.setVisible(true);
		listView.setLookAndFeel(ImageButtonLookandFeel::getLnf());


		viewBox.items.add(FlexItem(songsView).withMargin(1).withFlex(1));
		viewBox.items.add(FlexItem(listView).withMargin(1).withFlex(1));

		setBufferedToImage(true);
		songCreated = false;
		bootupState = true;
		curretSongRow = -1;
		refreshSongUI();

		_pSongView = AppSongView::getAppSongViewInstance(this);
		addChildComponent(_pSongView);
		_pSongView->setBounds(xStartSongView1, yStartSongView1, widthSongView1, heightSongView1);

		_pAppOptPtr = AppOptions::GetOptionsInstance();

		_pSongOptCtl = IgniteControlOptions::GetAppOptInstance();
		addChildComponent(_pSongOptCtl);
		_pSongOptCtl->setBounds(xStartOptctl, yStartOptctl, widthOptctl, heightOptctlt);
		_pSongOptCtl->addOptCtlListener(this);
		enableControlButton(IgniteControlOptions::STOP, false);

		_pPopUpPtr = CommonPopUp::getCommonPopupInstance();

		_pSongCopyComp = new IgniteCopy();
		addChildComponent(_pSongCopyComp);
		_pSongCopyComp->setBounds(xStartLoopCmp, yStartLoopCmp, widthCopy, heightCopy);
		_pSongCopyComp->setVisible(false);
		getAssets();

		_pSongCopyComp->onButtonPressed = [this](IgniteCopy::copyCtlKey key, String CopyName)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;

				copySong(key, CopyName);
			};
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Song module constructor completed");
	}

	IgniteSong::~IgniteSong()
	{
		cleanup();
	}

	int IgniteSong::getSelected(void)
	{
		return selected;
	}

	mt::Ignite::songData* IgniteSong::getSongDataPtr()
	{
		if (songCreated)
		{
			return &gSongs.getReference(selected);
		}
		else
		{
			return nullptr;
		}
	}

	void IgniteSong::updateDataFromEngine()
	{
		std::vector<IgniteParamList> songList;
		std::vector<IgniteParamList> sceneList;
		std::vector<IgniteParamList> sceneParamList;
		IgniteParamList params;
		auto sceneCount = 0;
		int activeSong = 0;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Load song data");
		bootupState = true;
		list.clear();
		gSongs.clear();
		params.param = (paramId_t)SongParam::paramSongNameList;
		params.startPos = 0;
		params.Count = 100;
		songList.push_back(params);
		songList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, songList);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Number of songs: " + String(songList.size()));
		//read last saved active song from engine and set it  
		std::vector<IgniteParamList> activeSongIdSingleParam;
		params.param = (paramId_t)SongParam::paramActiveSong;
		activeSongIdSingleParam.push_back(params);
		activeSongIdSingleParam = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, activeSongIdSingleParam);
		if (!activeSongIdSingleParam.empty())
		{
			activeSong = activeSongIdSingleParam[0].MsgValue.get<std::int32_t>();
		}
		else
			activeSong = 0;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Last Active song Index = " + String(activeSong));
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Current Selected song Index = " + String(selected));

		selected = 0;
		curretSongRow = -1;
		if (!songList.empty())
		{
			songCreated = statusBarData.isSongCreated = true;

			for (auto& i : songList)
			{
				addSong(-1);
				setSongData(mt::Ignite::songsDataIndex::SONG_NAME, i.MsgValue.get<std::string>(), true);
				if (i.MsgValue.get<std::string>() == gSongs[activeSong].name)
				{
					gSongs.getReference(activeSong).songLoaded = true;
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)activeSong);
					sceneList.clear();
					params.param = (paramId_t)SceneParam::paramSceneCount;
					params.startPos = 0;
					sceneList.push_back(params);
					sceneList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, sceneList);
					sceneCount = sceneList[0].MsgValue.get<std::uint32_t>();

					DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Number of Scenes in Song : " + String(gSongs[activeSong].name) + " is: " + String(sceneCount));

					if (sceneCount > 0)
					{
						sceneList.clear();

						params.param = (paramId_t)SongParam::paramTrackCountCmd;
						sceneParamList.push_back(params);
						sceneParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, sceneParamList);
						gSongs.getReference(activeSong).num_of_tracks = ((int(sceneParamList[0].MsgValue.get<std::uint8_t>()) + 15) / 16);

						DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Number of Tracks in Song : " + String(gSongs[activeSong].name) + " is: " + String(gSongs.getReference(activeSong).num_of_tracks));

						sceneList.clear();
						params.param = (paramId_t)SceneParam::paramSceneNameList;
						params.startPos = 0;
						sceneList.push_back(params);
						sceneList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, sceneList);

						for (auto j = 0; j < sceneCount; j++)
							IgniteScene::getAppSceneInstance()->addScene(false, sceneList[j].MsgValue.get<std::string>());

						setSongData(mt::Ignite::songsDataIndex::SELECTED_SCENE, 0, true);
						setSongData(mt::Ignite::songsDataIndex::CURRENT_SCENE, 0, true);
						statusBarData.editSceneIndex = 1;
						IgniteScene::getAppSceneInstance()->setSelected(0);
						if (sceneCount > 1)
						{
							setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, 1, true);
							statusBarData.nextSceneIndex = 2;
						}
						else
						{
							setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, 0, true);
							statusBarData.nextSceneIndex = 1;
						}
						AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::TRACK, true);
					}
				}
				selected++;
			}
			enableViewBox(true);
		}
		else
		{
			songCreated = statusBarData.isSongCreated = statusBarData.isSceneCreated = false;
			updateSelectionInfo();
			_pSongView->setVisible(false);
			changeSongOptionsVisibility(true);
			return;
		}

		if (this->isVisible())
			_pSongView->setVisible(false);

		AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::SCENE, true);

		if ((activeSong >= songList.size()) && (songList.size() > 0))
			activeSong = (int)songList.size() - 1;
		setSelected(activeSong);
		setCurrentEditSceneEditTrack();
		removeScenesFromList();
	}

	void IgniteSong::requestDataFromEngine(int index)
	{
		std::vector<IgniteParamList> songParams;
		IgniteParamList params;

		std::vector<IgniteParamList> sceneList;
		std::vector<IgniteParamList> sceneParamList;
		auto sceneCount = 0;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Requested data for Song:  " + String(gSongs[index].name));
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)index);
		selected = index;

		if (!gSongs[index].songLoaded)
		{
			gSongs.getReference(index).songLoaded = true;
			songParams.clear();
			sceneList.clear();

			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "song not loaded :  " + String(gSongs[index].name));
			params.param = (paramId_t)SongParam::paramTrackCountCmd;
			songParams.push_back(params);
			songParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, songParams);
			gSongs.getReference(index).num_of_tracks = ((int(songParams[0].MsgValue.get<std::uint8_t>()) + 15) / 16);
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Number of Tracks in Song : " + String(gSongs[index].name) + " is: " + String(gSongs.getReference(index).num_of_tracks));

			params.param = (paramId_t)SceneParam::paramSceneCount;
			sceneList.push_back(params);
			sceneList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, sceneList);
			sceneCount = sceneList[0].MsgValue.get<std::uint32_t>();

			if (sceneCount > 0)
			{
				sceneList.clear();
				params.param = (paramId_t)SceneParam::paramSceneNameList;
				params.startPos = 0;
				params.Count = 10;
				sceneList.push_back(params);
				sceneList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, sceneList);

				for (auto j = 0; j < sceneCount; j++)
					IgniteScene::getAppSceneInstance()->addScene(false, sceneList[j].MsgValue.get<std::string>());

				setSongData(mt::Ignite::songsDataIndex::SELECTED_SCENE, 0, false);
				setSongData(mt::Ignite::songsDataIndex::CURRENT_SCENE, 0, false);

				if (sceneCount > 1)
					setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, 1, false);
				else
					setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, 0, false);

				AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::TRACK, true);
			}
		}

		songParams.clear();
		params.param = (paramId_t)SongParam::paramSongName;
		params.startPos = (uint8_t)index;
		songParams.push_back(params);

		params.param = (paramId_t)SongParam::paramSongSwing;
		params.startPos = (uint8_t)index;
		songParams.push_back(params);

		params.param = (paramId_t)SongParam::paramSongTempo;
		params.startPos = (uint8_t)index;
		songParams.push_back(params);

		params.param = (paramId_t)SongParam::paramTrackCountCmd;
		params.startPos = (uint8_t)index;
		songParams.push_back(params);

		params.param = (paramId_t)SongParam::paramApplySongSwingToScenes;
		params.startPos = (uint8_t)index;
		songParams.push_back(params);

		params.param = (paramId_t)SongParam::paramApplySongTempoToScenes;
		params.startPos = (uint8_t)index;
		songParams.push_back(params);

		songParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, songParams);

		setSongData(mt::Ignite::songsDataIndex::SONG_NAME, songParams[0].MsgValue.get<std::string>(), true);
		setSongData(mt::Ignite::songsDataIndex::SWING, songParams[1].MsgValue.get<std::uint8_t>(), true);
		setSongData(mt::Ignite::songsDataIndex::TEMPO, songParams[2].MsgValue.get<std::float_t>(), true);
		setSongData(mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX, ((int(songParams[3].MsgValue.get<std::uint8_t>()) + 15) / 16), true);
		setGlobalSwingState(songParams[4].MsgValue.get<bool>());
		setGlobalTempoState(songParams[5].MsgValue.get<bool>());
	}

	void IgniteSong::updateLables()
	{
		std::vector<IgniteParamList> songParams;
		IgniteParamList params;

		params.param = (paramId_t)SongParam::paramUndoListCmd;
		params.startPos = 0;
		songParams.push_back(params);
		songParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, songParams);

		/*_pSongView->updateLableText(mt::Ignite::songsDataIndex::TEMPO, false);
		_pSongView->updateLableText(mt::Ignite::songsDataIndex::SWING, false);
		_pSongView->updateLableText(mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX, false);
		_pSongView->updateLableText(mt::Ignite::songsDataIndex::CURRENT_SCENE, false);
		_pSongView->updateLableText(mt::Ignite::songsDataIndex::NEXT_SCENE, false);

		for (auto& j : songParams)
		{
			if (j.param == (paramId_t)SongParam::paramSongTempo)
				_pSongView->updateLableText(mt::Ignite::songsDataIndex::TEMPO, true);
			if (j.param == (paramId_t)SongParam::paramSongSwing)
				_pSongView->updateLableText(mt::Ignite::songsDataIndex::SWING, true);
			if (j.param == (paramId_t)SongParam::paramTrackCountCmd)
				_pSongView->updateLableText(mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX, true);
			if (j.param == (paramId_t)SceneParam::paramCurrentPlaySceneId)
				_pSongView->updateLableText(mt::Ignite::songsDataIndex::CURRENT_SCENE, true);
			if (j.param == (paramId_t)SceneParam::paramNextPlaySceneId)
				_pSongView->updateLableText(mt::Ignite::songsDataIndex::NEXT_SCENE, true);
		}*/
		if (songParams.size() == 1)
		{
			enableControlButton(IgniteControlOptions::SAVE, false);
			enableControlButton(IgniteControlOptions::UNDO, false);
			songDataUpdated = false;
			_pSongView->updateSongEditIndication();
		}
		_pSongView->repaint();
	}

	void IgniteSong::setSelected(int index, bool force_update)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Selected Song:  " + String(gSongs[index].name));

		auto opMode = _pSongOptCtl->getOperationMode();

		// Highlist selected row.
		if (listView.getToggleState())
		{
			for (auto& i : list)
				i->highlight(false);

			list[index]->highlight(true);

			if (((index + 1) * ROW_HEIGHT) > ((view.getViewArea().getHeight() + view.getViewPositionY()) - ROW_HEIGHT))
			{
				view.setViewPosition(view.getViewPositionX(), (view.getViewPositionY() + view.getViewArea().getHeight()) - ROW_HEIGHT);
			}
			else if ((index * ROW_HEIGHT) <= view.getViewPositionY())
			{
				view.setViewPosition(view.getViewPositionX(), (view.getViewPositionY() - view.getViewArea().getHeight()) + ROW_HEIGHT);
			}
		}

		if (!getViewStatus())
		{
			if ((list.size() > 0))
			{
				enableControlButton(IgniteControlOptions::DEL, true);
				enableControlButton(IgniteControlOptions::COPY, true);
			}
			if ((list.size() > 1))
			{
				enableControlButton(IgniteControlOptions::MOVE, true);
			}
		}

		if ((index == selected) && !songDataUpdated && !force_update)
			return;

		auto sceneComp = IgniteScene::getAppSceneInstance();

		if (((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE)) && (listView.getToggleState()) && (selected != index))
			IgniteControlOptions::GetAppOptInstance()->setPlayButtonMode();

		if (selected != index)
			enableControlButton(IgniteControlOptions::ALL, false);
		if (isPositiveAndBelow(index, list.size()))
			selected = index;

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)index);

		statusBarData.isSceneCreated = false;//reset and get data for new song
		requestDataFromEngine(selected);
		auto song_data = gSongs.getReference(selected);
		statusBarData.songIndex = selected;
		statusBarData.isSceneCreated = song_data.sceneCreated;

		if (song_data.sceneCreated)
		{
			sceneComp->refreshSceneUI();
			if ((opMode != mt::Ignite::operatioMode::PLAY_MODE) || (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE))
			{
				_pSongView->updateSceneLists(&song_data);
				IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
			}
			AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::TRACK, true);
		}
		else
		{
			updateSelectionInfo();
			AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::TRACK, false);
		}
		if (this->isVisible())
		{
			if ((opMode != mt::Ignite::operatioMode::PLAY_MODE) || (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE))
			{
				_pSongView->refreshData(selected, &song_data);

				if (view.isVisible())
				{
					if ((list.size() > 0) && !getViewStatus())
					{
						enableControlButton(IgniteControlOptions::DEL, true);
						enableControlButton(IgniteControlOptions::COPY, true);
					}

					if ((list.size() > 1))
					{
						enableControlButton(IgniteControlOptions::MOVE, true);
					}
				}
			}

			if (songDataUpdated && !IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled())
			{
				enableControlButton(IgniteControlOptions::SAVE, true);
				enableControlButton(IgniteControlOptions::UNDO, true);
			}
			else
			{
				enableControlButton(IgniteControlOptions::SAVE, false);
				enableControlButton(IgniteControlOptions::UNDO, false);
			}
			if (getViewStatus())
			{
				_pSongView->clearTapTempoData();
			}

		}
		setFpSongLedState();
	}

	String IgniteSong::getSongName()
	{
		std::vector<IgniteParamList> songNameList;
		IgniteParamList params;

		songNameList.clear();
		params.param = (paramId_t)SongParam::paramSongNameList;
		songNameList.push_back(params);
		songNameList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, songNameList);

		if (songNameList.size())
			return songNameList[songNameList.size() - 1].MsgValue.get<std::string>();
		else
			return "";
	}

	void IgniteSong::cleanup()
	{
		container.removeAllChildren();
		list.clear();
	}

	void IgniteSong::populate(int songIndex)
	{
		mt::Ignite::songData songdata;
		IgniteSongRow* row;
		String name = getSongName();

		curretSongRow++;
		songdata = gSongs[list.size()];

		if (name != "")
			songdata.name = getSongName();

		row = new IgniteSongRow(list.size(), songdata, this);
		if (songIndex != -1)
			list.insert(songIndex, row);
		else
			list.add(row);
		container.addAndMakeVisible(row);
		songCreated = statusBarData.isSongCreated = true;
		enableViewBox(true);
		if (list.size() == 1)
			refreshSongUI(true);
		else
			refreshSongUI();
	}

	void IgniteSong::refreshSongUI(bool force_refresh)
	{
		int index = 0;

		if (this->isVisible())
		{
			_PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSongNameList);
			for (auto& i : list)
			{
				i->setBounds(0, ((heightSongList + 8) * index++), widthSongList, heightSongList);
				i->onMouseDown = [this](IgniteSongRow* Row)
					{
						showPopup((Row->getSerialNumber()));
					};
			}

			// Set overall view container size....
			container.setSize(widthScreen - 20, ((index + 1) * (heightSongContainer + 8)));
			if (songCreated)
			{
				AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::SCENE, true);
				setSelected(selected, force_refresh);
			}
		}

	}

	void IgniteSong::addSong(int songIndex)
	{
		gSongs.add({ list.size() ,String("SONG-" + String(list.size() + 1)), 0, 0, 0, list.size(), (float)defTempo, (uint8_t)defSwing, 0, false });
		populate(songIndex);
		if (!addNewInList.isVisible())
		{
			_pSongView->setVisible(true);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::SONG);
		}
		else
			listView.setToggleState(true, dontSendNotification);
		addNew.setVisible(false);
		_pEmptySongComp->setVisible(false);
		listView.setVisible(true);
		songsView.setVisible(true);
		enableViewBox(true);
		setFpSongLedState();
	}

	void IgniteSong::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		if ((buttonThatWasClicked == &addNew) || (buttonThatWasClicked == &addNewInList))
		{
			auto tmpSelected = selected;
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramAddSongCmd, 0);
			if (IgniteSongConfig::getAppSongConfigInstance()->getInsertPosition() == mt::Ignite::songInsertPosition[0])
			{
				addSong(-1);
			}
			else if (IgniteSongConfig::getAppSongConfigInstance()->getInsertPosition() == mt::Ignite::songInsertPosition[1])
			{
				addSong(-1);
				setSelected(list.size() - 1);
				moveSong(tmpSelected + 1);
				setSelected(tmpSelected);
			}
		}

		else if (buttonThatWasClicked == &listView)
		{
			view.setVisible(true);
			changeSongOptionsVisibility(false);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::SONG);
		}
		else if (buttonThatWasClicked == &songsView)
		{
			view.setVisible(false);
			changeSongOptionsVisibility(true);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::SONG);
		}
	}

	void IgniteSong::getAssets()
	{
		imageaAddNewInListDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newsong-default.png"));
		imageaAddNewInListHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newsong-hover.png"));
		imageEmptyBg = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("img-empty-state-song.png"));
		songsDetailDefaultHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-default-hover.png"));
		songsDetailActiveHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-active-hover.png"));
		songsDetaiDisabledHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-active-hover.png"));
		songsListDefaultHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-default-hover.png"));
		songsListActiveHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-active-hover.png"));
		songsListDisabledHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-active-hover.png"));
		imageSongDetailDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-default.png"));
		imageSongDetailActive = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-active.png"));
		imageSongDetailHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-detail-default-hover.png"));
		imageSongListDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-default.png"));
		imageSongListActive = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-active.png"));
		imageSongListHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-song-list-hover.png"));

		_pEmptySongComp->setBackgrounImage(imageEmptyBg);
		songsView.setImage(imageSongDetailDefault, imageSongDetailActive, imageSongDetailHover);
		listView.setImage(imageSongListDefault, imageSongListActive, imageSongListHover);
		songsView.sethoverImages(songsDetailDefaultHover, songsDetailActiveHover, songsDetaiDisabledHover);
		listView.sethoverImages(songsListDefaultHover, songsListActiveHover, songsListDisabledHover);
	}

	void IgniteSong::paint(juce::Graphics& g)
	{
		if (bootupState)
			bootupState = false;
		if (addNewInList.isVisible())
		{
			if (addNewInList.getToggleState())
				addNewInList.setToggleState(false, dontSendNotification);
		}

		if (addNew.isVisible())
		{
			g.drawImage(addNew.isMouseOver() ? imageaAddNewInListHover : imageaAddNewInListDefault, ui::Rectangle<float>{ xStartAddNewBtn, yStartAddNewBtn, widthAddNewBtn, heightAddNewBtn }, RectanglePlacement::centred);
		}
	}

	void IgniteSong::resized()
	{
	}

	void IgniteSong::enableViewBox(bool state)
	{
		if (state)
		{
			viewBox.performLayout(ui::Rectangle(xStartViewBtn, yStartViewBtn, widthViewBtn, heightViewBtn));
		}
		else
		{
			viewBox.performLayout(ui::Rectangle(0, 0, 0, 0));
		}
	}

	void IgniteSong::showPopup(int index)
	{
		auto song_data = gSongs[selected];


		if (IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled())
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, selected);
			songDataUpdated = false;
			updateLables();
			setSelected(index);
			_pSongView->refreshData(selected, &song_data);
		}
		else
		{
			if (index == selected)
			{
				setSelected(index);
				_pSongView->refreshData(selected, &song_data);
				return;
			}
			prevSelected = index;
			if (_pPopUpPtr->isDialogOpen.get() != true)
			{
				_pPopUpPtr->close();
				_pPopUpPtr->setPopUpPropories("Switch Song", 604, 290);
				_pPopUpPtr->setbtnsize(184, 44);
				_pPopUpPtr->addImageButton("Cancel", "btn-cancel-174x44-default-hover.png", "btn-cancel-174x44-default.png", 2);
				_pPopUpPtr->addImageButton("Confirm Switch", "btn-confirmswitch-174x44-default.png", "btn-confirmswitch-174x44-default-hover.png", 1);
				_pPopUpPtr->addImageButton("Save and Switch", "btn-saveswitch-174x44-default.png", "btn-saveswitch-174x44-default-hover.png", 3);
				_pPopUpPtr->show();
				_pPopUpPtr->popupType = POPUP_CHANGEWARNING;
				_pPopUpPtr->isCloseButtonVisible(false);
				_pPopUpPtr->setInfoText(song_data.name + " active song unsaved data will be lost");
				_pPopUpPtr->onSelectionChanged = [this](int r)
					{
						auto song_data = gSongs[selected];
						if (r == 1)
						{
							_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramClearUndoList, selected);
							songDataUpdated = false;
							updateLables();
							setSelected(selected, true);
							setSelected(prevSelected);
						}
						if (r == 3)
						{
							_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
							_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, selected);
							songDataUpdated = false;
							updateLables();
							setSelected(selected, true);
							setSelected(prevSelected);
						}

						if (r == 2)
							listFocused = prevSelected - 1;

						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);

						_pSongView->refreshData(selected, &song_data);
					};
			}
		}
	}

	void IgniteSong::changeSongOptionsVisibility(bool bShouldShow)
	{
		mainmodePtr = AppMainmode::getMainModeInstance();
		if (_pEmptySongComp->isVisible())
			return;

		listView.setToggleState(false, dontSendNotification);
		songsView.setToggleState(false, dontSendNotification);
		listView.setVisible(songCreated);
		songsView.setVisible(songCreated);
		enableViewBox(false);

		if (!bShouldShow)
		{
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Song View:  List View");

			view.setVisible(true);
			container.setVisible(true);
			if (!addNew.isVisible())
				addNewInList.setVisible(true);
			_pSongView->setVisible(false);
			listView.setToggleState(true, dontSendNotification);
			enableViewBox(true);
			mainmodePtr->resetPadLedsOnViewChange();
			mainmodePtr->setFpEditTrackLedState();
			setFpSongLedState();
		}
		else
		{
			mainmodePtr->resetPadLedsOnViewChange();
			mainmodePtr->setFpFillAndSoloLedState();
			mainmodePtr->setFpEditTrackLedState();
			if (songCreated)
			{
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Song View:  Detailed View");

				_pSongView->setVisible(true);
				addNewInList.setVisible(false);
				view.setVisible(false);
				container.setVisible(false);
				songsView.setToggleState(true, dontSendNotification);
				enableViewBox(true);
				moveEnabled = false;
			}
			else
			{
				_pEmptySongComp->setVisible(true);
				addNew.setVisible(true);
				addNewInList.setVisible(false);
				if (isVisible())
					_pAppOptPtr->taboptionChanges(mt::Ignite::tabs::SONG, getViewStatus());
			}
			enableControlButton(IgniteControlOptions::UNDO, false);
		}
		refreshSongUI();
		auto opMode = _pSongOptCtl->getOperationMode();
		if (bShouldShow && ((opMode != mt::Ignite::operatioMode::PLAY_MODE) || (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE)))
		{
			enableControlButton(IgniteControlOptions::DEL, false);
			enableControlButton(IgniteControlOptions::COPY, false);
			enableControlButton(IgniteControlOptions::MOVE, false);
		}
	}

	void IgniteSong::setCurrentEditSceneEditTrack()
	{
		if (songCreated)
		{
			auto& gSongRef = gSongs.getReference(selected);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
			if (gSongRef.sceneCreated)
			{
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Edit scene : " + String(gSongRef.current_scene) + " Edit Track: " + String(gSongRef.scene_data[gSongRef.current_scene].current_track));
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, gSongRef.current_scene);
				IgniteTrack::GetAppTrackInstance()->updateCurrentTrackDataFromEngine(gSongRef.scene_data[gSongRef.current_scene].current_track);
				isSongSetupComplete();
			}
		}
	}

	bool IgniteSong::isSongSetupComplete()
	{
		auto recordSetupCompleted = false;
		auto playSetupCompleted = false;

		if (songCreated)
		{
			auto& gSongRef = gSongs.getReference(selected);
			if (gSongRef.sceneCreated)
			{
				for (size_t scenes = 0; scenes < gSongRef.scene_data.size(); ++scenes)
				{
					{
						for (const auto& trackId : gSongRef.scene_data[scenes].track_data)
						{

							if ((scenes == gSongRef.current_scene) /*&& (gSongRef.scene_data[scenes].current_track == i)*/)
							{
								if (trackId.instrument != " ")
								{
									if (trackId.recordState)
									{
										enableControlButton(IgniteControlOptions::REC, true);
										recordSetupCompleted = true;
									}
								}
							}
							if ((trackId.outputInstrument != " ") && (trackId.pattern != " "))
							{
								if (!trackId.trackState)
								{
									enableControlButton(IgniteControlOptions::PLAY, true);
									if (recordSetupCompleted)
										return recordSetupCompleted;
									else
										playSetupCompleted = true;
								}
							}
						}

					}
				}
			}
		}
		enableControlButton(IgniteControlOptions::PLAY, playSetupCompleted);
		enableControlButton(IgniteControlOptions::REC, recordSetupCompleted);
		return false;
	}

	bool IgniteSong::isTrackConfigured(int track_id)
	{
		auto& gSongRef = gSongs.getReference(selected);
		auto trackId = gSongRef.scene_data[gSongRef.selectedScene].track_data[track_id];
		if ((trackId.outputInstrument != " ") && (trackId.pattern != " "))
			return true;
		else
			return false;
	}

	mt::Ignite::sceneData* IgniteSong::addSceneData(mt::Ignite::sceneData& scData)
	{
		auto& gSongRef = gSongs.getReference(selected);
		gSongRef.num_scenes++;
		scData.serialNum = scData.id = gSongRef.num_scenes - 1;
		scData.song_index = selected;
		scData.num_of_track_banks = getSongData(mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX);
		gSongRef.scene_data.push_back(scData);
		gSongRef.sceneCreated = statusBarData.isSceneCreated = true;
		//_pSongView->updateSceneLists(&gSongRef);
		if (gSongRef.num_scenes == 2)
		{
			if (gSongRef.scene_data[0].songadv == 1)
				gSongRef.next = 0;
			else
				gSongRef.next = 1;
		}
		return &scData;
	}

	mt::Ignite::sceneData* IgniteSong::getSceneData(int sceneIndex)
	{
		auto& gSongRef = gSongs.getReference(selected);
		return &gSongRef.scene_data[sceneIndex];
	}

	void IgniteSong::addToSceneList(juce::OwnedArray<ui::IgniteSceneRow>* sceneArray)
	{
		auto& gSongRef = gSongs.getReference(selected);
		gSongRef.list = (juce::OwnedArray <mt::IgniteSceneRow>*)sceneArray;
	}

	void IgniteSong::removeScenesFromList()
	{
		if (songCreated)
		{
			auto& gSongRef = gSongs.getReference(selected);
			_pSongView->updateSceneLists(&gSongRef);
		}
	}

	juce::OwnedArray<IgniteSceneRow>* IgniteSong::getSceneList()
	{
		auto& gSongRef = gSongs.getReference(selected);
		if (gSongRef.sceneCreated)
			return (juce::OwnedArray <ui::IgniteSceneRow>*)gSongRef.list;
		else
			return NULL;
	}

	void IgniteSong::updateSongData(uint8_t param, uint8_t value)
	{
		if (!songCreated)
			return;

		auto& gSongRef = gSongs.getReference(selected);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " param id : " + String((int)param) + " Value: " + String(value));

		switch (param)
		{
		case (uint8_t)SongParam::paramSongSwing:
			gSongRef.swing = value;
			break;
		case (uint8_t)SceneParam::paramCurrentPlaySceneUnmutedTracks:
			sceneActiveTracks = value;
			_pSongView->updateScene(&gSongRef, sceneActiveTracks, nextSceneActiveTracks);
			break;
		case (uint8_t)SceneParam::paramNextPlaySceneUnmutedTracks:
			nextSceneActiveTracks = value;
			_pSongView->updateScene(&gSongRef, sceneActiveTracks, nextSceneActiveTracks);
			break;
		}
	}

	void IgniteSong::updateSongData(uint8_t param, const uint32_t value)
	{
		if (!songCreated)
			return;

		auto& gSongRef = gSongs.getReference(selected);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " param id : " + String((int)param) + " Value: " + String(value));

		switch (param)
		{
		case (uint8_t)SceneParam::paramCurrentPlaySceneId:
			currentScene = value;
			statusBarData.editSceneIndex = value + 1;
			setSongData(mt::Ignite::songsDataIndex::CURRENT_SCENE, (int)value, true);
			_pSongView->updateScene(&gSongRef, sceneActiveTracks, nextSceneActiveTracks);
			SceneStatusBar::GetSceneBarInstance()->onNumofBarchangedinScene(getSceneData(gSongs[selected].current_scene)->length);
			IgniteScene::getAppSceneInstance()->setCurrentPlayingScene(value);
			break;
		case (uint8_t)SceneParam::paramNextPlaySceneId:
			nextScene = value;
			statusBarData.nextSceneIndex = value + 1;
			setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, (int)value, true);
			_pSongView->updateScene(&gSongRef, sceneActiveTracks, nextSceneActiveTracks);
			break;
		case (uint8_t)SceneParam::paramSceneCount:
			setSongData(mt::Ignite::songsDataIndex::NUM_SCENES, (int)value, true);
			break;
		}

		auto sceneData = gSongs[selected].scene_data[gSongs[selected].current_scene];
		statusBarData.swing = sceneData.swing;
		statusBarData.tempo = sceneData.tempo;
		statusBarData.songadv = sceneData.songadv;
		statusBarData.currentTrack = sceneData.current_track;
		statusBarData.sceneLength = sceneData.length;
		IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
	}

	void IgniteSong::updateSongData(uint8_t param, const float value)
	{
		if (!songCreated)
			return;

		auto& gSongRef = gSongs.getReference(selected);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " param id : " + String((int)param) + " Value: " + String(value));

		switch (param)
		{
		case (uint8_t)SongParam::paramSongTempo:
			gSongRef.tempo = value;
			break;
		}
	}
	void IgniteSong::updateSongData(uint8_t param, const std::string& value)
	{
		if (!songCreated)
			return;

		auto& gSongRef = gSongs.getReference(selected);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " param id : " + String((int)param) + " Value: " + String(value));

		switch (param)
		{
		case (uint8_t)SongParam::paramSongName:
			gSongRef.name = value;
			break;
		}
	}

	void IgniteSong::setSongData(mt::Ignite::songsDataIndex item_id, uint8_t data, bool is_internal)
	{
		auto& gSongRef = gSongs.getReference(selected);
		bool mode = false;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal));
		auto opMode = _pSongOptCtl->getOperationMode();

		if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
			mode = true;

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
		switch (item_id)
		{
		case mt::Ignite::songsDataIndex::SWING:
			gSongRef.swing = data;
			if (globalSwing)
			{
				statusBarData.swing = data;
				updateSongGlobalSwingToScenes(data);
				IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
			}
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSongSwing, data, mode);
			break;
		default: break;
		}
		_pSongView->refreshData(selected, &gSongRef);

		if (IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, selected);

		list[selected]->refreshSongRowUI();
	}


	void IgniteSong::setSongData(mt::Ignite::songsDataIndex item_id, int data, bool is_internal)
	{
		auto& gSongRef = gSongs.getReference(selected);
		auto opMode = _pSongOptCtl->getOperationMode();

		//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
		switch (item_id)
		{
		case mt::Ignite::songsDataIndex::NUM_SCENES:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SceneParam::paramAddTotalScenesCmd, data);
			gSongRef.num_scenes = data;
			break;
		case mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX:
			gSongRef.num_of_tracks = data;
			if (gSongRef.sceneCreated)
			{
				for (auto i = 0; i < gSongRef.num_scenes; i++)
				{
					gSongRef.scene_data[i].num_of_track_banks = data;
					IgniteTrack::GetAppTrackInstance()->updateNumOfTracks(gSongRef.scene_data[i]);
				}
				IgniteTrack::GetAppTrackInstance()->refreshTrackUI();

			}
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramTrackCountCmd, (data * 16));
			break;
		case mt::Ignite::songsDataIndex::CURRENT_SCENE:
			if (data == -1)
				data++;
			gSongRef.current_scene = data;
			gSongRef.selectedScene = data;
			statusBarData.editSceneIndex = data + 1;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramCurrentPlaySceneId, data);

			break;
		case mt::Ignite::songsDataIndex::NEXT_SCENE:
			if (data == -1)
			{
				if (gSongs.getReference(selected).scene_data[0].songadv == 1)
					data++;
				else
					data += 2;
			}

			gSongRef.next = data;
			statusBarData.nextSceneIndex = data + 1;
			if (!is_internal)
			{
				//_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramNextPlaySceneId, data);
				if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
				{
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramManualPlayNewSceneCmd, data);
					IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
				}
			}
			break;
		case mt::Ignite::songsDataIndex::SELECTED_SONG:
			gSongRef.selectedSong = data;
			break;
		case mt::Ignite::songsDataIndex::SELECTED_SCENE:
			gSongRef.selectedScene = data;
			break;
		case mt::Ignite::songsDataIndex::LOOP_SCENE_START:
			if (data == -1)
				gSongRef.loopSceenStart = data + 2;
			else
				gSongRef.loopSceenStart = data + 1;

			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramStartScene, data);
			break;
		case mt::Ignite::songsDataIndex::LOOP_SCENE_END:
			if (data == -1)
				gSongRef.loopSceenEnd = data + 2;
			else
				gSongRef.loopSceenEnd = data + 1;

			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramEndScene, data);
			break;

		default: break;
		}

		if (IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, selected);

		_pSongView->refreshData(selected, &gSongRef);
	}

	void IgniteSong::setSongData(mt::Ignite::songsDataIndex item_id, float data, bool is_internal, bool isPlayModeData)
	{
		auto& gSongRef = gSongs.getReference(selected);
		bool mode = false;
		auto opMode = _pSongOptCtl->getOperationMode();

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal) + " isPlayModeData: " + String((int)isPlayModeData));

		if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE) || isPlayModeData)
			mode = true;

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
		switch (item_id)
		{
		case mt::Ignite::songsDataIndex::TEMPO:
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSongTempo, data, mode);
			gSongRef.tempo = data;
			if (globalTempo)
			{
				statusBarData.tempo = data;
				updateSongGlobalTempoToScenes(data);
				IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
			}
			break;
		default: break;
		}

		if (IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, selected);

		_pSongView->refreshData(selected, &gSongRef);

		list[selected]->refreshSongRowUI();
	}

	void IgniteSong::setSongData(mt::Ignite::songsDataIndex item_id, bool data, bool is_internal)
	{
		auto& gSongRef = gSongs.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String((int)data) + " is internal: " + String((int)is_internal));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, selected);
		switch (item_id)
		{
		case mt::Ignite::songsDataIndex::FILLMODE:

			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramFillModeSwitchOn, data);
			gSongRef.isFillMode = data;
			break;

		case mt::Ignite::songsDataIndex::SCENE_CREATED:
			gSongRef.sceneCreated = statusBarData.isSceneCreated = data;
			break;

		default: break;
		}

		if (IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, selected);

		_pSongView->refreshData(selected, &gSongRef);

		list[selected]->refreshSongRowUI();
	}
	void IgniteSong::setSongData(mt::Ignite::songsDataIndex item_id, String data, bool is_internal)
	{
		std::string name = data.toStdString();
		auto& gSongRef = gSongs.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " item id : " + String((int)item_id) + " Value: " + String(data) + " is internal: " + String((int)is_internal));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);

		switch (item_id)
		{
		case mt::Ignite::songsDataIndex::SONG_NAME:
			if (!is_internal)
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSongName, data.toStdString());
			gSongRef.name = data;
			break;
		default: break;
		}

		if (IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled() && !is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, selected);

		_pSongView->refreshData(selected, &gSongRef);

		list[selected]->refreshSongRowUI();
	}

	var IgniteSong::getSongData(mt::Ignite::songsDataIndex item_id)
	{
		switch (item_id)
		{
		case mt::Ignite::songsDataIndex::SONG_ID:
			return gSongs[selected].id;
		case mt::Ignite::songsDataIndex::TEMPO:
			return gSongs[selected].tempo;
		case mt::Ignite::songsDataIndex::SWING:
			return gSongs[selected].swing;
		case mt::Ignite::songsDataIndex::SONG_NAME:
			return gSongs[selected].name;
		case mt::Ignite::songsDataIndex::CURRENT_SCENE:
			return gSongs[selected].current_scene;
		case mt::Ignite::songsDataIndex::NEXT_SCENE:
			return gSongs[selected].next;
		case mt::Ignite::songsDataIndex::NUM_SCENES:
			return gSongs[selected].num_scenes;
		case mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX:
			return gSongs[selected].num_of_tracks;
		case mt::Ignite::songsDataIndex::SCENE_CREATED:
			return gSongs[selected].sceneCreated;
		case mt::Ignite::songsDataIndex::SELECTED_SONG:
			return gSongs[selected].selectedSong;
		case mt::Ignite::songsDataIndex::SELECTED_SCENE:
			return gSongs[selected].selectedScene;
		case mt::Ignite::songsDataIndex::LOOP_SCENE_START:
			return gSongs[selected].loopSceenStart;
		case mt::Ignite::songsDataIndex::LOOP_SCENE_END:
			return gSongs[selected].loopSceenEnd;
		case mt::Ignite::songsDataIndex::FILLMODE:
			return gSongs[selected].isFillMode;
		default: return -1;
		}
	}

	void IgniteSong::updateSongGlobalTempoToScenes(float data)
	{
		for (auto i = 0; i < gSongs[selected].num_scenes; i++)
		{
			getSceneData(i)->tempo = data;
		}
	}

	void IgniteSong::updateSongGlobalSwingToScenes(uint8_t data)
	{
		for (auto i = 0; i < gSongs[selected].num_scenes; i++)
		{
			getSceneData(i)->swing = data;
		}
	}

	void IgniteSong::saveGlobalSongDataOfScenes()
	{
		auto active_scene = gSongs[selected].current_scene;

		for (auto i = 0; i < gSongs[selected].num_scenes; i++)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, i);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveSceneCmd, i);
		}
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, active_scene);
	}

	void IgniteSong::deleteCurrentSong()
	{
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramDeleteSongCmd, 0);

		container.removeChildComponent(selected);
		list.remove(selected);
	}

	void IgniteSong::deleteAllSongs()
	{
		if (list.size() > 0)
		{
			int selected_songIndex = getSelected();
			if (_pPopUpPtr->isDialogOpen.get() != true)
			{
				_pPopUpPtr->close();
				_pPopUpPtr->setPopUpPropories(" Delete all Songs from this device ", 604, 290);
				_pPopUpPtr->setbtnsize(265, 44);
				_pPopUpPtr->setInfoText(" Are you sure you want to delete all songs");
				_pPopUpPtr->addImageButton("Cancel", "btn-secondary-cancel-267x44-default.png", "btn-secondary-cancel-267x44-default-hover.png", 2);
				_pPopUpPtr->addImageButton("delete", "btn-delete-267x44-default.png", "btn-delete-267x44-default-hover.png", 1);
				_pPopUpPtr->popupType = POPUP_WIPEDATA;
				_pPopUpPtr->isCloseButtonVisible(false);
				_pPopUpPtr->show();
				_pPopUpPtr->onSelectionChanged = [this, &selected_songIndex](int r)
					{
						if (r == 1)
						{
							changeSongOptionsVisibility(false);
							int song_index = list.size() - 1;
							while (list.size())
							{
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)song_index);
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramDeleteSongCmd, song_index);

								container.removeChildComponent(song_index);
								list.remove(song_index);
								gSongs.removeAndReturn(song_index);
								song_index--;
							}

							if (auto dw = getCurrentlyModalComponent())
								dw->exitModalState(0);

							songCreated = statusBarData.isSongCreated = statusBarData.isSceneCreated = false;
							selected = 0;
							addNewInList.setVisible(false);
							changeSongOptionsVisibility(true);
							AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::SCENE, false);
							AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::TRACK, false);
						}
						else
						{
							if (auto dw = getCurrentlyModalComponent())
								dw->exitModalState(0);
						}
						updateSelectionInfo();
					};
			}
		}
		enableControlButton(IgniteControlOptions::PLAY, false);
		enableControlButton(IgniteControlOptions::REC, false);
	}

	void IgniteSong::updateSelectionInfo(void)
	{
		String displayMessage1 = "Song and playback detail will appear here";
		String displayMessage2 = " ";

		if (songCreated)
		{
			displayMessage1 = "Song::[" + String((int)getSongData(mt::Ignite::songsDataIndex::SONG_ID) + 1) + "] ";
			AppStatusbar::GetAppStatusBarInstance()->textHandler.TextLabel.setJustificationType(juce::Justification::left);
			/*AppStatusbar::GetAppStatusBarInstance()->barLabel.setVisible(true);
			AppStatusbar::GetAppStatusBarInstance()->beatStepLabel.setVisible(true);
			AppStatusbar::GetAppStatusBarInstance()->timeLabel.setVisible(true);*/
		}
		else
		{
			AppStatusbar::GetAppStatusBarInstance()->textHandler.TextLabel.setJustificationType(juce::Justification::right);
			/*AppStatusbar::GetAppStatusBarInstance()->barLabel.setVisible(false);
			AppStatusbar::GetAppStatusBarInstance()->beatStepLabel.setVisible(false);
			AppStatusbar::GetAppStatusBarInstance()->timeLabel.setVisible(false);*/
		}
		AppStatusbar::GetAppStatusBarInstance()->setDisplayMessage(displayMessage1, displayMessage2);
	}

	String IgniteSong::getEditSceneName()
	{
		if (gSongs[selected].sceneCreated)
			return String(gSongs[selected].scene_data[gSongs[selected].selectedScene].id + 1);
		return "";
	}

	String IgniteSong::getCurrentSceneName()
	{
		if (gSongs[selected].sceneCreated)
			return String(gSongs[selected].scene_data[gSongs[selected].current_scene].id + 1);
		return "";
	}

	bool IgniteSong::getFillMode(void)
	{
		return gSongs[selected].isFillMode;
	}

	String IgniteSong::getNextSceneName()
	{
		if (gSongs[selected].sceneCreated)
		{
			if (gSongs[selected].num_scenes == 1)
			{
				setSongData(mt::Ignite::songsDataIndex::NEXT_SCENE, 0, true);
				return String(gSongs[selected].scene_data[gSongs[selected].current_scene].id + 1);
			}
			else
				return String(gSongs[selected].scene_data[gSongs[selected].next].id + 1);
		}
		else
			return "";
	}

	int IgniteSong::getActiveTracks()
	{
		if (gSongs[selected].sceneCreated)
		{
			if ((gSongs[selected].sceneCreated == true) && (gSongs[selected].current_scene > 0))
				return (sceneActiveTracks = gSongs[selected].scene_data[(gSongs[selected].current_scene)].activeTracks);
			else
				return (sceneActiveTracks = gSongs[selected].scene_data[0].activeTracks);
		}
		return -1;
	}

	int IgniteSong::getActiveTracksNext()
	{
		if ((gSongs[selected].sceneCreated == true) && (gSongs[selected].next > 0))
			return (nextSceneActiveTracks = gSongs[selected].scene_data[(gSongs[selected].next)].activeTracks);
		else
			return (nextSceneActiveTracks = gSongs[selected].scene_data[0].activeTracks);
	}

	void IgniteSong::copySong(IgniteCopy::copyCtlKey key, String copyName)
	{
		if (copyName.isNotEmpty())
		{
			if (key == IgniteCopy::copyCtlKey::COPY)
			{
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramCopySong, copyName.toStdString());
				updateDataFromEngine();
			}
		}

		_pSongCopyComp->exitModalState(0);
		_pSongCopyComp->setVisible(false);
	}

	void IgniteSong::resetStatusBarPlayInfo()
	{
		//String displayMessage = "Time: 00:00:00";
		//String barDisplayMessage = "Bar:Beat:Step::00:";
		//String beatStepDisplayMessage = "00:00";
		//auto statusBarComp = AppStatusbar::GetAppStatusBarInstance();

		//stopTimer();
		//statusBarComp->setTimeMessage(displayMessage);
		//statusBarComp->setBarDisplayMessage(barDisplayMessage);
		//statusBarComp->setBeatStepDisplayMessage(beatStepDisplayMessage);
	}

	void IgniteSong::setEditIndication(bool state)
	{
		if (!IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled() && state)
		{
			enableControlButton(IgniteControlOptions::SAVE, true);
			enableControlButton(IgniteControlOptions::UNDO, true);
			songDataUpdated = true;
			_pSongView->updateSongEditIndication();
		}
		else
		{
			enableControlButton(IgniteControlOptions::SAVE, false);
			enableControlButton(IgniteControlOptions::UNDO, false);
			songDataUpdated = false;
			_pSongView->updateSongEditIndication();
		}
	}

	void IgniteSong::onOptCtlButtonClicked(IgniteControlOptions::eKey option)
	{
		if (!songCreated)
			return;

		auto opMode = _pSongOptCtl->getOperationMode();

		if (option == IgniteControlOptions::SAVEALLON)
		{
			songDataUpdated = false;
			if (this->isVisible())
			{
				if (_pSongView->isVisible())
					_pSongView->updateSongEditIndication();
				/*if (listView.getToggleState())
					for (auto i = 0; i < list.size(); i++)
						list[i]->updateSongEditIndication();*/
						//updateSongEditIndication();[i].updateSongEditIndication();

			}
		}
		else if (option == IgniteControlOptions::AUTOSAVEON)
		{
			isAutoSaveEnbaled = true;
			isAutosave = true;
		}
		else if (option == IgniteControlOptions::AUTOSAVEOFF)
		{
			isAutoSaveEnbaled = false;
			isAutosave = false;
		}

		if (option == IgniteControlOptions::PLAY)
		{
			if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
			{
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)IgniteSong::getAppSongInstance()->getSelected());
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramActiveScene, (uint32_t)IgniteScene::getAppSceneInstance()->getSelected());
				//_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramFillModeSwitchOn, this->getFillMode());
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramPlaySongCmd, (uint8_t)0);
				_pSongView->sceneBoxEnable(false);
				enableControlButton(IgniteControlOptions::STOP, true);
				enableControlButton(IgniteControlOptions::REVERSE, true);
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Play Started");

			}
			else if ((opMode == mt::Ignite::operatioMode::IDLE_MODE) || (opMode == mt::Ignite::operatioMode::PAUSE_MODE))
			{
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramPauseSongCmd, (uint8_t)0);
				_pSongView->sceneBoxEnable(true);
				if (listView.getToggleState())
				{
					enableControlButton(IgniteControlOptions::DEL, true);
					enableControlButton(IgniteControlOptions::COPY, true);
					enableControlButton(IgniteControlOptions::MOVE, true);
				}
				enableControlButton(IgniteControlOptions::REVERSE, false);
				PatternStatusBar::GetPatternBarInstance()->setActivePatternBar((uint8_t)-1);
				SceneStatusBar::GetSceneBarInstance()->setActiveSceneBar((uint8_t)-1);
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Idle Mode");
			}
			IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
		}
		else if (option == IgniteControlOptions::STOP)
		{
			timeSeconds = timeMinutes = timeHours = 0;
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramStopSongCmd, (uint8_t)0);
			
			if (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramStopRecordCmd, (uint8_t)0);

			resetStatusBarPlayInfo();

			_pSongView->sceneBoxEnable(true);

			if (listView.getToggleState())
			{
				enableControlButton(IgniteControlOptions::DEL, true);
				enableControlButton(IgniteControlOptions::COPY, true);
				enableControlButton(IgniteControlOptions::MOVE, true);
			}
			PatternStatusBar::GetPatternBarInstance()->setActivePatternBar((uint8_t)-1);
			SceneStatusBar::GetSceneBarInstance()->setActiveSceneBar((uint8_t)-1);
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Play Stopped");
		}
		else if (option == IgniteControlOptions::REVERSE)
		{
			enableControlButton(IgniteControlOptions::REVERSE, false);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramReStartSongCmd, (uint8_t)0);
            juce::Component::SafePointer<IgniteSong> safe(this);
            const auto tab=AppMainmode::getMainModeInstance()->GetActiveTab();
            juce::Timer::callAfterDelay(250,[safe,tab]
            {
                if(!safe || AppMainmode::getMainModeInstance()->GetActiveTab()!=tab)return;
                const auto mode=safe->_pSongOptCtl->getOperationMode();
                if(mode==mt::Ignite::operatioMode::PLAY_MODE || mode==mt::Ignite::operatioMode::PLAY_REC_MODE)
                    safe->enableControlButton(IgniteControlOptions::REVERSE,true);
            });
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Reverse Play");
		}
		else if (option == IgniteControlOptions::REC)
		{
			enableControlButton(IgniteControlOptions::STOP, true);
			if (opMode == mt::Ignite::operatioMode::IDLE_MODE)
				resetStatusBarPlayInfo();
		}
		else
		{
			if (isVisible())
			{
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Options Key :  " + String(option));
				switch (option) {
				case IgniteControlOptions::SAVE:
					saveSong();
					break;
				case IgniteControlOptions::DEL:
					deleteSong();
					break;
				case IgniteControlOptions::UNDO:
					if (songDataUpdated)
					{
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramUndoCmd, selected);
						Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 50);
						updateLables();
						setSelected(selected, true);
					}
					break;
				case IgniteControlOptions::COPY:
					_pSongCopyComp->setLabelText(getSongData(mt::Ignite::songsDataIndex::SONG_NAME).toString());
					_pSongCopyComp->setVisible(true);
					_pSongCopyComp->runModalLoop();
					break;
				case IgniteControlOptions::MOVE:
					_pAppOptPtr->optionMove.getToggleState() ? moveEnabled = true : moveEnabled = false;
					moveEnabled ? view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all) : view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::never);
					_pAppOptPtr->enableMoveoptions(mt::Ignite::tabs::SONG, getViewStatus(), moveEnabled);
					break;
				case IgniteControlOptions::MOVETOP:
					moveSong(0);
					break;
				case IgniteControlOptions::MOVEBTM:
					moveSong(list.size());
					break;
				case IgniteControlOptions::MOVEDOWN:
					if ((getSelected() + 1) < list.size())
						moveSong(getSelected() + 1);
					break;
				case IgniteControlOptions::MOVEUP:
					if ((getSelected() - 1) >= 0)
						moveSong(getSelected() - 1);
					break;
				case IgniteControlOptions::FILLMODE:
					if (!IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled())
						songDataUpdated = true;
					//setSongData(mt::Ignite::songsDataIndex::FILLMODE, AppOptions::GetOptionsInstance()->getFillModeStatus(), false);
					break;
				case IgniteControlOptions::CONFIG:
					AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::MENU);
					AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::SONG_WINDOW);
					break;
				case IgniteControlOptions::MIDI_SONG_CONFIG:
					AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::MENU);
					AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::MIDI_WINDOW);
					break;
				case IgniteControlOptions::CVIO_SONG_CONFIG:
					AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::MENU);
					AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::CVIO_WINDOW);
					break;
				}
			}
		}
	}

	int IgniteSong::getViewStatus()
	{
		if (songsView.getToggleState())
			return 1;
		else if (listView.getToggleState())
			return 0;
		else
			return -1;
	}

	void IgniteSong::triggerButtonEvent(int index)
	{
		switch (index)
		{
		case 1:
			break;
		case 2:
			break;
		default:
			break;
		}
	}

	void IgniteSong::deleteSong()
	{
		int selected_songIndex = getSelected();

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Song Delete: " + String(gSongs[selected_songIndex].name));
		if (_pPopUpPtr->isDialogOpen.get() != true)
		{
			_pPopUpPtr->close();
			_pPopUpPtr->setPopUpPropories("Delete " + gSongs[selected_songIndex].name, 604, 290);
			_pPopUpPtr->setbtnsize(265, 44);
			_pPopUpPtr->setInfoText("Are you sure you want to delete " + gSongs[selected_songIndex].name);
			_pPopUpPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
			_pPopUpPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
			_pPopUpPtr->popupType = POPUP_WITHOKCANCEL;
			_pPopUpPtr->isCloseButtonVisible(false);
			_pPopUpPtr->show();
			_pPopUpPtr->onSelectionChanged = [this, selected_songIndex](int r)
				{
					if (r == 1)
					{
						DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Selected song Index : " + String(selected_songIndex));
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected_songIndex);
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramDeleteSongCmd, 0);
						if (gSongs[selected].sceneCreated)
						{
							IgniteTrack::GetAppTrackInstance()->trackCreated = false;
						}
						selected_songIndex ? (selected--) : (selected = 0);
						DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Selected song : Selected : " + String(selected));

						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);

						updateDataFromEngine();
						if (gSongs.size() > 0)
						{
							if (selected_songIndex == 0)
								setSelected(selected_songIndex);
							else
								setSelected(selected_songIndex - 1);
						}
						else
						{
							songCreated = statusBarData.isSongCreated = statusBarData.isSceneCreated = false;
							enableViewBox(false);
							changeSongOptionsVisibility(true);
							addNewInList.setVisible(false);
							AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::SCENE, false);
							AppMainmode::getMainModeInstance()->enableTab(mt::Ignite::tabs::TRACK, false);
							enableControlButton(IgniteControlOptions::STOP, false);
							enableControlButton(IgniteControlOptions::PLAY, false);
							enableControlButton(IgniteControlOptions::REC, false);
						}
						AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
						setFpSongLedState();
					}
					else
					{
						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);
					}
				};
		}
	}

	void IgniteSong::saveSong()
	{
		if (_pPopUpPtr->isDialogOpen.get() != true)
		{
			_pPopUpPtr->close();
			_pPopUpPtr->setPopUpPropories("Save ", 604, 290);
			_pPopUpPtr->setbtnsize(265, 44);
			_pPopUpPtr->setInfoText(String(" Save " + gSongs[selected].name + " edits? "));
			_pPopUpPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
			_pPopUpPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
			_pPopUpPtr->popupType = POPUP_WITHOKCANCEL;
			_pPopUpPtr->isCloseButtonVisible(false);
			_pPopUpPtr->show();
			_pPopUpPtr->onSelectionChanged = [this](int r)
				{
					if (r == 1)
					{
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramActiveSong, (uint32_t)selected);
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveSongCmd, selected);
						std::string name = getSongData(mt::Ignite::songsDataIndex::SONG_NAME).toString().toStdString();
						saveGlobalSongDataOfScenes();
						songDataUpdated = false;
						list[selected]->refreshSongRowUI();
					}
					updateLables();
					if (auto dw = getCurrentlyModalComponent())
						dw->exitModalState(0);
				};
		}

	}

	void IgniteSong::moveSong(juce::Point<float> position)
	{
		int index = 0;

		/* Dragging a song from top to bottom - songs will be shifted up
		   Dragging a song from bottom to top - songs will be shifted down */
		if (position.getY() > 0)
		{
			index = static_cast<int>(((selected * heightSongList) + position.getY()) / (heightSongList + 8));
		}
		else if (position.getY() < 0)
		{
			index = static_cast<int>(((selected * (heightSongList + 8)) + position.getY()) / heightSongList);
		}

		if (index < 0)
			index = 0;

		if ((index == selected))
			return;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ": Song " + String(gSongs[selected].name) + " moved to index : " + String(index));

		/* Get the song edit status */
		auto editStatus = getSongDataUpdateStatus();
		/* Reset the song edit status*/
		setSongDataUpdateStatus(false);

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramMoveSong, index);
		updateDataFromEngine();

		if ((index > 0) && (index >= list.size()))
			index = list.size() - 1;

		/* Set the song edit status */
		setSongDataUpdateStatus(editStatus);

		setSelected(index);
	}

	void IgniteSong::moveSong(int index)
	{
		if ((index == selected))
			return;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ": Song " + String(gSongs[selected].name) + " moved to index : " + String(index));

		/* Get the song edit status */
		auto editStatus = getSongDataUpdateStatus();
		/* Reset the song edit status*/
		setSongDataUpdateStatus(false);

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramMoveSong, index);
		updateDataFromEngine();
		if ((list.size() == index) && (index > 0))
			index--;

		/* Set the song edit status */
		setSongDataUpdateStatus(editStatus);

		setSelected(index);
	}

	void IgniteSong::enableControlButton(IgniteControlOptions::eKey key, bool state)
	{
		if (IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled() && ((key == IgniteControlOptions::SAVE) || (key == IgniteControlOptions::SAVE)))
		{
			songDataUpdated = false;
			return;
		}
		else
			_pSongOptCtl->enableKey(key, state);
	}

	void IgniteSong::setFpSongLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		auto currentBank = 0;
		auto songStart = 0;

		if (songCreated && gSongs.size() > 0)
		{
			for (int song = 0; song < gSongs.size(); song++)
			{
				if ((song == 0) || (song % 16) != 0)
				{
					gSongs.getReference(song).current_song_bank = currentBank;
				}
				else
				{
					currentBank = currentBank + 1;
					gSongs.getReference(song).current_song_bank = currentBank;
				}
				if (gSongs[song].current_song_bank == currentSongBank)
				{
					if (song == getSelected())
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + songStart), fpLEDPattern::EN_RGB_LED_LBLUE);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + songStart), fpLEDPattern::EN_RGB_LED_LPURPLE);

					songStart++;

					if (songStart >= 16)
						break;
				}
			}
		}
	}

	bool IgniteSong::pannelKeyHandler(paramId_t param, uint32_t keyValue)
	{
		if (!songCreated && (param != (paramId_t)FrontPannelParam::paramKey_INSERT))
			return false;

		if ((keyValue == (uint32_t)FrontPannelKeyStatus::paramKeyStatusReleased) && (param != (paramId_t)FrontPannelParam::paramKey_ROTARYBAR))
			return true;

		bool handled = false;
		itemParam = param;
		itemkeyValue = keyValue;

		if (param == (paramId_t)FrontPannelParam::paramKey_INSERT && !songCreated)
		{
			this->postCommandMessage(12);
			return true;
		}

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ": Param " + String(param) + " keyValue : " + String(keyValue));

		if (_pSongCopyComp->isVisible())
			return _pSongCopyComp->PannelKeyHandler(param, keyValue);

		if (param == (paramId_t)FrontPannelParam::paramKey_NEXT_TIE && _pSongOptCtl->getOperationMode() == mt::Ignite::operatioMode::IDLE_MODE)
		{
			if (!list.isEmpty())
			{
				if (listFocused < 0)
				{
					listFocused = 0;
				}
				else
				{
					list[listFocused]->setFocus(false, false);
					this->postCommandMessage(4);

					if (listFocused + 1 >= list.size())
						listFocused = 0;
					else
						listFocused = jmin<int>(jmax<int>((listFocused + 1), 0), (list.size() - 1));

					if ((listFocused) * 52 > (view.getViewArea().getHeight() - 52))
						view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() + 52);
				}
				list[listFocused]->setFocus(true, false);
				repaint();
			}
			return true;
		}

		if (_pPopUpPtr != nullptr && _pPopUpPtr->IsEditing() && _pPopUpPtr->popupType != POPUP_WIPEDATA)
			return _pPopUpPtr->FPKeyHandler(true, param, keyValue);

		if (optionHandlerEnabled && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
		{
			handled = PannelKeyOptionsHandler(param, keyValue);
		}
		else
		{
			if (getViewStatus())
			{
				if (list.isEmpty())
				{
					if (addNew.isVisible() == true)
					{
						addNew.setTextColour(Theme::Instance().getColour(systemOrange));

						if ((FrontPannelParam)param == FrontPannelParam::paramKey_RSELECT || ((FrontPannelParam)param == FrontPannelParam::paramKey_INSERT))
							this->postCommandMessage(2); //Add new Button
					}
				}
				else
				{
					if (param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
					{
						optionHandlerEnabled ? optionHandlerEnabled = false : optionHandlerEnabled = true;
						PannelKeyOptionsHandler(param, keyValue);
					}
					else
						handled = (_pSongView->pannelKeyHandler(param, keyValue));
				}
			}
			else if (!list.isEmpty())
			{

				if (param == (paramId_t)FrontPannelParam::paramKey_INSERT)
				{
					this->postCommandMessage(12);
					return true;
				}

				if (param == (paramId_t)FrontPannelParam::paramKey_ROTARYBAR)
				{
					if (FrountPannel::GetFrountPannelInstance()->getKeyStatus((paramId_t)FrontPannelParam::paramKey_ROTARYBAR) == FrontPannelKeyStatus::paramKeyStatusPressed)
					{
						AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
						for (int songBank = 0; songBank <= (gSongs.size() / 16); songBank++)
						{
							_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + songBank), fpLEDPattern::EN_RGB_LED_GREEN);
						}
					}
					else if (FrountPannel::GetFrountPannelInstance()->getKeyStatus((paramId_t)FrontPannelParam::paramKey_ROTARYBAR) == FrontPannelKeyStatus::paramKeyStatusReleased)
					{
						setFpSongLedState();
					}
					return true;
				}

				setFpSongLedState();

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
						auto songsInBank = 0;
						currentSongBank = (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
						int index = 0;
						AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
						for (auto song : gSongs)
						{
							if (song.current_song_bank == currentSongBank)
							{
								if (index == selected)
									_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + songsInBank), fpLEDPattern::EN_RGB_LED_LBLUE);
								else
									_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + songsInBank), fpLEDPattern::EN_RGB_LED_LPURPLE);
								songsInBank++;
							}
							if (songsInBank >= 16) break;
							index++;
						}

					}
					else if (FrountPannel::GetFrountPannelInstance()->getDeleteKeyStatus() == FrontPannelKeyStatus::paramKeyStatusOnHold)
					{
						auto selectedSong = (currentSongBank * 16) + (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
						if (gSongs.size() > selectedSong)
						{
							selected = selectedSong;
							setSelected(selected, true);
							_pSongOptCtl->PannelKeyHandler((paramId_t)FrontPannelParam::paramKey_DELETE, keyValue);
						}
						else
							return true;//nothing to do
					}
					else
					{
						auto selectedSong = (currentSongBank * 16) + (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
						if (gSongs.size() > selectedSong)
						{
							if (selected == selectedSong)
							{
								changeSongOptionsVisibility(false);
								AppOptions::GetOptionsInstance()->taboptionChanges(mt::Ignite::tabs::SONG, 0);
								changeSongOptionsVisibility(true);
							}
							else
							{
								selected = selectedSong;
								setSelected(selected, true);
							}
						}
						else
							return true;//nothing to do
					}
					view.setViewPosition(view.getViewPositionX(), selected > 7 ? (selected - 3) * ROW_HEIGHT : 0);
					break;
				}

				if (param == (paramId_t)FrontPannelParam::paramKey_COPY_NAV)
				{
					this->postCommandMessage(5);
					return true;
				}

				if (listFocused < 0)
				{
					listFocused = 0;
				}
				else
				{
					list[listFocused]->setFocus(false, false);

					switch ((FrontPannelParam)param)
					{
					case FrontPannelParam::paramKey_RMOVELEFT:
						if (list[listFocused]->isEditing())
							this->postCommandMessage(4);
						else
						{
							if (listFocused + 1 >= list.size() && addNewInList.isVisible())
							{
								addNewInList.setState(Button::ButtonState::buttonDown);
								listFocused = (list.size() - 1);
								addNewInListHiglited = true;
							}
							else
								listFocused = jmin<int>(jmax<int>((listFocused + 1), 0), (list.size() - 1));

							if (((listFocused + 3) * ROW_HEIGHT >= view.getViewPositionY()) && listFocused > 3)
								view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() + ROW_HEIGHT);
						}
						break;
					case FrontPannelParam::paramKey_RMOVERIGHT:
						if (list[listFocused]->isEditing())
							this->postCommandMessage(4);
						else
						{
							if (addNewInListHiglited)
							{
								addNewInList.setState(Button::ButtonState::buttonOver);
								addNewInListHiglited = false;
								listFocused = (list.size() - 1);
							}
							else
								listFocused = jmin<int>(jmax<int>((listFocused - 1), 0), (list.size() - 1));

							if (((listFocused - 3) * ROW_HEIGHT < view.getViewPositionY()))
								view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() - ROW_HEIGHT);
						}
						break;
					case FrontPannelParam::paramKey_PAGESHIFT:
						optionHandlerEnabled ? optionHandlerEnabled = false : optionHandlerEnabled = true;
						if (optionHandlerEnabled)
						{
							PannelKeyOptionsHandler(param, keyValue);
							for (auto i = 0; i < list.size(); i++)
								list[i]->setFocus(false, (selected == i) ? true : false);
						}
						else
						{
							PannelKeyOptionsHandler(param, keyValue);
							list[listFocused]->setFocus(true, false);
						}
						break;
					case FrontPannelParam::paramKey_RDBLSELECT:
					case FrontPannelParam::paramKey_RSELECT:
						if (keyValue == (uint32)FrontPannelKeyStatus::paramKeyStatusPressed)
						{
							if (addNewInListHiglited)
								this->postCommandMessage(2); //Add new in list Button
							else if (listFocused == list.size())
								list[listFocused]->postCommandMessage(1);
							else
								this->postCommandMessage(4);
						}
						break;
					case FrontPannelParam::paramKey_RETURN_NAV:
						if (list[listFocused]->isEditing())
							this->postCommandMessage(4);
						break;
					}
				}
				if (!optionHandlerEnabled && !addNewInListHiglited)
					list[listFocused]->setFocus(true, false);
				repaint();
			}
		}
		return handled;
	}

	bool IgniteSong::PannelKeyOptionsHandler(paramId_t param, uint32_t keyValue)
	{
		bool setFocus = true;
		bool moveBtnState = _pAppOptPtr->getBtnState(IgniteControlOptions::eKey::MOVEUP);
		if (keyFocusedItem == optionsViewItemID::NO_FOCUS)
		{
			/*1st key after Page visible */
			if (moveBtnState && !getViewStatus())
				keyFocusedItem = optionsViewItemID::SONG_MOVE_DOWN;
			else
				keyFocusedItem = optionsViewItemID::SONG_MOVE;
		}
		else
		{
			PannelSendKey(false, param, keyValue); // clear last Focus
			if (!optionHandlerEnabled)
				return true;
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				keyFocusedItem = (keyFocusedItem + 1 >= optionsViewItemID::FOCUS_MAX ? (optionsViewItemID::FOCUS_MAX) : keyFocusedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				if (moveBtnState && !getViewStatus())
					keyFocusedItem = (keyFocusedItem - 1 <= optionsViewItemID::NO_FOCUS ? (optionsViewItemID::NO_FOCUS + 1) : keyFocusedItem - 1);
				else
					keyFocusedItem = (keyFocusedItem - 1 <= optionsViewItemID::SONG_MOVE_LAST ? (optionsViewItemID::SONG_MOVE_LAST + 1) : keyFocusedItem - 1);
				break;
			}
		}
		if (setFocus)
		{
			PannelSendKey(true, param, keyValue);
		}
		return true;
	}

	void IgniteSong::PannelSendKey(bool isFocuse, paramId_t param, uint32_t keyValue)
	{
		itemFocus = isFocuse;
		switch (optionsView[keyFocusedItem])
		{
		case SONG_MOVE_DOWN:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEDOWN, isFocuse, param, keyValue);
			break;
		case SONG_MOVE_UP:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEUP, isFocuse, param, keyValue);
			break;
		case SONG_MOVE_TOP:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVETOP, isFocuse, param, keyValue);
			break;
		case SONG_MOVE_LAST:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEBTM, isFocuse, param, keyValue);
			break;
		case SONG_MOVE:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVE, isFocuse, param, keyValue);
			break;
		case SONG_UNDO:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::UNDO, isFocuse, param, keyValue);
			break;
		case SONG_COPY:
			if (!isFocuse)
				_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::COPY, isFocuse, param, keyValue);
			else
				this->postCommandMessage(3);
			break;
		case SONG_CONFIG:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::CONFIG, isFocuse, param, keyValue);
			break;
		case SONG_MIDI_CONFIG:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MIDI_SONG_CONFIG, isFocuse, param, keyValue);
			break;
		case SONG_CV_CONFIG:
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::CVIO_SONG_CONFIG, isFocuse, param, keyValue);
			break;
		default:
			break;
		}
	}
	void IgniteSong::handleCommandMessage(int id)
	{
		if (id == 1)
			list[listFocused]->onMouseDown(list[listFocused]);
		else if (id == 2)
		{
			buttonClicked(&addNew);
		}
		else if (id == 3)
			_pSongOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::COPY, itemFocus, itemParam, itemkeyValue);
		else if (id == 4)
			list[listFocused]->pannelKeyHandler(itemParam, itemkeyValue);
		else if (id == 5)
		{
			_pSongCopyComp->setLabelText(getSongData(mt::Ignite::songsDataIndex::SONG_NAME).toString());
			_pSongCopyComp->setVisible(true);
			_pSongCopyComp->runModalLoop();
		}
		else if (id == 8)
			updateDataFromEngine();
		else if (id == 12)
		{
			buttonClicked(&addNewInList);
		}
	}

	////////////////////////////////////////////////
	//   Class: Ignite Song row
	////////////////////////////////////////////////
	IgniteSongRow::IgniteSongRow(int SerialNumber, mt::Ignite::songData& Data, IgniteSong* songPtr) : songdata(Data)
	{
		_pSong = songPtr;
		srNo.makeVisible(this);
		name.makeVisibleAndAddListner(this);
		songState.makeVisible(this);

		songState.setDisplayTitle(false);
		name.setEditable(false, true, false);
		name.setDisplayTitle(false);
		songState.setJustificationType(juce::Justification::centred);
		name.setJustificationType(juce::Justification::centred);
		songState.setTextFont(font_12);
		serialNumber = String(SerialNumber + 1);
		srNo.setText(String(SerialNumber), juce::dontSendNotification);
		name.setText(String(serialNumber) + ": " + songdata.name, juce::dontSendNotification);
		name.setLableFont(font_10);
		box.items.add(FlexItem(songState).withMargin(0).withFlex(2));
		box.items.add(FlexItem(name).withMargin(0).withFlex(5));
		addMouseListener(this, true);
		getAssets();
		name.onValueChanged = [this](String value)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;

				_pSong->setSongData(mt::Ignite::songsDataIndex::SONG_NAME, value, false);
				if (_pSong->getSongData(mt::Ignite::songsDataIndex::NUM_SCENES))
					IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();

				_pSong->setEditIndication(true);
				refreshSongRowUI();
			};

		name.ValidateDataOnChange = [this](String value)
			{
				Component::BailOutChecker checker(this);
				if (checker.shouldBailOut())
					return KEY_PAD_DATA_VALIDATE_OK;

				if (_pSong->getSongData(mt::Ignite::songsDataIndex::SONG_NAME) != value)
				{
					//check for duplicate
					for (auto& i : gSongs)
					{
						if (i.name == value)
							return KEY_PAD_DATA_DUPLICATE_ERROR; // Name already present
					}
				}
				return KEY_PAD_DATA_VALIDATE_OK;
			};
	}

	IgniteSongRow::~IgniteSongRow()
	{}

	void IgniteSongRow::highlight(bool State)
	{
		State ? songState.setText("Active", dontSendNotification) : songState.setText("InActive", dontSendNotification);
		isHighlighted = State;
		repaint();
	}

	int IgniteSongRow::getSerialNumber()
	{
		return srNo.getText().getIntValue();
	}

	void IgniteSongRow::mouseDown(const MouseEvent& event)
	{
	}

	void IgniteSongRow::mouseUp(const MouseEvent& event)
	{
		if (rowIsBeingDraggedOver)
		{
			if (_pSong->getSongMoveState())
			{
				_pSong->moveSong(event.position);
			}
			rowIsBeingDraggedOver = false;
		}
		else if (onMouseDown)
			onMouseDown(this);
	}

	void IgniteSongRow::mouseDrag(const MouseEvent& event)
	{
		juce::DragAndDropContainer* dragC = juce::DragAndDropContainer::findParentDragContainerFor(this);
		if (dragC)
		{
			if (isHighlighted)
			{
				if (!dragC->isDragAndDropActive())
				{
					if (_pSong->getSongMoveState())
						dragC->startDragging("TargetSource", this);
					rowIsBeingDraggedOver = true;
				}
			}
		}
	}

	void IgniteSongRow::refreshSongRowUI()
	{
		String songNameString = _pSong->getSongData(mt::Ignite::songsDataIndex::SONG_NAME);

		songNameString = serialNumber + ": " + songNameString;
		if (_pSong->getSongDataUpdateStatus() && !_pSong->isGlobalAutosaveEnabled())
			songNameString = songNameString + "*";

		name.setText(songNameString, juce::dontSendNotification);
		songState.setText("Active", dontSendNotification);
	}

	void IgniteSongRow::labelTextChanged(Label* labelThatHasChanged)
	{
		if (labelThatHasChanged == &name)
		{
			if (!IgniteSongConfig::getAppSongConfigInstance()->isAutoSaveEnabled())
				_pSong->setSongDataUpdateStatus(true);

			_pSong->setSongData(mt::Ignite::songsDataIndex::SONG_NAME, name.getText(), false);
		}
	}

	void IgniteSongRow::getAssets()
	{
		imageSongRowActiveHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-list-item-active-hover.png"));
		imageSongRowActiveDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-list-item-active.png"));
		imageSongRowinActiveHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-list-item-inactive-hover.png"));
		imageSongRowinActiveDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-list-item-inactive.png"));
	}

	void IgniteSongRow::paint(juce::Graphics& g)
	{
		auto b = getLocalBounds().toFloat();
		String s;
		if (isHighlighted)
		{
			g.drawImage(isMouseOver ? imageSongRowActiveHover : imageSongRowActiveDefault, getLocalBounds().toFloat(), RectanglePlacement::centred);
		}
		else
		{
			g.drawImage(isMouseOver ? imageSongRowinActiveHover : imageSongRowinActiveDefault, getLocalBounds().toFloat(), RectanglePlacement::centred);
		}

	}

	void IgniteSongRow::resized()
	{
		box.performLayout(getLocalBounds());
	}

	void IgniteSongRow::setFocus(bool hasFocus, bool activeHasFocus)
	{
		if (hasFocus)
			isMouseOver = true;
		else
			isMouseOver = false;
		if (activeHasFocus)
		{
			songState.setBackgroundColour(juce::Colour(0x28AAEE));
			name.setBackgroundColour(juce::Colour(0x155781));
		}
		repaint();
	};

	bool IgniteSongRow::pannelKeyHandler(paramId_t param, uint32_t keyValue)
	{
		if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT || param == (paramId_t)FrontPannelParam::paramKey_NEXT_TIE)
		{
			onMouseDown(this);
			_pSong->setFpSongLedState();

		}
		else if (param == (paramId_t)FrontPannelParam::paramKey_RDBLSELECT)
		{
			name.editLable();
			name.PannelKeyHandler(param, keyValue);
		}
		return true;
	}

	void IgniteSongRow::mouseMove(const MouseEvent& event)
	{
#if PHY_MOUSE_OVER
		isMouseOver = true;
		repaint();
#endif
	}

	void IgniteSongRow::mouseExit(const MouseEvent& event)
	{
#if PHY_MOUSE_OVER
		isMouseOver = false;
		repaint();
#endif
	}
};
