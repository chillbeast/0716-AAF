// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopMediaSnapshot.h"
#include "DesktopMidiPorts.h"
#include <deque>

namespace IgniteDesktop::Media
{
    inline void validateMidiMetadata(const juce::MidiMessage& message)
    {
        if(!message.isMetaEvent())return;
        const int type=message.getMetaEventType(),length=message.getMetaEventLength();
        if(type==0x51 && length!=3)throw std::runtime_error("MIDI contains an invalid tempo event.");
        if(type==0x51 && message.getTempoSecondsPerQuarterNote()<=0)throw std::runtime_error("MIDI contains a zero tempo.");
        if(type==0x21 && length!=1)throw std::runtime_error("MIDI contains an invalid port event.");
        if(type==0x58 && (length!=4 || message.getMetaEventData()[0]==0 || message.getMetaEventData()[1]>5))
            throw std::runtime_error("MIDI contains an invalid or unsupported time signature.");
    }
    struct MidiImport
    {
        std::vector<ClientPatternData> patterns;
        double tempo=120;
        int duplicates=0,unmatched=0;
        static MidiImport parse(const juce::File& file)
        {
            if(file.getSize()>32*1024*1024)throw std::runtime_error("MIDI file exceeds the 32 MB limit.");
            auto stream=file.createInputStream();juce::MidiFile midi;
            if(!stream || !midi.readFrom(*stream,false))throw std::runtime_error("Cannot read this MIDI file.");
            if(midi.getTimeFormat()<=0)throw std::runtime_error("Import requires MIDI with PPQ timing, rather than SMPTE timing.");
            MidiImport result;constexpr int chunkTicks=16*16*12;
            struct Note{int pitch,velocity,start,end;};
            struct Event{juce::MidiMessage message;int tick;};
            std::map<std::pair<int,int>,std::vector<Event>> groups;int messages=0;
            for(int track=0;track<midi.getNumTracks();++track)
            {
                const auto* sequence=midi.getTrack(track);
                for(int index=0;index<sequence->getNumEvents();++index)
                {
                    const auto& message=sequence->getEventPointer(index)->message;
                    validateMidiMetadata(message);
                    if(!std::isfinite(message.getTimeStamp()) || message.getTimeStamp()<0)throw std::runtime_error("MIDI contains invalid event timing.");
                    if(message.isTempoMetaEvent() && message.getTimeStamp()==0)result.tempo=60.0/message.getTempoSecondsPerQuarterNote();
                    if(message.getChannel()<1)continue;
                    if(++messages>200000)throw std::runtime_error("MIDI exceeds the 200,000-event import limit.");
                    const auto tick=int64_t(std::llround(message.getTimeStamp()*48.0/midi.getTimeFormat()));
                    if(tick>chunkTicks*256)throw std::runtime_error("MIDI clip exceeds the 4096-bar import limit.");
                    if(message.isNoteOnOrOff() || message.isController() || message.isProgramChange() || message.isPitchWheel() || message.isAftertouch() || message.isChannelPressure())groups[{track,message.getChannel()}].push_back({message,int(tick)});
                }
            }
            if(groups.size()>64)throw std::runtime_error("MIDI has more than 64 track/channel groups.");
            for(auto& group:groups)
            {
                std::map<int,std::deque<std::pair<int,int>>> held;std::vector<Note> notes;std::vector<Event> aux;int end=0;
                for(const auto& event:group.second)
                {
                    end=std::max(end,event.tick+(event.message.isNoteOff()?0:1));
                    if(event.message.isNoteOn())held[event.message.getNoteNumber()].push_back({event.tick,event.message.getVelocity()});
                    else if(event.message.isNoteOff())
                    {
                        auto& list=held[event.message.getNoteNumber()];if(list.empty()){++result.unmatched;continue;}
                        const auto start=list.front();list.pop_front();notes.push_back({event.message.getNoteNumber(),start.second,start.first,std::max(start.first+1,event.tick)});
                    }
                    else aux.push_back(event);
                }
                for(const auto& pitch:held)for(const auto& start:pitch.second){notes.push_back({pitch.first,start.second,start.first,std::max(start.first+1,end)});++result.unmatched;}
                if(notes.empty() && aux.empty())continue;
                int chunks=std::max(1,(end+chunkTicks-1)/chunkTicks);
                for(int part=0;part<chunks;++part)
                {
                    const int begin=part*chunkTicks,limit=std::min(begin+chunkTicks,std::max(begin+1,end));
                    ClientPatternData data{};data.numBars=uint8_t(std::max(1,(limit-begin+191)/192));data.patternLastStepId=int16_t(data.numBars*16);data.timeBase=6;
                    data.patternDispName=(file.getFileNameWithoutExtension()+" T"+juce::String(group.first.first+1)+" Ch"+juce::String(group.first.second)+" Part"+juce::String(part+1)).toStdString();
                    // UI time-base index 6 is overwritten by the caller using the model mapping.
                    for(int bar=0;bar<data.numBars;++bar)
                    {auto& b=data.BarDataList[uint8_t(bar)];b.BarId=uint16_t(bar);b.stepCount=16;for(int step=0;step<16;++step){auto& s=b.StepDataList[uint16_t(step)];s={};s.BarId=uint16_t(bar);s.stepId=uint16_t(bar*16+step);s.enable=true;}}
                    bool populated=false;
                    for(const auto& note:notes)if(note.start<limit && note.end>begin)
                    {
                        const int start=std::max(note.start,begin)-begin,duration=std::min(note.end,limit)-std::max(note.start,begin),step=start/12;
                        auto& s=data.BarDataList.at(uint8_t(step/16)).StepDataList.at(uint16_t(step%16));
                        ClientPatternNodeData node{};node.note=uint8_t(note.pitch);node.nodeId=node.note;node.velocity=uint8_t(note.velocity);node.offset=TickOffset_t(start%12);node.lenInSteps=uint8_t(std::min(255,duration/12));node.lenInTicks=TickLength_t(duration-int(node.lenInSteps)*12);node.step_id=uint16_t(step);node.onState=true;
                        if(s.nodeList[node.note].count(node.offset))++result.duplicates;
                        s.nodeList[node.note][node.offset]=node;s.nodeCount=uint8_t(s.nodeList.size());populated=true;
                    }
                    for(const auto& event:aux)if(event.tick>=begin && event.tick<limit)
                    {
                        const int time=event.tick-begin,step=time/12;const auto* bytes=event.message.getRawData();
                        auto& s=data.BarDataList.at(uint8_t(step/16)).StepDataList.at(uint16_t(step%16));
                        s.auxList.push_back({true,uint16_t(bytes[0]&0xf0),int8_t(bytes[1]),int8_t(event.message.getRawDataSize()>2?bytes[2]:0),TickOffset_t(time%12)});populated=true;
                    }
                    if(populated)result.patterns.push_back(std::move(data));
                    if(result.patterns.size()>512)throw std::runtime_error("MIDI creates more than 512 patterns. Import a smaller selection.");
                }
            }
            if(result.patterns.empty())throw std::runtime_error("MIDI contains no supported notes or controller events.");return result;
        }
        std::vector<std::string> apply(IgniteEngine& engine) const
        {
            auto lock=engine.lockDesktopState();std::vector<std::string> names;
            for(auto data:patterns){data.timeBase=engine.patternManager->mapTbToUiIndex(TimeBase::Base_16);names.push_back(engine.patternManager->addDesktopPattern(std::move(data)));}
            return names;
        }
    };
}
