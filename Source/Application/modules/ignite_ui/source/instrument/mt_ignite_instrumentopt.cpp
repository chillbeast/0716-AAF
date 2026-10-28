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

namespace ui
{
	IgniteInstrumentOpt::IgniteInstrumentOpt(IgniteInstruments* iPtr)
	{
		instrPtr = iPtr;
		_pInfoWindow = CommonInfoWindow::GetInfoWindowInstance();
		optPtr = IgniteControlOptions::GetAppOptInstance();

		addAndMakeVisible(midiPort);
		midiPort.updateListOptions(mt::Ignite::portList);
		midiPort.setSelected(0);
		midiPort.setDisplayFormat(CommonListBox::BoxType::withTitleBottom);
		midiPort.setValueColour(Theme::Instance().getColour(customOrange));
		midiPort.setComponentID("midiPortComp");
		midiPort.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, juce::Rectangle{ 0,0,604,600 });

		addAndMakeVisible(midiChannel);
		midiChannel.setDisplayTitle(true);
		midiChannel.setJustificationType(juce::Justification::centred);
		midiChannel.setTextColour(Theme::Instance().getColour(customOrange));
		midiChannel.setComponentID("midiChannelComp");

		addAndMakeVisible(xposeButton);
		xposeButton.addListener(this);
		xposeButton.setClickingTogglesState(true);
		xposeButton.setToggleState(false, dontSendNotification);
		xposeButton.setComponentID("xposeComp");
		xposeButton.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		xposeButton.setDisableImage("toggle-xpose-disabled.png");
		xposeButton.sethoverImages("toggle-xpose-hover.png", "toggle-xpose-active-hover.png", "toggle-xpose-disabled-1.png");

		/*addAndMakeVisible(edtrkButton);
		edtrkButton.addListener(this);
		edtrkButton.setClickingTogglesState(true);
		edtrkButton.setToggleState(false, dontSendNotification);
		edtrkButton.setComponentID("edtrkComp");
		edtrkButton.setDisableImage("toggle-edtrk-disabled.png");
		edtrkButton.sethoverImages("toggle-edtrk-hover.png", "toggle-edtrk-active-hover.png", "toggle-edtrk-disabled-1.png");
		edtrkButton.setLookAndFeel(ImageButtonLookandFeel::getLnf());*/

		addAndMakeVisible(ftsButton);
		ftsButton.addListener(this);
		ftsButton.setClickingTogglesState(true);
		ftsButton.setToggleState(false, dontSendNotification);
		ftsButton.setComponentID("ftsComp");
		ftsButton.setDisableImage("toggle-inst-fts-disabled.png");
		ftsButton.sethoverImages("toggle-inst-fts-hover.png", "toggle-inst-fts-active-hover.png", "toggle-inst-fts-disabled-1.png");
		ftsButton.setLookAndFeel(ImageButtonLookandFeel::getLnf());

		/*addAndMakeVisible(pgmChangeButton);
		pgmChangeButton.setDisplayTitle(true);
		pgmChangeButton.setJustificationType(juce::Justification::centred);
		pgmChangeButton.setTextColour(Theme::Instance().getColour(customOrange));
		ftsButton.setComponentID("pgmNumComp");

		addAndMakeVisible(cc0Button);
		cc0Button.setDisplayTitle(true);
		cc0Button.setJustificationType(juce::Justification::centred);
		cc0Button.setTextColour(Theme::Instance().getColour(customOrange));
		ftsButton.setComponentID("pgmLSBComp");

		addAndMakeVisible(cc32Button);
		cc32Button.setDisplayTitle(true);
		cc32Button.setJustificationType(juce::Justification::centred);
		cc32Button.setTextColour(Theme::Instance().getColour(customOrange));
		ftsButton.setComponentID("pgmMSBComp");

		addAndMakeVisible(pgmButton);
		pgmButton.addListener(this);
		pgmButton.setClickingTogglesState(true);
		pgmButton.setToggleState(false, dontSendNotification);
		pgmButton.setComponentID("pgmComp");
		pgmButton.sethoverImages("toggle-presendpgm-hover.png", "toggle-presendpgm-active-hover.png", "toggle-presendpgm-disabled-1.png");
		pgmButton.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		pgmButton.setDisableImage("toggle-presendpgm-disabled.png");*/

