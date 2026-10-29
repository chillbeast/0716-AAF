#include <string>
#include "Firmware/Application/Model/InstrumentManager.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Firmware/Application/Model/ModelParams.h"

using namespace Ignite;

InstrumentManager::InstrumentManager (IParamChangeListener* listener, IgniteEngine& model) : paramlistener (listener), engine (model), Module ("InstrumentManager", (moduleId_t)ModuleName::moduleInstrument, (unsigned int)InstrumentParam::paramInstrumentMax, listener)
{
	numberOfInstruments = 0;
	currentInstrumentName = "";
	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramInstrumentAdd, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)InstrumentParam::paramSetActiveInstrument, "0", "0", "0");
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramInstrumentChannel, 0, 15, -1);
	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramInstrumentPort, 0, 127, 0);
	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramInstrumentRemove, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)InstrumentParam::paramInstrumentName, "0", "0", "0");
	AddParameter<std::string> ((paramId_t)InstrumentParam::paramInstrumentDispName, "0", "0", "0");
	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramInstrumentNameList, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)InstrumentParam::paramInstrumentSave, "0", "0", "0");

	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramUndoCmd, 0, 127, 0);

	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramDefaultNote, 0, 127, 0);
	AddParameter<int> ((paramId_t)InstrumentParam::paramInternalSoundPgm, -1, 0x7fffffff, 0);//depends on sound font file, max value in current SF is < 1000
	AddParameter<bool> ((paramId_t)InstrumentParam::paramInternalSound, false, true, false);
    AddParameter<std::string>(paramId_t(InstrumentParam::paramDesktopSoundFontPath),"","","");
	AddParameter<bool> ((paramId_t)InstrumentParam::paramMultiTimbralStatus, false, true, false);
	AddParameter<bool> ((paramId_t)InstrumentParam::paramPolySpreadStatus, false, true, false);
	AddParameter<bool> ((paramId_t)InstrumentParam::paramTransposeDefeatStatus, false, true, false);
	AddParameter<bool> ((paramId_t)InstrumentParam::paramFtsDefeatStatus, false, true, false);
	AddParameter<bool> ((paramId_t)InstrumentParam::paramEditTrackThruStatus, false, true, false);
	AddParameter<bool> ((paramId_t)InstrumentParam::paramCc0BankMStatus, false, true, false);
	AddParameter<bool> ((paramId_t)InstrumentParam::paramCc0BankLStatus, false, true, false);
	AddParameter<bool> ((paramId_t)InstrumentParam::paramDisplayNoteNum, false, true, false);
	AddParameter<bool> ((paramId_t)InstrumentParam::paramPreSendPgmChange, false, true, false);

	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramClearUndoList, 0, 127, 0);
	AddParameter<std::string> ((paramId_t)InstrumentParam::paramUsedByListCmd, "0", "0", "0");
	AddParameter<std::string> ((paramId_t)InstrumentParam::paramUsedBySongList, "0", "0", "0");
	AddParameter<std::string>((paramId_t)InstrumentParam::paramUsedBySceneList, "0", "0", "0");
	AddParameter<std::string>((paramId_t)InstrumentParam::paramUsedByTrackList, "0", "0", "0");

	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVActiveKnobCmd, 0, 23, 0);// 8 knobs (0-7) supported Firmware 1
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVType, 0, 3, 0);//support only midi cc, alt midi cc & pgm change for Firmware 1
	AddParameter<std::string> ((paramId_t)InstrumentParam::paramTVName, "0", "0", "0");
	AddParameter<bool> ((paramId_t)InstrumentParam::paramTVKnobPresenceQuery, 0, 1, 0);

	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVMidiCCNum, 0, 127, 0);
	AddParameter<float> ((paramId_t)InstrumentParam::paramTVMidiCCValue, -12.0, 20000.0, 0.0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVMidiCCInitValue, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVMidiCCMinValue, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVMidiCCMaxValue, 0, 127, 0);

	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVMidiPgmNum, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVMidiPgmBankLsb, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVMidiPgmBankMsb, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramInstrumentPgmNum, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramInstrumentPgmBankLsb, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramInstrumentPgmBankMsb, 0, 127, 0);

	AddParameter<int16_t> ((paramId_t)InstrumentParam::paramTVSeqQuantPercent, 0, 255, 0);
	AddParameter<int16_t> ((paramId_t)InstrumentParam::paramTVSeqVelocityPercent, 0, 255, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVSeqVelocityRef, 0, 127, 0);
	AddParameter<int16_t> ((paramId_t)InstrumentParam::paramTVSeqLengthPercent, 0, 2000, 0);
	AddParameter<int16_t> ((paramId_t)InstrumentParam::paramTVSeqNotePercent, 0, 255, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVSeqNoteRef, 0, 127, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVSeqTransposeSemiTone, -12, 12, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVSeqTransposeOctave, -4, 4, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVSeqFtsRootNote, (int8_t)FTSRootNote::NoteMin, (int8_t)FTSRootNote::NoteMax, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVSeqFtsScale, (int8_t)ScaleList::ScaleMin, (int8_t)ScaleList::ScaleMax, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTVSeqReichValue, -8, +8, 0);//PhaseShift

	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramTVSaveScope, (uint8_t)TrackValueSaveType::TVSaveActiveToScene, (uint8_t)TrackValueSaveType::TVSaveAllToSong, (uint8_t)TrackValueSaveType::TVSaveActiveToScene);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramAddTrackValueCmd, 0, 15, 0);//knob id
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramDeleteTrackValueCmd, 0, 15, 0);//knob id
	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramCountTrackValueCmd, 0, 8, 0);//8TVs max for Firmware1
	AddParameter<bool> ((paramId_t)InstrumentParam::paramTVKnobEnable, 0, 1, 1);

	AddParameter<std::string> ((paramId_t)InstrumentParam::paramInstrumentCopy, "0", "0", "0");
	AddParameter<uint64_t> ((paramId_t)InstrumentParam::paramInstrumentMoveCmd, 0, 0x3FF000003FF, 0);

	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTMActiveTrack, 0, 63, 0);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTMPan, 0, 127, 63);
	AddParameter<int8_t> ((paramId_t)InstrumentParam::paramTMVolume, 0, 127, 63);

	AddParameter<uint8_t> ((paramId_t)InstrumentParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<uint8_t> ((paramId_t)InstrumentParam::ParamDeleteAllInstrument, 0, 127, 0);

	AddParameter<bool>((paramId_t)InstrumentParam::paramTrackEffectMuteUnmute, 0, 1, 1);
	AddParameter<int8_t>((paramId_t)InstrumentParam::paramSaveAll, 0, 1, 0);

	std::vector<std::string> instrumentNameList = engine.fileHandler->getInstrumentNameList ();
	if (!instrumentNameList.empty ())
	{
		InstrumentData instrumentData = { {},{},0, };
		for (auto it = instrumentNameList.begin (); it != instrumentNameList.end (); ++it)
		{
			instrumentData = { {},{},0, };
			instrumentData = engine.fileHandler->GetInstrumentData (*it);
			instrumentList.push_back (Instrument (instrumentData, this));
			++numberOfInstruments;
		}

		sortInstrumentsById ();
		currentInstrumentName = (*instrumentNameList.begin ());
		SetDefaultUndoListData ();
	}
	_tmTrackId = -1;
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleInstrument, true);
}
void InstrumentManager::SetDefaultUndoListData ()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};
	ClearUndoDefaultList ();
	for (auto i = (paramId_t)InstrumentParam::paramall + 1; i < (paramId_t)InstrumentParam::paramInstrumentMax; i++)
	{
		if ((i == (paramId_t)InstrumentParam::paramInstrumentNameList) ||
			(i == (paramId_t)InstrumentParam::paramUsedByListCmd) ||
			(i == (paramId_t)InstrumentParam::paramUsedBySongList) ||
			(i == (paramId_t)InstrumentParam::paramUsedBySceneList) ||
			(i == (paramId_t)InstrumentParam::paramUsedByTrackList) ||
			(i == (paramId_t)InstrumentParam::paramUndoListCmd) ||
			(i == (paramId_t)InstrumentParam::paramTVActiveKnobCmd) ||
			(i == (paramId_t)InstrumentParam::paramTVSaveScope) ||
			(i == (paramId_t)InstrumentParam::paramTMActiveTrack) ||
			(i == (paramId_t)InstrumentParam::paramTMPan) ||
			(i == (paramId_t)InstrumentParam::paramTMVolume) ||
			(i == (paramId_t)InstrumentParam::paramInstrumentMoveCmd) ||
			(i == (paramId_t)InstrumentParam::paramTVMidiPgmBankLsb) ||
			(i == (paramId_t)InstrumentParam::ParamDeleteAllInstrument) ||
			(i == (paramId_t)InstrumentParam::paramTVMidiPgmBankMsb)
			)
		{
			//ignore command params, used in get param getParamList
		}
		else
		{
			AddToUndoDefaultList ((paramId_t)i, FindParameter ((paramId_t)i)->getValue ());
			paramlistdata.param = (paramId_t)i;
			paramlist.push_back (paramlistdata);
		}
	}

	retParam = getParamList ((moduleId_t)ModuleName::moduleInstrument, paramlist);
	for (std::vector<IgniteParamList>::iterator it = retParam.begin (); it != retParam.end () && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList (it->param, it->MsgValue);
	}
}
void InstrumentManager::setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		//set value for each, this calls onParameterChange
		FindParameter ((*it).param)->setValue (moduleSrc, (*it).MsgValue, isPlayMode);
	}
}

