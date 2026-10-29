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
	IgniteTrackValues::IgniteTrackValues()
	{
		back.makeVisibleAndAddListner(this);
		back.setColour(juce::TextButton::buttonColourId, Theme::Instance().getColour(neutralGrey1));

		trackMainEnvlopBox.flexDirection = FlexBox::Direction::column;
		trackMainEnvlopBox.items.add(FlexItem(sliderContainerBox1).withFlex(1).withMargin(5));
		trackMainEnvlopBox.items.add(FlexItem(sliderContainerBox).withFlex(1));

		header.makeVisible(this);
		header.setDisplayTitle(false);
		header.setEditable(false, false, false);
		header.setJustificationType(Justification::centred);

		for (int i = 0; i < MAX_TRACK_VALUE / 2; i++)
		{
			slider[i].setTitle(intSoundDefaultknobLable[i]);
			addAndMakeVisible(slider[i]);
			slider[i].showTitleAndValue(true, Slider::TextBoxBelow, Justification::centred);
			slider[i].setDisplayProportions(.6f, .2f, .2f);
			slider[i].addMouseListener(this, true);
			slider[i].addListener(this);
			slider[i].setLookAndFeel(CommonRotaryCv_gate::getLnf());
			slider[i].setValueColour(ui::Theme::Instance().getColour(neutralWhite));
			slider[i].setTitleColour(ui::Theme::Instance().getColour(neutralWhite));
			sliderContainerBox1.items.add(FlexItem(slider[i]).withFlex(1).withMaxHeight(160).withMaxWidth(200));
		}

		for (int i = 4; i < MAX_TRACK_VALUE; i++)
		{
			slider[i].setTitle(intSoundDefaultknobLable[i]);
			addAndMakeVisible(slider[i]);
			slider[i].showTitleAndValue(true, Slider::TextBoxBelow, Justification::centred);
			slider[i].setDisplayProportions(.6f, .2f, .2f);
			slider[i].addMouseListener(this, true);
			slider[i].addListener(this);
			slider[i].setLookAndFeel(CommonRotaryCv_gate::getLnf());
			slider[i].setValueColour(ui::Theme::Instance().getColour(neutralWhite));
			slider[i].setTitleColour(ui::Theme::Instance().getColour(neutralWhite));
			sliderContainerBox.items.add(FlexItem(slider[i]).withFlex(1).withMaxHeight(160).withMaxWidth(200));
		}
		setKnobIndex();
	}

	IgniteTrackValues::~IgniteTrackValues()
	{
	}

	void IgniteTrackValues::setDefaultKnobTitle()
	{
		for (int i = 0; i < MAX_TRACK_VALUE; i++)
		{
			slider[i].setTitle(soundStatus ? intSoundDefaultknobLable[i] : extSoundDefaultknobLable[i]);
			slider[i].disableTitleEdit(!soundStatus);
		}
	}


	void IgniteTrackValues::setInstSoundStatus(bool s)
	{
		soundStatus = s; setDefaultKnobTitle();
		s ? header.setText("Track Internal FX", dontSendNotification) : header.setText("Track External CC", dontSendNotification);
	}


	void IgniteTrackValues::paint(juce::Graphics& g)
	{
		Image Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("bg-title.png"));
		g.drawImage(Draw, ui::Rectangle<float>{ 10, 10, 1004, 48 });
	}

	void IgniteTrackValues::resized()
	{
		back.setBounds(10, 10, 44, 44);
		header.setBounds(10, 15, 800, 36);
		sliderContainerBox.justifyContent = FlexBox::JustifyContent::center;
		sliderContainerBox1.justifyContent = FlexBox::JustifyContent::center;
		trackMainEnvlopBox.performLayout(ui::Rectangle(xStartTrackValueFlexBox, yStartTrackValueFlexBox + 5, widthTrackValueFlexBox, 415));
		back.toFront(true);
	}

	void IgniteTrackValues::getDataHolder()
	{
		String s = (IgniteTrack::GetAppTrackInstance()->getTrackData(mt::Ignite::trackDataIndex::TRACK_OUT_INSTR));
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, s.toStdString());
		std::vector<IgniteParamList> Params;
		IgniteParamList params;

		Params.clear();
		params.param = (paramId_t)InstrumentParam::paramCountTrackValueCmd;
		Params.push_back(params);
		Params = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, Params);

		auto tmp = activeKnob;
		for (auto knob_index = 0; knob_index < Params[0].MsgValue.get< int8>() / 3; knob_index++)
		{
			getdatafromEngine(knob_index);
			slider[knob_index].setFloatValue(activeKnobdata.currentValue, dontSendNotification);
			if (!soundStatus)
				slider[knob_index].setTitle(activeKnobdata.name);
		}
		activeKnob = tmp;
		setTrackValueData(mt::Ignite::TrackValueDataIndex::Active_Knob, activeKnob);
	}

	void IgniteTrackValues::setKnobIndex()
	{
		String a = "A";
		for (int i = 0; i < MAX_TRACK_VALUE; i++)
			slider[i].setKnobTitle(a.charToString('A' + i), true);
	}

	void IgniteTrackValues::getdatafromEngine(int knob)
	{
		std::vector<IgniteParamList> Params;
		IgniteParamList params;

		auto tmp_knob = (knob + (MAX_TRACK_VALUE * (1 + altFunEnabled)));

		/* Same Knob value to be used for Resonance, Volume& Release */
		if (tmp_knob == 16)
			tmp_knob = 8;
		else if (tmp_knob == 18)
			tmp_knob = 10;

		activeKnobdata.slot = knob;
		setTrackValueData(mt::Ignite::TrackValueDataIndex::Active_Knob, soundStatus ? tmp_knob : knob);

		Params.clear();
		params.param = (paramId_t)InstrumentParam::paramTVType;
		Params.push_back(params);
		Params = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, Params);
		activeKnobdata.trackValueType = Params[0].MsgValue.get<uint8>();

		//TODO - verify once for internal(main/alternate) and external
		if (activeKnobdata.trackValueType == static_cast<int>(TrackValueType::TVTypeMidiCC) || soundStatus)
		{
			Params.clear();
			params.param = (paramId_t)InstrumentParam::paramTVMidiCCNum;
			Params.push_back(params);
			params.param = (paramId_t)InstrumentParam::paramTVMidiCCMinValue;
			Params.push_back(params);
			params.param = (paramId_t)InstrumentParam::paramTVMidiCCMaxValue;
			Params.push_back(params);
			params.param = (paramId_t)InstrumentParam::paramTVMidiCCValue;
			Params.push_back(params);
			params.param = (paramId_t)InstrumentParam::paramTVMidiCCInitValue;
			Params.push_back(params);
			params.param = (paramId_t)InstrumentParam::paramTVName;
			Params.push_back(params);
			Params = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, Params);

			activeKnobdata.trackValueSelected = Params[0].MsgValue.get<int8>();
			activeKnobdata.minValue = Params[1].MsgValue.get<float>();
			activeKnobdata.maxValue = Params[2].MsgValue.get<float>();
			activeKnobdata.currentValue = Params[3].MsgValue.get<float>();
			activeKnobdata.initValue = Params[4].MsgValue.get<int8>();
			activeKnobdata.name = Params[5].MsgValue.get<std::string>();

		}
		else if (activeKnobdata.trackValueType == static_cast<int>(TrackValueType::TVTypePgmChange))
		{
			Params.clear();
			params.param = (paramId_t)InstrumentParam::paramTVMidiPgmBankLsb;
			Params.push_back(params);
			params.param = (paramId_t)InstrumentParam::paramTVMidiPgmBankMsb;
			Params.push_back(params);
			params.param = (paramId_t)InstrumentParam::paramTVMidiPgmNum;
			Params.push_back(params);
			params.param = (paramId_t)InstrumentParam::paramTVName;
			Params.push_back(params);
			Params = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, Params);

			activeKnobdata.minValue = 0;
			activeKnobdata.maxValue = 127;
			activeKnobdata.lsbValue = Params[0].MsgValue.get<int8>();
			activeKnobdata.msbValue = Params[1].MsgValue.get<int8>();
			activeKnobdata.currentValue = activeKnobdata.initValue = Params[2].MsgValue.get<int8>();
			activeKnobdata.name = Params[3].MsgValue.get<std::string>();

			if (activeKnobdata.lsbValue < 0)
				activeKnobdata.lsbValue = 0;

			if (activeKnobdata.msbValue < 0)
				activeKnobdata.msbValue = 0;

			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiPgmBankLsb, activeKnobdata.lsbValue);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiPgmBankMsb, activeKnobdata.msbValue);
		}
		auto ledcnt= ((15 * (activeKnobdata.currentValue-slider[activeKnobdata.slot].getMin())) / (slider[activeKnobdata.slot].getMax() - slider[activeKnobdata.slot].getMin()));
		
		if (activeKnobdata.currentValue == slider[activeKnobdata.slot].getMin())
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_RELEASE + knob), (uint16_t)0, false);
			DBG("LED COUNT MIN:" + String(ledcnt) + ":" + String(activeKnobdata.currentValue));
		}
		else if (ledcnt >= 0 && ledcnt <= 15)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_RELEASE + knob), (uint16_t)ledcnt + 1, false);
			DBG("LED COUNT:" + String(ledcnt)+":"+String(activeKnobdata.currentValue));
		}
	
		if (activeKnobdata.currentValue < 0)
		{
			activeKnobdata.currentValue = 0;
			
			//Don't set CC param values, if knob is set to Program Change
			if (activeKnobdata.trackValueType == static_cast<int>(TrackValueType::TVTypeMidiCC))
			{
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCValue, activeKnobdata.currentValue);
			}
		}

	}

	void IgniteTrackValues::setSliderEditCtl(bool state)
	{
		for (auto& knob : slider)
			knob.setEditCtl(state);
	}

	void IgniteTrackValues::updateSlider(mt::Ignite::TrackValues* p)
	{
		soundStatus ? (activeKnobdata.slot = p->slot + (MAX_TRACK_VALUE * (1 + altFunEnabled))) : activeKnobdata.slot = p->slot;
		setTrackValueData(mt::Ignite::TrackValueDataIndex::Active_Knob, activeKnobdata.slot);
		setTrackValueData(mt::Ignite::TrackValueDataIndex::TVType, p->trackValueType);

		if (p->trackValueType == 1)
		{
			setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCNum, p->trackValueSelected);
			setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCInitValue, p->initValue);
			setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCMinValue, p->minValue);
			setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCMaxValue, p->maxValue);
			setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, p->initValue);
			if (!soundStatus)
				slider[activeKnobdata.slot].setTitle("CC #" + String(p->trackValueSelected));
			setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCName, ("CC #" + String(p->trackValueSelected)).toStdString());
		}
		else if (p->trackValueType == 2)
		{
			setTrackValueData(mt::Ignite::TrackValueDataIndex::SeqMidiPgmBankLsb, p->lsbValue);
			setTrackValueData(mt::Ignite::TrackValueDataIndex::SeqTVMidiPgmBankMsb, p->msbValue);
			setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCName, "Program Change");
			if (!soundStatus)
				slider[activeKnobdata.slot].setTitle("Program Change");
		}

		slider[activeKnobdata.slot].setFloatValue(p->initValue, NotificationType::sendNotification);
		IgniteControlOptions::GetAppOptInstance()->enableKey(IgniteControlOptions::eKey::SAVE, true);
	}

	void IgniteTrackValues::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		if (buttonThatWasClicked == &back)
			IgniteTrack::GetAppTrackInstance()->changeTrackOptionsVisibility(IgniteTrack::trackScreenIndex::OPTION_WINDOW);
	}

	void IgniteTrackValues::setAlternativeFunc()
	{
		if (soundStatus)
		{
			slider[0].setUnit("ms ", true);
			slider[2].setUnit("%", true);
			slider[3].setUnit("%", true);
			slider[4].setUnit("%", true);
			slider[0].setRange(0, 1000, 10);
			slider[2].setRange(0, 100, 1);
			slider[3].setRange(0, 100, 1);
			slider[4].setRange(0, 100, 1);
			slider[5].setRange(1, 100, 1);
			if (altFunEnabled)
			{
				slider[1].setUnit("db/oct", true);
				slider[1].changeDisplayName(FILTER_MODE);
				slider[1].setRange(12, 24, 12);
				slider[3].changeDisplayName(VOLUME_ALT);
				slider[4].setUnit("%", true);
				slider[4].changeDisplayName(REVERB_DRY);
				slider[5].setUnit("%", true);
				slider[5].changeDisplayName(CHORUS_DEPTH);
				slider[6].setUnit("dB", true);
				slider[6].changeDisplayName(EQ_GAIN);
				slider[6].setRange(-12, 12, 0.5f);
				slider[6].setNumDecimalPlacesToDisplay(3);
				slider[7].changeDisplayName(EQ_GAIN);
				slider[7].setUnit("dB", true);
				slider[7].setRange(-12, 12, 0.5f);
				slider[7].setNumDecimalPlacesToDisplay(3);
			}
			else
			{
				slider[1].changeDisplayName(FILTER_CUTOFF);
				slider[1].setUnit("Hz", true);
				slider[1].setRange(20, 20000, 200.0f);
				slider[3].changeDisplayName(VOLUME_MASTER);
				slider[4].changeDisplayName(REVERB_WIDTH);
				slider[5].changeDisplayName(CHORUS_RATE);
				slider[5].setUnit("Hz", true);
				slider[6].changeDisplayName(EQ_FREQ);
				slider[6].setUnit("Hz", true);
				slider[6].setRange(8000, 20000, 100);
				slider[7].changeDisplayName(EQ_FREQ);
				slider[7].setUnit("Hz", true);
				slider[7].setRange(20, 5000, 50);
			}
		}
		else
		{
			slider[0].setRange(0, 127, 1);
			slider[1].setRange(0, 127, 1);
			slider[2].setRange(0, 127, 1);
			slider[3].setRange(0, 127, 1);
			slider[4].setRange(0, 127, 1);
			slider[5].setRange(0, 127, 1);
			slider[6].setRange(0, 127, 1);
			slider[7].setRange(0, 127, 1);
			slider[0].changeDisplayName("");
			slider[1].changeDisplayName("");
			slider[2].changeDisplayName("");
			slider[3].changeDisplayName("");
			slider[4].changeDisplayName("");
			slider[5].changeDisplayName("");
			slider[6].changeDisplayName("");
			slider[7].changeDisplayName("");
			slider[0].setUnit("", false);
			slider[1].setUnit("", false);
			slider[2].setUnit("", false);
			slider[3].setUnit("", false);
			slider[4].setUnit("", false);
			slider[5].setUnit("", false);
			slider[6].setUnit("", false);
			slider[7].setUnit("", false);
		}
	}

	void IgniteTrackValues::onbuttonClicked(common_rotaryslider* ptr, bool value)
	{
		if (value)
		{
			slider[getKnobIndex(ptr->getKnob())].getSlider().setEnabled(true);
			slider[getKnobIndex(ptr->getKnob())].setAlpha(1.0f);
			slider[getKnobIndex(ptr->getKnob())].setValueVisible(true);
			if (!soundStatus)
				slider[getKnobIndex(ptr->getKnob())].disableTitleEdit(true);
		}
		else
		{
			slider[getKnobIndex(ptr->getKnob())].getSlider().setEnabled(false);
			slider[getKnobIndex(ptr->getKnob())].setAlpha(0.5f);
			slider[getKnobIndex(ptr->getKnob())].setValueVisible(true);
			if (!soundStatus)
				slider[getKnobIndex(ptr->getKnob())].disableTitleEdit(false);
		}
		setTrackValueData(mt::Ignite::TrackValueDataIndex::Active_Knob, getKnobIndex(ptr->getKnob()));
		setTrackValueData(mt::Ignite::TrackValueDataIndex::EffectState, value);
	}

	void IgniteTrackValues::onDoubleClicked(common_rotaryslider* ptr)
	{
		valueSelectionPtr = TrackValueSelection::GetAppTrackValueSelectionInstance();
		getdatafromEngine(getKnobIndex(ptr->getKnob()));

		activeKnobdata.internalsound = soundStatus;
		valueSelectionPtr->updateRotaryData(activeKnobdata);
		IgniteTrack::GetAppTrackInstance()->trackValueSelectPtr->setVisible(true);
		IgniteTrack::GetAppTrackInstance()->trackValueSelectPtr->runModalLoop();
	}

	void IgniteTrackValues::updateTrackValueScreen()
	{
		setAlternativeFunc();
		getDataHolder();
		String s = (IgniteTrack::GetAppTrackInstance()->getTrackData(mt::Ignite::trackDataIndex::TRACK_OUT_INSTR));
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, s.toStdString());
		IgniteTrack::GetAppTrackInstance()->changeTrackOptionsVisibility(IgniteTrack::trackScreenIndex::TRACKVALUE_WINOW);
	}

	void IgniteTrackValues::onRotaryValueChange(common_rotaryslider* ptr, double Value)
	{
		getdatafromEngine(getKnobIndex(ptr->getKnob()));
		slider[activeKnobdata.slot].setValueVisible(true);
		if (!soundStatus)
		{
			if (activeKnobdata.trackValueType == 1)
				setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, (int)Value);
			else
				setTrackValueData(mt::Ignite::TrackValueDataIndex::SeqMidiPgmNum, (int)Value);
		}
		else
		{
			int x = 0;
			for (; x < MAX_TRACK_VALUE; x++)
				if (ptr == &slider[x])
					break;
			setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, (float)Value);

			if (!altFunEnabled)
			{
				if ((activeKnobdata.name == "Cutoff"))
				{

					auto minDelta = abs(FilterFreqMap[0] - Value);
					auto nearestIndex = 0;
					float cValue = 0;
					if (isFromFp != -1)
					{
						int i = 0;
						for (; i < 128 ; i++)
							if (FilterFreqMap[i] == activeKnobdata.currentValue)
								break;
						if (isFromFp && i < 127)
						{
							cValue = FilterFreqMap[i + 1];
						}
						else if (i >= 1 && isFromFp == 0)
						{
							cValue = FilterFreqMap[i - 1];
						}
						else
							cValue = activeKnobdata.currentValue;
						if (cValue != activeKnobdata.currentValue)
						{
							setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, cValue);
							slider[activeKnobdata.slot].setFloatValue((double)cValue, dontSendNotification);
						}
						isFromFp = -1;
						return;
					}


					for (auto j = 0; j < 128; j++) {
						auto delta = abs(FilterFreqMap[j] - Value);
						if (delta < minDelta) {
							minDelta = delta;
							nearestIndex = j;
						}
					}
					setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, FilterFreqMap[nearestIndex]);
					slider[activeKnobdata.slot].setFloatValue((double)FilterFreqMap[nearestIndex], dontSendNotification);
				}
			}


			
		}
		
		IgniteControlOptions::GetAppOptInstance()->enableKey(IgniteControlOptions::eKey::SAVE, true);
	}

	void IgniteTrackValues::onTitleChange(common_rotaryslider* ptr, String name)
	{
		std::vector<IgniteParamList> Params;
		IgniteParamList param;
		param.param = (paramId_t)InstrumentParam::paramTVType;
		Params.push_back(param);

		param.param = (paramId_t)InstrumentParam::paramTVMidiCCNum;
		Params.push_back(param);

		setTrackValueData(mt::Ignite::TrackValueDataIndex::Active_Knob, soundStatus ? (getKnobIndex(ptr->getKnob()) + MAX_TRACK_VALUE) : getKnobIndex(ptr->getKnob()));
		Params = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, Params);

		if (Params[0].MsgValue.get<uint8>() == 1)
			ptr->setTitle(name + "(CC#" + String(Params[1].MsgValue.get<uint8>()) + ")");
		else
			ptr->setTitle(name + "(PC)");

		setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCName, name.toStdString());
	}

	void IgniteTrackValues::setTrackValueData(mt::Ignite::TrackValueDataIndex item_id, int data)
	{
		switch (item_id)
		{
		case mt::Ignite::TrackValueDataIndex::AddTrackValueCmd:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramAddTrackValueCmd, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::Active_Knob:
			activeKnob = (int8_t)data;
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVActiveKnobCmd, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::TVType:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVType, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::MidiCCNum:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCNum, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::MidiCCName:
			break;
		case mt::Ignite::TrackValueDataIndex::MidiCCInitValue:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCInitValue, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::MidiCCMinValue:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCMinValue, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::MidiCCMaxValue:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCMaxValue, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqMidiPgmNum:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiPgmNum, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqQuantPercent:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqQuantPercent, (int16_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqVelocityPercent:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqVelocityPercent, (int16_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqVelocityRef:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqVelocityRef, (int16_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqLengthPercent:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqLengthPercent, (int16_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqNotePercent:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqNotePercent, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqNoteRef:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqNoteRef, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqTransposeSemiTone:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqTransposeSemiTone, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqTransposeOctave:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqTransposeOctave, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqFtsScale:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqFtsScale, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqFtsRootNote:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqFtsRootNote, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqReichValue:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVSeqReichValue, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqMidiPgmBankLsb:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiPgmBankLsb, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::SeqTVMidiPgmBankMsb:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiPgmBankMsb, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::INSTRUMENT:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, (int8_t)data);
			break;
		case mt::Ignite::TrackValueDataIndex::EffectState:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTrackEffectMuteUnmute, (bool)data);
			break;
		case mt::Ignite::TrackValueDataIndex::MidiCCValue:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCValue, (float)data);
			break;
		case mt::Ignite::TrackValueDataIndex::ALL:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVActiveKnobCmd, (int8_t)data);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCValue, data);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCInitValue, (int8_t)data);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCMinValue, (int8_t)data);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCMaxValue, (int8_t)data);
			break;
		default: break;
		}
	}

	void IgniteTrackValues::setTrackValueData(mt::Ignite::TrackValueDataIndex item_id, float data)
	{
		switch (item_id)
		{
		case mt::Ignite::TrackValueDataIndex::MidiCCValue:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVMidiCCValue, data);
			break;
		default: break;
		}
	}

	void IgniteTrackValues::setTrackValueData(mt::Ignite::TrackValueDataIndex item_id, std::string data)
	{
		switch (item_id)
		{
		case mt::Ignite::TrackValueDataIndex::MidiCCName:
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramTVName, (std::string)data);
			break;
		default: break;
		}
	}

	int IgniteTrackValues::getKnobIndex(String knob)
	{
		juce_wchar a = knob[0];
		for (int i = 0; i < MAX_TRACK_VALUE; i++)
			if (a == (juce_wchar)('A' + i))
				return i;
		return 0;
	}


	TrackValueSelection::TrackValueSelection()
	{
		initLable.setJustificationType(juce::Justification::left);
		initLable.setEditable(false, false, false);
		initLable.makeVisible(this);

		maxLable.setJustificationType(juce::Justification::left);
		maxLable.setEditable(false, false, false);
		maxLable.makeVisible(this);

		BlsbLable.setJustificationType(juce::Justification::left);
		BlsbLable.setEditable(false, false, false);
		BlsbLable.makeVisible(this);

		BmsbLable.setJustificationType(juce::Justification::left);
		BmsbLable.setEditable(false, false, false);
		BmsbLable.makeVisible(this);

		knobLabel.setFont(juce::Font(font_12, juce::Font::plain).withTypefaceStyle("Regular"));
		knobLabel.setJustificationType(juce::Justification::centred);
		knobLabel.setEditable(false, false, false);
		knobLabel.setColour(juce::Label::backgroundColourId, juce::Colour(ui::CommonColorID::customYellow1));
		knobLabel.setColour(juce::Label::outlineColourId, Colours::white);
		knobLabel.makeVisible(this);

		Midicc.makeVisibleAndAddListner(this);
		PgmCtr.makeVisibleAndAddListner(this);
		Midicc.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		PgmCtr.setLookAndFeel(ImageButtonLookandFeel::getLnf());
		Midicc.sethoverImages("btn-midicc-default-hover.png", "btn-midicc-active-hover.png", "");
		PgmCtr.sethoverImages("btn-progchange-default-hover.png", "btn-progchange-active-hover.png", "");

		addAndMakeVisible(view);
		addAndMakeVisible(container);

		view.setViewedComponent(&container, true);
		view.setScrollOnDragMode(Viewport::ScrollOnDragMode::all);

		ctlApply.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		ctlCancel.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		ctlclear.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());

		addAndMakeVisible(valueBox);
		valueBox.updateListOptions(valueList);
		valueBox.setDisplayFormat(CommonListBox::BoxType::withoutTitleBar);
		valueBox.setSelected(0);

		addAndMakeVisible(knobBox);
		knobBox.updateListOptions(knobValues);
		knobBox.setDisplayFormat(CommonListBox::BoxType::withTitleBar);
		knobBox.setSelected(0);
		knobBox.setInterceptsMouseClicks(false, false);

		addAndMakeVisible(midiBox);
		midiBox.updateListOptions(MidiCcValues);
		midiBox.setDisplayFormat(CommonListBox::BoxType::withTitleBar);
		midiBox.setSelected((int)MidiCcValues.size() - 1);
		midiBox.setEnabled(false);
		midiBox.setInterceptsMouseClicks(false, false);

		addAndMakeVisible(ctlApply);
		ctlApply.addListener(this);
		ctlApply.setColour(juce::TextButton::buttonColourId, juce::Colours::black);

		addAndMakeVisible(ctlCancel);;
		ctlCancel.addListener(this);
		ctlCancel.setColour(juce::TextButton::buttonColourId, juce::Colours::black);

		addAndMakeVisible(ctlclear);
		ctlclear.addListener(this);
		ctlclear.setColour(juce::TextButton::buttonColourId, juce::Colours::black);


		ctlBox.items.add(FlexItem(ctlCancel).withMargin({ 0,10,0,0 }).withFlex(1));
		ctlBox.items.add(FlexItem(ctlclear).withMargin({ 0,10,0,0 }).withFlex(1));
		ctlBox.items.add(FlexItem(ctlApply).withMargin(0).withFlex(1));

		initValue.makeVisible(this);
		initValue.setDisplayTitle(false);
		initValue.setJustificationType(juce::Justification::centred);
		initValue.setVisible(true);
		initValue.setcustombounds(true, ui::Rectangle{ 185,0,42, 44 }, ui::Rectangle{ 0,0,0, 0 });

		maxValue.makeVisible(this);
		maxValue.setDisplayTitle(false);
		maxValue.setJustificationType(juce::Justification::centredLeft);
		maxValue.setVisible(true);
		maxValue.setcustombounds(true, ui::Rectangle{ 175,0,42, 44 }, ui::Rectangle{ 0,0,0, 0 });
		maxValue.setValue(127.0f);

		bankLSB.makeVisible(this);
		bankLSB.setDisplayTitle(false);
		bankLSB.setJustificationType(juce::Justification::centredRight);
		bankLSB.setVisible(false);
		bankLSB.setcustombounds(true, ui::Rectangle{ 175,0,42, 44 }, ui::Rectangle{ 0,0,0, 0 });

		bankMSB.makeVisible(this);
		bankMSB.setDisplayTitle(false);
		bankMSB.setJustificationType(juce::Justification::centredRight);
		bankMSB.setVisible(false);
		bankMSB.setcustombounds(true, ui::Rectangle{ 175,0,42, 44 }, ui::Rectangle{ 0,0,0, 0 });

		valuePtr = IgniteTrackValues::GetAppTrackValueInstance();

		setSize(widthLenDia, heightLenDia);
		midiBox.setVisible(false);
		selection_state = 0;

		warnPopupPtr = CommonWarningPopUp::getCommonWarningPopupInstance();
		optPtr = IgniteControlOptions::GetAppOptInstance();

		knobBox.onSelectionChanged = [this](int value, const juce::String& name)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;
				presentTrackValues.slot = knobBox.getSelectedIndex();
			};

		initValue.onValueChanged = [this](int value)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;
				presentTrackValues.initValue = (int8)value;
			};

		maxValue.onValueChanged = [this](int value)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;
				presentTrackValues.maxValue = (int8)value;
			};

		bankLSB.onValueChanged = [this](int value)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;
				presentTrackValues.lsbValue = (int8)value;
			};

		bankMSB.onValueChanged = [this](int value)
			{
				Component::BailOutChecker checker(this);

				if (checker.shouldBailOut())
					return;
				presentTrackValues.msbValue = (int8)value;
			};

		initValue.onPlayTimeValueChanged = [this](int value)
			{
				auto opMode = optPtr->getOperationMode();
				if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
				{
					Component::BailOutChecker checker(this);

					if (checker.shouldBailOut())
						return;
					presentTrackValues.initValue = (int8)value;
				}
			};

		maxValue.onPlayTimeValueChanged = [this](int value)
			{
				auto opMode = optPtr->getOperationMode();
				if ((opMode == mt::Ignite::operatioMode::PLAY_MODE) || (opMode == mt::Ignite::operatioMode::PLAY_REC_MODE))
				{
					Component::BailOutChecker checker(this);

					if (checker.shouldBailOut())
						return;
					presentTrackValues.maxValue = (int8)value;
				}
			};

		for (int i = 0; i < MidiCcValues.size(); i++)
		{
			auto option = new CommonToggleButton(MidiCcValues[i]);
			options.add(option);
			option->setRadioGroupId(55, NotificationType::dontSendNotification);
			option->makeVisibleAndAddListner(this);
			option->setLookAndFeel(CommonToggleButtonLooknFeel::getLnf());
			container.addAndMakeVisible(option);
		}

		initValue.onFocusChanged = [this](bool a)
			{
				repaint();
			};

		maxValue.onFocusChanged = [this](bool a)
			{
				repaint();
			};

		bankLSB.onFocusChanged = [this](bool a)
			{
				repaint();
			};

		bankMSB.onFocusChanged = [this](bool a)
			{
				repaint();
			};
	}

	void TrackValueSelection::updateRotaryData(mt::Ignite::TrackValues data)
	{
		String a;
		knobLabel.setText("Knob " + a.charToString('A' + data.slot), dontSendNotification);
		if (!data.internalsound)
		{
			Midicc.setToggleState(true, dontSendNotification);
			PgmCtr.setEnabled(true);
			if (data.trackValueType == 1)
			{
				Midicc.setToggleState(true, sendNotification);
				PgmCtr.setToggleState(false, dontSendNotification);
				options.getReference(data.trackValueSelected)->setToggleState(true, dontSendNotification);
			}
			else
			{
				Midicc.setToggleState(false, dontSendNotification);
				PgmCtr.setToggleState(true, sendNotification);
				options.getFirst()->setToggleState(true, dontSendNotification);
				options.getFirst()->setToggleState(false, dontSendNotification);
			}

			initValue.setValue(data.initValue);
			maxValue.setValue(data.maxValue);

			presentTrackValues.initValue = data.initValue;
			presentTrackValues.maxValue = data.maxValue;
			presentTrackValues.minValue = data.minValue;
			presentTrackValues.trackValueType = data.trackValueType;
			presentTrackValues.trackValueSelected = data.trackValueSelected;

			if (data.trackValueType == 1)
			{
				maxValue.setVisible(true);
				initValue.setVisible(true);
				bankMSB.setVisible(false);
				bankLSB.setVisible(false);
				minValue.setValue(data.minValue);
				maxValue.setValue(data.maxValue);
				initValue.setValue(data.initValue);
				midiBox.setVisible(true);
			}
			else if (data.trackValueType == 2)
			{
				bankLSB.setVisible(true);
				bankMSB.setVisible(true);
				maxValue.setVisible(false);
				initValue.setVisible(false);
				midiBox.setVisible(false);
				initValue.setValue(data.initValue);
			}
			presentTrackValues = data;
		}
		else
		{
			options.getFirst()->setToggleState(true, dontSendNotification);
			options.getFirst()->setToggleState(false, dontSendNotification);
			maxValue.setVisible(true);
			initValue.setVisible(true);
			initValue.setValue(data.initValue);
			minValue.setValue(data.minValue);
			maxValue.setValue(data.maxValue);
			presentTrackValues.initValue = data.minValue;
			presentTrackValues.maxValue = data.minValue;
			presentTrackValues.minValue = data.minValue;
			presentTrackValues.trackValueSelected = data.trackValueSelected;
			presentTrackValues.trackValueType = 1;
			Midicc.setToggleState(true, sendNotification);
		}
	}

	TrackValueSelection::~TrackValueSelection()
	{
	}

	void TrackValueSelection::paint(juce::Graphics& g)
	{
		auto b = getLocalBounds().toFloat();
		String Imagename;
		Image Draw;

		g.fillAll(Theme::Instance().getColour(transparentBlack));
		if (!selection_state)
		{
			g.setColour(ui::Theme::Instance().getColour(customGrey5));
			g.fillRoundedRectangle(b, 3.0f);
			g.setColour(ui::Theme::Instance().getColour(customBlue16));
			g.fillRoundedRectangle(b.getX() + 2, b.getY() + 2, b.getWidth() - 4, b.getHeight() - 4, 3.0f);
		}
		else
		{
			g.setColour(ui::Theme::Instance().getColour(customGrey5));
			g.fillRoundedRectangle(ui::Rectangle{ b.getX(),b.getY(),b.getWidth(),(float)360 }.toFloat(), 3.0f);
			g.setColour(ui::Theme::Instance().getColour(customBlue16));
			g.fillRoundedRectangle(b.getX() + 2, b.getY() + 2, b.getWidth() - 4, 360 - 4, 3.0f);
		}

		g.setColour(ui::Theme::Instance().getColour(customGrey5));
		g.fillRect(ui::Rectangle{ 0, 80, getLocalBounds().getWidth(),  1 });
		g.fillRect(ui::Rectangle{ 0, 148, getLocalBounds().getWidth(), 1 });
		if (!selection_state)
		{
			Imagename = initValue.getFocusState() ? "track-modal-input-default-hover.png" : "track-modal-input-default.png";//? "track-modal-input-active-hover.png" : "track-modal-input-active.png" : PgmCtr.isMouseOver() ? "track-modal-input-default-hover.png" : "track-modal-input-default.png";
			Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
			g.drawImage(Draw, ui::Rectangle<float>{25, 359, 267, 50}, RectanglePlacement::centred);

			Imagename = maxValue.getFocusState() ? "track-modal-input-default-hover.png" : "track-modal-input-default.png";
			Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
			g.drawImage(Draw, ui::Rectangle<float>{300, 359, 267, 50}, RectanglePlacement::centred);
		}
		else
		{
			Imagename = bankLSB.getFocusState() ? "track-modal-input-default-hover.png" : "track-modal-input-default.png";
			Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
			g.drawImage(Draw, ui::Rectangle<float>{25, 180, 267, 50}, RectanglePlacement::centred);

			Imagename = bankMSB.getFocusState() ? "track-modal-input-default-hover.png" : "track-modal-input-default.png";
			Draw = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
			g.drawImage(Draw, ui::Rectangle<float>{300, 180, 267, 50}, RectanglePlacement::centred);
		}
	}

	void TrackValueSelection::resized()
	{
		auto b = getLocalBounds().reduced(10);

		const auto spaceW{ 7 }, spaceH{ 7 };
		auto w = 130;
		auto h = 50;
		int column = 0, x = b.getX() + 3, y = b.getY();

		for (auto opt : options)
		{
			opt->setBounds(x, y, w, h);
			x += (w + spaceW);
			if (++column >= 4)
			{
				x = b.getX() + 3;
				y += (h + spaceH);
				column = 0;
			}
		}
		auto height = y + (h + spaceH);

		view.setBounds(10, 150, 560, 200);
		view.setScrollBarsShown(false, false, true, false);
		container.setSize(560, height);
		knobLabel.setBounds(0, 20, getLocalBounds().getWidth(), 50);
		Midicc.setBounds(20, 90, 271, 50);
		PgmCtr.setBounds(299, 90, 271, 50);
		initLable.setBounds(45, 365, 264, 32);
		maxLable.setBounds(323, 365, 264, 32);
		initValue.setBounds(25, 359, 267, 50);
		initValue.setJustificationType(Justification::centredRight);
		maxValue.setBounds(303, 359, 267, 50);
		maxValue.setJustificationType(Justification::centredRight);
		BlsbLable.setBounds(45, 186, 264, 32);
		BmsbLable.setBounds(325, 186, 264, 32);
		bankLSB.setBounds(25, 180, 267, 50);
		bankMSB.setBounds(303, 180, 267, 50);
		if (!selection_state)
			ctlBox.performLayout(ui::Rectangle{ 25,418,542,44 });
		else
			ctlBox.performLayout(ui::Rectangle{ 25,280,542,44 });
	}

	void TrackValueSelection::updateAccidentals(bool state)
	{
		if (trackValuePtr != nullptr)
			state ? trackValuePtr->sliderPtr[valueUpdateIndex]->setStringLimit(mt::Ignite::rootNoteNamesFlats, 1)
			: trackValuePtr->sliderPtr[valueUpdateIndex]->setStringLimit(mt::Ignite::rootNoteNamesSharps, 1);
	}

	void TrackValueSelection::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		if (buttonThatWasClicked == &ctlApply)
		{
			ctlApply.setToggleState(false, dontSendNotification);
			if (presentTrackValues.trackValueType != -1)
			{
				if (presentTrackValues.trackValueType == 1 && presentTrackValues.trackValueSelected == -1)
				{
					warnPopupPtr->close();
					warnPopupPtr->setWarningText("Please Select a CC Value");
					warnPopupPtr->show();
					warnPopupPtr->onSelectionChanged = [this](int r)
						{
							if (auto dw = getCurrentlyModalComponent())
								dw->exitModalState(0);
						};
				}
				else if (presentTrackValues.trackValueType == 1 && ((presentTrackValues.maxValue == 0) || (presentTrackValues.maxValue <= presentTrackValues.initValue)))
				{
					if (presentTrackValues.trackValueType == 1 && (presentTrackValues.maxValue == 0))
					{
						warnPopupPtr->close();
						warnPopupPtr->setWarningText("Max value should not be zero");
						warnPopupPtr->show();
						warnPopupPtr->onSelectionChanged = [this](int r)
							{
								if (auto dw = getCurrentlyModalComponent())
									dw->exitModalState(0);
							};
					}
					else if (presentTrackValues.maxValue <= presentTrackValues.initValue)
					{

						auto ptr = CommonWarningPopUp::getCommonWarningPopupInstance();
						ptr->close();
						ptr->setWarningText("Init value sholud be less than max value");
						ptr->show();
						ptr->onSelectionChanged = [this](int r)
							{
								if (auto dw = getCurrentlyModalComponent())
									dw->exitModalState(0);
							};
					}
				}
				else
				{
					valuePtr->setSliderEditCtl(false);
					valuePtr->updateSlider(&presentTrackValues);
					TrackValueSelection::GetAppTrackValueSelectionInstance()->exitModalState(0);
					TrackValueSelection::GetAppTrackValueSelectionInstance()->setVisible(false);
				}
			}
		}
		else if (buttonThatWasClicked == &ctlCancel)
		{
			ctlCancel.setToggleState(false, dontSendNotification);
			TrackValueSelection::GetAppTrackValueSelectionInstance()->exitModalState(0);
			TrackValueSelection::GetAppTrackValueSelectionInstance()->setVisible(false);
			maxValue.setVisible(false);
			initValue.setVisible(false);
			bankMSB.setVisible(false);
			bankLSB.setVisible(false);
			BmsbLable.setVisible(false);
			BlsbLable.setVisible(false);
			valuePtr->setSliderEditCtl(false);
		}
		else if (buttonThatWasClicked == &ctlclear)
		{
			ctlclear.setToggleState(false, dontSendNotification);
			maxValue.setValue(0);
			initValue.setValue(0);
			bankMSB.setValue(0);
			bankLSB.setValue(0);
		}
		else if (buttonThatWasClicked == &Midicc)
		{
			if (Midicc.getToggleState())
			{
				PgmCtr.setToggleState(false, dontSendNotification);
				view.setVisible(true);
				container.setVisible(true);
				initValue.setVisible(true);
				maxValue.setVisible(true);
				initLable.setVisible(true);
				maxLable.setVisible(true);
				bankLSB.setVisible(false);
				bankMSB.setVisible(false);
				BmsbLable.setVisible(false);
				BlsbLable.setVisible(false);
				selection_state = 0;
				presentTrackValues.trackValueType = 1;
				valuePtr->setTrackValueData(mt::Ignite::TrackValueDataIndex::TVType, 1);
				valuePtr->getdatafromEngine(valuePtr->activeKnob);
				resized();
				repaint();
			}
			else
			{
				Midicc.setToggleState(true, dontSendNotification);
			}

			if (presentTrackValues.internalsound)
				Midicc.setToggleState(true, dontSendNotification);
		}
		else if (buttonThatWasClicked == &PgmCtr)
		{
			if (PgmCtr.getToggleState())
			{
				Midicc.setToggleState(false, dontSendNotification);
				view.setVisible(false);
				container.setVisible(false);
				initValue.setVisible(false);
				maxValue.setVisible(false);
				initLable.setVisible(false);
				maxLable.setVisible(false);
				bankLSB.setVisible(true);
				bankMSB.setVisible(true);
				BmsbLable.setVisible(true);
				BlsbLable.setVisible(true);
				selection_state = 1;
				presentTrackValues.trackValueType = 2;
				valuePtr->setTrackValueData(mt::Ignite::TrackValueDataIndex::TVType, 2);
				valuePtr->getdatafromEngine(valuePtr->activeKnob);
				bankLSB.setValue(valuePtr->activeKnobdata.lsbValue);
				bankMSB.setValue(valuePtr->activeKnobdata.msbValue);
				resized();
				repaint();
			}
			else
			{
				PgmCtr.setToggleState(true, dontSendNotification);
			}
		}
		else
		{
			for (int i = 0; i < options.size(); i++)
			{
				if (buttonThatWasClicked == options[i])
				{
					presentTrackValues.trackValueSelected = i;
				}
			}
		}
	}

	int IgniteTrackValues::isEditing()
	{
		int index = 0;
		slider[0].isEditing ? index = 1 : 0;
		slider[1].isEditing ? index = 2 : 0;
		slider[2].isEditing ? index = 3 : 0;
		slider[3].isEditing ? index = 4 : 0;
		slider[4].isEditing ? index = 5 : 0;
		slider[5].isEditing ? index = 6 : 0;
		slider[6].isEditing ? index = 7 : 0;
		slider[7].isEditing ? index = 8 : 0;

		return index;
	}

	bool IgniteTrackValues::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		if (isEditing())
			valueSelectionPtr->PannelKeyHandler(param, KeyValue, isEditing());

		else
		{
			if (hideSlider == nullptr)
				hideSlider = CommonAutoHideSlider::GetHideSliderInstance();

			slider[current_slider].FPKeyHandler(false, param, 3);
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RELEASE:
				slider[0].FPKeyHandler(true, param, KeyValue);
				current_slider = 0;
				break;
			case FrontPannelParam::paramKey_FILTER:
				
				if (slider[1].getTitle().getText() == "CUTOFF")
				{
					isFromFp = (int8_t)KeyValue;
					onRotaryValueChange(&slider[1], slider[1].getValue());
				}
				else
					slider[1].FPKeyHandler(true, param, KeyValue);
				current_slider = 1;
				break;
			case FrontPannelParam::paramKey_RESONANCE:
				slider[2].FPKeyHandler(true, param, KeyValue);
				current_slider = 2;
				break;
			case FrontPannelParam::paramKey_VOLUME:
				slider[3].FPKeyHandler(true, param, KeyValue);
				current_slider = 3;
				break;
			case FrontPannelParam::paramKey_REVERBE:
				current_slider = 4;
				slider[4].FPKeyHandler(true, param, KeyValue);
				break;
			case FrontPannelParam::paramKey_CHORUS:
				current_slider = 5;
				slider[5].FPKeyHandler(true, param, KeyValue);
				break;
			case FrontPannelParam::paramKey_EQHI:
				current_slider = 6;
				slider[6].FPKeyHandler(true, param, KeyValue);
				break;
			case FrontPannelParam::paramKey_EQLO:
				current_slider = 7;
				slider[7].FPKeyHandler(true, param, KeyValue);
				break;
			case FrontPannelParam::paramKey_RSELECT:
				if (soundStatus && KeyValue == 1)
					onbuttonClicked(&slider[current_slider], slider[getKnobIndex(slider[current_slider].getKnob())].getAlpha() == (1.0f) ? 0 : 1);
				slider[current_slider].FPKeyHandler(true, param, KeyValue);
				valueSelectionPtr->clearPrevTrackValuFocus();
				break;
			case FrontPannelParam::paramKey_RETURN_NAV:
				IgniteTrack::GetAppTrackInstance()->changeTrackOptionsVisibility(IgniteTrack::trackScreenIndex::OPTION_WINDOW);
				break;

			case FrontPannelParam::paramKey_RMOVELEFT:
				current_slider = (current_slider + 1) >= 8 ? (7) : (current_slider + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				current_slider = (current_slider - 1 <= 0 ? 0 : current_slider - 1);
				break;

			}
			slider[current_slider].FPKeyHandler(true, param, 3);

		}
		return true;
	}

	int TrackValueSelection::isEditing()
	{
		int index = 0;
		initValue.IsEditing() ? index = 4 : 0;
		maxValue.IsEditing() ? index = 5 : 0;
		return index;
	}

	void TrackValueSelection::clearPrevTrackValuFocus()
	{
		(Midicc.getToggleState()) ? KeyFocusedItem = TrackValuesSelViewItemID::MIDI_CC : KeyFocusedItem = TrackValuesSelViewItemID::PGM_CHANGE;
		PannelSendKey(true, (paramId_t)FrontPannelParam::paramKey_MouseClick, 0);
	}

	bool TrackValueSelection::PannelKeyHandler(paramId_t param, uint32_t KeyValue, int knob)
	{
		bool setfocus = true;
		int EditItem = TrackValuesSelViewItemID::NO_FOCUS;

		if (warnPopupPtr != nullptr && warnPopupPtr->IsEditing())
			return warnPopupPtr->FPKeyHandler(true, param, KeyValue);

		if (!Midicc.getToggleState())
			pgmPannelKeyHandler(param, KeyValue, knob);

		if (KeyFocusedItem == TrackValuesSelViewItemID::NO_FOCUS)
		{
			/*1st key after Page visible */
			KeyFocusedItem = TrackValuesSelViewItemID::MIDI_CC;
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
			else if ((((KeyFocusedItem + 1 == CC_BOX) || (KeyFocusedItem - 1 == CC_BOX)) && (param != (paramId_t)FrontPannelParam::paramKey_RSELECT)) || (KeyFocusedItem == CC_BOX))
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
					options.getReference(presentTrackValues.trackValueSelected)->setToggleState(false, dontSendNotification);
				else
					PannelSendKey(false, param, KeyValue); // clear last Focus

				midiBox.setEditing(true);
				setfocus = true;
				KeyFocusedItem = CC_BOX;
			}
			else
			{
				midiBox.setEditing(false);
				PannelSendKey(false, param, KeyValue); // clear last Focus
				switch ((FrontPannelParam)param)
				{
				case FrontPannelParam::paramKey_RMOVELEFT:
					KeyFocusedItem = (KeyFocusedItem + 1 >= TrackValuesSelViewItemID::FOCUS_MAX ? (TrackValuesSelViewItemID::FOCUS_MAX) : KeyFocusedItem + 1);
					break;
				case FrontPannelParam::paramKey_RMOVERIGHT:
					KeyFocusedItem = (KeyFocusedItem - 1 <= TrackValuesSelViewItemID::NO_FOCUS ? (TrackValuesSelViewItemID::NO_FOCUS + 1) : KeyFocusedItem - 1);
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

	void TrackValueSelection::PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		switch (OptionsView[KeyFocusedItem])
		{
		case MIDI_CC:
			Midicc.FPKeyHandler(isFocuse, param, 0);
			break;
		case CC_BOX:
			CCFPKeyHandler(isFocuse, param, KeyValue);
			break;
		case INIT_VALUE:
			initValue.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case MAX_VALUE:
			maxValue.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case PGM_CHANGE:
			PgmCtr.FPKeyHandler(isFocuse, param, 0);
			break;
		case TrackValuesSelViewItemID::APPLY:
			ctlApply.FPKeyHandler(isFocuse, param, 0);
			break;
		case TrackValuesSelViewItemID::CANCEL:
			ctlCancel.FPKeyHandler(isFocuse, param, 0);
			break;
		case TrackValuesSelViewItemID::RESET:
			ctlclear.FPKeyHandler(isFocuse, param, 0);
			break;
		default:
			break;
		}
		repaint();
	}

	bool TrackValueSelection::CCFPKeyHandler(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT)
		{
			this->postCommandMessage(1);
		}
		else if (param == (paramId_t)FrontPannelParam::paramKey_RETURN_NAV)
		{
			midiBox.setEditing(false);
			KeyFocusedItem = APPLY;
			PannelSendKey(true, param, KeyValue);
		}
		else if (param == (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
		{
			midiBox.setEditing(false);
			KeyFocusedItem = MIDI_CC;
			PannelSendKey(true, param, KeyValue);
		}
		else if (isFocuse)
		{
			if (currentFocus == 0)
				options[0]->setOutlineColour((Colour(0xFFFFFFFF).withBrightness(1.0f)));

			if (currentFocus >= 0)
				options[currentFocus]->setOutlineColour(Theme::Instance().getColour(systemBlue));

			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				currentFocus = (currentFocus + 1 > (int)(options.size() - 1) ? (int)(options.size() - 1) : currentFocus + 1);
				options[currentFocus]->setOutlineColour((Colour(0xFFFFFFFF).withBrightness(1.0f)));
				if (currentFocus > 11 && !(currentFocus % 4))
					view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() + 57);

				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				currentFocus = (currentFocus - 1 < 0 ? 0 : currentFocus - 1);
				options[currentFocus]->setOutlineColour((Colour(0xFFFFFFFF).withBrightness(1.0f)));
				if (currentFocus <= 116 && !(currentFocus % 4))
					view.setViewPosition(view.getViewPositionX(), view.getViewPositionY() - 57);
				break;
			}
		}

		else
		{
			if (currentFocus == 0)
				options[0]->setOutlineColour(Theme::Instance().getColour(systemBlue));

			options[currentFocus]->setOutlineColour(Theme::Instance().getColour(systemBlue));
		}
		return true;
	}
	void TrackValueSelection::handleCommandMessage(int id)
	{
		if (id == 1)
		{
			buttonClicked(options[currentFocus]);
			options.getReference(currentFocus)->setToggleState(true, dontSendNotification);
		}
	}

	int TrackValueSelection::isPgmEditing()
	{
		int index = 0;
		bankLSB.IsEditing() ? index = 3 : 0;
		bankMSB.IsEditing() ? index = 4 : 0;
		return index;
	}

	bool TrackValueSelection::pgmPannelKeyHandler(paramId_t param, uint32_t KeyValue, int knob)
	{
		bool setfocus = true;
		int EditItem = TrackValuesPgmSelViewItemID::PGMNO_FOCUS;

		if (pgmKeyFocusedItem == TrackValuesPgmSelViewItemID::PGMNO_FOCUS)
		{
			/*1st key after Page visible */
			pgmKeyFocusedItem = TrackValuesPgmSelViewItemID::PGMPGM_CHANGE;
		}
		else
		{
			EditItem = isEditing();
			if (EditItem)
			{
				if (pgmKeyFocusedItem != EditItem)
				{
					pgmPannelSendKey(false, param, KeyValue);
					pgmKeyFocusedItem = EditItem;
				}
				setfocus = true;
			}
			else
			{
				pgmPannelSendKey(false, param, KeyValue); // clear last Focus
				switch ((FrontPannelParam)param)
				{
				case FrontPannelParam::paramKey_RMOVELEFT:
					pgmKeyFocusedItem = (pgmKeyFocusedItem + 1 >= TrackValuesPgmSelViewItemID::PGMFOCUS_MAX ? (TrackValuesPgmSelViewItemID::PGMFOCUS_MAX) : pgmKeyFocusedItem + 1);
					break;
				case FrontPannelParam::paramKey_RMOVERIGHT:
					pgmKeyFocusedItem = (pgmKeyFocusedItem - 1 <= TrackValuesPgmSelViewItemID::PGMNO_FOCUS ? (TrackValuesPgmSelViewItemID::PGMNO_FOCUS + 1) : pgmKeyFocusedItem - 1);
					break;
				case FrontPannelParam::paramKey_RETURN_NAV:
					IgniteTrack::GetAppTrackInstance()->changeTrackOptionsVisibility(IgniteTrack::trackScreenIndex::OPTION_WINDOW);
					break;
				}
			}
		}
		if (setfocus)
		{
			pgmPannelSendKey(true, param, KeyValue);
		}
		return true;
	}

	void TrackValueSelection::pgmPannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		switch (pgmOptionsView[pgmKeyFocusedItem])
		{
		case PGMMIDI_CC:
			Midicc.FPKeyHandler(isFocuse, param, 0);
			break;
		case PGMPGM_CHANGE:
			PgmCtr.FPKeyHandler(isFocuse, param, 0);
			break;
		case LSB:
			bankLSB.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case MSB:
			bankMSB.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case PGMAPPLY:
			ctlApply.FPKeyHandler(isFocuse, param, 0);
			break;
		case PGMCANCEL:
			ctlCancel.FPKeyHandler(isFocuse, param, 0);
			break;
		case PGMRESET:
			ctlclear.FPKeyHandler(isFocuse, param, 0);
			break;
		default:
			break;
		}
		repaint();
	}
}