		addAndMakeVisible(presetPgm);
		presetPgm.addListener(this);
		presetPgm.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		xposeButton.setComponentID("programComp");

		instrPresetPtr = new InstrumentPreset(instrPtr);
		instrPresetPtr->setPresetGroup(0);
		addAndMakeVisible(intSoundButton);
		intSoundButton.addListener(this);
		intSoundButton.setClickingTogglesState(true);
		intSoundButton.setToggleState(false, dontSendNotification);
		intSoundButton.setComponentID("intSoundComp");
		intSoundButton.sethoverImages("toggle-internalsound-hover.png","toggle-internalsound-active-hover.png", "toggle-internalsound-disabled-1.png");
		intSoundButton.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		intSoundButton.setDisableImage("toggle-internalsound-disabled.png");

		InstrBox_colume1.items.add(FlexItem(midiPort).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume1.items.add(FlexItem(midiChannel).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume1.items.add(FlexItem(ftsButton).withMargin({ 5,8,8,0 }).withFlex(1));
		//InstrBox_colume1.items.add(FlexItem(pgmButton).withMargin({ 5,8,8,0 }).withFlex(1));

		InstrBox_colume2.items.add(FlexItem(intSoundButton).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume2.items.add(FlexItem(presetPgm).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume2.items.add(FlexItem(xposeButton).withMargin({ 0,8,8,0 }).withFlex(1));

		/*InstrBox_colume2.items.add(FlexItem(pgmChangeButton).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume2.items.add(FlexItem(cc0Button).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume2.items.add(FlexItem(cc32Button).withMargin({ 0,8,8,0 }).withFlex(1));

		InstrBox_colume3.items.add(FlexItem(intSoundButton).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume3.items.add(FlexItem(presetPgm).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume3.items.add(FlexItem(edtrkButton).withMargin({ 0,8,8,0 }).withFlex(1));

		InstrBox_colume4.items.add(FlexItem(ftsButton).withMargin({ 0,8,8,0 }).withFlex(1));
		InstrBox_colume4.items.add(FlexItem(xposeButton).withMargin({ 0,8,8,0 }).withFlex(1));*/

		midiPort.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, juce::Rectangle{ 0,0,604,450 });
		getAssets();
		midiChannel.onValueChanged = [this](int value)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			auto port = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT);
			auto channel = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL);

			if (channel != value)
			{
				if (port != -1 && !instrPtr->isChannelUsed(mt::Ignite::portList[port], value))
				{
					instrPtr->markAsChannelInUse(mt::Ignite::portList[port], value);
				}
				else
				{
					if (port != -1 && instrPtr->isMidiUsed(getPortName(port)) && instrPtr->isChannelUsed(mt::Ignite::portList[port], value))
					{
						_pInfoWindow->setTextandShow("MIDI Port:" + getPortName(port).toUpperCase() + " and " + "Channel:" + ((String)value).toUpperCase() + " already assigned to other instrument");
						if (channel > 0)
							midiChannel.setValue((float)channel);
						else
							midiChannel.setText("---", dontSendNotification);
					}
					else
					{
						if (port != -1)
							instrPtr->markAsChannelFree(mt::Ignite::portList[port], channel);
					}
					return;
				}
				instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL, value, false);
				if (!instrPtr->autoSaveState)
				{
					instrPtr->instrumentsDataUpdated = true;
					instrPtr->updateInstrEditIndication();
					//midiChannel.setName("CHANNEL*");
					instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
					instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
				}
			}
		};

		midiChannel.onPlayTimeValueChanged = [this](int value)
		{
			if ((optPtr->getOperationMode() == mt::Ignite::operatioMode::PLAY_MODE) || (optPtr->getOperationMode() == mt::Ignite::operatioMode::PLAY_REC_MODE))
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;

				auto port = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT);
				auto channel = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL);

				if (channel != value)
				{
					if (port != -1 && !instrPtr->isChannelUsed(mt::Ignite::portList[port], value))
					{
						instrPtr->markAsChannelInUse(mt::Ignite::portList[port], value);
					}
					else
					{
						if (port != -1 && instrPtr->isMidiUsed(getPortName(port)) && instrPtr->isChannelUsed(mt::Ignite::portList[port], value))
						{
							_pInfoWindow->setTextandShow("MIDI Port:" + getPortName(port).toUpperCase() + " and " + "Channel:" + ((String)value).toUpperCase() + " already assigned to other instrument");
							if (channel > 0)
								midiChannel.setValue((float)channel);
							else
								midiChannel.setText("---", dontSendNotification);
						}
						else
						{
							if (port != -1)
								instrPtr->markAsChannelFree(mt::Ignite::portList[port], channel);
						}
						return;
					}

					if (!instrPtr->autoSaveState)
					{
						instrPtr->instrumentsDataUpdated = true;
						//midiChannel.setName("CHANNEL*");
						instrPtr->updateInstrEditIndication();
						instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
						instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
					}
					instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL, value, false);
				}
			}
		};
		midiPort.onApplyValueChanged = [this](int value, const juce::String& name)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			auto port =(int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT);

			if (port != value)
			{
				if (value >= (int)InstrumentPorts::cvPort1 && value <= (int)InstrumentPorts::cvPort8)
				{
					midiChannel.setAlpha((float)0.2);
					midiChannel.setInterceptsMouseClicks(false, false);
					if(!instrPtr->isCvUsed(name))
						instrPtr->markAsCvInUse(name);
					else
					{
						DBG("CV IS not free");
						if (port == -1)
							midiPort.setSpecificString("---");
						else
							midiPort.setSelected(port);

						_pInfoWindow->setTextandShow("CV Port: " + name.toUpperCase() + " is already assigned to other instrument");
						return;
					}
				}
				else if ((value >= (int)InstrumentPorts::midiPort1 && value <= (int)InstrumentPorts::midiPort5)
					|| (value >= (int)InstrumentPorts::usbMidiPort1 && value <= (int)InstrumentPorts::usbMidiPort6))
				{
					midiChannel.setAlpha((float)1.0);
					midiChannel.setInterceptsMouseClicks(true, true);

					if (!instrPtr->isMidiUsed(name))
						instrPtr->markAsMidiInUse(name);
				}

				if (port >= (int)InstrumentPorts::cvPort1 && port <= (int)InstrumentPorts::cvPort8)
				{
					if (port != -1)
					{
						auto oldPortName = mt::Ignite::portList[port];
						instrPtr->markAsCvFree(oldPortName);
					}
				}
				else if ((value >= (int)InstrumentPorts::midiPort1 && value <= (int)InstrumentPorts::midiPort5)
					|| (value >= (int)InstrumentPorts::usbMidiPort1 && value <= (int)InstrumentPorts::usbMidiPort6))
				{
					if (port != -1)
					{
						auto oldMidiPortName = mt::Ignite::portList[port];
						instrPtr->markAsMidiFree(oldMidiPortName);
					}
				}

				if (!instrPtr->autoSaveState)
				{
					instrPtr->instrumentsDataUpdated = true;
					instrPtr->updateInstrEditIndication();
					//midiPort.setTitleText("MIDI PORT*");
					instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
					instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
				}
				instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT, value, false);
			}
		};

		/*pgmChangeButton.onValueChanged = [this](int value)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			if ((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PGM_CHANGE) != value)
			{
				instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PGM_CHANGE, value, false);
				if (!instrPtr->autoSaveState)
				{
					instrPtr->instrumentsDataUpdated = true;
					instrPtr->updateInstrEditIndication();
					instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
					instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
				}
			}
		};

		pgmChangeButton.onPlayTimeValueChanged = [this](int value)
		{
			if ((optPtr->getOperationMode() == mt::Ignite::operatioMode::PLAY_MODE) || (optPtr->getOperationMode() == mt::Ignite::operatioMode::PLAY_REC_MODE))
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;

				if ((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PGM_CHANGE) != value)
				{
					if (!instrPtr->autoSaveState)
					{
						instrPtr->instrumentsDataUpdated = true;
						instrPtr->updateInstrEditIndication();
						instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
						instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
					}
					instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PGM_CHANGE, value, false);
				}
			}
		};

		cc0Button.onValueChanged = [this](int value)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			if ((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC0) != value)
			{
				instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC0, value, false);
				if (!instrPtr->autoSaveState)
				{
					instrPtr->instrumentsDataUpdated = true;
					instrPtr->updateInstrEditIndication();
					instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
					instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
				}
				refreshInternalSoundData(intSoundButton.getToggleState());
			}
		};

		cc32Button.onValueChanged = [this](int value)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			if ((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC32) != value)
			{
				if (!instrPtr->autoSaveState)
				{
					instrPtr->instrumentsDataUpdated = true;
					instrPtr->updateInstrEditIndication();
					instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
					instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
				}
				instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC32, value, false);
				refreshInternalSoundData(intSoundButton.getToggleState());
			}
		};*/

		midiPort.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		midiChannel.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		/*cc0Button.onFocusChanged = [this](bool a)
		{
			repaint();
		};
		cc32Button.onFocusChanged = [this](bool a)
		{
			repaint();
		}*/;
	}

	IgniteInstrumentOpt::~IgniteInstrumentOpt()
	{
	}

	void IgniteInstrumentOpt::getAssets()
	{
		imageDefaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("inst-bg-container-default.png"));
		imageActiveImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("inst-bg-container-default-hover.png"));
		imageBtnActiveImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("inst-bg-container-active.png"));
		imageBtnActiveHoverImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("inst-bg-container-active-hover.png"));
	}
	void IgniteInstrumentOpt::paint(juce::Graphics& g)
	{
		g.drawImage(midiPort.getFocusState() ? imageActiveImage : imageDefaultImage, ui::Rectangle<float>{ 0, 15, 250, 95 }, RectanglePlacement::centred);
		g.drawImage(midiChannel.getFocusState() ? imageActiveImage : imageDefaultImage, ui::Rectangle<float>{ 258, 15, 250, 95 }, RectanglePlacement::centred);
		////g.drawImage(pgmChangeButton.getFocusState() ? imageActiveImage : imageDefaultImage, ui::Rectangle<float>{ 0, 118, 250, 95 }, RectanglePlacement::centred);
		////g.drawImage(cc0Button.getFocusState() ? imageActiveImage : imageDefaultImage, ui::Rectangle<float>{ 258, 118, 250, 95 }, RectanglePlacement::centred);
		////g.drawImage(cc32Button.getFocusState() ? imageActiveImage : imageDefaultImage, ui::Rectangle<float>{ 516, 118, 250, 95 }, RectanglePlacement::centred);
		Image DrawImage = (presetPgm.getToggleState() ? ((presetPgm.isMouseOver() || presetPgm.isFocused) ? imageBtnActiveHoverImage : imageBtnActiveImage) : ((presetPgm.isMouseOver() || presetPgm.isFocused) ? imageActiveImage : imageDefaultImage));
		g.drawImage(DrawImage, ui::Rectangle<float>{258, 120, 250, 95});

	}

	void IgniteInstrumentOpt::resized()
	{
		InstrBox_colume1.performLayout(ui::Rectangle(0, 10, 774, 108));
		InstrBox_colume2.performLayout(ui::Rectangle(0, 118, 774, 108));
		/*InstrBox_colume3.performLayout(ui::Rectangle(0, 220, 774, 108));
		InstrBox_colume4.performLayout(ui::Rectangle(0, 322, 516, 108));*/
	}

	void IgniteInstrumentOpt::updateLableText(mt::Ignite::instrumentsDataIndex item_id, bool state)
	{
		switch (item_id)
		{
		case mt::Ignite::instrumentsDataIndex::INSTR_PORT:
			state ? midiPort.setTitleText("MIDI PORT*") : midiPort.setTitleText("MIDI PORT");
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL:
			state ? midiChannel.setName("CHANNEL*") : midiChannel.setName("CHANNEL");
			break;
		/*case mt::Ignite::instrumentsDataIndex::INSTR_EDTRK:
			state ? edtrkButton.setButtonText("EDTRK THRU*") : edtrkButton.setButtonText("EDTRK THRU");
			break;*/
		case mt::Ignite::instrumentsDataIndex::INSTR_FTS:
			state ? ftsButton.setButtonText("FTS*") : ftsButton.setButtonText("FTS");
			break;
		/*case mt::Ignite::instrumentsDataIndex::INSTR_PGM:
			state ? pgmButton.setButtonText("PRE SEND PGM*") : pgmButton.setButtonText("PRE SEND PGM");
			break;*/
		case mt::Ignite::instrumentsDataIndex::INSTR_XPOSE:
			state ? xposeButton.setButtonText("XPOSE*") : xposeButton.setButtonText("XPOSE");
			break;
		case mt::Ignite::instrumentsDataIndex::INTERNAL_SOUND:
			state ? intSoundButton.setButtonText("INTERNAL SOUND*") : intSoundButton.setButtonText("INTERNAL SOUND");
			break;
		}
	}

	void IgniteInstrumentOpt::updateAllLablesText(bool state)
	{
		state ? midiPort.setTitleText("MIDI PORT*") : midiPort.setTitleText("MIDI PORT");
		state ? midiChannel.setName("CHANNEL*") : midiChannel.setName("CHANNEL");
		//state ? edtrkButton.setButtonText("EDTRK THRU*") : edtrkButton.setButtonText("EDTRK THRU");
		state ? ftsButton.setButtonText("FTS*") : ftsButton.setButtonText("FTS");
		//state ? pgmButton.setButtonText("PRE SEND PGM*") : pgmButton.setButtonText("PRE SEND PGM");
		state ? xposeButton.setButtonText("XPOSE*") : xposeButton.setButtonText("XPOSE");
		state ? intSoundButton.setButtonText("INTERNAL SOUND*") : intSoundButton.setButtonText("INTERNAL SOUND");
	}

	void IgniteInstrumentOpt::updateMidiPorts()
	{
		midiPort.updateListOptions(mt::Ignite::portList);
	}

	void IgniteInstrumentOpt::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		instrPtr->instrumentsDataUpdated = true;

		if (buttonThatWasClicked == &xposeButton)
		{
			instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_XPOSE, xposeButton.getToggleState(), false);

		}
		/*else if (buttonThatWasClicked == &edtrkButton)
		{
			instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_EDTRK, edtrkButton.getToggleState(), false);

		}*/
		else if (buttonThatWasClicked == &ftsButton)
		{
			instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_FTS, ftsButton.getToggleState(), false);
		}
		/*else if (buttonThatWasClicked == &pgmButton)
		{
			instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PGM, pgmButton.getToggleState(), false);

		}*/
		else if (buttonThatWasClicked == &intSoundButton)
		{
			instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::INTERNAL_SOUND, intSoundButton.getToggleState(), false);
			refreshInternalSoundData(intSoundButton.getToggleState());
		}
		else if (buttonThatWasClicked == &presetPgm)
		{
			currentPreset = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX);
			instrPresetPtr->refreshData();
			dialogViewPort.setBounds(0, 0, 604, 472);
			dialogViewPort.setViewedComponent(instrPresetPtr, false);
			CommonUtils::showCustomDialog("PRESET", &dialogViewPort, [this](int r)
				{
					if (r == 0)
					{
						instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX, currentPreset, false);
					}
				}, ui::Rectangle<int>{0, 0, 604, 560}, true);
		}
		else
		{
			instrPtr->instrumentsDataUpdated = false;
			return;
		}

		if (!instrPtr->autoSaveState)
		{
			instrPtr->updateInstrEditIndication();
			instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
			instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
		}
	}

	void IgniteInstrumentOpt::refreshInternalSoundData(bool is_internal_sound_enabled)
	{
		/*if (is_internal_sound_enabled)
		{
			program.setAlpha((float)1.0);
			midiPort.setAlpha((float)0.2);
			midiChannel.setAlpha((float)0.2);
			cc0Button.setAlpha((float)0.2);
			cc32Button.setAlpha((float)0.2);
			midiPort.setInterceptsMouseClicks(false, false);
			midiChannel.setInterceptsMouseClicks(false, false);
			cc0Button.setInterceptsMouseClicks(false, false);
			cc0Button.setInterceptsMouseClicks(false, false);
			cc32Button.setInterceptsMouseClicks(false, false);
			program.setInterceptsMouseClicks(true, true);
			pgmChangeButton.setAlpha((float)0.2);
			pgmChangeButton.setInterceptsMouseClicks(false, false);
		}
		else
		{
			cc0Button.setAlpha((float)1.0);
			cc32Button.setAlpha((float)1.0);
			midiPort.setAlpha((float)1.0);
			midiChannel.setAlpha((float)1.0);
			program.setAlpha((float)0.2);
			midiPort.setInterceptsMouseClicks(true, true);
			midiChannel.setInterceptsMouseClicks(true, true);
			cc0Button.setInterceptsMouseClicks(true, true);
			cc32Button.setInterceptsMouseClicks(true, true);
			program.setInterceptsMouseClicks(false, false);
			if (((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC0) > 0) && ((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC32) > 0))
			{
				pgmChangeButton.setAlpha((float)1.0);
				pgmChangeButton.setInterceptsMouseClicks(true, true);
			}
			else
			{
				pgmChangeButton.setAlpha((float)0.2);
				pgmChangeButton.setInterceptsMouseClicks(false, false);
			}
		}*/
	}

	void IgniteInstrumentOpt::refreshData()
	{
		auto port = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT);
		(port >= 0) ? midiPort.setSelected((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT)) : midiPort.setSpecificString("---");

		if (port >= (int)InstrumentPorts::cvPort1 && port <= (int)InstrumentPorts::cvPort8)
		{
			midiChannel.setAlpha((float)0.2);
			midiChannel.setInterceptsMouseClicks(false, false);
		}
		else if ((port >= (int)InstrumentPorts::midiPort1 && port <= (int)InstrumentPorts::midiPort5)
			|| (port >= (int)InstrumentPorts::usbMidiPort1 && port <= (int)InstrumentPorts::usbMidiPort6) || port == -1)
		{
			midiChannel.setAlpha((float)1.0);
			midiChannel.setInterceptsMouseClicks(true, true);
		}

		auto channel = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL);
		((int)channel > 0) ? midiChannel.setText((String)channel, dontSendNotification) : midiChannel.setText("---", dontSendNotification);

		//edtrkButton.setToggleState((bool)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_EDTRK), dontSendNotification);
		xposeButton.setToggleState((bool)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_XPOSE), dontSendNotification);
		ftsButton.setToggleState((bool)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_FTS), dontSendNotification);

		/*((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PGM_CHANGE) > 0) ?
			pgmChangeButton.setText(instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PGM_CHANGE).toString(), dontSendNotification) : pgmChangeButton.setText("---", dontSendNotification);

		((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC0) > 0) ?
			cc0Button.setText(instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC0).toString(), dontSendNotification) : cc0Button.setText("---", dontSendNotification);

		((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC32) > 0) ?
			cc32Button.setText(instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CC32).toString(), dontSendNotification) : cc32Button.setText("---", dontSendNotification);

		pgmButton.setToggleState((bool)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PGM), dontSendNotification);*/

		(instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INTERNAL_SOUND)) ? refreshInternalSoundData(true) : refreshInternalSoundData(false);
		intSoundButton.setToggleState((bool)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::INTERNAL_SOUND), dontSendNotification);
		auto presetIndex = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX) - 1;
		if (presetIndex < 0)
			presetIndex = 0;
		if (presetIndex > 137)
			presetIndex = 137;
		presetPgm.setButtonText("PRESET \n" + mt::Ignite::presetList.at(presetIndex));
        const auto actualProgram = int(instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX)) - 1;
        if (actualProgram >= 0x40000000)
            if (auto* engine = static_cast<Ignite::IgniteEngine*>(_PUiVirtualEngine.get()))
                presetPgm.setButtonText("PRESET \n" + juce::String(engine->songManager->desktopPresetName(actualProgram)));
	}

	int IgniteInstrumentOpt::isEditing()
	{
		int index = 0;
		midiPort.IsEditing() ? index = 1 : 0;
		midiChannel.IsEditing() ? index = 2 : 0;
		/*pgmChangeButton.IsEditing() ? index = 4 : 0;
		cc0Button.IsEditing() ? index = 5 : 0;
		cc32Button.IsEditing() ? index = 6 : 0;*/
		return index;
	}
	bool IgniteInstrumentOpt::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		bool setfocus = true;
		int EditItem = InstrumentDetailsViewItemID::NO_FOCUS;
		if (KeyFocusedItem == InstrumentDetailsViewItemID::NO_FOCUS)
		{
			/*1st key after Page visible */
			KeyFocusedItem = InstrumentDetailsViewItemID::PORT;

			if (FrontPannelParam::paramKey_RETURN_NAV == (FrontPannelParam)param)
				AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::MENU_WINDOW);
		}
		else
		{
			EditItem = isEditing();
			if (EditItem)
			{
				if (KeyFocusedItem != EditItem)
				{
					PannelSendKey(false, param, KeyValue);
					KeyFocusedItem = EditItem;
				}
				setfocus = true;
			}
			else
			{
				PannelSendKey(false, param, KeyValue); // clear last Focus
				if (instrPtr->option_handler_focus != instrPtr->INST_DETAIL)
					return true;
				switch ((FrontPannelParam)param)
				{
				case FrontPannelParam::paramKey_RMOVELEFT:
					KeyFocusedItem = (KeyFocusedItem + 1 >= InstrumentDetailsViewItemID::FOCUS_MAX ? (InstrumentDetailsViewItemID::FOCUS_MAX) : KeyFocusedItem + 1);
					break;
				case FrontPannelParam::paramKey_RMOVERIGHT:
					KeyFocusedItem = (KeyFocusedItem - 1 <= InstrumentDetailsViewItemID::NO_FOCUS ? (InstrumentDetailsViewItemID::NO_FOCUS + 1) : KeyFocusedItem - 1);
					break;
				case FrontPannelParam::paramKey_RETURN_NAV:
					AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::MENU_WINDOW);
					break;
				case FrontPannelParam::paramRot_ROW:
					if (KeyValue == ROTARY_RIGHT || KeyValue == ROTARY_FASTRIGHT || KeyValue == ROTARY_SUPERFASTRIGHT)
					{
						PannelSendKey(false, (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT, 0);
						KeyFocusedItem = (KeyFocusedItem + INSTRUMENT_ROW_ITEMS >= InstrumentDetailsViewItemID::FOCUS_MAX ? ((KeyFocusedItem + INSTRUMENT_ROW_ITEMS) == (InstrumentDetailsViewItemID::FOCUS_MAX) ? (InstrumentDetailsViewItemID::FOCUS_MAX) : KeyFocusedItem) : KeyFocusedItem + INSTRUMENT_ROW_ITEMS);
						PannelSendKey(true, (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT, 0);
					}
					else if (KeyValue == ROTARY_LEFT || KeyValue == ROTARY_FASTLEFT || KeyValue == ROTARY_SUPERFASTLEFT)
					{
						PannelSendKey(false, (paramId_t)FrontPannelParam::paramKey_RMOVELEFT, 0);
						KeyFocusedItem = (KeyFocusedItem - INSTRUMENT_ROW_ITEMS <= InstrumentDetailsViewItemID::NO_FOCUS ? (KeyFocusedItem) : KeyFocusedItem - INSTRUMENT_ROW_ITEMS);
						PannelSendKey(true, (paramId_t)FrontPannelParam::paramKey_RMOVELEFT, 0);
					}
					repaint();
					setfocus = false;
					break;
				}
			}
		}
		if (setfocus)
		{
			PannelSendKey(true, param, KeyValue);
		}
		return true;
	}

	void IgniteInstrumentOpt::PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT && (KeyValue == (uint8_t)FrontPannelKeyStatus::paramKeyStatusReleased))
			return;

		switch (InstrumentDetailsView[KeyFocusedItem])
		{
		case PORT:
			if (midiPort.getAlpha() == 1.0)
				midiPort.FPKeyHandler(isFocuse, param, KeyValue);
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					KeyFocusedItem++;
				else
					KeyFocusedItem--;

				PannelSendKey(isFocuse, param, KeyValue);
			}
			break;
		case CHANNEL:
			if (midiChannel.getAlpha() == 1.0)
				midiChannel.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		/*case PGM_CHANGE:
			if (pgmChangeButton.getAlpha() == 1.0)
				pgmChangeButton.FPKeyHandler(isFocuse, param, KeyValue);
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					KeyFocusedItem++;
				else
					KeyFocusedItem--;

				PannelSendKey(isFocuse, param, KeyValue);
			}
			break;
		case CC0_BANKL:
			if (cc0Button.getAlpha() == 1.0)
				cc0Button.FPKeyHandler(isFocuse, param, KeyValue);
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					KeyFocusedItem++;
				else
					KeyFocusedItem--;

				PannelSendKey(isFocuse, param, KeyValue);
			}
			break;
		case CC32_BANKM:
			if (cc32Button.getAlpha() == 1.0)
				cc32Button.FPKeyHandler(isFocuse, param, KeyValue);
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					KeyFocusedItem++;
				else
					KeyFocusedItem--;

				PannelSendKey(isFocuse, param, KeyValue);
			}
			break;
		case PGM:
			pgmButton.FPKeyHandler(isFocuse, param, KeyValue);
			if ((paramId_t)param == (paramId_t)FrontPannelParam::paramKey_RSELECT && isFocuse)
			{
				buttonClicked(&pgmButton);
			}
			break;*/
		case XPOSE:
			xposeButton.FPKeyHandler(isFocuse, param, 0);
			break;
		/*case THRU:
			edtrkButton.FPKeyHandler(isFocuse, param, KeyValue);
			if ((paramId_t)param == (paramId_t)FrontPannelParam::paramKey_RSELECT && isFocuse)
			{
				buttonClicked(&edtrkButton);
			}
			break;*/
		case FTS:
			ftsButton.FPKeyHandler(isFocuse, param, 0);
			break;
		case INTERNAL_SOUND:
			intSoundButton.FPKeyHandler(isFocuse, param, 0);
			break;
		case PRESET:
			presetPgm.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		default:
			break;
		}
		repaint();
	}

	void IgniteInstrumentOpt::mouseUp(const MouseEvent& event)
	{
		auto a = event.eventComponent->getComponentID();
		int NewFocusedItem = InstrumentDetailsViewItemID::NO_FOCUS;

		if (a == "midiPortComp")					NewFocusedItem = InstrumentDetailsViewItemID::PORT;
		else if (a == "midiChannelComp")			NewFocusedItem = InstrumentDetailsViewItemID::CHANNEL;
		else if (a == "xposeComp")					NewFocusedItem = InstrumentDetailsViewItemID::XPOSE;
	//	else if (a == "edtrkComp")					NewFocusedItem = InstrumentDetailsViewItemID::THRU;
		else if (a == "ftsComp")					NewFocusedItem = InstrumentDetailsViewItemID::FTS;
		//else if (a == "pgmComp")					NewFocusedItem = InstrumentDetailsViewItemID::PGM;
		else if (a == "intSoundComp")				NewFocusedItem = InstrumentDetailsViewItemID::INTERNAL_SOUND;
		else if (a == "programComp")				NewFocusedItem = InstrumentDetailsViewItemID::PRESET;
		/*else if (a == "pgmNumComp")					NewFocusedItem = InstrumentDetailsViewItemID::PGM_CHANGE;
		else if (a == "pgmLSBComp")					NewFocusedItem = InstrumentDetailsViewItemID::CC0_BANKL;
		else if (a == "pgmMSBComp")					NewFocusedItem = InstrumentDetailsViewItemID::CC32_BANKM;*/

		if (NewFocusedItem != InstrumentDetailsViewItemID::NO_FOCUS && NewFocusedItem != KeyFocusedItem)
		{
			PannelSendKey(false, (paramId_t)FrontPannelParam::paramKey_MouseClick, 0);
			KeyFocusedItem = NewFocusedItem;
			PannelSendKey(true, (paramId_t)FrontPannelParam::paramKey_MouseClick, 0);
		}
		//#endif
	}
};