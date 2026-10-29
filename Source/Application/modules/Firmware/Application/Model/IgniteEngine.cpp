#include "SettingsManager/SettingsRemoteControlCnfg.h"
#include "SettingsManager/SettingsSculptModeCnfg.h"
#include "SettingsManager/SettingsUdsCnfg.h"
#include <stdlib.h>
#include <iostream>
#include "IgniteEngine.h"
#include "Common/Utils/MLog.h"
#include "Common/Modules/IModule.h"
#include "Common/Modules/IListener.h"
#include "Common/Modules/IMidiEndPoint.h"

#include "Firmware/Application/Model/PatternManager.h"
#include "Firmware/Application/Model/FrontPannelManager.h"

#include "Firmware/Application/Model/SettingsManager/SettingsMidiCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsCvioCnfg.h"

#include "Firmware/Application/Model/SettingsManager/SettingsSongCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsSceneCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsTrackCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsTrackInstrumentMixer.h"
#include "Firmware/Application/Model/SettingsManager/SettingsPatternCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsRecordCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsBRayCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsRemoteControlCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsSculptModeCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsUdsCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsMidiClkCnfg.h"
#include "Common/Modules/CMidiClock.h"
#include "Firmware/Application/Model/SettingsManager/SettingsTransposeControlCnfg.h"
#include "Firmware/Application/Model/SettingsManager/SettingsMidiMachineControlCnfg.h"

using namespace Ignite;
using namespace Ignite::Midi;

std::unique_ptr<IgniteEngine> _PIgniteEngine = nullptr;
std::unique_ptr<SongManager> _PSongMgr = nullptr;
std::unique_ptr<SceneManager> _PSceneMgr = nullptr;
std::unique_ptr<TrackManager> _TrackMgr = nullptr;
std::unique_ptr<PatternManager> _PPatternMgr = nullptr;
std::unique_ptr<InstrumentManager> _PInstrumentMgr = nullptr;
std::unique_ptr<FrontPannelManager> _PFrontPannelMgr = nullptr;
std::unique_ptr<EngineFileHandler> FileHandler = nullptr;
std::unique_ptr<SettingsMidiCnfg> _settingsMidiCnfg = nullptr;
std::unique_ptr<SettingsCvioCnfg> _settingsCvioCnfg = nullptr;
std::unique_ptr<SettingsGeneralCnfg> _settingsGeneralCnfg = nullptr;

std::unique_ptr<SettingsSongCnfg> _settingsSongCnfg = nullptr;
std::unique_ptr<SettingsSceneCnfg> _settingsSceneCnfg = nullptr;
std::unique_ptr<SettingsTrackCnfg> _settingsTrackCnfg = nullptr;
std::unique_ptr<SettingsTrackInstrumentMixer> _settingsTrackInstrumentMixer = nullptr;
std::unique_ptr<SettingsPatternCnfg> _settingsPatternCnfg = nullptr;
std::unique_ptr<SettingsRecordCnfg> _settingsRecordCnfg = nullptr;
std::unique_ptr<SettingsBRayCnfg> _settingsBRayCnfg = nullptr;
std::unique_ptr<SettingsRemoteControlCnfg> _settingsRemoteControlCnfg = nullptr;
std::unique_ptr<SettingsSculptModeCnfg> _settingsSculptModeCnfg = nullptr;
std::unique_ptr<SettingsUdsCnfg> _settingsUdsCnfg = nullptr;
std::unique_ptr<SettingsMidiClkCnfg> _settingsMidiClkCnfg = nullptr;
std::unique_ptr<SettingsTransposeControlCnfg> _settingsTransposeControlCnfg = nullptr;
std::unique_ptr<SettingsMidiMachineControlCnfg> _settingsMidiMachineControlCnfg = nullptr;

std::unique_ptr<SequenceManager> _pSequenceManager = nullptr;

IDevice* EngineInit (void)
{
	//Create engine
	_PIgniteEngine = std::make_unique<IgniteEngine> ();
	_PIgniteEngine->initialize (_PSongMgr.get (), _PPatternMgr.get (), _PInstrumentMgr.get ());
	return (IDevice*)_PIgniteEngine.get ();
}

