#include "mt_ignite_ui_interface.h"
#include <iostream>
#include <cstdlib> 
using namespace Ignite;
using namespace ui;


UiInterface::UiInterface(IDevice* device) : engine(device),
Thread("Publish Thread"),
enginePublishQ(IgniteConstants::msgQueueSizeUI)
{
	this->moduleId = (moduleId_t)ModuleName::moduleUI;
	engine->addListener(this);
	//Subscribe for all params
	engine->subscribe((moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramall, this);
	engine->subscribe((moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramall, this);
	engine->subscribe((moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramall, this);
	engine->subscribe((moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramall, this);
	engine->subscribe((moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramall, this);
	engine->subscribe((moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleCvioCnfg, (paramId_t)SettingsCvioCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingSongCnfg, (paramId_t)SettingsSongCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingSceneCnfg, (paramId_t)SettingsSceneCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsTrackCnfg, (paramId_t)SettingsTrackCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsTrackInstrumentMixer, (paramId_t)SettingsTrackInstrumentMixerParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsPatternCnfg, (paramId_t)SettingsPatternCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsRecordCnfg, (paramId_t)SettingsRecordCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsBRayCnfg, (paramId_t)SettingsBRayCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsRemoteControlCnfg, (paramId_t)SettingsRemoteControlCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsSculptModeCnfg, (paramId_t)SettingsSculptModeCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsUdsCnfg, (paramId_t)SettingsUdsCnfgParam::paramall, this);
	engine->subscribe ((moduleId_t)ModuleName::moduleSettingsMidiClkCnfg, (paramId_t)SettingsMidiClkCnfgParam::paramall, this);

	watchdogThread = std::make_unique<WatchDogService>();
	watchdogThread->pingWatchdogThread();
	lastWatchdogPing = juce::Time::getMillisecondCounter();
	watchdogThread->startThread();
#ifdef _WIN32
	startTimer(10);
#else
	startTimer(4000);
	startThread();
#endif
}
void UiInterface::timerCallback()
{
#ifdef _WIN32
    // Apply UI updates on the message thread. A publisher thread holding the
    // MessageManagerLock could otherwise wait on a model call made by the UI.
    IgniteMessageParam msg;
    for (int count = 0; count < 64 && enginePublishQ.get(msg, TimeOut::kNoWait); ++count)
        publishHandler(msg.moduleDst, msg.moduleSrc, msg.param, msg.MsgValue, msg.isPlayMode);
    const auto now = juce::Time::getMillisecondCounter();
    if (now - lastWatchdogPing < 4000 && lastWatchdogPing != 0) return;
    lastWatchdogPing = now;
#endif
    watchdogThread->pingWatchdogThread();
}
void UiInterface::run(void)
{
	IgniteMessageParam msg;
	while (!threadShouldExit() && !CThread::desktopShutdownRequested())
	{
		if (!enginePublishQ.get(msg, TimeOut::kWaitForever)) break;
		publishHandler(msg.moduleDst, msg.moduleSrc, msg.param, msg.MsgValue, msg.isPlayMode);
	}
}
UiInterface::~UiInterface()
{
    stopTimer();
    signalThreadShouldExit();
    stopThread(3000);
    if (watchdogThread) watchdogThread->stopThread(3000);
}
void UiInterface::publish(moduleId_t module, moduleId_t moduleSrc, paramId_t param, const AnyType& value, bool isResponse)
{
	if (enginePublishQ.numQueued() < IgniteConstants::msgQueueSizeUI)
	{
		IgniteMessageParam msg;
		msg.moduleDst = module;
		msg.moduleSrc = moduleSrc;
		msg.param = param;
		msg.MsgValue = value;
		msg.isPlayMode = isResponse;
		enginePublishQ.put(msg, TimeOut::kNoWait);
	}
	else
	{
		enginePublishQ.reset();
		std::cout << "UiInterface::publish:: Q Full" << std::endl;
	}
}

void updatesongdataEx(paramId_t param, const AnyType& value)
{
	//(std::string)(value.get<std::string>().c_str());
	//value.get<float>();
	//value.get<uint8_t>();
	//value.get<uint32_t>(); - not expected

	IgniteSong* songRef = IgniteSong::getAppSongInstance();
	IgniteInstruments* instrRef = IgniteInstruments::GetAppInstrumentsInstance();
	//IgniteScene* sceneRef = IgniteScene::getAppSceneInstance();

	switch ((SongParam)param)
	{
		case SongParam::paramSongName:
			songRef->updateSongData(param, value.get<std::string>());
			break;
		case SongParam::paramSongSwing:
			songRef->updateSongData(param, value.get<uint8_t>());
			break;
		case SongParam::paramSongTempo:
			songRef->updateSongData(param, value.get<float>());
			break;
		case SongParam::paramAddSongCmd:
			break;
		case SongParam::paramDeleteSongCmd:
			break;
		case SongParam::paramActiveSong:
			break;
		case SongParam::paramPlaySongCmd:
			break;
		case SongParam::paramPauseSongCmd:
			break;
		case SongParam::paramTrackCountCmd:
			break;
		case SongParam::paramUsbHostMidiChangeNotification:
			instrRef->updateMidiDeviceList();
			break;
		default:
			//TODO
			break;
	}
}


void updateScenedataEx(paramId_t param, const AnyType& value)
{
	IgniteSong* songRef = IgniteSong::getAppSongInstance();
	IgniteScene* sceneRef = IgniteScene::getAppSceneInstance();
	IgniteTrack* TrackRef = IgniteTrack::GetAppTrackInstance();

	switch ((SceneParam)param)
	{
		case SceneParam::paramSceneName:
			sceneRef->updateSceneData(param, value.get<std::string>());
			break;
		case SceneParam::paramSceneSwing:
			sceneRef->updateSceneData(param, value.get<uint8_t>());
			break;
		case SceneParam::paramCurrentPlaySceneId:
			songRef->updateSongData(param, value.get<uint32_t>());
			break;
		case SceneParam::paramNextPlaySceneId:
			songRef->updateSongData(param, value.get<uint32_t>());
			break;
		case SceneParam::paramSceneTempo:
			sceneRef->updateSceneData(param, value.get<float>());
			break;
		case SceneParam::paramSceneAdvanceMode:
			sceneRef->updateSceneData(param, value.get<int8_t>());
			break;
		case SceneParam::paramSceneGBar:
			sceneRef->updateSceneData(param, value.get<int8_t>());
			break;
		case SceneParam::paramSceneFTSRootNote:
			sceneRef->updateSceneData(param, value.get<int8_t>());
			break;
		case SceneParam::paramSceneFTSScale:
			sceneRef->updateSceneData(param, value.get<int8_t>());
			break;
		case SceneParam::paramSceneLength:
			sceneRef->updateSceneData(param, value.get<int16_t>());
			break;
		case SceneParam::paramSceneTransposeValue:
			sceneRef->updateSceneData(param, value.get<int8_t>());
			break;
		case SceneParam::paramSceneMuteHold:
			sceneRef->updateSceneData(param, value.get<int8_t>());
			break;
		case SceneParam::paramSceneTransposeFTSPriority:
			sceneRef->updateSceneData(param, value.get<int8_t>());
			break;
		case SceneParam::paramAddSceneCmd:
			break;
		case SceneParam::paramSceneCount:
			songRef->updateSongData(param, value.get<uint32_t>());
			break;
		case SceneParam::paramDeleteSceneCmd:
			break;
		case SceneParam::paramActiveScene:
			break;
		case SceneParam::paramCurrentPlaySceneUnmutedTracks:
			songRef->updateSongData(param, value.get<uint8_t>());
			break;
		case SceneParam::paramNextPlaySceneUnmutedTracks:
			songRef->updateSongData(param, value.get<uint8_t>());
			break;
		case SceneParam::paramSceneBarId:
			sceneRef->updateSceneData(param, value.get<uint8_t>());
			TrackRef->updateTrackData(param, value.get<uint8_t>());
			break;
		default:
			//TODO
			break;
	}
}
void updateTrackdataEx(paramId_t param, const AnyType& value)
{
	IgnitePattern* patternRef = IgnitePattern::getAppPatternInstance();
	IgniteTrack* TrackRef = IgniteTrack::GetAppTrackInstance();
//	IgniteScene* sceneRef = IgniteScene::getAppSceneInstance();

	switch ((TrackParam)param)
	{
		case TrackParam::paramTrackName:
			break;
		case TrackParam::paramActiveTrack:
			break;
		case TrackParam::paramTrackInstrument:
			break;
		case TrackParam::paramTrackPattern:
			break;
		case TrackParam::paramTrackMute:
			break;
		case TrackParam::paramTrackPatternBarAndStepId://Used in play time
		{
			uint16_t data = value.get<uint16_t>();
			uint8_t trackBarId = (data >> 8) & 0xFF;
			uint8_t trackPlayStepId = data & 0x00FF;
			patternRef->updatePatternData(trackBarId, trackPlayStepId);
		}
		break;
		case TrackParam::paramTrackPatternBarId:
			TrackRef->updateTrackData(param, value.get<uint8_t>());
			patternRef->updatePatternData(param, value.get<uint8_t>());
			break;
		case TrackParam::paramTrackPatternStepId:
			TrackRef->updateTrackData(param, value.get<uint8_t>());
			patternRef->updatePatternData(param, value.get<uint8_t>());
			break;
		default:
			//TODO
			break;
	}
}

void updateInstrumentdataEx(paramId_t param, const AnyType& value)
{
	IgniteInstruments* instrRef = IgniteInstruments::GetAppInstrumentsInstance();

	switch ((InstrumentParam)param)
	{
	case InstrumentParam::paramInstrumentChannel:
		break;
	case InstrumentParam::paramInstrumentPort:
		break;
	case InstrumentParam::paramInstrumentRemove:
		break;
	case InstrumentParam::paramInstrumentAdd:
		break;
	case InstrumentParam::paramInstrumentName:
		instrRef->updateInstrumentsData(param, value.get<std::string>());
		break;
	case InstrumentParam::paramInstrumentSave:
		break;
	default:
		//TODO
		break;
	}
}

void updatePatterndataEx(paramId_t param, const AnyType& value)
{
	//IgnitePattern* patternRef= IgnitePattern::getAppPatternInstance();

	//switch ((PatternParam)param)
	//{
	//
	//default:
	//	//TODO
	//	break;
	//}
}



void UiInterface::publishHandler(moduleId_t module, moduleId_t moduleSrc, paramId_t param, const AnyType& value, bool isResponse)
{
	//IgniteSong* songRef;
	//IgnitePattern* patternRef;
	//IgniteInstruments* instrRef;

	switch ((moduleId_t)module)
	{
	case (moduleId_t)ModuleName::moduleSong:
	{
		MessageManagerLock mml(Thread::getCurrentThread());

		if (!mml.lockWasGained())
			return; // another thread is trying to kill us!

		if (((moduleId_t)ModuleName::moduleUI != moduleSrc) || isResponse)
			updatesongdataEx(param, value);
	}
	break;
	case (moduleId_t)ModuleName::moduleScene:
	{
		MessageManagerLock mml(Thread::getCurrentThread());

		if (!mml.lockWasGained())
			return; // another thread is trying to kill us!

		if (((moduleId_t)ModuleName::moduleUI != moduleSrc) || isResponse)
			updateScenedataEx(param, value);
	}
	break;
	case (moduleId_t)ModuleName::moduleTrack:
	{
		bool handleData = true;
		MessageManagerLock mml(Thread::getCurrentThread());

		if (!mml.lockWasGained())
			return; // another thread is trying to kill us!

		switch ((uint8_t)param)//handle CV gate data
		{
		case (uint8_t)TrackParam::paramCvVoltsPort1://fall through
		case (uint8_t)TrackParam::paramCvVoltsPort2:
		case (uint8_t)TrackParam::paramCvVoltsPort3:
		case (uint8_t)TrackParam::paramCvVoltsPort4:
		case (uint8_t)TrackParam::paramCvVoltsPort5:
		case (uint8_t)TrackParam::paramCvVoltsPort6:
		case (uint8_t)TrackParam::paramCvVoltsPort7:
		case (uint8_t)TrackParam::paramCvVoltsPort8:
		{
			uint8_t portId = (uint8_t)param - (uint8_t)TrackParam::paramCvVoltsPort1;
			float cvVoltage = value.get<float>();
			CVControl::getCVInterface()->setCvVoltage(portId, cvVoltage);
			handleData = false;//not to update Track
		}
		break;
		case (uint8_t)TrackParam::paramCvGateState:
		{
			uint8_t val = value.get<uint8_t>();
			uint8_t gateId = (val >> 4) & 0x0F;
			uint8_t gatevotage = val & 0x0F;
			CVControl::getCVInterface()->setGateVoltage(gateId, gatevotage);
			handleData = false;//not to update Track
		}
		break;
		}

		if (handleData && (((moduleId_t)ModuleName::moduleUI != moduleSrc) || isResponse))
			updateTrackdataEx(param, value);
	}
	break;
	case (moduleId_t)ModuleName::modulePattern:
	{
		MessageManagerLock mml(Thread::getCurrentThread());

		if (!mml.lockWasGained())
			return; // another thread is trying to kill us!

		if (((moduleId_t)ModuleName::moduleUI != moduleSrc) || isResponse)
			updatePatterndataEx(param, value);
		//	instrRef = IgniteInstruments::GetAppInstrumentsInstance();
		//	instrRef->updateInstrumentsData(param, value);
	}
	break;
	case (moduleId_t)ModuleName::moduleInstrument:
	{
		MessageManagerLock mml(Thread::getCurrentThread());

		if (!mml.lockWasGained())
			return; // another thread is trying to kill us!

		if (((moduleId_t)ModuleName::moduleUI != moduleSrc) || isResponse)
			updateInstrumentdataEx(param, value);
	}
	break;
	case (moduleId_t)ModuleName::moduleFrontPannel:
	{
		MessageManagerLock mml(Thread::getCurrentThread());
		if (!mml.lockWasGained())
			return; // another thread is trying to kill us!
		FrountPannel* FrountPannelRef = FrountPannel::GetFrountPannelInstance();
		FrountPannelRef->EnginePublish(module, moduleSrc, param, value, isResponse);
		break;
	}
	break;
	}
}
