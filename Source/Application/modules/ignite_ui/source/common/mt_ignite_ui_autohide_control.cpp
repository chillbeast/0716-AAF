namespace ui
{
	extern juce::Array<mt::Ignite::instrumentsData> gInstruments;

	template<int blurShift, bool enhanceContrast = false>
	inline void blurImage(juce::Image& img)
	{
		if (!img.isValid() || !img.getWidth() || !img.getHeight()) return;
		img = img.convertedToFormat(juce::Image::ARGB);
		juce::Image::BitmapData bm(img, 0, 0, img.getWidth(), img.getHeight(), juce::Image::BitmapData::readWrite);
		int                     h = img.getHeight();
		int                     w = img.getWidth();

		auto pxBlur = [](int p, int& s, int blrShift, bool enhContrast = false) -> int {
			int px = p << 16;
			s += (px - s) >> blrShift;
			if (enhContrast)
			{
				px = s >> 8;
				px = ((((98304 - px) >> 7) * px >> 16) * px >> 16); // sine clamp
				return juce::jlimit(0, 255, px);
			}
			return s >> 16;
		};

		for (int y = 0; y < h; y++)
		{
			for (int c = 0; c < 4; c++)
			{
				uint8_t* p = bm.getLinePointer(y) + c;
				int      s = p[0] << 16;
				for (int x = 0; x < w; x++, p += 4)
					p[0] = (uint8_t)pxBlur(int(p[0]), s, blurShift);

				p -= 4;
				for (int x = 0; x < w; x++, p -= 4)
					p[0] = (uint8_t)pxBlur(int(p[0]), s, blurShift);
			}
		}

		for (int x = 0; x < w; x++)
		{
			for (int c = 0; c < 4; c++)
			{
				uint8_t* p = bm.getPixelPointer(x, 0) + c;
				int      incr = int(bm.getPixelPointer(x, 1) - bm.getPixelPointer(x, 0));
				int      s = p[0] << 16;
				for (int y = 0; y < h; y++, p += incr)
					p[0] = (uint8_t)pxBlur(int(p[0]), s, blurShift);

				p -= incr;
				for (int y = 0; y < h; y++, p -= incr)
					p[0] = (uint8_t)pxBlur(int(p[0]), s, blurShift, enhanceContrast);
			}
		}
	} // blurImage
	CommonAutoHideSlider::CommonAutoHideSlider()
	{
		addMouseListener(this, true);
		sliderContainerBox.flexDirection = FlexBox::Direction::row;
		sliderContainerBox1.flexDirection = FlexBox::Direction::row;
		addAndMakeVisible(_uiDimComp);
		_uiDimComp.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colours::black.withAlpha(0.5f));
		_uiDimComp.toFront(false);
		_uiDimComp.setComponentID("label");
		for (int i = 0; i < 4; i++)
		{
			addAndMakeVisible(slider[i]);
			slider[i].showTitleAndValue(true, Slider::TextBoxBelow, Justification::centred);
			slider[i].setDisplayProportions(.6f, .2f, .2f);
			slider[i].addListener(this);
			slider[i].setLookAndFeel(CommonRotaryCv_gate::getLnf());
			slider[i].showTitle(true, Justification::centred);
			slider[i].setComponentID(String(i));
			sliderContainerBox1.items.add(FlexItem(slider[i]).withFlex(1).withMaxHeight(800).withMaxWidth(400));
		}

		for (int i = 4; i < 8; i++)
		{
			addAndMakeVisible(slider[i]);
			slider[i].showTitleAndValue(true, Slider::TextBoxBelow, Justification::centred);
			slider[i].setDisplayProportions(.6f, .2f, .2f);
			slider[i].addListener(this);
			slider[i].setLookAndFeel(CommonRotaryCv_gate::getLnf());
			slider[i].showTitle(true, Justification::centred);
			slider[i].setComponentID(String(i));
			sliderContainerBox.items.add(FlexItem(slider[i]).withFlex(1).withMaxHeight(800).withMaxWidth(400));
		}
		String a = "A";
		for (int i = 0; i < MAX_TRACK_VALUE; i++)
			slider[i].setKnobTitle(a.charToString('A' + i), true);
		
	}

	CommonAutoHideSlider::~CommonAutoHideSlider()
	{

	}

	void CommonAutoHideSlider::paint(juce::Graphics& g)
	{

	}

	void CommonAutoHideSlider::resized()
	{
		_uiDimComp.setBounds(0, 0, 1024, 600);
	}

	void CommonAutoHideSlider::timerCallback()
	{
		stopTimer();
		SetVisible(false);
		sliderContainerBox.performLayout(ui::Rectangle<float>{0, 0, 0, 0});
		sliderContainerBox1.performLayout(ui::Rectangle<float>{0, 0, 0, 0});
	}

	void CommonAutoHideSlider::SetVisible(bool state) 
	{
		
			IgniteParamList params;
			std::vector<IgniteParamList> Params;
			Params.clear();
			params.param = (paramId_t)TrackParam::paramActiveTrack;
			Params.push_back(params);
			params.param = (paramId_t)TrackParam::paramTrackInstrument;
			Params.push_back(params);
			String s;
			if (state)
			{
				Params = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, Params);
				//activeTrack = Params[0].MsgValue.get<uint8>();
				s = Params[0].MsgValue.get<std::string>();
			}
			//if (activeTrack != -1 && s != " ");
			{
				if (isVisible && state)
				{
					stopTimer();
					startTimer(timems);
				}
				else if (!isVisible && state)
				{
					if (TrackValuePtr == nullptr)
						TrackValuePtr = IgniteTrackValues::GetAppTrackValueInstance();
					altstate = TrackValuePtr->getFxAltState();
					if (TrackValuePtr != nullptr)
					{
						if (gInstruments.size())
						{
							for (int i = 0; i < gInstruments.size(); i++)
							{
								if (gInstruments[i].name == s)
								{
									soundStatus = gInstruments[i].isIntSoundEnabled;
								}
							}
						}
						_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, s.toStdString());
						setAlternativeFunc();
						
						for (int i = 0; i < 8; i++)
						{
							TrackValuePtr->getdatafromEngine(i);
							activeknobdata = TrackValuePtr->getactiveknobdata();
							slider[i].setValue(activeknobdata.currentValue,dontSendNotification);
							slider[i].setTitle(activeknobdata.name);
							if (activeknobdata.name == "Cutoff")
							{
								Vcutoff = activeknobdata.currentValue;
							}
							DBG(activeknobdata.currentValue);
						}

					}

					startTimer(timems);
				}
				else if (!state)
					stopTimer();

				setVisible(state);

				isVisible = state;
				if (state)
					toFront(true);
				else
					toFront(false);
				
			}
		
	};


	bool CommonAutoHideSlider::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
			int index = -1;
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RELEASE:
				slider[0].FPKeyHandler(true, param, KeyValue);
				index = 0;
				break;
			case FrontPannelParam::paramKey_FILTER:
				if (slider[1].getTitle().getText() == "Cutoff")
				{
					isFromFp = (int8_t)KeyValue;
					onRotaryValueChange(&slider[1], slider[1].getValue());
				}
				else
					slider[1].FPKeyHandler(true, param, KeyValue);
				index = 1;
				break;
			case FrontPannelParam::paramKey_RESONANCE:
				slider[2].FPKeyHandler(true, param, KeyValue);
				index = 2;
				break;
			case FrontPannelParam::paramKey_VOLUME:
				slider[3].FPKeyHandler(true, param, KeyValue);
				index = 3;
				break;
			case FrontPannelParam::paramKey_REVERBE:
				slider[4].FPKeyHandler(true, param, KeyValue);
				index = 4;
				break;
			case FrontPannelParam::paramKey_CHORUS:
				slider[5].FPKeyHandler(true, param, KeyValue);
				index = 5;
				break;
			case FrontPannelParam::paramKey_EQHI:
				slider[6].FPKeyHandler(true, param, KeyValue);
				index = 6;
				break;
			case FrontPannelParam::paramKey_EQLO:
				slider[7].FPKeyHandler(true, param, KeyValue);
				index = 7;
				break;
			case FrontPannelParam::paramKey_RSELECT:
				break;
			case FrontPannelParam::paramKey_RETURN_NAV:
				IgniteTrack::GetAppTrackInstance()->changeTrackOptionsVisibility(IgniteTrack::trackScreenIndex::OPTION_WINDOW);
				break;
			default:
				break;
			}
			sliderContainerBox1.items.clear();
			sliderContainerBox.items.clear();

			for (int i = 0; i < 4; i++)
			{
				sliderContainerBox.items.add(FlexItem(slider[i]).withFlex(1).withMaxHeight(200).withMaxWidth(200));
			}

			for (int i = 4; i < 8; i++)
			{
				sliderContainerBox1.items.add(FlexItem(slider[i]).withFlex(1).withMaxHeight(200).withMaxWidth(200));
			}

			sliderContainerBox.performLayout(ui::Rectangle(100, 120 , widthTrackValueFlexBox, 200));
			sliderContainerBox1.performLayout(ui::Rectangle(100, 340, widthTrackValueFlexBox, 200));

		return true;
	}

	void CommonAutoHideSlider::onRotaryValueChange(common_rotaryslider* ptr, double Value)
	{
		int rotaryId;
		for (rotaryId = 0; rotaryId < 8; rotaryId++)
			if (ptr == &slider[rotaryId])
				break;
		auto tmp_knob = (rotaryId + (MAX_TRACK_VALUE * (1 + altstate)));

		/* Same Knob value to be used for Resonance, Volume& Release */
		if (tmp_knob == 16)
			tmp_knob = 8;
		else if (tmp_knob == 18)
			tmp_knob = 10;

		activeknobdata.slot = rotaryId;
		TrackValuePtr->setTrackValueData(mt::Ignite::TrackValueDataIndex::Active_Knob, soundStatus ? tmp_knob : rotaryId);

		if (!soundStatus)
		{
			if (activeknobdata.trackValueType == 1)
				TrackValuePtr->setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, (int)Value);
			else
				TrackValuePtr->setTrackValueData(mt::Ignite::TrackValueDataIndex::SeqMidiPgmNum, (int)Value);
		}
		else
		{
			TrackValuePtr->setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, (float)Value);
			
			if (!altstate)
			{
				if ((ptr->getTitle().getText() == "Cutoff"))
				{
					auto minDelta = abs(FilterFreqMap[0] - Value);
					auto nearestIndex = 0;
					float cValue = 0;
					if (isFromFp != -1)
					{
						int i = 0;
						for (; i < 128; i++)
							if (FilterFreqMap[i] == Vcutoff)
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
							cValue = Vcutoff;
						if (cValue != Vcutoff)
						{
							TrackValuePtr->setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, cValue);
							slider[rotaryId].setFloatValue((double)cValue, dontSendNotification);
							Vcutoff = cValue;
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
					TrackValuePtr->setTrackValueData(mt::Ignite::TrackValueDataIndex::MidiCCValue, FilterFreqMap[nearestIndex]);
					slider[rotaryId].setFloatValue((double)FilterFreqMap[nearestIndex], dontSendNotification);
					activeknobdata.currentValue = FilterFreqMap[nearestIndex];
				}
			}
		}

		auto ledcnt = ((15 * (slider[rotaryId].getValue() - slider[rotaryId].getMin())) / (slider[rotaryId].getMax() - slider[rotaryId].getMin()));


		if ((slider[rotaryId].getValue() == slider[rotaryId].getMin()))
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_RELEASE + rotaryId), (uint16_t)0, false);
		else if (ledcnt >= 0 && ledcnt <= 15)
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_RELEASE + rotaryId), (uint16_t)ledcnt + 1, false);

		stopTimer();
		startTimer(timems);
		IgniteControlOptions::GetAppOptInstance()->enableKey(IgniteControlOptions::eKey::SAVE, true);
	}

	void CommonAutoHideSlider::setAlternativeFunc()
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


			if (altstate)
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

	void CommonAutoHideSlider::mouseDown(const MouseEvent& _event)
	{
		if (_event.eventComponent->getComponentID() == "label")
			SetVisible(false);

	}

};		