IgniteEngine::IgniteEngine () : engineThread ("engine", IgniteConstants::threadStackSize, osPriorityHigh),
engineQueue (IgniteConstants::msgQueueSize),
engineMsgQ (IgniteConstants::msgQueueSize)
{
	Module* module = nullptr;

	FileHandler = std::make_unique<EngineFileHandler> (*this);
	fileHandler = FileHandler.get ();

	//Create Pattern Mgr
	_PPatternMgr = std::make_unique<PatternManager> (this, *this);
	module = (Module*)_PPatternMgr.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	//Create Song Mgr
	_PSongMgr = std::make_unique<SongManager> (this, *this);
	_PSongMgr->setInstance(_PSongMgr.get());
	module = (Module*)_PSongMgr.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}
	//Create Scene Mgr
	_PSceneMgr = std::make_unique<SceneManager> (this, *this, _PSongMgr.get ());
	module = (Module*)_PSceneMgr.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}
	//Create Track Mgr
	_TrackMgr = std::make_unique<TrackManager> (this, *this, _PSongMgr.get ());
	trackManager = _TrackMgr.get ();
	module = (Module*)_TrackMgr.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	CMidiClock::getInstance ().initialize (this);
#if 0//#ifdef ENABLE_PARALLEL_EXE
	//#include <ppl.h> // TODO: not include in Linux
	//TODO : UI may not sync with engine// resolve in linux build
	concurrency::parallel_invoke (
		[this] {_PPatternMgr = std::make_unique<PatternManager> (this, *this); Module* module = (Module*)_PPatternMgr.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_PInstrumentMgr = std::make_unique<InstrumentManager> (this, *this); Module* module = (Module*)_PInstrumentMgr.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_PFrontPannelMgr = std::make_unique<FrontPannelManager> (this, *this); Module* module = (Module*)_PFrontPannelMgr.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsMidiCnfg = std::make_unique<SettingsMidiCnfg> (this, *this); Module* module = (Module*)_settingsMidiCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsCvioCnfg = std::make_unique<SettingsCvioCnfg> (this, *this); Module* module = (Module*)_settingsCvioCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsGeneralCnfg = std::make_unique<SettingsGeneralCnfg> (this, *this); Module* module = (Module*)_settingsGeneralCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsSongCnfg = std::make_unique<SettingsSongCnfg> (this, *this); Module* module = (Module*)_settingsSongCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsSceneCnfg = std::make_unique<SettingsSceneCnfg> (this, *this); Module* module = (Module*)_settingsSceneCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsTrackCnfg = std::make_unique<SettingsTrackCnfg> (this, *this); Module* module = (Module*)_settingsTrackCnfg.get (); _modules.emplace (module->GetModuleID (), module); }
	);

	concurrency::parallel_invoke (
		[this] {_settingsTrackInstrumentMixer = std::make_unique<SettingsTrackInstrumentMixer> (this, *this); Module* module = (Module*)_settingsTrackInstrumentMixer.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsPatternCnfg = std::make_unique<SettingsPatternCnfg> (this, *this); Module* module = (Module*)_settingsPatternCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsRecordCnfg = std::make_unique<SettingsRecordCnfg> (this, *this); Module* module = (Module*)_settingsRecordCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsBRayCnfg = std::make_unique<SettingsBRayCnfg> (this, *this); Module* module = (Module*)_settingsBRayCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsRemoteControlCnfg = std::make_unique<SettingsRemoteControlCnfg> (this, *this); Module* module = (Module*)_settingsRemoteControlCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsSculptModeCnfg = std::make_unique<SettingsSculptModeCnfg> (this, *this); Module* module = (Module*)_settingsSculptModeCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsUdsCnfg = std::make_unique<SettingsUdsCnfg> (this, *this); Module* module = (Module*)_settingsUdsCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsMidiClkCnfg = std::make_unique<SettingsMidiClkCnfg> (this, *this); Module* module = (Module*)_settingsMidiClkCnfg.get (); _modules.emplace (module->GetModuleID (), module); },
		[this] {_settingsTransposeControlCnfg = std::make_unique<SettingsTransposeControlCnfg> (this, *this); Module* module = (Module*)_settingsTransposeControlCnfg.get (); _modules.emplace (module->GetModuleID (), module); }
	);
