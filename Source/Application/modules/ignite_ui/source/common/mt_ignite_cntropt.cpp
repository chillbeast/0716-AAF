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

extern std::unique_ptr<IDevice> _PUiVirtualEngine;
 //==============================================================================
namespace ui
{
	IgniteControlOptions::IgniteControlOptions()
	{
		playButton.setComponentID("firmware-play");
        playButton.setName("Play / pause");
        stopButton.setComponentID("firmware-stop");
        stopButton.setName("Stop");
        revButton.setComponentID("firmware-rewind");
        revButton.setName("Rewind");
		playButton.makeVisibleAndAddListner(this);
		stopButton.makeVisibleAndAddListner(this);
		recButton.makeVisibleAndAddListner(this);
		saveButton.makeVisibleAndAddListner(this);
		delButton.makeVisibleAndAddListner(this);
		revButton.makeVisibleAndAddListner(this);

		optionsPtr = AppOptions::GetOptionsInstance();
		RecordPtr = IgniteRecordModale::GetAppRecordInstance();
		enableKey(ALL, false);

		ctlBox.items.add(FlexItem(recButton).withFlex(1));
		ctlBox.items.add(FlexItem(revButton).withFlex(1));
		ctlBox.items.add(FlexItem(stopButton).withFlex(1));
		ctlBox.items.add(FlexItem(playButton).withFlex(1));
		
		ctrBox1.items.add(FlexItem(saveButton).withFlex(1));
		ctrBox1.items.add(FlexItem(delButton).withFlex(1));

		playButton.setDisableImage("btn-play-disabled.png");
		playButton.sethoverImages("btn-play-default-hover.png", "btn-play-active-hover.png", "btn-play-disabled.png");

		recButton.setDisableImage("btn-record-disabled.png");
		recButton.sethoverImages("btn-record-default-hover.png", "btn-record-active-hover.png","btn-record-disabled.png");

		playButton.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		stopButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		recButton.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		saveButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		delButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		revButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());

