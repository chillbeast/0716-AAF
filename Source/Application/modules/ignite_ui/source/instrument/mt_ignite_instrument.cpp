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

using namespace Ignite;

namespace ui
{
	juce::Array<mt::Ignite::instrumentsData> gInstruments;


	////////////////////////////////////////////////
	//   Class: Ignite Instruments
	////////////////////////////////////////////////
	IgniteInstruments::IgniteInstruments()
	{
		_pInfoWindow = CommonInfoWindow::GetInfoWindowInstance();

		addNewInList.makeVisibleAndAddListner(this);
		addNewInList.setColour(juce::TextButton::buttonColourId, Theme::Instance().getColour(neutralGrey1));
		addAndMakeVisible(view);
		addAndMakeVisible(container);
		view.setViewedComponent(&container, true);
		view.setBounds(10, 68, 230, 400);
		view.setScrollBarsShown(true, false, true, true);
		view.setScrollBarThickness(5);
		view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);

		setBufferedToImage(true);
		instrumentsCreated = false;
		curretInstrumentsRow = -1;
		refreshInstrumentsUI();

		instrOptInstance = IgniteInstrumentOpt::GetInstrumentInstanceOpt(this);
		addChildComponent(instrOptInstance);
		instrOptInstance->setBounds(250, yStartInstrView, 784, 420);