#else


	//Create Instrument Mgr
	_PInstrumentMgr = std::make_unique<InstrumentManager> (this, *this);
	instrumentManager = _PInstrumentMgr.get (); // Instrument required by Settings Mixer

	module = (Module*)_PInstrumentMgr.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_PFrontPannelMgr = std::make_unique<FrontPannelManager> (this, *this);
	module = (Module*)_PFrontPannelMgr.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsMidiCnfg = std::make_unique<SettingsMidiCnfg> (this, *this);
	module = (Module*)_settingsMidiCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}
	_settingsCvioCnfg = std::make_unique<SettingsCvioCnfg> (this, *this);
	module = (Module*)_settingsCvioCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}
	_settingsGeneralCnfg = std::make_unique<SettingsGeneralCnfg> (this, *this);
	module = (Module*)_settingsGeneralCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}
	_settingsSongCnfg = std::make_unique<SettingsSongCnfg> (this, *this);
	module = (Module*)_settingsSongCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsSceneCnfg = std::make_unique<SettingsSceneCnfg> (this, *this);
	module = (Module*)_settingsSceneCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsTrackCnfg = std::make_unique<SettingsTrackCnfg> (this, *this);
	module = (Module*)_settingsTrackCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}
	_settingsTrackInstrumentMixer = std::make_unique<SettingsTrackInstrumentMixer> (this, *this);
	module = (Module*)_settingsTrackInstrumentMixer.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsPatternCnfg = std::make_unique<SettingsPatternCnfg> (this, *this);
	module = (Module*)_settingsPatternCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsRecordCnfg = std::make_unique<SettingsRecordCnfg> (this, *this);
	module = (Module*)_settingsRecordCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsBRayCnfg = std::make_unique<SettingsBRayCnfg> (this, *this);
	module = (Module*)_settingsBRayCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsRemoteControlCnfg = std::make_unique<SettingsRemoteControlCnfg> (this, *this);
	module = (Module*)_settingsRemoteControlCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsSculptModeCnfg = std::make_unique<SettingsSculptModeCnfg> (this, *this);
	module = (Module*)_settingsSculptModeCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsUdsCnfg = std::make_unique<SettingsUdsCnfg> (this, *this);
	module = (Module*)_settingsUdsCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}
	_settingsMidiClkCnfg = std::make_unique<SettingsMidiClkCnfg> (this, *this);
	module = (Module*)_settingsMidiClkCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsTransposeControlCnfg = std::make_unique<SettingsTransposeControlCnfg> (this, *this);
	module = (Module*)_settingsTransposeControlCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_settingsMidiMachineControlCnfg = std::make_unique<SettingsMidiMachineControlCnfg> (this, *this);
	module = (Module*)_settingsMidiMachineControlCnfg.get ();
	if (_modules.find (module->GetModuleID ()) == _modules.end ()) {
		_modules.emplace (module->GetModuleID (), module);
	}

	_pSequenceManager = std::make_unique <SequenceManager>(this, *this);
	module = (Module*)_pSequenceManager.get();
	if (_modules.find(module->GetModuleID()) == _modules.end())
	{
		_modules.emplace(module->GetModuleID(), module);
	}
#endif //ENABLE_PARALLEL_EXE
	//Create engine state machine
	

	updateModuleReadyStatus((moduleId_t)ModuleName::moduleEngState, true);
	updateModuleReadyStatus((moduleId_t)ModuleName::moduleUndo, true);
	updateModuleReadyStatus((moduleId_t)ModuleName::moduleUI, true); //TODO: set from UI after subscribe?
}

