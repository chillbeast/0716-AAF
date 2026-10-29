// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopMidiImport.h"
#include "DesktopMidiNotes.h"
#include <numeric>
#include <tuple>

namespace IgniteDesktop::Media
{
    // A fully validated import plan. Parsing never touches the running engine.
    struct MidiSong
    {
        struct Event { juce::MidiMessage message; int tick; };
        using Note=MidiNotes::Note;
        struct Part
        {
            juce::String name;
            int channel=1,port=int(midiDataIOPorts::usbDevMidi1);
            std::vector<Note> notes;
            std::vector<Event> auxiliary;
        };
        struct Section
        {
            int begin=0,end=0,numerator=4,denominator=4,barSteps=16;
            double tempo=120;
            juce::String name;
        };
        juce::String name;
        std::vector<Part> parts;
        std::vector<Section> sections;
        std::vector<std::vector<ClientPatternData>> patterns;
        int roundedBoundaries=0,unmatched=0,duplicates=0,retriggers=0,normalisedMeters=0;
        int ignoredSysEx=0;

        static MidiSong parse(const juce::File& file)
        {
            if(file.getSize()>32*1024*1024)throw std::runtime_error("MIDI file exceeds the 32 MB limit.");
            auto input=file.createInputStream();juce::MidiFile midi;int type=-1;
            if(!input || !midi.readFrom(*input,false,&type))throw std::runtime_error("Cannot read this MIDI file.");
            if(type!=0 && type!=1)throw std::runtime_error("Song import requires a type 0 or type 1 MIDI file.");
            if(midi.getTimeFormat()<=0)throw std::runtime_error("Song import requires PPQ timing, rather than SMPTE.");
            MidiSong result;result.name=file.getFileNameWithoutExtension();
            std::map<int,double> tempos{{0,120}};
            std::map<int,std::pair<int,int>> meters{{0,{4,4}}};
            std::map<int,juce::String> markers;
            std::map<std::tuple<int,int,int>,std::vector<Event>> groups;
            std::map<int,juce::String> names;
            int end=0,eventCount=0;
            auto gridBoundary=[&](int tick)
            {
                const int rounded=int(std::llround(double(tick)/12))*12;
                if(rounded!=tick)++result.roundedBoundaries;
                return rounded;
            };
            for(int track=0;track<midi.getNumTracks();++track)
            {
                int port=int(midiDataIOPorts::usbDevMidi1);
                const auto* sequence=midi.getTrack(track);
                for(int i=0;i<sequence->getNumEvents();++i)
                {
                    const auto& message=sequence->getEventPointer(i)->message;
                    validateMidiMetadata(message);
                    const double scaled=message.getTimeStamp()*48.0/midi.getTimeFormat();
                    if(!std::isfinite(scaled) || scaled<0 || scaled>4096*192)
                        throw std::runtime_error("MIDI timing is invalid or exceeds the 4096-bar source limit.");
                    if(++eventCount>200000)throw std::runtime_error("MIDI exceeds the 200,000-event import limit.");
                    const int tick=int(std::llround(scaled));
                    const bool cleanup=message.isController() && (message.getControllerNumber()>=120 ||
                        (message.getControllerNumber()==64 && message.getControllerValue()==0));
                    end=std::max(end,tick+(message.getChannel()>0 && !message.isNoteOff() && !cleanup?1:0));
                    if(message.isTrackNameEvent())names[track]=message.getTextFromTextMetaEvent();
                    if(message.isTempoMetaEvent())
                    {
                        const double tempo=60.0/message.getTempoSecondsPerQuarterNote();
                        if(!std::isfinite(tempo) || tempo<20 || tempo>360)
                            throw std::runtime_error("A MIDI tempo is outside Ignite's 20-360 BPM range.");
                        tempos[gridBoundary(tick)]=tempo;
                    }
                    if(message.isTimeSignatureMetaEvent())
                    {
                        int numerator=0,denominator=0;message.getTimeSignatureInfo(numerator,denominator);
                        if(numerator<1 || numerator>32 || denominator<1 || denominator>32 || (denominator&(denominator-1)))
                            throw std::runtime_error("MIDI contains an unsupported time signature.");
                        meters[gridBoundary(tick)]={numerator,denominator};
                    }
                    if(message.isMetaEvent() && message.getMetaEventType()==6)
                        markers[gridBoundary(tick)]=message.getTextFromTextMetaEvent();
                    if(message.isSysEx())++result.ignoredSysEx;
                    if(message.isMetaEvent() && message.getMetaEventType()==0x21)
                    {
                        port=message.getMetaEventData()[0];
                        if(!isMidiPortId(port))throw std::runtime_error("MIDI uses a port outside Ignite's MIDI routes (0-4, 13-23). Remap the port or import clips instead.");
                    }
                    if(message.getChannel()>0 && (message.isNoteOnOrOff() || message.isController() ||
                        message.isProgramChange() || message.isPitchWheel() || message.isAftertouch() || message.isChannelPressure()))
                        groups[{track,port,message.getChannel()}].push_back({message,tick});
                }
            }
            if(groups.empty())throw std::runtime_error("MIDI contains no supported musical events.");
            if(groups.size()>64)throw std::runtime_error("MIDI has more than 64 track/port/channel parts.");
            end=std::max(12,((end+11)/12)*12);
            for(auto& group:groups)
            {
                const auto [track,port,channel]=group.first;
                Part part;part.name=(names.count(track)?names.at(track):"Track "+juce::String(track+1))+" Ch"+juce::String(channel);
                part.channel=channel;part.port=port;
                MidiNotes timeline;
                int msb=0,lsb=0;
                for(const auto& event:group.second)
                {
                    const auto& m=event.message;
                    timeline.accept(m,event.tick);
                    if(m.isNoteOnOrOff())continue;
                    if(m.isController() && (m.getControllerNumber()==0 || m.getControllerNumber()==32))
                    {
                        if(m.getControllerNumber()==0)msb=m.getControllerValue();else lsb=m.getControllerValue();
                        const int bank=channel==10 && msb==0 && lsb==0?128:msb*128+lsb;
                        part.auxiliary.push_back({juce::MidiMessage::controllerEvent(channel,0,bank>>7),event.tick});
                        part.auxiliary.push_back({juce::MidiMessage::controllerEvent(channel,32,bank&127),event.tick});
                    }
                    else part.auxiliary.push_back(event);
                }
                timeline.finish(end);result.unmatched+=timeline.unmatched;part.notes=std::move(timeline.notes);
                result.parts.push_back(std::move(part));
            }
            if(std::none_of(result.parts.begin(),result.parts.end(),[](const auto& part){return !part.notes.empty() || !part.auxiliary.empty();}))
                throw std::runtime_error("MIDI contains no supported musical events.");
            std::set<int> boundaries{0,end};
            for(const auto& t:tempos)if(t.first<end)boundaries.insert(t.first);
            for(const auto& m:meters)if(m.first<end)boundaries.insert(m.first);
            for(const auto& m:markers)if(m.first<end)boundaries.insert(m.first);
            for(auto it=boundaries.begin(),next=std::next(it);next!=boundaries.end();++it,++next)
            {
                int begin=*it;
                const auto meter=std::prev(meters.upper_bound(begin))->second;
                int meterSteps=meter.first*16/meter.second;
                if(meter.first*16%meter.second || meterSteps<1 || meterSteps>16)
                {meterSteps=16;++result.normalisedMeters;}
                const int maximum=(256/meterSteps)*meterSteps;
                while(begin<*next)
                {
                    Section section;section.begin=begin;section.end=std::min(*next,begin+maximum*12);
                    section.numerator=meter.first;section.denominator=meter.second;
                    section.barSteps=std::gcd((section.end-begin)/12,meterSteps);
                    if((section.end-begin)/(12*section.barSteps)>200)
                    {section.end=begin+((section.end-begin)/(12*meterSteps))*12*meterSteps;section.barSteps=meterSteps;}
                    section.tempo=std::prev(tempos.upper_bound(begin))->second;
                    section.name="Scene "+juce::String(int(result.sections.size())+1);
                    if(markers.count(begin))section.name+=" - "+juce::File::createLegalFileName(markers.at(begin)).substring(0,48);
                    result.sections.push_back(section);begin=section.end;
                    if(result.sections.size()>128)throw std::runtime_error("MIDI creates more than 128 scenes. Import a smaller arrangement.");
                }
            }
            if(result.sections.size()*result.parts.size()>512)
                throw std::runtime_error("MIDI creates more than 512 patterns. Import a smaller arrangement.");
            int generatedEvents=0;
            for(const auto& section:result.sections)
            {
                std::vector<ClientPatternData> clips;
                for(const auto& part:result.parts)
                {
                    auto data=makePattern(section,part,result,generatedEvents);
                    clips.push_back(std::move(data));
                }
                result.patterns.push_back(std::move(clips));
            }
            return result;
        }

