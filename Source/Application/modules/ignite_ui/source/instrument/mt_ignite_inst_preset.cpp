/*
 * Copyright (c) - 2024 - MUSIC Tribe, MY.
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
	InstrumentPreset::InstrumentPreset(IgniteInstruments* iPtr)
	{
		instrPtr = iPtr;
		presetTitle.makeVisible(this);
		presetTitle.setJustificationType(juce::Justification::centred);

		closeBtn.makeVisible(this);
		closeBtn.addListener(this);
		closeBtn.setBounds(553, 30, 20, 20);
		closeBtn.toFront(true);

		prevPresetGroupButton.makeVisibleAndAddListner(this);
		nextPresetGroupButton.makeVisibleAndAddListner(this);
		addAndMakeVisible(&presetGroupButton);
		presetGroupName.addArray({ "Piano", "Chromatic Percussion", "Organ", "Guitar", "Bass", "Strings", "Brass", "Reed",
						   "Pipe", "Synth Lead", "Synth Pad", "Synth Effects", "Ethnic", "Percussive", "Sound effects", "Drums" });

		preset1.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset2.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset3.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset4.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset5.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset6.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset7.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset8.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset9.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset10.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset11.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset12.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset13.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset14.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset15.makeVisibleAddListnerAndAddRadioGroup(this, 45679);
		preset16.makeVisibleAddListnerAndAddRadioGroup(this, 45679);

		prevPresetGroupButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		presetGroupButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		nextPresetGroupButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset1.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset2.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset3.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset4.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset5.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset6.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset7.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset8.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset9.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset10.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset11.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset12.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset13.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset14.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset15.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		preset16.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());

		getAssets();
		setSize(604, 474);
	}

	InstrumentPreset::~InstrumentPreset()
	{
	}

	void InstrumentPreset::getAssets()
	{
		modelContainer = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("modal-container-2.png"));
		presetBgd = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-selection-bg.png"));
		devider = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("Rectangle 100509.png"));
		presetItemDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-item-bg-default.png"));
		presetItemActive = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-item-bg-active.png"));
		presetItemDefaultHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-item-bg-default-hover.png"));
		presetItemActiveHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-item-bg-active-hover.png"));
		presetItemDisabled = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-item-bg-disabled.png"));
		presetLeftCaretDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-caret-left-default.png"));
		presetLeftCaretHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-caret-left-hover.png"));
		presetLeftCaretDisabled = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-caret-left-disabled.png"));
		presetRightCaretDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-caret-right-default.png"));
		presetRightCaretHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-caret-right-hover.png"));
		presetRightCaretDisabled = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("preset-caret-right-disabled.png"));
	}

	void InstrumentPreset::paint(juce::Graphics& g)
	{
		g.drawImage(modelContainer, juce::Rectangle <float>{ (float)0, (float)0, 604, 478 }, RectanglePlacement::centred);
		g.drawImage(devider, juce::Rectangle <float>{ (float)0, (float)79, 604, 1 }, RectanglePlacement::centred);
		g.drawImage(presetBgd, ui::Rectangle<float>{ 30, 110, 544, 50 }, RectanglePlacement::centred);

		Image DrawImage = (prevPresetGroupButton.isEnabled()) ? ((prevPresetGroupButton.isMouseOver() || prevPresetGroupButton.isFocused) ? presetLeftCaretHover : presetLeftCaretDefault) : presetLeftCaretDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{ 30, 110, 34, 50 }, RectanglePlacement::centred);

		DrawImage = (nextPresetGroupButton.isEnabled()) ? ((nextPresetGroupButton.isMouseOver() || nextPresetGroupButton.isFocused) ? presetRightCaretHover : presetRightCaretDefault) : presetRightCaretDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{ 540, 110, 34, 50 }, RectanglePlacement::centred);

		DrawImage = (preset1.isEnabled()) ? (preset1.getToggleState() ? ((preset1.isMouseOver() || preset1.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset1.isMouseOver() || preset1.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{30, 168, 133, 68});

		DrawImage = (preset2.isEnabled()) ? (preset2.getToggleState() ? ((preset2.isMouseOver() || preset2.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset2.isMouseOver() || preset2.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{167, 168, 133, 68});

		DrawImage = (preset3.isEnabled()) ? (preset3.getToggleState() ? ((preset3.isMouseOver() || preset3.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset3.isMouseOver() || preset3.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{304, 168, 133, 68});

		DrawImage = (preset4.isEnabled()) ? (preset4.getToggleState() ? ((preset4.isMouseOver() || preset4.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset4.isMouseOver() || preset4.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{442, 168, 133, 68});

		DrawImage = (preset5.isEnabled()) ? (preset5.getToggleState() ? ((preset5.isMouseOver() || preset5.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset5.isMouseOver() || preset5.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{30, 240, 133, 68});

		DrawImage = (preset6.isEnabled()) ? (preset6.getToggleState() ? ((preset6.isMouseOver() || preset6.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset6.isMouseOver() || preset6.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{167, 240, 133, 68});

		DrawImage = (preset7.isEnabled()) ? (preset7.getToggleState() ? ((preset7.isMouseOver() || preset7.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset7.isMouseOver() || preset7.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{304, 240, 133, 68});

		DrawImage = (preset8.isEnabled()) ? (preset8.getToggleState() ? ((preset8.isMouseOver() || preset8.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset8.isMouseOver() || preset8.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
		g.drawImage(DrawImage, ui::Rectangle<float>{442, 240, 133, 68});

		if ((currentPresetGroup == 5) || (currentPresetGroup == 15))
		{
			DrawImage = (preset9.isEnabled()) ? (preset9.getToggleState() ? ((preset9.isMouseOver() || preset9.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset9.isMouseOver() || preset9.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
			g.drawImage(DrawImage, ui::Rectangle<float>{30, 312, 133, 68});

			DrawImage = (preset10.isEnabled()) ? (preset10.getToggleState() ? ((preset10.isMouseOver() || preset10.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset10.isMouseOver() || preset10.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
			g.drawImage(DrawImage, ui::Rectangle<float>{167, 312, 133, 68});
		}

		if (currentPresetGroup == 5)
		{
			DrawImage = (preset11.isEnabled()) ? (preset11.getToggleState() ? ((preset11.isMouseOver() || preset11.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset11.isMouseOver() || preset11.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
			g.drawImage(DrawImage, ui::Rectangle<float>{304, 312, 133, 68});

			DrawImage = (preset12.isEnabled()) ? (preset12.getToggleState() ? ((preset12.isMouseOver() || preset12.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset12.isMouseOver() || preset12.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
			g.drawImage(DrawImage, ui::Rectangle<float>{442, 312, 133, 68});

			DrawImage = (preset13.isEnabled()) ? (preset13.getToggleState() ? ((preset13.isMouseOver() || preset13.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset13.isMouseOver() || preset13.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
			g.drawImage(DrawImage, ui::Rectangle<float>{30, 385, 133, 68});

			DrawImage = (preset14.isEnabled()) ? (preset14.getToggleState() ? ((preset14.isMouseOver() || preset14.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset14.isMouseOver() || preset14.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
			g.drawImage(DrawImage, ui::Rectangle<float>{167, 385, 133, 68});

			DrawImage = (preset15.isEnabled()) ? (preset15.getToggleState() ? ((preset15.isMouseOver() || preset15.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset15.isMouseOver() || preset15.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
			g.drawImage(DrawImage, ui::Rectangle<float>{304, 385, 133, 68});

			DrawImage = (preset16.isEnabled()) ? (preset16.getToggleState() ? ((preset16.isMouseOver() || preset16.isFocused) ? presetItemActiveHover : presetItemActive) : ((preset16.isMouseOver() || preset16.isFocused) ? presetItemDefaultHover : presetItemDefault)) : presetItemDisabled;
			g.drawImage(DrawImage, ui::Rectangle<float>{442, 385, 133, 68});
		}
	}

	void InstrumentPreset::setPresetGroup(int groupIndex)
	{
		if (groupIndex < 0)
			groupIndex = 0;

		presetGroupButton.setButtonText(presetGroupName[groupIndex]);
		loadPresets(groupIndex);
	}

	void InstrumentPreset::setPresetButton(int index)
	{
		switch (index)
		{
		case 0:
			preset1.setToggleState(true, dontSendNotification);
			break;
		case 1:
			preset2.setToggleState(true, dontSendNotification);
			break;
		case 2:
			preset3.setToggleState(true, dontSendNotification);
			break;
		case 3:
			preset4.setToggleState(true, dontSendNotification);
			break;
		case 4:
			preset5.setToggleState(true, dontSendNotification);
			break;
		case 5:
			preset6.setToggleState(true, dontSendNotification);
			break;
		case 6:
			preset7.setToggleState(true, dontSendNotification);
			break;
		case 7:
			preset8.setToggleState(true, dontSendNotification);
			break;
		case 8:
			preset9.setToggleState(true, dontSendNotification);
			break;
		case 9:
			preset10.setToggleState(true, dontSendNotification);
			break;
		case 10:
			preset11.setToggleState(true, dontSendNotification);
			break;
		case 11:
			preset12.setToggleState(true, dontSendNotification);
			break;
		case 12:
			preset13.setToggleState(true, dontSendNotification);
			break;
		case 13:
			preset14.setToggleState(true, dontSendNotification);
			break;
		case 14:
			preset15.setToggleState(true, dontSendNotification);
			break;
		case 15:
			preset16.setToggleState(true, dontSendNotification);
			break;
		}
	}

	void InstrumentPreset::resized()
	{
		presetTitle.setBounds(30, 10, 544, heightNumpadTitle);

		presetGroupButton.setBounds(30, 110, 544, 50);
		prevPresetGroupButton.setBounds(30, 110, 34, 50);
		nextPresetGroupButton.setBounds(540, 110, 34, 50);
		prevPresetGroupButton.toFront(true);
		nextPresetGroupButton.toFront(true);

		preset1.setBounds({ 30, 168, 133, 68 });
		preset2.setBounds({ 167, 168, 133, 68 });
		preset3.setBounds({ 304, 168, 133, 68 });
		preset4.setBounds({ 442, 168, 133, 68 });

		preset5.setBounds({ 30, 240, 133, 68 });
		preset6.setBounds({ 167, 240, 133, 68 });
		preset7.setBounds({ 304, 240, 133, 68 });
		preset8.setBounds({ 442, 240, 133, 68 });

		preset9.setBounds({ 30, 312, 133, 68 });
		preset10.setBounds({ 167, 312, 133, 68 });
		preset11.setBounds({ 304, 312, 133, 68 });
		preset12.setBounds({ 442, 312, 133, 68 });

		preset13.setBounds({ 30, 385, 133, 68 });
		preset14.setBounds({ 167, 385, 133, 68 });
		preset15.setBounds({ 304, 385, 133, 68 });
		preset16.setBounds({ 442, 385, 133, 68 });
	}

	void InstrumentPreset::loadPresets(int presetGroup)
	{
		preset9.setVisible(false);
		preset10.setVisible(false);
		preset11.setVisible(false);
		preset12.setVisible(false);
		preset13.setVisible(false);
		preset14.setVisible(false);
		preset15.setVisible(false);
		preset16.setVisible(false);

		auto presetIndex = (presetGroup * 8);
		if (presetGroup == 5)
			presetIndex = ((presetGroup - 1) * 8) + 8;
		else if (presetGroup > 5)
			presetIndex = ((presetGroup) * 8) + 8;

		preset1.setButtonText(mt::Ignite::presetList.at(presetIndex + 0));
		preset2.setButtonText(mt::Ignite::presetList.at(presetIndex + 1));
		preset3.setButtonText(mt::Ignite::presetList.at(presetIndex + 2));
		preset4.setButtonText(mt::Ignite::presetList.at(presetIndex + 3));
		preset5.setButtonText(mt::Ignite::presetList.at(presetIndex + 4));
		preset6.setButtonText(mt::Ignite::presetList.at(presetIndex + 5));
		preset7.setButtonText(mt::Ignite::presetList.at(presetIndex + 6));
		preset8.setButtonText(mt::Ignite::presetList.at(presetIndex + 7));

		if ((presetGroup == 5) || (presetGroup == 15))
		{
			preset9.setVisible(true);
			preset10.setVisible(true);
			preset9.setButtonText(mt::Ignite::presetList.at(presetIndex + 8));
			preset10.setButtonText(mt::Ignite::presetList.at(presetIndex + 9));

			if (presetGroup == 5)
			{
				preset11.setVisible(true);
				preset12.setVisible(true);
				preset13.setVisible(true);
				preset14.setVisible(true);
				preset15.setVisible(true);
				preset16.setVisible(true);
				preset11.setButtonText(mt::Ignite::presetList.at(presetIndex + 10));
				preset12.setButtonText(mt::Ignite::presetList.at(presetIndex + 11));
				preset13.setButtonText(mt::Ignite::presetList.at(presetIndex + 12));
				preset14.setButtonText(mt::Ignite::presetList.at(presetIndex + 13));
				preset15.setButtonText(mt::Ignite::presetList.at(presetIndex + 14));
				preset16.setButtonText(mt::Ignite::presetList.at(presetIndex + 15));
			}
		}
	}

	void InstrumentPreset::sendPreset(int preset, bool sendToEngine)
	{
		auto presetIndex = (currentPresetGroup * 8);
		if (currentPresetGroup > 5)
			presetIndex = (currentPresetGroup * 8) + 8;

		presetIndex += preset + 1;
		DBG("Preset index == " + String(presetIndex));

		if ((int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX) != presetIndex)
		{
			if (!instrPtr->autoSaveState)
			{
				instrPtr->enableControlButton(IgniteControlOptions::SAVE, true);
				instrPtr->enableControlButton(IgniteControlOptions::UNDO, true);
			}
			instrPtr->setInstrumentData(mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX, presetIndex, !sendToEngine);
		}
	}

	void InstrumentPreset::refreshData()
	{
		auto presetIndex = (int)instrPtr->getInstrumentData(mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX) - 1;
        if (presetIndex >= 0x40000000)
        {
            currentPresetGroup = 0;
            setPresetGroup(0);
            return; // opening the GM picker must not overwrite a custom bank selection
        }
		DBG(String(presetIndex));
		auto group = presetIndex / 8;
		auto index = presetIndex % 8;

		if (presetIndex >= 48 && presetIndex <= 63)
		{
			group = 6;
			index = presetIndex % 16;
		}
		else if (presetIndex >= 128)
		{
			group = 16;
			index = presetIndex % 8;
		}
		else
		{
			group = presetIndex / 8;
			index = presetIndex % 8;
		}
		if (group - 1 < 0)
			group = 1;
		currentPresetGroup = group - 1;
		setPresetGroup(group - 1);
		setPresetButton(index);
		sendPreset(index, false);
	}

	void InstrumentPreset::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		if (buttonThatWasClicked == &closeBtn)
		{
			if (juce::DialogWindow* dw = findParentComponentOfClass<juce::DialogWindow>())
				dw->exitModalState(0);
			CommonUtils::SetUtilsVisiblity(false);
		}
		else if (buttonThatWasClicked == &prevPresetGroupButton)
		{
			currentPresetGroup -= 1;
			if (currentPresetGroup < 0)
				currentPresetGroup = 0;
			presetGroupButton.setButtonText(presetGroupName[currentPresetGroup]);
			loadPresets(currentPresetGroup);
			sendPreset(0);
			setPresetButton(0);
		}
		else if (buttonThatWasClicked == &nextPresetGroupButton)
		{
			currentPresetGroup += 1;
			if (currentPresetGroup > 15)
				currentPresetGroup = 15;
			presetGroupButton.setButtonText(presetGroupName[currentPresetGroup]);
			loadPresets(currentPresetGroup);
			sendPreset(0);
			setPresetButton(0);
		}
		else if (buttonThatWasClicked == &preset1)
			sendPreset(0);
		else if (buttonThatWasClicked == &preset2)
			sendPreset(1);
		else if (buttonThatWasClicked == &preset3)
			sendPreset(2);
		else if (buttonThatWasClicked == &preset4)
			sendPreset(3);
		else if (buttonThatWasClicked == &preset5)
			sendPreset(4);
		else if (buttonThatWasClicked == &preset6)
			sendPreset(5);
		else if (buttonThatWasClicked == &preset7)
			sendPreset(6);
		else if (buttonThatWasClicked == &preset8)
			sendPreset(7);
		else if (buttonThatWasClicked == &preset9)
			sendPreset(8);
		else if (buttonThatWasClicked == &preset10)
			sendPreset(9);
		else if (buttonThatWasClicked == &preset11)
			sendPreset(10);
		else if (buttonThatWasClicked == &preset12)
			sendPreset(11);
		else if (buttonThatWasClicked == &preset13)
			sendPreset(12);
		else if (buttonThatWasClicked == &preset14)
			sendPreset(13);
		else if (buttonThatWasClicked == &preset15)
			sendPreset(14);
		else if (buttonThatWasClicked == &preset16)
			sendPreset(15);

	}
	bool InstrumentPreset::pannelKeyHandler(paramId_t param, uint32_t keyValue)
	{
		return true;
	}

	void InstrumentPreset::pannelSendKey(bool isFocuse, paramId_t param, uint32_t keyValue)
	{
	}
}
