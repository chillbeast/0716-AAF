#include "mt_ignite_bar_view.h"
#define STEP_BUTTON_SPACE 4
#define NOTE_PORT 4
namespace ui
{
	StepSequencer::StepSequencer()
	{
		addAndMakeVisible(stepControl);
		addAndMakeVisible(stepPreset);
		barIndex = 0;
		isGangModeEnabled = false;

		addAndMakeVisible(barInsert);
		barInsert.addListener(this);
		barInsert.setVisible(true);

		addAndMakeVisible(sculptBtn);
		sculptBtn.addListener(this);
		sculptBtn.setVisible(true);
		sculptBtn.setToggleable(true);
		sculptBtn.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		sculptBtn.setDisableImage("btn-link105x36-sculpt-disabled.png");
		sculptBtn.sethoverImages("btn-link105x36-sculpt-default-hover.png", "btn-link105x36-sculpt-active-hover.png", "btn-link105x36-sculpt-disabled-hover.png");
		sculptBtn.setEnabled(false);

		addAndMakeVisible(gangBtn);
		gangBtn.addListener(this);
		gangBtn.setVisible(true);
		gangBtn.setClickingTogglesState(false);
		gangBtn.setToggleable(true);
		gangBtn.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		gangBtn.sethoverImages("btn-gangedit-120x44-default-hover.png", "btn-gangedit-120x44-active-hover.png", "");

		addAndMakeVisible(imageButtonPencil);
		imageButtonPencil.addListener(this);
		imageButtonPencil.setVisible(true);
		imageButtonPencil.setClickingTogglesState(false);
		imageButtonPencil.setToggleable(true);
		imageButtonPencil.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		imageButtonPencil.sethoverImages("btn-pen-default-hover.png", "btn-pen-active-hover.png", "");

		addAndMakeVisible(gbarSync);
		gbarSync.addListener(this);
		gbarSync.setVisible(true);
		gbarSync.setClickingTogglesState(false);
		gbarSync.setToggleable(true);
		gbarSync.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		gbarSync.sethoverImages("btn-toggle176x36-gbar-default-hover.png", "btn-toggle176x36-gbar-active-hover.png", "");

		addAndMakeVisible(xposeDefeat);
		xposeDefeat.addListener(this);
		xposeDefeat.setVisible(true);
		xposeDefeat.setClickingTogglesState(false);
		xposeDefeat.setToggleable(true);
		xposeDefeat.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		xposeDefeat.sethoverImages("btn-toggle176x36-transpose-default-hover.png", "btn-toggle176x36-transpose-active-hover.png", "");

		copy.makeVisibleAndAddListner(this);
		loop.makeVisibleAndAddListner(this);
		optionUndo.makeVisibleAndAddListner(this);
		copy.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		copy.setDisableImage("btn-copy-disabled.png");
		loop.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		loop.setDisableImage("btn-loop-disabled.png");
		optionUndo.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		optionUndo.setDisableImage("btn-undo-disabled.png");

		barLoopCountLable.setEditable(false, false, false);
		barLoopCountLable.makeVisible(this);
		barLoopCountLable.setDisplayBorder(false);

		barLoopCount.makeVisible(this);
		barLoopCount.setDisplayTitle(true);
		barLoopCount.setJustificationType(juce::Justification::centredRight);
		barLoopCount.setTextColour(Theme::Instance().getColour(customOrange));

		patternRotateBtn.makeVisibleAndAddListner(this);
		leftNudgeButton.makeVisibleAndAddListner(this);
		rightNudgeButton.makeVisibleAndAddListner(this);
		addAndMakeVisible(&nudgeText);
		nudgeText.setButtonText("BAR 1/1");

		barInsert.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		patternRotateBtn.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		leftNudgeButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		nudgeText.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		rightNudgeButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());

		barOptionFlex1.items.add(FlexItem(nudgeText).withMargin(0).withFlex(1));
		barOptionFlex3.items.add(FlexItem(barLoopCount).withMargin({ 4,6,0,0 }).withFlex(1));

		barOptionFlex2.items.add(FlexItem(copy).withMargin(0).withFlex(1));
		barOptionFlex2.items.add(FlexItem(loop).withMargin(0).withFlex(1));
		barOptionFlex2.items.add(FlexItem(optionUndo).withMargin(0).withFlex(1));

		selectedIndex.resize(256);

		_pInfoWindow = CommonInfoWindow::GetInfoWindowInstance();
		optPtr = IgniteControlOptions::GetAppOptInstance();
		optPtr->addOptCtlListener(this);

		barLoopCount.onValueChanged = [this](int barRepeatCount)
		{
			auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
			_pPatternPtr->setBarData(mt::Ignite::barDataIndex::BAR_REPEAT, -1, (uint8_t)barRepeatCount, false);
			if (!IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled())
			{
				_pPatternPtr->setPatternDataUpdateStatus(true);
				_pPatternPtr->enableControlButton(IgniteControlOptions::SAVE, true);
				_pPatternPtr->enableControlButton(IgniteControlOptions::UNDO, true);
				_pPatternPtr->updatePatternEditIndication();
			}
		};

		stepControl.rightNudgeChanged.connect([this](bool nudgeRight)
			{
				IgnitePattern::getAppPatternInstance()->getNextBarAndUpdate(true);
				refreshBarUI();
				repaint();
			});

		stepControl.leftNudgeChanged.connect([this](bool nudgeLeft)
			{
				IgnitePattern::getAppPatternInstance()->getNextBarAndUpdate(false);
				refreshBarUI();
				repaint();
			});

		stepPreset.valuesChanged.connect([this](std::vector<int>& values, int16_t offset, bool flip, bool isPlayModeData, bool sliderState)
			{
				auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
				isSliderDragging = sliderState;

				if (!isPresetsEnabled)
				{
					for (auto i = 0; i < 16; i++)
					{
						if (getGateStatus(i))
						{
							values[i] = (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), i) + offset;
							if (sliderState && dragging)
							{
								prevValue[i] = (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), i);
							}
						}
					}
				}
				if (sliderState)
				{
					for (auto i = 0; i < 16; i++)
					{
						if (getGateStatus(i))
							prevValue[i] = prevValue[i] + offset;
					}
					dragging = false;
				}
				else
					dragging = true;