		playButton.setEnabled(false);
		recButton.setEnabled(false);
		popupPtr = CommonPopUp::getCommonPopupInstance();
		
	}


	IgniteControlOptions::~IgniteControlOptions()
	{

	}

	//Image IgniteControlOptions::getSnapShot()
	//{
	//	if (auto w = juce::TopLevelWindow::getActiveTopLevelWindow())
	//		//DBG(w);
	//		return w->createComponentSnapshot(ui::Rectangle<int>{0, 0, 1024, 600}, true);
	//	//return *(new Image());
	//}

	void IgniteControlOptions::updatePlayBtnState()
	{
		playButton.setImage("btn-play-default.png", "btn-pause-active.png", "btn-play-default-hover.png");
		playButton.setInterceptsMouseClicks(true, true);
		playButton.setAlpha(1.0f);

		switch (opMode)
		{
		case mt::Ignite::operatioMode::IDLE_MODE:
		case mt::Ignite::operatioMode::RECORD_MODE:
			playButton.setImage("btn-play-default.png", "btn-play-default.png", "btn-play-default-hover.png");
			break;
		case mt::Ignite::operatioMode::PLAY_MODE:
			playButton.setImage("btn-pause-active.png", "btn-pause-active.png", "btn-pause-active-hover.png");
			break;
		case mt::Ignite::operatioMode::PAUSE_MODE:
			playButton.setImage("btn-play-active.png", "btn-play-active.png", "btn-play-active-hover.png");
			updateRecBtnState();
			break;
		case mt::Ignite::operatioMode::PLAY_REC_MODE:
			playButton.setImage("btn-pause-active.png", "btn-pause-active.png", "btn-pause-active-hover.png");
			break;
		case mt::Ignite::operatioMode::AUDITION_MODE:
			playButton.setImage("btn-play-active.png", "btn-play-active.png", "btn-play-active-hover.png");
			playButton.setInterceptsMouseClicks(false, false);
			break;
		}
	}

	void IgniteControlOptions::updateRecBtnState()
	{
		recButton.setImage("btn-record-default.png", "btn-record-active.png", "btn-record-default-hover.png");
		recButton.setInterceptsMouseClicks(true, true);
		recButton.setAlpha(1.0f);
		
		switch (opMode)
		{
		case mt::Ignite::operatioMode::IDLE_MODE:
			if (!isSafeStopEnabled && safeStopCount == 0)
			{
				stopButton.setEnabled(false);
				stopButton.setToggleState(false, dontSendNotification);
			}
			break;
		case mt::Ignite::operatioMode::PLAY_MODE:
		case mt::Ignite::operatioMode::PAUSE_MODE:
			recButton.setImage("btn-record-default.png", "btn-record-default.png", "btn-record-default-hover.png");
			break;
		case mt::Ignite::operatioMode::RECORD_MODE:
		case mt::Ignite::operatioMode::PLAY_REC_MODE:
			recButton.setImage("btn-record-active.png", "btn-record-active.png", "btn-record-active-hover.png");
			break;
		case mt::Ignite::operatioMode::AUDITION_MODE:
			recButton.setImage("btn-record-active.png", "btn-record-active.png", "btn-record-active-hover.png");
			recButton.setInterceptsMouseClicks(false, false);
			break;
		}
	}

	void IgniteControlOptions::paint(juce::Graphics& g)
	{
		Image Draw;
		String ImageName;

		if (revButton.isEnabled())
			ImageName=revButton.getToggleState() ? (revButton.isMouseOver() || revButton.isFocused) ? "btn-reverse-active-hover.png" : "btn-reverse-active.png" : (revButton.isMouseOver() || revButton.isFocused) ? "btn-reverse-default-hover.png" : "btn-reverse-default.png";
		else
			ImageName = "btn-reverse-disabled.png";
		Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(ImageName));
		g.drawImage(Draw, ui::Rectangle<float>{98, (float)10, 60, 56}, RectanglePlacement::centred);

		if (stopButton.isEnabled())
			ImageName = stopButton.getToggleState() ? (stopButton.isMouseOver() || stopButton.isFocused) ? "btn-stop-active-hover.png" : "btn-stop-active.png" : (stopButton.isMouseOver() || stopButton.isFocused) ? "btn-stop-default-hover.png" : "btn-stop-default.png";
		else
			ImageName =  "btn-stop-disabled.png";
		Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(ImageName));
		g.drawImage(Draw, ui::Rectangle<float>{163, (float)10, 60, 56}, RectanglePlacement::centred);

		if (saveButton.isEnabled())
			ImageName = saveButton.getToggleState() ? (saveButton.isMouseOver() || saveButton.isFocused) ? "btn-save-active-hover.png" : "btn-save-active.png" : (saveButton.isMouseOver() || saveButton.isFocused) ? "btn-save-default-hover.png" : "btn-save-default.png";
		else
			ImageName = "btn-save-disabled.png";
		Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(ImageName));
		g.drawImage(Draw, ui::Rectangle<float>{315, (float)10, 60, 56}, RectanglePlacement::centred);

		if (delButton.isEnabled())
			ImageName = delButton.getToggleState() ? (delButton.isMouseOver() || delButton.isFocused) ? "btn-delete-active-hover.png" : "btn-delete-active.png" : (delButton.isMouseOver() || delButton.isFocused) ? "btn-delete-default-hover.png" : "btn-delete-default.png";
		else
			ImageName = "btn-delete-disabled.png";
		Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(ImageName));
		g.drawImage(Draw, ui::Rectangle<float>{380, (float)10, 60, 56}, RectanglePlacement::centred);

	}

	void IgniteControlOptions::resized()
	{
		ctlBox.performLayout(juce::Rectangle{ (float)33, (float)getY(), (float)260, (float)getHeight() });
		ctrBox1.performLayout(juce::Rectangle{ (float)widthspacebtwnctrl + 265, (float)getY(), (float)130, (float)getHeight() });
	}

	void IgniteControlOptions::enableKey(eKey option, bool state)
	{
		switch (option)
		{
		case PLAY:
			playButton.setEnabled(state);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_PLAY, (state ? fpLEDPattern::EN_LED_PATT_ON: fpLEDPattern::EN_LED_PATT_OFF));
			break;
		case SAVE:
			if (!isAutoSaveEnabled)
				saveButton.setEnabled(state);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SAVE, (state ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			break;
		case UNDO:
			optionsPtr->enableKey(UNDO, state);
			break;
		case COPY:
			optionsPtr->enableKey(COPY, state);
			break;
		case LOOP:
			optionsPtr->enableKey(LOOP, state);
			break;
		case MOVE:
			optionsPtr->enableKey(MOVE, state);
			break;
		case REC:
			recButton.setEnabled(state);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_REC, (state ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			break;
		case DEL:
			delButton.setEnabled(state);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_DELETE, (state ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			break;
		case STOP:
			stopButton.setEnabled(state);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_STOP, (state ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			break;
		case AUTOSAVE:
			optionsPtr->enableKey(AUTOSAVE, state);
			break;
		case REVERSE:
			revButton.setEnabled(state);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_BACKFORWARD, (state ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			break;
		case ALL:
			saveButton.setEnabled(state);
			delButton.setEnabled(state);
			revButton.setEnabled(state);
			optionsPtr->enableKey(ALL, state);
			break;
		}
	}

	void IgniteControlOptions::addOptCtlListener(Listener* listener)
	{
		listeners.add(listener);
	}

	void IgniteControlOptions::removeOptCtlListener(Listener* listener)
	{
		listeners.remove(listener);
	}

	void IgniteControlOptions::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		Component::BailOutChecker checker(this);

		if (checker.shouldBailOut())
			return;
		if (buttonThatWasClicked == &playButton)
		{
			if (playButton.getToggleState())
			{
				if(opMode == mt::Ignite::operatioMode::RECORD_MODE)
					opMode = mt::Ignite::operatioMode::PLAY_REC_MODE;
				else
					opMode = mt::Ignite::operatioMode::PLAY_MODE;
			}
			else
			{
				opMode = mt::Ignite::operatioMode::PAUSE_MODE;
			}
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(PLAY); });
			updatePlayBtnState();
		}
		else if (buttonThatWasClicked == &stopButton)
		{
			int engineState;
			playButton.setToggleState(false, dontSendNotification);
			stopButton.setToggleState(false, dontSendNotification);
			revButton.setToggleState(false, dontSendNotification);

			updateRecBtnState();
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(STOP); });

			std::vector<IgniteParamList> SettingsGeneralCnfgParams;
			IgniteParamList params;

			params.param = (paramId_t)SongParam::paramSystemState;
			SettingsGeneralCnfgParams.push_back(params);

			SettingsGeneralCnfgParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, SettingsGeneralCnfgParams);

			engineState = (int)SettingsGeneralCnfgParams[0].MsgValue.get<int>();
			SettingsGeneralCnfgParams.clear();

			if (engineState == IDLE)
			{
				opMode = mt::Ignite::operatioMode::IDLE_MODE;
				stopButton.setEnabled(false);
				revButton.setEnabled(false);
				updatePlayBtnState();
			}
		}
		else if (buttonThatWasClicked == &recButton)
		{
			if (opMode == mt::Ignite::operatioMode::AUDITION_MODE)
			{
				if (!recButton.getToggleState())
				{
					stopButton.setToggleState(false, dontSendNotification);
				}
			}
			else
			{
				if (!recButton.getToggleState())
				{
					if (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE)
						opMode = mt::Ignite::operatioMode::PLAY_MODE;
					else
						opMode = mt::Ignite::operatioMode::IDLE_MODE;
				}
				else
				{
					if (opMode == mt::Ignite::operatioMode::PLAY_MODE)
						opMode = mt::Ignite::operatioMode::PLAY_REC_MODE;
					else
						opMode = mt::Ignite::operatioMode::RECORD_MODE;
				}
			}
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(REC); });
			updateRecBtnState();
			recButton.setAlpha(1.0f);
		}
		else if (buttonThatWasClicked == &saveButton)
		{
			if (isSavePopUpOn && CommonPopUp::getCommonPopupInstance()->isDialogOpen.get())
			{
				AppMainmode::getMainModeInstance()->PannelKeyHandler(AppMainmode::getMainModeInstance()->GetActiveTab(),(paramId_t) FrontPannelParam::paramKey_RSELECT, 1);
				isSavePopUpOn = false;
			}

			if (saveButton.getToggleState())
				saveButton.setToggleState(false, dontSendNotification);
			if (!isSaveallEnabled)
			{
				isSavePopUpOn = true;
				listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(SAVE); });
			}
			else
			{
				if (popupPtr->isDialogOpen.get() != true)
				{
					popupPtr->close();
					popupPtr->setPopUpPropories("Save ", 604, 290);
					auto saveMsg = "Save all the changes?";
					popupPtr->setbtnsize(265, 44);
					popupPtr->setInfoText(String(saveMsg));
					popupPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
					popupPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
					popupPtr->popupType = POPUP_WITHOKCANCEL;
					popupPtr->isCloseButtonVisible(false);
					popupPtr->show();
					popupPtr->onSelectionChanged = [this](int r)
					{
						if (r == 1)
						{
							Component::BailOutChecker checker(this);

							if (checker.shouldBailOut())
								return 0;
							listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(SAVEALLON); });
							_PUiVirtualEngine->SaveAllData();
						}
						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);
						return 0;
						};
				}

			}
		}
		else if (buttonThatWasClicked == &delButton)
		{
			if (delButton.getToggleState())
				delButton.setToggleState(false, dontSendNotification);
			if (isDeletePopUpOn && CommonPopUp::getCommonPopupInstance()->isDialogOpen.get())
			{
				AppMainmode::getMainModeInstance()->PannelKeyHandler(AppMainmode::getMainModeInstance()->GetActiveTab(), (paramId_t)FrontPannelParam::paramKey_RSELECT, 1);
				isDeletePopUpOn = false;
			}
			else
			{
				isDeletePopUpOn = true;
				listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(DEL); });
			}
		}
		else if (buttonThatWasClicked == &revButton)
		{
			if (revButton.getToggleState())
				revButton.setToggleState(false, dontSendNotification);
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(REVERSE); });
		}

	}

	void IgniteControlOptions::onCtrlOptnClick(eKey option)
	{
		Component::BailOutChecker checker(this);

		if (checker.shouldBailOut())
			return;

		switch (option)
		{
		case ui::IgniteControlOptions::COPY:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(COPY); });
			break;
		case ui::IgniteControlOptions::UNDO:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(UNDO); });
			break;
		case ui::IgniteControlOptions::MOVE:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(MOVE); });
			break;
		case ui::IgniteControlOptions::LOOP:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(LOOP); });
			break;
		case ui::IgniteControlOptions::CONFIG:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(CONFIG); });
			break;
		case ui::IgniteControlOptions::MOVEUP:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(MOVEUP); });
			break;
		case ui::IgniteControlOptions::MOVEDOWN:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(MOVEDOWN); });
			break;
		case ui::IgniteControlOptions::MOVETOP:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(MOVETOP); });
			break;
		case ui::IgniteControlOptions::MOVEBTM:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(MOVEBTM); });
			break;
		case ui::IgniteControlOptions::AUTOSAVE:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(AUTOSAVE); });
			break;
		case ui::IgniteControlOptions::SONGLIST:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(SONGLIST); });
			break;
		case ui::IgniteControlOptions::SCENELIST:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(SCENELIST); });
			break;
		case ui::IgniteControlOptions::TRACKVIEW:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(TRACKVIEW); });
			break;
		case ui::IgniteControlOptions::PATSTEP:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(PATSTEP); });
			break;
		case ui::IgniteControlOptions::PATBAR:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(PATBAR); });
			break;
		case ui::IgniteControlOptions::PATGRID:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(PATGRID); });
			break;
		case ui::IgniteControlOptions::MIDI_GLOBAL:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(MIDI_GLOBAL); });
			break;
		case ui::IgniteControlOptions::MIDI_SONG:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(MIDI_SONG); });
			break;
		case ui::IgniteControlOptions::CV_GLOBAL:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(CV_GLOBAL); });
			break;
		case ui::IgniteControlOptions::CV_SONG:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(CV_SONG); });
			break;
		case ui::IgniteControlOptions::MIDI_SONG_CONFIG:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(MIDI_SONG_CONFIG); });
			break;
		case ui::IgniteControlOptions::CVIO_SONG_CONFIG:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(CVIO_SONG_CONFIG); });
			break;
		case ui::IgniteControlOptions::SONG_SETTING_ACTIVE:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(SONG_SETTING_ACTIVE); });
			break;
		case ui::IgniteControlOptions::SONG_SETTING_GLOBAL:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(SONG_SETTING_GLOBAL); });
			break;
		case ui::IgniteControlOptions::PATTERN:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(PATTERN); });
			break;
		case ui::IgniteControlOptions::INST:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(INST); });
			break;
		case ui::IgniteControlOptions::ALT_FN:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(ALT_FN); });
			break;
		case ui::IgniteControlOptions::TRACK_CTL:
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(TRACK_CTL); });
			break;
		default:
			break;
		}
	}
	bool IgniteControlOptions::PannelKeyHandler(eKey key, bool isFocus, paramId_t param, uint32_t KeyValue)
	{
		return optionsPtr->PannelKeyHandler(key, isFocus, param, KeyValue);
	}

	bool IgniteControlOptions::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		bool handled = true;

		if (isEditing)
		{
			RecordPtr->PannelKeyHandler(param, KeyValue);
			return true;
		}

		switch ((FrontPannelParam)param)
		{
		case FrontPannelParam::paramKey_REC:
			this->postCommandMessage(1);
			break;
		case FrontPannelParam::paramKey_BACKFORWARD:
			this->postCommandMessage(2);
			break;
		case FrontPannelParam::paramKey_STOP:
			this->postCommandMessage(3);
			break;
		case FrontPannelParam::paramKey_PLAY:
			this->postCommandMessage(4);
			break;
		case FrontPannelParam::paramKey_SAVE:
			this->postCommandMessage(5);
			break;
		case FrontPannelParam::paramKey_DELETE:
			this->postCommandMessage(6);
			break;

		}

		return handled;
	}

	void IgniteControlOptions::handleCommandMessage(int id)
	{
		if (id == 1)
		{
			if (recButton.isEnabled() && opMode != mt::Ignite::operatioMode::AUDITION_MODE)
			{
				recButton.setToggleState(recButton.getToggleState()?false:true, sendNotification);
			}
		}
		else if (id ==2)
		{
			if (revButton.isEnabled())
			{
				revButton.setToggleState(true, sendNotification);
			}
		}
		else if (id == 3)
		{
			if (stopButton.isEnabled())
			{
				stopButton.setToggleState(true, sendNotification);
			}
		}
		else if (id == 4)
		{
			if (playButton.isEnabled() && opMode != mt::Ignite::operatioMode::AUDITION_MODE)
			{
				if(playButton.getToggleState())
					playButton.setToggleState(false, sendNotification);
				else
					playButton.setToggleState(true, sendNotification);
			}
		}	
		else if (id == 5)
		{
			//if (FrountPannel::GetFrountPannelInstance()->getKeyStatus((paramId_t)FrontPannelParam::paramKey_PAGESHIFT) == FrontPannelKeyStatus::paramKeyStatusOnHold)
			if(FrountPannel::GetFrountPannelInstance()->getShiftKeyStatus())
			{
				sendSaveAllNotification();
			}
			else if (saveButton.isEnabled())
				saveButton.setToggleState(true, sendNotification);
			
		}
		else if (id == 6)
		{
			if (delButton.isEnabled())
			{
				delButton.setToggleState(false, dontSendNotification);
				sendDeleteforAllListners();
			}
		}
	}

	bool IgniteControlOptions::SaveStatusChanged(eKey option,bool state)
	{
		Component::BailOutChecker checker(this);

		if (checker.shouldBailOut())
			return 0;

		if (option == SAVEALLON)
		{
			isSaveallEnabled = true;
		}
		else if (option == SAVEALLOFF)
		{
			isSaveallEnabled = false;
		}
		else if (option == AUTOSAVEON)
		{
			isAutoSaveEnabled = true;
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(AUTOSAVEON); });
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgAutoSaveAll, true, false);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSave, true, false);
			if (saveButton.isEnabled())
			{
				saveButton.setEnabled(false);
				repaint();
			}
		}
		else if (option == AUTOSAVEOFF)
		{
			isAutoSaveEnabled = false;
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(AUTOSAVEOFF); });
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgAutoSaveAll, false, false);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSave, true, false);
		}
		return 0;
	}
	void IgniteControlOptions::sendSaveAllNotification()
	{
		Component::BailOutChecker checker(this);

		if (checker.shouldBailOut())
			return;
		listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(SAVEALLON); });
		_PUiVirtualEngine->SaveAllData();
	}

	void IgniteControlOptions::sendDeleteforAllListners()
	{
		Component::BailOutChecker checker(this);

		if (checker.shouldBailOut())
			return;
		if (isDeletePopUpOn && CommonPopUp::getCommonPopupInstance()->isDialogOpen.get())
		{
			AppMainmode::getMainModeInstance()->PannelKeyHandler(AppMainmode::getMainModeInstance()->GetActiveTab(), (paramId_t)FrontPannelParam::paramKey_RSELECT, 1);
			isDeletePopUpOn = false;
		}
		else
		{
			isDeletePopUpOn = true;
			listeners.callChecked(checker, [this](Listener& l) { l.onOptCtlButtonClicked(DEL); });
		}
	}
};