void IgniteEngine::initialize (SongManager* songmgr, PatternManager* patternmgr, InstrumentManager* instrumentmgr)
{
	songManager = songmgr;
	sceneManager = _PSceneMgr.get();
	patternManager = patternmgr;
	instrumentManager = instrumentmgr;
	frontPannelManager = _PFrontPannelMgr.get ();
	settingsCvioCnfg = _settingsCvioCnfg.get ();
	settingsSceneCnfg = _settingsSceneCnfg.get();
	settingsTrackCnfg = _settingsTrackCnfg.get();
	settingsSongCnfg = _settingsSongCnfg.get();
	settingsPatternCnfg = _settingsPatternCnfg.get();
	settingsMidiClkCnfg = _settingsMidiClkCnfg.get();
	settingsBrayCnfg = _settingsBRayCnfg.get();
	settingsTransposeControlCnfg = _settingsTransposeControlCnfg.get();
	settingsGeneralCnfg = _settingsGeneralCnfg.get();
	settingsMidiMachineControlCnfg = _settingsMidiMachineControlCnfg.get ();
	settingsRecordCnfg = _settingsRecordCnfg.get ();
	settingsMidiCnfg = _settingsMidiCnfg.get();

	sequenceManager = _pSequenceManager.get();

	if ((songManager == nullptr) || (patternManager == nullptr) || (instrumentManager == nullptr))
	{
		return;
	}

	//module inits - with dependancies on other modules
	songManager->init();
	sequenceManager->init();
	settingsMidiClkCnfg->init();

	auto enginethreadcb = [](void* context) { static_cast<IgniteEngine*> (context)->engineThreadFunc (); };

	engineThread.startThread (enginethreadcb, this);

	_PFrontPannelMgr->FrontPannelStartThreads ();
}

void IgniteEngine::engineThreadFunc (void)
{
	IgniteMessageParam msg;
	/* Do housekeeping - after all module init */
	enginePowerOnHousekeeping();
	while (!CThread::desktopShutdownRequested())
	{
		if (!engineMsgQ.get(msg, TimeOut::kWaitForever)) break;
		auto stateLock = lockDesktopState();
		if (_modules.find (msg.moduleDst) != _modules.end ())
		{
			if (msg.messageType == MsgType::msgSetParam)
			{
				_modules[msg.moduleDst]->setParam (msg.moduleSrc, msg.param, (msg.MsgValue), msg.isPlayMode);
			}
			else if (msg.messageType == MsgType::msgSetParamList)
			{
				_modules[msg.moduleDst]->setParamList (msg.moduleSrc, (msg.paramlist), msg.isPlayMode);
			}
			else if (msg.messageType == MsgType::msgGetParam)
			{
				_modules[msg.moduleDst]->getParam (msg.moduleSrc, msg.param);
			}
			else if (msg.messageType == MsgType::msgSetCommandParam)
			{
				_modules[msg.moduleDst]->SetCommandParam (msg.moduleSrc, msg.param, msg.isPlayMode);
			}
		}
	}
}
void IgniteEngine::shutdownDesktop()
{
#ifdef _WIN32
    CThread::requestDesktopShutdown();
    juce::Logger::writeToLog("Shutdown: stopping clock");
    CMidiClock::getInstance().shutdownDesktop();
    juce::Logger::writeToLog("Shutdown: stopping song workers");
    songManager->shutdownDesktop();
    juce::Logger::writeToLog("Shutdown: stopping panel workers");
    frontPannelManager->shutdownDesktop();
    juce::Logger::writeToLog("Shutdown: stopping engine worker");
    engineThread.stopThread();
    juce::Logger::writeToLog("Shutdown: engine workers stopped");
#endif
}
void IgniteEngine::addListener (IListener* listener)
{
	auto stateLock = lockDesktopState();
	if (_listeners.find (listener) == _listeners.end ()) {
		_listeners.emplace (listener, Subscriptions ());
	}
}