        std::string apply(IgniteEngine& engine,const juce::String& namePrefix="MIDI-") const
        {
            auto lock=engine.lockDesktopState();auto& songs=*engine.songManager;
            if(songs.getSystemState()!=SYSTEM_STATE::IDLE)throw std::runtime_error("Stop playback and recording before importing a song.");
            if(engine.desktopHasUnsavedChanges())throw std::runtime_error("Save your current edits before importing a song.");
            if(songs.songInfoList.size()>=1000)throw std::runtime_error("The song library is full.");
            std::set<std::tuple<int,int,int>> assignments;
            for(size_t s=0;s<sections.size();++s)for(size_t p=0;p<parts.size();++p)
            {const auto state=programAt(parts[p],sections[s].begin);assignments.emplace(int(p),state.first,state.second);}
            if(engine.instrumentManager->instrumentList.size()+assignments.size()>255)
                throw std::runtime_error("This import exceeds the 255-instrument library limit.");
            std::string songName=(namePrefix+juce::File::createLegalFileName(name).substring(0,48)).toStdString();
            const auto base=songName;int suffix=2;
            while(std::any_of(songs.songInfoList.begin(),songs.songInfoList.end(),[&](const auto& item){return item.second==songName;}))
                songName=base+" ("+std::to_string(suffix++)+")";
            auto song=std::make_unique<CSong>(0,songName,float(sections.front().tempo),0,0,&songs);
            song->_songData->applySongTempoToScenes=false;song->_songData->applySongSwingToScenes=false;
            song->_songData->gTrackCount=uint8_t(parts.size());
            std::map<std::tuple<int,int,int>,std::string> instruments;
            for(const auto& assignment:assignments)
            {
                const auto [part,bank,program]=assignment;
                auto instrument=engine.instrumentManager->addDesktopMidiInstrument(parts[size_t(part)].name.toStdString(),parts[size_t(part)].channel,bank,program);
                engine.instrumentManager->getInstrument(instrument)->_instrumentData->midiPort=int8_t(parts[size_t(part)].port);
                instruments[assignment]=instrument;
            }
            for(size_t s=0;s<sections.size();++s)
            {
                const auto& section=sections[s];Scene scene(int(s),section.name.toStdString(),float(section.tempo),0,0,0,&songs);
                auto& d=*scene._sceneData;d.gBarLenInSteps=int8_t(section.barSteps);
                d.numBars=uint16_t((section.end-section.begin)/(12*section.barSteps));
                d.desktopMeterNumerator=uint8_t(section.numerator);d.desktopMeterDenominator=uint8_t(section.denominator);
                for(size_t p=0;p<parts.size();++p)
                {
                    auto data=patterns.at(s).at(p);data.timeBase=engine.patternManager->mapTbToUiIndex(TimeBase::Base_16);
                    const auto pattern=engine.patternManager->addDesktopPattern(std::move(data));
                    const auto state=programAt(parts[p],section.begin);
                    d.trackList.emplace_back(int8_t(p),parts[p].name.toStdString(),false,&songs);auto& track=d.trackList.back();
                    track.setTrackMainPattern(engine.patternManager->GetPattern(pattern));
                    track.setTrackTxInstrument(engine.instrumentManager->getInstrument(instruments.at({int(p),state.first,state.second})));
                }
                d.numberOfTracks=int8_t(parts.size());song->addScene(scene,false,false);
            }
            songs.installDesktopSong(std::move(song));return songName;
        }

