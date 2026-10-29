// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "Firmware/Application/Model/IgniteEngine.h"
#include <stdexcept>
#include <set>
#include "Firmware/Application/Model/InstrumentManager.h"
#include "Firmware/Application/Model/PatternManager.h"
#include "Firmware/Application/Model/SongManager.h"
#include "Firmware/Application/Model/SequenceManager.h"

namespace IgniteDesktop::Media
{
    using namespace Ignite;
    inline juce::XmlElement& child(juce::XmlElement& parent,const char* tag){return *parent.createNewChildElement(tag);}
    inline const juce::XmlElement& required(const juce::XmlElement& parent,const char* tag)
    {auto* item=parent.getChildByName(tag);if(!item)throw std::runtime_error("Incomplete export snapshot");return *item;}
    inline void writeTV(juce::XmlElement& parent,const std::unordered_map<int8_t,TrackValue>& values)
    {
        for(const auto& item:values)
        {
            const auto& value=item.second;auto& xml=child(parent,"TV");
            xml.setAttribute("knob",item.first);xml.setAttribute("name",value.name);xml.setAttribute("type",int(value.paramType));xml.setAttribute("scope",int(value.savedAt));
            if(value.paramType==TrackValueType::TVTypeMidiCC)
            {const auto& cc=value.param.tvMidiCC;xml.setAttribute("cc",cc.num);xml.setAttribute("value",cc.value);xml.setAttribute("init",cc.initValue);xml.setAttribute("min",cc.minValue);xml.setAttribute("max",cc.maxValue);}
            else if(value.paramType==TrackValueType::TVTypePgmChange)
            {xml.setAttribute("lsb",value.param.tvPgmChange.bankLSB);xml.setAttribute("msb",value.param.tvPgmChange.bankMSB);xml.setAttribute("program",value.param.tvPgmChange.midiPgmNum);}
        }
    }
    inline std::unordered_map<int8_t,TrackValue> readTV(const juce::XmlElement& parent)
    {
        std::unordered_map<int8_t,TrackValue> result;
        for(const auto* xml:parent.getChildIterator())if(xml->hasTagName("TV"))
        {
            TrackValue value{};value.knobId=int8_t(xml->getIntAttribute("knob"));value.name=xml->getStringAttribute("name").toStdString();value.paramType=TrackValueType(xml->getIntAttribute("type"));value.savedAt=TrackValueScope(xml->getIntAttribute("scope"));
            if(value.paramType==TrackValueType::TVTypeMidiCC)value.param.tvMidiCC={int8_t(xml->getIntAttribute("cc")),int8_t(xml->getIntAttribute("init")),int8_t(xml->getIntAttribute("max")),int8_t(xml->getIntAttribute("min")),float(xml->getDoubleAttribute("value"))};
            else value.param.tvPgmChange={int8_t(xml->getIntAttribute("lsb")),int8_t(xml->getIntAttribute("msb")),int8_t(xml->getIntAttribute("program"))};
            result[value.knobId]=value;
        }
        return result;
    }
    inline void writePattern(juce::XmlElement& xml,const ClientPatternData& data)
    {
        xml.setAttribute("key",data.patternName);xml.setAttribute("name",data.patternDispName);xml.setAttribute("bars",data.numBars);xml.setAttribute("last",data.patternLastStepId);xml.setAttribute("base",data.timeBase);
        xml.setAttribute("loop",data.barLoopOn);xml.setAttribute("loopStart",data.barLoopStartId);xml.setAttribute("loopEnd",data.barLoopEndId);
        for(int bar=0;bar<data.numBars;++bar)
        {
            const auto& source=data.BarDataList.at(uint8_t(bar));auto& target=child(xml,"Bar");target.setAttribute("id",bar);target.setAttribute("count",source.stepCount);target.setAttribute("repeat",source.barRepeatCount);target.setAttribute("defeat",source.barTransposeDefeat);target.setAttribute("sync",source.gBarSyncOn);target.setAttribute("direction",source.direction);
            for(int step=0;step<16;++step)
            {
                const auto& value=source.StepDataList.at(uint16_t(step));auto& item=child(target,"Step");item.setAttribute("id",step);item.setAttribute("on",value.enable);item.setAttribute("skip",value.skipStatus);item.setAttribute("tie",value.tieStatus);item.setAttribute("defeat",value.transposeDefeatStatus);
                for(const auto& pitch:value.nodeList)for(const auto& offset:pitch.second)
                {const auto& node=offset.second;auto& n=child(item,"N");n.setAttribute("pitch",node.note);n.setAttribute("velocity",node.velocity);n.setAttribute("steps",node.lenInSteps);n.setAttribute("ticks",node.lenInTicks);n.setAttribute("offset",node.offset);n.setAttribute("on",node.onState);}
                for(const auto& aux:value.auxList)
                {auto& a=child(item,"A");a.setAttribute("type",aux.type);a.setAttribute("a",aux.data1);a.setAttribute("b",aux.data2);a.setAttribute("offset",aux.offset);a.setAttribute("on",aux.enabled);}
            }
        }
    }
    inline ClientPatternData readPattern(const juce::XmlElement& xml)
    {
        ClientPatternData data{};data.numBars=uint8_t(xml.getIntAttribute("bars"));data.patternLastStepId=int16_t(xml.getIntAttribute("last"));data.timeBase=uint8_t(xml.getIntAttribute("base"));data.patternName=xml.getStringAttribute("key").toStdString();data.patternDispName=xml.getStringAttribute("name").toStdString();data.barLoopOn=xml.getBoolAttribute("loop");data.barLoopStartId=int8_t(xml.getIntAttribute("loopStart"));data.barLoopEndId=int8_t(xml.getIntAttribute("loopEnd"));
        if(data.numBars<1 || data.numBars>16 || data.patternLastStepId<1 || data.patternLastStepId>data.numBars*16)throw std::runtime_error("Invalid pattern size");
        for(const auto* bar:xml.getChildIterator())if(bar->hasTagName("Bar"))
        {
            const auto id=bar->getIntAttribute("id");if(id<0 || id>=data.numBars)throw std::runtime_error("Invalid pattern bar");
            auto& target=data.BarDataList[uint8_t(id)];target.BarId=uint16_t(id);target.stepCount=uint16_t(bar->getIntAttribute("count"));target.barRepeatCount=uint8_t(bar->getIntAttribute("repeat"));target.barTransposeDefeat=bar->getBoolAttribute("defeat");target.gBarSyncOn=bar->getBoolAttribute("sync");target.direction=uint8_t(bar->getIntAttribute("direction"));
            for(const auto* step:bar->getChildIterator())if(step->hasTagName("Step"))
            {
                const auto index=step->getIntAttribute("id");if(index<0 || index>=16)throw std::runtime_error("Invalid pattern step");
                auto& value=target.StepDataList[uint16_t(index)];value={};value.BarId=uint16_t(id);value.stepId=uint16_t(id*16+index);value.enable=step->getBoolAttribute("on");value.skipStatus=step->getBoolAttribute("skip");value.tieStatus=step->getBoolAttribute("tie");value.transposeDefeatStatus=step->getBoolAttribute("defeat");
                for(const auto* event:step->getChildIterator())
                {
                    if(event->hasTagName("N"))
                    {ClientPatternNodeData n{};n.note=uint8_t(event->getIntAttribute("pitch"));n.nodeId=n.note;n.velocity=uint8_t(event->getIntAttribute("velocity"));n.lenInSteps=uint8_t(event->getIntAttribute("steps"));n.lenInTicks=TickLength_t(event->getIntAttribute("ticks"));n.offset=TickOffset_t(event->getIntAttribute("offset"));n.onState=event->getBoolAttribute("on");n.step_id=value.stepId;value.nodeList[n.note][n.offset]=n;}
                    else if(event->hasTagName("A"))value.auxList.push_back({event->getBoolAttribute("on"),uint16_t(event->getIntAttribute("type")),int8_t(event->getIntAttribute("a")),int8_t(event->getIntAttribute("b")),TickOffset_t(event->getIntAttribute("offset"))});
                }
                value.nodeCount=uint8_t(value.nodeList.size());
            }
            if(target.StepDataList.size()!=16)throw std::runtime_error("Incomplete pattern bar");
        }
        if(data.BarDataList.size()!=data.numBars)throw std::runtime_error("Incomplete pattern");return data;
    }
    inline std::unique_ptr<juce::XmlElement> capture(IgniteEngine& engine,bool sceneOnly,int stem,const juce::String& kind)
    {
        auto lock=engine.lockDesktopState();auto& songs=*engine.songManager;
        if(!songs.activeSong || !songs.activeSong->editScene)throw std::runtime_error("Select a song and scene first.");
        if(songs.getSystemState()!=SYSTEM_STATE::IDLE)throw std::runtime_error("Stop playback and recording before exporting.");
        auto job=std::make_unique<juce::XmlElement>("IgniteMediaJob");job->setAttribute("version",1);job->setAttribute("kind",kind);job->setAttribute("name",songs.activeSong->getName());job->setAttribute("master",songs.getMasterVolume());job->setAttribute("voices",songs.desktopVoiceLimit);job->setAttribute("font",songs.desktopSoundFontPath);job->setAttribute("tail",2.0);
        auto& scales=child(*job,"Scales");for(const auto& entry:songs.desktopScaleIntervals()){auto& scale=child(scales,"Scale");scale.setAttribute("id",entry.first);for(int interval:entry.second)child(scale,"I").setAttribute("n",interval);}
        std::set<std::string> usedInstruments,usedPatterns;
        for(auto& scene:songs.activeSong->_songData->sceneList)if(!sceneOnly || &scene==songs.activeSong->editScene)
            for(auto& track:scene._sceneData->trackList){usedInstruments.insert(track._trackData->trackTxInstrumentName);usedPatterns.insert(track._trackData->mainPatternName);}
        auto& instruments=child(*job,"Instruments");
        for(const auto& instrument:engine.instrumentManager->instrumentList)
        {
            if(!usedInstruments.count(instrument.getName()))continue;
            const auto& d=*instrument._instrumentData;auto& i=child(instruments,"Instrument");i.setAttribute("key",d.instrumentName);i.setAttribute("name",d.instrumentDispName);i.setAttribute("port",d.midiPort);i.setAttribute("channel",d.midiChannel);i.setAttribute("program",d.internalSoundPgm);i.setAttribute("internal",d.internalSound);i.setAttribute("font",d.soundFontPath);i.setAttribute("transposeDefeat",d.transposeDefeatStatus);i.setAttribute("scaleDefeat",d.ftsDefeatStatus);i.setAttribute("preProgram",d.preSendPgmChange);i.setAttribute("lsb",d.pgmChangeData.bankLSB);i.setAttribute("msb",d.pgmChangeData.bankMSB);i.setAttribute("midiProgram",d.pgmChangeData.midiPgmNum);i.setAttribute("volume",d.tm.volume);i.setAttribute("pan",d.tm.pan);writeTV(i,d.tv);
        }
        auto& patterns=child(*job,"Patterns");for(auto& pattern:engine.patternManager->patternList)if(usedPatterns.count(pattern.getName()))writePattern(child(patterns,"Pattern"),engine.EngineGetPatternData(pattern.getName()));
        bool playable=false;
        auto& scenes=child(*job,"Scenes");
        for(auto& source:songs.activeSong->_songData->sceneList)
        {
            if(sceneOnly && &source!=songs.activeSong->editScene)continue;
            auto& s=child(scenes,"Scene");const auto& d=*source._sceneData;s.setAttribute("name",d.sceneName);s.setAttribute("tempo",d.sceneTempo);s.setAttribute("swing",d.sceneSwing);s.setAttribute("bars",d.numBars);s.setAttribute("steps",d.gBarLenInSteps);s.setAttribute("scale",d.scaleFTS);s.setAttribute("root",d.noteFTS);s.setAttribute("transpose",d.transposeValue);s.setAttribute("priority",d.transposeAfterFTS);
            for(auto& track:source._sceneData->trackList)
            {
                const auto& t=*track._trackData;auto& item=child(s,"Track");item.setAttribute("id",t.trackId);item.setAttribute("name",t.trackName);item.setAttribute("instrument",t.trackTxInstrumentName);item.setAttribute("pattern",t.mainPatternName);
                item.setAttribute("mute",t.trackMute || (stem>=0 && t.trackId!=stem) || (track.getSystemSoloMode() && !t.trackSolo));writeTV(item,t.tvTrackData);
                for(int key=0;key<16;++key){auto& fx=child(item,"FX");fx.setAttribute("key",key);fx.setAttribute("value",track.desktopEffectValues()[size_t(key)]);}
                if(!item.getBoolAttribute("mute") && t.mainPattern && t.trackTxInstrument && (kind=="midi" || t.trackTxInstrument->getInternalSound()))playable=true;
            }
        }
        if(!playable)throw std::runtime_error("The selected range has no enabled, assigned tracks for this export.");
        return job;
    }
    inline void load(IgniteEngine& engine,const juce::XmlElement& job)
    {
        if(!job.hasTagName("IgniteMediaJob") || job.getIntAttribute("version")!=1)throw std::runtime_error("Unsupported export snapshot");
        auto lock=engine.lockDesktopState();auto& songs=*engine.songManager;songs.desktopOffline=true;songs.desktopMonitorEnabled=false;songs.setDesktopVoiceLimit(job.getIntAttribute("voices",32));
        std::map<int,std::vector<int>> scales;for(const auto* scale:required(job,"Scales").getChildIterator())for(const auto* interval:scale->getChildIterator())scales[scale->getIntAttribute("id")].push_back(interval->getIntAttribute("n"));songs.setDesktopScaleIntervals(scales);
        std::map<std::string,std::string> instruments,patterns;
        for(const auto* source:required(job,"Instruments").getChildIterator())
        {
            const auto name=engine.instrumentManager->addInstrument();auto* instrument=engine.instrumentManager->getInstrument(name);auto& d=*instrument->_instrumentData;
            d.instrumentDispName=source->getStringAttribute("name").toStdString();d.midiPort=int8_t(source->getIntAttribute("port"));d.midiChannel=int8_t(std::clamp(source->getIntAttribute("channel"),0,15));d.internalSound=source->getBoolAttribute("internal");d.internalSoundPgm=source->getIntAttribute("program");d.soundFontPath=source->getStringAttribute("font").toStdString();d.transposeDefeatStatus=source->getBoolAttribute("transposeDefeat");d.ftsDefeatStatus=source->getBoolAttribute("scaleDefeat");d.preSendPgmChange=source->getBoolAttribute("preProgram");d.pgmChangeData={int8_t(source->getIntAttribute("lsb")),int8_t(source->getIntAttribute("msb")),int8_t(source->getIntAttribute("midiProgram"))};d.tm={int8_t(source->getIntAttribute("volume")),int8_t(source->getIntAttribute("pan"))};d.tv=readTV(*source);d.tvCount=uint8_t(d.tv.size());instruments[source->getStringAttribute("key").toStdString()]=name;
        }
        for(const auto* source:required(job,"Patterns").getChildIterator())patterns[source->getStringAttribute("key").toStdString()]=engine.patternManager->addDesktopPattern(readPattern(*source));
        songs.activeSong=std::make_unique<CSong>(0,job.getStringAttribute("name").toStdString(),120,0,0,&songs);auto& song=*songs.activeSong->_songData;song.applySongTempoToScenes=false;song.applySongSwingToScenes=false;
        int sceneIndex=0;
        for(const auto* source:required(job,"Scenes").getChildIterator())
        {
            Scene scene(sceneIndex++,source->getStringAttribute("name").toStdString(),float(source->getDoubleAttribute("tempo")),int8_t(source->getIntAttribute("swing")),0,0,&songs);auto& d=*scene._sceneData;
            d.numBars=uint16_t(source->getIntAttribute("bars"));d.gBarLenInSteps=int8_t(source->getIntAttribute("steps"));d.scaleFTS=uint8_t(source->getIntAttribute("scale"));d.noteFTS=uint8_t(source->getIntAttribute("root"));d.transposeValue=int8_t(source->getIntAttribute("transpose"));d.transposeAfterFTS=source->getBoolAttribute("priority");
            if(d.numBars<1 || d.numBars>200 || d.gBarLenInSteps<1 || d.gBarLenInSteps>16 || !std::isfinite(d.sceneTempo) || d.sceneTempo<20 || d.sceneTempo>360)throw std::runtime_error("Invalid scene duration or tempo");
            for(const auto* item:source->getChildIterator())if(item->hasTagName("Track"))
            {
                d.trackList.emplace_back(int8_t(item->getIntAttribute("id")),item->getStringAttribute("name").toStdString(),item->getBoolAttribute("mute"),&songs);auto& track=d.trackList.back();
                auto instrument=instruments.find(item->getStringAttribute("instrument").toStdString());if(instrument!=instruments.end())track.setTrackTxInstrument(engine.instrumentManager->getInstrument(instrument->second));
                auto pattern=patterns.find(item->getStringAttribute("pattern").toStdString());if(pattern!=patterns.end())track.setTrackMainPattern(engine.patternManager->GetPattern(pattern->second));
                for(const auto* fx:item->getChildIterator())if(fx->hasTagName("FX")){const int key=fx->getIntAttribute("key");if(key>=0 && key<16)track.desktopEffectValues()[size_t(key)]=float(fx->getDoubleAttribute("value"));}
                track._trackData->tvTrackData=readTV(*item);track._trackData->tvTrackDataCount=uint8_t(track._trackData->tvTrackData.size());
            }
            d.numberOfTracks=int8_t(d.trackList.size());song.gTrackCount=uint8_t(std::max(int(song.gTrackCount),int(d.numberOfTracks)));song.sceneList.push_back(scene);
        }
        song.numberOfScenes=sceneIndex;if(!sceneIndex)throw std::runtime_error("The selected range contains no scenes.");songs.activeSong->setEditScene(0);songs.activeSong->editScene->setEditTrack(0);engine.sequenceManager->initSequence();songs.setMasterVolume(float(job.getDoubleAttribute("master",0.7)));
    }
}