				if (!sliderState && !isPresetsEnabled)
				{
					for (auto i = 0; i < 16; i++)
					{
						if (getGateStatus(i))
							values[i] = prevValue[i];
					}
				}
				if (isGangModeEnabled)
					_pPatternPtr->setPresetValueForSteps(s_mapStringToParamValueType[_pPatternPtr->getActiveParamControl()], values, false, isPlayModeData);
				else
					_pPatternPtr->setPresetValueForSteps(s_mapStringToParamValueType[_pPatternPtr->getActiveParamControl()], _pPatternPtr->getPatternData(mt::Ignite::ptDataIndex::NODE_ID), values, true, isPlayModeData);
				repaint();
			});


		stepControl.controlParamChanged.connect([this](juce::String param)
			{
				updateBarData();
				auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
				_pPatternPtr->setActiveParamControl(param);
				std::fill(selectedIndex.begin(), selectedIndex.end(), 0);
				updateSliderRange();
				_pPatternPtr->setFpStepLedState();
			});

		stepControl.presetOn.connect([this](bool enable)
			{
				stepPreset.enablePresets(enable);
				isPresetsEnabled = enable;
			});

		barLoopCount.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		rangeLow = 0;
		rangeHigh = 16;
		stepPreset.setQl(16);
		getAssets();
		this->setBufferedToImage(true);
	}

	void StepSequencer::updateSliderRange()
	{
		float range;
		updateLegnthDelay();
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		if (_pPatternPtr->getActiveParamControl() == "Len:Steps")
			range = (((float)_pPatternPtr->getPatternData(mt::Ignite::ptDataIndex::BARNUM) + 1) * 16) / 2;
		else
			range = (float)(s_mapStringToParamMaxType[_pPatternPtr->getActiveParamControl()] + 1) / 2;
		stepPreset.setSliderRange(-range, range);
		repaint();
	}

	void StepSequencer::updateLableText(mt::Ignite::ptDataIndex item_id, bool state)
	{
		stepControl.updateLableText(item_id, state);
	}

	void StepSequencer::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		auto patternAutoSaveEnabled = IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled();

		if (buttonThatWasClicked == &xposeDefeat)
		{
			keyFocusedItem = barViewItemID::XPOSE_DEFEAT;
			if (xposeDefeat.getToggleState())
			{
				xposeDefeat.setToggleState(false, dontSendNotification);
			}
			else
			{
				xposeDefeat.setToggleState(true, dontSendNotification);
			}

			_pPatternPtr->setBarData(mt::Ignite::barDataIndex::TRANSPOSE_DEFEAT_STATUS, -1, xposeDefeat.getToggleState(), false);
			if (!patternAutoSaveEnabled)
			{
				_pPatternPtr->setPatternDataUpdateStatus(true);
				_pPatternPtr->updatePatternEditIndication();
				_pPatternPtr->enableControlButton(IgniteControlOptions::SAVE, true);
				_pPatternPtr->enableControlButton(IgniteControlOptions::UNDO, true);
			}
		}
		else if (buttonThatWasClicked == &gbarSync)
		{
			keyFocusedItem = barViewItemID::GBAR_SYNC;
			if (gbarSync.getToggleState())
			{
				gbarSync.setToggleState(false, dontSendNotification);
			}
			else
			{
				gbarSync.setToggleState(true, dontSendNotification);
			}

			_pPatternPtr->setBarData(mt::Ignite::barDataIndex::GBAR_SYNC, -1, gbarSync.getToggleState(), false);
			if (!patternAutoSaveEnabled)
			{
				_pPatternPtr->setPatternDataUpdateStatus(true);
				_pPatternPtr->updatePatternEditIndication();
				_pPatternPtr->enableControlButton(IgniteControlOptions::SAVE, true);
				_pPatternPtr->enableControlButton(IgniteControlOptions::UNDO, true);
			}
		}
		else if (buttonThatWasClicked == &copy)
		{
			_pPatternPtr->onOptCtlButtonClicked(IgniteControlOptions::eKey::COPY);
			if (copy.getToggleState())
				copy.setToggleState(false, dontSendNotification);
		}
		else if (buttonThatWasClicked == &loop)
		{
			_pPatternPtr->onOptCtlButtonClicked(IgniteControlOptions::eKey::LOOP);
			if (loop.getToggleState())
				loop.setToggleState(false, dontSendNotification);
		}
		else if (buttonThatWasClicked == &optionUndo)
		{
			_pPatternPtr->onOptCtlButtonClicked(IgniteControlOptions::eKey::UNDO);
		}
		else if (buttonThatWasClicked == &barInsert)
		{
			keyFocusedItem = barViewItemID::INSERT_BAR;
			_pPatternPtr->showPatternBars();
			_pMainmodePtr = AppMainmode::getMainModeInstance();
			_pMainmodePtr->barInsertComp->setBarIndex((int)_pPatternPtr->getPatternData(mt::Ignite::ptDataIndex::BARNUM) + 1);
			_pMainmodePtr->barInsertComp->setVisible(true);
			_pMainmodePtr->barInsertComp->runModalLoop();
		}
		else if (buttonThatWasClicked == &gangBtn)
		{
			if (gangBtn.getToggleState())
			{
				isGangModeEnabled = false;
				gangBtn.setToggleState(false, dontSendNotification);
				stepControl.setPresetButtonVisibility(true);
				_pPatternPtr->setGangMode(false);
				stepControl.enableDisableNoteBox(true);
				repaint();
			}
			else
			{
				isGangModeEnabled = true;
				stepControl.setPresetButtonVisibility(false);
				_pPatternPtr->setGangMode(true);
				gangBtn.setToggleState(true, dontSendNotification);
				stepControl.enableDisableNoteBox(false);
			}
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_GANG, (isGangModeEnabled ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			if (isGangModeEnabled)
				_pInfoWindow->setTextandShow("GANG: ON");
			else
				_pInfoWindow->setTextandShow("GANG: OFF");
			_pPatternPtr->resetGangStepState();
			repaint();
		}
		else if (buttonThatWasClicked == &imageButtonPencil)
		{
			imageButtonPencil.setToggleState(!imageButtonPencil.getToggleState(), dontSendNotification);
		}
		else if (buttonThatWasClicked == &sculptBtn)
		{
			keyFocusedItem = barViewItemID::SCULPT;
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsSculptModeCnfg, (paramId_t)SettingsSculptModeCnfgParam::sculptButtonOnState, sculptBtn.getToggleState());
			AppGrid::getGridInstance()->setSculptButtonState(sculptBtn.getToggleState());
			_pPatternPtr->requestDataFromEngine(_pPatternPtr->getSelected());
			sculptKeyStatus = sculptBtn.getToggleState();
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SCULPT, (sculptKeyStatus ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			if (sculptKeyStatus)
				_pInfoWindow->setTextandShow("SCULPT: ON");
			else
				_pInfoWindow->setTextandShow("SCULPT: OFF");
			repaint();
		}
		else if (buttonThatWasClicked == &patternRotateBtn)
		{
			stepControl.setSelectedFirstStep(0);
			stepControl.showFirstStepListBox();
		}
		else if (buttonThatWasClicked == &rightNudgeButton)
		{
			keyFocusedItem = barViewItemID::BAR_RIGHT;
			stepControl.rightNudgeChanged.emit(rightNudgeButton.getToggleState());
		}
		else if (buttonThatWasClicked == &leftNudgeButton)
		{
			keyFocusedItem = barViewItemID::BAR_LEFT;
			stepControl.leftNudgeChanged.emit(leftNudgeButton.getToggleState());
		}
	}

	void StepSequencer::getAssets()
	{
		insertDefaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-link105x36-insertbar-default.png"));
		insertHoverImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-link105x36-insertbar-default-hover.png"));
		rotateDefaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-link136x36-rotate-default.png"));
		rotateHoverImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-link136x36-rotate-default-hover.png"));
		scrollDefaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("scroll-option-patternbar-default.png"));
		scrollHoverImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("scroll-option-patternbar-default-hover.png"));
		scrollLeftDefaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("scroll-caret-patternbar-left-default.png"));
		scrollLeftHoverImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("scroll-caret-patternbar-left-default-hover.png"));
		scrollRightDefaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("scroll-caret-patternbar-right-default.png"));
		scrollRightHoverImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("scroll-caret-patternbar-right-default-hover.png"));
		bgDefaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("bg-modal-container-130x36-default.png"));
		bgHoverImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("bg-modal-container-130x36-default-hover.png"));
	}

	void StepSequencer::onOptCtlButtonClicked(IgniteControlOptions::eKey option)
	{
		if (option == IgniteControlOptions::PLAY)
		{
			sculptBtn.setToggleState(false,dontSendNotification);
			sculptBtn.setEnabled(sculptBtn.isEnabled() ? false : true);
		}
		else if (option == IgniteControlOptions::STOP)
			sculptBtn.setEnabled(false);

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsSculptModeCnfg, (paramId_t)SettingsSculptModeCnfgParam::sculptButtonOnState, sculptBtn.isEnabled());

		if (!sculptBtn.isEnabled())
		{
			if (sculptKeyStatus)
			{
				sculptKeyStatus = false;
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SCULPT, (fpLEDPattern::EN_LED_PATT_OFF));
				_pInfoWindow->setTextandShow("SCULPT: OFF");
			}
		}

		repaint();
	}

	void StepSequencer::refreshBarData()
	{
		_pPatternComp = IgnitePattern::getAppPatternInstance();
		currentPattern = (uint8_t)_pPatternComp->getSelected();
		patternData = _pPatternComp->getPatternData(currentPattern);

		barNum = patternData->enginePatternData.numBars;
		stepThreshold = (barNum * 16) - 1;
		barId = patternData->barIndex + 1;
		activeStepIndex = patternData->barIndex * 16;
		if (this->isVisible())
		{
			stepsState = _pPatternComp->getGangStepsState();
			currentControl = _pPatternComp->getActiveParamControl();
			noteColour = Theme::Instance().getNoteColourForParam((int)s_mapStringToParamValueType[currentControl], 100);

			white = Theme::Instance().getColour(CommonColorID::neutralWhite);
			orange = Theme::Instance().getColour(customOrange9);
			grey3 = Theme::Instance().getColour(customGrey3);
			grey2 = Theme::Instance().getColour(customGrey2);
			grey4 = Theme::Instance().getColour(customWhite);

			markingWidth = levelRect.getHeight() / 4;
			markingY = markingWidth + levelRect.getY();
			unitHeightCache = (currentControl != "Len:Steps") ? 1.0f / s_mapStringToParamMaxType[currentControl] : 1.0f / stepThreshold;
			noteColourIntensity = 100;
		}
	}

	void StepSequencer::paint(juce::Graphics& g)
	{
		if (buttonWidth == 0)
			return;

		g.drawLine(0, markingY, static_cast<float>(getWidth()), markingY);
		markingY += markingWidth;

		if (sculptBtn.getToggleState())
			_pPatternComp->requestDataFromEngine(currentPattern);

		for (int i = 0; i < 16; i++)
		{
			int index = i + activeStepIndex;
			auto stepGateStatus = getGateStatus(index);

			if (i >= rangeHigh || i < rangeLow)
				continue;

			if (stepGateStatus)
				noteColourIntensity = 100;
			else
				noteColourIntensity = 30;

			auto nodeTextColour = Theme::Instance().getNoteColourForParam(1, noteColourIntensity);

			selectedIndex[i] = true;
			int value;

			if (stepGateStatus)
			{
				if (isSliderDragging)
				{
					if (isGangModeEnabled)
					{
						if (stepsState[index])
						{
							value = prevValue[i];
							if (value < 0)
								value = 0;
							if (currentControl != "Len:Steps")
								stepThreshold = s_mapStringToParamMaxType[currentControl];

							if (value > stepThreshold)
								value = stepThreshold;
						}
						else
							value = getParamFromEngine(currentControl, index);
					}
					else
					{
						value = prevValue[i];
						if (value < 0)
							value = 0;

						if (currentControl != "Len:Steps")
							stepThreshold = s_mapStringToParamMaxType[currentControl];

						if (value > stepThreshold)
							value = stepThreshold;
					}
				}
				else
					value = getParamFromEngine(currentControl, index);
			}
			else
				value = getParamFromEngine(currentControl, index);

			//DBG("value= " + String(value));
			if ((value > 0) && _pPatternComp->isNotePresent(i, -1))
			{
				auto lbr = getLevelButtonRect(i + 1);
				float unit_height;

				if (currentControl != "Len:Steps")
					unit_height = lbr.getHeight() * (1.0f / s_mapStringToParamMaxType[currentControl]);
				else
					unit_height = lbr.getHeight() * (1.0f / stepThreshold);

				float posFoVel = (float)value * unit_height; //127 is velocity of selected note
				posFoVel = lbr.getHeight() - posFoVel;
				lbr.setY(lbr.getY() + posFoVel);
				lbr.setHeight(lbr.getHeight() - posFoVel);
				g.setColour(Theme::Instance().getNoteColourForParam((int)s_mapStringToParamValueType[currentControl], noteColourIntensity));

				g.fillRoundedRectangle(lbr, 3.0);

				g.setColour(nodeTextColour);
				if (currentControl == "Offset")
					g.drawText(juce::String((int)value- ((stepThreshold+1)/2)), lbr, Justification::centred);

				else if (currentControl == "Len:Tick")
				{
					auto val = getParamFromEngine("Len:Steps", i);
					g.drawText(String((int)val) + ":" + juce::String((int)value), lbr, Justification::centred);
				}
				else if (currentControl == "Len:Steps")
				{
					auto val = getParamFromEngine("Len:Tick", i);
					g.drawText(juce::String((int)value) + ":" + String((int)val), lbr, Justification::centred);
				}
				else
					g.drawText(juce::String((int)value), lbr, Justification::centred);
			}
			if (stepsState[index])
			{
				g.setColour(white);
				g.drawRect(getLevelButtonRect(i + 1), 1.0f);
			}

			if (getGateStatus(i)) {
				//Button Color
				g.setColour(orange);
			}
			else
			{
				g.setColour(grey3);
			}
			//draw buttons
			auto btnRect = getStepButtonRect(i + 1);
			g.fillRoundedRectangle(btnRect, 3.0);
			if (stepFocusedId == i)
			{
				g.setColour(noteColour);
				g.drawRect(btnRect, 1.0f);
			}
			auto center = btnRect.getCentre();

			if (_pPlayHead == nullptr)
			{
				_pPlayHead = StepPlayhead::getPlayHeadInstance();
			}
			_pPlayHead->setViewColour(noteColour);
			_pPlayHead->SetViewBounds(ui::Rectangle<float>{ 10, levelRect.getY(), buttonWidth + STEP_BUTTON_SPACE, levelRect.getHeight() });

			if (getGateStatus(i))
				g.setColour(juce::Colours::white);
			else
				g.setColour(grey4);

			g.drawText("ST " + String(barId) + String(":") + String(i + 1), btnRect, Justification::centred);
		}

		nudgeText.setButtonText("BAR " + String(barId) + "/" + String(barNum));

		Image DrawImage;

		DrawImage = ((barInsert.isMouseOver() || barInsert.isFocused) ? insertHoverImage : insertDefaultImage);
		g.drawImage(DrawImage, ui::Rectangle<float>{10, 428, 105, 36 }, RectanglePlacement::centred);

		DrawImage = ((patternRotateBtn.isMouseOver() || patternRotateBtn.isFocused) ? rotateHoverImage : rotateDefaultImage);
		g.drawImage(DrawImage, ui::Rectangle<float>{ 594, 428, 136, 36 }, RectanglePlacement::centred);

		DrawImage = nudgeText.isMouseOver() ? scrollHoverImage : scrollDefaultImage;
		g.drawImage(DrawImage, ui::Rectangle<float>{ 734, 428, 147, 36 }, RectanglePlacement::centred);

		DrawImage = (leftNudgeButton.isMouseOver() || leftNudgeButton.isFocused) ? scrollLeftHoverImage : scrollLeftDefaultImage;
		g.drawImage(DrawImage, ui::Rectangle<float>{ 737, 430, 24, 32 }, RectanglePlacement::centred);

		DrawImage = (rightNudgeButton.isMouseOver() || rightNudgeButton.isFocused) ? scrollRightHoverImage : scrollRightDefaultImage;
		g.drawImage(DrawImage, ui::Rectangle<float>{ 856, 430, 24, 32 }, RectanglePlacement::centred);

		DrawImage = ((barLoopCount.isMouseOver() || barLoopCount.getFocusState()) ? bgHoverImage : bgDefaultImage);
		g.drawImage(DrawImage, ui::Rectangle<float>{885, 428, 130, 36 }, RectanglePlacement::centred);
	}

	void StepSequencer::resized()
	{
		stepControl.setBounds(10, 384, 1005, 36);
		stepPreset.setBounds(976, 64, 39, 312);
		stepRect = ui::Rectangle<float>(10, 350, 966, 24);
		levelRect = ui::Rectangle<float>(10, 64, 966, 284);
		barOptionFlex1.performLayout(Rectangle(734, 428, 147, 36));
		barInsert.setBounds(10, 428, 105, 36);
		gbarSync.setBounds(119, 428, 176, 36);
		xposeDefeat.setBounds(299, 428, 176, 36);
		sculptBtn.setBounds(479, 428, 105, 36);
		patternRotateBtn.setBounds(594, 428, 136, 36);
		barLoopCountLable.setBounds(889, 435, 100, 22);
		barOptionFlex3.performLayout(Rectangle(885, 428, 130, 38));
		barOptionFlex2.performLayout(Rectangle(862, 10, 158, 44));
		gangBtn.setBounds(686, 10, 120, 44);
		imageButtonPencil.setBounds(814, 10, 44, 44);
		leftNudgeButton.setBounds(737, 430, 24, 32);
		rightNudgeButton.setBounds(856, 430, 24, 32);
		leftNudgeButton.toFront(true);
		rightNudgeButton.toFront(true);
		calculateButtonWidth();
	}

	ui::Rectangle<float> StepSequencer::getLevelButtonRect(int index)
	{
		float xPos;

		if (index == 1)
		{
			xPos = 10;
		}
		else
		{
			xPos = levelRect.getX() + ((index - 1) * STEP_BUTTON_SPACE) + ((index - 1) * buttonWidth);
		}

		return ui::Rectangle<float>(xPos, levelRect.getY(), buttonWidth, levelRect.getHeight());
	}

	ui::Rectangle<float> StepSequencer::getStepButtonRect(int index)
	{
		float xPos;

		if (index == 1)
		{
			xPos = 10;
		}
		else
		{
			xPos = stepRect.getX() + ((index - 1) * STEP_BUTTON_SPACE) + ((index - 1) * buttonWidth);
		}

		return ui::Rectangle<float>(xPos, stepRect.getY() + 2, buttonWidth, stepButtonHeight);
	}

	void StepSequencer::mouseDrag(const MouseEvent& event)
	{
		isMouseDragEnd = false;
		enableDisableButton(event.position);
	}

	void StepSequencer::mouseUp(const MouseEvent& event)
	{
		isMouseDragEnd = true;
		lastIndex = -1;
	}

	void StepSequencer::mouseDown(const MouseEvent& event)
	{
		isMouseDragEnd = true;
		enableDisableButton(event.position);
	}

	void StepSequencer::updateBarData()
	{
		refreshBarData();
		repaint();
	}

	void StepSequencer::calculateButtonWidth()
	{
		float effectiveWidth = stepRect.getWidth() - (STEP_BUTTON_SPACE * (rangeHigh - 1)) - 10;
		buttonWidth = effectiveWidth / rangeHigh;
		repaint();
	}

	void StepSequencer::refreshBarUI()
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();

		if (_pPatternPtr->getBarData(mt::Ignite::barDataIndex::TRANSPOSE_DEFEAT_STATUS, -1))
		{
			xposeDefeat.setToggleState(true, dontSendNotification);
		}
		else
		{
			xposeDefeat.setToggleState(false, dontSendNotification);
		}

		barLoopCount.setText(String((int)_pPatternPtr->getBarData(mt::Ignite::barDataIndex::BAR_REPEAT, -1)), dontSendNotification);

		if (_pPatternPtr->getBarData(mt::Ignite::barDataIndex::GBAR_SYNC, -1))
		{
			gbarSync.setToggleState(true, dontSendNotification);
		}
		else
		{
			gbarSync.setToggleState(false, dontSendNotification);
		}
		updateLegnthDelay();
		updateLastStep(true);
		refreshBarData();
	}

	void StepSequencer::enableDisableButton(juce::Point<float> position, int stepIndex)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		int index = 0;
		if (stepIndex != -1)
			index = stepIndex;
		else
			index = static_cast<int>((position.getX() - 10) / (buttonWidth + 4));

		currentControl = _pPatternPtr->getActiveParamControl();
		stepThreshold = (((int)_pPatternPtr->getPatternData(mt::Ignite::ptDataIndex::BARNUM) + 1) * 16) - 1;

		if (currentControl != "Len:Steps")
			stepThreshold = s_mapStringToParamMaxType[currentControl];

		if (selectedStep > -1)
		{
			_pPatternPtr->resetGangStepState();
			selectedStep = -1;
		}
		if (index >= 16 || lastIndex == index)
			return;

		if (isGangModeEnabled)
			_pPatternPtr->getGangStepState(index) ? (_pPatternPtr->setGangStep(index, false)) : (_pPatternPtr->setGangStep(index, true));

		else
		{
			if (stepRect.contains(position) || stepIndex != -1)
			{
				selectedIndex[index] = !selectedIndex[index];
				lastIndex = index;

				if (imageButtonPencil.getToggleState())
				{
					if (_pPatternPtr->isNotePresent(index, -1))
						_pPatternPtr->delNote(-1, index, -1);
					else
						_pPatternPtr->addStepNoteIfNotPresent(index);
				}
				else
				{
					if (!getGateStatus(index))
						_pPatternPtr->setStepData(mt::Ignite::stepDataIndex::GATE_STATUS, -1, index, true, false);
					else
						_pPatternPtr->setStepData(mt::Ignite::stepDataIndex::GATE_STATUS, -1, index, (uint8_t)false, false);
				}
				updateIndication();
			}
			else if (levelRect.contains(position))
			{
				float y = position.getY();
				int velocity = static_cast<int>(std::round((stepThreshold * (y - levelRect.getY())) / levelRect.getHeight()));
				velocity = stepThreshold - velocity;

				if (velocity < 0 || velocity > stepThreshold)
					return;
				if (getGateStatus(index))
					setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (uint8_t)velocity);
			}
		}
		repaint();
	}

	void StepSequencer::setParamToEngine(juce::String param, int index, int16_t data)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		_pPatternPtr->setDataForSelectedNode(s_mapStringToParamValueType[param], index, data, false, !isMouseDragEnd);
		updateIndication();
	}

	void StepSequencer::updateIndication()
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();

		if (!IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled())
		{
			_pPatternPtr->setPatternDataUpdateStatus(true);
			_pPatternPtr->updatePatternEditIndication();
			_pPatternPtr->enableControlButton(IgniteControlOptions::SAVE, true);
			_pPatternPtr->enableControlButton(IgniteControlOptions::UNDO, true);
		}
	}

	bool StepSequencer::getGateStatus(int stepIndex)
	{
		int index = 0;
		if (stepIndex >= 16)
			index = stepIndex - (patternData->barIndex * 16);
		else
			index = stepIndex;

		return patternData->enginePatternData.BarDataList.at((const unsigned char)patternData->barIndex).StepDataList.at((const unsigned char)index).enable;
	}

	var StepSequencer::getParamFromEngine(juce::String param, int index)
	{
		auto tmp1 = patternData->stepIndex;
		var value = -1;

		if (index >= 16)
			patternData->stepIndex = index - (patternData->barIndex * 16);
		else
			patternData->stepIndex = index;


		if (patternData->enginePatternData.BarDataList.at((const unsigned char)patternData->barIndex).StepDataList.at((const unsigned char)patternData->stepIndex).nodeList.count((const unsigned char)patternData->nodeIndex))
		{
			switch (s_mapStringToParamValueType[param])
			{
			case mt::Ignite::nodeDataIndex::DELAY:
				value = patternData->enginePatternData.BarDataList.at((const unsigned char)patternData->barIndex).StepDataList.at((const unsigned char)patternData->stepIndex).nodeList.at((const unsigned char)patternData->nodeIndex).begin()->second.offset + ((maxDelayVar + 1) / 2);
				DBG("get: " + String((int)patternData->enginePatternData.BarDataList.at((const unsigned char)patternData->barIndex).StepDataList.at((const unsigned char)patternData->stepIndex).nodeList.at((const unsigned char)patternData->nodeIndex).begin()->second.offset));
				break;
			case mt::Ignite::nodeDataIndex::NOTE_NUM:
				value = patternData->enginePatternData.BarDataList.at((const unsigned char)patternData->barIndex).StepDataList.at((const unsigned char)patternData->stepIndex).nodeList.at((const unsigned char)patternData->nodeIndex).begin()->second.note;
				break;
			case mt::Ignite::nodeDataIndex::STEP_LEN_IN_STEP:
				value = patternData->enginePatternData.BarDataList.at((const unsigned char)patternData->barIndex).StepDataList.at((const unsigned char)patternData->stepIndex).nodeList.at((const unsigned char)patternData->nodeIndex).begin()->second.lenInSteps;
				break;
			case mt::Ignite::nodeDataIndex::STEP_LEN_IN_TICK:
				value = patternData->enginePatternData.BarDataList.at((const unsigned char)patternData->barIndex).StepDataList.at((const unsigned char)patternData->stepIndex).nodeList.at((const unsigned char)patternData->nodeIndex).begin()->second.lenInTicks;
				break;
			case mt::Ignite::nodeDataIndex::VELO:
				value = patternData->enginePatternData.BarDataList.at((const unsigned char)patternData->barIndex).StepDataList.at((const unsigned char)patternData->stepIndex).nodeList.at((const unsigned char)patternData->nodeIndex).begin()->second.velocity;
				break;
			}
			patternData->stepIndex = tmp1;
		}
		return value;
	}

	StepSequencer::~StepSequencer()
	{
		cleanup();
	}

	void StepSequencer::cleanup()
	{

	}

	void StepSequencer::setBarStepValues(paramId_t param, uint32_t keyValue, int index)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		int Threshold = s_mapStringToParamMaxType[_pPatternPtr->getActiveParamControl()];
		int value = getParamFromEngine(_pPatternPtr->getActiveParamControl(), index);

		if ((getGateStatus(index)))
		{
			if (keyValue == ROTARY_RIGHT && (value < Threshold))
				setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), index) + 1);
			else if (keyValue == ROTARY_LEFT && (value > 0))
				setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), index) - 1);
			else if (keyValue == ROTARY_FASTRIGHT && (value < Threshold))
				(value + ROTARY_FASTJUMP) < Threshold ? setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), index) + ROTARY_FASTJUMP): setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)Threshold);
			else if (keyValue == ROTARY_FASTLEFT && (value > 0))
				(value - ROTARY_FASTJUMP) > 0 ? setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), index) - ROTARY_FASTJUMP): setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)0);
			else if (keyValue == ROTARY_SUPERFASTRIGHT && (value < Threshold))
				(value + ROTARY_SUPERFASTJUMP) < Threshold ? setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), index) + ROTARY_SUPERFASTJUMP) : setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)Threshold);
			else if (keyValue == ROTARY_SUPERFASTLEFT && (value > 0))
				(value - ROTARY_SUPERFASTJUMP) > 0 ? setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), index) - ROTARY_SUPERFASTJUMP) : setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)0);
		}
	}

	void StepSequencer::setStepValues(paramId_t param, int index)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		if ((param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT) || (param == (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT))
		{
			if ((getGateStatus(index)))
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), index) + 1);
				else if (param == (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT)
					setParamToEngine(_pPatternPtr->getActiveParamControl(), index, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), index) - 1);
			}
		}
		else if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
		{
			getGateStatus(index) ? _pPatternPtr->setStepData(mt::Ignite::stepDataIndex::GATE_STATUS, -1, index, false, false) :
				_pPatternPtr->setStepData(mt::Ignite::stepDataIndex::GATE_STATUS, -1, index, true, false);
		}
	}

	void StepSequencer::setStepValues(paramId_t param, uint8_t value)
	{
		bool* stepsGangState = IgnitePattern::getAppPatternInstance()->getGangStepsState();
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();

		for (auto i = 0; i < 16; i++)
		{
			if ((param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT) || (param == (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT))
			{
				if (stepsGangState[i] && (getGateStatus(i)))
				{
					if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
						setParamToEngine(_pPatternPtr->getActiveParamControl(), i, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), i) + value);
					else if (param == (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT)
						setParamToEngine(_pPatternPtr->getActiveParamControl(), i, (int16_t)(int)getParamFromEngine(_pPatternPtr->getActiveParamControl(), i) - value);
				}
			}
			else if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
			{
				if (stepsGangState[i])
					getGateStatus(i) ? _pPatternPtr->setStepData(mt::Ignite::stepDataIndex::GATE_STATUS, -1, i, false, false) :
					_pPatternPtr->setStepData(mt::Ignite::stepDataIndex::GATE_STATUS, -1, i, true, false);
			}
		}
	}

	int StepSequencer::isEditing()
	{
		int index = 0;
		stepControl.getPadListBoxEditStatus() ? index = 1 : 0;
		stepControl.getParamValueListBoxEditStatus() ? index = 2 : 0;
		stepControl.getTimeBaseValueListBoxEditStatus() ? index = 3 : 0;
		stepControl.getDirectionListBoxEditStatus() ? index = 4 : 0;
		stepControl.getLastStepEditStatus() ? index = 5 : 0;
		barLoopCount.IsEditing() ? index = 14 : 0;
		stepControl.getFirstStepEditStatus() ? index = 11 : 0;
		return index;
	}


	void StepSequencer::setPencilIconState(bool state)
	{
		if (state == true)
		{
			if (!imageButtonPencil.getToggleState())
				imageButtonPencil.setToggleState(state, dontSendNotification);
		}
		else
		{
			if (imageButtonPencil.getToggleState())
				imageButtonPencil.setToggleState(state, dontSendNotification);
		}
	}

	void StepSequencer::setGangButtonState()
	{
		if (gangBtn.getToggleState())
			gangBtn.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, 0);
	}

	void StepSequencer::updateLegnthDelay() 
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();

		s_mapStringToParamMaxType.at("Offset") = ((_pPatternPtr->getDelayRange().max + 1) * 2) - 1;
		s_mapStringToParamMaxType.at("Len:Tick") = (_pPatternPtr->getTickRange().max);
		maxDelayVar = s_mapStringToParamMaxType["Offset"];
		maxTickLengthVar = s_mapStringToParamMaxType["Len:Tick"];
		repaint();
	}

	void StepSequencer::setFpStepGangLedState()
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (_pPatternPtr->isInitCompleted() && _pPatternPtr->isPatternCreated())
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();

			for (int step = 0; step < 16; step++)
			{
				if (_pPatternPtr->getGangStepState(step))
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_RGB_LED_LBLUE);
				else
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_LED_PATT_OFF);
			}
		}
	}

	void StepSequencer::setFpStepNodeLedState()
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (_pPatternPtr->isInitCompleted() && _pPatternPtr->isPatternCreated())
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();

			for (int step = 0; step < 16; step++)
			{
				if (_pPatternPtr->isNotePresent(step, -1))
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_RGB_LED_WHITE);
				else
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_LED_PATT_OFF);
			}
		}
	}

	bool StepSequencer::handleGang(paramId_t param, uint32_t KeyValue)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusReleased)
			return true;
		else if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusPressed)
		{
			_pPatternPtr->setGangKeyStatus(!_pPatternPtr->getGangKeyStatus());
			gangBtn.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, 0);

			if (_pPatternPtr->getGangKeyStatus() == true)
			{
				if (_pPatternPtr->getTieKeyStatus() == true)
				{
					_pPatternPtr->setTieKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_NEXT_TIE, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getTransposeKeyStatus() == true)
				{
					_pPatternPtr->setTransposeKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_FILL_X, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getSkipKeyStatus() == true)
				{
					_pPatternPtr->setSkipKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getInsertKeyStatus() == true)
				{
					setPencilIconState(false);
					_pPatternPtr->setInsertKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_INSERT, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getLastKeyStatus() == true)
				{
					_pPatternPtr->setLastKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_LAST, fpLEDPattern::EN_LED_PATT_OFF);
				}

				setFpStepGangLedState();
				_pPatternPtr->setFpStepGateLedState();
			}
			else
			{
				_pPatternPtr->setFpStepLedState();
				AppMainmode::getMainModeInstance()->resetTrackSelLeds();
			}
		}
		return true;
	}

	bool StepSequencer::handleInsert(paramId_t param, uint32_t KeyValue)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusReleased)
			return true;
		else if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusPressed)
		{
			_pPatternPtr->setInsertKeyStatus(!_pPatternPtr->getInsertKeyStatus());
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_INSERT, (_pPatternPtr->getInsertKeyStatus() ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			this->postCommandMessage(12);

			if (_pPatternPtr->getInsertKeyStatus() == true)
			{
				setPencilIconState(true);

				if (_pPatternPtr->getTieKeyStatus() == true)
				{
					_pPatternPtr->setTieKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_NEXT_TIE, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getTransposeKeyStatus() == true)
				{
					_pPatternPtr->setTransposeKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_FILL_X, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getSkipKeyStatus() == true)
				{
					_pPatternPtr->setSkipKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getGangKeyStatus() == true)
				{
					setGangButtonState();
					_pPatternPtr->setGangKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_GANG, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getLastKeyStatus() == true)
				{
					_pPatternPtr->setLastKeyStatus(false);
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_LAST, fpLEDPattern::EN_LED_PATT_OFF);
				}

				setFpStepNodeLedState();
				_pPatternPtr->setFpStepGateLedState();
			}
			else
			{
				setPencilIconState(false);
				_pPatternPtr->setFpStepLedState();
				AppMainmode::getMainModeInstance()->resetTrackSelLeds();
			}
		}
		return true;
	}

	bool StepSequencer::handleTrackSelection(paramId_t param, uint32_t KeyValue)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		int stepIndex = 0;
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusPressed)
		{
			stepIndex = param - (paramId_t)FrontPannelParam::paramKey_TRACKSEL1;
			if (_pPatternPtr->isInitCompleted() && _pPatternPtr->isPatternCreated())
			{
				if (_pPatternPtr->getInsertKeyStatus() == true)
				{
					enableDisableButton(juce::Point<float>{0, 0}, stepIndex);
					lastIndex = -1;
					auto isNoteAdded = _pPatternPtr->isNotePresent(stepIndex, -1);
					if (isNoteAdded)
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pPatternPtr->fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_RGB_LED_WHITE);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pPatternPtr->fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (_pPatternPtr->getGangMode())
				{
					enableDisableButton(juce::Point<float>{0, 0}, stepIndex);
					lastIndex = -1;
					auto isStepGanged = _pPatternPtr->getGangStepState(stepIndex);
					if (isStepGanged)
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pPatternPtr->fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_RGB_LED_LBLUE);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pPatternPtr->fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_LED_PATT_OFF);
				}
				else
				{
					enableDisableButton(juce::Point<float>{0, 0}, stepIndex);
					lastIndex = -1;

					if(getGateStatus(stepIndex))
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pPatternPtr->fpRGBDispLed[(FrontPannelParam)param], _pPatternPtr->fpPatternParamLedValues[_pPatternPtr->getActiveParamControl()]);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pPatternPtr->fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_LED_PATT_OFF);
				}
			}
		}
		return true;
	}

	bool StepSequencer::handleRotary(paramId_t param, uint32_t keyValue)
	{
		int keyIndex = 0;
		barIndex = IgnitePattern::getAppPatternInstance()->getCurrentBarId();
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		keyIndex = (barIndex * 16) + (param - (paramId_t)FrontPannelParam::paramRot_PADROT1);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (!_pPatternPtr->getGangKeyStatus())
			setBarStepValues(param, keyValue, keyIndex);
		else
		{
			if (_pPatternPtr->getGangStepState(keyIndex))
			{
				if ((keyValue == ROTARY_RIGHT || keyValue == ROTARY_FASTRIGHT || keyValue == ROTARY_SUPERFASTRIGHT))
					stepPreset.setStepValues((paramId_t)FrontPannelParam::paramKey_RMOVELEFT);
				else if ((keyValue == ROTARY_LEFT || keyValue == ROTARY_FASTLEFT || keyValue == ROTARY_SUPERFASTLEFT))
					stepPreset.setStepValues((paramId_t)FrontPannelParam::paramKey_RMOVERIGHT);
			}
			else
			{
				setBarStepValues(param, keyValue, keyIndex);
			}
		}

		repaint();
		return true;
	}

	bool StepSequencer::handleBar(paramId_t param, uint32_t keyValue)
	{
		bool isKeyHandled = false;
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		switch ((FrontPannelParam)param)
		{
		case FrontPannelParam::paramRot_ROW:
		case FrontPannelParam::paramKey_ROTARYROW:
			isKeyHandled = stepControl.pannelKeyHandler(param, keyValue);
			break;
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
			isKeyHandled = handleTrackSelection(param, keyValue);
			break;
		case FrontPannelParam::paramKey_INSERT:
			isKeyHandled = handleInsert(param, keyValue);
			break;
		case FrontPannelParam::paramKey_GANG:
			isKeyHandled = handleGang(param, keyValue);
			break;
		case FrontPannelParam::paramRot_PADROT1:
		case FrontPannelParam::paramRot_PADROT2:
		case FrontPannelParam::paramRot_PADROT3:
		case FrontPannelParam::paramRot_PADROT4:
		case FrontPannelParam::paramRot_PADROT5:
		case FrontPannelParam::paramRot_PADROT6:
		case FrontPannelParam::paramRot_PADROT7:
		case FrontPannelParam::paramRot_PADROT8:
		case FrontPannelParam::paramRot_PADROT9:
		case FrontPannelParam::paramRot_PADROT10:
		case FrontPannelParam::paramRot_PADROT11:
		case FrontPannelParam::paramRot_PADROT12:
		case FrontPannelParam::paramRot_PADROT13:
		case FrontPannelParam::paramRot_PADROT14:
		case FrontPannelParam::paramRot_PADROT15:
		case FrontPannelParam::paramRot_PADROT16:
			isKeyHandled = handleRotary(param, keyValue);
			break;
		}

		return isKeyHandled;
	}

	bool StepSequencer::pannelKeyHandler(paramId_t param, uint32_t keyValue)
	{
		bool setfocus = true;
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		barIndex = IgnitePattern::getAppPatternInstance()->getCurrentBarId();
		frontPanelPtr = FrountPannel::GetFrountPannelInstance();

		if (AppMainmode::getMainModeInstance()->patternBarCopyComp->isVisible())
			return AppMainmode::getMainModeInstance()->patternBarCopyComp->pannelKeyHandler(param, keyValue);

		if (param == (paramId_t)FrontPannelParam::paramKey_COPY_NAV)
		{
			this->postCommandMessage(1);
			return true;
		}

		if (AppMainmode::getMainModeInstance()->barInsertComp->isVisible())
			return AppMainmode::getMainModeInstance()->barInsertComp->pannelKeyHandler(param, keyValue);

		auto opMode = optPtr->getOperationMode();
		if ((paramId_t)FrontPannelParam::paramKey_SCULPT == param && (opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
		{
			if ((FrontPannelKeyStatus)keyValue == FrontPannelKeyStatus::paramKeyStatusPressed)
			{
				sculptKeyStatus = !sculptKeyStatus;
				sculptBtn.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, 0);
				return true;
			}
		}

		switch ((FrontPannelParam)param)
		{
		case FrontPannelParam::paramRot_ROW:
		case FrontPannelParam::paramKey_ROTARYROW:
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
		case FrontPannelParam::paramKey_INSERT:
		case FrontPannelParam::paramKey_GANG:
		case FrontPannelParam::paramRot_PADROT1:
		case FrontPannelParam::paramRot_PADROT2:
		case FrontPannelParam::paramRot_PADROT3:
		case FrontPannelParam::paramRot_PADROT4:
		case FrontPannelParam::paramRot_PADROT5:
		case FrontPannelParam::paramRot_PADROT6:
		case FrontPannelParam::paramRot_PADROT7:
		case FrontPannelParam::paramRot_PADROT8:
		case FrontPannelParam::paramRot_PADROT9:
		case FrontPannelParam::paramRot_PADROT10:
		case FrontPannelParam::paramRot_PADROT11:
		case FrontPannelParam::paramRot_PADROT12:
		case FrontPannelParam::paramRot_PADROT13:
		case FrontPannelParam::paramRot_PADROT14:
		case FrontPannelParam::paramRot_PADROT15:
		case FrontPannelParam::paramRot_PADROT16:
			return handleBar(param, keyValue);
		}

		if ((optionHandlerEnabled == BAR_STEPS))
		{
			return barKeyHandler(param, keyValue);
		}

		if (optionHandlerEnabled == BAR_PRESETS && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
		{
			presetKeyHandler(param, keyValue);
		}

		if (optionHandlerEnabled && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
		{
			pannelKeyOptionsHandler(param, keyValue);
		}
		else
		{
			int EditItem = barViewItemID::NO_FOCUS;
			if (keyFocusedItem == barViewItemID::NO_FOCUS)
			{
				/*1st key after Page visible */
				keyFocusedItem = barViewItemID::GRID_NODE;
			}
			else
			{
				EditItem = isEditing();
				if (EditItem)
				{
					if (keyFocusedItem != EditItem)
					{
						pannelSendKey(false, param, keyValue);
						keyFocusedItem = EditItem;
					}
					setfocus = true;
				}
				else
				{
					pannelSendKey(false, param, keyValue); // clear last Focus
					switch ((FrontPannelParam)param)
					{
					case FrontPannelParam::paramRot_BAR:
						if(keyValue == ROTARY_DIRECTION_LEFT)
							stepPreset.setStepValues((paramId_t)FrontPannelParam::paramKey_RMOVERIGHT);
						else if(keyValue == ROTARY_DIRECTION_RIGHT)
							stepPreset.setStepValues((paramId_t)FrontPannelParam::paramKey_RMOVELEFT);
						break;
					case FrontPannelParam::paramKey_RMOVELEFT:
						keyFocusedItem = (keyFocusedItem + 1 >= barViewItemID::FOCUS_MAX ? (barViewItemID::FOCUS_MAX) : keyFocusedItem + 1);
						break;
					case FrontPannelParam::paramKey_RMOVERIGHT:
						keyFocusedItem = (keyFocusedItem - 1 <= barViewItemID::NO_FOCUS ? (barViewItemID::NO_FOCUS + 1) : keyFocusedItem - 1);
						break;
					case FrontPannelParam::paramKey_PAGESHIFT:
						if (!isPresetsEnabled)
						{
							if (optionHandlerEnabled == BAR_PARAM)
							{
								optionHandlerEnabled = BAR_OPTIONS;
							}
							else if (optionHandlerEnabled == BAR_OPTIONS)
							{
								optionHandlerEnabled = BAR_PARAM;
							}
							else if (optionHandlerEnabled == BAR_STEPS)
							{
								_pPatternPtr->resetGangStepState();
								optionHandlerEnabled = BAR_PARAM;
							}
							pannelKeyOptionsHandler(param, keyValue);
						}
						else
						{
							if (optionHandlerEnabled == BAR_PARAM)
							{
								optionHandlerEnabled = BAR_OPTIONS;
							}
							else if (optionHandlerEnabled == BAR_OPTIONS)
							{
								optionHandlerEnabled = BAR_PRESETS;
								presetKeyHandler(param, keyValue);
							}
							else if (optionHandlerEnabled == BAR_PRESETS)
							{
								optionHandlerEnabled = BAR_PARAM;
								presetKeyHandler(param, keyValue);
							}
							else if (optionHandlerEnabled == BAR_STEPS)
							{
								_pPatternPtr->resetGangStepState();
								optionHandlerEnabled = BAR_PARAM;
							}
							pannelKeyOptionsHandler(param, keyValue);
						}
						break;
					}
				}
				if (optionHandlerEnabled != BAR_PARAM)
					return true;
			}
			if (setfocus)
			{
				pannelSendKey(true, param, keyValue);
			}
		}
		return true;
	}

	bool StepSequencer::pannelKeyOptionsHandler(paramId_t param, uint32_t keyValue)
	{
		bool setfocus = true;

		if (AppMainmode::getMainModeInstance()->patternBarCopyComp->isVisible())
			return AppMainmode::getMainModeInstance()->patternBarCopyComp->pannelKeyHandler(param, keyValue);

		if (IgnitePattern::getAppPatternInstance()->_pBarLoopComp->isVisible())
			return IgnitePattern::getAppPatternInstance()->_pBarLoopComp->PannelKeyHandler(param, keyValue);

		if (barKeyFocusedItem == barOptionsViewItemID::BAR_NO_FOCUS)
		{
			/*1st key after Page visible */
			barKeyFocusedItem = barOptionsViewItemID::BAR_GANG_EDIT;
		}
		else
		{
			pannelSendOptionsKey(false, param, keyValue); // clear last Focus
			if (optionHandlerEnabled != BAR_OPTIONS)
				return true;
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				barKeyFocusedItem = (barKeyFocusedItem + 1 >= barOptionsViewItemID::BAR_FOCUS_MAX ? (barOptionsViewItemID::BAR_FOCUS_MAX) : barKeyFocusedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				barKeyFocusedItem = (barKeyFocusedItem - 1 <= barOptionsViewItemID::BAR_NO_FOCUS ? (barOptionsViewItemID::BAR_NO_FOCUS + 1) : barKeyFocusedItem - 1);
				break;
			}
		}
		if (setfocus)
		{
			pannelSendOptionsKey(true, param, keyValue);
		}
		return true;
	}

	bool StepSequencer::presetKeyHandler(paramId_t param, uint32_t keyValue)
	{
		bool setfocus = true;

		if (barPresetKeyFocusedItem == barPresetViewItemID::BAR_PRESET_NO_FOCUS)
		{
			barPresetKeyFocusedItem = barPresetViewItemID::BAR_PRESET_RAMP;
		}
		else
		{
			pannelSendPresetKey(false, param, keyValue); // clear last Focus
			if (optionHandlerEnabled != BAR_PRESETS)
				return true;
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				barPresetKeyFocusedItem = (barPresetKeyFocusedItem + 1 >= barPresetViewItemID::BAR_PRESET_FOCUS_MAX ? (barPresetViewItemID::BAR_PRESET_FOCUS_MAX) : barPresetKeyFocusedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				barPresetKeyFocusedItem = (barPresetKeyFocusedItem - 1 <= barPresetViewItemID::BAR_PRESET_NO_FOCUS ? (barPresetViewItemID::BAR_PRESET_NO_FOCUS + 1) : barPresetKeyFocusedItem - 1);
				break;
			}
		}
		if (setfocus)
		{
			pannelSendPresetKey(true, param, keyValue);
		}
		return true;
	}

	bool StepSequencer::barKeyHandler(paramId_t param, uint32_t keyValue)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();
		bool handleTrackSel = false;
		int keyIndex = 0;

		switch ((FrontPannelParam)param)
		{
		case FrontPannelParam::paramKey_PAGESHIFT:
			optionHandlerEnabled = BAR_PARAM;
			_pPatternPtr->resetGangStepState();
			pannelSendKey(false, param, keyValue);
			keyFocusedItem = GRID_NODE;
			pannelSendKey(true, param, keyValue);
			break;
		case FrontPannelParam::paramKey_RMOVELEFT:
		case FrontPannelParam::paramKey_RMOVERIGHT:
		case FrontPannelParam::paramKey_RSELECT:
			/*isGangModeEnabled ? setStepValues(param) :*/
			if (handleTrackSel)
				setStepValues(param, keyIndex);
			break;
		}

		repaint();
		return true;
	}

	void StepSequencer::pannelSendKey(bool isFocuse, paramId_t param, uint32_t keyValue)
	{
		stepFocusedId = -1;

		switch (barDetailView[keyFocusedItem])
		{
		case GRID_NODE:
		case PARAM:
		case TIMEBASE:
		case DIRECTION:
		case LAST_STEP:
		case PRESET:
			stepControl.pannelSendKey(isFocuse, param, keyValue);
			break;
		case INSERT_BAR:
			barInsert.FPKeyHandler(isFocuse, param, keyValue);
			break;
		case GBAR_SYNC:
			gbarSync.FPKeyHandler(isFocuse, param, 0);
			break;
		case XPOSE_DEFEAT:
			xposeDefeat.FPKeyHandler(isFocuse, param, 0);
			break;
		case SCULPT:
			if (sculptBtn.isEnabled())
				sculptBtn.FPKeyHandler(isFocuse, param, 0);
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					keyFocusedItem++;
				else
					keyFocusedItem--;

				pannelSendKey(isFocuse, param, keyValue);
			}
			break;
		case ROTATE_PATTERN:
			patternRotateBtn.FPKeyHandler(isFocuse, param, 0);
			if (stepControl.getFirstStepEditStatus())
				stepControl.setFirstStepFPHandler(isFocuse, param, keyValue);
			break;
		case BAR_LEFT:
			leftNudgeButton.FPKeyHandler(isFocuse, param, 0);
			break;
		case BAR_RIGHT:
			rightNudgeButton.FPKeyHandler(isFocuse, param, 0);
			break;
		case BAR_REPEAT:
			barLoopCount.FPKeyHandler(isFocuse, param, keyValue);
			break;
		default:
			break;
		}
		repaint();
	}

	void StepSequencer::pannelSendOptionsKey(bool isFocuse, paramId_t param, uint32_t keyValue)
	{
		switch (barOptionsView[barKeyFocusedItem])
		{
		case BAR_GANG_EDIT:
			gangBtn.FPKeyHandler(isFocuse, param, 0);
			break;
		case BAR_NOTE_EDIT:
			imageButtonPencil.FPKeyHandler(isFocuse, param, 0);
			break;
		case BAR_COPY:
			if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
				copy.FPKeyHandler(isFocuse, param, 0);
			else
				copy.setState((Button::ButtonState)isFocuse);
			break;
		case BAR_LOOP:
			if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
				this->postCommandMessage(2);
			else
				loop.setState((Button::ButtonState)isFocuse);
			break;
		case BAR_UNDO:
			if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
				optionUndo.FPKeyHandler(isFocuse, param, keyValue);
			optionUndo.setState((Button::ButtonState)isFocuse);
			break;
		default:
			break;
		}
		repaint();
	}

	void StepSequencer::handleCommandMessage(int cmdid)
	{
		auto _pPatternPtr = IgnitePattern::getAppPatternInstance();

		if (cmdid == 1)
		{
			_pMainmodePtr = AppMainmode::getMainModeInstance();
			_pMainmodePtr->patternBarCopyComp->setMaxBarIndex((int)_pPatternPtr->getPatternData(mt::Ignite::ptDataIndex::BARNUM) + 1);
			_pMainmodePtr->patternBarCopyComp->setVisible(true);
			_pMainmodePtr->patternBarCopyComp->runModalLoop();
		}
		else if (cmdid == 2)
			buttonClicked(&loop);
		else if (cmdid == 5)
			stepPreset._pRampPreset->nextPresetValueType();
		else if (cmdid == 6)
			stepPreset._pPulsePreset->nextPresetValueType();
		else if (cmdid == 7)
			stepPreset._pTriangle->nextPresetValueType();
		else if (cmdid == 12)
		{
			if(_pPatternPtr->getInsertKeyStatus())
				_pInfoWindow->setTextandShow("INSERT: ON");
			else
				_pInfoWindow->setTextandShow("INSERT: OFF");
		}
	}

	void StepSequencer::pannelSendPresetKey(bool isFocuse, paramId_t param, uint32_t keyValue)
	{
		switch (barPresetView[barPresetKeyFocusedItem])
		{
		case BAR_PRESET_RAMP:
			if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
				postCommandMessage(5);
			break;
		case BAR_PRESET_PULSE:
			if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
				postCommandMessage(6);
			break;
		case BAR_PRESET_TRIANGLE:
			if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
				postCommandMessage(7);
			break;
		case BAR_PRESET_INVERSE:
			stepPreset.inverseLevels.FPKeyHandler(isFocuse, param, keyValue);
			break;
		case BAR_PRESET_FLIP:
			stepPreset.flipLevels.FPKeyHandler(isFocuse, param, keyValue);
			break;
		default:
			break;
		}
		repaint();
	}
}
