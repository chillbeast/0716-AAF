FrountPannel::FrountPannel()
{
	myCommandManager = new ApplicationCommandManager();
	popupPtr = CommonPopUp::getCommonPopupInstance();
	for (uint8_t i = 0; i < (uint8_t)FrontPannelParam::paramKey_End_KeyMapping; i++)
	{
		KeyStatus[i] = (uint8_t)FrontPannelKeyStatus::paramKeyStatusIdle;
	}
	_pInfoWindow = CommonInfoWindow::GetInfoWindowInstance();
}

FrountPannel::~FrountPannel()
{

}

void FrountPannel::EnginePublish(moduleId_t module, moduleId_t moduleSrc, paramId_t paramID, const AnyType& value, bool isResponse)
{
	uint8_t KeyValue = 0;

	AppMainmode* AppMainmodeRef = AppMainmode::getMainModeInstance();
	mt::Ignite::tabs tabIndex = AppMainmodeRef->GetActiveTab();

	if (paramID == (paramId_t)FrontPannelParam::paramKey_TempoValue)
	{
		tapTempoValue = value.get<float>();
		if (tabIndex == mt::Ignite::tabs::SCENE)
			IgniteScene::getAppSceneInstance()->_pSceneView->setTapTempo(tapTempoValue, true, true);
		else
			IgniteSong::getAppSongInstance()->_pSongView->setTapTempo(tapTempoValue, true, true);
		return;
	}
	else if (paramID == (paramId_t)FrontPannelParam::paramKey_KeyDataReady)
	{
		std::vector<IgniteParamList> KeyParamList;
		IgniteParamList params;
		params.param = (paramId_t)FrontPannelParam::paramKey_KeyDataReady;
		KeyParamList.push_back(params);
		KeyParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, KeyParamList);

		if (KeyParamList.empty())
			return;

		multiKeyPressed = KeyParamList.begin()->Count;//only the 1st will have the correct value
		for (auto& i : KeyParamList)
		{
			if ((uint8_t)i.param < (uint8_t)FrontPannelParam::paramKey_End_KeyMapping)
				KeyStatus[i.param] = static_cast<uint8_t>(i.startPos); //Not using Anytype to avoid multiple calls to Anytype, #improve performance
		}
		for (uint8_t param = 0; param < (uint8_t)FrontPannelParam::paramKey_End_KeyMapping; param++)//KeyStatus seq map to paramID(param)
		{
			bool isKeyActive = false;
			KeyValue = KeyStatus[param];
			if ((KeyValue != (uint8_t)FrontPannelKeyStatus::paramKeyStatusIdle))
			{
				isKeyActive = KeyValue != (uint8_t)FrontPannelKeyStatus::paramKeyStatusReleased;

				switch ((FrontPannelParam)param)
				{
				case FrontPannelParam::paramKey_MENU:
					if (isKeyActive && KeyValue != (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold)
						AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::MENU);
					break;
				case FrontPannelParam::paramKey_SONG:
					if (isKeyActive && KeyValue != (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold)
						AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::SONG);
					break;
				case FrontPannelParam::paramKey_SCENE:
					if (isKeyActive && KeyValue != (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold)
						AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::SCENE);
					break;
				case FrontPannelParam::paramKey_TRACK:
					if (isKeyActive && KeyValue != (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold)
						AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::TRACK);
					break;
				case FrontPannelParam::paramKey_PATTERN:
					if (isKeyActive && KeyValue != (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold)
						AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::PATTERN);
					break;
				case FrontPannelParam::paramKey_PAGESHIFT:
					if (isKeyActive && KeyValue == (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed)
					{
						isShiftKeyActive = isKeyActive && !isShiftKeyActive ? true : false;
						postCommandMessage(1);
						// To indicate the user that the Shift key is engaged
						_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_SHIFT, (isShiftKeyActive ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
						Handle_NavigationKey(AppMainmodeRef, param, KeyValue);
					}
					break;
				default:
					if ((paramId_t)param >= (paramId_t)FrontPannelParam::paramKey_ROTARYROW && (paramId_t)param < (paramId_t)FrontPannelParam::paramFrontPannelMax)
						Handle_NavigationKey(AppMainmodeRef, param, KeyValue);
					break;
				}
			}
		}
	}
	else if (paramID == (paramId_t)SongParam::paramSyncIn)
	{
		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleSong, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::param_SyncOut, value.get<uint16_t>());
	}
	else if (paramID == (paramId_t)SongParam::paramSyncOut)
	{
		//TODO move this out of publish(), to an api which can be used for requesting data from engine.
		_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleSong, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::param_SyncIn, true);
	}
	else if (paramID == (paramId_t)FrontPannelParam::paramKey_DevStatus)
	{
		auto devStatus = (DEV_STATUS)value.get<int>();

		if (devStatus == DEV_STATUS::PLAYTRIBE_OP_STARTED)
			postCommandMessage(2);
		else
			postCommandMessage(3);
	}
	else if (paramID == (paramId_t)FrontPannelParam::paramKey_FwUpgrade)
	{
		postCommandMessage(4);
	}
	else
	{
		KeyValue = value.get<uint8_t>();
		HandleKeyIndication(AppMainmodeRef, paramID, KeyValue);
	}
	return;
}