void InstrumentManager::onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		(InstrumentParam)param->GetParamID () != InstrumentParam::paramSetActiveInstrument &&
		(InstrumentParam)param->GetParamID () != InstrumentParam::paramUndoCmd &&
		(InstrumentParam)param->GetParamID () != InstrumentParam::paramInstrumentMoveCmd
		)
	{
		AddToUndoList (param);
	}
	ParamHandler (moduleSrc, param, true);
}

void InstrumentManager::ParamHandler (moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	Instrument* instrument = nullptr;
	if (moduleSrc != (moduleId_t)ModuleName::moduleInstrument)
	{
		if (!currentInstrumentName.empty ())
		{
			instrument = getInstrument (currentInstrumentName);
		}
		switch ((InstrumentParam)param->GetParamID ())
		{
			case InstrumentParam::paramUndoCmd:
				doParamUndo ();
				break;

			case InstrumentParam::paramClearUndoList:
				if (param->getValue().get<int8_t>() == 0)
					ResetToDefaultData();
				ClearUndoList ();
				break;
			case InstrumentParam::ParamDeleteAllInstrument:
				clearAllInstrument ();
				break;

			case InstrumentParam::paramInstrumentAdd:
				InstrumentManager::addInstrument ();
				break;

			case InstrumentParam::paramInstrumentRemove:
				engine.fileHandler->DeleteInstrument (currentInstrumentName);
				InstrumentManager::removeInstrument (currentInstrumentName);
				break;

			case InstrumentParam::paramSaveAll:
				if (GetUndoParamsInList().empty())
					break;
				//fallthrough
			case InstrumentParam::paramInstrumentSave:
			{
				SaveData();
			}
			break;

			case InstrumentParam::paramSetActiveInstrument:
				InstrumentManager::setActiveInstrument ((std::string)(param->getValue ().get<std::string> ().c_str ()));
				break;

			case InstrumentParam::paramInstrumentName:
				InstrumentManager::setInstrumentName (currentInstrumentName, (std::string)(param->getValue ().get<std::string> ().c_str ()));
				break;
			case InstrumentParam::paramInstrumentDispName:
				InstrumentManager::setInstrumentDispName (currentInstrumentName, (std::string)(param->getValue ().get<std::string> ().c_str ()));
				break;

			case InstrumentParam::paramInstrumentChannel:
				InstrumentManager::setInstrumentChannel (currentInstrumentName, param->getValue ().get<uint8_t> ());
				if (instrument)
				{
					for (auto iter : _instrumentListeners)
					{
						iter->handleMidiSettingChange(instrument->getId());
					}
				}
				break;

			case InstrumentParam::paramInstrumentPort:
				InstrumentManager::setInstrumentPort (currentInstrumentName, param->getValue ().get<uint8_t> ());
				if (instrument)
				{
					for (auto iter : _instrumentListeners)
					{
						iter->handleMidiSettingChange(instrument->getId());
					}
				}
				break;

			case InstrumentParam::paramDefaultNote:
				if (instrument)
					instrument->setDefaultNote (param->getValue ().get<uint8_t> ());
				break;

            case InstrumentParam::paramDesktopSoundFontPath:
                if(instrument)
                {
                    instrument->_instrumentData->soundFontPath=param->getValue().get<std::string>();
                    engine.songManager->desktopInstrumentFontChanged(instrument->getName());
                }
                break;
			case InstrumentParam::paramInternalSoundPgm:
				if (instrument)
				{
					int newProgram = param->getValue().get<int>();
					int currentProgram = instrument->getInternalSoundPgm();

					if (newProgram != currentProgram)
					{
						instrument->setInternalSoundPgm(newProgram);

						for (auto iter : _instrumentListeners)
						{
							iter->handleInternalPgmChange(instrument->getId(), newProgram);
						}
					}
				}
				break;

			case InstrumentParam::paramInternalSound:
				if (instrument)
					instrument->setInternalSound (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramMultiTimbralStatus:
				if (instrument)
					instrument->setMultiTimbralStatus (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramPolySpreadStatus:
				if (instrument)
					instrument->setPolySpreadStatus (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramTransposeDefeatStatus:
				if (instrument)
					instrument->setTransposeDefeatStatus (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramFtsDefeatStatus:
				if (instrument)
					instrument->setFtsDefeatStatus (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramEditTrackThruStatus:
				if (instrument)
					instrument->setEditTrackThruStatus (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramCc0BankMStatus:
				if (instrument)
					instrument->setCc0BankMStatus (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramCc0BankLStatus:
				if (instrument)
					instrument->setCc0BankLStatus (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramDisplayNoteNum:
				if (instrument)
					instrument->setDisplayNoteNum (param->getValue ().get<bool> ());
				break;

			case InstrumentParam::paramPreSendPgmChange:
				if (instrument)
					instrument->setPreSendPgmChange (param->getValue ().get<bool> ());
				break;

				// TRACK VALUE
			/* TV are created by default at instrument creation
			* new param added to enable/disable TV
			case InstrumentParam::paramAddTrackValueCmd:
				if (instrument)
				{
					instrument->addTVKnob (param->getValue ().get<int8_t> ());
				}
				break;
			*/

			case InstrumentParam::paramDeleteTrackValueCmd:
				if (instrument)
				{
					instrument->deleteTVKnob (param->getValue ().get<int8_t> ());
				}
				break;

			case InstrumentParam::paramTVActiveKnobCmd:
				if (instrument)
				{
					auto knobId = param->getValue ().get<int8_t> ();
					if (instrument->_instrumentData->tv.count (knobId) != 0)
					{
						instrument->setTVActiveKnob (knobId);
					}
					else
					{
						assert (0);
					}
				}
				break;

			case InstrumentParam::paramTVName:
				if (instrument)
				{
					instrument->setTVName (param->getValue ().get<std::string> ());
				}
				break;

			case InstrumentParam::paramTVType:
				if (instrument)
				{
					instrument->setTVParamType ( static_cast <TrackValueType> (param->getValue ().get<int8_t> ()));
				}
				break;

			case InstrumentParam::paramTVMidiCCNum:
				if (instrument)
				{
					instrument->setTVMidiCCNum (param->getValue ().get<int8_t> ());
				}
				break;

			case InstrumentParam::paramTVMidiCCValue:
				if (instrument)
				{
					instrument->setTVMidiCCValue (param->getValue ().get<float> ());
				}
				break;

			case InstrumentParam::paramTVMidiCCInitValue:
				if (instrument)
				{
					instrument->setTVMidiCCInitValue (param->getValue ().get<int8_t> ());
				}
				break;

			case InstrumentParam::paramTVMidiCCMinValue:
				if (instrument)
				{
					instrument->setTVMidiCCMinValue (param->getValue ().get<int8_t> ());
				}
				break;

			case InstrumentParam::paramTVMidiCCMaxValue:
				if (instrument)
				{
					instrument->setTVMidiCCMaxValue (param->getValue ().get<int8_t> ());
				}
				break;

			case InstrumentParam::paramInstrumentPgmNum:
				if (instrument)
				{
					instrument->setTVMidiPgmNumber (param->getValue ().get<int8_t> (), TvLocation::InstrumentLevel);
				}
				break;

			case InstrumentParam::paramInstrumentPgmBankMsb:
				if (instrument)
				{
					instrument->setTVMidiPgmBankMsb (param->getValue ().get<int8_t> (), TvLocation::InstrumentLevel);
				}
				break;

			case InstrumentParam::paramInstrumentPgmBankLsb:
				if (instrument)
				{
					instrument->setTVMidiPgmBankLsb (param->getValue ().get<int8_t> (), TvLocation::InstrumentLevel);
				}
				break;

			case InstrumentParam::paramTVMidiPgmNum:
				if (instrument)
				{
					instrument->setTVMidiPgmNumber (param->getValue ().get<int8_t> (), TvLocation::TrackLevel);
				}
				break;

			case InstrumentParam::paramTVMidiPgmBankMsb:
				if (instrument)
				{
					instrument->setTVMidiPgmBankMsb (param->getValue ().get<int8_t> (), TvLocation::TrackLevel);
				}
				break;

			case InstrumentParam::paramTVMidiPgmBankLsb:
				if (instrument)
				{
					instrument->setTVMidiPgmBankLsb (param->getValue ().get<int8_t> (), TvLocation::TrackLevel);
				}
				break;

				/*
				case InstrumentParam::paramTVSeqQuantPercent:
					if (instrument)
					{
						instrument->setTVQuantisationPercent(param->getValue().get<int16_t>());
					}
					break;
				*/

				/*
				case InstrumentParam::paramTVSeqVelocityPercent:
					if (instrument)
					{
						instrument->setTVVelocityPercent (param->getValue ().get<int16_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqVelocityRef:
					if (instrument)
					{
						instrument->setTVVelocityReference (param->getValue ().get<int8_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqLengthPercent:
					if (instrument)
					{
						instrument->setTVLengthPercent (param->getValue ().get<int16_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqNotePercent:
					if (instrument)
					{
						instrument->setTVNotePercent (param->getValue ().get<int16_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqNoteRef:
					if (instrument)
					{
						instrument->setTVNoteRef (param->getValue ().get<int8_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqTransposeSemiTone:
					if (instrument)
					{
						instrument->setTVTransposeBySemiTone (param->getValue ().get<int8_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqTransposeOctave:
					if (instrument)
					{
						instrument->setTVTransposeByOctave (param->getValue ().get<int8_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqFtsScale:
					if (instrument)
					{
						instrument->setTVFtsScale ((ScaleList)param->getValue ().get<int8_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqFtsRootNote:
					if (instrument)
					{
						instrument->setTVFtsRootNote ((FTSRootNote)param->getValue ().get<int8_t> ());
					}
					break;

				case InstrumentParam::paramTVSeqReichValue:
					if (instrument)
					{
						//instrument->setTVReich(param->getValue().get<int8_t>());
					}
					break;
				*/

			case InstrumentParam::paramTVSaveScope:
				if (instrument)
				{
					instrument->setTrackValueScope (param->getValue ().get<uint8_t> ());
				}
				break;

				//TODO:  TRACK MIXER only params moved from here to a module.
			case InstrumentParam::paramTMActiveTrack:
				setTrackMixerTrack (param->getValue ().get<uint8_t> ());
				break;

			case InstrumentParam::paramTMPan:
				setTrackMixerPanValue (param->getValue ().get<uint8_t> ());
				break;

			case InstrumentParam::paramTMVolume:
				setTrackMixerVolumeValue (param->getValue ().get<uint8_t> ());
				break;

			case InstrumentParam::paramInstrumentCopy:
				InstrumentManager::copyInstrument ((std::string)(param->getValue ().get<std::string> ().c_str ()));
				break;

			case InstrumentParam::paramInstrumentMoveCmd:
				InstrumentManager::moveInstrument (param->getValue ().get<uint64_t> ());
				break;
			case InstrumentParam::paramTrackEffectMuteUnmute:
				instrument->setTVEffectStatus(param->getValue().get<bool>());
					break;

			default:
				//assert(0);
				break;
		}
		if (engine.getAutoSaveStatus()|| engine.settingsGeneralCnfg->_SettingsGeneralCnfgData.InstrumentAutoSave)
		{
			SaveData();
		}
		if (SendNotification)
		{
			/* Send notification to the subscribed modules via engine */
			paramlistener->onParameterChanged (moduleSrc, param);
		}
	}
}
void InstrumentManager::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if (SaveInstrument(currentInstrumentName))
        ClearUndoList();

}
bool InstrumentManager::SaveInstrument(std::string name)
{
    if(name.empty() || engine.fileHandler->saveInstrument(name,getInstrumentData(name))!=FILE_OK) return false;
    engine.desktopSaved("instrument:"+name);
    if(name==currentInstrumentName) {SetDefaultUndoListData();ClearUndoList();}
    return true;
}

Instrument* InstrumentManager::getInstrument (std::string InstrumentName)
{
	for (auto it = instrumentList.begin (); it != instrumentList.end (); ++it)
	{
		if (InstrumentName == it->_instrumentData->instrumentName)
			return &(*it);
	}
	return nullptr;
}

InstrumentData InstrumentManager::getInstrumentData (std::string InstrumentName)
{
	InstrumentData instrumentData = { {},{},0, };
	for (auto it = instrumentList.begin (); it != instrumentList.end (); ++it)
	{
		if (InstrumentName == it->_instrumentData->instrumentName)
		{
			instrumentData = *it->_instrumentData;
		}
	}
	return instrumentData;
}

void InstrumentManager::SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	ParamHandler (moduleSrc, FindParameter (param), false);
}

std::vector<IgniteParamList> InstrumentManager::getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData;

	Instrument* instrument = nullptr;
	if (!currentInstrumentName.empty ())
	{
		instrument = getInstrument (currentInstrumentName);
	}

	//loop to fill all data and return the value
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin (); it != paramlist.end (); ++it)
	{
		switch ((InstrumentParam)(*it).param)
		{
			case InstrumentParam::paramInstrumentNameList:
			{
				struct IgniteParamList tempParam = { 0 };
				for (auto nameIter = instrumentList.begin (); nameIter != instrumentList.end (); ++nameIter)
				{
					tempParam.param = (paramId_t)InstrumentParam::paramInstrumentName;
					tempParam.MsgValue = nameIter->_instrumentData->instrumentName;
					retParam.push_back (tempParam);
					tempParam.param = (paramId_t)InstrumentParam::paramInstrumentDispName;
					tempParam.MsgValue = nameIter->_instrumentData->instrumentDispName;
					retParam.push_back(tempParam);
				}
			}
			break;
			case InstrumentParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList ();

				paramData.param = (paramId_t)InstrumentParam::paramUndoListCmd;
				paramData.Count = (uint32_t)undoList.size (); //set the number of params
				retParam.push_back (paramData);

				for (auto itlist = undoList.begin (); itlist < undoList.end (); ++itlist)
				{
					paramData.param = *itlist;
					paramData.Count = Count;
					Count++;
					retParam.push_back (paramData);
				}
			}
			break;
			case InstrumentParam::paramUsedByListCmd:
			{
				std::string name = (*it).MsgValue.get<std::string> ().c_str ();
				instrument = getInstrument (name);
				paramData = { 0 };
				paramData.param = (paramId_t)InstrumentParam::paramUsedByListCmd;
				paramData.Count = instrument->_instrumentData->UsedByCount;
				paramData.MsgValue = name;
				retParam.push_back (paramData);

				for (auto itlist = instrument->_instrumentData->usedByList.begin (); itlist != instrument->_instrumentData->usedByList.end (); ++itlist)
				{
					paramData.param = (paramId_t)InstrumentParam::paramUsedBySongList;
					paramData.MsgValue = itlist->songName;
					retParam.push_back (paramData);
					paramData.param = (paramId_t)InstrumentParam::paramUsedBySceneList;
					paramData.MsgValue = itlist->sceneName;
					retParam.push_back(paramData);
					paramData.param = (paramId_t)InstrumentParam::paramUsedByTrackList;
					paramData.MsgValue = itlist->trackName;
					retParam.push_back(paramData);
				}
			}
			break;
			case InstrumentParam::paramInstrumentName:
				if (instrument)
					paramData = { (*it).param, instrument->getName (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramInstrumentPort:
				if (instrument)
					paramData = { (*it).param, instrument->getMidiPort (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramInstrumentChannel:
				if (instrument)
					paramData = { (*it).param, instrument->getMidiChannel (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramDefaultNote:
				if (instrument)
					paramData = { (*it).param, instrument->getDefaultNote (), 0, 0 };
				retParam.push_back (paramData);
				break;
            case InstrumentParam::paramDesktopSoundFontPath:
                if(instrument)retParam.push_back({(*it).param,instrument->_instrumentData->soundFontPath,0,0});
                break;
			case InstrumentParam::paramInternalSoundPgm:
				if (instrument)
					paramData = { (*it).param, instrument->getInternalSoundPgm (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramInternalSound:
				if (instrument)
					paramData = { (*it).param, instrument->getInternalSound (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramMultiTimbralStatus:
				if (instrument)
					paramData = { (*it).param, instrument->getMultiTimbralStatus (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramPolySpreadStatus:
				if (instrument)
					paramData = { (*it).param, instrument->getPolySpreadStatus (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramTransposeDefeatStatus:
				if (instrument)
					paramData = { (*it).param, instrument->getTransposeDefeatStatus (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramFtsDefeatStatus:
				if (instrument)
					paramData = { (*it).param, instrument->getFtsDefeatStatus (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramEditTrackThruStatus:
				if (instrument)
					paramData = { (*it).param, instrument->getEditTrackThruStatus (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramCc0BankMStatus:
				if (instrument)
					paramData = { (*it).param, instrument->getCc0BankMStatus (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramCc0BankLStatus:
				if (instrument)
					paramData = { (*it).param, instrument->getCc0BankLStatus (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramDisplayNoteNum:
				if (instrument)
					paramData = { (*it).param, instrument->getDisplayNoteNum (), 0, 0 };
				retParam.push_back (paramData);
				break;
			case InstrumentParam::paramPreSendPgmChange:
				if (instrument)
					paramData = { (*it).param, instrument->getPreSendPgmChange (), 0, 0 };
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVType:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVParamType (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVName:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVName (), 0, 0 };
				}
				else
					paramData.MsgValue = "";//to avoid crash
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVMidiCCNum:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiCCNum (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVMidiCCValue:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiCCValue (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVMidiCCInitValue:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiCCInitValue (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVMidiCCMinValue:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiCCMinValue (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVMidiCCMaxValue:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiCCMaxValue (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVMidiPgmNum:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiPgmNumber (TvLocation::TrackLevel), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVMidiPgmBankLsb:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiPgmBankLsb (TvLocation::TrackLevel), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVMidiPgmBankMsb:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiPgmBankMsb (TvLocation::TrackLevel), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramInstrumentPgmNum:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiPgmNumber (TvLocation::InstrumentLevel), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramInstrumentPgmBankLsb:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiPgmBankLsb (TvLocation::InstrumentLevel), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramInstrumentPgmBankMsb:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVMidiPgmBankMsb (TvLocation::InstrumentLevel), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramCountTrackValueCmd:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTrackValueCount (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVKnobPresenceQuery:
				if (instrument)
				{
					//inform ui to set the required knob id at count field
					int8_t knobId = (int8_t )((*it).Count);
					paramData = { (*it).param, instrument->getTrackValuePresenceStatus (knobId), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSaveScope:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTrackValueScope (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

				/*
				case InstrumentParam::paramTVSeqQuantPercent:
					if (instrument)
					{
						paramData = { (*it).param, instrument->getTVQuantisationPercent(), 0, 0 };
					}
					retParam.push_back(paramData);
					break;

			case InstrumentParam::paramTVSeqVelocityPercent:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVVelocityPercent (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSeqVelocityRef:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVVelocityReference (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSeqLengthPercent:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVLengthPercent (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSeqNotePercent:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVNotePercent (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSeqNoteRef:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVNoteRef (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSeqTransposeSemiTone:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVTransposeBySemiTone (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSeqTransposeOctave:
				if (instrument)
				{
					paramData = { (*it).param, instrument->getTVTransposeByOctave (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSeqFtsScale:
				if (instrument)
				{
					paramData = { (*it).param, (int8_t)instrument->getTVFtsScale (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTVSeqFtsRootNote:
				if (instrument)
				{
					paramData = { (*it).param, (int8_t)instrument->getTVFtsRootNote (), 0, 0 };
				}
				retParam.push_back (paramData);
				break;

				case InstrumentParam::paramTVSeqReichValue:
					if (instrument)
					{
						paramData = { (*it).param, instrument->getTVReich(), 0, 0 };
					}
					retParam.push_back(paramData);
					break;
				*/

			case InstrumentParam::paramTMActiveTrack:
				if (instrument)
				{
					paramData = { (*it).param, _tmTrackId, 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTMPan:
				if (getTxInstrumentFromTrackId (_tmTrackId) != nullptr)
				{
					paramData = { (*it).param, getTxInstrumentFromTrackId (_tmTrackId)->getTrackMixerPan (), 0, 0 };
				}
				else
				{
					paramData = { (*it).param, 0, 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			case InstrumentParam::paramTMVolume:
				if (getTxInstrumentFromTrackId (_tmTrackId) != nullptr)
				{
					paramData = { (*it).param, (int8_t)getTxInstrumentFromTrackId (_tmTrackId)->getTrackMixerVolume (), 0, 0 };
				}
				else
				{
					paramData = { (*it).param, 0, 0, 0 };
				}
				retParam.push_back (paramData);
				break;

			default:
				//assert (0);
				break;
		}
	}
	return retParam;
}

void InstrumentManager::getParam (moduleId_t moduleSrc, paramId_t param)
{
	Instrument* instrument = nullptr;
	if (!currentInstrumentName.empty ())
	{
		instrument = getInstrument (currentInstrumentName);
	}

	switch ((InstrumentParam)param)
	{
		case InstrumentParam::paramInstrumentPort:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getMidiPort ());
			break;
		case InstrumentParam::paramInstrumentChannel:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getMidiChannel ());
			break;
		case InstrumentParam::paramDefaultNote:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getDefaultNote ());
			break;
        case InstrumentParam::paramDesktopSoundFontPath:
            if(instrument)engine.SendResponse(moduleId_t(ModuleName::moduleInstrument),moduleSrc,param,instrument->_instrumentData->soundFontPath);
            break;
		case InstrumentParam::paramInternalSoundPgm:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getInternalSoundPgm ());
			break;
		case InstrumentParam::paramInternalSound:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getInternalSound ());
			break;
		case InstrumentParam::paramMultiTimbralStatus:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getMultiTimbralStatus ());
			break;
		case InstrumentParam::paramPolySpreadStatus:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getPolySpreadStatus ());
			break;
		case InstrumentParam::paramTransposeDefeatStatus:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getTransposeDefeatStatus ());
			break;
		case InstrumentParam::paramFtsDefeatStatus:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getFtsDefeatStatus ());
			break;
		case InstrumentParam::paramEditTrackThruStatus:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getEditTrackThruStatus ());
			break;
		case InstrumentParam::paramCc0BankMStatus:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getCc0BankMStatus ());
			break;
		case InstrumentParam::paramCc0BankLStatus:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getCc0BankLStatus ());
			break;
		case InstrumentParam::paramDisplayNoteNum:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getDisplayNoteNum ());
			break;
		case InstrumentParam::paramPreSendPgmChange:
			if (instrument)
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getPreSendPgmChange ());
			break;

		case InstrumentParam::paramTMActiveTrack:
			if (instrument)
			{
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, _tmTrackId);
			}
			break;

		case InstrumentParam::paramTMPan:
			if (instrument)
			{
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getTrackMixerPan ());
			}
			break;

		case InstrumentParam::paramTMVolume:
			if (instrument)
			{
				engine.SendResponse ((moduleId_t)ModuleName::moduleInstrument, moduleSrc, param, instrument->getTrackMixerVolume ());
			}
			break;

		default:
			//TODO WARN?
			break;
	}
}

std::string InstrumentManager::addInstrument (void)
{
	uint8_t port = -1;//set some default value
	uint8_t chan = -1;//set some default value

	std::string name;

	++numberOfInstruments;

	name = "INSTR" + std::to_string (numberOfInstruments);
	modifyInstrumentNameIfDuplicate (name);
	Instrument tempInstrument = Instrument (numberOfInstruments - 1, name, port, chan, this);

	//add in instrument manager list
	instrumentList.push_back (tempInstrument);

	//save in filesystem
	SaveInstrument (name);
	return name;
}

void InstrumentManager::modifyInstrumentNameIfDuplicate (std::string& name)
{
	for (auto& instrumentIter : instrumentList)
	{
		if (instrumentIter.getName () == name)
		{
			srand ((uint32_t)time (NULL));
			int16_t randomTag = rand () % 9999;
			std::string tagString = "_" + std::to_string (randomTag);
			name = name + tagString;
		}
	}
}

void InstrumentManager::setActiveInstrument (std::string name)
{
	if (name != currentInstrumentName)
	{
		currentInstrumentName = name;
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData ();
	}
}

void InstrumentManager::setInstrumentDispName(std::string instrumentName, std::string Newname)
{
	setInstrumentName(instrumentName, Newname);
}
void InstrumentManager::setInstrumentName (std::string instrumentName, std::string Newname)
{
	auto start = instrumentList.begin ();
	auto end = instrumentList.end ();

	auto i = start;

	for (; i != end; ++i)
	{
		if (instrumentName == i->getName ())
		{
			i->setName (Newname);
			engine.fileHandler->saveInstrument(currentInstrumentName, (*(i)->_instrumentData));
			///engine.fileHandler->RenameInstrument (instrumentName, Newname);
			currentInstrumentName = instrumentName;
			break;
		}
	}

	if (i == end)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to set instrument name");
	}
}

void InstrumentManager::setInstrumentPort (std::string instrumentName, uint8_t val)
{
	auto start = instrumentList.begin ();
	auto end = instrumentList.end ();

	auto i = start;

	for (; i != end; ++i)
	{
		if (instrumentName == i->getName ())
		{
			i->setMidiPort (val);
			break;
		}
	}

	if (i == end)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to set instrument midi port");
	}
}

void InstrumentManager::setInstrumentChannel (std::string instrumentName, uint8_t val)
{
	auto start = instrumentList.begin ();
	auto end = instrumentList.end ();

	auto i = start;

	for (; i != end; ++i)
	{
		if (currentInstrumentName == i->getName ())
		{
			i->setMidiChannel (val);
			break;
		}
	}

	if (i == end)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to set instrument midi channel");
	}
}

int8_t InstrumentManager::getInstrumentPort (std::string instrumentName)
{
	auto start = instrumentList.begin ();
	auto end = instrumentList.end ();

	for (auto i = start; i != end; ++i)
	{
		if (instrumentName == i->getName ())
		{
			return i->getMidiPort ();
		}
	}
	
	DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get instrument midi port");
	return (int8_t)(-1);
}

int8_t InstrumentManager::getInstrumentChannel (std::string instrumentName)
{
	auto start = instrumentList.begin ();
	auto end = instrumentList.end ();

	for (auto i = start; i != end; ++i)
	{
		if (instrumentName == i->getName ())
		{
			return i->getMidiChannel ();
		}
	}

	DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR Unable to get instrument midi channel");
	return (int8_t)(-1);
}

uint8_t InstrumentManager::getInstrumentCount (void) const
{
	return numberOfInstruments;
}

void InstrumentManager::clearAllInstrument ()
{
	instrumentList.clear ();
	numberOfInstruments = 0;
	ClearUndoList ();
	engine.fileHandler->DeleteAllInstrument ();
}

void InstrumentManager::removeInstrument (std::string instrumentName)
{
	auto start = instrumentList.begin ();
	auto end = instrumentList.end ();

	for (auto i = start; i != end; ++i)
	{
		if (instrumentName == (*i)._instrumentData->instrumentName)
		{
			instrumentList.erase (i);
			--numberOfInstruments;
			break;
		}
	}
}
bool InstrumentManager::copyInstrument (std::string name)
{
	if (!name.empty ())
	{
		modifyInstrumentNameIfDuplicate (name);
		++numberOfInstruments;
		Instrument tempInstrument = Instrument (getInstrumentData (currentInstrumentName), this);

		tempInstrument.setName (name);

		//Remove any track links from newly created pattern
		tempInstrument._instrumentData->UsedByCount = 0;
		tempInstrument._instrumentData->usedByList.clear ();
		tempInstrument._instrumentData->instrumentName = name;

		//add in pattern manager list
		instrumentList.push_back (tempInstrument);

		//save in filesystem
		engine.fileHandler->saveInstrument (name, *(tempInstrument._instrumentData));

		return true;
	}
	return false;
}


void InstrumentManager::moveInstrument (uint64_t positionData)
{
	uint32_t curId = (positionData & 0x3FF00000000) >> 32;
	uint32_t newId = positionData & 0x3FF;

	for (auto& instrument : instrumentList)
	{
		// If movement is +/- 1 position, LIST sort does not work as required.
		// So in this case which is like swapping, also update the id of the instrument,
		// which is swapping its position with current instrument.
		// For other cases when movement is greater than +/- 1 positions,
		// only update the moving instrument's id, to new id.

		if ((std::abs (int (curId - newId)) == 1) && (instrument.getId () == (int32_t) newId))
		{
			//assign cur instrument id to the other (adjacent) instrument
			instrument.setId (curId);
		}
		else if ((newId < curId) && (instrument.getId () == (int32_t)newId))
		{
			instrument.setId (newId + 1);
		}
		else if (instrument.getId () == (int32_t)curId)
		{
			instrument.setId (newId);
		}
	}

	sortInstrumentsById ();
	reAssignSequentialIdsForInstrumentsAndSave ();
}

struct sortInstrumentCompareFunction
{
	bool operator() (Instrument& first, Instrument& second)
	{
		return (first.getId () < second.getId ());
	}
};

void InstrumentManager::sortInstrumentsById (void)
{
	instrumentList.sort (sortInstrumentCompareFunction ());
}

void InstrumentManager::reAssignSequentialIdsForInstrumentsAndSave (void)
{
	//Re-assign ids starting from 0
	int32_t count = 0;
	for (auto& instrumentIter : instrumentList)
	{
		if (instrumentIter.getId () != count)
		{
			instrumentIter.setId (count);
		}

		//save to FS
		updateInstrumentData (instrumentIter.getName (), *(instrumentIter._instrumentData));
		++count;
	}
}

void InstrumentManager::updateInstrumentData (std::string name, InstrumentData& data)
{
	engine.fileHandler->DeleteInstrument (name);
	engine.fileHandler->saveInstrument (name, data);
}
void InstrumentManager::SaveEditTrackMixerInstrument (int8_t trackId)
{
	auto instr = getTxInstrumentFromTrackId (trackId);
	if (instr != nullptr)
		engine.fileHandler->saveInstrument (instr->getName (), getInstrumentData (instr->getName ()));
}
void InstrumentManager::setTrackMixerTrack (int8_t trackId)
{
	_tmTrackId = trackId;
}

void InstrumentManager::setTrackMixerPanValue (int8_t pan)
{
	auto instr = getTxInstrumentFromTrackId (_tmTrackId);
	if (instr != nullptr)
		instr->setTrackMixerPan (pan);
}

void InstrumentManager::setTrackMixerVolumeValue (int8_t volume)
{
	auto instr = getTxInstrumentFromTrackId (_tmTrackId);
	if (instr != nullptr)
		instr->setTrackMixerVolume (volume);
}

int8_t InstrumentManager::getTrackMixerTrack (void)
{
	return _tmTrackId;
}

int8_t InstrumentManager::getTrackMixerPanValue (void)
{
	auto instr = getTxInstrumentFromTrackId (_tmTrackId);
	if (instr != nullptr)
		return instr->getTrackMixerPan ();

	return -1;
}

int8_t InstrumentManager::getTrackMixerVolumeValue (void)
{
	auto instr = getTxInstrumentFromTrackId (_tmTrackId);
	if (instr != nullptr)
		return instr->getTrackMixerVolume ();

	return -1;
}

Instrument* InstrumentManager::getInstrumentFromListIndex (int8_t index)
{
	for (auto& instrumentIter : instrumentList)
	{
		if (instrumentIter.getId () == index)
		{
			return &instrumentIter;
		}
	}
	return nullptr;
}

Instrument* InstrumentManager::getRxInstrumentFromTrackId (int8_t trackId)
{
	Instrument* retInstrument = nullptr;

	if (engine.songManager != nullptr)
	{
		if ((engine.songManager->activeSong != nullptr) && (engine.songManager->getActiveSong ()->getEditScene () != nullptr))
		{
			auto& trackList = engine.songManager->getActiveSong ()->getEditScene ()->_sceneData->trackList;

			for (auto& track : trackList)
			{
				if (track.getTrackId () == trackId)
				{
					retInstrument = track.getTrackRxInstrument ();
					break;
				}
			}
		}
	}
	return retInstrument;
}

Instrument* InstrumentManager::getTxInstrumentFromTrackId(int8_t trackId)
{
	Instrument* retInstrument = nullptr;

	if (engine.songManager != nullptr)
	{
		if ((engine.songManager->activeSong != nullptr) && (engine.songManager->getActiveSong()->getEditScene() != nullptr))
		{
			auto& trackList = engine.songManager->getActiveSong()->getEditScene()->_sceneData->trackList;

			for (auto& track : trackList)
			{
				if (track.getTrackId() == trackId)
				{
					retInstrument = track.getTrackTxInstrument();
					break;
				}
			}
		}
	}
	return retInstrument;
}

void InstrumentManager::InstrumentMngrPowerOnHousekeeping ()
{
	for (auto& instrumentIter : instrumentList)
	{
		bool SaveFile = false;
		for (auto it = instrumentIter._instrumentData->usedByList.begin (); it != instrumentIter._instrumentData->usedByList.end ();)
		{
			bool valid = false;
			valid = engine.trackManager->IsInstrumentAssigned ((instrumentIter)._instrumentData->instrumentName, it->songName, it->sceneName, it->trackName);
			if (!valid)
			{
				it = instrumentIter._instrumentData->usedByList.erase (it);
				SaveFile = true;
			}
			else
				++it;
		}
		if(SaveFile)
			engine.fileHandler->saveInstrument ((instrumentIter)._instrumentData->instrumentName, *(instrumentIter)._instrumentData);
	}
}

void InstrumentManager::addInstrumentListener (IInstrumentListener* listener)
{
	if (listener)
	{
		_instrumentListeners.insert(listener);
	}
}
