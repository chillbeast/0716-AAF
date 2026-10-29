// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopServices.h"
#include "DesktopCloseGuard.h"
#include "DesktopEditorTest.h"

class DesktopCorrectnessTest
{
public:
    static int run(Ignite::IgniteEngine& engine,IgniteDesktop::MidiRouter& router,bool verify)
    {
        using namespace Ignite;
        int checks=0,failures=0;
        auto check=[&](bool ok,const juce::String& name) {++checks;if(!ok)++failures;juce::Logger::writeToLog(juce::String(ok?"PASS: ":"FAIL: ")+name);return ok;};
        auto command=[&](ModuleName module,auto parameter,AnyType value)
        {return engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(module),paramId_t(parameter),value);};
        auto& songs=*engine.songManager;
        auto root=SongManager::desktopSettingsFile().getParentDirectory();
        auto manifest=root.getChildFile("correctness.json");
        if(verify)
        {
            auto saved=juce::JSON::parse(manifest);
            command(ModuleName::moduleSong,SongParam::paramActiveSong,int32_t(0));
            command(ModuleName::moduleScene,SceneParam::paramActiveScene,int32_t(0));
            auto first=engine.EngineGetPatternData(saved["first"].toString().toStdString());
            check(first.BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).velocity==113,"earlier edited pattern survives a fresh process");
            auto second=engine.EngineGetPatternData(saved["second"].toString().toStdString());
            check(second.BarDataList.at(0).StepDataList.at(0).nodeList.count(67)>0,"later edited pattern survives a fresh process");
            check(!engine.desktopHasUnsavedChanges(),"opening a saved arrangement does not produce unsaved edits");
            const auto inserted=engine.EngineGetPatternData(saved["inserted"].toString().toStdString());
            check(inserted.numBars==2 && inserted.BarDataList.at(0).StepDataList.at(1).nodeList.count(62),"inserted and Sculpt-edited notes survive a fresh process");
        }
        else
        {
            command(ModuleName::moduleTrack,TrackParam::paramActiveTrack,uint8_t(0));
            auto* track=songs.activeSong->editScene->editTrack;
            const auto first=track->getTrackMainPattern()->getName();
            const auto output=track->getTrackTxInstrument()->getName();
            auto inputName=engine.instrumentManager->addInstrument();
            auto* input=engine.instrumentManager->getInstrument(inputName);
            input->setMidiPort(13);input->setMidiChannel(15);engine.instrumentManager->SaveInstrument(inputName);
            command(ModuleName::moduleTrack,TrackParam::paramTrackInstrument,inputName);
            track->prepareDesktopMonitor();
            auto midi=[&](const juce::MidiMessage& message){router.receive(13,message);};
            auto cc=[&](int number,int value){midi(juce::MidiMessage::controllerEvent(16,number,value));};
            auto render=[&](int blocks=50)
            {
                std::vector<float> audio;audio.reserve(size_t(blocks)*960);
                for(int b=0;b<blocks;++b) {track->sfSynthesizeAudio();for(int i=0;i<960;++i)audio.push_back(track->audioBuff.getSample(0,i));}
                return audio;
            };
            auto rms=[](const auto& audio){double sum=0;for(auto value:audio)sum+=double(value)*value;return std::sqrt(sum/audio.size());};
            auto frequency=[](const auto& audio)
            {
                int count=0;double first=-1,last=-1;
                for(size_t i=2001;i<audio.size();++i)if(audio[i-1]<=0 && audio[i]>0)
                {const double crossing=double(i-1)-audio[i-1]/double(audio[i]-audio[i-1]);if(first<0)first=crossing;last=crossing;++count;}
                return count>1 ? (count-1)*48000.0/(last-first) : 0;
            };
            auto note=[&]{midi(juce::MidiMessage::noteOn(16,69,uint8_t(100)));};
            auto program=[&](int bank,int number)
            {cc(120,0);cc(0,bank>>7);cc(32,bank&127);midi(juce::MidiMessage::programChange(16,number));note();};
            program(0,0);auto baseline=render();const double baseFrequency=frequency(baseline),baseLevel=rms(baseline);
            check(baseFrequency>430 && baseFrequency<450 && baseLevel>0.0001,"channel 16 MIDI input reaches the selected internal instrument");
            program(0,1);check(std::abs(frequency(render())/baseFrequency-2.0)<0.02,"incoming program changes select a different audible preset");
            program(42,7);check(std::abs(frequency(render())/baseFrequency-0.5)<0.02,"incoming bank/program changes select a non-GM preset");
            program(129,5);check(std::abs(frequency(render())/baseFrequency-std::pow(2.0,7.0/12))<0.02,"bank MSB and LSB combine correctly");
            program(129,64);check(rms(render())<0.000001,"a missing MIDI program does not reuse the previous sound");
            program(0,0);cc(64,127);midi(juce::MidiMessage::noteOff(16,69));
            check(rms(render())>baseLevel*0.5,"router sustain holds a released note");
            cc(64,0);check(rms(render(100))<baseLevel*0.2,"releasing sustain ends the recorded voice");
            program(0,0);midi(juce::MidiMessage::pitchWheel(16,16383));
            check(std::abs(frequency(render())/baseFrequency-std::pow(2.0,2.0/12))<0.02,"pitch bend is applied through the internal MIDI route");
            cc(121,0);cc(11,0);render(1);
            // SF2's default attenuation modulator has a finite 96 dB range.
            check(rms(render())<baseLevel*0.0001,"expression attenuates the routed voice by at least 80 dB after smoothing");
            cc(121,0);check(rms(render())>baseLevel*0.5,"reset controllers restores expression");
            midi(juce::MidiMessage::controllerEvent(1,11,0));
            check(rms(render())>baseLevel*0.5,"other input channels cannot change the selected track's synth controls");
            cc(64,127);cc(123,0);check(rms(render())>baseLevel*0.5,"routed All Notes Off honors sustain");
            cc(120,0);check(rms(render())<0.000001,"routed All Sound Off clears voices and effects");
            songs.desktopPanic();note();check(rms(render())>baseLevel*0.5,"the internal synth remains playable after Panic");
            cc(120,0);
            check(engine.SaveAllData() && !engine.desktopHasUnsavedChanges(),"complete desktop save clears the current pending changes");

            command(ModuleName::moduleCvioCnfg,SettingsCvioCnfgParam::paramActiveCVPort,int8_t(0));
            command(ModuleName::moduleCvioCnfg,SettingsCvioCnfgParam::paramCVLaw,int8_t(1));
            check(engine.desktopHasUnsavedChanges(),"CV settings edits participate in pending-change protection");
            check(engine.settingsCvioCnfg->saveForDesktop() && !engine.desktopHasUnsavedChanges(),"individual CV settings save clears its pending changes");

            auto node=engine.EngineGetPatternData(first).BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0);
            node.velocity=113;engine.EngineAddNodeData(first,0,0,node);
            check(engine.desktopHasUnsavedChanges(),"direct note editing marks the pattern as unsaved");
            const auto second=engine.patternManager->addPattern();
            ClientPatternNodeData extra{};extra.note=extra.nodeId=67;extra.velocity=90;extra.lenInSteps=1;extra.onState=true;extra.step_id=0;
            engine.EngineAddNodeData(second,0,0,extra);
            check(engine.SaveAllData() && engine.fileHandler->GetpatternBarStepData(first,0,16).front().nodeList.at(60).at(0).velocity==113,
                  "Save All persists earlier library edits after another pattern is edited");
            auto* data=new juce::DynamicObject();data->setProperty("first",juce::String(first));data->setProperty("second",juce::String(second));
            manifest.replaceWithText(juce::JSON::toString(juce::var(data)));

            auto globalRecord=root.getChildFile("Ignite_folder/Settings/SettingRecordCnfg.txt");
            const auto oldGlobal=globalRecord.loadFileAsString();
            command(ModuleName::moduleSettingsRecordCnfg,SettingsRecordCnfgParam::SettingDataLocation,int8_t(1));
            command(ModuleName::moduleSettingsRecordCnfg,SettingsRecordCnfgParam::SettingRecordNoteEventParam,int8_t(0));
            check(engine.desktopHasUnsavedChanges(),"firmware settings edits participate in the close guard");
            check(engine.SaveAllData() && engine.settingsRecordCnfg->FindParameter(paramId_t(SettingsRecordCnfgParam::SettingDataLocation))->getValue().get<int8_t>()==1 && globalRecord.loadFileAsString()==oldGlobal,
                  "Save All preserves song-scoped settings instead of overwriting global defaults");
            auto recordFile=root.getChildFile("Ignite_folder/Songs").getChildFile(songs.activeSong->getName()).getChildFile("SettingRecordCnfg.txt");
            const auto original=recordFile.loadFileAsString();
            command(ModuleName::moduleSettingsRecordCnfg,SettingsRecordCnfgParam::SettingRecordNoteEventParam,int8_t(1));
            DesktopCloseGuard::Answer answer;int closes=0,errors=0;
            DesktopCloseGuard close([&]{return engine.desktopHasUnsavedChanges();},[&]{return engine.SaveAllData();},[&]{++closes;},
                [&](auto callback){answer=std::move(callback);},[&]{++errors;});
            close.request();answer(DesktopCloseGuard::cancel);
            check(closes==0 && engine.desktopHasUnsavedChanges(),"Cancel keeps the actual edited model open");
            close.request();answer(DesktopCloseGuard::discard);
            check(closes==1 && recordFile.loadFileAsString()==original,"Discard does not save the edited settings");
#ifdef _WIN32
            auto locked=CreateFileW(recordFile.getFullPathName().toWideCharPointer(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
            check(locked!=INVALID_HANDLE_VALUE,"failure test locks the existing saved settings");
            close.request();answer(DesktopCloseGuard::save);
            check(closes==1 && errors==1 && engine.desktopHasUnsavedChanges() && recordFile.loadFileAsString()==original,
                  "failed close/save keeps Ignite open and preserves the old settings");
            if(locked!=INVALID_HANDLE_VALUE) CloseHandle(locked);
#endif
            close.request();answer(DesktopCloseGuard::save);
            check(closes==2 && !engine.desktopHasUnsavedChanges(),"successful close/save retries all required writes");
            DesktopEditorTest::run(engine,check,command);
        }
        juce::Logger::writeToLog("Correctness checks: "+juce::String(checks)+", failures: "+juce::String(failures));
        return failures;
    }
};
