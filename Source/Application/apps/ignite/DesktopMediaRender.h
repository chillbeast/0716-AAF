// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopMediaSnapshot.h"

namespace IgniteDesktop::Media
{
    inline int renderJob(Ignite::IgniteEngine& engine,const juce::File& file)
    {
        try
        {
            auto job=juce::XmlDocument::parse(file);if(!job)throw std::runtime_error("Cannot read export snapshot");
            auto& songs=*engine.songManager;const bool midi=job->getStringAttribute("kind")=="midi";songs.desktopOfflineAudio=!midi;
            load(engine,*job);
            const juce::File output(job->getStringAttribute("output"));
            juce::MidiFile midiFile;midiFile.setTicksPerQuarterNote(960);juce::MidiMessageSequence conductor;
            conductor.addEvent(juce::MidiMessage::textMetaEvent(3,job->getStringAttribute("name")),0);
            std::map<int,juce::MidiMessageSequence> tracks;double tick=0;int64_t totalFrames=0;double quarter=0,peak=0,totalSeconds=0;
            songs.desktopOfflineMidi=[&](const juce::MidiMessage& message,Track* track)
            {
                if(!midi || !track || track->trackMuted() || !track->getTrackMainPattern() || !track->getTrackTxInstrument())return;
                auto& sequence=tracks[track->getTrackId()];auto copy=message;copy.setTimeStamp(tick);sequence.addEvent(copy);
            };
            std::unique_ptr<juce::AudioFormatWriter> writer;
            if(!midi)
            {
                auto stream=output.createOutputStream();if(!stream)throw std::runtime_error("Cannot open WAV output");
                juce::WavAudioFormat format;writer.reset(format.createWriterFor(stream.get(),48000,2,32,{},0));
                if(!writer)throw std::runtime_error("Cannot create stereo WAV writer");stream.release();
            }
            const juce::File progress=file.getSiblingFile("progress.txt");
            auto writeAudio=[&](int64_t frames)
            {
                while(frames>0)
                {
                    const int count=int(std::min<int64_t>(960,frames));float buffer[1920]{};songs.renderDesktopStereo(buffer,count);
                    for(int i=0;i<count*2;++i){if(!std::isfinite(buffer[i]))throw std::runtime_error("The synth produced invalid samples");peak=std::max(peak,double(std::abs(buffer[i])));}
                    const float* channels[]{buffer,buffer+count};if(!writer->writeFromFloatArrays(channels,2,count))throw std::runtime_error("Writing the WAV failed");
                    frames-=count;totalFrames+=count;
                }
            };
            int sceneIndex=0;int64_t pulseCount=0;
            for(auto& scene:songs.activeSong->_songData->sceneList)
            {
                songs.initPlayScene(sceneIndex);songs._state=SYSTEM_STATE::PLAY;songs._isSongPlaying=false;
                for(auto& track:scene._sceneData->trackList)
                {
                    if(!midi && !track.trackMuted() && track.getTrackMainPattern() && track.getTrackTxInstrument() && track.getTrackTxInstrument()->getInternalSound())
                    {
                        if(!track._trackSf || !track.desktopPresetValid)throw std::runtime_error("A required SoundFont is missing. Repair the instrument assignment before exporting.");
                        track.restoreDesktopEffects();
                    }
                    if(midi && !track.trackMuted() && track.getTrackMainPattern() && track.getTrackTxInstrument())
                    {
                        auto name=juce::MidiMessage::textMetaEvent(3,juce::String(track.getTrackName()));name.setTimeStamp(quarter*960);tracks[track.getTrackId()].addEvent(name);
                        if(auto* instrument=track.getTrackTxInstrument();instrument && (instrument->getInternalSound() || instrument->_instrumentData->preSendPgmChange))
                        {
                            const int encoded=instrument->getInternalSoundPgm();
                            const int bank=instrument->getInternalSound()?(SoundFontProgram::isCustom(encoded)?SoundFontProgram::bank(encoded):(encoded>127?128:0)):((int(instrument->_instrumentData->pgmChangeData.bankMSB)<<7)|int(instrument->_instrumentData->pgmChangeData.bankLSB));
                            const int program=instrument->getInternalSound()?(SoundFontProgram::isCustom(encoded)?SoundFontProgram::program(encoded):(encoded>127?Track::getTsfPresetFromIgniteIndex(encoded):encoded)):instrument->_instrumentData->pgmChangeData.midiPgmNum;
                            if(bank<0 || bank>16383)throw std::runtime_error("An SF2 bank number exceeds the MIDI bank range. Use WAV export or select a MIDI-compatible bank.");
                            const int channel=std::clamp(int(instrument->getMidiChannel())+1,1,16);
                            auto add=[&](juce::MidiMessage message){message.setTimeStamp(quarter*960);tracks[track.getTrackId()].addEvent(message);};
                            const uint8_t port[]{0xff,0x21,0x01,uint8_t(std::clamp(int(instrument->getMidiPort()),0,127))};add(juce::MidiMessage(port,4));
                            add(juce::MidiMessage::controllerEvent(channel,0,std::clamp(bank>>7,0,127)));add(juce::MidiMessage::controllerEvent(channel,32,bank&127));add(juce::MidiMessage::programChange(channel,std::clamp(program,0,127)));
                        }
                    }
                }
                songs.setMasterVolume(float(job->getDoubleAttribute("master",0.7)));
                const double tempo=scene.getTempo();const auto pulses=int64_t(scene.getBarCount())*scene.getStepsPerBar()*songs._scenePulsesPerStep;
                if(pulses<1)throw std::runtime_error("A scene has no playable duration");
                const double seconds=double(pulses)*60.0/(tempo*SET_PPQN);
                totalSeconds+=seconds;
                if(totalSeconds>1800)throw std::runtime_error("Export range exceeds the 30-minute limit");
                auto tempoEvent=juce::MidiMessage::tempoMetaEvent(int(std::llround(60000000.0/tempo)));tempoEvent.setTimeStamp(quarter*960);conductor.addEvent(tempoEvent);
                auto marker=juce::MidiMessage::textMetaEvent(6,juce::String(scene.getName()));marker.setTimeStamp(quarter*960);conductor.addEvent(marker);
                int64_t sceneFrames=0;
                for(int64_t pulse=0;pulse<pulses;++pulse)
                {
                    tick=(quarter+double(pulse)/SET_PPQN)*960;
                    songs.pulse(uint16_t(pulse%songs._scenePulsesPerStep));
                    if(!midi)
                    {
                        const auto next=int64_t(std::llround(double(pulse+1)*48000.0*60.0/(tempo*SET_PPQN)));
                        writeAudio(next-sceneFrames);sceneFrames=next;
                    }
                    if((++pulseCount%192)==0)progress.replaceWithText("Scene "+juce::String(sceneIndex+1)+" / "+juce::String(songs.activeSong->_songData->numberOfScenes));
                }
                quarter+=double(pulses)/SET_PPQN;tick=quarter*960;
                for(auto& track:scene._sceneData->trackList)
                {
                    if(track.trackMuted() || !track.getTrackMainPattern() || !track.getTrackTxInstrument())continue;
                    songs.sendMidiMessage(juce::MidiMessage::controllerEvent(std::max(1,int(track.getTxInstrumentChannel())+1),64,0),0,track.getTrackTxInstrument() && track.getTrackTxInstrument()->getInternalSound(),&track);
                    songs.sendMidiMessage(juce::MidiMessage::allNotesOff(std::max(1,int(track.getTxInstrumentChannel())+1)),0,track.getTrackTxInstrument() && track.getTrackTxInstrument()->getInternalSound(),&track);
                }
                ++sceneIndex;
            }
            if(midi)
            {
                conductor.addEvent(juce::MidiMessage::endOfTrack(),tick);midiFile.addTrack(conductor);
                for(auto& item:tracks){item.second.addEvent(juce::MidiMessage::endOfTrack(),tick);midiFile.addTrack(item.second);}
                auto stream=output.createOutputStream();if(!stream || !midiFile.writeTo(*stream,1))throw std::runtime_error("Writing the MIDI file failed");stream->flush();if(stream->getStatus().failed())throw std::runtime_error("Flushing the MIDI file failed");
            }
            else
            {
                songs._state=SYSTEM_STATE::IDLE;
                writeAudio(int64_t(std::llround(std::clamp(job->getDoubleAttribute("tail",2),0.0,10.0)*48000)));
                if(!writer->flush())throw std::runtime_error("Flushing the WAV failed");writer.reset();
                juce::WavAudioFormat checkFormat;std::unique_ptr<juce::AudioFormatReader> check(checkFormat.createReaderFor(output.createInputStream().release(),true));
                if(!check || check->lengthInSamples!=totalFrames || check->numChannels!=2)throw std::runtime_error("The completed WAV failed validation");
            }
            songs.desktopOfflineMidi={};
            auto result=std::make_unique<juce::XmlElement>("MediaResult");result->setAttribute("frames",juce::String(totalFrames));result->setAttribute("peak",peak);result->setAttribute("quarters",quarter);result->setAttribute("success",true);
            if(!result->writeTo(file.getSiblingFile("result.xml")))throw std::runtime_error("Cannot write export result");
            juce::Logger::writeToLog("Media export complete: "+output.getFullPathName());return 0;
        }
        catch(const std::exception& error)
        {
            juce::XmlElement result("MediaResult");result.setAttribute("success",false);result.setAttribute("error",juce::String(error.what()));result.writeTo(file.getSiblingFile("result.xml"));juce::Logger::writeToLog("Media export failed: "+juce::String(error.what()));return 1;
        }
    }
}