std::vector<IgniteParamList> IgniteEngine::getParam (moduleId_t moduleSrc, moduleId_t moduleDst, std::vector<IgniteParamList> paramlist)
{
	auto stateLock = lockDesktopState();
	IgniteMessageParam msg;
	msg.moduleDst = moduleDst;
	msg.moduleSrc = moduleSrc;
	msg.param = 0;
	msg.MsgValue = 0;
	msg.messageType = MsgType::msgGetParamList;

	if (!paramlist.empty ())
	{
		copy (paramlist.begin (), paramlist.end (), back_inserter (msg.paramlist));
	}
	//no Msg Q for get list, to improve performance
	if (_modules.find (msg.moduleDst) != _modules.end ())
	{
		return (_modules[msg.moduleDst]->getParamList (msg.moduleSrc, (msg.paramlist)));
	}
	else
	{
		std::vector<IgniteParamList> retParam;
		return retParam;
	}
}
void IgniteEngine::getParam (moduleId_t moduleSrc, moduleId_t moduleDst, paramId_t param)
{
	auto stateLock = lockDesktopState();
	IgniteMessageParam msg;
	msg.moduleDst = moduleDst;
	msg.moduleSrc = moduleSrc;
	msg.param = param;
	msg.MsgValue = 0;
	msg.messageType = MsgType::msgGetParam;
	engineMsgQ.put (msg, TimeOut::kNoWait);
}

void IgniteEngine::setParam (moduleId_t moduleSrc, moduleId_t moduleDst, std::vector<IgniteParamList> paramlist, bool isPlayMode)
{
	IgniteMessageParam msg;
	msg.moduleDst = moduleDst;
	msg.moduleSrc = moduleSrc;
	msg.param = 0;
	msg.MsgValue = 0;
	msg.isPlayMode = isPlayMode;
	msg.messageType = MsgType::msgSetParamList;

	if (!paramlist.empty ())
	{
		copy (paramlist.begin (), paramlist.end (), back_inserter (msg.paramlist));
	}
	engineMsgQ.put (msg, TimeOut::kNoWait);
}
void IgniteEngine::setParam (moduleId_t moduleSrc, moduleId_t moduleDst, paramId_t param, const AnyType value, bool isPlayMode)
{
	IgniteMessageParam msg;
	msg.moduleDst = moduleDst;
	msg.moduleSrc = moduleSrc;
	msg.param = param;
	msg.MsgValue = value;
	msg.messageType = MsgType::msgSetParam;
	msg.isPlayMode = isPlayMode;
	engineMsgQ.put (msg, TimeOut::kNoWait);
}

// send commands bypassing queue - Blocking call
bool IgniteEngine::SetCommand (moduleId_t moduleSrc, moduleId_t moduleDst, paramId_t param, AnyType value, bool isPlayMode)
{
	auto stateLock = lockDesktopState();
	if (_modules.find (moduleDst) != _modules.end ())
	{
		_modules[moduleDst]->setParam (moduleSrc, param, value, isPlayMode);
		return true; //TODO get return from module level API
	}
	else
	{
		return false;
	}
}

void IgniteEngine::SetCommandParam (moduleId_t moduleSrc, moduleId_t moduleDst, paramId_t param, bool isPlayMode)
{
	IgniteMessageParam msg;
	msg.moduleDst = moduleDst;
	msg.moduleSrc = moduleSrc;
	msg.param = param;
	msg.MsgValue = 0;
	msg.messageType = MsgType::msgSetCommandParam;
	msg.isPlayMode = isPlayMode;
	engineMsgQ.put (msg, TimeOut::kNoWait);
}