    private:
        static std::pair<int,int> programAt(const Part& part,int tick)
        {
            int msb=part.channel==10?1:0,lsb=0,program=0,activeBank=msb*128;
            for(const auto& event:part.auxiliary)
            {
                if(event.tick>tick)break;
                const auto& m=event.message;
                if(m.isProgramChange()){program=m.getProgramChangeNumber();activeBank=msb*128+lsb;}
                if(m.isController() && m.getControllerNumber()==0)msb=m.getControllerValue();
                if(m.isController() && m.getControllerNumber()==32)lsb=m.getControllerValue();
            }
            return {activeBank,program};
        }
        static ClientPatternData makePattern(const Section& section,const Part& part,MidiSong& result,int& generated)
        {
            ClientPatternData data{};const int length=section.end-section.begin,steps=length/12;
            data.numBars=uint8_t((steps+15)/16);data.patternLastStepId=int16_t(steps);
            data.patternDispName=(part.name+" / "+section.name).toStdString();
            for(int bar=0;bar<data.numBars;++bar)
            {
                auto& b=data.BarDataList[uint8_t(bar)];b.BarId=uint16_t(bar);b.stepCount=uint16_t(std::min(16,steps-bar*16));
                for(int step=0;step<16;++step)
                {auto& s=b.StepDataList[uint16_t(step)];s={};s.BarId=uint16_t(bar);s.stepId=uint16_t(bar*16+step);s.enable=true;}
            }
            auto count=[&]{if(++generated>500000)throw std::runtime_error("MIDI creates too many scene events. Import a smaller arrangement.");};
            for(const auto& note:part.notes)if(note.start<section.end && note.soundEnd>section.begin)
            {
                const int start=std::max(note.start,section.begin)-section.begin;
                const bool pedalHeld=note.start<section.begin && note.end<=section.begin;
                const int end=pedalHeld?note.soundEnd:note.end;
                const int duration=std::min(end,section.end)-std::max(note.start,section.begin),step=start/12;
                auto& s=data.BarDataList.at(uint8_t(step/16)).StepDataList.at(uint16_t(step%16));
                ClientPatternNodeData node{};node.note=node.nodeId=uint8_t(note.pitch);node.velocity=uint8_t(note.velocity);node.onState=true;
                node.offset=TickOffset_t(start%12);node.step_id=uint16_t(step);
                node.lenInSteps=uint8_t(std::min(255,duration/12));node.lenInTicks=TickLength_t(duration-int(node.lenInSteps)*12);
                if(s.nodeList[node.note].count(node.offset))++result.duplicates;
                s.nodeList[node.note][node.offset]=node;s.nodeCount=uint8_t(s.nodeList.size());count();
                if(note.start<section.begin)++result.retriggers;
            }
            auto auxiliary=[&](const juce::MidiMessage& message,int tick)
            {
                const int step=tick/12;const auto* bytes=message.getRawData();
                data.BarDataList.at(uint8_t(step/16)).StepDataList.at(uint16_t(step%16)).auxList.push_back(
                    {true,uint16_t(bytes[0]&0xf0),int8_t(bytes[1]),int8_t(message.getRawDataSize()>2?bytes[2]:0),TickOffset_t(tick%12)});count();
            };
            std::map<int,Event> state;
            std::vector<Event> parameters;
            for(const auto& event:part.auxiliary)
            {
                if(event.tick>=section.begin)break;
                const auto& m=event.message;
                if(m.isController() && m.getControllerNumber()==121)
                {
                    // Reset Controllers clears the selector/data latch, while
                    // registered parameter values such as bend range persist.
                    if(!parameters.empty())parameters.push_back(event);
                    for(auto it=state.begin();it!=state.end();)
                    {
                        const int cc=it->first;
                        if(cc==0 || cc==32 || cc==7 || cc==39 || cc==10 || cc==42)++it;
                        else it=state.erase(it);
                    }
                }
                if(m.isController() && m.getControllerNumber()<120)
                {
                    const int cc=m.getControllerNumber();
                    if(cc==6 || cc==38 || (cc>=96 && cc<=101))parameters.push_back(event);
                    else state.insert_or_assign(cc,event);
                }
                else if(m.isPitchWheel())state.insert_or_assign(128,event);
                else if(m.isChannelPressure())state.insert_or_assign(129,event);
            }
            std::vector<Event> restore;for(const auto& item:state)restore.push_back(item.second);
            restore.insert(restore.end(),parameters.begin(),parameters.end());
            std::stable_sort(restore.begin(),restore.end(),[](const auto& a,const auto& b){return a.tick<b.tick;});
            for(const auto& event:restore)auxiliary(event.message,0);
            for(const auto& event:part.auxiliary)if(event.tick>=section.begin && event.tick<section.end)
                auxiliary(event.message,event.tick-section.begin);
            return data;
        }
    };
}
