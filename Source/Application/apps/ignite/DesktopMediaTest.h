// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopMidiImport.h"
#include "DesktopMediaJob.h"

class DesktopMediaTest
{
public:
    DesktopMediaTest(Ignite::IgniteEngine& model,std::function<void(int)> finished):engine(model),done(std::move(finished)){}
    void start(bool verify)
    {
        using namespace Ignite;
        root=SongManager::desktopSettingsFile().getParentDirectory();
        if(verify)
        {
            auto xml=juce::XmlDocument::parse(root.getChildFile("media-manifest.xml"));
            if(!check(bool(xml),"MIDI import manifest exists")){finish();return;}
            for(auto* item:xml->getChildIterator())check(engine.patternManager->GetPattern(item->getStringAttribute("name").toStdString())!=nullptr,"imported pattern survives a fresh launch");
            engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleSong),paramId_t(SongParam::paramActiveSong),int32_t(0));
            engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleScene),paramId_t(SceneParam::paramActiveScene),int32_t(0));
            engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleTrack),paramId_t(TrackParam::paramActiveTrack),uint8_t(0));
            auto* reopened=engine.songManager->activeSong->editScene->editTrack;reopened->prepareDesktopMonitor();
            check(reopened->_trackData->hasDesktopFx && reopened->EQgainDb[FILTER_HIGHEQ_INDEX]==6,"saved effect settings are restored in a fresh process");
            finish();return;
        }
        auto& songs=*engine.songManager;
        engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleScene),paramId_t(SceneParam::paramActiveScene),int32_t(0));
        engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleTrack),paramId_t(TrackParam::paramActiveTrack),uint8_t(0));
        source=songs.activeSong->editScene->editTrack->getTrackMainPattern()->getName();
        engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleScene),paramId_t(SceneParam::paramSceneTempo),120.0f);
        engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleScene),paramId_t(SceneParam::paramSceneSwing),uint8_t(0));
        songs.activeSong->editScene->setTempo(120);songs.activeSong->editScene->setSwing(0);songs.activeSong->editScene->setBarCount(1);engine.SaveAllData();
        juce::MidiFile midi;midi.setTicksPerQuarterNote(480);juce::MidiMessageSequence tempo,first,second;
        tempo.addEvent(juce::MidiMessage::tempoMetaEvent(500000),0);midi.addTrack(tempo);
        first.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(101)),0);first.addEvent(juce::MidiMessage::noteOff(1,60),240);
        first.addEvent(juce::MidiMessage::controllerEvent(1,11,64),480);first.addEvent(juce::MidiMessage::noteOn(1,64,uint8_t(90)),480);first.addEvent(juce::MidiMessage::noteOff(1,64),960);
        first.addEvent(juce::MidiMessage::noteOn(1,72,uint8_t(80)),65*480);first.addEvent(juce::MidiMessage::noteOff(1,72),66*480);midi.addTrack(first);
        second.addEvent(juce::MidiMessage::noteOn(2,67,uint8_t(95)),0);second.addEvent(juce::MidiMessage::pitchWheel(2,10000),120);second.addEvent(juce::MidiMessage::noteOff(2,67),480);midi.addTrack(second);
        auto input=root.getChildFile("original-clip.mid");{auto output=input.createOutputStream();midi.writeTo(*output);}
        auto clips=IgniteDesktop::Media::MidiImport::parse(input);
        check(clips.patterns.size()==3 && clips.tempo==120,"MIDI import groups channels and splits a long clip into 16-bar parts");
        check(clips.patterns.front().BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).velocity==101,"MIDI import preserves note velocity");
        const auto& note=clips.patterns.front().BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0);
        check(note.lenInSteps*12+note.lenInTicks==24,"MIDI PPQ conversion preserves note duration at the internal resolution");
        const auto count=engine.patternManager->patternList.size();const auto originalSong=songs.activeSong.get();
        auto names=clips.apply(engine);
        check(names.size()==3 && engine.patternManager->patternList.size()==count+3 && songs.activeSong.get()==originalSong && songs.activeSong->editScene->editTrack->getTrackMainPattern()->getName()==source,"import adds new library patterns without replacing the current arrangement");
        check(engine.desktopHasUnsavedChanges(),"imported patterns participate in unsaved-change protection");
        root.getChildFile("bad.mid").replaceWithText("not a MIDI file");bool rejected=false;try{IgniteDesktop::Media::MidiImport::parse(root.getChildFile("bad.mid"));}catch(...){rejected=true;}
        check(rejected && engine.patternManager->patternList.size()==count+3,"malformed MIDI is rejected before model changes");
        juce::MidiFile smpte;smpte.setSmpteTimeFormat(25,40);smpte.addTrack(first);{auto output=root.getChildFile("smpte.mid").createOutputStream();smpte.writeTo(*output);}rejected=false;try{IgniteDesktop::Media::MidiImport::parse(root.getChildFile("smpte.mid"));}catch(...){rejected=true;}check(rejected,"SMPTE MIDI timing is rejected with an actionable error");
        juce::MidiFile empty;empty.setTicksPerQuarterNote(480);empty.addTrack(tempo);{auto output=root.getChildFile("empty.mid").createOutputStream();empty.writeTo(*output);}rejected=false;try{IgniteDesktop::Media::MidiImport::parse(root.getChildFile("empty.mid"));}catch(...){rejected=true;}check(rejected,"MIDI without musical events is rejected");
        juce::MidiFile ccOnly;ccOnly.setTicksPerQuarterNote(480);juce::MidiMessageSequence ccEvents;ccEvents.addEvent(juce::MidiMessage::controllerEvent(1,11,32),480);ccOnly.addTrack(ccEvents);{auto output=root.getChildFile("cc-only.mid").createOutputStream();ccOnly.writeTo(*output);}auto ccClips=IgniteDesktop::Media::MidiImport::parse(root.getChildFile("cc-only.mid"));check(ccClips.patterns.at(0).BarDataList.at(0).StepDataList.at(4).auxList.size()==1,"the final controller event in a MIDI clip is retained");
        juce::XmlElement manifest("Imported");for(const auto& name:names)manifest.createNewChildElement("Pattern")->setAttribute("name",name);manifest.writeTo(root.getChildFile("media-manifest.xml"));
        check(engine.SaveAllData(),"imported patterns save through the regular save path");
        auto* fxTrack=songs.activeSong->editScene->editTrack;fxTrack->prepareDesktopMonitor();fxTrack->setInternalAudioEffects(uint8_t(InternalAudioCC::EQ_HIGH),6,true);
        check(engine.desktopHasUnsavedChanges() && engine.SaveAllData(),"effect edits are tracked and saved with the song");
        fxTrack->unloadDesktopSoundFont();fxTrack->prepareDesktopMonitor();check(fxTrack->EQgainDb[FILTER_HIGHEQ_INDEX]==6,"effect settings survive synth reconstruction");
        next();
    }