void IgniteEngine::subscribe (moduleId_t module, paramId_t param, IListener* src)
{
	auto stateLock = lockDesktopState();
	if (_listeners.find (src) != _listeners.end ()) {
		if (_modules.find (module) != _modules.end ()) {
			if (param) {
				ParameterBase* pParam = _modules[module]->FindParameter (param);
				if (pParam) {
					_listeners[src].add (pParam);
					// Publish parameter
					//src->publish(module, module, param, pParam->getValue(), false);
				}
			}
			else { //(paramall)- all parameter subscribe
				_listeners[src].add (_modules[module]);
				// Publish all parameters
				for (uint8_t id = 1; id <= _modules[module]->GetParameterCount (); id++) {
					ParameterBase* pParam = _modules[module]->FindParameter (id);
					if (pParam) {
						//src->publish(module, module, id, pParam->getValue(), false);
					}
				}
			}
		}
	}
}
void IgniteEngine::unsubscribe (moduleId_t module, paramId_t param, IListener* src)
{
	auto stateLock = lockDesktopState();
	if (_listeners.find (src) != _listeners.end ()) {
		std::cout << "Unsubscribe Param, Module Id: " << static_cast<uint32_t>(module) << ", Param Id: " << static_cast<uint32_t>(param) << std::endl;
		if (_modules.find (module) != _modules.end ()) {
			if (param) { // Individual param unsubscribe
				ParameterBase* pParam = _modules[module]->FindParameter (param);
				if (pParam) {
					_listeners[src].remove (pParam);
				}
			}
			else { // All param unsubscribe
				// UnSubscribe Module
				_listeners[src].remove (_modules[module]);
				// UnSubscribe All Params
				for (uint8_t id = 1; id <= _modules[module]->GetParameterCount (); id++) {
					ParameterBase* pParam = _modules[module]->FindParameter (id);
					if (pParam) {
						_listeners[src].remove (pParam);
					}
				}
			}
		}
	}
}
bool IgniteEngine::SaveAllData()
{
#ifdef _WIN32
    auto stateLock=lockDesktopState();
    desktopChanges.beginSave();
    bool saved=true;
    try
    {
        // A complete explicit save includes previously edited library items.
        if(songManager->activeSong) saved=songManager->saveSong() && saved;
        for(auto& instrument:instrumentManager->instrumentList) saved=instrumentManager->SaveInstrument(instrument.getName()) && saved;
        for(auto& pattern:patternManager->patternList) saved=patternManager->SavePattern(&pattern,true) && saved;
        saved=static_cast<SettingsGeneralCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingGnrlCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsMidiCnfg*>(_modules.at(moduleId_t(ModuleName::moduleMidiCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsCvioCnfg*>(_modules.at(moduleId_t(ModuleName::moduleCvioCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsSongCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingSongCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsSceneCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingSceneCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsTrackCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsTrackCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsPatternCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsPatternCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsRecordCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsRecordCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsBRayCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsBRayCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsRemoteControlCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsRemoteControlCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsSculptModeCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsSculptModeCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsUdsCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsUdsCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsMidiClkCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsMidiClkCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsTransposeControlCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsTransposeControlCnfg)))->saveForDesktop() && saved;
        saved=static_cast<SettingsMidiMachineControlCnfg*>(_modules.at(moduleId_t(ModuleName::moduleSettingsMidiMachineControlCnfg)))->saveForDesktop() && saved;

    }
    catch(const std::exception& error)
    {
        juce::Logger::writeToLog("Save failed: "+juce::String(error.what())); saved=false;
    }
    if(saved) patternManager->desktopMarkSaved();
    return desktopChanges.finishSave(saved);
#else

	auto stateLock = lockDesktopState();
    auto desktopSaveParam = [this](moduleId_t source, moduleId_t destination, paramId_t parameter, int value)
    {
#ifdef _WIN32
        SetCommand(source, destination, parameter, value);
#else
        setParam(source, destination, parameter, value);
#endif
    };
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::modulePattern, (paramId_t)PatternParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleScene, (paramId_t)SceneParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsBRayCnfg, (paramId_t)SettingsBRayCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleCvioCnfg, (paramId_t)SettingsCvioCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsMidiClkCnfg, (paramId_t)SettingsMidiClkCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsMidiMachineControlCnfg, (paramId_t)SettingsMidiMachineControlCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsPatternCnfg, (paramId_t)SettingsPatternCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsRecordCnfg, (paramId_t)SettingsRecordCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsRemoteControlCnfg, (paramId_t)SettingsRemoteControlCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingSceneCnfg, (paramId_t)SettingsSceneCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsSculptModeCnfg, (paramId_t)SettingsSculptModeCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingSongCnfg, (paramId_t)SettingsSongCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsTrackCnfg, (paramId_t)SettingsTrackCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsTrackInstrumentMixer, (paramId_t)SettingsTrackInstrumentMixerParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsTransposeControlCnfg, (paramId_t)SettingsTransposeControlCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingsUdsCnfg, (paramId_t)SettingsUdsCnfgParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSaveAll, 0);
	desktopSaveParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack, (paramId_t)TrackParam::paramSaveAll, 0);

	return true;
#endif
}
void IgniteEngine::onParameterChanged (moduleId_t moduleSrc, ParameterBase* param)
{
	auto stateLock = lockDesktopState();
	for (auto& it : _listeners) {
		// Check if Subscription exists for param or module
		if (it.second.exist (param) || it.second.exist (&param->GetModule ()))
		{
			it.first->publish (param->GetModule ().GetModuleID (), moduleSrc, param->GetParamID (), param->getValue (), false);
		}
	}
}

void IgniteEngine::onParameterEdited(ParameterBase* parameter)
{
#ifdef _WIN32
    const auto module=parameter->GetModule().GetModuleID();
    if(module==(moduleId_t)ModuleName::moduleInstrument && instrumentManager)
        desktopEdited("instrument:"+instrumentManager->desktopActiveName());
    else if(module>=(moduleId_t)ModuleName::moduleSong && module<=(moduleId_t)ModuleName::moduleTrack && songManager && songManager->activeSong)
        desktopEdited("song");
    else if(module>=9 && module<=24) desktopEdited("settings:"+std::to_string(module));
#endif
}

void IgniteEngine::SendResponse (moduleId_t module, moduleId_t moduleSrc, paramId_t param, const AnyType& value)
{
	auto stateLock = lockDesktopState();
	ParameterBase* pParam = _modules[module]->FindParameter (param);
	if (pParam != NULL)
	{
		for (auto& it : _listeners)
		{
			// Send back to the requested module
			if ((it.second.exist(pParam) || it.second.exist(&pParam->GetModule())) && (moduleSrc == it.first->moduleId))
			{
				it.first->publish(module, moduleSrc, param, value, true);
			}
		}
	}
	//else TODO: Warn
}

std::list<std::string> IgniteEngine::EngineGetPatternList (bool isDisplayName)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrGetPatterList (isDisplayName);
}
ClientPatternData IgniteEngine::EngineGetPatternData (std::string patternName)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrGetPatternData (patternName);
}
std::list<ClienPatterntBarData>  IgniteEngine::EngineGetBarData (std::string patternName)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrGetBarData (patternName);
}
std::list<ClientPatternStepData> IgniteEngine::EngineGetStepData (std::string patternName, uint16_t BarId)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrGetStepData (patternName, BarId);
}