void FrountPannel::simulatorInput(FrontPannelParam param, uint8_t value)
{
    const auto id = static_cast<paramId_t>(param);
    if (id >= static_cast<paramId_t>(FrontPannelParam::paramKey_End_KeyMapping)) return;
    const bool rotation = (id >= 62 && id <= 88);
    if (!rotation) KeyStatus[id] = value;
    int held = 0;
    for (int i = 0; i < 62; ++i)
        if (KeyStatus[i] == static_cast<uint8_t>(FrontPannelKeyStatus::paramKeyStatusPressed)
            || KeyStatus[i] == static_cast<uint8_t>(FrontPannelKeyStatus::paramKeyStatusOnHold)) ++held;
    multiKeyPressed = held > 1;
    if (param == FrontPannelParam::paramKey_INTERNALSOUNDS)
        static_cast<IgniteEngine*>(_PUiVirtualEngine.get())->songManager->handleMcuSpecialKey(id,
            value == static_cast<uint8_t>(FrontPannelKeyStatus::paramKeyStatusReleased) ? 0 : 1);
    if (!rotation && value == static_cast<uint8_t>(FrontPannelKeyStatus::paramKeyStatusOnHold)
        && param != FrontPannelParam::paramKey_PAGESHIFT)
        Handle_NavigationKey(AppMainmode::getMainModeInstance(), id, value);
    else
        HandleKeyIndication(AppMainmode::getMainModeInstance(), id, value);
    if (!rotation && value == static_cast<uint8_t>(FrontPannelKeyStatus::paramKeyStatusReleased))
        KeyStatus[id] = static_cast<uint8_t>(FrontPannelKeyStatus::paramKeyStatusIdle);
}

bool FrountPannel::HandleKeyIndication(AppMainmode* AppMainmodeRef, paramId_t param, uint8_t KeyValue)
{
	bool handled = true;
	/* Keeping all Keys, for Simulator and future improvments as Engine can send individula Key event */
	if (((FrontPannelKeyStatus)KeyValue == FrontPannelKeyStatus::paramKeyStatusPressed && param != (paramId_t)FrontPannelParam::paramKey_TEMPO) || ((FrontPannelParam)param >= FrontPannelParam::paramKey_ROTARY_NAV && (FrontPannelParam)param <= FrontPannelParam::paramRot_BAR))
	{
		switch ((FrontPannelParam)param)
		{
		case FrontPannelParam::paramKey_MENU:
			AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::MENU);
			break;
		case FrontPannelParam::paramKey_SONG:
			AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::SONG);
			break;
		case FrontPannelParam::paramKey_SCENE:
			AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::SCENE);
			break;
		case FrontPannelParam::paramKey_TRACK:
			AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::TRACK);
			break;
		case FrontPannelParam::paramKey_PATTERN:
			AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::PATTERN);
			break;
		case FrontPannelParam::paramKey_PAGESHIFT:
			if ((FrontPannelKeyStatus)KeyValue == FrontPannelKeyStatus::paramKeyStatusPressed)
			{
				isShiftKeyActive = isShiftKeyActive ? false : true;

				postCommandMessage(1);
				// To indicate the user that the Shift key is engaged
				_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_PAGESHIFT, (isShiftKeyActive ? fpLEDPattern::EN_LED_PATT_ON : fpLEDPattern::EN_LED_PATT_OFF));
			}
			break;
		default:
			if ((paramId_t)param >= (paramId_t)FrontPannelParam::paramall && (paramId_t)param < (paramId_t)FrontPannelParam::paramFrontPannelMax)
				Handle_NavigationKey(AppMainmodeRef, param, KeyValue);
			break;
		}
	}
	return handled;
}

bool FrountPannel::Handle_NavigationKey(AppMainmode* AppMainmodeRef, paramId_t param, uint8_t KeyValue)
{
	bool handled = true;
	mt::Ignite::tabs tab_index = AppMainmodeRef->GetActiveTab();
	switch (tab_index)
	{
	case mt::Ignite::tabs::MENU:
	case mt::Ignite::tabs::SONG:
	case mt::Ignite::tabs::SCENE:
	case mt::Ignite::tabs::TRACK:
	case mt::Ignite::tabs::PATTERN:
		switch ((FrontPannelParam)param)
		{
		case FrontPannelParam::paramKey_SCULPT:
			if (tab_index != mt::Ignite::tabs::PATTERN)
				AppMainmodeRef->HandleTabKey(mt::Ignite::tabs::PATTERN);
			handled = AppMainmodeRef->PannelKeyHandler(mt::Ignite::tabs::PATTERN, (paramId_t)FrontPannelParam::paramKey_SCULPT, KeyValue);
			break;

		default:
			handled = AppMainmodeRef->PannelKeyHandler(tab_index, param, KeyValue);
			break;
		}
	default:
		handled = false;
		break;
	}
	return handled;
}

void FrountPannel::handleCommandMessage(int cmdId)
{
	if (cmdId == 1)
	{
		_pInfoWindow->setTextandShow(isShiftKeyActive ? "Shift: ON" : "Shift: OFF");
	}
	if (cmdId == 2)
	{
		IgniteSettings::GetAppSettingsInstance()->pcOpOngoingPopUpCtl(true);
	}
	else if (cmdId == 3)
	{
		IgniteSettings::GetAppSettingsInstance()->pcOpOngoingPopUpCtl(false);
	}
	else if (cmdId==4)
	{
		IgniteSettings::GetAppSettingsInstance()->changeFwStatus(false);
	}
}
