// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopServices.h"
#include "Firmware/Application/Model/tsf.h"
class DesktopSoundTest
{
public:
    static int run(Ignite::IgniteEngine& engine,bool verify,bool missing)
    {
        using namespace Ignite;
        int checks=0,failures=0;
        auto check=[&](bool ok,const juce::String& name){++checks;if(!ok)++failures;juce::Logger::writeToLog(juce::String(ok?"PASS: ":"FAIL: ")+name);return ok;};
        auto command=[&](ModuleName module,auto parameter,AnyType value){return engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(module),paramId_t(parameter),value);};
        auto& songs=*engine.songManager;
        const auto root=SongManager::desktopSettingsFile().getParentDirectory();
        const auto manifestFile=root.getChildFile("sound-test.json");
        const auto bankA=juce::SystemStats::getEnvironmentVariable("IGNITE_TEST_BANK_A",juce::SystemStats::getEnvironmentVariable("IGNITE_SOUNDFONT","")).toStdString();
        const auto bankB=juce::SystemStats::getEnvironmentVariable("IGNITE_TEST_BANK_B","").toStdString();
        auto render=[](Track& track,int blocks=30)
        {
            std::array<std::vector<float>,2> audio;
            for(int i=0;i<blocks;++i){track.sfSynthesizeAudio();for(int ch=0;ch<2;++ch)audio[ch].insert(audio[ch].end(),track.audioBuff.getReadPointer(ch),track.audioBuff.getReadPointer(ch)+960);}
            return audio;
        };
        auto rms=[](const auto& data){double sum=0;for(auto x:data)sum+=double(x)*x;return std::sqrt(sum/std::max(size_t(1),data.size()));};
        auto frequency=[](const auto& data)
        {
            double first=-1,last=-1;int crossings=0;
            for(size_t i=2001;i<data.size();++i)if(data[i-1]<=0 && data[i]>0){const double at=double(i-1)-data[i-1]/double(data[i]-data[i-1]);if(first<0)first=at;last=at;++crossings;}
            return crossings>1?(crossings-1)*48000.0/(last-first):0;
        };
        command(ModuleName::moduleSong,SongParam::paramActiveSong,int32_t(0));
        command(ModuleName::moduleScene,SceneParam::paramActiveScene,int32_t(0));
        command(ModuleName::moduleTrack,TrackParam::paramActiveTrack,uint8_t(0));
        std::string error;
        if(verify || missing)
        {
            const auto manifest=juce::JSON::parse(manifestFile);
            const auto nameA=manifest["instrumentA"].toString().toStdString(),nameB=manifest["instrumentB"].toString().toStdString();
            auto* first=engine.instrumentManager->getInstrument(nameA);auto* second=engine.instrumentManager->getInstrument(nameB);
            check(first && second && first->_instrumentData->soundFontPath==bankA && second->_instrumentData->soundFontPath==bankB,"independent SF2 paths survive save and backup restore in a fresh process");
            check(songs.desktopVoiceLimit==64,"voice limit survives a fresh process");
            if(juce::SystemStats::getEnvironmentVariable("IGNITE_SOUNDFONT","").toStdString()!=bankA)check(!songs.get_tsf_instance(),"a missing default bank does not prevent explicit instrument banks from loading");
            Track a(0,"Reopen A",false,&songs),b(0,"Reopen B",false,&songs);a.setTrackTxInstrument(first);b.setTrackTxInstrument(second);
            a.prepareDesktopMonitor();b.prepareDesktopMonitor();a.sfNoteOn(0,69,0.8f);b.sfNoteOn(0,69,0.8f);
            const auto firstAudio=render(a),secondAudio=render(b);
            check(std::abs(frequency(firstAudio[0])-440)<5 && std::abs(frequency(firstAudio[1])-880)<8,"reopened stereo instrument retains independent left and right pitches");
            if(missing)check(rms(secondAudio[0])<1e-8 && rms(firstAudio[0])>0.001,"missing assigned SF2 stays silent while the other instrument continues sounding");
            else check(std::abs(frequency(secondAudio[0])-880)<8 && std::abs(frequency(secondAudio[1])-880)<8,"a second bank plays its own sound after reopening");
        }
        else
        {
            auto* active=songs.activeSong->editScene->editTrack;
            const auto nameA=active->getTrackTxInstrument()->getName(),nameB=engine.instrumentManager->addInstrument();
            auto* first=engine.instrumentManager->getInstrument(nameA);auto* second=engine.instrumentManager->getInstrument(nameB);
            check(songs.assignDesktopSoundFont(nameA,bankA,SoundFontProgram::encode(0,0),error),"assign stereo SF2 to the first instrument");
            check(songs.assignDesktopSoundFont(nameB,bankB,SoundFontProgram::encode(0,0),error),"assign a different SF2 to the second instrument");
            auto fontA=songs.soundFontForInstrument(first,error),fontB=songs.soundFontForInstrument(second,error);
            check(fontA && fontB && fontA!=fontB && songs.soundFontForInstrument(first,error)==fontA,"loaded banks are shared by path without merging distinct banks");
            Track a(0,"Audio probe A",false,&songs),b(0,"Audio probe B",false,&songs);
            a.setTrackTxInstrument(first);b.setTrackTxInstrument(second);a.prepareDesktopMonitor();b.prepareDesktopMonitor();
            a.sfNoteOn(0,69,0.8f);b.sfNoteOn(0,69,0.8f);
            auto firstAudio=render(a),secondAudio=render(b);
            check(a.audioBuff.getNumChannels()==2 && std::abs(frequency(firstAudio[0])-440)<5 && std::abs(frequency(firstAudio[1])-880)<8,"stereo SF2 regions retain independent pitches in both channels");
            check(std::abs(frequency(secondAudio[0])-880)<8 && std::abs(frequency(secondAudio[1])-880)<8,"two instruments render different banks at the same time");
            b.sfHandleMidi(juce::MidiMessage::controllerEvent(1,10,0));
            // The full SF2 engine smooths controller changes; measure the settled pan.
            render(b,1);auto panned=render(b);
            check(rms(panned[0])>0.001 && rms(panned[1])<1e-8,"MIDI pan reaches the stereo render output");
            b.sfHandleMidi(juce::MidiMessage::controllerEvent(1,10,64));
            const auto oldDefault=songs.get_tsf_instance();
            check(songs.browseDesktopSoundFont(bankB,error) && songs.get_tsf_instance()==oldDefault && first->_instrumentData->soundFontPath==bankA,"browsing another bank preserves the default and existing assignments");
            check(songs.desktopPreviewPreset(0),"a browsed bank can be previewed independently");
            songs.stopJuceAudioCallback();float preview[1920]{};
            std::vector<float> previewAudio;for(int i=0;i<20;++i){songs.renderDesktopStereo(preview);previewAudio.insert(previewAudio.end(),preview,preview+960);}
            songs.desktopStopPreview();
            check(std::abs(frequency(previewAudio)-523.251)<8,"preview uses the browsed bank rather than the default bank");
            check(!songs.assignDesktopSoundFont(nameA,root.getChildFile("missing.sf2").getFullPathName().toStdString(),SoundFontProgram::encode(0,0),error) && first->_instrumentData->soundFontPath==bankA,"failed bank assignment preserves the previous instrument bank");
            check(!songs.assignDesktopSoundFont(nameA,bankB,SoundFontProgram::encode(0,127),error) && first->_instrumentData->soundFontPath==bankA,"unavailable preset is rejected before changing the assignment");
            command(ModuleName::moduleInstrument,InstrumentParam::paramSetActiveInstrument,nameA);
            command(ModuleName::moduleInstrument,InstrumentParam::paramDesktopSoundFontPath,bankB);
            command(ModuleName::moduleInstrument,InstrumentParam::paramUndoCmd,uint8_t(1));
            check(first->_instrumentData->soundFontPath==bankA,"instrument Undo restores its previous SF2 assignment");
            check(songs.loadDesktopSoundFont(bankB,error) && songs.soundFontPathForInstrument(first)==bankA,"changing the default does not replace an explicitly assigned bank");
            check(songs.loadDesktopSoundFont(bankA,error),"restore the default bank after replacement testing");
            auto rebuild=[&]{b.unloadDesktopSoundFont();b.prepareDesktopMonitor();};
            for(int limit:{8,32,64})
            {
                songs.setDesktopVoiceLimit(limit);rebuild();for(int note=48;note<72;++note)b.sfNoteOn(0,uint8_t(note),0.5f);
                check(b.desktopActiveVoices()==std::min(limit,24),"configured voice limit "+juce::String(limit)+" handles a 24-note chord");
            }
            songs._state=SYSTEM_STATE::PLAY;songs.setDesktopVoiceLimit(32);
            check(songs.getSystemState()==SYSTEM_STATE::IDLE,"changing voice limits while playing leaves transport stopped");
            songs.setDesktopVoiceLimit(64);
            check(!songs.setDesktopVoiceLimit(0) && !songs.setDesktopVoiceLimit(999) && songs.desktopVoiceLimit==64,"invalid voice limits preserve the current setting");
            a.unloadDesktopSoundFont();a.prepareDesktopMonitor();rebuild();
            a.setInternalAudioEffects(uint8_t(InternalAudioCC::RELEASE),1000,false);b.setInternalAudioEffects(uint8_t(InternalAudioCC::RELEASE),100,false);
            a.sfNoteOn(0,69,0.8f);b.sfNoteOn(0,69,0.8f);render(a,3);render(b,3);a.sfNoteOff(0,69);b.sfNoteOff(0,69);render(a,20);render(b,20);
            check(a.desktopActiveVoices()>0 && b.desktopActiveVoices()==0,"release time is per synth and uses the configured milliseconds");

            b.sfReset();b.effectsInit();
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_HIGH),4000,false);b.setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_HIGH),6,true);
            auto high=b.eqFilter[FILTER_HIGHEQ_INDEX].state;
            check(high->getMagnitudeForFrequency(12000,48000)>high->getMagnitudeForFrequency(100,48000)*1.8,"high EQ boosts high frequencies rather than bass");
            command(ModuleName::moduleInstrument,InstrumentParam::paramSetActiveInstrument,nameA);
            command(ModuleName::moduleInstrument,InstrumentParam::paramTVActiveKnobCmd,int8_t(22));
            command(ModuleName::moduleInstrument,InstrumentParam::paramTVMidiCCValue,-6.0f);
            check(engine.instrumentManager->FindParameter(paramId_t(InstrumentParam::paramTVMidiCCValue))->getValue().get<float>()==-6.0f,"firmware control parameters accept negative EQ gain");
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_HIGH),0,true);
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_LOW),1000,false);b.setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_LOW),-6,true);
            auto low=b.eqFilter[FILTER_LOWEQ_INDEX].state;
            check(low->getMagnitudeForFrequency(100,48000)<low->getMagnitudeForFrequency(12000,48000)*0.6,"low EQ cuts bass without cutting treble");
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_LOW),0,true);
            check(!b.EQisEnabled[0] && !b.EQisEnabled[1],"zero dB bypasses both EQ bands");
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::VOLUME),100,true);
            auto clear=[&]{std::fill_n(b._twentyMsBuffer,1920,0.0f);};
            b.eqFilter[0].reset();b.setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_LOW),6,true);clear();b._twentyMsBuffer[0]=1;b.processInternalEffects();
            bool isolated=true;for(int i=960;i<1920;++i)isolated &= b._twentyMsBuffer[i]==0;
            check(isolated,"stereo EQ maintains separate channel histories");
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_LOW),0,true);
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::REVERB),0,true);b.setInternalAudioEffects(uint8_t(InternalAudioCC::REVERB),100,false);
            check(b.igniteReverb.getParameters().dryLevel==0 && b.igniteReverb.getParameters().width==1,"reverb knob changes reach the actual processor");
            b.igniteReverb.reset();double tail=0;
            for(int block=0;block<12;++block){clear();if(!block)b._twentyMsBuffer[0]=1;b.processInternalEffects();if(block)for(int i=0;i<1920;++i)tail+=std::abs(b._twentyMsBuffer[i]);}
            check(tail>0.001,"stereo reverb produces an audible impulse tail");
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::REVERB),0,false);
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::CHORUS),3,false);b.setInternalAudioEffects(uint8_t(InternalAudioCC::CHORUS),50,true);
            b.sfNoteOn(0,69,0.8f);auto chorus=render(b);
            bool finite=true;for(const auto& channel:chorus)for(auto sample:channel)finite &= std::isfinite(sample);
            check(b.chorusStatus[0] && finite && rms(chorus[0])>0.001,"chorus processes stereo without out-of-bounds state access");
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::CHORUS),0,false);
            check(!b.chorusStatus[0],"chorus zero rate bypasses the effect");
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::VOLUME),0,true);auto silent=render(b,2);
            check(rms(silent[0])==0 && rms(silent[1])==0,"track gain silences both channels");
            b.setInternalAudioEffects(uint8_t(InternalAudioCC::VOLUME),100,true);
            songs.desktopPanic();active->prepareDesktopMonitor();active->sfNoteOn(0,69,0.8f);float mixed[1920]{};
            songs.renderDesktopStereo(mixed);
            check(std::any_of(mixed,mixed+960,[](float x){return std::abs(x)>0.0001f;}) && std::any_of(mixed+960,mixed+1920,[](float x){return std::abs(x)>0.0001f;}),"stereo mixer renders both output channels");
            songs.prepareToPlay(4800,48000);juce::AudioBuffer<float> source(2,4800);
            songs.getNextAudioBlock({&source,0,4800});
            std::vector<float> sourceLeft(source.getReadPointer(0),source.getReadPointer(0)+4800),sourceRight(source.getReadPointer(1),source.getReadPointer(1)+4800);
            check(std::abs(frequency(sourceLeft)-440)<5 && std::abs(frequency(sourceRight)-880)<8,"Windows audio source preserves independent stereo through variable block adaptation");
            auto preferences=IgniteDesktop::readPreferences();preferences.getDynamicObject()->setProperty("SynthVoices",64);preferences.getDynamicObject()->setProperty("SoundFont",juce::String(bankA));IgniteDesktop::writePreferences(preferences);
            check(engine.SaveAllData(),"independent SF2 instrument assignments save successfully");
            auto* data=new juce::DynamicObject();data->setProperty("instrumentA",juce::String(nameA));data->setProperty("instrumentB",juce::String(nameB));manifestFile.replaceWithText(juce::JSON::toString(juce::var(data)));
            auto archive=root.getChildFile("sound-backup.zip");
            check(IgniteDesktop::ProjectArchive::exportTo(IgniteDesktop::ProjectArchive::dataRoot(),archive).wasOk(),"project backup exports independent bank assignments");
            juce::ZipFile zip(archive);bool samples=false;for(int i=0;i<zip.getNumEntries();++i)samples |= zip.getEntry(i)->filename.endsWithIgnoreCase(".sf2");
            check(!samples,"project backup keeps SF2 sample files external");
            check(IgniteDesktop::ProjectArchive::restoreFrom(archive,IgniteDesktop::ProjectArchive::dataRoot()).wasOk(),"project backup restores multi-bank instruments for fresh-process verification");
        }
        juce::Logger::writeToLog("Sound checks: "+juce::String(checks)+", failures: "+juce::String(failures));return failures;
    }
};