bool IgniteEngine::EngineSetPatternData (moduleId_t moduleSrc, std::string patternName, ClientPatternData patternData, bool isPlayModeData, PatternDataUpdateReasonType UpdateReasonType )
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrSetPatternDataOptimized (moduleSrc, patternName, &patternData, UpdateReasonType, isPlayModeData);
	//return patternManager->PatternMngrSetPatternData (moduleSrc, patternName, patternData, UpdateReasonType, isPlayModeData);
}

#if 0
bool IgniteEngine::EngineSetPatternData(moduleId_t moduleSrc, std::string patternName, ClientPatternData& patternData, PatternDataUpdateLocation location, bool isPlayModeData, PatternDataUpdateReasonType UpdateReasonType)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrSetPatternDataOptimized(moduleSrc, patternName, patternData, location, UpdateReasonType, isPlayModeData);
}
#endif

bool IgniteEngine::EngineSetBarData (moduleId_t moduleSrc, std::string patternName, uint16_t BarId, ClienPatterntBarData barData)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrSetBarData (moduleSrc, patternName, BarId, barData);
}
bool IgniteEngine::EngineSetStepData (moduleId_t moduleSrc, std::string patternName, uint16_t BarId, uint16_t stepId, ClientPatternStepData stepData)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrSetStepData (moduleSrc, patternName, BarId, stepId, stepData);
}
bool IgniteEngine::EngineCopyPatternBar (std::string patternName, uint16_t srcBarId, uint16_t destBarId)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternCopyBar (patternName, srcBarId, destBarId);
}

bool  IgniteEngine::EngineAddPatternBar (std::string patternName, bool CopyAndCreate, uint16_t FromBarId, uint16_t NumBarToAdd)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternAddBar (patternName, CopyAndCreate, FromBarId, NumBarToAdd);
}

bool  IgniteEngine::EngineInsertPatternBar (std::string patternName, uint16_t InsertPosition)
{
	auto stateLock = lockDesktopState();
	return patternManager->patternInsertBar (patternName, InsertPosition);
}

