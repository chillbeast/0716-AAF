// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopMidiSong.h"
#include "DesktopMediaJob.h"

class DesktopMidiSongTest
{
public:
    DesktopMidiSongTest(Ignite::IgniteEngine& model,std::function<void(int)> finished):engine(model),done(std::move(finished)){}
    void start(bool verify)
    {
        using namespace Ignite;using namespace IgniteDesktop::Media;
        root=SongManager::desktopSettingsFile().getParentDirectory();
        try
        {
            if(verify)
            {
                auto manifest=juce::XmlDocument::parse(root.getChildFile("song-manifest.xml"));
                if(!check(bool(manifest),"song import manifest exists")){finish();return;}
                const auto name=manifest->getStringAttribute("name").toStdString();
                auto& songs=*engine.songManager;
                for(const auto& item:songs.songInfoList)if(item.second==name)songs.setActiveSong(item.first);
                inspect();
                check(songs.activeSong->getName()==name,"imported song selection survives reopening");
                check(!engine.desktopHasUnsavedChanges(),"saved imported song reopens without pending edits");
                songs.setActiveSong(0);check(songs.activeSong && songs.activeSong->getName()!=name,"the previous song remains available after import");
                finish();return;
            }
            check(engine.SaveAllData(),"save the existing arrangement before song import");
            previous=engine.songManager->activeSong->getName();
            makeFixture();const auto file=root.getChildFile("Song test.mid");
            plan=MidiSong::parse(file);
            check(plan.parts.size()==2 && plan.sections.size()==4,"MIDI tracks and tempo/meter boundaries form a complete arrangement");
            check(plan.sections[2].numerator==3 && plan.sections[2].barSteps==12,"three-quarter meter selects the correct global bar length");
            check(plan.sections.back().end==14*48,"end-of-track preserves trailing duration without extra cleanup bars");
            check(plan.retriggers==2,"held notes crossing scene boundaries are explicitly retriggered");
            check(plan.patterns[1][0].BarDataList.at(0).StepDataList.at(0).nodeList.count(64)==1,"a note spanning a tempo boundary appears in the following scene");
            check(plan.patterns[1][0].BarDataList.at(0).StepDataList.at(0).auxList.size()>=3,"controller and pitch state is restored at a scene boundary");
            check(plan.patterns[2][0].patternLastStepId==12,"partial pattern bars keep their exact active length");
            invalidFixtures();
            fidelityFixtures();
            const auto count=engine.songManager->songInfoList.size();
            engine.desktopEdited("song");bool refused=false;try{plan.apply(engine);}catch(...){refused=true;}
            check(refused && engine.songManager->songInfoList.size()==count,"song import protects unsaved work without creating partial entries");
            check(engine.SaveAllData(),"save clears the import prerequisite");
            imported=plan.apply(engine);
            check(engine.songManager->songInfoList.size()==count+1,"song import adds one song while retaining the collection");
            check(engine.desktopHasUnsavedChanges(),"new song, patterns and instruments participate in pending-change tracking");
            inspect();
            check(engine.SaveAllData(),"normal Save All persists the imported arrangement");
            check(!engine.desktopHasUnsavedChanges(),"saving clears all imported library edits");
            juce::XmlElement manifest("SongImport");manifest.setAttribute("name",imported);manifest.writeTo(root.getChildFile("song-manifest.xml"));
            // Refresh the real firmware UI as the Projects button does.
            ui::IgniteSong::getAppSongInstance()->updateDataFromEngine();
            ui::IgnitePattern::getAppPatternInstance()->updateDataFromEngine();
            target=root.getChildFile("song-roundtrip.mid");
            begin(capture(engine,false,-1,"midi"),[this](juce::Result result)
            {
                check(result.wasOk(),"imported song exports as a complete MIDI arrangement");
                try
                {
                    const auto roundtrip=IgniteDesktop::Media::MidiSong::parse(target);
                    check(roundtrip.sections.size()==4 && roundtrip.sections.back().end==14*48,"MIDI round trip retains tempo segments and total duration");
                    check(roundtrip.sections[2].numerator==3 && roundtrip.sections[2].denominator==4,"MIDI round trip retains meter changes");
                    check(std::abs(roundtrip.sections[1].tempo-98.6)<0.001,"MIDI round trip retains fractional tempo");
                    check(roundtrip.parts.size()==2,"MIDI round trip retains musical tracks and channels");
                }
                catch(const std::exception& error){check(false,juce::String(error.what()));}
                renderAudio();
            });
        }
        catch(const std::exception& error){check(false,juce::String(error.what()));finish();}
    }
private:
    void makeFixture()
    {
        juce::MidiFile file;file.setTicksPerQuarterNote(480);juce::MidiMessageSequence conductor,piano,drums;
        auto tempo=[&](double bpm,double quarter){conductor.addEvent(juce::MidiMessage::tempoMetaEvent(int(std::llround(60000000.0/bpm))),quarter*480);};
        tempo(123.4,0);tempo(98.6,4);tempo(111.1,11);
        conductor.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4,4),0);
        conductor.addEvent(juce::MidiMessage::timeSignatureMetaEvent(3,4),8*480);
        conductor.addEvent(juce::MidiMessage::textMetaEvent(6,"Intro"),0);
        conductor.addEvent(juce::MidiMessage::textMetaEvent(6,"Verse / Chorus"),8*480);
        piano.addEvent(juce::MidiMessage::textMetaEvent(3,"Piano / Lead"),0);
        piano.addEvent(juce::MidiMessage::programChange(1,0),0);
        piano.addEvent(juce::MidiMessage::controllerEvent(1,7,100),0);
        piano.addEvent(juce::MidiMessage::controllerEvent(1,11,90),0);
        piano.addEvent(juce::MidiMessage::pitchWheel(1,10000),2*480);
        piano.addEvent(juce::MidiMessage::programChange(1,1),4*480);
        piano.addEvent(juce::MidiMessage::controllerEvent(1,0,0),int(10.75*480));
        piano.addEvent(juce::MidiMessage::controllerEvent(1,32,42),int(10.75*480));
        piano.addEvent(juce::MidiMessage::programChange(1,7),11*480);
        auto note=[&](juce::MidiMessageSequence& sequence,int channel,int pitch,double start,double end)
        {sequence.addEvent(juce::MidiMessage::noteOn(channel,pitch,uint8_t(100)),start*480);sequence.addEvent(juce::MidiMessage::noteOff(channel,pitch),end*480);};
        note(piano,1,60,0,1);note(piano,1,64,3.5,4.5);note(piano,1,67,8.25,9);note(piano,1,69,10.5,11.5);
        drums.addEvent(juce::MidiMessage::textMetaEvent(3,"Drums"),0);
        drums.addEvent(juce::MidiMessage::controllerEvent(10,0,0),0);
        drums.addEvent(juce::MidiMessage::controllerEvent(10,32,0),0);
        drums.addEvent(juce::MidiMessage::programChange(10,0),0);
        note(drums,10,36,0,0.5);note(drums,10,38,6,6.25);note(drums,10,42,10.75,11);
        conductor.addEvent(juce::MidiMessage::endOfTrack(),14*480);
        file.addTrack(conductor);file.addTrack(piano);file.addTrack(drums);
        auto output=root.getChildFile("Song test.mid").createOutputStream();file.writeTo(*output,1);
    }
    void invalidFixtures()
    {
        using IgniteDesktop::Media::MidiSong;
        auto reject=[&](const juce::File& file,const juce::String& text){bool refused=false;try{MidiSong::parse(file);}catch(...){refused=true;}check(refused,text);};
        auto save=[&](juce::MidiFile& midi,const char* name,int type=1)
        {const auto file=root.getChildFile(name);{auto output=file.createOutputStream();midi.writeTo(*output,type);}return file;};
        juce::MidiFile file;file.setTicksPerQuarterNote(480);juce::MidiMessageSequence notes;
        notes.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(100)),0);notes.addEvent(juce::MidiMessage::noteOff(1,60),480);file.addTrack(notes);
        reject(save(file,"type2.mid",2),"type 2 independent sequences are rejected before import");
        file.setSmpteTimeFormat(25,40);reject(save(file,"smpte.mid"),"SMPTE song timing is rejected");
        root.getChildFile("bad.mid").replaceWithText("invalid");reject(root.getChildFile("bad.mid"),"malformed song MIDI is rejected");
        juce::MidiFile dense;dense.setTicksPerQuarterNote(480);dense.addTrack(notes);juce::MidiMessageSequence tempo;
        for(int i=0;i<130;++i)tempo.addEvent(juce::MidiMessage::tempoMetaEvent(500000+i*1000),i*120);
        dense.addTrack(tempo);reject(save(dense,"dense.mid"),"too many tempo scenes are rejected before mutation");
        juce::MidiFile tiny;tiny.setTicksPerQuarterNote(480);tiny.addTrack(notes);juce::MidiMessageSequence odd;
        odd.addEvent(juce::MidiMessage::tempoMetaEvent(500000),48);odd.addEvent(juce::MidiMessage::timeSignatureMetaEvent(5,4),0);tiny.addTrack(odd);
        const auto rounded=MidiSong::parse(save(tiny,"rounded.mid"));
        check(rounded.roundedBoundaries>0 && rounded.normalisedMeters>0,"off-grid boundaries and long meters are reported explicitly");
        juce::MidiFile single;single.setTicksPerQuarterNote(480);single.addTrack(notes);
        check(MidiSong::parse(save(single,"type0.mid",0)).parts.size()==1,"type 0 channel data imports successfully");
        juce::MidiFile many;many.setTicksPerQuarterNote(480);for(int i=0;i<65;++i)many.addTrack(notes);
        reject(save(many,"many.mid"),"more than 64 parts is rejected");
        const uint8_t badTempo[]{0xff,0x51,0x01,0};
        juce::MidiFile malformed;malformed.setTicksPerQuarterNote(480);juce::MidiMessageSequence metadata;
        metadata.addEvent(juce::MidiMessage(badTempo,4),0);malformed.addTrack(metadata);malformed.addTrack(notes);
        reject(save(malformed,"bad-tempo.mid"),"short tempo payloads are rejected before reading their bytes");
        const uint8_t badMeter[]{0xff,0x58,0x04,4,255,24,8};
        juce::MidiFile invalidMeter;invalidMeter.setTicksPerQuarterNote(480);juce::MidiMessageSequence signature;
        signature.addEvent(juce::MidiMessage(badMeter,7),0);invalidMeter.addTrack(signature);invalidMeter.addTrack(notes);
        reject(save(invalidMeter,"bad-meter.mid"),"invalid meter exponents are rejected before conversion");
        juce::MidiFile reset;reset.setTicksPerQuarterNote(480);juce::MidiMessageSequence controllers;
        controllers.addEvent(juce::MidiMessage::controllerEvent(1,11,10),0);
        controllers.addEvent(juce::MidiMessage::pitchWheel(1,10000),0);
        controllers.addEvent(juce::MidiMessage::controllerEvent(1,121,0),240);
        controllers.addEvent(juce::MidiMessage::tempoMetaEvent(600000),480);
        controllers.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(100)),0);
        controllers.addEvent(juce::MidiMessage::noteOff(1,60),960);reset.addTrack(controllers);
        const auto resetPlan=MidiSong::parse(save(reset,"reset.mid"));
        check(resetPlan.patterns[1][0].BarDataList.at(0).StepDataList.at(0).auxList.empty(),"controller reset prevents stale expression and bend from returning in later scenes");
    }
    void inspect()
    {
        using namespace Ignite;
        auto& song=*engine.songManager->activeSong;
        if(!check(song.getNumOfScenes()==4,"the model contains all four imported scenes"))return;
        auto it=song._songData->sceneList.begin();auto& first=*it++;
        check(first.getTempo()>123.39 && first.getTempo()<123.41,"fractional initial scene tempo is preserved");
        check(first._sceneData->numberOfTracks==2 && first._sceneData->trackList.front().getTrackMainPattern(),"imported patterns and instruments are assigned to audible tracks");
        auto& second=*it++;
        check(std::abs(second.getTempo()-98.6)<0.001,"fractional tempo change survives the model/save path");
        check(SoundFontProgram::program(second._sceneData->trackList.front().getTrackTxInstrument()->getInternalSoundPgm())==1,"program changes select the next scene's instrument");
        auto& third=*it++;
        check(third._sceneData->desktopMeterNumerator==3 && third.getStepsPerBar()==12,"saved time signature and three-quarter scene grid agree");
        auto& fourth=*it;
        check(SoundFontProgram::bank(fourth._sceneData->trackList.front().getTrackTxInstrument()->getInternalSoundPgm())==42,"bank and program changes carry into later scenes");
        auto drum=first._sceneData->trackList.begin();++drum;
        check(drum->getTrackTxInstrument()->getMidiChannel()==9 && SoundFontProgram::bank(drum->getTrackTxInstrument()->getInternalSoundPgm())==128,"GM percussion uses channel ten and SF2 bank 128");
    }
    void renderAudio()
    {
        try
        {
            target=root.getChildFile("song.wav");
            begin(IgniteDesktop::Media::capture(engine,false,-1,"wav"),[this](juce::Result result)
            {
                check(result.wasOk(),"imported song renders to WAV with its assigned sounds");
                juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(target.createInputStream().release(),true));
                const double seconds=4*60.0/plan.sections[0].tempo+7*60.0/plan.sections[1].tempo+3*60.0/plan.sections[3].tempo+2;
                check(reader && std::abs(reader->lengthInSamples-std::llround(seconds*48000))<6,"audio duration follows all imported tempo and meter sections");
                if(reader)
                {
                    juce::AudioBuffer<float> block(2,48000);reader->read(&block,0,48000,0,true,true);
                    check(block.getMagnitude(0,48000)>0.0001f,"imported MIDI produces audible internal audio");
                }
                pedalAudio();
            });
        }
        catch(const std::exception& error){check(false,juce::String(error.what()));finish();}
    }
    void fidelityFixtures()
    {
        using namespace IgniteDesktop::Media;
        auto save=[&](const juce::MidiMessageSequence& sequence,const char* name)
        {
            juce::MidiFile midi;midi.setTicksPerQuarterNote(480);juce::MidiMessageSequence conductor;
            conductor.addEvent(juce::MidiMessage::tempoMetaEvent(500000),0);
            conductor.addEvent(juce::MidiMessage::textMetaEvent(6,"Continuation"),4*480);
            conductor.addEvent(juce::MidiMessage::endOfTrack(),8*480);midi.addTrack(conductor);midi.addTrack(sequence);
            const auto file=root.getChildFile(name);{auto output=file.createOutputStream();midi.writeTo(*output,1);}return file;
        };
        juce::MidiMessageSequence pedal;
        pedal.addEvent(juce::MidiMessage::controllerEvent(1,64,127),0);
        pedal.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(100)),0);
        pedal.addEvent(juce::MidiMessage::noteOff(1,60),480);
        pedal.addEvent(juce::MidiMessage::controllerEvent(1,64,0),6*480);
        const auto held=MidiSong::parse(save(pedal,"pedal.mid"));
        check(held.parts[0].notes[0].end==48 && held.parts[0].notes[0].soundEnd==6*48,"pedal-held notes retain physical and sounding durations independently");
        check(held.patterns[0][0].BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).lenInSteps==4,"the first scene retains the original physical note-off");
        check(held.patterns[1][0].BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).lenInSteps==8,"pedal-held voices are recreated in the next scene until pedal release");
        check(held.retriggers==1 && held.unmatched==0,"pedal boundary recreation is reported without false note repairs");
        for(int cc:{120,123,124,125,126,127})
        {
            juce::MidiMessageSequence off;
            off.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(100)),0);
            off.addEvent(juce::MidiMessage::controllerEvent(1,cc,0),2*480);
            off.addEvent(juce::MidiMessage::noteOff(1,60),6*480);
            const auto plan=MidiSong::parse(save(off,("mode-"+std::to_string(cc)+".mid").c_str()));
            check(plan.parts[0].notes[0].soundEnd==2*48 && plan.patterns[1][0].BarDataList.at(0).StepDataList.at(0).nodeList.empty() && plan.unmatched==0,
                "channel mode "+juce::String(cc)+" ends notes without reviving them in a later scene");
        }
        for(int cc:{120,121,123})
        {
            auto sequence=pedal;sequence.addEvent(juce::MidiMessage::controllerEvent(1,cc,0),2*480);
            const auto plan=MidiSong::parse(save(sequence,("pedal-mode-"+std::to_string(cc)+".mid").c_str()));
            check(plan.parts[0].notes[0].soundEnd==(cc==123?6:2)*48,
                "pedal state obeys channel mode "+juce::String(cc));
        }
        juce::MidiMessageSequence parameters;
        parameters.addEvent(juce::MidiMessage::controllerEvent(1,101,0),0);
        parameters.addEvent(juce::MidiMessage::controllerEvent(1,100,0),0);
        parameters.addEvent(juce::MidiMessage::controllerEvent(1,6,12),0);
        parameters.addEvent(juce::MidiMessage::controllerEvent(1,101,127),480);
        parameters.addEvent(juce::MidiMessage::controllerEvent(1,100,127),480);
        parameters.addEvent(juce::MidiMessage::pitchWheel(1,12000),2*480);
        parameters.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(100)),0);
        parameters.addEvent(juce::MidiMessage::noteOff(1,60),7*480);
        const auto rpn=MidiSong::parse(save(parameters,"rpn.mid"));
        const auto& restored=rpn.patterns[1][0].BarDataList.at(0).StepDataList.at(0).auxList;
        check(restored.size()==6 && restored[0].data1==101 && restored[0].data2==0 && restored[1].data1==100 && restored[2].data1==6 && restored[2].data2==12 && restored[3].data2==127,
            "RPN selection, data entry and deselection retain their order across scenes");
        parameters.addEvent(juce::MidiMessage::controllerEvent(1,121,0),3*480);
        const auto resetRpn=MidiSong::parse(save(parameters,"rpn-reset.mid"));
        const auto& resetEvents=resetRpn.patterns[1][0].BarDataList.at(0).StepDataList.at(0).auxList;
        check(resetEvents.size()==6 && resetEvents[2].data1==6 && resetEvents[2].data2==12 && resetEvents.back().data1==121,
            "Reset Controllers clears stale bend while preserving registered bend-range setup");
        auto reject=[&](const juce::File& file,bool clips,const juce::String& text)
        {bool refused=false;try{if(clips)MidiImport::parse(file);else MidiSong::parse(file);}catch(...){refused=true;}check(refused,text);};
        const uint8_t zeroTempo[]{0xff,0x51,0x03,0,0,0};auto bad=pedal;bad.addEvent(juce::MidiMessage(zeroTempo,6),0);
        const auto zero=save(bad,"zero-tempo.mid");reject(zero,false,"song import rejects zero tempo");reject(zero,true,"clip import rejects zero tempo");
        const uint8_t badPort[]{0xff,0x21,0};bad=pedal;bad.addEvent(juce::MidiMessage(badPort,3),0);
        const auto port=save(bad,"bad-port.mid");reject(port,false,"song import rejects malformed port metadata");reject(port,true,"clip import rejects malformed port metadata");
        const uint8_t unsupportedPort[]{0xff,0x21,1,24};bad=pedal;bad.addEvent(juce::MidiMessage(unsupportedPort,4),0);
        reject(save(bad,"unsupported-port.mid"),false,"unavailable logical ports are rejected instead of silently remapped");
        bool validPorts=true,cvRejected=true;
        for(int portId=0;portId<int(midiDataIOPorts::maxMidiIOPorts);++portId)
        {
            const uint8_t bytes[]{0xff,0x21,1,uint8_t(portId)};
            juce::MidiMessageSequence part;part.addEvent(juce::MidiMessage(bytes,4),0);
            part.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(100)),0);
            part.addEvent(juce::MidiMessage::noteOff(1,60),480);
            bool accepted=false;int imported=-1;
            try{const auto plan=MidiSong::parse(save(part,("port-"+std::to_string(portId)+".mid").c_str()));accepted=true;imported=plan.parts[0].port;}catch(...){}
            if(portId>=int(midiDataIOPorts::cv1) && portId<=int(midiDataIOPorts::cv8))cvRejected&=!accepted;
            else validPorts&=accepted && imported==portId;
        }
        check(validPorts,"all 16 DIN, USB-device and USB-host MIDI port IDs survive import without remapping");
        check(cvRejected,"CV destinations are rejected as MIDI file port routes");
        juce::MidiMessageSequence repeats;
        repeats.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(100)),0);
        repeats.addEvent(juce::MidiMessage::noteOn(1,60,uint8_t(90)),240);
        repeats.addEvent(juce::MidiMessage::noteOff(1,60),480);
        repeats.addEvent(juce::MidiMessage::noteOff(1,60),960);
        const auto clips=MidiImport::parse(save(repeats,"repeated.mid"));
        check(clips.unmatched==0 && clips.patterns[0].BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).lenInSteps==4,
            "clip import preserves FIFO note-off pairing for overlapping pitches");
    }
    void pedalAudio()
    {
        try
        {
            const auto plan=IgniteDesktop::Media::MidiSong::parse(root.getChildFile("pedal.mid"));
            plan.apply(engine);check(engine.SaveAllData(),"pedal arrangement saves through the normal project path");
            target=root.getChildFile("pedal.wav");
            begin(IgniteDesktop::Media::capture(engine,false,-1,"wav"),[this](juce::Result result)
            {
                check(result.wasOk(),"pedal-held arrangement renders through the isolated WAV exporter");
                juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(target.createInputStream().release(),true));
                if(check(bool(reader),"pedal WAV is readable"))
                {
                    juce::AudioBuffer<float> buffer(2,9600);
                    reader->read(&buffer,0,9600,67200,true,true);const float before=buffer.getRMSLevel(0,0,9600);
                    reader->read(&buffer,0,9600,108000,true,true);const float after=buffer.getRMSLevel(0,0,9600);
                    check(before>0.0001f && after>before*0.4f,"pedal-held audio remains audible after the imported scene boundary");
                    reader->read(&buffer,0,9600,216000,true,true);
                    check(buffer.getRMSLevel(0,0,9600)<before*0.1f,"pedal release ends the recreated voice without stuck notes");
                }
                engine.songManager->setActiveSong(0);
                check(engine.songManager->activeSong->getName()==previous,"the original arrangement remains unchanged and selectable");finish();
            });
        }
        catch(const std::exception& error){check(false,juce::String(error.what()));finish();}
    }
    bool check(bool value,const juce::String& text)
    {++checks;if(!value)++failures;juce::Logger::writeToLog(juce::String(value?"PASS: ":"FAIL: ")+text);return value;}
    void finish(){juce::Logger::writeToLog("MIDI song checks: "+juce::String(checks)+", failures: "+juce::String(failures));done(failures);}
    void begin(std::unique_ptr<juce::XmlElement> snapshot,std::function<void(juce::Result)> callback)
    {auto result=job.start(std::move(snapshot),target,std::move(callback));if(result.failed())throw std::runtime_error(result.getErrorMessage().toStdString());}
    Ignite::IgniteEngine& engine;
    std::function<void(int)> done;
    IgniteDesktop::Media::Job job;
    IgniteDesktop::Media::MidiSong plan;
    juce::File root,target;
    std::string previous,imported;
    int checks=0,failures=0;
};
