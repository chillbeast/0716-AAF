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
	juce::Array<mt::Ignite::patternData> gPatterns;
	////////////////////////////////////////////////
	//   Class: Ignite Pattern
	////////////////////////////////////////////////
	IgnitePattern::IgnitePattern()
	{
		addNewP.makeVisibleAndAddListner(this);
		addNewInList.makeVisibleAndAddListner(this);
		addNewP.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		addNewP.setColour(juce::TextEditor::outlineColourId, Theme::Instance().getColour(customBlue13));
		addNewP.setColour(TextButton::ColourIds::textColourOffId, Theme::Instance().getColour(customBlue13));
		addNewP.setColour(TextButton::ColourIds::buttonColourId, Theme::Instance().getColour(customdblue));
		addNewP.setBounds(Rectangle(xStartAddNewBtn, yStartAddNewBtn, widthAddNewBtn, heightAddNewBtn));
		addNewP.setVisible(true);
		addNewInList.setBounds(Rectangle(xStartAddNewInList, yStartAddNewInList, widthAddNewInList, heightAddNewInList));
		addNewInList.setToggleable(false);
		addNewInList.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		addNewInList.setVisible(false);

		addAndMakeVisible(view);
		addAndMakeVisible(container);

		view.setViewedComponent(&container, true);
		view.setBounds(xStartPatternListLayout, yStartPatternListLayout, widthPatternListLayout, heightPatternListLayout - 26);
		view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);
		view.setScrollBarThickness(5);
		view.setScrollBarsShown(true, false, true, false);
		container.setVisible(true);
		view.setVisible(true);

		curretPatternRow = -1;
		selected = 0;

		ClientPatternStepData sData;
		_pStepComp = IgniteStep::getStepInstance(this, sData);
		addAndMakeVisible(_pStepComp);
		_pStepComp->setBounds(xStartSongView1, yStartSongView1, widthSongView1, heightSongView1);
		_pStepComp->setVisible(false);

		patternName.setFont(juce::Font(font_13, juce::Font::plain).withTypefaceStyle("Regular"));
		patternName.setJustificationType(juce::Justification::centredLeft);
		patternName.setEditable(false, false, false);
		patternName.setColour(juce::Label::backgroundColourId, juce::Colour(ui::CommonColorID::neutralGrey4));
		patternName.setColour(juce::Label::outlineColourId, Colours::white);
		patternName.makeVisible(this);
		patternName.setVisible(false);
		patternName.toBack();

		patternListNameLable.setFont(juce::Font(font_10, juce::Font::plain).withTypefaceStyle("Regular"));
		patternListNameLable.setJustificationType(juce::Justification::centred);
		patternListNameLable.setEditable(false, false, false);
		patternListNameLable.setColour(juce::Label::backgroundColourId, juce::Colour(ui::CommonColorID::neutralGrey4));
		patternListNameLable.setColour(juce::Label::outlineColourId, Colours::white);
		patternListNameLable.makeVisible(this);
		patternListNameLable.setVisible(true);

		patternDirectionLable.setFont(juce::Font(font_10, juce::Font::plain).withTypefaceStyle("Regular"));
		patternDirectionLable.setJustificationType(juce::Justification::centred);
		patternDirectionLable.setEditable(false, false, false);
		patternDirectionLable.setColour(juce::Label::backgroundColourId, juce::Colour(ui::CommonColorID::neutralGrey4));
		patternDirectionLable.setColour(juce::Label::outlineColourId, Colours::white);
		patternDirectionLable.makeVisible(this);
		patternDirectionLable.setVisible(true);

		patternNumBarLable.setFont(juce::Font(font_10, juce::Font::plain).withTypefaceStyle("Regular"));
		patternNumBarLable.setJustificationType(juce::Justification::centred);
		patternNumBarLable.setEditable(false, false, false);
		patternNumBarLable.setColour(juce::Label::backgroundColourId, juce::Colour(ui::CommonColorID::neutralGrey4));
		patternNumBarLable.setColour(juce::Label::outlineColourId, Colours::white);
		patternNumBarLable.makeVisible(this);
		patternNumBarLable.setVisible(true);

		patternTimebaseLable.setFont(juce::Font(font_10, juce::Font::plain).withTypefaceStyle("Regular"));
		patternTimebaseLable.setJustificationType(juce::Justification::centred);
		patternTimebaseLable.setEditable(false, false, false);
		patternTimebaseLable.setColour(juce::Label::backgroundColourId, juce::Colour(ui::CommonColorID::neutralGrey4));
		patternTimebaseLable.setColour(juce::Label::outlineColourId, Colours::white);
		patternTimebaseLable.makeVisible(this);
		patternTimebaseLable.setVisible(true);

		patternLastStepLable.setFont(juce::Font(font_10, juce::Font::plain).withTypefaceStyle("Regular"));
		patternLastStepLable.setJustificationType(juce::Justification::centred);
		patternLastStepLable.setEditable(false, false, false);
		patternLastStepLable.setColour(juce::Label::backgroundColourId, juce::Colour(ui::CommonColorID::neutralGrey4));
		patternLastStepLable.setColour(juce::Label::outlineColourId, Colours::white);
		patternLastStepLable.makeVisible(this);
		patternLastStepLable.setVisible(true);

		patternRotateLable.setFont(juce::Font(font_10, juce::Font::plain).withTypefaceStyle("Regular"));
		patternRotateLable.setJustificationType(juce::Justification::centred);
		patternRotateLable.setEditable(false, false, false);
		patternRotateLable.setColour(juce::Label::backgroundColourId, juce::Colour(ui::CommonColorID::neutralGrey4));
		patternRotateLable.setColour(juce::Label::outlineColourId, Colours::white);
		patternRotateLable.makeVisible(this);
		patternRotateLable.setVisible(true);

		_pComponentStep = StepSequencer::getStepSequencerInstance();
		addAndMakeVisible(_pComponentStep);
		_pComponentStep->setBounds(0, 0, layout::middle.getWidth(), layout::middle.getHeight() + 15);
		_pComponentStep->setVisible(false);

		_pComponentGrid = AppGrid::getGridInstance();
		addAndMakeVisible(_pComponentGrid);
		_pComponentGrid->setBounds(0, 0, layout::middle.getWidth(), layout::middle.getHeight() - 7);
		_pComponentGrid->setVisible(false);

		_pEmptyPatternComp = new class AppDnp();
		addChildComponent(_pEmptyPatternComp);
		_pEmptyPatternComp->setBounds(xStartEmptyComp, yStartEmptyComp, widthEmptyComp, heightEmptyComp);
		_pEmptyPatternComp->setVisible(true);
		_pEmptyPatternComp->setDisplayMessage(String(" "));
		_pEmptyPatternComp->setBackgrounImage(juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("img-empty-state-pattern.png")));

		addAndMakeVisible(barView);
		barView.addListener(this);
		barView.setToggleState(false, dontSendNotification);
		barView.setVisible(true);
		barView.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		barView.sethoverImages("btn-pattern-bar-default-hover.png", "btn-pattern-bar-active-hover.png", "btn-pattern-bar-disabled-hover.png");

		addAndMakeVisible(listView);
		listView.addListener(this);
		listView.setToggleState(true, dontSendNotification);
		listView.setVisible(true);
		listView.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		listView.sethoverImages("btn-song-list-default-hover.png", "btn-song-list-active-hover.png", "btn-song-list-default-hover.png");

		addAndMakeVisible(stepView);
		stepView.addListener(this);
		stepView.setToggleState(false, dontSendNotification);
		stepView.setVisible(true);
		stepView.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		stepView.sethoverImages("btn-pattern-step-default-hover.png", "btn-pattern-step-active-hover.png", "btn-pattern-step-disabled-hover.png");

		addAndMakeVisible(gridView);
		gridView.addListener(this);
		gridView.setToggleState(false, dontSendNotification);
		gridView.setVisible(true);
		gridView.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		gridView.sethoverImages("btn-pattern-grid-default-hover.png", "btn-pattern-grid-active-hover.png", "btn-pattern-grid-disabled-hover.png");

		patternBox.items.add(FlexItem(listView).withFlex(1));
		patternBox.items.add(FlexItem(stepView).withFlex(1));
		patternBox.items.add(FlexItem(barView).withFlex(1));
		patternBox.items.add(FlexItem(gridView).withFlex(1));

		patternLableBox.items.add(FlexItem(patternListNameLable).withMargin({ 10, 10, 10, 10 }).withFlex(1.33f));
		patternLableBox.items.add(FlexItem(patternDirectionLable).withMargin({ 10, 10, 10, 10 }).withFlex(1));
		patternLableBox.items.add(FlexItem(patternNumBarLable).withMargin({ 10, 10, 10, 10 }).withFlex(1));
		patternLableBox.items.add(FlexItem(patternTimebaseLable).withMargin({ 10, 10, 10, 10 }).withFlex(1));
		patternLableBox.items.add(FlexItem(patternLastStepLable).withMargin({ 10, 10, 10, 10 }).withFlex(1));
		patternLableBox.items.add(FlexItem(patternRotateLable).withMargin({ 10, 10, 10, 10 }).withFlex(1));

		_pAppOptPtr = AppOptions::GetOptionsInstance();

		_pPatternOptCtl = IgniteControlOptions::GetAppOptInstance();
		_pPatternOptCtl->setBounds(xStartOptctl, yStartOptctl, widthOptctl, heightOptctlt);
		_pPatternOptCtl->addOptCtlListener(this);
		enableViewBox((PatternView)-1);
		_pInfoWindow = CommonInfoWindow::GetInfoWindowInstance();

		_pBarLoopComp = new IgniteLoopControl(&barList);
		addChildComponent(_pBarLoopComp);
		_pBarLoopComp->setBounds(129, 64, 766, 350);
		_pBarLoopComp->setTitle("Pattern Bar Loop");
		_pBarLoopComp->setVisible(false);
		_pPatternStatusBarPtr = PatternStatusBar::GetPatternBarInstance();
		_pBarLoopComp->onButtonPressed = [this](IgniteLoopControl::loopKey key, int loopStartIndex, int loopEndIndex)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;
			setLoopControl(key, loopStartIndex - 1, loopEndIndex - 1);
		};

		_pPatternCopyComp = new IgniteCopy();
		addChildComponent(_pPatternCopyComp);
		_pPatternCopyComp->setBounds(xStartLoopCmp, yStartLoopCmp, widthCopy, heightCopy);
		_pPatternCopyComp->setVisible(false);

		_pPopUpPtr = CommonPopUp::getCommonPopupInstance();
		_pWarnPopupPtr = CommonWarningPopUp::getCommonWarningPopupInstance();
		scenePtr = IgniteScene::getAppSceneInstance();

		_pPatternCopyComp->onButtonPressed = [this](IgniteCopy::copyCtlKey key, String copyName)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			copyPattern(key, copyName);
		};
		updateDataFromEngine();
	}

	IgnitePattern::~IgnitePattern()
	{
		cleanup();
	}

	void IgnitePattern::copyPatternBar(IgniteBarCopy::copyBarCtlKey key, int fromBarId, int toBarId)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		_pMainmodeptr = AppMainmode::getMainModeInstance();

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "From Bar ID : " + String(fromBarId) + "To Bar ID : " + String(toBarId));
		if (key == IgniteBarCopy::copyBarCtlKey::COPY)
		{
			if (fromBarId != toBarId)
			{
				_PUiVirtualEngine->EngineCopyPatternBar(gPatternRef.enginePatternData.patternName, (uint16_t)fromBarId, (uint16_t)toBarId);
				updateDataFromEngine();
				repaint();
				setSelected(getSelected());
				list[selected]->updateLastStep(false);
				_pComponentStep->updateLastStep();
				_pPatternStatusBarPtr->onNumbofBarchangePattern(gPatternRef.enginePatternData.numBars - 1);
				setPatternEditIndication(true);

				_pMainmodeptr->patternBarCopyComp->exitModalState(0);
				_pMainmodeptr->patternBarCopyComp->setVisible(false);
			}
			else
			{
				_pWarnPopupPtr->close();
				_pWarnPopupPtr->setWarningText("Source and Destination bar position is same !!!");
				_pWarnPopupPtr->show();
				_pWarnPopupPtr->onSelectionChanged = [this](int r)
				{
					if (auto dw = getCurrentlyModalComponent())
						dw->exitModalState(0);
				};
			}
		}
		else if (key == IgniteBarCopy::copyBarCtlKey::CANCEL)
		{
			_pMainmodeptr->patternBarCopyComp->exitModalState(0);
			_pMainmodeptr->patternBarCopyComp->setVisible(false);
		}
	}

	bool IgnitePattern::insertBlankStep()
	{
		if (!patternCreated || selected < 0 || selected >= gPatterns.size()) return false;
		const auto& pattern = gPatterns.getReference(selected);
		if (!_PUiVirtualEngine->EngineInsertPatternStep(pattern.enginePatternData.patternName,
			uint16_t(pattern.barIndex), 1, uint8_t(getCurrentBarStepId(pattern.stepIndex)), {})) return false;
		requestDataFromEngine(selected);
		refreshPatternUI();
		setPatternEditIndication(true);
		repaint();
		return true;
	}

	void IgnitePattern::insertBar(IgniteBarInsert::insertCtlKey key, int barNumber)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		_pMainmodeptr = AppMainmode::getMainModeInstance();

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Bar ID : " + String(barNumber));

		if (key == IgniteBarInsert::insertCtlKey::INSERT)
		{
			if ((gPatternRef.enginePatternData.numBars - 1) < 15)
			{
				_PUiVirtualEngine->EngineInsertPatternBar(gPatternRef.enginePatternData.patternName, (uint16_t)barNumber);
				updateDataFromEngine();
				repaint();
				setSelected(getSelected());
				list[selected]->updateLastStep(false);
				_pComponentStep->updateLastStep();
				_pPatternStatusBarPtr->onNumbofBarchangePattern((gPatternRef.enginePatternData.numBars - 1));
				setPatternEditIndication(true);

				if (_pMainmodeptr->barInsertComp->isVisible())
				{
					_pMainmodeptr->barInsertComp->exitModalState(0);
					_pMainmodeptr->barInsertComp->setVisible(false);
					getSelectedBarAndUpdate(getCurrentBarId());
				}
			}
			else
			{
				_pWarnPopupPtr->close();
				_pWarnPopupPtr->setWarningText("Reached Maximum number of bars !!!");
				_pWarnPopupPtr->show();
				_pWarnPopupPtr->onSelectionChanged = [this](int r)
				{
					if (auto dw = getCurrentlyModalComponent())
						dw->exitModalState(0);
				};

			}
		}
		else if (key == IgniteBarInsert::insertCtlKey::CANCEL)
		{
			_pMainmodeptr->barInsertComp->exitModalState(0);
			_pMainmodeptr->barInsertComp->setVisible(false);
		}
	}

	void IgnitePattern::copyPattern(IgniteCopy::copyCtlKey key, String copyName)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "new name : " + copyName);

		if (copyName.isNotEmpty())
		{
			if (key == IgniteCopy::copyCtlKey::COPY)
			{
				_PUiVirtualEngine->EnginecopyPattern(gPatternRef.enginePatternData.patternName, copyName.toStdString());
				updateDataFromEngine();
			}
		}
		_pPatternCopyComp->exitModalState(0);
		_pPatternCopyComp->setVisible(false);
	}

	void IgnitePattern::updateAccidentals(bool state)
	{
		if (patternCreated)
		{
			auto& gPatternRef = gPatterns.getReference(selected);

			_pComponentStep->updateAccidentals(state);
			_pStepComp->updateAccidentals(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)), state);
			isAccidentalEnabled = state;
		}
	}

	void IgnitePattern::setLoopControl(IgniteLoopControl::loopKey key, int loopStartIndex, int loopEndIndex)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " loopStartIndex : " + String(loopStartIndex) + "loopEndIndex : " + String(loopEndIndex));

		if ((key == IgniteLoopControl::loopKey::APPLY))
		{
			_pPatternStatusBarPtr->PatternloopSelOnStatBar(loopStartIndex, loopEndIndex);
			setPatternData(mt::Ignite::ptDataIndex::BAR_LOOP_ON, (uint8_t)true, false);
			setPatternData(mt::Ignite::ptDataIndex::BAR_LOOP_START, (uint8_t)loopStartIndex, false);
			setPatternData(mt::Ignite::ptDataIndex::BAR_LOOP_END, (uint8_t)loopEndIndex, false);
			setPatternEditIndication(true);

		}
		else if ((key == IgniteLoopControl::loopKey::CANCEL))
		{
		}
		else if (key == IgniteLoopControl::loopKey::CLEAR)
		{
			_pPatternStatusBarPtr->PatternloopSelOnStatBar(-1, -1);
			setPatternData(mt::Ignite::ptDataIndex::BAR_LOOP_ON, (uint8_t)false, false);
			setPatternEditIndication(true);
		}

		_pBarLoopComp->exitModalState(0);
		_pBarLoopComp->setVisible(false);
	}

	void IgnitePattern::updateBarList()
	{
		barList.clear();
		barList.push_back("No Bar Selected");
		for (auto i = 0; i <= gPatterns[selected].enginePatternData.numBars - 1; i++)
		{
			barList.push_back("BAR NUM: " + String(i + 1));
		}
	}

	void IgnitePattern::rotatePattern(uint16_t barId, uint16_t stepId)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " New start step ID : " + String(stepId));

		_PUiVirtualEngine->PatternMngrPatternRotate(gPatternRef.enginePatternData.patternName, barId, stepId);
		setSelected(selected);
		if (_pComponentStep->isVisible())
		{
			_pComponentStep->refreshBarData();
			_pComponentStep->repaint();
		}
	}

	void IgnitePattern::updateDataFromEngine()
	{
		auto patternList = _PUiVirtualEngine->EngineGetPatternList();
		auto patternDispNameList = _PUiVirtualEngine->EngineGetPatternList(true);
		int activePattern = getSelected();
		selected = 0;

		list.clear();
		gPatterns.clear();

		if (patternList.size() != 0)
		{
			auto it_dispNameList = patternDispNameList.begin();
			for (auto& i : patternList)
			{
				mt::Ignite::patternData pdata;
				setDefaultPdata(&pdata, mt::Ignite::ptype::MONO);
				pdata.enginePatternData = _PUiVirtualEngine->EngineGetPatternData(i);
				addPattern(&pdata, false);
				list[selected]->refreshPatternRowUI();
				selected++;
				it_dispNameList++;
			}
			selected = activePattern;
			enableViewBox(PatternView::LIST);
		}
		else
			patternCreated = statusBarData.isPatternCreated = false;

		if (!initCompleted)
			initCompleted = true;
		else
			refreshPatternUI();
	}

	void IgnitePattern::requestDataFromEngine(int index)
	{
		auto& gPatternRef = gPatterns.getReference(index);
		gPatternRef.enginePatternData = _PUiVirtualEngine->EngineGetPatternData(gPatternRef.enginePatternData.patternName);
	}

	int IgnitePattern::getStepId(int stepId)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		if (stepId < 16)
			return stepId;
		else
		{
			return (stepId - (gPatternRef.barIndex * 16));
		}
	}

	int IgnitePattern::getActiveStepIndex()
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		return (gPatternRef.barIndex * 16);
	}

	int IgnitePattern::getCurrentBarStepId(int step_id)
	{
		if (step_id < 16)
			return step_id;
		return (step_id - (gPatterns.getReference(selected).barIndex * 16));
	}

	void IgnitePattern::setGangStep(int step_index, bool state)
	{
		selectedStepInGang[step_index + (gPatterns.getReference(selected).barIndex * 16)] = state;
	}

	bool IgnitePattern::getGangStepState(int step_index)
	{
		return selectedStepInGang[step_index + (gPatterns.getReference(selected).barIndex * 16)];
	}

	void IgnitePattern::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
	{
	}

	void IgnitePattern::showPopup(int index)
	{
		patternName.setText((String)gPatterns[selected].enginePatternData.patternDispName, dontSendNotification);
		if ((index == selected) || !patternDataUpdated)
		{
			setSelected(index);
			return;
		}
		prevSelected = index;
		if (_pPopUpPtr->isDialogOpen.get() != true)
		{
			_pPopUpPtr->close();
			_pPopUpPtr->setPopUpPropories("Switch Pattern", 604, 290);
			_pPopUpPtr->setbtnsize(184, 44);
			_pPopUpPtr->addImageButton("Cancel", "btn-cancel-174x44-default-hover.png", "btn-cancel-174x44-default.png", 2);
			_pPopUpPtr->addImageButton("Confirm Switch", "btn-confirmswitch-174x44-default.png", "btn-confirmswitch-174x44-default-hover.png", 1);
			_pPopUpPtr->addImageButton("Save and Switch", "btn-saveswitch-174x44-default.png", "btn-saveswitch-174x44-default-hover.png", 3);
			_pPopUpPtr->popupType = POPUP_CHANGEWARNING;
			_pPopUpPtr->isCloseButtonVisible(false);
			_pPopUpPtr->show();
			_pPopUpPtr->setInfoText("Unsaved data of current pattern will be lost!!!");
			_pPopUpPtr->onSelectionChanged = [this](int r)
			{
				auto& gPatternRef = gPatterns.getReference(selected);

				if (r == 1)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramClearUndoList, gPatternRef.enginePatternData.patternName);
					patternDataUpdated = false;
					updateLables();
					setSelected(prevSelected);
				}
				if (r == 3)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);
					patternDataUpdated = false;
					updateLables();
					setSelected(prevSelected);
				}

				if (r == 2)
					patternRowHighLightIndex = prevSelected - 1;

				if (auto dw = getCurrentlyModalComponent())
					dw->exitModalState(0);
			};
		}

	}

	void IgnitePattern::labelTextChanged(Label* labelThatHasChanged)
	{
	}

	int IgnitePattern::getSelected(void)
	{
		return selected;
	}

	void IgnitePattern::setSelected(int index)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "set selected pattern index : " + String(index));

		if (!isPositiveAndBelow(index, list.size()))
			return;

		selected = index;
		refreshStatusbar();

		if (_pPlayHead != nullptr)
			_pPlayHead->setVisible(selected == editPattern);

		// Highlist selected row.
		for (auto& i : list)
			i->highlight(false);

		if (patternCreated && this->isVisible())
		{
			list[selected]->highlight(true);

			int rowHeightOffset = (index + 1) * ROW_HEIGHT;
			int viewAreaHeight = view.getViewArea().getHeight();
			int viewPosY = view.getViewPositionY();

			if (rowHeightOffset > (viewAreaHeight + viewPosY - ROW_HEIGHT))
				view.setViewPosition(view.getViewPositionX(), viewPosY + viewAreaHeight - ROW_HEIGHT);
			else if ((index * ROW_HEIGHT) <= viewPosY)
				view.setViewPosition(view.getViewPositionX(), viewPosY - viewAreaHeight + ROW_HEIGHT);

			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatterns[index].enginePatternData.patternName);

			requestDataFromEngine(selected);

			setPatternIndex(selected);
			list[selected]->refreshPatternRowUI();

			statusBarData.numberOfBars = gPatterns[selected].enginePatternData.numBars;
			statusBarData.numberOfSteps = gPatterns[selected].enginePatternData.patternLastStepId;
			statusBarData.timeBase = gPatterns[selected].enginePatternData.timeBase;
			statusBarData.editPattern = editPattern;

			if ((opMode != mt::Ignite::operatioMode::PLAY_MODE) && (opMode != mt::Ignite::operatioMode::PLAY_REC_MODE)) {
				enableControlButton(IgniteControlOptions::DEL, list.size() > 0);
				enableControlButton(IgniteControlOptions::COPY, list.size() > 0);

				if (list.size() > 1) {
					enableControlButton(IgniteControlOptions::MOVE, true);
					enableControlButton(IgniteControlOptions::LOOP, barView.getToggleState());
				}
				else {
					enableControlButton(IgniteControlOptions::MOVE, false);
				}

				PatternStatusBar::GetPatternBarInstance()->onNumbofBarchangePattern(gPatterns[selected].enginePatternData.numBars - 1);
			}

			if ((patternDataUpdated) && (!IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled()))
			{
				enableControlButton(IgniteControlOptions::UNDO, true);
				enableControlButton(IgniteControlOptions::SAVE, true);
			}

			if (getTabIndex() == 0)
				setFpPatternLedState();
		}
	}

	void IgnitePattern::cleanup()
	{
		container.removeAllChildren();
		list.clear();
	}

	String IgnitePattern::getPatternName()
	{
		auto patternNameList = _PUiVirtualEngine->EngineGetPatternList();
		auto patternDispNameList = _PUiVirtualEngine->EngineGetPatternList(true);
		std::list<std::string>::const_iterator dispIt = patternDispNameList.begin();
		std::list<std::string>::const_iterator nameIt = patternNameList.begin();
		auto gPatternsSize = gPatterns.size();

		int index = 0;
		for (int j = 0; j < gPatternsSize && dispIt != patternDispNameList.end() && nameIt != patternNameList.end(); ++j, ++dispIt, ++nameIt)
		{
			auto& gPatternRef = gPatterns.getReference(j);
			gPatternRef.enginePatternData.patternDispName = *dispIt;
			gPatternRef.enginePatternData.patternName = *nameIt;

			if ((patternNameList.size() - 1) == index++)
				return *nameIt;
		}
		return "";
	}

	void IgnitePattern::populate(mt::Ignite::patternData* data, bool isPatternUpdateneeded, int pattern_index)
	{
		IgnitePatternRow* row;
		String name = getPatternName();

		if (name != "")
			data->enginePatternData.patternName = name.toStdString();

		row = new IgnitePatternRow(list.size(), *data, this);
		if (pattern_index != -1)
			list.insert(pattern_index, row);
		else
			list.add(row);
		container.addAndMakeVisible(row);
		if (isPatternUpdateneeded)
			refreshPatternUI();
		IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
	}

	void IgnitePattern::refreshStatusbar()
	{
		if (patternCreated)
		{
			_pPatternStatusBarPtr->onNumbofBarchangePattern(gPatterns[selected].enginePatternData.numBars - 1);

			if (gPatterns[selected].enginePatternData.barLoopOn)
				_pPatternStatusBarPtr->PatternloopSelOnStatBar(gPatterns[selected].enginePatternData.barLoopStartId,
					gPatterns[selected].enginePatternData.barLoopEndId);
		}
	}

	void IgnitePattern::refreshBarLoop()
	{
		if (gPatterns[selected].enginePatternData.barLoopOn)
		{
			_pBarLoopComp->setLoopStartIndex(gPatterns[selected].enginePatternData.barLoopStartId + 1);
			_pBarLoopComp->setLoopEndIndex(gPatterns[selected].enginePatternData.barLoopOn + 1);
		}
		else
		{
			_pBarLoopComp->setLoopStartIndex(0);
			_pBarLoopComp->setLoopEndIndex(0);
		}
	}

	void IgnitePattern::refreshPatternUI()
	{
		int index = 0;

		if (!initCompleted)
			updateDataFromEngine();

		int rowHeightIncrement = heightSongList + 8;
		for (auto& i : list)
		{
			i->setBounds(0, (rowHeightIncrement * index++), widthSongList, heightSongList);
			i->onMouseDown = [this](IgnitePatternRow* Row)
			{
				showPopup((Row->getSerialNumber()));
			};
		}

		// Set overall view container size....
		container.setSize(widthScreen - 20, ((index + 1) * (heightSongContainer + 8)));

		if (patternCreated)
		{
			setSelected(selected);
			list[selected]->updateLastStep(true);
		}
		else
		{
			_pEmptyPatternComp->setVisible(true);
		}

		bool addNewPVisible = addNewP.isVisible();
		stepView.setVisible(!addNewPVisible);
		gridView.setVisible(!addNewPVisible);
		listView.setVisible(!addNewPVisible);
		barView.setVisible(!addNewPVisible);
		if(getTabIndex() == 0 || getTabIndex() == 1)
			_pAppOptPtr->showOptions(!addNewPVisible);
	}

	void IgnitePattern::setDefaultPdata(mt::Ignite::patternData* data, mt::Ignite::ptype type)
	{
		data->enginePatternData.patternName = String("PATTERN-" + String(list.size() + 1)).toStdString();
		data->enginePatternData.timeBase = statusBarData.timeBase = (uint8_t)mt::Ignite::timebase::FOUR;
		data->enginePatternData.numBars = statusBarData.numberOfBars = 1;
		data->enginePatternData.patternLastStepId = statusBarData.numberOfSteps = 15;
		data->id = patternIndex = list.size();
		data->barIndex = data->stepIndex = 0;
		data->nodeIndex = IgnitePatternConfig::getAppPatternConfigInstance()->getDefaultNote();
	}

	void IgnitePattern::addPattern(mt::Ignite::patternData* data, bool isPatternUpdateneeded, bool isPatternCreatedInTrack)
	{
		defaultNote = IgnitePatternConfig::getAppPatternConfigInstance()->getDefaultNote();
		ClienPatterntBarData bData = { 0, 16 };
		ClientPatternStepData sData = { 0, 1, 1, {}, true, false, false, false };
		Ignite::ClientPatternNodeData nData = { (uint16_t)defaultNote, (uint8_t)defaultNote, 38, 0, 15, 0, false };

		if (isPatternUpdateneeded)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramAddPattern, 0);

		sData.nodeList[nData.note].emplace(
			std::piecewise_construct,
			std::forward_as_tuple(nData.offset),
			std::forward_as_tuple(nData)
		);

		sData.BarId = 0;
		for (uint8_t i = 0; i < bData.stepCount; i++)
		{
			sData.stepId = i;
			bData.StepDataList.emplace((uint8_t)i, sData);
		}

		data->enginePatternData.BarDataList.emplace((uint8_t)sData.BarId, bData);

		curretPatternRow++;
		gPatterns.add({ data->id, data->enginePatternData, false, 0, 0, defaultNote });
		patternCreated = statusBarData.isPatternCreated = true;
		if (getTabIndex() == 0)
			addNewInList.setVisible(true);
		addNewP.setVisible(false);
		_pEmptyPatternComp->setVisible(false);
		populate(data, isPatternUpdateneeded);
		enableViewBox(PatternView::LIST);

		if (isPatternCreatedInTrack == false && isPatternCreated() && isInitCompleted())
			setFpPatternLedState();
	}

	void IgnitePattern::setPatternEditIndication(bool state)
	{
		if (!patternDataUpdated && !IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled())
		{
			patternDataUpdated = state;
			updatePatternEditIndication();
			enableControlButton(IgniteControlOptions::SAVE, state);
			enableControlButton(IgniteControlOptions::UNDO, state);
		}
	}

	void IgnitePattern::addStepEvent(int barIndex, int stepIndex)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto auxVec = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).auxList;

		Aux auxEvent{ false, (uint16_t)AuxEventType::controlChange, 0, 0 };

		for (auto i = 0; i < 128; i++)
		{
			if (std::find(auxVec.begin(), auxVec.end(), auxEvent) == auxVec.end())
			{
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).auxList.push_back(auxEvent);
				_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_StepData);
				return;
			}
			else
			{
				auxEvent.data1 = (int8_t)i;
				continue;
			}
		}
	}

	void IgnitePattern::delStepEvent(int eventType, int barIndex, int stepIndex)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto index = 0;
		auto& auxVec = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).auxList;

		for (auto& i : auxVec)
		{
			if (i.data1 == eventType)
			{
				auxVec.erase(auxVec.begin() + index++);
				_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_StepData);
				return;
			}
		}
	}

	void IgnitePattern::addNextNote(int barIndex, int stepIndex)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		for (auto i = 0; i < 128; i++)
		{
			if (gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.count((uint8_t)i) == 0)
			{
				addNote(barIndex, stepIndex, i);
				return;
			}
		}
	}

	void IgnitePattern::addNote(int barIndex, int stepNum, int noteNum)
	{
		Ignite::ClientPatternNodeData nData = { (uint16_t)noteNum, (uint8_t)noteNum, 38, 0, 15, 0, false };
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Add note : " + String(noteNum) + " at step : " + String(stepNum));
		if (barIndex < 0)
			barIndex = gPatternRef.barIndex;

		auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepNum);
		if (stepDataIter.nodeList.count((uint8_t)noteNum) == 0)
		{
//			stepDataIter.nodeList[nData.note].emplace((uint8_t)noteNum, nData);
			stepDataIter.nodeList[nData.note].emplace(
				std::piecewise_construct,
				std::forward_as_tuple(nData.offset),
				std::forward_as_tuple(nData)
			);
			stepDataIter.nodeCount += 1;
			_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Note Added : " + String(noteNum) + " at step : " + String(stepNum));
		}
		if (noteNum == gPatterns[selected].nodeIndex)
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
			for (int i = 0; i < 16; i++)
				if (isNotePresent(i, noteNum))
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + i), fpLEDPattern::EN_RGB_LED_WHITE);
		}
	}

	void IgnitePattern::addNotes(int sMin, int sMax, int nMin, int nMax)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		nMin = std::max(0, nMin);
		nMax = std::min(127, nMax);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Add note : " + String(nMin) + " to note : " + String(nMax) + " at start step : " + String(sMin) + " to step : " + String(sMin));

		Ignite::ClientPatternNodeData nData;
		for (auto i = sMin; i <= sMax; i++)
		{
			auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i);
			for (auto j = nMin; j <= nMax; j++)
			{
				nData = { (uint16_t)j, (uint8_t)j, 38, 0, 15, 0, false };
//				stepDataIter.nodeList[nData.note].emplace((uint8_t)j, nData);
				stepDataIter.nodeList[nData.note].emplace(
					std::piecewise_construct,
					std::forward_as_tuple(nData.offset),
					std::forward_as_tuple(nData)
				);
			}
			stepDataIter.nodeCount = (uint8_t)stepDataIter.nodeList.size();
		}
		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
		
		if (nMin == gPatterns[selected].nodeIndex)
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
			for (int i = 0; i < 16; i++)
				if (isNotePresent(i, nMin))
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + i), fpLEDPattern::EN_RGB_LED_WHITE);
		}
	}

	bool IgnitePattern::delNote(int barIndex, int stepNum, int noteNum, bool isUpdateEngine)
	{
		auto isNoteDeleted = false;
		auto& gPatternRef = gPatterns.getReference(selected);

		if (noteNum < 0)
			noteNum = gPatternRef.nodeIndex;

		if (barIndex < 0)
			barIndex = gPatternRef.barIndex;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Delete note : " + String(noteNum) + " at step : " + String(stepNum));

		auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepNum);
		if (stepDataIter.nodeList.count((uint8_t)noteNum) == 1)
		{
			stepDataIter.nodeList.erase((uint8_t)noteNum);
			stepDataIter.nodeCount -= 1;
			isNoteDeleted = true;
		}

		if (noteNum == gPatterns[selected].nodeIndex)
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
			for (int i = 0; i < 16; i++)
				if (isNotePresent(i, noteNum))
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + i), fpLEDPattern::EN_RGB_LED_WHITE);
		}
		if(isNoteDeleted && isUpdateEngine)
		{
			setPatternEditIndication(true);
			_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
		}
		return isNoteDeleted;
	}

	void IgnitePattern::delNotes(int sMin, int sMax, int nMin, int nMax)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto isNoteDeleted = false;

		nMin = std::max(0, nMin);
		nMax = std::min(127, nMax);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Delete note : " + String(nMin) + " to note : " + String(nMax) + " at start step : " + String(sMin) + " to step : " + String(sMin));

		if (!selectedNotesInGrid.size())
		{
			for (auto i = sMin; i <= sMax; i++)
			{
				auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i);
				for (auto j = nMin; j <= nMax; j++)
				{
					if (stepDataIter.nodeList.count((uint8_t)j) == 1)
					{
						stepDataIter.nodeList.erase((uint8_t)j);
						isNoteDeleted = true;
					}
				}
				stepDataIter.nodeCount = (uint8_t)stepDataIter.nodeList.size();
				if (nMin == gPatterns[selected].nodeIndex)
				{
					AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
					for (int k = 0; k < 16; k++)
						if (isNotePresent(k, nMin))
							_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + k), fpLEDPattern::EN_RGB_LED_WHITE);
				}
			}
		}
		else
		{
			for (auto& node : selectedNotesInGrid)
			{
				if(delNote(-1, node.step, node.note))
					isNoteDeleted = true;
			}
			selectedNotesInGrid.clear();
		}
		if (isNoteDeleted)
		{
			setPatternEditIndication(true);
			_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
		}
	}

	void IgnitePattern::cutNote(int stepNum, int noteNum)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Cut note : " + String(noteNum) + " at step : " + String(stepNum));

		int stepIndex = getCurrentBarStepId(stepNum);
		auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)stepIndex);
		if ((stepDataIter.nodeList.find((uint8_t)noteNum) != stepDataIter.nodeList.end()) && (stepDataIter.nodeList.at((const unsigned char)noteNum).begin()->second.noteCount <= 0))
		{
			delNote(-1, stepIndex, noteNum);
		}
		else if (stepDataIter.nodeList.find((uint8_t)noteNum) != stepDataIter.nodeList.end())
		{
			stepDataIter.nodeList.at((const unsigned char)noteNum).begin()->second.noteCount--;
		}
	}

	void IgnitePattern::notesMove(bool rowRotary, bool direction)
	{

		if (rowRotary)
		{
			if (!selectedNotesInGridVec.empty() && selectedNotesInGridVec[0].note >= 0 && selectedNotesInGridVec[0].note <= 127)
			{
				// Edge cases for note limits
				if (selectedNotesInGridVec[0].note == 0 && !direction)
					return;
				if (selectedNotesInGridVec[0].note == 127 && direction)
					return;

				pasteSelectedNotes(selectedNotesInGridVec[0].step, selectedNotesInGridVec[0].note + (direction ? 1 : -1), true);

				/*remove notes*/
				for (const auto& node : selectedNotesInGrid)
					cutNote(node.note, node.step);

				for (auto& node : selectedNotesInGridVec)
					direction ? (node.note += 1) : (node.note -= 1);
				setPatternEditIndication(true);
				_pComponentGrid->moveViewPortVertically(selectedNotesInGridVec[0].note);
			}
		}
		else
		{
			if (!selectedNotesInGridVec.empty() && selectedNotesInGridVec[0].step >= 0 && selectedNotesInGridVec[0].note <= 15)
			{
				if (selectedNotesInGridVec[0].step == 0 && direction == false)
					return;
				if (selectedNotesInGridVec[0].step == 15 && direction == true)
					return;
				pasteSelectedNotes(selectedNotesInGridVec[0].step + (direction ? 1 : -1), selectedNotesInGridVec[0].note, true);
				/*remove notes*/
				for (auto i = 0; i < 16; i++)
				{
					for (auto j = 0; j < 128; j++)
						if (getSelectedNote({ i, j }))
							cutNote(i, j);
				}

				for (auto& node : selectedNotesInGridVec)
					direction ? (node.step += 1) : (node.step -= 1);

				for (auto& copiedNode : nodeListData)
				{
					direction ? (copiedNode.step_id += 1) : (copiedNode.step_id -= 1);
				}
				_pComponentGrid->moveViewPortHorizontaly(selectedNotesInGridVec[0].step);
				setPatternEditIndication(true);
			}
		}
	}

	void IgnitePattern::resetSelectedNoteRefCount()
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		/*Clear Ref count*/
		for (auto i = 0; i < 16; i++)
			for (auto j = 0; j < 128; j++)
				if (gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.count((uint8_t)j) != 0)
					gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.noteCount = 0;

		/*reset nodelist */
		nodeListData.clear();
	}

	void IgnitePattern::notesCutPaste(int stepNum, int noteNum)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Cut paste note : " + String(noteNum) + " at step : " + String(stepNum));

		noteCutPasteActive = true;
		pasteSelectedNotes(stepNum, noteNum);
		for (auto i = 0; i < 16; i++)
		{
			for (auto j = 0; j < 128; j++)
				if (getSelectedNote({ i, j }))
					cutNote(i, j);
		}
		selectedNotesInGrid.clear();
		clearCutCopy();
		setPatternEditIndication(true);
		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
		noteCutPasteActive = false;
	}

	void IgnitePattern::copyNote(int stepNum, int noteNum)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Copy note : " + String(noteNum) + " at step : " + String(stepNum));

		int stepIndex = getCurrentBarStepId(stepNum);
		auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)stepIndex);

		for (auto& it : nodeListData)
			if ((it.note == noteNum) && (it.step_id == stepNum))
				return;

		if (stepDataIter.nodeList.find((uint8_t)noteNum) != stepDataIter.nodeList.end())
		{
			stepDataIter.nodeList.find((uint8_t)noteNum)->second.begin()->second.step_id = (uint16_t)stepNum;
			nodeListData.push_back(stepDataIter.nodeList.find((uint8_t)noteNum)->second.begin()->second);
		}
	}

	void IgnitePattern::pasteSingleNote(int step, int note, ClientPatternNodeData nData)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto stepIndex = getCurrentBarStepId(step);
		auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)stepIndex);
		if (stepDataIter.nodeList.count((uint8_t)note) == 0)
		{
			nData.note = (uint8_t)note;
			nData.nodeId = (uint16_t)note;
			//stepDataIter.nodeList[nData.note].emplace((uint8_t)note, nData);
			stepDataIter.nodeList[nData.note].emplace(
				std::piecewise_construct,
				std::forward_as_tuple(nData.offset),
				std::forward_as_tuple(nData)
			);
			stepDataIter.nodeCount += 1;
			//if (!stepDataIter.enable)
				//stepDataIter.enable = true;
		}
		else
		{
			stepDataIter.nodeList.at((const unsigned char)note).begin()->second.noteCount++;
		}
		setPatternEditIndication(true);
	}

	void IgnitePattern::pasteSelectedNotes(int stepNum, int noteNum, bool isMoveOp)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Paste selected notes : " + String(noteNum) + " at step : " + String(stepNum) + " Direction: " + String((int)isMoveOp));

		/* Get the delta - first note in selected vector with destination note and similar for steps*/
		auto stepDelta = stepNum - selectedNotesInGridVec[0].step;
		auto noteDelta = noteNum - selectedNotesInGridVec[0].note;

		if (!pasteCompleted)
		{
			for (auto& note : selectedNotesInGridVec)
				copyNote(note.step, note.note);
			pasteCompleted = true;
		}

		for (auto i = 0; i < 16; i++)
		{
			for (auto j = 0; j < 128; j++)
			{
				if (getSelectedNote({ i, j }))
				{
					auto dest_step = i + stepDelta;
					auto dest_note = j + noteDelta;

					if ((dest_step >= 0) && (dest_step < 16) && (dest_note >= 0) && (dest_note < 128))
					{
						if (nodeListData.empty()) break;

						for (auto& node : nodeListData)
						{
							if ((node.step_id == i) && (node.note == j))
							{
								if (isMoveOp)
									node.nodeId = node.note = (uint8_t)dest_note;
								pasteSingleNote(dest_step, dest_note, node);
							}
						}
					}
				}
			}
		}

		if (!noteCutPasteActive)
			_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
	}

	bool IgnitePattern::getSelectedNote(mt::Ignite::selectedNode node)
	{
		return (selectedNotesInGrid.find(node) != selectedNotesInGrid.end());
	}

	void IgnitePattern::setSelectedNoteForAllSteps(int note)
	{
		for (auto i = 0; i < 16; i++)
			if (isNotePresent(i, note))
				if (selectedNotesInGrid.insert({i, note}).second)
					selectedNotesInGridVec.push_back({ i, note });
	}

	void IgnitePattern::setNotesForSelectedStep(int step, bool state)
	{
		if (state)
		{
			for (auto i = 0; i < 128; i++)
				if (isNotePresent(step, i))
					if (selectedNotesInGrid.insert({ step, i }).second)
						selectedNotesInGridVec.push_back({ step, i });
		}
		//else
		//{
		//	for (auto i = selectedNotesInGrid.begin(); i != selectedNotesInGrid.end(); ++i)
		//	{
		//		if (i->step == step)
		//			selectedNotesInGrid.erase(i);
		//	}
		//}
	}

	void IgnitePattern::setSelectedNote(int step, int note)
	{
		mt::Ignite::selectedNode node{ step, note  };
		if (getSelectedNote({ step, note }))
		{
			selectedNotesInGrid.erase(node);
			auto it = std::remove(selectedNotesInGridVec.begin(), selectedNotesInGridVec.end(), node);
			if (it != selectedNotesInGridVec.end())
				selectedNotesInGridVec.erase(it, selectedNotesInGridVec.end());
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Remove in selected list note : " + String(note) + " at step : " + String(step));
		}
		else
		{
			if (selectedNotesInGrid.insert({ step, note }).second)
				selectedNotesInGridVec	.push_back({ step, note });
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Add selected list note : " + String(note) + " at step : " + String(step));
		}
	}

	void IgnitePattern::IgnitePattern::resetNoteSelection()
	{
		selectedNotesInGridVec.clear();
		selectedNotesInGrid.clear();
	}

	void IgnitePattern::setSelectedNotes(int start_step, int end_step, int start_note, int end_note)
	{
		start_note = std::max(0, start_note);
		end_note = std::min(127, end_note);
		for (auto i = start_step; i <= end_step; i++)
			for (auto j = start_note; j <= end_note; j++)
				if (isNotePresent(i, j))
					setSelectedNote(i, j);
	}
	void IgnitePattern::resetGangStepState()
	{
		for (auto i = 0; i < 256; i++)
			selectedStepInGang[i] = false;
	}

	void IgnitePattern::resetGangNoteState()
	{
		for (auto i = 0; i < 128; i++)
			selectedNotes[i] = false;
	}

	void IgnitePattern::setGangMode(bool state)
	{
		if (isGangModeEnabled != state)
		{
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Gang state : " + String((int)state));

			isGangModeEnabled = state;
			//_pComponentStep->setGangMode(state);
			_pComponentGrid->setGangMode(state);
			resetGangStepState();
		}
	}

	void IgnitePattern::parameterViewDrag(int selected_step, int velocity, mt::Ignite::nodeDataIndex id)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		int currentStepIndex = gPatterns[selected].stepIndex;
		auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)selected_step);
		setPatternData(mt::Ignite::ptDataIndex::STEP_ID, (uint8_t)selected_step, true);
		for (int note = 0; note < 128; note++) {
			if (stepDataIter.nodeList.find((uint8_t)note) != stepDataIter.nodeList.end())
				setNodeData(id, -1, -1, note, (uint8_t)velocity, false);
		}
		setPatternData(mt::Ignite::ptDataIndex::STEP_ID, (uint8_t)currentStepIndex, true);
	}

	void IgnitePattern::clearCutCopy()
	{
		if (!nodeListData.empty())
			nodeListData.clear();
		pasteCompleted = false;
	}

	ClientPatternNodeData IgnitePattern::getSingleNote()
	{
		if (!nodeListData.empty())
			return nodeListData.front();

		return {};
	}

	void IgnitePattern::addBar(mt::Ignite::patternData* data, int barid)
	{
		ClienPatterntBarData bData = { (uint8_t)barid, 16 };
		addStep(&bData, 16);
		data->enginePatternData.BarDataList.emplace((uint8_t)barid, bData);
	}

	void IgnitePattern::addStep(ClienPatterntBarData* data, int numSteps)
	{
		ClientPatternStepData sData = { 0, 1, 1, {}, true, false, false, false };

		for (uint16_t i = 0; i < 16; i++)
		{
			sData.stepId = i;
			data->StepDataList.emplace(i, sData);
		}
	}

	void IgnitePattern::addNote(mt::Ignite::patternData* data, int numNotes)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Add Notes : " + String(numNotes));

		defaultNote = IgnitePatternConfig::getAppPatternConfigInstance()->getDefaultNote();

		Ignite::ClientPatternNodeData nData = { (uint16_t)defaultNote, (uint8_t)defaultNote, 38, 0, 15, 0, false };
		auto& gPatternRef = gPatterns.getReference(selected);

		for (auto i = 0; i < numNotes; i++)
		{
			data->enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)).nodeList[nData.note].insert({nData.note, nData});
		}
		data->enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)).nodeCount = (uint8_t)numNotes;
	}

	mt::Ignite::patternData* IgnitePattern::getPatternData(uint8_t index)
	{
		auto& gPatternRef = gPatterns.getReference(index);
		return &gPatternRef;
	}

	void IgnitePattern::setPatternIndex(int index)
	{
		if (index == selected)
			return;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Set new index : " + String(index));

		if (patternCreated && ((index) <= curretPatternRow) && (index >= 0))
		{
			selected = index;
			requestDataFromEngine(index);

			auto& gPatternRef = gPatterns.getReference(selected);
			gPatternRef.barIndex = 0;
			gPatternRef.stepIndex = 0;

			patternName.setText((String)gPatterns[selected].enginePatternData.patternDispName, dontSendNotification);
			//changePatternViewVisibility((PatternView)getTabIndex());
			//refreshStepUI(&gPatterns[index].enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
			refreshStatusbar();
		}
	}

	void IgnitePattern::setEditTrackPattern(String pt_name)
	{
		auto index = 0;
		if (patternCreated)
		{
			if (gPatterns.getReference(selected).enginePatternData.patternName == pt_name)
				return;

			for (auto& i : gPatterns)
			{
				if (i.enginePatternData.patternName == pt_name)
				{
					setSelected(index);
					PatternStatusBar::GetPatternBarInstance()->onNumbofBarchangePattern(gPatterns[index].enginePatternData.numBars - 1);
					return;
				}
				else
					index++;
			}
		}
	}

	void IgnitePattern::getNextNodeAndUpdate(bool direction)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		int index = getCurrentNode();
		int iter = 0;

		if (direction)
		{
			index++;
			if (index > gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)).nodeCount)
				return;
		}
		else
		{
			index--;
			if (index < 0)
				return;
		}

		for (auto& node : gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)).nodeList)
		{
			if ((index - 1) == iter)
				gPatternRef.nodeIndex = node.second.begin()->second.note;
			iter++;
		}

		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
	}

	void IgnitePattern::getNextStepAndUpdate(bool direction)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto stepStart = getActiveStepIndex();
		auto stepEnd = (stepStart + 16) - 1;
		if (direction)
		{
			if ((stepStart + gPatternRef.stepIndex) >= stepEnd)
				return;
			gPatternRef.stepIndex++;
		}
		else
		{
			if ((stepStart + gPatternRef.stepIndex) <= stepStart)
				return;
			gPatternRef.stepIndex--;
		}
		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pStepComp->getPrevStepIndex(), fpLEDPattern::EN_LED_PATT_OFF);
		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_TRACKROTARY1 + gPatternRef.stepIndex), fpLEDPattern::EN_LED_PATT_ON);
		_pStepComp->setPrevStepIndex((paramId_t)FrontPannelParam::paramKey_TRACKROTARY1 + gPatternRef.stepIndex);
	}

	int IgnitePattern::getCurrentBarId()
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		return gPatternRef.barIndex;
	}

	void IgnitePattern::updateLastStep(int stepId)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "last step id : " + String(stepId));

		if ((int)(gPatterns[selected].enginePatternData.patternLastStepId) != stepId)
		{
			setPatternData(mt::Ignite::ptDataIndex::LASTSTEP, (int16_t)stepId, false);
			if (!IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled())
			{
				patternDataUpdated = true;
				updatePatternEditIndication();
				enableControlButton(IgniteControlOptions::SAVE, true);
				enableControlButton(IgniteControlOptions::UNDO, true);
			}
			list[selected]->setLastSelectedStep(stepId);
			_pComponentStep->stepControl.setSelectedLastStep(stepId);
			_pComponentStep->refreshBarData();
		}
	}

	void IgnitePattern::setCurrentStepLedState()
	{
		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pStepComp->getPrevStepIndex(), fpLEDPattern::EN_LED_PATT_OFF);
		auto currentStepId = getStepId(_pStepComp->getEditStep());
		currentStepId = currentStepId + (paramId_t)FrontPannelParam::paramKey_TRACKROTARY1;
		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)currentStepId, fpLEDPattern::EN_LED_PATT_ON);
		_pStepComp->setPrevStepIndex(currentStepId);
	}

	void IgnitePattern::setFpModeLedState()
	{
		setCurrentStepLedState();
		if (insertKeyStatus == true)
		{
			if (getTabIndex() == 2)
				_pComponentStep->setFpStepNodeLedState();
			else if (getTabIndex() == 3)
				_pComponentGrid->showNoteRGB();
		}
		else if (gangKeyStatus == true)
		{
			if (getTabIndex() == 2)
				_pComponentStep->setFpStepGangLedState();
			else if (getTabIndex() == 3)
				_pComponentGrid->showNoteRGB();
		}
		else if (tieKeyStatus == true)
		{
			setFpStepTieLedState();
			setFpStepGateLedState();
		}
		else if (transposeKeyStatus == true)
		{
			setFpStepTransposeLedState();
			setFpStepGateLedState();
		}
		else if (skipKeyStatus == true)
		{
			setFpStepSkipLedState();
			setFpStepGateLedState();
		}
		else if (lastKeyStatus == true)
		{
			setFpPatternLastStepLedState();
			setFpStepGateLedState();
		}
		else
		{
			setFpStepLedState();
			AppMainmode::getMainModeInstance()->resetTrackSelLeds();
		}
	}

	void IgnitePattern::updateBarData()
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		resetGangStepState();
		resetNoteSelection();
		gPatternRef.stepIndex = 0; // (gPatternRef.barIndex * 16);
		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
		_pComponentGrid->refreshScreen();
		setFpModeLedState();
		_pComponentStep->refreshBarData();
	}

	void IgnitePattern::updateSelectedBarData()
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		resetGangStepState();
		resetNoteSelection();
		gPatternRef.stepIndex = 0;
		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
		_pComponentGrid->refreshScreen();
		setCurrentStepLedState();

		if (gangKeyStatus == true)
		{
			if (getTabIndex() == 2)
				_pComponentStep->setFpStepGangLedState();
			else if (getTabIndex() == 3)
				_pComponentGrid->showNoteRGB();
		}
		else if (tieKeyStatus == true)
		{
			setFpStepTieLedState();
			setFpStepGateLedState();
		}
		else if (transposeKeyStatus == true)
		{
			setFpStepTransposeLedState();
			setFpStepGateLedState();
		}
		else if (skipKeyStatus == true)
		{
			setFpStepSkipLedState();
			setFpStepGateLedState();
		}
		else if (lastKeyStatus == true)
		{
			setFpPatternLastStepLedState();
			setFpStepGateLedState();
		}
		else
		{
			setFpStepLedState();
			AppMainmode::getMainModeInstance()->resetTrackSelLeds();
		}
	}

	void IgnitePattern::getSelectedBarAndUpdate(int barNumber)
	{
		if (!list.isEmpty() && getTabIndex() != 0)
		{
			auto& gPatternRef = gPatterns.getReference(selected);

			if ((gPatternRef.enginePatternData.numBars > 0) && (gPatternRef.enginePatternData.numBars <= 16)
				&& (barNumber >= 0) && (barNumber < gPatternRef.enginePatternData.numBars))
			{
				gPatternRef.barIndex = barNumber;
				updateSelectedBarData();
				if (getTabIndex() == 2)
				{
					_pComponentStep->refreshBarUI();
					_pComponentStep->repaint();
				}
			}
		}
	}

	void IgnitePattern::getNextBarAndUpdate(bool direction)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		if ((gPatternRef.enginePatternData.numBars > 0) && (gPatternRef.enginePatternData.numBars <= 16))
		{
			if (direction)
			{
				gPatternRef.barIndex++;
				if (gPatternRef.barIndex >= gPatternRef.enginePatternData.numBars)
					gPatternRef.barIndex = 0;
			}
			else
			{
				gPatternRef.barIndex--;
				if ((gPatternRef.barIndex < 0))
					gPatternRef.barIndex = gPatternRef.enginePatternData.numBars - 1;
			}
			_pStepComp->setEditBar(gPatternRef.barIndex);
			updateBarData();
		}
	}

	void IgnitePattern::refreshStepUI(ClientPatternStepData* data)
	{
		if (nullptr != data && (stepView.getToggleState()))
		{
			//_pStepComp->updateStepData(data);
			_pStepComp->setVisible(true);
		}
	}

	void IgnitePattern::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		if ((buttonThatWasClicked == &addNewP) || (buttonThatWasClicked == &addNewInList))
		{
			mt::Ignite::patternData pdata;
			auto tmpSelected = selected;

			setDefaultPdata(&pdata, mt::Ignite::ptype::MONO);
			if (IgnitePatternConfig::getAppPatternConfigInstance()->getPatternInsertPosition() == mt::Ignite::patternInsertPosition[0])
			{
				addPattern(&pdata, true);
			}
			else if (IgnitePatternConfig::getAppPatternConfigInstance()->getPatternInsertPosition() == mt::Ignite::patternInsertPosition[1])
			{
				addPattern(&pdata, true);
				setSelected(list.size() - 1);
				movePattern(tmpSelected + 1);
				setSelected(tmpSelected);
			}
			addNewInList.setToggleState(false, dontSendNotification);
			view.setVisible(true);
			changePatternViewVisibility(PatternView::LIST);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::PATTERN);
		}
		else if (buttonThatWasClicked == &barView)
		{
			view.setVisible(false);
			changePatternViewVisibility(PatternView::BAR);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::PATTERN);
		}
		else if (buttonThatWasClicked == &stepView)
		{
			view.setVisible(false);
			changePatternViewVisibility(PatternView::STEP);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::PATTERN);
		}
		else if (buttonThatWasClicked == &gridView)
		{
			view.setVisible(false);
			changePatternViewVisibility(PatternView::GRID);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::PATTERN);
		}
		else if (buttonThatWasClicked == &listView)
		{
			view.setVisible(true);
			changePatternViewVisibility(PatternView::LIST);
			AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::PATTERN);
		}
	}

	void IgnitePattern::setPatternBarViewModes()
	{
		AppMainmode::getMainModeInstance()->resetPatternStepModes();

		if (tieKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_NEXT_TIE, fpLEDPattern::EN_LED_PATT_ON);
			setFpStepTieLedState();
			setFpStepGateLedState();
		}
		else if (transposeKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_FILL_X, fpLEDPattern::EN_LED_PATT_ON);
			setFpStepTransposeLedState();
			setFpStepGateLedState();
		}
		else if (skipKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, fpLEDPattern::EN_LED_PATT_ON);
			setFpStepSkipLedState();
			setFpStepGateLedState();
		}
		else if (insertKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_INSERT, fpLEDPattern::EN_LED_PATT_ON);
			_pComponentStep->setFpStepNodeLedState();
			setFpStepGateLedState();
		}
		else if (gangKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_GANG, fpLEDPattern::EN_LED_PATT_ON);
			_pComponentStep->setFpStepGangLedState();
			setFpStepGateLedState();
		}
		else if (lastKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_LAST, fpLEDPattern::EN_LED_PATT_ON);
			setFpPatternLastStepLedState();
			setFpStepGateLedState();
		}
		else
		{
			setFpStepLedState();
		}
	}

	void IgnitePattern::setPatternStepModes()
	{
		AppMainmode::getMainModeInstance()->resetPatternStepModes();

		if (tieKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_NEXT_TIE, fpLEDPattern::EN_LED_PATT_ON);
			setFpStepTieLedState();
			setFpStepGateLedState();
		}
		else if (transposeKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_FILL_X, fpLEDPattern::EN_LED_PATT_ON);
			setFpStepTransposeLedState();
			setFpStepGateLedState();
		}
		else if (skipKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, fpLEDPattern::EN_LED_PATT_ON);
			setFpStepSkipLedState();
			setFpStepGateLedState();
		}
		else if (lastKeyStatus == true)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_LAST, fpLEDPattern::EN_LED_PATT_ON);
			setFpPatternLastStepLedState();
			setFpStepGateLedState();
		}
		else
		{
			if (currentView== PatternView::GRID && patternCreated)
			{
				if (!insertKeyStatus)
					setFpStepLedState();
				else 
					_pComponentGrid->showNoteRGB();
			}
			else
				setFpStepLedState();
		}
	}

	void IgnitePattern::resetInsertStatus()
	{
		setInsertKeyStatus(false);
		_pComponentStep->setPencilIconState(false);
	}

	void IgnitePattern::resetGangStatus()
	{
		setGangKeyStatus(false);
		_pComponentStep->setGangButtonState();
	}

	void IgnitePattern::changePatternViewVisibility(PatternView viewToShow, bool isPatternCreatedInTrack)
	{
		ctlPatternListHeader(false);

		if (addNewP.isVisible())
			return;

		currentView = viewToShow;
		addNewP.setVisible(!patternCreated);
		addNewInList.setVisible(false);
		barView.setToggleState(false, dontSendNotification);
		listView.setToggleState(false, dontSendNotification);
		stepView.setToggleState(false, dontSendNotification);
		gridView.setToggleState(false, dontSendNotification);
		container.setVisible(false);
		_pComponentStep->setVisible(false);
		_pComponentGrid->setVisible(false);
		_pStepComp->setVisible(false);
		view.setVisible(false);
		container.setVisible(false);
		setGangMode(false);
		patternName.setVisible(patternCreated);

		if (!patternCreated)
			return;

		if (patternDataUpdated)
			patternName.setText(((String)gPatterns[selected].enginePatternData.patternDispName + "*"), dontSendNotification);
		else
			patternName.setText((String)gPatterns[selected].enginePatternData.patternDispName, dontSendNotification);

		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pStepComp->getPrevStepIndex(), fpLEDPattern::EN_LED_PATT_OFF);

		if (viewToShow == PatternView::BAR)
		{
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "New Pattern View : BAR View");
			barView.setToggleState(true, dontSendNotification);
			if (patternCreated)
				_pComponentStep->setVisible(true);
			_pComponentStep->refreshBarUI();
			enableViewBox(PatternView::BAR);
			if (gPatterns.size() == 0)
			{
				enableControlButton(IgniteControlOptions::DEL, false);
				enableControlButton(IgniteControlOptions::COPY, false);
				enableControlButton(IgniteControlOptions::MOVE, false);
				enableControlButton(IgniteControlOptions::LOOP, false);

			}
			if ((gPatterns[selected].enginePatternData.numBars - 1) > 0)
				enableControlButton(IgniteControlOptions::LOOP, true);
			_pComponentStep->pannelKeyHandler((paramId_t)FrontPannelParam::paramKey_MouseClick, 0);
			setPatternBarViewModes();
			AppMainmode::getMainModeInstance()->resetRotaryLedsOnViewChange();
		}
		else if (viewToShow == PatternView::STEP)
		{
			optionHandlerEnabled = PATTERN_DETAIL;
			if (patternCreated)
			{
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "New Pattern View : Step View");
				barView.setVisible(true);
				stepView.setVisible(true);
				gridView.setVisible(true);
				addNewInList.setVisible(false);
				_pStepComp->setVisible(true);
				stepView.setToggleState(true, dontSendNotification);
				//refreshPatternUI();
				_pStepComp->updateStepData();
				enableViewBox(PatternView::STEP);
				_pStepComp->pannelKeyHandler((paramId_t)FrontPannelParam::paramKey_MouseClick, 0);
				//enableControlButton(IgniteControlOptions::DEL, false);
			}
			else
			{
				barView.setVisible(false);
				stepView.setVisible(false);
				gridView.setVisible(false);
				_pEmptyPatternComp->setVisible(true);
				addNewP.setVisible(true);
				_pAppOptPtr->showOptions(false);
			}

			setCurrentStepLedState();
			setPatternStepModes();
			resetGangStatus();
			setFpRotaryLedState();
		}
		else if (viewToShow == PatternView::LIST)
		{
			optionHandlerEnabled = PATTERN_LIST;
			if (!patternCreated)
				return;

			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "New Pattern View : List View");

			ctlPatternListHeader(true);
			patternName.setVisible(false);
			listView.setToggleState(true, dontSendNotification);
			view.setVisible(true);
			container.setVisible(true);
			if (!addNewP.isVisible())
				addNewInList.setVisible(true);

			_pComponentGrid->pannelKeyHandler((paramId_t)FrontPannelParam::paramKey_MouseClick, 0);
			enableViewBox(PatternView::LIST);
			enableControlButton(IgniteControlOptions::DEL, (list.size() > 0 ? true : false));
			enableControlButton(IgniteControlOptions::COPY, (list.size() > 0 ? true : false));
			enableControlButton(IgniteControlOptions::MOVE, (list.size() > 0 ? true : false));
			enableControlButton(IgniteControlOptions::LOOP, (list.size() > 0 ? true : false));
			if (isPatternCreatedInTrack == false && isPatternCreated() && isInitCompleted())
			{
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pStepComp->getPrevStepIndex(), fpLEDPattern::EN_LED_PATT_OFF);
				AppMainmode::getMainModeInstance()->resetPatternStepModes();
				setFpPatternLedState();
				resetGangStatus();
				for (int rot = 0; rot < LISTVIEW_MAPPED_COMP; rot++)
				{
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_TRACKROTARY4 + rot), fpLEDPattern::EN_LED_PATT_Blink5_Off);
				}
			}
		}
		else if (viewToShow == PatternView::GRID)
		{
			if (!patternCreated)
				return;

			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "New Pattern View : Grid View");

			gridView.setToggleState(true, dontSendNotification);
			if (patternCreated)
			{
				_pComponentGrid->setVisible(true);
				_pComponentGrid->refreshScreen();
				_pComponentGrid->pannelKeyHandler((paramId_t)FrontPannelParam::paramKey_MouseClick, 0);
			}
			enableViewBox(PatternView::GRID);
			enableControlButton(IgniteControlOptions::DEL, false);
			enableControlButton(IgniteControlOptions::COPY, false);
			enableControlButton(IgniteControlOptions::MOVE, false);
			enableControlButton(IgniteControlOptions::LOOP, false);
			setPatternStepModes();
			resetGangStatus();
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam((int)(FrontPannelParam::paramKey_ROTARYROW)), fpLEDPattern::EN_LED_PATT_ON);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam((int)(FrontPannelParam::paramKey_ROTARYBAR)), fpLEDPattern::EN_LED_PATT_ON);
			AppMainmode::getMainModeInstance()->resetRotaryLedsOnViewChange();
		}
	}

	void IgnitePattern::updatePatternData(uint8_t trackBarId, uint8_t trackPlayStepId)
	{
		if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::RECORD_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
		{
			int barNumber = trackBarId + 1;
			int stepNumber = (trackPlayStepId % 16) + 1;
			auto& gPatternRef = gPatterns.getReference(editPattern);
			auto trackComp = IgniteTrack::GetAppTrackInstance();

			trackComp->setActiveBarID(barNumber, stepNumber);
			if (barNumber != previousBarNumber)
			{
				_pPatternStatusBarPtr->setActivePatternBar(trackBarId);
				if (barNumber != (int)gPatternRef.enginePatternData.numBars)
					resetNoteSelection();

				if (IgnitePatternConfig::getAppPatternConfigInstance()->getBarFollowState() && (editPattern == selected))
					setPatternData(mt::Ignite::ptDataIndex::BAR_ID, trackBarId, true);
				previousBarNumber = barNumber;
			}
			else
				trackComp->updateSelectionInfo();

			if (editPattern == selected)
			{
				if (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE || opMode == mt::Ignite::operatioMode::RECORD_MODE)
					gPatternRef.enginePatternData = _PUiVirtualEngine->EngineGetPatternData(gPatternRef.enginePatternData.patternName);

				gPatternRef.stepIndex = stepNumber - 1;

				setCurrentStepPlaying((uint16_t)gPatternRef.stepIndex);
				if (_pPlayHead == nullptr)
				{
					_pPlayHead = StepPlayhead::getPlayHeadInstance();
					addChildComponent(_pPlayHead);
					_pPlayHead->setBounds(getLocalBounds());
					_pPlayHead->setVisible(true);
				}

				if (onPlayHeadChanged)
				{
					onPlayHeadChanged(stepNumber);
					setFpPlayHeadLedState(stepNumber);
				}
			}
		}
	}

	void IgnitePattern::updatePatternData(uint8_t param, const std::string& value)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Pattern Name : " + String(value));

		switch (param)
		{
		case (uint8_t)PatternParam::paramPatternName:
			gPatternRef.enginePatternData.patternName = value;
			break;
		}
	}

	void IgnitePattern::setPatternData(mt::Ignite::ptDataIndex item_id, uint8_t data, bool is_internal)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		int newNumOfBars = 0;
		auto trackPatternIndex = IgniteTrack::GetAppTrackInstance()->_pChild->getPatternIndex(IgniteTrack::GetAppTrackInstance()->getTrackData(mt::Ignite::trackDataIndex::TRACK_PATTERN));

		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);
		switch (item_id)
		{
			//Direction set to bar level property
			//So may be we can remove direction from top level pattern list page
			//Or use it as common direction setting for all bars (<-- Current Implementation).
		case mt::Ignite::ptDataIndex::DIR_INDEX:
			for (auto& bar : gPatternRef.enginePatternData.BarDataList)
			{
				setBarData(mt::Ignite::barDataIndex::DIR_INDEX, bar.first, data, is_internal);
			}
			break;
		case mt::Ignite::ptDataIndex::TIMEBASE:
			gPatternRef.enginePatternData.timeBase = data;
			list[selected]->refreshPatternRowUI();
			list[selected]->updateLengthandOffset();
			statusBarData.timeBase = data;
			_pComponentStep->refreshBarUI();
			break;
		case mt::Ignite::ptDataIndex::BARNUM:
			newNumOfBars = (data + 1) - gPatternRef.enginePatternData.numBars;
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " Value: " + String(data) + " number of bars : " + String(newNumOfBars));

			if (newNumOfBars > 0)
				_PUiVirtualEngine->EngineAddPatternBar(gPatternRef.enginePatternData.patternName, false, 0, (uint16_t)(newNumOfBars));
			else
				_PUiVirtualEngine->EngineRemovePatternBar(gPatternRef.enginePatternData.patternName, 0, (uint16_t)(std::abs(newNumOfBars)));

			requestDataFromEngine(selected);

			if (gPatternRef.barIndex > (gPatternRef.enginePatternData.numBars - 1))
			{
				gPatternRef.barIndex = data;
				gPatternRef.stepIndex = 0;
			}
			list[selected]->updateLastStep(false, true);
			statusBarData.numberOfBars = gPatternRef.enginePatternData.numBars;
			statusBarData.numberOfSteps = gPatternRef.enginePatternData.patternLastStepId;
			break;
		case mt::Ignite::ptDataIndex::BAR_ID:
			gPatternRef.barIndex = data;
			gPatternRef.stepIndex = 0;
			_pComponentGrid->refreshScreen();
			if (_pComponentStep->isVisible())
			{
				_pComponentStep->refreshBarData();
				_pComponentStep->repaint();
			}
			break;
		case mt::Ignite::ptDataIndex::STEP_ID:
			gPatternRef.stepIndex = data;
			break;
		case mt::Ignite::ptDataIndex::NODE_ID:
			gPatternRef.nodeIndex = data;
			break;
		case mt::Ignite::ptDataIndex::BAR_LOOP_ON:
			gPatternRef.enginePatternData.barLoopOn = data;
			break;
		case mt::Ignite::ptDataIndex::BAR_LOOP_START:
			gPatternRef.enginePatternData.barLoopStartId = data;
			break;
		case mt::Ignite::ptDataIndex::BAR_LOOP_END:
			gPatternRef.enginePatternData.barLoopEndId = data;
			break;
		default: break;
		}
		if ((mt::Ignite::ptDataIndex::DIR_INDEX == item_id) || (mt::Ignite::ptDataIndex::TIMEBASE == item_id) || (mt::Ignite::ptDataIndex::BAR_LOOP_ON == item_id) ||
			(mt::Ignite::ptDataIndex::BAR_LOOP_START == item_id) || (mt::Ignite::ptDataIndex::BAR_LOOP_END == item_id))
			_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData,false, PatternDataUpdateReasonType::PatternDataUpdateReason_barLoopOn);

		if ((IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled()) && (!is_internal))
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);

		/*
		* Pattern updates by user are available with UI first.
		* So if user deletes a bar while playing, in certain circumstances engine is still playing the deleted bar,
		* as the delete information has not yet reached the engine pattern.
		* So Engine will publish bar id or step id that are non existent in UI's updated pattern.
		* So check for that.
		*/
		if ((gPatternRef.barIndex < gPatternRef.enginePatternData.BarDataList.size()) &&
			(getCurrentBarStepId(gPatternRef.stepIndex) <= gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.size()))
		{
			refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
		}

		if(selected == (trackPatternIndex - 1))
			IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
	}

	void IgnitePattern::setBarData(mt::Ignite::barDataIndex item_id, int index, uint8_t data, bool is_internal)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " Value: " + String(data) + " Is Internal: " + String((int)is_internal));

		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);

		switch (item_id)
		{
		case mt::Ignite::barDataIndex::NUM_STEPS:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).stepCount = data; //TODO: update to bar index
			break;
		case mt::Ignite::barDataIndex::TRANSPOSE_DEFEAT_STATUS:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).barTransposeDefeat = data;
		case mt::Ignite::barDataIndex::BAR_REPEAT:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).barRepeatCount = data;
			break;
		case mt::Ignite::barDataIndex::GBAR_SYNC:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).gBarSyncOn = data;
			break;
		case mt::Ignite::barDataIndex::DIR_INDEX:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)index).direction = data;
			list[selected]->refreshPatternRowUI();
			break;
		default: break;
		}

		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
		if ((IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled()) && (!is_internal))
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);

		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
	}

	void IgnitePattern::setStepData(mt::Ignite::stepDataIndex item_id, int bar_index, int step_index, uint8_t data, bool is_internal)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " Value: " + String(data) + " Is Internal: " + String((int)is_internal));

		if (step_index < 0)
			step_index = gPatternRef.stepIndex;

		if (bar_index < 0)
			bar_index = gPatternRef.barIndex;

		int stepIndex = getCurrentBarStepId(step_index);


		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);

		switch (item_id)
		{
		case mt::Ignite::stepDataIndex::NODE_COUNT:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).nodeCount = data;
			break;
		case mt::Ignite::stepDataIndex::GATE_STATUS:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).enable = data;
			break;
		case mt::Ignite::stepDataIndex::SKIP_STATUS:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).skipStatus = data;
			break;
		case mt::Ignite::stepDataIndex::TRANSPOSE_DEFEAT_STATUS:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).transposeDefeatStatus = data;
			break;
		case mt::Ignite::stepDataIndex::TIE_STATUS:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).tieStatus = data;
			break;
		default: break;
		}

		//PatternDataUpdateLocation tmp = { static_cast<int8_t>(gPatternRef.barIndex), static_cast<int8_t>(stepIndex), 0, 0 };
		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData);
		if ((IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled()) && (!is_internal))
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);

		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
	}

	void IgnitePattern::setStepEventData(mt::Ignite::stepEventDataIndex item_id, int bar_index, int step_index, int index, uint8_t data, bool is_internal)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		if (bar_index < 0)
			bar_index = gPatternRef.barIndex;
		if (step_index < 0)
			step_index = gPatternRef.stepIndex;

		auto stepIndex = getCurrentBarStepId(step_index);

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " Value: " + String(data) + " Is Internal: " + String((int)is_internal));

		stepIndex = getCurrentBarStepId(stepIndex);

		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);

		switch (item_id)
		{
		case mt::Ignite::stepEventDataIndex::EVENT_NAME:
			/* EVENT TYPE MIDICC=A0, PITCHBEND=B0...  list box index start with 0 */
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].type = 16 * (data + 10);
			break;
		case mt::Ignite::stepEventDataIndex::EVENT_STATUS:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].enabled = data;
			break;
		case mt::Ignite::stepEventDataIndex::EVENT_TYPE:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].data1 = data;
			break;
		case mt::Ignite::stepEventDataIndex::EVENT_VALUE:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].data2 = data;
			break;
		case mt::Ignite::stepEventDataIndex::EVENT_OFFSET:
			gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].offset = data;
			break;
		default: break;
		}

		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData);
		if ((IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled()) && (!is_internal))
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);

		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
	}

	void IgnitePattern::setDataForSelectedNode(mt::Ignite::nodeDataIndex item_id, int stepId, int16_t data, bool is_internal, bool isPlayModeData)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto tmpIndex = gPatternRef.stepIndex;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + "Step id : " + String(stepId) + " Value: " + String(data));

		gPatternRef.stepIndex = stepId;
		setNodeData(item_id, -1, -1, -1, data, false, isPlayModeData);
		gPatternRef.stepIndex = tmpIndex;
	}

	int IgnitePattern::getCurrentNode()
	{
		int index = 1;
		auto& gPatternRef = gPatterns.getReference(selected);
		auto stepIndex = getCurrentBarStepId(gPatternRef.stepIndex);

		for (auto& node : gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList)
		{
			if (node.second.begin()->second.note == gPatternRef.nodeIndex) //(int)getNodeData(mt::Ignite::nodeDataIndex::NOTE_NUM))
				return index;
			index++;
		}
		return 0;
	}

	void IgnitePattern::addStepNoteIfNotPresent(int stepNum)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto& stepDataIter = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)stepNum);

		if (stepDataIter.nodeList.find((uint8_t)gPatternRef.nodeIndex) == stepDataIter.nodeList.end())
		{
			addNote(-1, stepNum, gPatternRef.nodeIndex);
		}
	}

	bool IgnitePattern::isNotePresent(int stepNum, int noteNum)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		stepNum = (stepNum < 0) ? gPatternRef.stepIndex : stepNum;
		noteNum = (noteNum < 0) ? gPatternRef.nodeIndex : noteNum;

		if (gPatternRef.barIndex >= gPatternRef.enginePatternData.BarDataList.size()) {
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " ERR : Out of Range Index : Bar " + String(gPatternRef.barIndex) + ":: Step " + String(stepNum));
			return false;
		}

		const auto& barData = gPatternRef.enginePatternData.BarDataList[static_cast<unsigned char>(gPatternRef.barIndex)];
		
		if (stepNum >= barData.StepDataList.size()) {
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " ERR : Out of Range Index : Bar " + String(gPatternRef.barIndex) + ":: Step " + String(stepNum));
			return false;
		}

		const auto& stepData = barData.StepDataList.at(static_cast<unsigned char>(stepNum));

		return stepData.nodeList.find(static_cast<uint8_t>(noteNum)) != stepData.nodeList.end();
	}

	void IgnitePattern::setActiveNode(uint8_t noteNum)
	{
		gPatterns.getReference(selected).nodeIndex = noteNum;
	}

	void IgnitePattern::setSelectedNotesValue(mt::Ignite::nodeDataIndex item_id, bool isPlayModeData)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto lenThreshold = (gPatternRef.enginePatternData.numBars * 16) - 1;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " isPlayModeData: " + String((int)isPlayModeData));

		if (lenThreshold > maxStepLength)
			lenThreshold = maxStepLength;

		for (auto& node : selectedNotesInGridVec)
		{
			auto value = node.value;
			switch (item_id)
			{
			case mt::Ignite::nodeDataIndex::VELO:
				if (value > 127)	value = 127;
				if (value < 0)		value = 0;
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)node.step).nodeList.at((const unsigned char)node.note).begin()->second.velocity = (uint8_t)value;
				break;
			case mt::Ignite::nodeDataIndex::DELAY:
				if (value - (getDelayRange().max + 1) > getDelayRange().max)	value = ((getDelayRange().max + 1) * 2) - 1;
				if (value - (getDelayRange().max + 1) <= getDelayRange().min)	value = 0;
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)node.step).nodeList.at((const unsigned char)node.note).begin()->second.offset = (int16_t)value - (getDelayRange().max + 1);
				break;
			case mt::Ignite::nodeDataIndex::STEP_LEN_IN_STEP:
				if (value > lenThreshold)	value = lenThreshold;
				if (value <= 0)	value = 0;
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)node.step).nodeList.at((const unsigned char)node.note).begin()->second.lenInSteps = (uint8_t)value;
				break;
			case mt::Ignite::nodeDataIndex::STEP_LEN_IN_TICK:
				//if (value >= 31)	value = 31;
				//if (value <= 0)	value = 0;
				if (value >= getTickRange().max)	value = getTickRange().max;
				if (value <= 0)	value = 0;
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)node.step).nodeList.at((const unsigned char)node.note).begin()->second.lenInTicks = (TickLength_t)value;
				break;
			}
		}
		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, isPlayModeData, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
		clearCutCopy();
		setPatternEditIndication(true);
	}

	void IgnitePattern::setSelectedNotesValue(mt::Ignite::nodeDataIndex item_id, int value, bool isPlayModeData)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto tmp = value;
		auto lenThreshold = (gPatternRef.enginePatternData.numBars * 16) - 1;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " Value: " + String(value));

		if (lenThreshold > maxStepLength)
			lenThreshold = maxStepLength;

		for (auto i = 0; i < 16; i++)
		{
			for (auto j = 0; j < 128; j++)
			{
				if (getSelectedNote({ i, j }) || selectedNotes[j])
				{
					if (gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).enable &&
						(gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.count((uint8_t)j) != 0))
					{
						switch (item_id)
						{
						case mt::Ignite::nodeDataIndex::VELO:
							value = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.velocity + value;
							if (value > 127)	value = 127;
							if (value < 0)		value = 0;
							gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.velocity = (uint8_t)value;
							break;
						case mt::Ignite::nodeDataIndex::DELAY:
							//value = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.offset + value;
							if (value - (getDelayRange().max + 1) > getDelayRange().max)	value = ((getDelayRange().max + 1) * 2) - 1;
							if (value - (getDelayRange().max + 1) <= getDelayRange().min)	value = 0;
							gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.offset = (int16_t)value;
							break;
						case mt::Ignite::nodeDataIndex::STEP_LEN_IN_STEP:
							value = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.lenInSteps + value;
							if (value > lenThreshold)	value = lenThreshold;
							if (value <= 0)	value = 0;
							gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.lenInSteps = (uint8_t)value;
							break;
						case mt::Ignite::nodeDataIndex::STEP_LEN_IN_TICK:
							value = gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.lenInTicks + value;
							if (value >= getTickRange().max)	value = getTickRange().max;
							if (value <= 0)	value = 0;
							gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)j).begin()->second.lenInTicks = (TickLength_t)value;
							break;
						}
					}
				}
				value = tmp;
			}
		}
		clearCutCopy();
		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, isPlayModeData, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
	}

	void IgnitePattern::setPresetValueForSteps(mt::Ignite::nodeDataIndex item_id, int note, std::vector<int> value, bool isApplyToAll, bool isPlayModeData)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto lenThreshold = (gPatternRef.enginePatternData.numBars * 16) - 1;

		if (lenThreshold > maxStepLength)
			lenThreshold = maxStepLength;

		note = gPatternRef.nodeIndex;

		for (auto i = 0; i < 16; i++)
		{
			if (gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).enable &&
				(gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.count((uint8_t)note) != 0))
			{
				switch (item_id)
				{
				case mt::Ignite::nodeDataIndex::VELO:
					if (value[i] >= 127)	value[i] = 127;
					if (value[i] <= 0)		value[i] = 0;
					gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)note).begin()->second.velocity = (uint8_t)value[i];
					break;
				case mt::Ignite::nodeDataIndex::DELAY:
					if (value[i] - (getDelayRange().max+1) >= getDelayRange().max)	value[i] = ((getDelayRange().max+1)*2)-1;
					if (value[i] - (getDelayRange().max + 1) <= getDelayRange().min)	value[i] = 0;
					DBG("set : " + String(value[i] - (getDelayRange().max + 1)));
					gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)note).begin()->second.offset = ((TickOffset_t)value[i]- (getDelayRange().max + 1));// -16;
					break;
				case mt::Ignite::nodeDataIndex::STEP_LEN_IN_STEP:
					if (value[i] > lenThreshold)	value[i] = lenThreshold;
					if (value[i] <= 0)	value[i] = 0;
					gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)note).begin()->second.lenInSteps = (uint8_t)value[i];
					break;
				case mt::Ignite::nodeDataIndex::STEP_LEN_IN_TICK:
					if (value[i] >= getTickRange().max)	value[i] = getTickRange().max;
					if (value[i] <= 0)	value[i] = 0;
					gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList.at((const unsigned char)note).begin()->second.lenInTicks = (TickLength_t)value[i];
					break;
				}
			}
		}
		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, isPlayModeData, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
		if (IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled())
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);
		else
			setPatternEditIndication(true);
	}

	void IgnitePattern::setPresetValueForSteps(mt::Ignite::nodeDataIndex item_id, std::vector<int> value, bool isApplyToAll, bool isPlayModeData)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto barStepStart = getActiveStepIndex();
		auto lenThreshold = (gPatternRef.enginePatternData.numBars * 16) - 1;

		if (lenThreshold > maxStepLength)
			lenThreshold = maxStepLength;

		for (auto i = 0; i < 16; i++)
		{
			if ((selectedStepInGang[i + barStepStart] || isApplyToAll) && gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).enable)
			{
				for (auto& node : gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)i).nodeList)
				{
					switch (item_id)
					{
					case mt::Ignite::nodeDataIndex::VELO:
						if (value[i] >= 127)	value[i] = 127;
						if (value[i] <= 0)		value[i] = 0;
						node.second.begin()->second.velocity = (uint8_t)value[i];
						break;
					case mt::Ignite::nodeDataIndex::DELAY:
					{
						VariableMinMax result = getDelayRange();
						if ((value[i] - (result.max + 1)) > result.max) {
							value[i] = ((result.max + 1) * 2) - 1;
						}
						else if ((value[i] - (result.max + 1)) <= result.min) {
							value[i] = 0;
						}
						node.second.begin()->second.offset = (int16_t)value[i] - (result.max + 1);
					}
						break;
					case mt::Ignite::nodeDataIndex::STEP_LEN_IN_STEP:
						if (value[i] > lenThreshold)	value[i] = lenThreshold;
						if (value[i] <= 0)	value[i] = 0;
						node.second.begin()->second.lenInSteps = (uint8_t)value[i];
						break;
					case mt::Ignite::nodeDataIndex::STEP_LEN_IN_TICK:
						if (value[i] >= getTickRange().max)	value[i] = getTickRange().max;
						if (value[i] <= 0)	value[i] = 0;
						node.second.begin()->second.lenInTicks = (TickLength_t)value[i];
						break;
					}
				}
			}
		}
		_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, isPlayModeData, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);
		if (IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled())
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);
		else
			setPatternEditIndication(true);
	}

	void IgnitePattern::setNodeData(mt::Ignite::nodeDataIndex item_id, int barIndex, int stepIndex, int nodeId, int16_t data, bool is_internal, bool isPlayModeData)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto nodeIndex = 0;
		int8_t lenThreshold = (gPatternRef.enginePatternData.numBars * 16) - 1;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " Value: " + String(data) + " Is Internal: " + String((int)is_internal));

		if (lenThreshold > maxStepLength)
			lenThreshold = maxStepLength;

		if (nodeId >= 0)
			nodeIndex = (uint8_t)nodeId;
		else
			nodeIndex = (uint8_t)gPatternRef.nodeIndex;

		if (stepIndex < 0)
			stepIndex = gPatternRef.stepIndex;

		if (barIndex < 0)
			barIndex = gPatternRef.barIndex;

		stepIndex = getCurrentBarStepId(stepIndex);

		if (gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.count((uint8_t)nodeIndex) == 1)
		{
			switch (item_id)
			{
			case mt::Ignite::nodeDataIndex::VELO:
				(data >= 127) ? data = 127 : data;
				(data <= 0) ? data = 0 : data;
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.velocity = (uint8_t)data;
				break;
			case mt::Ignite::nodeDataIndex::DELAY:
				 ((data -  (getDelayRange().max + 1)) > getDelayRange().max) ? data = ((getDelayRange().max + 1) * 2) - 1 : data;
				 ((data - (getDelayRange().max + 1)) <= getDelayRange().min) ? data = 0 : data;
				 DBG("data1 = " + String(data));
				 DBG("data2 = " + String(data - (getDelayRange().max + 1)));
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.offset = (TickOffset_t)(data - (getDelayRange().max + 1));
				break;
			case mt::Ignite::nodeDataIndex::STEP_LEN_IN_STEP:
				if (data != lengthHoldId)
				{
					(data > lenThreshold) ? data = lenThreshold : data;
					(data <= 0) ? data = 0 : data;
				}
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.lenInSteps = (uint8_t)data;
				break;
			case mt::Ignite::nodeDataIndex::STEP_LEN_IN_TICK:
				(data >= getTickRange().max) ? data = getTickRange().max : data;
				(data <= 0) ? data = 0 : data;
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.lenInTicks = (TickLength_t) data;
				break;
			case mt::Ignite::nodeDataIndex::NOTE_NUM:
				gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.note = (uint8_t)data;
				break;
			default: break;
			}
		}

		if (!is_internal)
			_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData, isPlayModeData, PatternDataUpdateReasonType::PatternDataUpdateReason_NodeData);

		if ((IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled()) && (!is_internal))
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);

		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
	}

	void IgnitePattern::setPatternData(mt::Ignite::ptDataIndex item_id, int16_t data, bool is_internal)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		mt::Ignite::trackData* gTrackRef = scenePtr->getTrackDataRef();

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " Value: " + String(data) + " Is Internal: " + String((int)is_internal));

		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);

		switch (item_id)
		{
		case mt::Ignite::ptDataIndex::LASTSTEP:
			gPatternRef.enginePatternData.patternLastStepId = statusBarData.numberOfSteps = data;
			list[selected]->refreshPatternRowUI();
			break;
		}

		if (!is_internal)
			_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, gPatternRef.enginePatternData.patternName, gPatternRef.enginePatternData);

		if ((IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled()) && (!is_internal))
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);

		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
		if(getPatternData(mt::Ignite::ptDataIndex::NAME) == gTrackRef->pattern)
			IgniteTrack::GetAppTrackInstance()->updateSelectionInfo();
	}

	void IgnitePattern::setPatternData(mt::Ignite::ptDataIndex item_id, String data, bool is_internal)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		std::string name = gPatternRef.enginePatternData.patternName;

		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " Value: " + String(data) + " Is Internal: " + String((int)is_internal));

		if (!is_internal)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);

		switch (item_id)
		{
		case mt::Ignite::ptDataIndex::NAME:
		{
			gPatternRef.enginePatternData.patternDispName = data.toStdString();
			if (!is_internal)
				_PUiVirtualEngine->EngineSetPatternData((moduleId_t)ModuleName::moduleUI, name, gPatternRef.enginePatternData, false, PatternDataUpdateReasonType::PatternDataUpdateReason_patternName);
		}
		break;
		default: break;
		}
		refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));

		if ((IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled()) && (!is_internal))
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);

		list[selected]->refreshPatternRowUI();
	}

	var IgnitePattern::getPatternData(mt::Ignite::ptDataIndex item_id)
	{
		statusBarData.activeBarId = 0;
		switch (item_id)
		{
		case mt::Ignite::ptDataIndex::ID:
			return gPatterns[selected].id;
		case mt::Ignite::ptDataIndex::NAME:
			return (String)gPatterns[selected].enginePatternData.patternDispName;
		case mt::Ignite::ptDataIndex::BARNUM:
			return gPatterns[selected].enginePatternData.numBars - 1;
		case mt::Ignite::ptDataIndex::DIR_INDEX:
		{
			if (gPatterns[selected].enginePatternData.BarDataList.size() > 0)
			{
				int firstBarDirection = (int) getBarData(mt::Ignite::barDataIndex::DIR_INDEX, 0);
				for (auto& bar : gPatterns[selected].enginePatternData.BarDataList)
				{
					int direction = bar.second.direction;
					if (direction == firstBarDirection)
					{
						continue;
					}
					else
					{
						return -1;//if one or more bars have diffrent directions, display "Bar-Level"
					}
				}
				return firstBarDirection;//if all bar's direction are same, return that
			}
		}
		case mt::Ignite::ptDataIndex::TIMEBASE:
			return (uint8_t)(gPatterns[selected].enginePatternData.timeBase);
		case mt::Ignite::ptDataIndex::LASTSTEP:
			return (int16_t)(gPatterns[selected].enginePatternData.patternLastStepId);
		case mt::Ignite::ptDataIndex::BAR_ID:
			return gPatterns[selected].barIndex;
		case mt::Ignite::ptDataIndex::STEP_ID:
			return gPatterns[selected].stepIndex;
		case mt::Ignite::ptDataIndex::NODE_ID:
			return gPatterns[selected].nodeIndex;
		case mt::Ignite::ptDataIndex::BAR_LOOP_ON:
			return (uint8_t)(gPatterns[selected].enginePatternData.barLoopOn);
		case mt::Ignite::ptDataIndex::BAR_LOOP_START:
			return (uint8_t)(gPatterns[selected].enginePatternData.barLoopStartId);
		case mt::Ignite::ptDataIndex::BAR_LOOP_END:
			return (uint8_t)(gPatterns[selected].enginePatternData.barLoopEndId);
		default:
			return -1;
		}
	}

	var IgnitePattern::getBarData(mt::Ignite::barDataIndex item_id, int index)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		int bar_index;
		if (index >= 0)
			bar_index = index;
		else
			bar_index = gPatternRef.barIndex;

		//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " index: " + String(index) + " bar_index: " + String(bar_index));

		switch (item_id)
		{
		case mt::Ignite::barDataIndex::ID:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).BarId;
		case mt::Ignite::barDataIndex::NUM_STEPS:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).stepCount;
		case mt::Ignite::barDataIndex::TRANSPOSE_DEFEAT_STATUS:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).barTransposeDefeat;
		case mt::Ignite::barDataIndex::BAR_REPEAT:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).barRepeatCount;
		case mt::Ignite::barDataIndex::GBAR_SYNC:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).gBarSyncOn;
		case mt::Ignite::barDataIndex::DIR_INDEX:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).direction;
		default:
			return -1;
		}
	}

	ClientPatternStepData* IgnitePattern::getStepData(int barIndex, int stepIndex)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		return &gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(stepIndex));
	}

	var IgnitePattern::getStepData(mt::Ignite::stepDataIndex item_id, int bar_index, int step_index)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		int stepIndex;

		if (step_index >= 0)
			stepIndex = getCurrentBarStepId(step_index);
		else
			stepIndex = getCurrentBarStepId(gPatternRef.stepIndex);


		if (bar_index < 0)
			bar_index = gPatternRef.barIndex;

		//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " index: " + String(index) + " stepIndex: " + String(stepIndex));

		switch (item_id)
		{
		case mt::Ignite::stepDataIndex::NODE_COUNT:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).nodeCount;
		case mt::Ignite::stepDataIndex::STEP_ID:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).stepId;
		case mt::Ignite::stepDataIndex::BAR_ID:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).BarId;
		case mt::Ignite::stepDataIndex::GATE_STATUS:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).enable;
		case mt::Ignite::stepDataIndex::SKIP_STATUS:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).skipStatus;
		case mt::Ignite::stepDataIndex::TRANSPOSE_DEFEAT_STATUS:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).transposeDefeatStatus;
		case mt::Ignite::stepDataIndex::TIE_STATUS:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).tieStatus;
		default:
			return -1;
		}
	}

	var IgnitePattern::getStepEventData(mt::Ignite::stepEventDataIndex item_id, int bar_index, int step_index, int index)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		if (step_index < 0)
			step_index = gPatternRef.stepIndex;

		if (bar_index < 0)
			bar_index = gPatternRef.barIndex;

		int stepIndex = getCurrentBarStepId(step_index);

		switch (item_id)
		{
		case mt::Ignite::stepEventDataIndex::EVENT_NAME:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].type;
		case mt::Ignite::stepEventDataIndex::EVENT_STATUS:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].enabled;
		case mt::Ignite::stepEventDataIndex::EVENT_TYPE:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].data1;
		case mt::Ignite::stepEventDataIndex::EVENT_VALUE:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].data2;
		case mt::Ignite::stepEventDataIndex::EVENT_OFFSET:
			return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)bar_index).StepDataList.at((const unsigned char)stepIndex).auxList[index].offset;
		default:
			return -1;
		}
	}

	var IgnitePattern::getDataForSelectedNode(mt::Ignite::nodeDataIndex item_id, int stepId, int nodeId)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		auto tmp1 = gPatternRef.stepIndex;
		auto tmp2 = gPatternRef.nodeIndex;
		if (nodeId >= 0)
			gPatternRef.nodeIndex = nodeId;
		gPatternRef.stepIndex = stepId;
		auto val = getNodeData(item_id, -1, -1, -1);
		gPatternRef.stepIndex = tmp1;
		if (nodeId >= 0)
			gPatternRef.nodeIndex = tmp2;
		return val;
	}

	var IgnitePattern::getNodeData(mt::Ignite::nodeDataIndex item_id, int barIndex, int step_index, int nodeIndex)
	{
		auto& gPatternRef = gPatterns.getReference(selected);

		if (barIndex < 0)
			barIndex = gPatternRef.barIndex;
		if (step_index < 0)
			step_index = gPatternRef.stepIndex;
		if (nodeIndex < 0)
			nodeIndex = gPatternRef.nodeIndex;

		auto stepIndex = getCurrentBarStepId(step_index);


		//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " Item id : " + String((int)item_id) + " stepIndex: " + String(stepIndex));

		if (gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.count((const unsigned char)nodeIndex))
		{
			switch (item_id)
			{
			case mt::Ignite::nodeDataIndex::DELAY:
				return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.offset + (getDelayRange().max + 1);
			case mt::Ignite::nodeDataIndex::NOTE_NUM:
				return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.note;
			case mt::Ignite::nodeDataIndex::STEP_LEN_IN_STEP:
				return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.lenInSteps;
			case mt::Ignite::nodeDataIndex::STEP_LEN_IN_TICK:
				return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.lenInTicks;
			case mt::Ignite::nodeDataIndex::VELO:
				return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)barIndex).StepDataList.at((const unsigned char)stepIndex).nodeList.at((const unsigned char)nodeIndex).begin()->second.velocity;
			}
		}
		return -1;
	}

	void IgnitePattern::updatePatternEditIndication()
	{
		list[getSelected()]->updateEditIndication();
		if (patternDataUpdated && barView.getToggleState())
			_pComponentStep->setUndoButtonState(true);
		else if (patternDataUpdated && gridView.getToggleState())
			_pComponentGrid->setUndoButtonState(true);
		else
		{
			_pComponentStep->setUndoButtonState(false);
			_pComponentGrid->setUndoButtonState(false);
		}

		if (patternDataUpdated)
			patternName.setText(((String)gPatterns[selected].enginePatternData.patternDispName + "*"), dontSendNotification);
		else
			patternName.setText((String)gPatterns[selected].enginePatternData.patternDispName, dontSendNotification);

	}

	void IgnitePattern::enableViewBox(PatternView state)
	{
		patternBox.performLayout(ui::Rectangle(xStartPatternLayout2, yStartPatternLayout1, widthPatternLayout, heightPatternLayout));
		patternLableBox.performLayout(ui::Rectangle(xStartPatternLayout2, yStartPatternLayout2, widthScreen - 20, 48));
	}

	/* TODO: update after support in engine added*/
	void IgnitePattern::updateLables()
	{
		//_pStepComp->updateLableText(mt::Ignite::ptDataIndex::DIR_INDEX, false);
		//_pStepComp->updateLableText(mt::Ignite::ptDataIndex::BARNUM, false);
		//_pStepComp->updateLableText(mt::Ignite::ptDataIndex::TIMEBASE, false);
		//_pStepComp->updateLableText(mt::Ignite::ptDataIndex::LASTSTEP, false);
		//_pStepComp->updateLableText(mt::Ignite::nodeDataIndex::VELO, false);
		//_pStepComp->updateLableText(mt::Ignite::nodeDataIndex::DELAY, false);
		//_pStepComp->updateLableText(mt::Ignite::nodeDataIndex::STEP_LEN_IN_STEP, false);
		//_pStepComp->updateLableText(mt::Ignite::nodeDataIndex::NOTE_NUM, false);
		//_pStepComp->updateLableText(mt::Ignite::nodeDataIndex::TIE_STATUS, false);
		///*	_pStepComp->updateLableText(mt::Ignite::stepDataIndex::TRANSPOSE_DEFEAT_STATUS, false);
		//	_pStepComp->updateLableText(mt::Ignite::stepDataIndex::SKIP_STATUS, false);*/
		//_pComponentStep->updateLableText(mt::Ignite::ptDataIndex::DIR_INDEX, false);
		//_pComponentStep->updateLableText(mt::Ignite::ptDataIndex::TIMEBASE, false);
		//_pComponentStep->updateLableText(mt::Ignite::ptDataIndex::LASTSTEP, false);

		auto patternParams = _PUiVirtualEngine->EnginePatternGetUndoList();

		if (!patternParams.size())
		{
			patternDataUpdated = false;
			updatePatternEditIndication();
			enableControlButton(IgniteControlOptions::UNDO, false);
			enableControlButton(IgniteControlOptions::SAVE, false);
		}
		else
			updatePatternEditIndication();
	}

	void IgnitePattern::paint(juce::Graphics& g)
	{
		if (this->isVisible())
		{
			if (bootupState)
			{
				bootupState = false;
			}

			if (addNewP.isVisible())
			{
				String name = addNewP.getToggleState() ? "btn-add-newpattern-active.png" : addNewP.isMouseOver() ? "btn-add-newpattern-hover.png" : "btn-add-newpattern-default.png";
				Image Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(name));
				g.drawImage(Draw, ui::Rectangle<float>{xStartAddNewBtn, yStartAddNewBtn, widthAddNewBtn, heightAddNewBtn}, RectanglePlacement::centred);
			}
		}
	}

	void IgnitePattern::resized()
	{
		patternName.setBounds(Rectangle(400, 15, 200, 40));
	}

	void IgnitePattern::deleteAllUnassignedPatterns()
	{
		int selected_pattern_index;

		if (list.size() > 0)
		{
			selected_pattern_index = getSelected();
			if (_pPopUpPtr->isDialogOpen.get() != true)
			{
				_pPopUpPtr->close();
				_pPopUpPtr->setPopUpPropories("Delete all Unassigned Patterns", 604, 290);
				_pPopUpPtr->setbtnsize(265, 44);
				_pPopUpPtr->setInfoText("Are you sure you want to delete all unassigned patterns?");
				_pPopUpPtr->addImageButton("Cancel", "btn-secondary-cancel-267x44-default.png", "btn-secondary-cancel-267x44-default-hover.png", 2);
				_pPopUpPtr->addImageButton("delete", "btn-delete-267x44-default.png", "btn-delete-267x44-default-hover.png", 1);
				_pPopUpPtr->popupType = POPUP_WIPEDATA;
				_pPopUpPtr->isCloseButtonVisible(false);
				_pPopUpPtr->show();
				_pPopUpPtr->onSelectionChanged = [this, &selected_pattern_index](int r)
				{
					bool isPatternRemaining = false;
					if (r == 1)
					{
						int index = list.size() - 1;
						for (auto i = index; i >= 0; i--)
						{
							auto& gPatternRef = gPatterns.getReference(i);
							gPatternRef.enginePatternData = _PUiVirtualEngine->EngineGetPatternData(gPatterns[i].enginePatternData.patternName);
							if (!(gPatternRef.enginePatternData.usedByList.size() > 0))
							{
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramRemovePattern, gPatternRef.enginePatternData.patternName);
								container.removeChildComponent(i);
								list.remove(i);
							}
							else
							{
								isPatternRemaining = true;
							}
						}
						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);

						updateDataFromEngine();
						IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
						IgniteTrack::GetAppTrackInstance()->updateTrackDataList();
						if (!isPatternRemaining)
						{
							enableControlButton(IgniteControlOptions::DEL, false);
							patternCreated = statusBarData.isPatternCreated = false;
						}
						changePatternViewVisibility(PatternView::STEP);
						refreshPatternUI();
					}
					else
					{
						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);
					}
				};
			}
		}
	}

	void IgnitePattern::movePattern(juce::Point<float> position)
	{
		int index = static_cast<int>(((getSelected() * (layout::row.getHeight())) + position.getY()) / (layout::row.getHeight()));
		if ((index == getSelected()))
			return;

		uint64_t SourceDestIndex = ((uint64_t)getSelected() << 32) | index;
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramMovePatternCmd, (uint64_t)SourceDestIndex);
		updateDataFromEngine();
		setSelected(index);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " moved to index : " + String(index));
	}

	void IgnitePattern::movePattern(int destIndex)
	{
		if (destIndex == getSelected())
			return;

		uint64_t SourceDestIndex = ((uint64_t)getSelected() << 32) | destIndex;
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramMovePatternCmd, (uint64_t)SourceDestIndex);
		updateDataFromEngine();
		if ((list.size() == destIndex) && (destIndex > 0))
			destIndex--;
		setSelected(destIndex);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " moved to index : " + String(destIndex));
	}

	void IgnitePattern::setFpPatternLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		auto _currentPatternBank = 0;
		auto patternStart = 0;

		if (gPatterns.size() > 0)
		{
			for (int pattern = 0; pattern < gPatterns.size(); pattern++)
			{
				if ((pattern == 0) || (pattern % 16) != 0)
				{
					gPatterns.getReference(pattern).current_pattern_bank = _currentPatternBank;
				}
				else
				{
					_currentPatternBank = _currentPatternBank + 1;
					gPatterns.getReference(pattern).current_pattern_bank = _currentPatternBank;
				}
				if (gPatterns[pattern].current_pattern_bank == _currentPatternBank)
				{
					if (pattern == getSelected())
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + patternStart), fpLEDPattern::EN_RGB_LED_LBLUE);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + patternStart), fpLEDPattern::EN_RGB_LED_LPURPLE);

					patternStart++;

					if (patternStart >= 16)
						break;
				}
			}
		}
	}

	bool IgnitePattern::handleTrackSelection(paramId_t param, uint32_t KeyValue)
	{
		int stepIndex = 0;
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusReleased)
			return true;
		else if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusPressed)
		{
			if ((!list.isEmpty()) && (getTabIndex() == 2) && (FrountPannel::GetFrountPannelInstance()->getBarKeyStatus() == FrontPannelKeyStatus::paramKeyStatusOnHold))
			{
				if (insertKeyStatus == true)
					insertBar(IgniteBarInsert::insertCtlKey::INSERT, (param - (paramId_t)FrontPannelParam::paramKey_TRACKSEL1));
				else if (insertKeyStatus == false)
					getSelectedBarAndUpdate(param - (paramId_t)FrontPannelParam::paramKey_TRACKSEL1);
			}
			else if (!list.isEmpty() && getTabIndex() != 0)
			{
				stepIndex = param - (paramId_t)FrontPannelParam::paramKey_TRACKSEL1;
				_pStepComp->setEditStep(stepIndex);
				_pStepComp->updateStepData();

				if (tieKeyStatus == true)
				{
					auto stepTieStatus = !getStepData(mt::Ignite::stepDataIndex::TIE_STATUS, -1, stepIndex);
					if(stepTieStatus)
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_RGB_LED_WHITE);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_LED_PATT_OFF);

					_pStepComp->tieButton.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, KeyValue);
				}
				else if (transposeKeyStatus == true)
				{
					auto stepTransposeStatus = !getStepData(mt::Ignite::stepDataIndex::TRANSPOSE_DEFEAT_STATUS, -1, stepIndex);
					if(stepTransposeStatus)
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)(int)fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_RGB_LED_LBLUE);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)(int)fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_LED_PATT_OFF);

					_pStepComp->xposeDefeatButton.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, KeyValue);
				}
				else if (skipKeyStatus == true)
				{
					auto stepSkipStatus = !getStepData(mt::Ignite::stepDataIndex::SKIP_STATUS, -1, stepIndex);
					if(stepSkipStatus)
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)(int)fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_RGB_LED_GREEN);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)(int)fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_LED_PATT_OFF);

					_pStepComp->skipButton.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, KeyValue);
				}
				else if (lastKeyStatus == true)
				{
					return handlePatternLastStep(param, KeyValue);
				}
				else
				{
					if (getTabIndex() == 1)
					{
						auto stepGateStatus = !getStepData(mt::Ignite::stepDataIndex::GATE_STATUS, -1, stepIndex);
						if (stepGateStatus)
							_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)(int)fpRGBDispLed[(FrontPannelParam)param], fpPatternParamLedValues[getActiveParamControl()]);
						else
							_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)(int)fpRGBDispLed[(FrontPannelParam)param], fpLEDPattern::EN_LED_PATT_OFF);

						_pStepComp->gateButton.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, KeyValue);

						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)_pStepComp->getPrevStepIndex(), fpLEDPattern::EN_LED_PATT_OFF);
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)param + 16, fpLEDPattern::EN_LED_PATT_ON);
						_pStepComp->setPrevStepIndex((paramId_t)FrontPannelParam::paramKey_TRACKROTARY1 + stepIndex);
						_pStepComp->setEditStep(stepIndex);
					}
					else if (getTabIndex() == 2)
					{
						_pComponentStep->pannelKeyHandler(param, KeyValue);
					}
					else if (getTabIndex() == 3)
					{
						_pComponentGrid->pannelKeyHandler(param, KeyValue);
					}
				}
			}
		}
		return true;
	}

	bool IgnitePattern::handlePatternLastStep(paramId_t param, uint32_t KeyValue)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (isPatternCreated() && isInitCompleted())
		{
			auto barIndex = getCurrentBarId();

			int offset = static_cast<int>(param) - static_cast<int>(FrontPannelParam::paramKey_TRACKSEL1);
			if (offset >= 0 && offset < 16)
			{
				auto stepIndex = (barIndex * 16) + offset;
				updateLastStep(stepIndex);
				setFpPatternLastStepLedState();
				return true;
			}
		}

		return false;
	}

	void IgnitePattern::setFpPatternLastStepLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));

		if (isPatternCreated() && isInitCompleted())
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();

			for (auto step = 0; step <= getLastStepIndex() - (getCurrentBarId() * 16); step++)
			{
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + (step % 16)), fpLEDPattern::EN_RGB_LED_GREEN);
			}
		}
	}

	bool IgnitePattern::pannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		if (!patternCreated && (param != (paramId_t)FrontPannelParam::paramKey_INSERT))
			return true;
		if ((KeyValue == (uint32_t)FrontPannelKeyStatus::paramKeyStatusReleased) && (param != (paramId_t)FrontPannelParam::paramKey_ROTARYBAR) && (param != (paramId_t)FrontPannelParam::paramKey_DELETE))
			return true;

		bool keyHandled = false;
		itemParam = param;
		itemKeyValue = KeyValue;
		_pSongCfgPtr = IgniteSongConfig::getAppSongConfigInstance();

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
				if (getTabIndex() != 0 && !(opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
					keyHandled = handleTrackSelection(param, KeyValue);
			break;
		case FrontPannelParam::paramKey_NEXT_TIE:
			keyHandled = handleTie(param, KeyValue);
			break;
		case FrontPannelParam::paramKey_FILL_X:
			keyHandled = handleTranspose(param, KeyValue);
			break;
		case FrontPannelParam::paramKey_SOLO_SKIP:
			keyHandled = handleSkip(param, KeyValue);
			break;
		case FrontPannelParam::paramKey_ROTARYBAR:
			if (getTabIndex() == 2)
			{
				if (KeyValue == (uint32_t)FrontPannelKeyStatus::paramKeyStatusPressed)
					showPatternBars();
				else if (KeyValue == (uint32_t)FrontPannelKeyStatus::paramKeyStatusReleased)
					getSelectedBarAndUpdate(getCurrentBarId());
				keyHandled = true;
			}
			break;
		case FrontPannelParam::paramKey_LAST:
			keyHandled = handleLastKey(param, KeyValue);
			break;
		default:
			break;
		}

		if (!keyHandled)
		{
			if (_pPatternCopyComp->isVisible())
				return _pPatternCopyComp->PannelKeyHandler(param, KeyValue);

			if (_pPopUpPtr != nullptr && _pPopUpPtr->IsEditing() && _pPopUpPtr->popupType != POPUP_WIPEDATA)
			{
				if (_pWarnPopupPtr != nullptr && _pWarnPopupPtr->IsEditing())
					return _pWarnPopupPtr->FPKeyHandler(true, param, KeyValue);
				else
					return _pPopUpPtr->FPKeyHandler(true, param, KeyValue);
			}

			if ((paramId_t)FrontPannelParam::paramKey_SCULPT == param)
			{
				if ((FrontPannelKeyStatus)KeyValue == FrontPannelKeyStatus::paramKeyStatusPressed)
				{
					if (getTabIndex() != 1)
						postCommandMessage(7);
					return _pComponentStep->pannelKeyHandler(param, KeyValue);
				}
			}

			if (list.isEmpty())
			{
				if (addNewP.isVisible() == true)
				{
					if ((FrontPannelParam)param == FrontPannelParam::paramKey_RSELECT || ((FrontPannelParam)param == FrontPannelParam::paramKey_INSERT))
						this->postCommandMessage(2); //Add new Button
				}
			}
			else
			{
				if (currentView == PatternView::BAR)
					keyHandled = (_pComponentStep->pannelKeyHandler(param, KeyValue));
				else if (currentView == PatternView::GRID)
				{
					keyHandled = (_pComponentGrid->pannelKeyHandler(param, KeyValue));
				}
				else if (currentView == PatternView::STEP)
				{
					keyHandled = _pStepComp->pannelKeyHandler(param, KeyValue);
				}
				else
				{
					if (param == (paramId_t)FrontPannelParam::paramKey_COPY_NAV)
					{
						this->postCommandMessage(5);
						return true;
					}

					if ((optionHandlerEnabled == PATTERN_OPTION) && !addNewP.isVisible() && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
					{
						keyHandled = pannelKeyOptionsHandler(param, KeyValue);
					}
					else if (!list.isEmpty() && currentView == PatternView::LIST)
					{
						keyHandled = true;
						list[patternRowHighLightIndex]->setFocus(false, false);
						addNewInList.setState(Button::ButtonState::buttonNormal);


						if (param == (paramId_t)FrontPannelParam::paramKey_INSERT)
						{
							this->postCommandMessage(2);
							return true;
						}
						if (param == (paramId_t)FrontPannelParam::paramKey_ROTARYBAR)
						{
							if (FrountPannel::GetFrountPannelInstance()->getKeyStatus((paramId_t)FrontPannelParam::paramKey_ROTARYBAR) == FrontPannelKeyStatus::paramKeyStatusOnHold)
							{
								AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
								for (int patternBank = 0; patternBank <= (gPatterns.size() / 16); patternBank++)
								{
									_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + patternBank), fpLEDPattern::EN_RGB_LED_GREEN);
								}
							}
							else if (FrountPannel::GetFrountPannelInstance()->getKeyStatus((paramId_t)FrontPannelParam::paramKey_ROTARYBAR) == FrontPannelKeyStatus::paramKeyStatusReleased)
							{
								setFpPatternLedState();
							}
							return true;
						}
						setFpPatternLedState();
						switch ((FrontPannelParam)param)
						{
						case FrontPannelParam::paramRot_PADROT4:
						case FrontPannelParam::paramRot_PADROT5:
						case FrontPannelParam::paramRot_PADROT6:
						case FrontPannelParam::paramRot_PADROT7:
						case FrontPannelParam::paramRot_PADROT8:
						case FrontPannelParam::paramKey_TRACKROTARY4:
						case FrontPannelParam::paramKey_TRACKROTARY5:
						case FrontPannelParam::paramKey_TRACKROTARY6:
						case FrontPannelParam::paramKey_TRACKROTARY7:
						case FrontPannelParam::paramKey_TRACKROTARY8:
						case FrontPannelParam::paramKey_RETURN_NAV:
							if (selected > -1 && selected < list.size())
								list[selected]->pannelKeyHandler(param, KeyValue);
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
							if (FrountPannel::GetFrountPannelInstance()->getBarKeyStatus() == FrontPannelKeyStatus::paramKeyStatusOnHold)
							{
								auto patternsInBank = 0;
								currentPatternBank = (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
								int index = 0;
								AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
								for (auto pattern : gPatterns)
								{
									if (pattern.current_pattern_bank == currentPatternBank)
									{
										if (index == selected)
											_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + patternsInBank), fpLEDPattern::EN_RGB_LED_LBLUE);
										else
											_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + patternsInBank), fpLEDPattern::EN_RGB_LED_LPURPLE);
										patternsInBank++;
									}
									if (patternsInBank >= 16) break;
									index++;
								}
							}
							else if (FrountPannel::GetFrountPannelInstance()->getDeleteKeyStatus() == FrontPannelKeyStatus::paramKeyStatusOnHold)
							{
								auto selectedPattern = (currentPatternBank * 16) + (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
								if (gPatterns.size() >= selectedPattern)
								{
									selected = selectedPattern;
									setSelected(selected);
									_pPatternOptCtl->PannelKeyHandler((paramId_t)FrontPannelParam::paramKey_DELETE, KeyValue);
								}
							}
							else
							{
								auto selectedPattern = (currentPatternBank * 16) + (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
								if (gPatterns.size() >= selectedPattern)
								{
									selected = (currentPatternBank * 16) + (param - (uint8_t)FrontPannelParam::paramKey_TRACKSEL1);
									setSelected(selected);
								}
							}
							view.setViewPosition(view.getViewPositionX(), selected>6? ROW_HEIGHT*(selected-2):0);
							break;
						}

						switch ((FrontPannelParam)param)
						{
						case FrontPannelParam::paramKey_RMOVELEFT:
							if (list[getSelected()]->isEditing())
								this->postCommandMessage(4);
							else
							{
								if (patternRowHighLightIndex + 1 >= list.size() && addNewInList.isVisible())
								{
									addNewInList.setState(Button::ButtonState::buttonOver);
									patternRowHighLightIndex = (list.size() - 1);
									addNewInListHiglited = true;
								}
								else
								{
									patternRowHighLightIndex = jmin<int>(jmax<int>((patternRowHighLightIndex + 1), 0), (list.size() - 1));
								}

								if (((patternRowHighLightIndex + 2) * ROW_HEIGHT > view.getViewPositionY()) && patternRowHighLightIndex > 2)
									view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() + ROW_HEIGHT);
							}
							optionHandlerEnabled = PATTERN_LIST;
							break;
						case FrontPannelParam::paramKey_RMOVERIGHT:
							if (list[getSelected()]->isEditing())
								this->postCommandMessage(4);
							else
							{
								if (addNewInListHiglited)
								{
									addNewInList.setState(Button::ButtonState::buttonNormal);
									addNewInListHiglited = false;
									patternRowHighLightIndex = (list.size() - 1);
								}
								else
									patternRowHighLightIndex = jmin<int>(jmax<int>((patternRowHighLightIndex - 1), 0), (list.size() - 1));

								if ((patternRowHighLightIndex - 2) * ROW_HEIGHT <= (view.getViewPositionY()))
									view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() - ROW_HEIGHT);
							}
							optionHandlerEnabled = PATTERN_LIST;
							break;
						case FrontPannelParam::paramKey_RDBLSELECT:
						case FrontPannelParam::paramKey_RSELECT:
							if (addNewInListHiglited)
								this->postCommandMessage(2); //Add new in list Button
							else if (patternRowHighLightIndex >= list.size())
								list[patternRowHighLightIndex]->postCommandMessage(1);
							else
								this->postCommandMessage(4);
							break;
						case FrontPannelParam::paramKey_PAGESHIFT:
							if (optionHandlerEnabled == PATTERN_LIST)
							{
								optionHandlerEnabled = PATTERN_OPTION;
							}
							else if (optionHandlerEnabled == PATTERN_OPTION)
							{
								optionHandlerEnabled = PATTERN_LIST;
							}
							break;
						case FrontPannelParam::paramKey_RETURN_NAV:
							if (list[getSelected()]->isEditing())
								this->postCommandMessage(4);
							break;
						}

						if ((optionHandlerEnabled == PATTERN_LIST) && !addNewInListHiglited)
							list[patternRowHighLightIndex]->setFocus(true, false);
						repaint();
					}
					else if (!list.isEmpty() && (optionHandlerEnabled == PATTERN_OPTION))
					{
						if (param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
						{
							if (optionHandlerEnabled == PATTERN_OPTION)
							{
								optionHandlerEnabled = PATTERN_LIST;
								addNewP.setBackgroundColour(juce::Colour(0x0E3252));
								param = (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT;
								pannelKeyOptionsHandler(param, KeyValue);
								keyHandled = pannelKeyHandler(param, KeyValue);
							}
							else
								keyHandled = pannelKeyOptionsHandler(param, KeyValue);
						}
					}
					else if (!list.isEmpty() && (optionHandlerEnabled == PATTERN_DETAIL))
					{
						if (param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
						{
							if (optionHandlerEnabled == PATTERN_DETAIL)
							{
								optionHandlerEnabled = PATTERN_OPTION;
								addNewP.setBackgroundColour(juce::Colour(0x0E3252));
								_pStepComp->pannelKeyHandler(param, KeyValue);
								keyHandled = pannelKeyOptionsHandler(param, KeyValue);
							}
							else
								keyHandled = _pStepComp->pannelKeyHandler(param, KeyValue);
						}
					}
					else
					{
						if ((FrontPannelParam)param == FrontPannelParam::paramKey_RSELECT)
							this->postCommandMessage(1); //Add new Button
						keyHandled = true;
					}
				}
			}
		}
		return keyHandled;
	}

	void IgnitePattern::showPatternBars()
	{
		AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
		if (!list.isEmpty() && getTabIndex() != 0)
		{
			auto& gPatternRef = gPatterns.getReference(getSelected());

			if ((gPatternRef.enginePatternData.numBars > 0) && (gPatternRef.enginePatternData.numBars <= 16))
			{
				for (int i = 0; i < gPatternRef.enginePatternData.numBars; i++)
				{
					if (i == gPatternRef.barIndex)
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + i), fpLEDPattern::EN_RGB_LED_LBLUE);
					else
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::param_RGB_DISP1 + i), fpLEDPattern::EN_RGB_LED_LPURPLE);
				}
			}
		}
	}

	bool IgnitePattern::handleLastKey(paramId_t param, uint32_t KeyValue)
	{
		if (isPatternCreated() && isInitCompleted() && getTabIndex() != 0)
		{
			lastKeyStatus = !lastKeyStatus;
			this->postCommandMessage(11);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_LAST, (lastKeyStatus ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));

			if (lastKeyStatus == true)
			{
				if (transposeKeyStatus == true)
				{
					transposeKeyStatus = false;
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_FILL_X, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (skipKeyStatus == true)
				{
					skipKeyStatus = false;
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (gangKeyStatus == true)
				{
					_pComponentStep->setGangButtonState();
					gangKeyStatus = false;
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_GANG, fpLEDPattern::EN_LED_PATT_OFF);
				}
				else if (insertKeyStatus == true)
				{
					_pComponentStep->setPencilIconState(false);
					insertKeyStatus = false;
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_INSERT, fpLEDPattern::EN_LED_PATT_OFF);
					_pComponentGrid->resetPencilToolState();
				}
				else if (tieKeyStatus == true)
				{
					tieKeyStatus = false;
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_NEXT_TIE, fpLEDPattern::EN_LED_PATT_OFF);
				}

				setFpPatternLastStepLedState();
				setFpStepGateLedState();
			}
			else
			{
				if (getTabIndex() != 0)
				{
					setFpStepLedState();
				}
				else
				{
					AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
					setFpPatternLedState();
				}
				AppMainmode::getMainModeInstance()->resetTrackSelLeds();
			}

			return true;
		}

		return false;
	}

	bool IgnitePattern::handleTie(paramId_t param, uint32_t KeyValue)
	{
		if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusReleased)
			return true;
		else if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusPressed)
		{
			if (!list.isEmpty() && getTabIndex() != 0)
			{
				tieKeyStatus = !tieKeyStatus;
				this->postCommandMessage(8);
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_NEXT_TIE, (tieKeyStatus ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));

				if (tieKeyStatus == true)
				{
					if (transposeKeyStatus == true)
					{
						transposeKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_FILL_X, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (skipKeyStatus == true)
					{
						skipKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (gangKeyStatus == true)
					{
						_pComponentStep->setGangButtonState();
						gangKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_GANG, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (insertKeyStatus == true)
					{
						_pComponentStep->setPencilIconState(false);
						insertKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_INSERT, fpLEDPattern::EN_LED_PATT_OFF);
						_pComponentGrid->resetPencilToolState();
					}
					else if (lastKeyStatus == true)
					{
						lastKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_LAST, fpLEDPattern::EN_LED_PATT_OFF);
					}

					setFpStepTieLedState();
					setFpStepGateLedState();
				}
				else
				{
					setFpStepLedState();
					AppMainmode::getMainModeInstance()->resetTrackSelLeds();
				}
			}
		}

		return true;
	}

	bool IgnitePattern::handleTranspose(paramId_t param, uint32_t KeyValue)
	{
		if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusReleased)
			return true;
		else if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusPressed)
		{
			if (!list.isEmpty() && getTabIndex() != 0)
			{
				transposeKeyStatus = !transposeKeyStatus;
				this->postCommandMessage(9);
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_FILL_X, (transposeKeyStatus ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));

				if (transposeKeyStatus == true)
				{
					if (tieKeyStatus == true)
					{
						tieKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_NEXT_TIE, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (skipKeyStatus == true)
					{
						skipKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (gangKeyStatus == true)
					{
						_pComponentStep->setGangButtonState();
						gangKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_GANG, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (insertKeyStatus == true)
					{
						_pComponentStep->setPencilIconState(false);
						insertKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_INSERT, fpLEDPattern::EN_LED_PATT_OFF);
						_pComponentGrid->resetPencilToolState();
					}
					else if (lastKeyStatus == true)
					{
						lastKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_LAST, fpLEDPattern::EN_LED_PATT_OFF);
					}

					setFpStepTransposeLedState();
					setFpStepGateLedState();
				}
				else
				{
					setFpStepLedState();
					AppMainmode::getMainModeInstance()->resetTrackSelLeds();
				}
			}
		}

		return true;
	}

	bool IgnitePattern::handleSkip(paramId_t param, uint32_t KeyValue)
	{
		if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusReleased)
			return true;
		else if (KeyValue == (paramId_t)FrontPannelKeyStatus::paramKeyStatusPressed)
		{
			if (!list.isEmpty() && getTabIndex() != 0)
			{
				skipKeyStatus = !skipKeyStatus;
				this->postCommandMessage(10);
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, (skipKeyStatus ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));

				if (skipKeyStatus == true)
				{
					if (tieKeyStatus == true)
					{
						tieKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_NEXT_TIE, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (transposeKeyStatus == true)
					{
						transposeKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_FILL_X, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (gangKeyStatus == true)
					{
						_pComponentStep->setGangButtonState();
						gangKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_GANG, fpLEDPattern::EN_LED_PATT_OFF);
					}
					else if (insertKeyStatus == true)
					{
						_pComponentStep->setPencilIconState(false);
						insertKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_INSERT, fpLEDPattern::EN_LED_PATT_OFF);
						_pComponentGrid->resetPencilToolState();
					}
					else if (lastKeyStatus == true)
					{
						lastKeyStatus = false;
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_LAST, fpLEDPattern::EN_LED_PATT_OFF);
					}

					setFpStepSkipLedState();
					setFpStepGateLedState();
				}
				else
				{
					setFpStepLedState();
					AppMainmode::getMainModeInstance()->resetTrackSelLeds();
				}
			}
		}

		return true;
	}

	bool IgnitePattern::pannelKeyOptionsHandler(paramId_t param, uint32_t keyValue)
	{
		bool setFocus = true;
		bool moveBtnState = _pAppOptPtr->getBtnState(IgniteControlOptions::eKey::MOVEUP);
		if (keyFocusedItem == optionsViewItemID::NO_FOCUS)
		{
			/*1st key after Page visible */
			if (moveBtnState)
				keyFocusedItem = optionsViewItemID::PATTERN_MOVE_DOWN;
			else
				keyFocusedItem = optionsViewItemID::PATTERN_MOVE;
		}
		else
		{
			pannelSendKey(false, param, keyValue); // clear last Focus
			if (optionHandlerEnabled != PATTERN_OPTION)
				return true;
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				keyFocusedItem = (keyFocusedItem + 1 >= optionsViewItemID::FOCUS_MAX ? (optionsViewItemID::FOCUS_MAX) : keyFocusedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				if (moveBtnState)
					keyFocusedItem = (keyFocusedItem - 1 <= optionsViewItemID::NO_FOCUS ? (optionsViewItemID::NO_FOCUS + 1) : keyFocusedItem - 1);
				else
					keyFocusedItem = (keyFocusedItem - 1 <= optionsViewItemID::PATTERN_MOVE_LAST ? (optionsViewItemID::PATTERN_MOVE_LAST + 1) : keyFocusedItem - 1);
				break;
			}
		}
		if (setFocus)
		{
			pannelSendKey(true, param, keyValue);
		}
		return true;
	}

	void IgnitePattern::pannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		itemFocus = isFocuse;

		switch (optionsView[keyFocusedItem])
		{
		case PATTERN_MOVE_DOWN:
			_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEDOWN, isFocuse, param, KeyValue);
			break;
		case PATTERN_MOVE_UP:
			_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEUP, isFocuse, param, KeyValue);
			break;
		case PATTERN_MOVE_TOP:
			_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVETOP, isFocuse, param, KeyValue);
			break;
		case PATTERN_MOVE_LAST:
			_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVEBTM, isFocuse, param, KeyValue);
			break;
		case PATTERN_MOVE:
			_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MOVE, isFocuse, param, KeyValue);
			break;
		case PATTERN_UNDO:
			_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::UNDO, isFocuse, param, KeyValue);
			break;
		case PATTERN_COPY:
			if (!isFocuse)
				_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::COPY, itemFocus, itemParam, itemKeyValue);
			else
				this->postCommandMessage(3);
			break;
		case PATTERN_CONFIG:
			_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::CONFIG, isFocuse, param, KeyValue);
			break;
		default:
			break;
		}
	}

	void IgnitePattern::handleCommandMessage(int id)
	{
		if (id == 2)
			buttonClicked(&addNewP);
		else if (id == 3)
			_pPatternOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::COPY, itemFocus, itemParam, itemKeyValue);
		else if (id == 4)
			list[patternRowHighLightIndex]->pannelKeyHandler(itemParam, itemKeyValue);
		else if (id == 5)
		{
			_pPatternCopyComp->setLabelText((String)gPatterns[selected].enginePatternData.patternDispName);
			_pPatternCopyComp->setVisible(true);
			_pPatternCopyComp->runModalLoop();
		}
		else if (id == 6)
			buttonClicked(&gridView);
		else if (id == 7)
			buttonClicked(&barView);
		else if (id == 8 || id == 9 || id == 10 || id == 11)
		{
			if (id == 8)
			{
				if (!tieKeyStatus)
				{
					_pInfoWindow->setTextandShow("TIE: OFF");
				}
				else
				{
					_pInfoWindow->setTextandShow("TIE: ON");
				}
			}
			else if (id == 9)
			{
				if (!transposeKeyStatus)
				{
					_pInfoWindow->setTextandShow("TRANSPOSE: OFF");
				}
				else
				{
					_pInfoWindow->setTextandShow("TRANSPOSE: ON");
				}
			}
			else if (id == 10)
			{
				if (!skipKeyStatus)
				{
					_pInfoWindow->setTextandShow("SKIP: OFF");
				}
				else
				{
					_pInfoWindow->setTextandShow("SKIP: ON");
				}
			}
			else if (id == 11)
			{
				if (!lastKeyStatus)
				{
					_pInfoWindow->setTextandShow("LAST STEP: OFF");
				}
				else
				{
					_pInfoWindow->setTextandShow("LAST STEP: ON");
				}
			}
		}
	}

	void IgnitePattern::setFpStepTieLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		if (initCompleted && patternCreated)
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
			for (int step = 0; step < 16; step++)
			{
				auto stepTieStatus = getStepData(mt::Ignite::stepDataIndex::TIE_STATUS, getCurrentBarId(), step);
				if (stepTieStatus)
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_RGB_LED_WHITE);
				else
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_LED_PATT_OFF);
			}
		}
	}

	void IgnitePattern::setFpStepTransposeLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		if (initCompleted && patternCreated)
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
			for (int step = 0; step < 16; step++)
			{
				auto stepTransposeStatus = getStepData(mt::Ignite::stepDataIndex::TRANSPOSE_DEFEAT_STATUS, getCurrentBarId(), step);
				if (stepTransposeStatus)
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_RGB_LED_LBLUE);
				else
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_LED_PATT_OFF);
			}
		}
	}

	void IgnitePattern::setFpStepSkipLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		if (initCompleted && patternCreated)
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
			for (int step = 0; step < 16; step++)
			{
				auto stepSkipStatus = getStepData(mt::Ignite::stepDataIndex::SKIP_STATUS, getCurrentBarId(), step);
				if (stepSkipStatus)
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_RGB_LED_GREEN);
				else
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_LED_PATT_OFF);
			}
		}
	}

	void IgnitePattern::setFpStepGateLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		if (initCompleted && patternCreated)
		{
			AppMainmode::getMainModeInstance()->resetTrackSelLeds();
			for (int step = 0; step < 16; step++)
			{
				auto stepGateStatus = getStepData(mt::Ignite::stepDataIndex::GATE_STATUS, getCurrentBarId(), step);
				if (stepGateStatus)
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::paramKey_TRACKSEL1 + step), fpLEDPattern::EN_LED_PATT_ON);
				else
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::paramKey_TRACKSEL1 + step), fpLEDPattern::EN_LED_PATT_OFF);
			}
		}
	}
	int IgnitePattern::getPatternCurrentBank()
	{
		return currentPatternBank;
	}

	void IgnitePattern::setFpStepLedState()
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__));
		if (initCompleted && patternCreated)
		{
			AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
			for (int step = 0; step < 16; step++)
			{
				auto stepGateStatus = getStepData(mt::Ignite::stepDataIndex::GATE_STATUS, getCurrentBarId(), step);
				if (stepGateStatus)
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpPatternParamLedValues[getActiveParamControl()]);
				else
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::param_RGB_DISP1 + step), fpLEDPattern::EN_LED_PATT_OFF);
			}
		}
	}

	void IgnitePattern::setFpPlayHeadLedState(int playStepNumber)
	{
		if (!initCompleted || !patternCreated)
			return;

		if (tieKeyStatus || transposeKeyStatus || skipKeyStatus)
			return;

		if (previousStepNumber >= 0 && previousStepNumber < 16)
		{
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::paramKey_TRACKSEL1 + previousStepNumber), fpLEDPattern::EN_LED_PATT_OFF);
		}

		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((int)FrontPannelParam::paramKey_TRACKSEL1 + (playStepNumber - 1)), fpLEDPattern::EN_LED_PATT_ON);
		previousStepNumber = playStepNumber - 1;
	}

	/* Reset Pattern views*/
	void IgnitePattern::resetPatterView()
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		gPatternRef.barIndex = 0;
		gPatternRef.stepIndex = 0;

		if (barView.getToggleState())
		{
			_pComponentStep->refreshBarData();
			_pComponentStep->repaint();
		}
		else if (gridView.getToggleState())
		{
			_pComponentGrid->refreshScreen();
			_pComponentGrid->resetGridView();
		}
	}

	void IgnitePattern::onOptCtlButtonClicked(IgniteControlOptions::eKey option)
	{
		if (!patternCreated)
			return;

		opMode = _pPatternOptCtl->getOperationMode();

		if (barView.getToggleState() || gridView.getToggleState())
		{
			if (opMode == mt::Ignite::operatioMode::IDLE_MODE)
			{
				if (barView.getToggleState())
				{
					_pComponentStep->refreshBarData();
					_pComponentStep->repaint();
					_pComponentStep->setSculptState(false);
				}
				else
				{
					_pComponentGrid->refreshScreen();
					_pComponentGrid->repaint();
					_pComponentGrid->setSculptState(false);
				}
			}
		}
		else if (option == IgniteControlOptions::SAVEALLON)
		{
			patternDataUpdated = false;
			patternName.setText((String)gPatterns[selected].enginePatternData.patternDispName, dontSendNotification);
			list[getSelected()]->updateEditIndication();
		}
		else if (option == IgniteControlOptions::AUTOSAVEON)
		{
			isAutosave = true;
		}
		else if (option == IgniteControlOptions::AUTOSAVEOFF)
		{
			isAutosave = false;
		}

		if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
		{
			setEditPattern(selected);
			_pComponentGrid->setSculptButtonState(true);
			/* TODO Disable add bar for all pattern row */
			/*list[editPattern]->enableBarEditButton(false);
			_pComponentStep->enableBarInsertButton(false);
			_pComponentStep->enableBarCopyButton(false);*/
		}
		else if (option == IgniteControlOptions::STOP)
		{
			/* Reset Grid position*/
			resetPatterView();
			_pComponentGrid->setSculptButtonState(false);
		}
		else if (option == IgniteControlOptions::REC)
		{
			/* Reset status Bar*/
			if (opMode == mt::Ignite::operatioMode::IDLE_MODE)
			{
				String displayMessage = "Time: 00:00:00";
				String barDisplayMessage = "Bar:Beat:Step::00:";
				String beatStepDisplayMessage = "00:00";
				auto statusBarComp = AppStatusbar::GetAppStatusBarInstance();

				statusBarComp->setTimeMessage(displayMessage);
				statusBarComp->setBarDisplayMessage(barDisplayMessage);
				statusBarComp->setBeatStepDisplayMessage(beatStepDisplayMessage);
				setPatternEditIndication(true);

				/* Reset Grid position*/
				resetPatterView();
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Recording Stopped");
			}
			else
			{
				list[editPattern]->enableBarEditButton(false);
				list[editPattern]->enableRotatePatternButton(false);
				_pComponentStep->enableBarInsertButton(false);
				_pComponentStep->enableRotatePatternButton(false);
				_pComponentStep->enableBarCopyButton(false);
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Recording Started");
			}
		}
		else if (isVisible())
		{
			list[editPattern]->enableRotatePatternButton(true);
			_pComponentStep->enableBarInsertButton(true);
			_pComponentStep->enableRotatePatternButton(true);
			_pComponentStep->enableBarCopyButton(true);
			_pComponentGrid->setSculptButtonState(false);
		}
		if(patternCreated)
			list[selected]->enableBarEditButton(true);

		if (isVisible())
		{
			auto& gPatternRef = gPatterns.getReference(selected);

			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + ":: Options Button pressed :: " + String(option));

			switch (option) {
			case IgniteControlOptions::SAVE:
				savePattern();
				break;
			case IgniteControlOptions::DEL:
				if (listView.getToggleState() /*|| gPatternRef.enginePatternData.numBars == 1*/)
				{
					if (_pPopUpPtr->isDialogOpen.get() != true)
					{
						_pPopUpPtr->close();
						_pPopUpPtr->setPopUpPropories("Delete " + String((String)gPatterns[selected].enginePatternData.patternDispName), 604, 290);
						_pPopUpPtr->setbtnsize(265, 44);
						_pPopUpPtr->setInfoText("Are you sure you want to delete " + gPatternRef.enginePatternData.patternName);
						_pPopUpPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
						_pPopUpPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
						_pPopUpPtr->popupType = POPUP_WITHOKCANCEL;
						_pPopUpPtr->isCloseButtonVisible(false);
						_pPopUpPtr->show();
						auto selectedPattern = getSelected();
						_pPopUpPtr->onSelectionChanged = [this, selectedPattern](int r)
						{
							auto selected_pattern_index = getSelected();

							if (r == 1)
							{
								auto& gPatternRef = gPatterns.getReference(selected);
								gPatternRef.enginePatternData = _PUiVirtualEngine->EngineGetPatternData(gPatternRef.enginePatternData.patternName);

								if ((gPatternRef.enginePatternData.usedByList.size() > 0) && (IgniteTrack::GetAppTrackInstance()->trackCreated))
								{
									String track_msg;
									(gPatternRef.enginePatternData.usedByList.size() > 1) ? (track_msg = "Tracks") : (track_msg = "Track");

									_pInfoWindow->setTextandShow("Delete operation not possible as selected pattern is Used by " + track_msg + " !!!");

									//_pWarnPopupPtr->close();
									//_pWarnPopupPtr->setWarningText("Delete operation not possible as selected pattern is Used by " /*+ String(gPatternRef.enginePatternData.usedByList.size()) + " " */ + track_msg + " !!!");
									//_pWarnPopupPtr->show();
									//_pWarnPopupPtr->onSelectionChanged = [this](int r)
									//{
									//	if (auto dw = getCurrentlyModalComponent())
									//		dw->exitModalState(0);
									//	DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Closing delete pop since there is a warning in pattern page");
									//	if (auto dw = getCurrentlyModalComponent())
									//		dw->exitModalState(0);
									//};
									return;
								}

								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramRemovePattern, gPatternRef.enginePatternData.patternName);

								if (auto dw = getCurrentlyModalComponent())
									dw->exitModalState(0);

								if ((selected_pattern_index == (gPatterns.size() - 1)) && (selected_pattern_index != 0))
								{
									selected--;
									selected_pattern_index--;
								}
								updateDataFromEngine();
								IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
								IgniteTrack::GetAppTrackInstance()->updateTrackDataList();
								if (gPatterns.size() > 0)
								{
									setSelected(selected_pattern_index);
								}
								else
								{
									patternCreated = statusBarData.isPatternCreated = false;
									changePatternViewVisibility(PatternView::STEP);
									refreshPatternUI();
									enableControlButton(IgniteControlOptions::DEL, false);
								}

								AppMainmode::getMainModeInstance()->resetPadLedsOnViewChange();
								setFpPatternLedState();
							}
							else
							{
								if (auto dw = getCurrentlyModalComponent())
									dw->exitModalState(0);
							}
						};
					}
				}

				if (barView.getToggleState() && gPatternRef.enginePatternData.numBars > 1)
				{
					int barId = gPatterns[selected].barIndex;
					if (_pPopUpPtr->isDialogOpen.get() != true)
					{
						_pPopUpPtr->close();
						_pPopUpPtr->setPopUpPropories("Delete Bar " + (String)(barId + 1), 604, 290);
						_pPopUpPtr->setbtnsize(265, 44);
						_pPopUpPtr->setInfoText("Are you sure you want to delete Bar " + (String)(barId + 1));
						_pPopUpPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
						_pPopUpPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
						_pPopUpPtr->popupType = POPUP_WITHOKCANCEL;
						_pPopUpPtr->isCloseButtonVisible(false);
						_pPopUpPtr->show();
						_pPopUpPtr->onSelectionChanged = [this, barId](int r)
						{
							if (r == 1)
							{
								int selectedPatternIndex = getSelected();
								auto& gPatternRef = gPatterns.getReference(selected);
								_PUiVirtualEngine->EngineRemovePatternBar(gPatternRef.enginePatternData.patternName, (uint16_t)barId);
								if (auto dw = getCurrentlyModalComponent())
									dw->exitModalState(0);
								updateDataFromEngine();
								repaint();
								setSelected(selectedPatternIndex);
								list[selected]->updateLastStep(false);
								_pComponentStep->updateLastStep();
								_pPatternStatusBarPtr->onNumbofBarchangePattern(gPatterns[selected].enginePatternData.numBars - 1);
								if (!IgnitePatternConfig::getAppPatternConfigInstance()->isAutoSaveEnabled())
								{
									patternDataUpdated = true;
									enableControlButton(IgniteControlOptions::SAVE, true);
									enableControlButton(IgniteControlOptions::UNDO, true);
								}
							}
							else
							{
								if (auto dw = getCurrentlyModalComponent())
									dw->exitModalState(0);
							}
						};
					}
				}
				if (stepView.getToggleState())
				{
					_pStepComp->handleDeleteEvent();
				}
				break;
			case IgniteControlOptions::UNDO:
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramUndoCmd, gPatternRef.enginePatternData.patternName);
				gPatternRef.enginePatternData = _PUiVirtualEngine->EngineGetPatternData(""); //NULL name to get data for active Pattern

				updateLables();
				setSelected(selected);
				IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
				if (stepView.getToggleState())
					refreshStepUI(&gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(gPatternRef.stepIndex)));
				else if (barView.getToggleState())
				{
					_pComponentStep->refreshBarData();
					_pComponentStep->repaint();
				}
				else if (gridView.getToggleState())
				{
					_pComponentGrid->refreshScreen();
					_pComponentGrid->repaint();
				}
				break;
			case IgniteControlOptions::LOOP:
				if (_pComponentStep->isVisible())
				{
					if (patternCreated)
					{
						updateBarList();
						refreshBarLoop();
						_pBarLoopComp->setVisible(true);
						_pBarLoopComp->runModalLoop();
					}
				}
				break;
			case IgniteControlOptions::COPY:
				if (listView.getToggleState())
				{
					_pPatternCopyComp->setLabelText((String)gPatterns[selected].enginePatternData.patternDispName);
					_pPatternCopyComp->setVisible(true);
					_pPatternCopyComp->runModalLoop();
				}
				else if (barView.getToggleState())
				{
					_pMainmodeptr = AppMainmode::getMainModeInstance();
					_pMainmodeptr->patternBarCopyComp->setMaxBarIndex(gPatterns[selected].enginePatternData.numBars);
					_pMainmodeptr->patternBarCopyComp->setVisible(true);
					_pMainmodeptr->patternBarCopyComp->runModalLoop();
				}

				break;
			case IgniteControlOptions::MOVE:
				_pAppOptPtr->optionMove.getToggleState() ? moveEnabled = true : moveEnabled = false;
				moveEnabled ? view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all) : view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::never);
				_pAppOptPtr->enableMoveoptions(mt::Ignite::tabs::PATTERN, 0, moveEnabled);
				break;
			case IgniteControlOptions::MOVETOP:
				movePattern(0);
				break;
			case IgniteControlOptions::MOVEBTM:
				movePattern(list.size());
				break;
			case IgniteControlOptions::MOVEDOWN:
				if ((getSelected() + 1) < list.size())
					movePattern(getSelected() + 1);
				break;
			case IgniteControlOptions::MOVEUP:
				if ((getSelected() - 1) >= 0)
					movePattern(getSelected() - 1);
				break;
			case IgniteControlOptions::CONFIG:
				AppMainmode::getMainModeInstance()->changeTabState(mt::Ignite::tabs::MENU);
				AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::PATTERN_WINDOW);
				break;
			}
		}
	}

	void IgnitePattern::savePattern()
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		if (_pPopUpPtr->isDialogOpen.get() != true)
		{
			_pPopUpPtr->close();
			_pPopUpPtr->setPopUpPropories("Save ", 604, 290);
			_pPopUpPtr->setbtnsize(265, 44);
			_pPopUpPtr->setInfoText(String(" Save " + gPatternRef.enginePatternData.patternName + " edits? "));
			_pPopUpPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
			_pPopUpPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
			_pPopUpPtr->popupType = POPUP_WITHOKCANCEL;
			_pPopUpPtr->isCloseButtonVisible(false);
			_pPopUpPtr->show();
			_pPopUpPtr->onSelectionChanged = [this, &gPatternRef](int r)
			{
				if (r == 1)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::setActivePattern, gPatternRef.enginePatternData.patternName);
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSavePatternCmd, gPatternRef.enginePatternData.patternName);
					patternDataUpdated = false;
					updateLables();
					updatePatternEditIndication();
				}

				if (auto dw = getCurrentlyModalComponent())
					dw->exitModalState(0);
			};
		}
	}

	int IgnitePattern::getTabIndex()
	{
		if (listView.getToggleState())
			return 0;
		else if (stepView.getToggleState())
			return 1;
		else if (barView.getToggleState())
			return 2;
		else if (gridView.getToggleState())
			return 3;
		else
			return -1;
	}

	uint16_t IgnitePattern::getCurrentStepPlaying()
	{
		if (editPattern == selected)
			return currentStepPlaying;
		else
			return 0;
	};

	uint8_t IgnitePattern::getNodeCount(int stepNum)
	{
		auto& gPatternRef = gPatterns.getReference(selected);
		return gPatternRef.enginePatternData.BarDataList.at((const unsigned char)gPatternRef.barIndex).StepDataList.at((const unsigned char)getCurrentBarStepId(stepNum)).nodeCount;
	}

	void IgnitePattern::ctlPatternListHeader(bool state)
	{
		if (state)
		{
			patternListNameLable.setVisible(true);
			patternDirectionLable.setVisible(true);
			patternNumBarLable.setVisible(true);
			patternTimebaseLable.setVisible(true);
			patternLastStepLable.setVisible(true);
			patternRotateLable.setVisible(true);
		}
		else
		{
			patternListNameLable.setVisible(false);
			patternDirectionLable.setVisible(false);
			patternNumBarLable.setVisible(false);
			patternTimebaseLable.setVisible(false);
			patternLastStepLable.setVisible(false);
			patternRotateLable.setVisible(false);
		}
	}

	void IgnitePattern::setFpRotaryLedState()
	{
		for (int rot = 0; rot < 16; rot++)
		{
			if ((_pStepComp->getPrevStepIndex() - (paramId_t)FrontPannelParam::paramKey_TRACKROTARY1) == rot)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_TRACKROTARY1 + rot), fpLEDPattern::EN_LED_PATT_ON);
			else
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_TRACKROTARY1 + rot), fpLEDPattern::EN_LED_PATT_OFF);
		}

		for (int rot = 0; rot < STEPVIEW_MAPPED_COMP; rot++)
		{
			if ((_pStepComp->getPrevStepIndex() - (paramId_t)FrontPannelParam::paramKey_TRACKROTARY3) == rot)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_TRACKROTARY3 + rot), fpLEDPattern::EN_LED_PATT_Blink5_On);
			else
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_TRACKROTARY3 + rot), fpLEDPattern::EN_LED_PATT_Blink5_Off);
		}
	}
};