		emptyInstrComp = new class AppDnp();
		addChildComponent(emptyInstrComp);
		emptyInstrComp->setBounds(10, 118, 1004, 352);
		emptyInstrComp->setDisplayMessage(String(" "));
		emptyInstrComp->setBackgrounImage(juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("img-empty-state-instrument.png")));

		appOptPtr = AppOptions::GetOptionsInstance();
		settingsInstance = IgniteSettings::GetAppSettingsInstance();

		instOptCtr = IgniteControlOptions::GetAppOptInstance();
		instOptCtr->setBounds(xStartOptctl, yStartOptctl, widthOptctl, heightOptctlt);
		instOptCtr->addOptCtlListener(this);

		addNew.makeVisibleAndAddListner(this);
		addNew.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		addNew.setColour(juce::TextEditor::outlineColourId, Theme::Instance().getColour(customBlue13));
		addNew.setColour(TextButton::ColourIds::textColourOffId, Theme::Instance().getColour(customBlue13));
		addNew.setColour(TextButton::ColourIds::buttonColourId, Theme::Instance().getColour(customdblue));
		addNewInList.setVisible(false);
		updateDataFromEngine();
		addNew.setBounds(Rectangle(xStartAddNewBtn, 63, widthAddNewBtn, heightAddNewBtn));

		instrumentCopyComp = new IgniteCopy();
		addChildComponent(instrumentCopyComp);
		instrumentCopyComp->setBounds(xStartLoopCmp, yStartLoopCmp, widthCopy, heightCopy);
		instrumentCopyComp->setVisible(false);

		popupPtr = CommonPopUp::getCommonPopupInstance();
		warnPopupPtr = CommonWarningPopUp::getCommonWarningPopupInstance();
		getAssets();
		instrumentCopyComp->onButtonPressed = [this](IgniteCopy::copyCtlKey key, String copyName)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			copyInstrument(key, copyName);
		};

		for (auto port = (int)InstrumentPorts::cvPort1; port <= (int)InstrumentPorts::cvPort8; port++)
			addCvToUseList(mt::Ignite::portList[port]);

		for (auto port = (int)InstrumentPorts::midiPort1; port <= (int)InstrumentPorts::midiPort5; port++)
		{
			addMidiToUseList(mt::Ignite::portList[port]);
			for (auto channel = 0; channel < 16; channel++)
			{
				addChannelToUseList(mt::Ignite::portList[port], channel);
			}
		}

		for (auto port = (int)InstrumentPorts::usbMidiPort1; port <= (int)InstrumentPorts::usbMidiPort6; port++)
		{
			addMidiToUseList(mt::Ignite::portList[port]);
			for (auto channel = 0; channel < 16; channel++)
			{
				addChannelToUseList(mt::Ignite::portList[port], channel);
			}
		}

		updateMidiDeviceList();
	}

	IgniteInstruments::~IgniteInstruments()
	{
		cleanup();
	}

	void IgniteInstruments::copyInstrument(IgniteCopy::copyCtlKey key, String copyName)
	{
		if (copyName.isNotEmpty())
		{
			if (key == IgniteCopy::copyCtlKey::COPY)
			{
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentCopy, copyName.toStdString());
				updateDataFromEngine();
			}
		}

		instrumentCopyComp->exitModalState(0);
		instrumentCopyComp->setVisible(false);
	}

	void IgniteInstruments::updateMidiDeviceList()
	{
		std::vector<IgniteParamList> instrNameList;
		IgniteParamList params;
		auto numOfHost = mt::Ignite::midiPortList.size() - 11;

		if (numOfHost > 0)
			mt::Ignite::midiPortList.erase(mt::Ignite::midiPortList.end() - numOfHost, mt::Ignite::midiPortList.end());

		numOfHost = mt::Ignite::portList.size() - 19;
		if(numOfHost > 0)
			mt::Ignite::portList.erase(mt::Ignite::portList.end() - numOfHost, mt::Ignite::portList.end());

		instrNameList.clear();
		params.param = (paramId_t)SongParam::paramUsbHostMidiDeviceNameList;
		params.startPos = 0;
		params.Count = 5;
		instrNameList.push_back(params);
		instrNameList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, instrNameList);

		if (instrNameList.size())
		{
			for (auto i = 0; i < instrNameList.size(); i++)
			{
				mt::Ignite::portList.push_back(instrNameList[i].MsgValue.get<std::string>());
				mt::Ignite::midiPortList.push_back(instrNameList[i].MsgValue.get<std::string>());
			}
		}
		instrOptInstance->updateMidiPorts();
		IgniteMidiConfig::GetAppMidiConfigInstance()->updateMidiPorts();
		IgniteAuditionSettings::GetAppAuditionInstance()->updateUSBMidiName();
	}

	int IgniteInstruments::getSelected(void)
	{
		return selected;
	}

	void IgniteInstruments::setSelected(int index)
	{
		if (selected != index)
			enableControlButton(IgniteControlOptions::ALL, false);

		if (isPositiveAndBelow(index, list.size()))
			selected = index;

		// Highlist selected row.
		for (auto& i : list)
			i->highlight(false);
		list[selected]->highlight(true);

		requestDataFromEngine(selected);

		instrOptInstance->setVisible(true);
		instrOptInstance->refreshData();
		if (this->isVisible())
		{
			if (instrumentsCreated && ((instOptCtr->getOperationMode() != mt::Ignite::operatioMode::PLAY_MODE) || (instOptCtr->getOperationMode() != mt::Ignite::operatioMode::PLAY_REC_MODE)))
			{
				enableControlButton(IgniteControlOptions::DEL, true);
				enableControlButton(IgniteControlOptions::COPY, true);
				if (list.size() > 1)
				{
					enableControlButton(IgniteControlOptions::MOVE, true);
				}
			}

			if (instrumentsDataUpdated && !autoSaveState)
			{
				enableControlButton(IgniteControlOptions::SAVE, true);
				enableControlButton(IgniteControlOptions::UNDO, true);
			}
		}
	}

	void IgniteInstruments::updateDataFromEngine()
	{
		std::vector<IgniteParamList> instrumentParamList;
		IgniteParamList params;

		list.clear();
		gInstruments.clear();

		params.param = (paramId_t)InstrumentParam::paramInstrumentNameList;
		instrumentParamList.push_back(params);
		instrumentParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentParamList);
		selected = 0;

		if (instrumentParamList.size() != 0)
		{
			instrumentsCreated = true;
			for (auto& i : instrumentParamList)
			{
				if (i.param == (paramId_t)InstrumentParam::paramInstrumentName)
				{
					addInstrument(false);
					setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_NAME, i.MsgValue.get<std::string>(), true);
				}
				if (i.param == (paramId_t)InstrumentParam::paramInstrumentDispName)
				{
					setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_DISP_NAME, i.MsgValue.get<std::string>(), true);
					selected++; //Increment selected after adding name and Disp name
				}
			}
			selected = 0;
			setSelected(selected);
			emptyInstrComp->setVisible(false);
			addNew.setVisible(false);
			addNewInList.setVisible(true);
			instrOptInstance->setVisible(true);
			settingsInstance->setWipeInstrumentsBtnState(true);
		}
		else
		{
			settingsInstance->setWipeInstrumentsBtnState(false);
			instrumentsCreated = false;
			emptyInstrComp->setVisible(true);
			addNew.setVisible(true);
			addNewInList.setVisible(false);
			instrOptInstance->setVisible(false);
		}
	}

	void IgniteInstruments::requestDataFromEngine(int index)
	{
		std::vector<IgniteParamList> instrumentParamList;
		IgniteParamList params;
		auto& gInstrumentRef = gInstruments.getReference(selected);

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstrumentRef.name.toStdString());

		params.param = (paramId_t)InstrumentParam::paramInstrumentName;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramInstrumentPort;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramInstrumentChannel;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramInstrumentPgmNum;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramInstrumentPgmBankLsb;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramInstrumentPgmBankMsb;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramDisplayNoteNum;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramEditTrackThruStatus;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramFtsDefeatStatus;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramInternalSound;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramInternalSoundPgm;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramMultiTimbralStatus;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramPolySpreadStatus;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramPreSendPgmChange;
		instrumentParamList.push_back(params);

		params.param = (paramId_t)InstrumentParam::paramTransposeDefeatStatus;
		instrumentParamList.push_back(params);

		instrumentParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentParamList);

		for (auto& instrData : instrumentParamList)
		{
			switch (instrData.param)
			{
			case (paramId_t)InstrumentParam::paramInstrumentName:
				gInstrumentRef.name = instrData.MsgValue.get< std::string >();
				break;
			case (paramId_t)InstrumentParam::paramInstrumentPort:
				gInstrumentRef.midiPortIndex = instrData.MsgValue.get< std::int8_t >();
				if (gInstrumentRef.midiPortIndex >= 12 && gInstrumentRef.midiPortIndex <= 18)
					markAsCvInUse(mt::Ignite::portList[gInstrumentRef.midiPortIndex]);
				break;
			case (paramId_t)InstrumentParam::paramInstrumentChannel:
				gInstrumentRef.midiChannelIndex = (instrData.MsgValue.get< std::int8_t >()) + 1;
				break;
			case (paramId_t)InstrumentParam::paramInstrumentPgmNum:
				gInstrumentRef.pgmChange = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramInstrumentPgmBankLsb:
				gInstrumentRef.cc0State = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramInstrumentPgmBankMsb:
				gInstrumentRef.cc32State = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramDisplayNoteNum:
				gInstrumentRef.noteNums = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramEditTrackThruStatus:
				gInstrumentRef.edtrkThruIndex = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramFtsDefeatStatus:
				gInstrumentRef.ftsState = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramInternalSoundPgm:
				gInstrumentRef.programIndex = instrData.MsgValue.get< int >();
				break;
			case (paramId_t)InstrumentParam::paramInternalSound:
				gInstrumentRef.isIntSoundEnabled = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramMultiTimbralStatus:
				gInstrumentRef.multiState = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramPolySpreadStatus:
				gInstrumentRef.polySpreadIndex = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramPreSendPgmChange:
				gInstrumentRef.pgm = instrData.MsgValue.get< std::uint8_t >();
				break;
			case (paramId_t)InstrumentParam::paramTransposeDefeatStatus:
				gInstrumentRef.xposeState = instrData.MsgValue.get< std::uint8_t >();
				break;
			}
		}
		instrOptInstance->refreshData();
	}

	void IgniteInstruments::updateLables()
	{
		std::vector<IgniteParamList> instrumentParamList;
		IgniteParamList params;

		params.param = (paramId_t)InstrumentParam::paramUndoListCmd;
		instrumentParamList.push_back(params);
		instrumentParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentParamList);

		/*instrOptInstance->updateAllLablesText(false);

		for (auto& j : instrumentParamList)
		{
			if (j.param == (paramId_t)InstrumentParam::paramInstrumentName)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_NAME, true);
			else if (j.param == (paramId_t)InstrumentParam::paramInstrumentPort)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_PORT, true);
			else if (j.param == (paramId_t)InstrumentParam::paramDisplayNoteNum)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_NOTENUM, true);
			else if (j.param == (paramId_t)InstrumentParam::paramInternalSoundPgm)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX, true);
			else if (j.param == (paramId_t)InstrumentParam::paramInstrumentChannel)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL, true);
			else if (j.param == (paramId_t)InstrumentParam::paramCc0BankLStatus)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_CC0, true);
			else if (j.param == (paramId_t)InstrumentParam::paramCc0BankMStatus)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_CC32, true);
			else if (j.param == (paramId_t)InstrumentParam::paramEditTrackThruStatus)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_EDTRK, true);
			else if (j.param == (paramId_t)InstrumentParam::paramFtsDefeatStatus)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_FTS, true);
			else if (j.param == (paramId_t)InstrumentParam::paramMultiTimbralStatus)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_MULTI, true);
			else if (j.param == (paramId_t)InstrumentParam::paramDisplayNoteNum)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_NOTENUM, true);
			else if (j.param == (paramId_t)InstrumentParam::paramInternalSoundPgm)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_PGM, true);
			else if (j.param == (paramId_t)InstrumentParam::paramPolySpreadStatus)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_POLY, true);
			else if (j.param == (paramId_t)InstrumentParam::paramTransposeDefeatStatus)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INSTR_XPOSE, true);
			else if (j.param == (paramId_t)InstrumentParam::paramInternalSound)
				instrOptInstance->updateLableText(mt::Ignite::instrumentsDataIndex::INTERNAL_SOUND, true);
		}*/
		if (instrumentParamList.size() == 1)
		{
			enableControlButton(IgniteControlOptions::SAVE, false);
			enableControlButton(IgniteControlOptions::UNDO, false);
			instrumentsDataUpdated = false;
			updateInstrEditIndication();
		}
		instrOptInstance->refreshData();
	}

	void IgniteInstruments::cleanup()
	{
		container.removeAllChildren();
		list.clear();
	}

	String IgniteInstruments::getInstrName()
	{
		std::vector<IgniteParamList> instrNameList;
		IgniteParamList params;

		instrNameList.clear();
		params.param = (paramId_t)InstrumentParam::paramInstrumentNameList;
		params.startPos = 0;
		params.Count = 100;
		instrNameList.push_back(params);
		instrNameList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrNameList);

		if (instrNameList.size())
		{
			int j = 0;
			for (auto i = 0; i < instrNameList.size(); i++)
			{
				if (instrNameList.size() / 2 == gInstruments.size())
				{
					auto& gInstrumentRef = gInstruments.getReference(j);
					gInstrumentRef.name = instrNameList[i++].MsgValue.get<std::string>();
					gInstrumentRef.DispName = instrNameList[i].MsgValue.get<std::string>();
					j++;
				}
			}
			return instrNameList[instrNameList.size() - 1].MsgValue.get<std::string>();
		}
		else
			return "";
	}

	void IgniteInstruments::populate()
	{
		mt::Ignite::instrumentsData instrumentsdata;
		IgniteInstrumentsRow* row;
		String name = getInstrName();

		curretInstrumentsRow++;
		instrumentsdata = gInstruments[list.size()];

		if (name != "")
			instrumentsdata.name = getInstrName();

		row = new IgniteInstrumentsRow(list.size(), instrumentsdata, this);
		list.add(row);
		container.addAndMakeVisible(row);
		instrumentsCreated = true;
		IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
		refreshInstrumentsUI();
	}

	void IgniteInstruments::refreshInstrumentsUI()
	{
		int index = 0;
		for (auto& i : list)
		{
			i->setBounds(0, ((52 * index++)), 220, 44);
			i->onMouseDown = [this](IgniteInstrumentsRow* Row)
			{
				showPopup((Row->getSerialNumber()));
			};
		}

		addNewInList.setBounds(0, (52 * index), 220, 44);

		// Set overall view container size....
		container.setSize(220, ((index + 1) * 52));
		//container.setSize(widthInstMenu, ((index + 1) * layout::row.getHeight()));
		if (instrumentsCreated)
		{
			setSelected(selected);
			container.addAndMakeVisible(addNewInList);
		}
		if (addNewInList.isVisible())
			addNew.setVisible(false);
	}

	void IgniteInstruments::addInstrument(bool isPatternUpdateneeded)
	{
		String instrument_name = String("INSTR" + String(list.size() + 1));
		gInstruments.add({ list.size() , instrument_name, instrument_name, list.size(), true, 0, 1 });
		if (isPatternUpdateneeded)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentAdd, 0);
			//_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, instrument_name.toStdString());
		}
		populate();
		emptyInstrComp->setVisible(false);
		addNewInList.setVisible(true);
		settingsInstance->setWipeInstrumentsBtnState(true);
	}

	void IgniteInstruments::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		if (buttonThatWasClicked == &addNewInList || buttonThatWasClicked == &addNew)
		{
			addNew.setVisible(false);
			addInstrument(true);
			repaint();
			if (addNewInList.getToggleState())
				addNewInList.setToggleState(false, dontSendNotification);
		}
	}

	void IgniteInstruments::getAssets()
	{
		imageAddInstActive = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newinstrument-active.png"));
		imageAddInstDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newinstrument-default.png"));
		imageAddInstHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-add-newinstrument-hover.png"));

	}


	void IgniteInstruments::paint(juce::Graphics& g)
	{

		MenuMode::GetMenuModeInstance()->setHeader("Instrument Definition");
		if (instrumentsCreated && this->isVisible() && ((instOptCtr->getOperationMode() != mt::Ignite::operatioMode::PLAY_MODE) || (instOptCtr->getOperationMode() != mt::Ignite::operatioMode::PLAY_REC_MODE)))
		{
			enableControlButton(IgniteControlOptions::DEL, true);
			enableControlButton(IgniteControlOptions::COPY, true);
		}

		if (addNew.isVisible())
		{
			Image Draw = addNew.getToggleState() ? imageAddInstActive : addNew.isMouseOver() ? imageAddInstHover : imageAddInstDefault;
			g.drawImage(Draw, ui::Rectangle<float>{xStartAddNewBtn, 63, widthAddNewBtn, heightAddNewBtn }, RectanglePlacement::centred);
		}
	}

	void IgniteInstruments::resized()
	{
	}

	void IgniteInstruments::updateData()
	{
		std::vector<IgniteParamList> SettingsGeneralCnfgParams;
		IgniteParamList params;
		//TODO: New setting 
		//params.param = (paramId_t)SettingsGeneralCnfgParam::SettingIstrumentAutoSaveParam;
		//SettingsGeneralCnfgParams.push_back(params);

		//SettingsGeneralCnfgParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, SettingsGeneralCnfgParams);

		//enableControlButton(IgniteControlOptions::AUTOSAVE, (bool)SettingsGeneralCnfgParams[0].MsgValue.get<int8_t>());
		//AppOptions::GetOptionsInstance()->changebtnstate(IgniteControlOptions::AUTOSAVE, (bool)SettingsGeneralCnfgParams[0].MsgValue.get<int8_t>());
	}

	void IgniteInstruments::updateInstrEditIndication()
	{
		list[selected]->updateEditIndication();
	}

	void IgniteInstruments::showPopup(int index)
	{
		if (popupPtr->isDialogOpen.get() != true)
		{
			popupPtr->close();
			popupPtr->setPopUpPropories("Switch Instrument", 604, 290);
			popupPtr->setbtnsize(184, 44);
			popupPtr->addImageButton("Cancel", "btn-cancel-174x44-default-hover.png", "btn-cancel-174x44-default.png", 2);
			popupPtr->addImageButton("Confirm Switch", "btn-confirmswitch-174x44-default.png", "btn-confirmswitch-174x44-default-hover.png", 1);
			popupPtr->addImageButton("Save and Switch", "btn-saveswitch-174x44-default.png", "btn-saveswitch-174x44-default-hover.png", 3);
			popupPtr->setInfoText(String(gInstruments[selected].name) + " active pattern unsaved data will be lost");

			if ((index == selected) || (!instrumentsDataUpdated) || autoSaveState)
			{
				setSelected(index);
				return;
			}
			prevSelected = index;
			popupPtr->popupType = POPUP_CHANGEWARNING;
			popupPtr->isCloseButtonVisible(false);
			popupPtr->show();
			popupPtr->onSelectionChanged = [this](int r)
			{
				if (r == 1)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramClearUndoList, gInstruments.getReference(selected).name.toStdString());
					instrumentsDataUpdated = false;
					updateLables();
					setSelected(prevSelected);
				}
				if (r == 3)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstruments.getReference(selected).name.toStdString());
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentSave, gInstruments.getReference(selected).name.toStdString());
					instrumentsDataUpdated = false;
					updateLables();
					setSelected(prevSelected);
				}

				if (r == 2)
					instRowHighLightIndex = prevSelected - 1;

				if (auto dw = getCurrentlyModalComponent())
					dw->exitModalState(0);

			};
		}
	}

	void IgniteInstruments::changeInstrumentsOptionsVisibility(bool bShouldShow)
	{
		instrOptInstance->setVisible(bShouldShow);
		for (auto eachRow : list)
		{
			eachRow->setVisible(!bShouldShow);
		}
		resized();
	}

	void IgniteInstruments::updateInstrumentsData(uint8_t param, const uint32_t value)
	{
		auto& gInstrumentRef = gInstruments.getReference(selected);

		switch ((InstrumentParam)param)
		{
		case InstrumentParam::paramInstrumentPort:
			gInstrumentRef.midiPortIndex = value;
			break;
		case InstrumentParam::paramInstrumentChannel:
			gInstrumentRef.midiChannelIndex = value;
			break;
		}
	}

	void IgniteInstruments::updateInstrumentsData(uint8_t param, const std::string& value)
	{
		auto& gInstrumentRef = gInstruments.getReference(selected);
		switch ((InstrumentParam)param)
		{
		case InstrumentParam::paramInstrumentName:
			gInstrumentRef.name = value;
			break;
		}
		instrOptInstance->refreshData();
		list[selected]->refreshInstrumentsRowUI();
	}

	void IgniteInstruments::setInstrumentData(mt::Ignite::instrumentsDataIndex item_id, int data, bool is_internal)
	{
		auto& gInstrumentRef = gInstruments.getReference(selected);
		bool mode = false;
		if ((instOptCtr->getOperationMode() == mt::Ignite::operatioMode::PLAY_MODE) || (instOptCtr->getOperationMode() == mt::Ignite::operatioMode::PLAY_REC_MODE))
			mode = true;

		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstrumentRef.name.toStdString());

		switch (item_id)
		{
		case mt::Ignite::instrumentsDataIndex::INSTR_PORT:
			gInstrumentRef.midiPortIndex = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentPort, (int8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL:
			gInstrumentRef.midiChannelIndex = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentChannel, (int8_t)(data - 1), mode);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_EDTRK:
			gInstrumentRef.edtrkThruIndex = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramEditTrackThruStatus, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_DEFNOTE:
			gInstrumentRef.defNoteIndex = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramDisplayNoteNum, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_MULTI:
			gInstrumentRef.multiState = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramMultiTimbralStatus, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_XPOSE:
			gInstrumentRef.xposeState = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTransposeDefeatStatus, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_POLY:
			gInstrumentRef.polySpreadIndex = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramPolySpreadStatus, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_FTS:
			gInstrumentRef.ftsState = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramFtsDefeatStatus, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_PGM_CHANGE:
			gInstrumentRef.pgmChange = (int8_t)data;
			if (!is_internal)
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentPgmNum, (int8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_CC0:
			gInstrumentRef.cc0State = (int8_t)data;
			if (!is_internal)
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentPgmBankLsb, (int8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_CC32:
			gInstrumentRef.cc32State = (int8_t)data;
			if (!is_internal)
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentPgmBankMsb, (int8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_NOTENUM:
			gInstrumentRef.noteNums = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramDisplayNoteNum, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_PGM:
			gInstrumentRef.pgm = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramPreSendPgmChange, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::INTERNAL_SOUND:
			gInstrumentRef.isIntSoundEnabled = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInternalSound, (uint8_t)data);
			break;
		case mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX:
			gInstrumentRef.programIndex = data - 1;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInternalSoundPgm, data - 1);
			break;
		default: break;
		}
		instrOptInstance->refreshData();
		list[selected]->refreshInstrumentsRowUI();
	}

	void IgniteInstruments::setInstrumentData(mt::Ignite::instrumentsDataIndex item_id, String data, bool is_internal)
	{
		auto& gInstrumentRef = gInstruments.getReference(selected);
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstrumentRef.name.toStdString());

		switch (item_id)
		{
		case mt::Ignite::instrumentsDataIndex::INSTR_NAME:
			gInstrumentRef.name = data;
			if (!is_internal)
			{



				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "set INSTR_NAME - UI should not modify FS name");
			}
			break;
		case mt::Ignite::instrumentsDataIndex::INSTR_DISP_NAME:
			gInstrumentRef.DispName = data;
			if (!is_internal)
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentDispName, data.toStdString());
			break;
		default: break;
		}
		instrOptInstance->refreshData();
		list[selected]->refreshInstrumentsRowUI();
	}

	var IgniteInstruments::getInstrumentData(mt::Ignite::instrumentsDataIndex item_id)
	{
		switch (item_id)
		{
		case mt::Ignite::instrumentsDataIndex::INSTR_NAME:
			return gInstruments[selected].name;
		case mt::Ignite::instrumentsDataIndex::INSTR_DISP_NAME:
			return gInstruments[selected].DispName;
		case mt::Ignite::instrumentsDataIndex::INSTR_PORT:
			return gInstruments[selected].midiPortIndex;
		case mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL:
			return gInstruments[selected].midiChannelIndex;
		case mt::Ignite::instrumentsDataIndex::INSTR_MULTI:
			return gInstruments[selected].multiState;
		case mt::Ignite::instrumentsDataIndex::INSTR_XPOSE:
			return gInstruments[selected].xposeState;
		case mt::Ignite::instrumentsDataIndex::INSTR_EDTRK:
			return gInstruments[selected].edtrkThruIndex;
		case mt::Ignite::instrumentsDataIndex::INSTR_POLY:
			return gInstruments[selected].polySpreadIndex;
		case mt::Ignite::instrumentsDataIndex::INSTR_FTS:
			return gInstruments[selected].ftsState;
		case mt::Ignite::instrumentsDataIndex::INSTR_PGM_CHANGE:
			return gInstruments[selected].pgmChange;
		case mt::Ignite::instrumentsDataIndex::INSTR_CC0:
			return gInstruments[selected].cc0State;
		case mt::Ignite::instrumentsDataIndex::INSTR_CC32:
			return gInstruments[selected].cc32State;
		case mt::Ignite::instrumentsDataIndex::INSTR_NOTENUM:
			return gInstruments[selected].noteNums;
		case mt::Ignite::instrumentsDataIndex::INSTR_PGM:
			return gInstruments[selected].pgm;
		case mt::Ignite::instrumentsDataIndex::INSTR_DEFNOTE:
			return gInstruments[selected].defNoteIndex;
		case mt::Ignite::instrumentsDataIndex::INTERNAL_SOUND:
			return gInstruments[selected].isIntSoundEnabled;
		case mt::Ignite::instrumentsDataIndex::PROGRAM_INDEX:
			return gInstruments[selected].programIndex + 1;
		default: return -1;
		}
	}

	void IgniteInstruments::deleteCurrentInstruments()
	{
		//Send request to Engine for deleting instruments
		container.removeChildComponent(selected);
		list.remove(selected);
		container.setSize(220, (list.size() * layout::row.getHeight()));
	}

	String IgniteInstruments::getCurrentPort()
	{
		return instrOptInstance->getPortName(getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT));
	}

	void IgniteInstruments::onOptCtlButtonClicked(AppOptCtl::pKey option)
	{

		switch (option) {
		case AppOptCtl::pKey::BACK:
			if (instrumentsDataUpdated)
			{
				instrumentsDataUpdated = false;
			}
			changeInstrumentsOptionsVisibility(false);
			break;
		case AppOptCtl::pKey::SAVE:
			if (instrumentsDataUpdated)
			{
				instrumentsDataUpdated = false;
				IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
			}
			changeInstrumentsOptionsVisibility(false);
			break;
		case AppOptCtl::pKey::COPY: break;
		case AppOptCtl::pKey::MOVE: break;
		case AppOptCtl::pKey::DEL:
			deleteCurrentInstruments();
			changeInstrumentsOptionsVisibility(false);
			break;
		}
		refreshInstrumentsUI();
	}

	void IgniteInstruments::moveInstrument(juce::Point<float> position)
	{
		int index = static_cast<int>(((getSelected() * (layout::row.getHeight())) + position.getY()) / (layout::row.getHeight()));
		if ((index == getSelected()))
			return;

		uint64_t SourceDestIndex = ((uint64_t)getSelected() << 32) | index;
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentMoveCmd, SourceDestIndex);
		updateDataFromEngine();
		setSelected(index);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " moved to index : " + String(index));
	}

	void IgniteInstruments::moveInstrument(int destIndex)
	{
		if ((destIndex == getSelected()))
			return;

		uint64_t SourceDestIndex = ((uint64_t)getSelected() << 32) | destIndex;
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentMoveCmd, SourceDestIndex);
		updateDataFromEngine();
		if ((list.size() == destIndex) && (destIndex > 0))
			destIndex--;
		setSelected(destIndex);
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + " moved to index : " + String(destIndex));
	}

	bool IgniteInstruments::isInstrumentUsed(String instr_name)
	{
		std::vector<IgniteParamList> instrumentUsedByList;
		IgniteParamList params;
		params.param = (paramId_t)InstrumentParam::paramUsedByListCmd;
		params.MsgValue = instr_name.toStdString();
		instrumentUsedByList.push_back(params);
		instrumentUsedByList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentUsedByList);

		if (instrumentUsedByList.size() > 1)
			return true;
		else
			return false;
	}

	bool IgniteInstruments::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		if (!instrumentsCreated &&(param != (paramId_t)FrontPannelParam::paramKey_INSERT) && (param != (paramId_t)FrontPannelParam::paramKey_RETURN_NAV))
			return false;

		if (KeyValue == (uint32_t)FrontPannelKeyStatus::paramKeyStatusReleased)
			return true;

		bool keyHandled = false;
		itemParam = param;
		itemKeyValue = KeyValue;

		if (instrumentCopyComp->isVisible())
			return instrumentCopyComp->PannelKeyHandler(param, KeyValue);

		if (popupPtr != nullptr && popupPtr->IsEditing())
		{
			if (warnPopupPtr != nullptr && warnPopupPtr->IsEditing())
				return warnPopupPtr->FPKeyHandler(true, param, KeyValue);
			else
			{
				if ((param != (paramId_t)FrontPannelParam::paramRot_ROW) && (param != (paramId_t)FrontPannelParam::paramKey_ROTARYROW))
				{
					return popupPtr->FPKeyHandler(true, param, KeyValue);
				}
				else
				{
					if (param == (paramId_t)FrontPannelParam::paramRot_ROW)
					{
						if (KeyValue == ROTARY_RIGHT || KeyValue == ROTARY_FASTRIGHT || KeyValue == ROTARY_SUPERFASTRIGHT)
							param = (paramId_t)FrontPannelParam::paramKey_RMOVELEFT;
						else if (KeyValue == ROTARY_LEFT || KeyValue == ROTARY_FASTLEFT || KeyValue == ROTARY_SUPERFASTLEFT)
							param = (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT;
					}
					else if (param == (paramId_t)FrontPannelParam::paramKey_ROTARYROW)
					{
						param = (paramId_t)FrontPannelParam::paramKey_RSELECT;
					}

					return popupPtr->FPKeyHandler(true, param, 1);
				}
			}
		}

		if (param == (paramId_t)FrontPannelParam::paramKey_NEXT_TIE && instOptCtr->getOperationMode() == mt::Ignite::operatioMode::IDLE_MODE)
		{
			if (!list.isEmpty())
			{
				list[instRowHighLightIndex]->SetFocus(false, false);
				this->postCommandMessage(4);

				if (instRowHighLightIndex + 1 >= list.size())
					instRowHighLightIndex = 0;
				else
					instRowHighLightIndex = jmin<int>(jmax<int>((instRowHighLightIndex + 1), 0), (list.size() - 1));

				if ((instRowHighLightIndex) * 52 > (view.getViewArea().getHeight() - 52))
					view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() + 52);

				list[instRowHighLightIndex]->SetFocus(true, false);
				repaint();
			}
			return true;
		}

		if (list.isEmpty())
		{
			if (addNew.isVisible() == true)
			{
				if ((FrontPannelParam)param == FrontPannelParam::paramKey_RSELECT || (FrontPannelParam)param == FrontPannelParam::paramKey_INSERT)
					this->postCommandMessage(2); //Add new Button
			}
			if (FrontPannelParam::paramKey_RETURN_NAV == (FrontPannelParam)param)
			{
				AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::MENU_WINDOW);
				return true;
			}
		}
		else
		{
			if (!list.isEmpty() && ((FrontPannelParam)param == FrontPannelParam::paramRot_ROW || (FrontPannelParam)param == FrontPannelParam::paramKey_ROTARYROW ||
				(FrontPannelParam)param == FrontPannelParam::paramKey_INSERT))
			{
				list[instRowHighLightIndex]->SetFocus(false, false);
				addNewInList.setState(Button::ButtonState::buttonNormal);
				switch ((FrontPannelParam)param)
				{
				case FrontPannelParam::paramRot_ROW:
					if (KeyValue == ROTARY_RIGHT || KeyValue == ROTARY_FASTRIGHT || KeyValue == ROTARY_SUPERFASTRIGHT)
					{
						if (list[instRowHighLightIndex]->isEditing())
							this->postCommandMessage(4);
						else
						{
							if (instRowHighLightIndex + 1 >= list.size() && addNewInList.isVisible())
							{
								addNewInList.setState(Button::ButtonState::buttonOver);
								instRowHighLightIndex = (list.size() - 1);
								addNewInListHiglited = true;
							}
							else
							{
								instRowHighLightIndex = jmin<int>(jmax<int>((instRowHighLightIndex + 1), 0), (list.size() - 1));
							}

							if ((instRowHighLightIndex) * 52 > (view.getViewArea().getHeight() - 52))
								view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() + 52);
						}
					}
					else if (KeyValue == ROTARY_LEFT || KeyValue == ROTARY_FASTLEFT || KeyValue == ROTARY_SUPERFASTLEFT)
					{
						if (list[instRowHighLightIndex]->isEditing())
							this->postCommandMessage(4);
						else
						{
							if (addNewInListHiglited)
							{
								addNewInList.setState(Button::ButtonState::buttonNormal);
								addNewInListHiglited = false;
								instRowHighLightIndex = (list.size() - 1);
							}
							else
								instRowHighLightIndex = jmin<int>(jmax<int>((instRowHighLightIndex - 1), 0), (list.size() - 1));

							if ((instRowHighLightIndex - 1) * 52 <= (view.getViewArea().getHeight()))
								view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() - 52);
						}
					}
					break;
				case FrontPannelParam::paramKey_ROTARYROW:
					if (addNewInListHiglited)
						this->postCommandMessage(2); //Add new in list Button
					else
						this->postCommandMessage(4);
					break;
				case FrontPannelParam::paramKey_INSERT:
					this->postCommandMessage(2); //Add new in list Button
					break;
				}
				if (!addNewInListHiglited)
					list[instRowHighLightIndex]->SetFocus(true, false);
				repaint();
			}
			else if ((option_handler_focus == INST_OPTION) && !addNew.isVisible() && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
			{
				keyHandled = PannelKeyOptionsHandler(param, KeyValue);
			}
			else if (param == (paramId_t)FrontPannelParam::paramKey_DELETE && KeyValue == 1)
			{
				postCommandMessage(5);
			}
			else if (param == (paramId_t)FrontPannelParam::paramKey_SAVE && KeyValue == 1)
			{
				postCommandMessage(6);
			}
			else if ((option_handler_focus == INST_DETAIL) && !addNew.isVisible() && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
			{
				keyHandled = instrOptInstance->PannelKeyHandler(param, KeyValue);
			}
			else if (!list.isEmpty() && (option_handler_focus == INST_OPTION))
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
				{
					if (option_handler_focus == INST_OPTION)
					{
						option_handler_focus = INST_DETAIL;
						addNew.setBackgroundColour(juce::Colour(0x0E3252));
						param = (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT;
						PannelKeyOptionsHandler(param, KeyValue);
						keyHandled = PannelKeyHandler(param, KeyValue);
					}
					else
						keyHandled = PannelKeyOptionsHandler(param, KeyValue);
				}
			}
			else if (!list.isEmpty() && (option_handler_focus == INST_DETAIL))
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
				{
					if (option_handler_focus == INST_DETAIL)
					{
						option_handler_focus = INST_OPTION;
						addNew.setBackgroundColour(juce::Colour(0x0E3252));
						instrOptInstance->PannelKeyHandler(param, KeyValue);
						keyHandled = PannelKeyOptionsHandler(param, KeyValue);
					}
					else
						keyHandled = instrOptInstance->PannelKeyHandler(param, KeyValue);
				}
			}
			else
			{
				if ((FrontPannelParam)param == FrontPannelParam::paramKey_RSELECT)
					this->postCommandMessage(1); //Add new Button
				keyHandled = true;
			}
		}
		return keyHandled;
	}

	bool IgniteInstruments::PannelKeyOptionsHandler(paramId_t param, uint32_t KeyValue)
	{
		bool setfocus = true;
		bool moveBtnState = appOptPtr->getBtnState(IgniteControlOptions::eKey::MOVEUP);
		if (KeyFocusedItem == InstOptionsViewItemID::NO_FOCUS)
		{
			/*1st key after Page visible */
			if (moveBtnState)
				KeyFocusedItem = InstOptionsViewItemID::INST_MOVE_DOWN;
			else
				KeyFocusedItem = InstOptionsViewItemID::INST_MOVE;
		}
		else
		{
			PannelSendKey(false, param, KeyValue); // clear last Focus
			if (option_handler_focus != INST_OPTION)
				return true;
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				KeyFocusedItem = (KeyFocusedItem + 1 >= InstOptionsViewItemID::FOCUS_MAX ? (InstOptionsViewItemID::FOCUS_MAX) : KeyFocusedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				if (moveBtnState)
					KeyFocusedItem = (KeyFocusedItem - 1 <= InstOptionsViewItemID::NO_FOCUS ? (InstOptionsViewItemID::NO_FOCUS + 1) : KeyFocusedItem - 1);
				else
					KeyFocusedItem = (KeyFocusedItem - 1 <= InstOptionsViewItemID::INST_MOVE_LAST ? (InstOptionsViewItemID::INST_MOVE_LAST + 1) : KeyFocusedItem - 1);
				break;
			}
		}
		if (setfocus)
		{
			PannelSendKey(true, param, KeyValue);
		}
		return true;
	}

	void IgniteInstruments::PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		itemFocus = isFocuse;
		switch (OptionsView[KeyFocusedItem])
		{
		case INST_MOVE_DOWN:
			instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::MOVEDOWN, isFocuse, param, KeyValue);
			break;
		case INST_MOVE_UP:
			instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::MOVEUP, isFocuse, param, KeyValue);
			break;
		case INST_MOVE_TOP:
			instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::MOVETOP, isFocuse, param, KeyValue);
			break;
		case INST_MOVE_LAST:
			instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::MOVEBTM, isFocuse, param, KeyValue);
			break;
		case INST_MOVE:
			instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::MOVE, isFocuse, param, KeyValue);
			break;
		case INST_UNDO:
			instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::UNDO, isFocuse, param, KeyValue);
			break;
		case INST_COPY:
			if(!isFocuse)
				instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::COPY, isFocuse, param, KeyValue);
			else
				this->postCommandMessage(3);
			break;
		case INST_AUTO_SAVE:
			instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::AUTOSAVE, isFocuse, param, KeyValue);
			break;
		default:
			break;
		}
	}

	void IgniteInstruments::handleCommandMessage(int id)
	{
		if (id == 2)
			buttonClicked(&addNew);
		else if (id == 3)
			instOptCtr->PannelKeyHandler(IgniteControlOptions::eKey::COPY, itemFocus, itemParam, itemKeyValue);
		else if (id == 4)
			list[instRowHighLightIndex]->PannelKeyHandler(itemParam, itemKeyValue);
		else if (id == 5)
			onOptCtlButtonClicked(IgniteControlOptions::DEL);
		else if(id ==6 )
			onOptCtlButtonClicked(IgniteControlOptions::SAVE);
	}

	void IgniteInstruments::addCvToUseList(const String& cvPort)
	{
		cvUseList[cvPort] = false;
	}

	void IgniteInstruments::removeCvFromUseList(const String& cvPort)
	{
		auto it = cvUseList.find(cvPort);
		if (it != cvUseList.end())
			cvUseList.erase(it);
	}
	void IgniteInstruments::markAsCvInUse(const String& cvPort)
	{
		auto it = cvUseList.find(cvPort);
		if (it != cvUseList.end())
			it->second = true;
	}
	void IgniteInstruments::markAsCvFree(const String& cvPort)
	{
		auto it = cvUseList.find(cvPort);
		if (it != cvUseList.end())
			it->second = false;
	}

	bool IgniteInstruments::isCvUsed(const String& cvPort)
	{
		auto it = cvUseList.find(cvPort);
		if (it != cvUseList.end())
			return it->second;
		return false;
	}

	void IgniteInstruments::addChannelToUseList(const String& midiport, int channel)
	{
		channelUseList[midiport][channel] = false;
	}

	void IgniteInstruments::markAsChannelInUse(const String& midiport, int channel)
	{
		if (channelUseList.find(midiport) != channelUseList.end() && channel >= 0 && channel < 16)
		{
			channelUseList[midiport][channel] = true;
		}
	}

	void IgniteInstruments::markAsChannelFree(const String& midiport, int channel)
	{
		if (channelUseList.find(midiport) != channelUseList.end() && channel >= 0 && channel < 16)
		{
			channelUseList[midiport][channel] = false;
		}
	}

	bool IgniteInstruments::isChannelUsed(const String& midiport, int channel)
	{
		if (channelUseList.find(midiport) != channelUseList.end() && channel >= 0 && channel < 16)
		{
			if (channelUseList[midiport][channel])
				return true;
			else
				return false;
		}
		return false;
	}

	void IgniteInstruments::addMidiToUseList(const String& midiPort)
	{
		midiUseList[midiPort] = false;
	}

	void IgniteInstruments::markAsMidiInUse(const String& midiPort)
	{
		auto it = midiUseList.find(midiPort);
		if (it != midiUseList.end())
			it->second = true;
	}

	void IgniteInstruments::markAsMidiFree(const String& midiPort)
	{
		auto it = midiUseList.find(midiPort);
		if (it != midiUseList.end())
			it->second = false;
	}

	bool IgniteInstruments::isMidiUsed(const String& midiPort)
	{
		auto it = midiUseList.find(midiPort);
		if (it != midiUseList.end())
			return it->second;
		return false;
	}

	void IgniteInstruments::deleteUnassignedInstruments()
	{
		instrumentsDataUpdated = false;
		if (list.size() > 0)
		{
			if (popupPtr->isDialogOpen.get() != true)
			{
				popupPtr->close();
				popupPtr->setPopUpPropories(" Delete all Unassigned Instruments ", 604, 290);
				popupPtr->setbtnsize(265, 44);
				popupPtr->setInfoText(" Are you sure you want to delete all unassigned instruemts?");
				popupPtr->addImageButton("Cancel", "btn-secondary-cancel-267x44-default.png", "btn-secondary-cancel-267x44-default-hover.png", 2);
				popupPtr->addImageButton("delete", "btn-delete-267x44-default.png", "btn-delete-267x44-default-hover.png", 1);
				popupPtr->popupType = POPUP_WIPEDATA;
				popupPtr->isCloseButtonVisible(false);
				popupPtr->show();
				popupPtr->onSelectionChanged = [this](int r)
				{
					if (r == 1)
					{
						int instrumentIndex = list.size() - 1;

						for (auto i = instrumentIndex; i >= 0; i--)
						{
							if(!isInstrumentUsed(gInstruments[i].name))
							{
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstruments[i].name.toStdString());
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentRemove, gInstruments[i].name.toStdString());

								if (i != selected)
									selected = i;

								auto port = (int)getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT);
								auto channel = (int)getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL);

								if (port >= (int)InstrumentPorts::cvPort1 && port <= (int)InstrumentPorts::cvPort8)
									markAsCvFree(mt::Ignite::portList[port]);
								else if ((port >= (int)InstrumentPorts::midiPort1 && port <= (int)InstrumentPorts::midiPort5) || (port >= (int)InstrumentPorts::usbMidiPort1 && port <= (int)InstrumentPorts::usbMidiPort6))
								{
									markAsMidiFree(mt::Ignite::portList[port]);
									markAsChannelFree(mt::Ignite::portList[port], channel);
								}
							}
						}

						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);

						updateDataFromEngine();
						IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
						if (gInstruments.size() > 0)
						{
							auto selectedInstrIndex = getSelected();

							if (selectedInstrIndex == 0)
								setSelected(selectedInstrIndex);
							else
								setSelected(selectedInstrIndex - 1);

							if (gInstruments.size() == 1)
							{
								enableControlButton(IgniteControlOptions::MOVE, false);
							}
						}
						else
						{
							instrumentsCreated = false;
							emptyInstrComp->setVisible(true);
							addNew.setVisible(true);
							addNewInList.setVisible(false);
							refreshInstrumentsUI();
							instrOptInstance->setVisible(false);
							enableControlButton(IgniteControlOptions::DEL, false);
							enableControlButton(IgniteControlOptions::COPY, false);
							settingsInstance->setWipeInstrumentsBtnState(false);
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
		enableControlButton(IgniteControlOptions::PLAY, false);
		enableControlButton(IgniteControlOptions::REC, false);
	}

	////////////////////////////////////////////////
	//   Class: Ignite Instruments row
	////////////////////////////////////////////////
	IgniteInstrumentsRow::IgniteInstrumentsRow(int SerialNumber, mt::Ignite::instrumentsData& Data, IgniteInstruments* instrumentsPtr) : instrumentsdata(Data)
	{
		_pInstruments = instrumentsPtr;
		srNo.makeVisible(this);
		name.makeVisibleAndAddListner(this);
		serialNumber = String(SerialNumber + 1);

		name.setDisplayTitle(true);
		name.setEditable(false, true, false);
		name.setDisplayTitle(false);
		name.addMouseListener(this, false);

		srNo.setText(String(SerialNumber), juce::dontSendNotification);
		name.setText(String(serialNumber) + ": " + instrumentsdata.name, juce::dontSendNotification);
		name.setJustificationType(Justification::centred);
		name.setLableFont(font_10);

		box.items.add(FlexItem(name).withMargin(3).withFlex(3));

		normalImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-inst-list-default-normal.png"));
		activeImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-inst-list-active-normal.png"));
		getAssets();
		name.onValueChanged = [this](String value)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			if (_pInstruments->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_DISP_NAME) != value)
			{
				_pInstruments->instrumentsDataUpdated = true;
				_pInstruments->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_DISP_NAME, value, false);
				if (!_pInstruments->autoSaveState)
				{
					_pInstruments->enableControlButton(IgniteControlOptions::SAVE, true);
					_pInstruments->enableControlButton(IgniteControlOptions::UNDO, true);
				}
				updateEditIndication();
			}
		};
		name.ValidateDataOnChange = [this](String value)
		{
			Component::BailOutChecker checker(this);
			if (checker.shouldBailOut())
				return KEY_PAD_DATA_VALIDATE_OK;

			if (_pInstruments->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_DISP_NAME) != value)
			{
				//check for duplicate
				for (auto& i : gInstruments)
				{
					if (i.name == value)
						return KEY_PAD_DATA_DUPLICATE_ERROR; // Name already present
				}
			}
			return KEY_PAD_DATA_VALIDATE_OK;
		};
	}

	IgniteInstrumentsRow::~IgniteInstrumentsRow()
	{}

	void IgniteInstrumentsRow::mouseUp(const MouseEvent& event)
	{
		if (rowIsBeingDraggedOver)
		{
			if (_pInstruments->move_enable)
			{
				_pInstruments->moveInstrument(event.position);
			}
			rowIsBeingDraggedOver = false;
		}
		else if (onMouseDown)
			onMouseDown(this);
	}

	void IgniteInstrumentsRow::mouseDrag(const MouseEvent& event)
	{
		juce::DragAndDropContainer* dragC = juce::DragAndDropContainer::findParentDragContainerFor(this);
		if (dragC)
		{
			if (isHighlighted)
			{
				if (!dragC->isDragAndDropActive())
				{
					if (_pInstruments->move_enable)
						dragC->startDragging("TargetSource", this);
					rowIsBeingDraggedOver = true;
				}
			}
		}
	}

	void IgniteInstrumentsRow::highlight(bool State)
	{
		isHighlighted = State;
		repaint();
	}

	int IgniteInstrumentsRow::getSerialNumber()
	{
		return srNo.getText().getIntValue();
	}

	void IgniteInstrumentsRow::mouseDown(const MouseEvent& event)
	{

	}

	void IgniteInstrumentsRow::showInstrumentsView()
	{
		auto pChild = IgniteInstrumentOpt::GetInstrumentInstanceOpt(_pInstruments);
		serialNo = getSerialNumber();
		_pInstruments->curretInstrumentsRow = serialNo;
		mt::Ignite::instrumentsData idata = gInstruments[serialNo];
		pChild->refreshData();
		_pInstruments->changeInstrumentsOptionsVisibility(true);
	}

	void IgniteInstrumentsRow::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		if (buttonThatWasClicked == &options)
		{
			showInstrumentsView();
		}
	}

	void IgniteInstrumentsRow::updateEditIndication()
	{
		String instrNameString = _pInstruments->getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_DISP_NAME);

		instrNameString = serialNumber + ": " + instrNameString;
		if (_pInstruments->instrumentsDataUpdated && !_pInstruments->isGlobalAutosaveEnabled() && !_pInstruments->autoSaveState)
			instrNameString = instrNameString + "*";
		IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
		name.setText(instrNameString, juce::dontSendNotification);
	}

	void IgniteInstrumentsRow::refreshInstrumentsRowUI()
	{
		updateEditIndication();
	}

	void IgniteInstrumentsRow::labelTextChanged(Label* labelThatHasChanged)
	{
		if (labelThatHasChanged == &name)
		{
			_pInstruments->setInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_NAME, name.getText(), false);
			_pInstruments->instrumentsDataUpdated = true;
			updateEditIndication();
		}
	}

	void IgniteInstrumentsRow::getAssets()
	{
		imageActive = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("list-item-active.png"));
		imageDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("list-item-default.png"));
		imageDefaultHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("list-item-default-hover.png"));
		imageActiveHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("list-item-active-hover.png"));
	}

	void IgniteInstrumentsRow::paint(juce::Graphics& g)
	{
		Image Draw = isHighlighted ? isMouseOver ? imageActiveHover : imageActive : isMouseOver ? imageDefaultHover : imageDefault;
		g.drawImage(Draw, getLocalBounds().toFloat(), RectanglePlacement::centred);
	}

	void IgniteInstrumentsRow::resized()
	{
		box.performLayout(getLocalBounds());
	}

	void IgniteInstrumentsRow::setColor(int colourID, Colour colour)
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

	void IgniteInstrumentsRow::SetFocus(bool hasFocus, bool activeHasFocus)
	{
		if (hasFocus)
			isMouseOver = true;
		else
			isMouseOver = false;
		if (activeHasFocus)
			name.setBackgroundColour(juce::Colour(0x2086BF));
		repaint();
	};

	bool IgniteInstrumentsRow::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT || param == (paramId_t)FrontPannelParam::paramKey_NEXT_TIE || param == (paramId_t)FrontPannelParam::paramKey_ROTARYROW)
		{
			onMouseDown(this);
		}
		else if (param == (paramId_t)FrontPannelParam::paramKey_RDBLSELECT)
		{
			name.editLable();
			name.PannelKeyHandler(param, KeyValue);
		}
		return true;
	}

	void IgniteInstruments::onOptCtlButtonClicked(IgniteControlOptions::eKey option)
	{
		int selected_instrument_index;

		if (!instrumentsCreated)
			return;
		if (option == IgniteControlOptions::SAVEALLON)
		{
			instrumentsDataUpdated = false;
			list[selected]->updateEditIndication();
		}
		else if (option == IgniteControlOptions::AUTOSAVEON)
		{
			isAutosave = true;
		}
		else if (option == IgniteControlOptions::AUTOSAVEOFF)
		{
			isAutosave = false;
		}
		if (this->isVisible())
		{
			switch (option) {
			case IgniteControlOptions::SAVE:
			{
				if (globalSettingUpdated)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSave, 0);
					globalSettingUpdated = false;
				}
				if (popupPtr->isDialogOpen.get() != true)
				{
					popupPtr->close();
					popupPtr->setPopUpPropories("Save ", 604, 290);
					popupPtr->setbtnsize(265, 44);
					popupPtr->setInfoText(String(" Save " + gInstruments[selected].name + " edits? "));
					popupPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
					popupPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
					popupPtr->popupType = POPUP_WITHOKCANCEL;
					popupPtr->isCloseButtonVisible(false);
					popupPtr->show();
					popupPtr->onSelectionChanged = [this](int r)
					{
						if (r == 1)
						{
							_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstruments[selected].name.toStdString());
							_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentSave, gInstruments[selected].name.toStdString());
							//Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 150);
							instrumentsDataUpdated = false;
							updateLables();
						}
						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);
					};
				}
			}
			break;
			case IgniteControlOptions::DEL:
				if (gInstruments.size() > 0)
				{
					selected_instrument_index = getSelected();
					if (popupPtr->isDialogOpen.get() != true)
					{
						popupPtr->close();
						popupPtr->setPopUpPropories("Delete " + gInstruments[selected].name, 604, 290);
						popupPtr->setbtnsize(265, 44);
						popupPtr->setInfoText("Are you sure you want to delete " + gInstruments[selected].name);
						popupPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
						popupPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
						popupPtr->popupType = POPUP_WITHOKCANCEL;
						popupPtr->isCloseButtonVisible(false);
						popupPtr->show();
						popupPtr->onSelectionChanged = [this, selected_instrument_index](int r)
						{
							std::vector<IgniteParamList> instrumentUsedByList;
							IgniteParamList params;
							params.param = (paramId_t)InstrumentParam::paramUsedByListCmd;
							params.MsgValue = gInstruments[selected].name.toStdString();
							instrumentUsedByList.push_back(params);
							instrumentUsedByList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentUsedByList);

							if (r == 1)
							{
								if (((instrumentUsedByList.size() - 1) > 0) && (IgniteTrack::GetAppTrackInstance()->trackCreated))
								{
									String track_msg;
									((instrumentUsedByList.size() - 1) > 1) ? (track_msg = "Tracks") : (track_msg = "Track");
									_pInfoWindow->setTextandShow("Delete operation not possible as selected Instrument is Used by " + track_msg + " !!!");
									//warnPopupPtr->close();
									//warnPopupPtr->setWarningText("Delete operation not possible as selected Instrument is Used by " /*+ String(gPatternRef.enginePatternData.usedByList.size()) + " " */ + track_msg + " !!!");
									//warnPopupPtr->close();
									//warnPopupPtr->show();
									//warnPopupPtr->onSelectionChanged = [this](int r)
									//{
									//	if (auto dw = getCurrentlyModalComponent())
									//		dw->exitModalState(0);
									//	DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Closing delete pop since there is a warning in inst page");
									//	if (auto dw = getCurrentlyModalComponent())
									//		dw->exitModalState(0);
									//};

									return;
								}
								auto selected_instr_index = getSelected();
								auto port = (int)getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_PORT);
								auto channel = (int)getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_CHANNEL);

								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstruments[selected].name.toStdString());
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentRemove, gInstruments[selected].name.toStdString());

								if (port >= (int)InstrumentPorts::cvPort1 && port <= (int)InstrumentPorts::cvPort8)
									markAsCvFree(mt::Ignite::portList[port]);
								else if ((port >= (int)InstrumentPorts::midiPort1 && port <= (int)InstrumentPorts::midiPort5) || (port >= (int)InstrumentPorts::usbMidiPort1 && port <= (int)InstrumentPorts::usbMidiPort6))
								{
									markAsMidiFree(mt::Ignite::portList[port]);
									markAsChannelFree(mt::Ignite::portList[port], channel);
								}

								if (auto dw = getCurrentlyModalComponent())
									dw->exitModalState(0);

								updateDataFromEngine();
								IgniteTrack::GetAppTrackInstance()->setInstrDataDirtyFlag(true);
								if (gInstruments.size() > 0)
								{
									if (selected_instr_index == 0)
										setSelected(selected_instr_index);
									else
										setSelected(selected_instrument_index - 1);

									if (gInstruments.size() == 1)
									{
										enableControlButton(IgniteControlOptions::MOVE, false);
									}
								}
								else
								{
									instrumentsCreated = false;
									emptyInstrComp->setVisible(true);
									addNew.setVisible(true);
									addNewInList.setVisible(false);
									refreshInstrumentsUI();
									instrOptInstance->setVisible(false);
									enableControlButton(IgniteControlOptions::DEL, false);
									enableControlButton(IgniteControlOptions::COPY, false);
									settingsInstance->setWipeInstrumentsBtnState(false);
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
				break;
			case IgniteControlOptions::UNDO:
				if (instrumentsDataUpdated)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstruments[selected].name.toStdString());
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramUndoCmd, gInstruments[selected].name.toStdString());
					Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 50);
					updateLables();
					setSelected(selected);
				}
				break;
			case IgniteControlOptions::COPY:
				instrumentCopyComp->setLabelText(getInstrumentData(mt::Ignite::instrumentsDataIndex::INSTR_NAME).toString());
				instrumentCopyComp->setVisible(true);
				instrumentCopyComp->runModalLoop();
				break;
			case IgniteControlOptions::MOVE:
				appOptPtr->optionMove.getToggleState() ? move_enable = true : move_enable = false;
				move_enable ? view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all) : view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::never);
				appOptPtr->enableMoveoptions(mt::Ignite::tabs::MENU, 2, move_enable);
				if (move_enable)
					MenuMode::GetMenuModeInstance()->setJustification(Justification::left);
				else
					MenuMode::GetMenuModeInstance()->setJustification(Justification::centred);
				break;
			case IgniteControlOptions::MOVETOP:
				moveInstrument(0);
				break;
			case IgniteControlOptions::MOVEBTM:
				moveInstrument(list.size());
				break;
			case IgniteControlOptions::MOVEDOWN:
				if ((getSelected() + 1) < list.size())
					moveInstrument(getSelected() + 1);
				break;
			case IgniteControlOptions::MOVEUP:
				if ((getSelected() - 1) >= 0)
					moveInstrument(getSelected() - 1);
				break;
			case IgniteControlOptions::AUTOSAVE:
				(autoSaveState) ? (autoSaveState = false) : (autoSaveState = true);

				if (instrumentsDataUpdated && autoSaveState)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstruments[selected].name.toStdString());
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramInstrumentSave, gInstruments[selected].name.toStdString());
					updateLables();
				}
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::SettingInstrumentAutoSaveParam, autoSaveState);
				enableControlButton(IgniteControlOptions::SAVE, false);
				globalSettingUpdated = false;
				break;
			}
		}
	}

	void IgniteInstrumentsRow::mouseMove(const MouseEvent& event)
	{
		isMouseOver = true;
		repaint();
	}

	void IgniteInstrumentsRow::mouseExit(const MouseEvent& event)
	{
		isMouseOver = false;
		repaint();
	}
};
