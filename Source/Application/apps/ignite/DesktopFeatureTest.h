// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopServices.h"

class DesktopFeatureTest
{
public:
    static int run(Ignite::IgniteEngine& engine, IgniteDesktop::MidiRouter& router, bool verify)
    {
        using namespace Ignite;
        int failures=0, checks=0;
        auto check=[&](bool ok,const juce::String& name) {++checks;if(!ok)++failures;juce::Logger::writeToLog(juce::String(ok?"PASS: ":"FAIL: ")+name);return ok;};
        auto command=[&](ModuleName module,auto parameter,AnyType value) {return engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(module),paramId_t(parameter),value);};
        auto& songs=*engine.songManager;
        const auto root=SongManager::desktopSettingsFile().getParentDirectory();
        const auto manifestFile=root.getChildFile("feature-test.json");
        if (verify)
        {
            auto manifest=juce::JSON::parse(manifestFile);
            command(ModuleName::moduleSong,SongParam::paramActiveSong,int32_t(0));
            command(ModuleName::moduleScene,SceneParam::paramActiveScene,int32_t(0));
            auto* instrument=engine.instrumentManager->getInstrument(manifest.getProperty("instrument","").toString().toStdString());
            check(instrument && instrument->getInternalSoundPgm()==SoundFontProgram::encode(42,7),"custom bank and program survive restart");
            auto steps=engine.EngineGetPatternData(manifest.getProperty("recordedPattern","").toString().toStdString());
            int notes=0;
            for (const auto& bar:steps.BarDataList) for(const auto& step:bar.second.StepDataList) for(const auto& pitch:step.second.nodeList) notes+=int(pitch.second.size());
            check(notes==2,"MIDI step-recorded notes survive restart");
            check(bool(IgniteDesktop::readPreferences().getProperty("MonitorInput",false)),"desktop MIDI preferences survive restart");
            auto live=engine.EngineGetPatternData(manifest.getProperty("livePattern","").toString().toStdString());
            bool found=false;
            for(const auto& bar:live.BarDataList)for(const auto& step:bar.second.StepDataList)found|=step.second.nodeList.count(72)>0;
            check(found,"live-recorded MIDI note survives restart");
        }
        else
        {
            command(ModuleName::moduleTrack,TrackParam::paramActiveTrack,uint8_t(0));
            const auto preferences=IgniteDesktop::readPreferences();
            check(router.save(),"desktop MIDI preferences save without physical ports");
            router.restore();
            const auto before=songs.desktopMidiReceived.load();
            for(int port:IgniteDesktop::MidiRouter::ports()) router.receive(port,juce::MidiMessage::controllerEvent(1,1,32));
            check(songs.desktopMidiReceived.load()==before+16,"all 16 logical MIDI inputs reach the firmware router");
            router.receive(255,juce::MidiMessage::noteOn(1,60,uint8_t(100)));
            check(songs.desktopMidiReceived.load()==before+16,"invalid MIDI input ports are ignored");
            auto output=songs.desktopMidiOutput;
            std::vector<int> sentPorts;
            songs.desktopMidiOutput=[&](const juce::MidiMessage&,uint8_t port){sentPorts.push_back(port);return true;};
            for(int port:IgniteDesktop::MidiRouter::ports()) songs.sendMidiExternal(juce::MidiMessage::noteOff(1,60),uint8_t(port));
            songs.desktopMidiOutput=output;
            check(sentPorts==IgniteDesktop::MidiRouter::ports(),"firmware output preserves all logical MIDI port IDs");
            auto old=songs.get_tsf_instance(); std::string error;
            check(!songs.loadDesktopSoundFont(root.getChildFile("missing.sf2").getFullPathName().toStdString(),error) && songs.get_tsf_instance()==old,
                  "failed SoundFont replacement preserves the current bank");
            const auto custom=juce::SystemStats::getEnvironmentVariable("IGNITE_TEST_SOUNDFONT","");
            if (!check(songs.loadDesktopSoundFont(custom.toStdString(),error),"custom SoundFont loads through a Unicode-capable stream")) return failures;
            auto presets=songs.getDesktopSoundFontPresets();
            check(presets.size()==1 && presets[0].bank==42 && presets[0].program==7,"browser enumerates actual non-GM bank and program numbers");
            const bool previewStarted=songs.desktopPreviewPreset(0);
            AudioDataType previewBlock[INTERNAL_AUDIO_20MS_BLOCK_SIZE]{};
            songs.renderTracksAndMix(previewBlock);
            float previewPeak=0;
            for (auto sample:previewBlock) previewPeak=std::max(previewPeak,std::abs(sample));
            songs.desktopPreviewNoteOff(); songs.desktopStopPreview();
            check(previewStarted && previewPeak>0.0001f,"custom preset preview renders audible samples without an assigned track");
            auto* track=songs.activeSong->editScene->editTrack;
            auto* instrument=track->getTrackTxInstrument();
            if (!check(instrument!=nullptr,"selected track retains its assigned instrument after bank replacement")) return failures;
            command(ModuleName::moduleInstrument,InstrumentParam::paramSetActiveInstrument,instrument->getName());
            command(ModuleName::moduleInstrument,InstrumentParam::paramInternalSoundPgm,SoundFontProgram::encode(42,7));
            engine.instrumentManager->SaveInstrument(instrument->getName());
            track->prepareDesktopMonitor(); track->sfNoteOn(0,60,0.8f); track->sfSynthesizeAudio();
            float peak=0; for(int i=0;i<track->audioBuff.getNumSamples();++i) peak=std::max(peak,std::abs(track->audioBuff.getSample(0,i)));
            check(peak>0.0001f && instrument->getInternalSoundPgm()==SoundFontProgram::encode(42,7),"non-GM preset assignment renders audible synth samples");
            songs.desktopPanic();

            auto inputName=engine.instrumentManager->addInstrument(); auto* input=engine.instrumentManager->getInstrument(inputName);
            input->setMidiPort(13); input->setMidiChannel(0); engine.instrumentManager->SaveInstrument(inputName);
            command(ModuleName::moduleTrack,TrackParam::paramTrackInstrument,inputName);
            const auto patternName=engine.patternManager->addPattern();
            command(ModuleName::moduleTrack,TrackParam::paramTrackPattern,patternName);
            command(ModuleName::moduleSettingsRecordCnfg,SettingsRecordCnfgParam::SettingRecordNoteEventParam,int8_t(1));
            command(ModuleName::moduleSong,SongParam::paramStartRecordCmd,true);
            check(songs.getSystemState()==SYSTEM_STATE::STEP_RECORD,"record command arms step recording");
            router.receive(13,juce::MidiMessage::noteOn(2,70,uint8_t(99)));
            router.receive(13,juce::MidiMessage::noteOff(2,70));
            router.receive(13,juce::MidiMessage::noteOn(1,65,uint8_t(100)));
            router.receive(13,juce::MidiMessage::noteOff(1,65));
            router.receive(13,juce::MidiMessage::noteOn(1,67,uint8_t(90)));
            router.receive(13,juce::MidiMessage::noteOff(1,67));
            command(ModuleName::moduleSong,SongParam::paramStopRecordCmd,true);
            auto recorded=engine.EngineGetPatternData(patternName);
            int notes=0;bool unwanted=false;
            for(const auto& bar:recorded.BarDataList)for(const auto& step:bar.second.StepDataList)for(const auto& pitch:step.second.nodeList){notes+=int(pitch.second.size());unwanted|=pitch.first==70;}
            check(notes==2 && !unwanted,"MIDI recording stores matching-channel notes and rejects other channels");
            const auto livePattern=engine.patternManager->addPattern();
            command(ModuleName::moduleTrack,TrackParam::paramTrackPattern,livePattern);
            engine.sequenceManager->initSequence(); songs.initPlayScene(0); songs._state=SYSTEM_STATE::PLAY;
            command(ModuleName::moduleSong,SongParam::paramStartRecordCmd,true);
            check(songs.getSystemState()==SYSTEM_STATE::LIVE_RECORD,"record command arms live recording during playback");
            router.receive(13,juce::MidiMessage::noteOn(1,70,uint8_t(99)));
            router.receive(13,juce::MidiMessage::noteOff(1,70));
            command(ModuleName::moduleTrack,TrackParam::paramTrackRecord,true);
            router.receive(13,juce::MidiMessage::noteOn(1,72,uint8_t(110)));
            for(int i=0;i<12;++i) track->pulse(0,0);
            router.receive(13,juce::MidiMessage::noteOff(1,72));
            command(ModuleName::moduleSong,SongParam::paramStopRecordCmd,true);
            songs._state=SYSTEM_STATE::IDLE;
            auto live=engine.EngineGetPatternData(livePattern);bool liveFound=false;
            for(const auto& bar:live.BarDataList)for(const auto& step:bar.second.StepDataList)
                if(step.second.nodeList.count(72))for(const auto& node:step.second.nodeList.at(72))
                    liveFound|=node.second.velocity==110 && !node.second.onState && (node.second.lenInTicks>0 || node.second.lenInSteps>0);
            check(liveFound,"live MIDI recording captures note velocity and duration");
            engine.SaveAllData();
            auto* metadata=new juce::DynamicObject();metadata->setProperty("instrument",juce::String(instrument->getName()));metadata->setProperty("recordedPattern",juce::String(patternName));metadata->setProperty("livePattern",juce::String(livePattern));
            manifestFile.replaceWithText(juce::JSON::toString(juce::var(metadata)));
            auto prefs=IgniteDesktop::readPreferences();prefs.getDynamicObject()->setProperty("SoundFont",custom);IgniteDesktop::writePreferences(prefs);

            auto archive=root.getChildFile("roundtrip.zip");
            check(IgniteDesktop::ProjectArchive::exportTo(IgniteDesktop::ProjectArchive::dataRoot(),archive).wasOk(),"project backup ZIP exports");
            auto restored=root.getChildFile("restore-check");restored.createDirectory();restored.getChildFile("obsolete").replaceWithText("old");
            check(IgniteDesktop::ProjectArchive::restoreFrom(archive,restored).wasOk() && !restored.getChildFile("obsolete").exists() && restored.getChildFile("Songs").isDirectory(),
                  "project archive restores complete libraries with a recovery copy");
            auto bad=root.getChildFile("bad.zip");
            {juce::ZipFile::Builder zip;zip.addEntry(new juce::MemoryInputStream("x",1,true),6,"../escaped.txt",juce::Time::getCurrentTime());juce::FileOutputStream output(bad);zip.writeToStream(output,nullptr);}
            auto sentinel=restored.getChildFile("sentinel");sentinel.replaceWithText("keep");
            check(IgniteDesktop::ProjectArchive::restoreFrom(bad,restored).failed() && sentinel.loadFileAsString()=="keep" && !root.getChildFile("escaped.txt").exists(),
                  "unsafe archive paths are rejected without replacing existing data");
            // Restoring the actual saved data is verified by a fresh process.
            check(IgniteDesktop::ProjectArchive::restoreFrom(archive,IgniteDesktop::ProjectArchive::dataRoot()).wasOk(),"exported arrangement replaces the test data transactionally");
            const auto allData=uint8_t(EngineDataFolderId::AllDataInternalStorage);
            check(engine.fileHandler->CreateBackUP(allData),"firmware backup command stages the saved libraries");
            auto savedInstrument=root.getChildFile("Ignite_folder_backup/Instruments").getChildFile(juce::String(instrument->getName())+".txt");
            const auto original=savedInstrument.loadFileAsString();
            savedInstrument.replaceWithText("<broken");
            check(!engine.fileHandler->RestoreBackUP(allData) && IgniteDesktop::ProjectArchive::dataRoot().getChildFile("Songs").isDirectory(),
                  "firmware restore rejects corrupted XML before replacing the current data");
            savedInstrument.replaceWithText(original);
            check(engine.fileHandler->RestoreBackUP(allData),"firmware backup restore succeeds with complete valid data");
        }
        juce::Logger::writeToLog("Feature checks: "+juce::String(checks)+", failures: "+juce::String(failures));
        return failures;
    }
};
