// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopDemo.h"
#include "DesktopMediaJob.h"

class DesktopDemoTest
{
public:
    DesktopDemoTest(Ignite::IgniteEngine& model,std::function<void(int)> finished):engine(model),done(std::move(finished)){}
    void start(bool verify,bool unavailable)
    {
        using namespace Ignite;root=SongManager::desktopSettingsFile().getParentDirectory();
        try
        {
            auto& songs=*engine.songManager;
            if(unavailable)
            {
                const auto count=songs.songInfoList.size();bool refused=false;
                try{IgniteDesktop::Demo::create(engine);}catch(const std::exception& error){refused=juce::String(error.what()).contains("General MIDI SF2");}
                check(refused && songs.songInfoList.size()==count && engine.patternManager->patternList.empty(),"missing/incomplete GM bank gives setup instructions without adding a partial demo");
                finish();return;
            }
            if(verify)
            {
                const auto manifest=juce::JSON::parse(root.getChildFile("demo-test.json"));
                const auto name=manifest["name"].toString().toStdString();
                for(const auto& entry:songs.songInfoList)if(entry.second==name)songs.setActiveSong(entry.first);
                check(songs.activeSong && songs.activeSong->getName()==name,"saved demo can be selected in a fresh process");
                inspect();
                check(!engine.desktopHasUnsavedChanges(),"demo reopens without pending edits");
                songs.setActiveSong(0);check(songs.activeSong && songs.activeSong->getName()!=name,"existing song remains selectable after creating the demo");
                finish();return;
            }
            check(engine.SaveAllData(),"save the existing song before creating a demo");
            const auto count=songs.songInfoList.size();engine.desktopEdited("song");bool refused=false;
            try{IgniteDesktop::Demo::create(engine);}catch(...){refused=true;}
            check(refused && songs.songInfoList.size()==count,"demo creation protects unsaved work");
            engine.SaveAllData();
            const auto alternate=juce::SystemStats::getEnvironmentVariable("IGNITE_DEMO_ALTERNATE","");std::string error;
            check(songs.browseDesktopSoundFont(alternate.toStdString(),error),"browse an unrelated non-GM bank before creating the demo");
            const auto name=IgniteDesktop::Demo::create(engine);
            check(songs.songInfoList.size()==count+1 && songs.activeSong->getName()==name,"demo is added and selected while retaining the collection");
            check(!engine.desktopHasUnsavedChanges(),"demo song, patterns and instruments save through normal Save All");
            inspect();
            auto* manifest=new juce::DynamicObject();manifest->setProperty("name",juce::String(name));
            root.getChildFile("demo-test.json").replaceWithText(juce::JSON::toString(juce::var(manifest)));
            IgniteDesktop::Demo::showSong();
            const auto before=songs.desktopAudioCallbacks.load();songs.desktopAudioPeak=0;
            engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleSong),paramId_t(SongParam::paramPlaySongCmd),uint8_t(0));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            check(songs.getSystemState()==SYSTEM_STATE::PLAY && songs.desktopAudioCallbacks.load()>before+2,"demo PLAY drives the Windows audio output");
            check(songs.desktopAudioPeak.load()>0.0001f,"demo notes produce a non-silent Windows output signal");
            for(int i=0;i<2 && songs.getSystemState()!=SYSTEM_STATE::IDLE;++i)
                engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleSong),paramId_t(SongParam::paramStopSongCmd),uint8_t(0));
            songs.stopJuceAudioCallback();
            renderNext();
        }
        catch(const std::exception& error){check(false,juce::String(error.what()));finish();}
    }
private:
    void check(bool passed,const juce::String& label)
    {++checks;if(!passed)++failures;juce::Logger::writeToLog(juce::String(passed?"PASS: ":"FAIL: ")+label);}
    void inspect()
    {
        auto& song=*engine.songManager->activeSong;int bars=0,patterns=0,notes=0;bool configured=true;
        check(song.getNumOfScenes()==4 && song._songData->gTrackCount==5,"demo contains four scenes and five tracks");
        for(auto& scene:song._songData->sceneList)
        {
            bars+=scene._sceneData->numBars;configured&=std::abs(scene._sceneData->sceneTempo-112)<0.001 && scene._sceneData->trackList.size()==5;
            for(auto& track:scene._sceneData->trackList)
            {
                auto* instrument=track.getTrackTxInstrument();auto* pattern=track.getTrackMainPattern();
                configured&=instrument && pattern && instrument->getInternalSound() && instrument->getMidiPort()==-1 && !track.getTrackMuteStatus();
                if(pattern){++patterns;const auto data=engine.EngineGetPatternData(pattern->getName());for(const auto& bar:data.BarDataList)for(const auto& step:bar.second.StepDataList)for(const auto& pitch:step.second.nodeList)notes+=int(pitch.second.size());}
            }
        }
        check(bars==16 && patterns==20 && notes>300,"sixteen arranged bars contain twenty assigned patterns and musical note data");
        check(configured,"all demo tracks use internal sound, valid assignments, 112 BPM and no external MIDI output");
    }
    void renderNext()
    {
        try
        {
            if(stem==5){finish();return;}
            const auto target=root.getChildFile(stem<0?"First Light Demo.wav":"demo-stem-"+juce::String(stem+1)+".wav");
            auto snapshot=IgniteDesktop::Media::capture(engine,false,stem,"wav");
            const auto result=job.start(std::move(snapshot),target,[this,target](juce::Result status)
            {
                check(status.wasOk(),stem<0?"demo full song renders through the isolated WAV exporter":"demo track stem exports");
                if(status.wasOk())
                {
                    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(target.createInputStream().release(),true));
                    check(reader && reader->numChannels==2 && std::abs(reader->lengthInSamples/reader->sampleRate-(16*4*60.0/112+2))<0.01,"demo render has the complete arrangement and two-second tail");
                    if(reader)
                    {
                        juce::AudioBuffer<float> audio(2,4096);float peak=0;bool finite=true;
                        for(int64_t offset=0;offset<reader->lengthInSamples;offset+=4096)
                        {
                            reader->read(&audio,0,4096,offset,true,true);
                            for(int channel=0;channel<2;++channel)for(int i=0;i<4096;++i){const float sample=audio.getSample(channel,i);finite&=std::isfinite(sample);peak=std::max(peak,std::abs(sample));}
                        }
                        check(finite && peak>0.00001f,"demo part renders finite, audible audio");
                    }
                }
                ++stem;renderNext();
            });
            if(result.failed())throw std::runtime_error(result.getErrorMessage().toStdString());
        }
        catch(const std::exception& error){check(false,juce::String(error.what()));finish();}
    }
    void finish(){juce::Logger::writeToLog("Demo checks: "+juce::String(checks)+", failures: "+juce::String(failures));done(failures);}
    Ignite::IgniteEngine& engine;std::function<void(int)> done;
    IgniteDesktop::Media::Job job;juce::File root;
    int checks=0,failures=0,stem=-1;
};