bool IgniteEngine::EngineRemovePatternBar (std::string patternName, uint16_t BarId, uint16_t NumBarToRemove)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternRemoveBar (patternName, BarId, NumBarToRemove);
}

bool IgniteEngine::EngineInsertPatternStep (std::string patternName, uint16_t BarId, uint8_t stepCount, uint8_t InsertAt, std::vector<ClientPatternStepData> ToStepDataList)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternInsertStep(patternName,BarId,stepCount,InsertAt,{ToStepDataList.begin(),ToStepDataList.end()});
}

bool IgniteEngine::EngineAddNodeData (std::string patternName, uint16_t BarId, uint16_t stepId, ClientPatternNodeData nodeData)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrAddNodeData (patternName, BarId, stepId, nodeData, false);
}

bool IgniteEngine::EngineAddNodeData (std::string patternName, uint16_t BarId, std::map<ClientPatternNodeData, std::list<uint8_t>> stepMap)
{
	auto stateLock = lockDesktopState();

	return patternManager->PatternMngrAddNodeData (patternName, BarId, stepMap, false);
}

bool IgniteEngine::EngineDelNodeData (std::string patternName, uint16_t BarId, uint16_t stepId, uint16_t nodeData)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternMngrDelNodeData (patternName, BarId, stepId, nodeData, false);
}

bool IgniteEngine::EnginecopyPattern (std::string SrcpatternName, std::string DstpatternName)
{
	auto stateLock = lockDesktopState();
	return patternManager->copyPattern (SrcpatternName, DstpatternName);
}

int8_t IgniteEngine::getSceneBarCount (void)
{
	auto stateLock = lockDesktopState();
	if ((songManager == nullptr) || (songManager->getActiveSong () == nullptr) || (songManager->getActiveSong ()->getEditScene () == nullptr))
	{
		return -1;
	}
	return (int8_t)songManager->getActiveSong ()->getEditScene ()->getBarCount ();
}
int8_t IgniteEngine::getSceneGbarStepCount (void)
{
	auto stateLock = lockDesktopState();
	if ((songManager == nullptr) || (songManager->getActiveSong () == nullptr) || (songManager->getActiveSong ()->getEditScene () == nullptr))
	{
		return -1;
	}

	return songManager->getActiveSong ()->getEditScene ()->getStepsPerBar ();
}
std::list<PatternDataUpdateReasonType> IgniteEngine::EnginePatternGetUndoList (void)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternGetUndoList ();
}
bool IgniteEngine::PatternMngrPatternRotate (std::string patternName, uint16_t BarID, uint16_t StepID)
{
	auto stateLock = lockDesktopState();
	return patternManager->PatternRotate (patternName, BarID, StepID);
}
void IgniteEngine::setAutoSaveAllModules(bool status)
{
	_autoSaveStatus = status;
}
bool IgniteEngine::getAutoSaveStatus(void) 
{ 
	return _autoSaveStatus;
};
// Handle abrupt power down
// Uncontrolled data loss handling
void IgniteEngine::enginePowerOnHousekeeping (void)
{
	return;
	//TODO : implement to handle uncontrolled power down
	/*patternManager->PatternMngrPowerOnHousekeeping ();
	instrumentManager->InstrumentMngrPowerOnHousekeeping ();*/
}

void IgniteEngine::updateModuleReadyStatus(moduleId_t moduleId, bool isReady)
{
	if (moduleId >= 0 && moduleId < (moduleId_t)ModuleName::moduleMax)
	{
		bool engineState = isEngineReady;
		if ((modulestatus[moduleId] != (moduleId_t)isReady))
		{
			modulestatus[moduleId] = (moduleId_t)isReady;
			isEngineReady = true;
			for (auto dev : modulestatus)// check each module status
			{
				if (dev == 0)
				{
					isEngineReady = false;
					break;
				}
			}
			if(engineState != isEngineReady) // if state changed inform UI to show pop-up
				SendResponse((moduleId_t)ModuleName::moduleSong, (moduleId_t)ModuleName::moduleUI, (paramId_t)SongParam::ParamEngineReady, (uint8_t)isEngineReady);
		}
	}
}