private:
    bool check(bool value,const juce::String& text)
    {++checks;if(!value)++failures;juce::Logger::writeToLog(juce::String(value?"PASS: ":"FAIL: ")+text);return value;}
    void finish(){juce::Logger::writeToLog("Media checks: "+juce::String(checks)+", failures: "+juce::String(failures));done(failures);}
    void next()
    {
        using namespace Ignite;
        try
        {
            if(phase==0)
            {
                target=root.getChildFile("mix.wav");auto snapshot=IgniteDesktop::Media::capture(engine,true,-1,"wav");
                begin(std::move(snapshot),[this](juce::Result result)
                {
                    check(result.wasOk(),"isolated renderer exports a stereo WAV");
                    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(target.createInputStream().release(),true));
                    if(check(reader && reader->numChannels==2 && reader->sampleRate==48000 && reader->bitsPerSample==32 && reader->usesFloatingPointData,"WAV export writes stereo 48 kHz floating-point audio"))
                    {
                        check(std::abs(reader->lengthInSamples-192000)<4,"WAV duration includes one scene pass and the two-second tail");
                        juce::AudioBuffer<float> samples(2,int(reader->lengthInSamples));reader->read(&samples,0,samples.getNumSamples(),0,true,true);
                        check(samples.getMagnitude(0,samples.getNumSamples())>0.0001f,"WAV export contains rendered sound");
                    }
                    ++phase;next();
                });return;
            }
            if(phase==1)
            {
                auto node=engine.EngineGetPatternData(source).BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0);node.velocity=103;engine.EngineAddNodeData(source,0,0,node);
                target=root.getChildFile("arrangement.mid");auto snapshot=IgniteDesktop::Media::capture(engine,false,-1,"midi");
                begin(std::move(snapshot),[this](juce::Result result)
                {
                    check(result.wasOk(),"isolated renderer exports the MIDI arrangement");juce::MidiFile file;auto input=target.createInputStream();
                    if(check(input && file.readFrom(*input) && file.getNumTracks()>1 && file.getTimeFormat()==960,"exported MIDI contains a conductor and musical tracks"))
                    {
                        bool found=false;for(int track=0;track<file.getNumTracks();++track)for(int event=0;event<file.getTrack(track)->getNumEvents();++event)
                        {const auto& m=file.getTrack(track)->getEventPointer(event)->message;found|=m.isNoteOn() && m.getNoteNumber()==60 && m.getVelocity()==103;}
                        check(found,"MIDI export uses unsaved data captured at export start");
                    }
                    check(engine.EngineGetPatternData(source).BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).velocity==23 && engine.desktopHasUnsavedChanges(),"editing during export remains in the parent session and stays unsaved");
                    ++phase;next();
                });
                node.velocity=23;engine.EngineAddNodeData(source,0,0,node);return;
            }
            if(phase==2)
            {
                target=root.getChildFile("keep.wav");target.replaceWithText("previous output");auto snapshot=IgniteDesktop::Media::capture(engine,true,-1,"wav");
                auto* instrument=snapshot->getChildByName("Instruments")->getFirstChildElement();instrument->setAttribute("font",root.getChildFile("missing.sf2").getFullPathName());
                begin(std::move(snapshot),[this](juce::Result result){check(result.failed() && target.loadFileAsString()=="previous output","failed render preserves the existing destination file");++phase;next();});return;
            }
            if(phase==3)
            {
                target=root.getChildFile("cancel.wav");target.replaceWithText("keep after cancel");auto snapshot=IgniteDesktop::Media::capture(engine,false,-1,"wav");
                begin(std::move(snapshot),[this](juce::Result result){check(result.failed() && target.loadFileAsString()=="keep after cancel","cancelling an export preserves the existing destination file");++phase;next();});job.cancel();return;
            }
            if(phase==4)
            {
                auto& songs=*engine.songManager;auto* original=songs.activeSong->editScene;
                const auto instrument=original->editTrack->getTrackTxInstrument()->getName();original->setTransposeValue(2);
                auto step=engine.EngineGetPatternData(source).BarDataList.at(0).StepDataList.at(1);step.auxList.push_back({true,0xb0,10,64,0});step.auxList.push_back({true,0xa0,60,50,0});engine.EngineSetStepData(moduleId_t(ModuleName::moduleUI),source,0,1,step);
                auto command=[&](ModuleName module,auto parameter,AnyType value){engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(module),paramId_t(parameter),value);};
                command(ModuleName::moduleScene,SceneParam::paramAddSceneCmd,int32_t(0));command(ModuleName::moduleScene,SceneParam::paramActiveScene,int32_t(1));command(ModuleName::moduleTrack,TrackParam::paramActiveTrack,uint8_t(0));
                command(ModuleName::moduleTrack,TrackParam::paramTrackOutInstrument,instrument);command(ModuleName::moduleTrack,TrackParam::paramTrackPattern,source);command(ModuleName::moduleTrack,TrackParam::paramTrackMute,uint8_t(0));
                songs.activeSong->editScene->setTempo(90);songs.activeSong->editScene->setSwing(0);songs.activeSong->editScene->setBarCount(1);
                target=root.getChildFile("two-scenes.mid");auto snapshot=IgniteDesktop::Media::capture(engine,false,-1,"midi");
                begin(std::move(snapshot),[this](juce::Result result)
                {
                    check(result.wasOk(),"song MIDI export traverses each scene once");juce::MidiFile file;auto input=target.createInputStream();bool tempo120=false,tempo90=false,controller=false,pressure=false,transposed=false,second=false;
                    if(input && file.readFrom(*input))for(int track=0;track<file.getNumTracks();++track)for(int event=0;event<file.getTrack(track)->getNumEvents();++event)
                    {
                        const auto& m=file.getTrack(track)->getEventPointer(event)->message;
                        if(m.isTempoMetaEvent()){tempo120|=std::abs(m.getTempoSecondsPerQuarterNote()-0.5)<1e-5;tempo90|=std::abs(m.getTempoSecondsPerQuarterNote()-2.0/3)<1e-5;}
                        controller|=m.isController() && m.getControllerNumber()==10 && m.getControllerValue()==64;
                        pressure|=m.isAftertouch() && m.getNoteNumber()==62;
                        transposed|=m.isNoteOn() && m.getNoteNumber()==62 && m.getTimeStamp()<3840;
                        second|=m.isNoteOn() && m.getNoteNumber()==60 && std::abs(m.getTimeStamp()-3840)<1;
                    }
                    check(tempo120 && tempo90,"song MIDI export preserves scene tempo changes");check(controller,"scene transpose leaves MIDI controller numbers unchanged");check(pressure,"polyphonic pressure follows the transposed note pitch");check(transposed && second,"scene transpose and subsequent scene timing are reflected in MIDI notes");
                    engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleScene),paramId_t(SceneParam::paramActiveScene),int32_t(0));engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleTrack),paramId_t(TrackParam::paramActiveTrack),uint8_t(0));
                    ++phase;next();
                });return;
            }
            if(phase==5)
            {
                target=root.getChildFile("whole-song.wav");
                begin(IgniteDesktop::Media::capture(engine,false,-1,"wav"),[this](juce::Result result)
                {
                    check(result.wasOk(),"WAV export renders the whole song with multiple scenes");
                    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(target.createInputStream().release(),true));
                    check(reader && std::abs(reader->lengthInSamples-320000)<4,"whole-song WAV duration includes both scene tempos and a single final tail");
                    ++phase;next();
                });return;
            }
            if(phase==6)
            {
                target=root.getChildFile("locked.wav");target.replaceWithText("locked destination");
                locked=CreateFileW(target.getFullPathName().toWideCharPointer(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
                check(locked!=INVALID_HANDLE_VALUE,"export failure test locks the existing destination");
                begin(IgniteDesktop::Media::capture(engine,true,0,"wav"),[this](juce::Result result){if(locked!=INVALID_HANDLE_VALUE)CloseHandle(locked);locked=INVALID_HANDLE_VALUE;check(result.failed() && target.loadFileAsString()=="locked destination","failed destination replacement preserves the old file");++phase;next();});return;
            }
            if(phase==7)
            {
                auto data=engine.EngineGetPatternData(source);
                for(int index=0;index<16;++index){auto step=data.BarDataList.at(0).StepDataList.at(uint16_t(index));step.nodeList.clear();step.nodeCount=0;step.auxList.clear();engine.EngineSetStepData(moduleId_t(ModuleName::moduleUI),source,0,uint16_t(index),step);}
                ClientPatternNodeData node{};node.note=node.nodeId=72;node.velocity=100;node.onState=true;node.lenInSteps=8;node.step_id=12;engine.EngineAddNodeData(source,0,12,node);
                engine.songManager->activeSong->editScene->setBarCount(2);target=root.getChildFile("sustained.wav");
                begin(IgniteDesktop::Media::capture(engine,true,0,"wav"),[this](juce::Result result)
                {
                    check(result.wasOk(),"individual track WAV export supports multi-bar notes");juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(target.createInputStream().release(),true));
                    if(reader){juce::AudioBuffer<float> samples(2,12000);reader->read(&samples,0,4800,80000,true,true);const auto before=samples.getRMSLevel(0,0,4800);reader->read(&samples,4800,4800,103200,true,true);check(samples.getRMSLevel(0,4800,4800)>before*0.6f,"notes crossing a scene bar boundary are not cut off early");}
                    else check(false,"read sustained WAV");
                    ++phase;next();
                });return;
            }
            finish();
        }
        catch(const std::exception& error){check(false,juce::String(error.what()));finish();}
    }
    void begin(std::unique_ptr<juce::XmlElement> snapshot,std::function<void(juce::Result)> callback)
    {
        const auto result=job.start(std::move(snapshot),target,std::move(callback));if(result.failed())throw std::runtime_error(result.getErrorMessage().toStdString());
    }
    Ignite::IgniteEngine& engine;
    std::function<void(int)> done;
    IgniteDesktop::Media::Job job;
    juce::File root,target;
    std::string source;
    int phase=0,checks=0,failures=0;
    HANDLE locked=INVALID_HANDLE_VALUE;
};
