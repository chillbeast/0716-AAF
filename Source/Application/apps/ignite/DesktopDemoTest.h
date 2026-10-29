// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopDemo.h"
#include "DesktopMediaJob.h"
#include "DesktopPanel.h"

class DesktopDemoTest : private juce::Timer
{
public:
    DesktopDemoTest(Ignite::IgniteEngine& model,std::function<void(int)> finished):engine(model),done(std::move(finished)){}
    ~DesktopDemoTest() override {stopTimer();}
    void start(bool verify,bool unavailable)
    {
        using namespace Ignite;root=SongManager::desktopSettingsFile().getParentDirectory();
        try
        {
            auto& songs=*engine.songManager;
            if(unavailable)
            {
                auto transport=[&](SongParam command){engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleSong),paramId_t(command),uint8_t(0));};
                transport(SongParam::paramPlaySongCmd);transport(SongParam::paramPauseSongCmd);transport(SongParam::paramReStartSongCmd);
                check(!songs.activeSong && songs.getSystemState()==SYSTEM_STATE::IDLE,"remote transport commands with no selected song stay idle without crashing");
                const auto count=songs.songInfoList.size();bool refused=false;
                try{IgniteDesktop::Demo::create(engine);}catch(const std::exception& error){refused=juce::String(error.what()).contains("General MIDI SF2");}
                check(refused && songs.songInfoList.size()==count && engine.patternManager->patternList.empty(),"missing/incomplete GM bank gives setup instructions without adding a partial demo");
                transport(SongParam::paramAddSongCmd);engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleSong),paramId_t(SongParam::paramActiveSong),int32_t(0));
                transport(SongParam::paramPlaySongCmd);transport(SongParam::paramPauseSongCmd);transport(SongParam::paramReStartSongCmd);
                check(songs.activeSong && songs.activeSong->getNumOfScenes()==0 && songs.getSystemState()==SYSTEM_STATE::IDLE,"transport commands ignore a song with no scenes");
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
                verifyMode=true;beginTransport();return;
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
            beginTransport();
        }
        catch(const std::exception& error){check(false,juce::String(error.what()));finish();}
    }
private:
    void beginTransport()
    {if(!verifyMode)IgniteDesktop::Demo::showSong();phase=0;deadline=juce::Time::getMillisecondCounterHiRes()+4000;startTimer(250);}
    juce::Button* transportButton(const char* id)
    {return dynamic_cast<juce::Button*>(ui::IgniteControlOptions::GetAppOptInstance()->findChildWithID(id));}
    void click(const char* id,bool immediate=false)
    {
        auto* button=transportButton(id);check(button && button->isEnabled(),juce::String(id)+" is enabled for the five-track demo");
        if(button)
        {
            // Rewind's cooldown must be checked before yielding to other timers.
            if(immediate)ui::IgniteControlOptions::GetAppOptInstance()->buttonClicked(button);
            else button->triggerClick();
        }
    }
    void key(int code)
    {
        auto* parent=ui::getAppHome().getParentComponent();
        auto* panel=parent?dynamic_cast<DesktopPanel*>(parent->getParentComponent()):nullptr;
        check(panel && panel->keyPressed(juce::KeyPress(code)),"desktop keyboard transport reaches the firmware controls");
    }
    void timerCallback() override
    {
        using namespace Ignite;auto& songs=*engine.songManager;
        try
        {
            switch(phase++)
            {
                case 0:
                    if(ui::AppMainmode::getMainModeInstance()->GetActiveTab()!=mt::Ignite::tabs::SONG && juce::Time::getMillisecondCounterHiRes()<deadline){--phase;return;}
                    juce::Logger::writeToLog("UI transport ready: tab="+juce::String(int(ui::AppMainmode::getMainModeInstance()->GetActiveTab()))+", song="+juce::String(songs.activeSong->getName()));
                    if(verifyMode)check(ui::AppMainmode::getMainModeInstance()->GetActiveTab()==mt::Ignite::tabs::SONG,"song launch option opens the saved project ready for transport");
                    check(ui::AppMainmode::getMainModeInstance()->GetActiveTab()==mt::Ignite::tabs::SONG,"demo opens in Song view before starting transport");
                    check(int(ui::IgniteSong::getAppSongInstance()->getSongData(mt::Ignite::songsDataIndex::NUM_TRACKS_INDEX))==1,"five tracks occupy one visible track bank");
                    before=songs.desktopAudioCallbacks.load();songs.desktopAudioPeak=0;click("firmware-play");deadline=juce::Time::getMillisecondCounterHiRes()+5000;break;
                case 1:
                    if((songs.getSystemState()!=SYSTEM_STATE::PLAY || songs.desktopAudioCallbacks.load()<=before+2 || songs.desktopAudioPeak.load()<=0.0001f) && juce::Time::getMillisecondCounterHiRes()<deadline){--phase;return;}
                    juce::Logger::writeToLog("UI transport result: state="+juce::String(int(songs.getSystemState()))+", callbacks="+juce::String(songs.desktopAudioCallbacks.load()-before)+", peak="+juce::String(songs.desktopAudioPeak.load())+", song="+juce::String(songs.activeSong->getName()));
                    check(songs.getSystemState()==SYSTEM_STATE::PLAY && songs.desktopAudioCallbacks.load()>before+2,"clicking the visible Play button starts live Windows audio");
                    check(songs.desktopAudioPeak.load()>0.0001f,"visible Play control produces non-silent demo audio");
                    click("firmware-play");break;
                case 2:
                    check(songs.getSystemState()==SYSTEM_STATE::PAUSE,"clicking the visible Play button again pauses playback");
                    key(juce::KeyPress::escapeKey);break;
                case 3:
                    check(songs.getSystemState()==SYSTEM_STATE::IDLE,"Escape stops the demo through the panel input path");
                    before=songs.desktopAudioCallbacks.load();songs.desktopAudioPeak=0;key(juce::KeyPress::spaceKey);deadline=juce::Time::getMillisecondCounterHiRes()+5000;break;
                case 4:
                    if((songs.getSystemState()!=SYSTEM_STATE::PLAY || songs.desktopAudioCallbacks.load()<=before+2 || songs.desktopAudioPeak.load()<=0.0001f) && juce::Time::getMillisecondCounterHiRes()<deadline){--phase;return;}
                    check(songs.getSystemState()==SYSTEM_STATE::PLAY && songs.desktopAudioCallbacks.load()>before+2 && songs.desktopAudioPeak.load()>0.0001f,"Space starts audible playback through the panel input path");
                    click("firmware-rewind",true);
                    check(transportButton("firmware-rewind") && !transportButton("firmware-rewind")->isEnabled(),"rewind handler returns with its cooldown still pending");
                    click("firmware-stop");startTimer(1400);break;
                default:
                    stopTimer();check(songs.getSystemState()==SYSTEM_STATE::IDLE,"visible Stop button returns transport to idle");
                    check(songs.activeSong->_songData->gTrackCount==5,"UI playback preserves the actual five-track arrangement");
                    check(transportButton("firmware-rewind") && !transportButton("firmware-rewind")->isEnabled(),"pending rewind does not re-enable controls after Stop");
                    songs.stopJuceAudioCallback();
                    if(verifyMode)
                    {songs.setActiveSong(0);check(songs.activeSong && songs.activeSong->getName()!="Demo - First Light","existing song remains selectable after creating the demo");finish();}
                    else renderNext();
                    break;
            }
        }
        catch(const std::exception& error){check(false,juce::String(error.what()));finish();}
    }
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
    void finish(){stopTimer();juce::Logger::writeToLog("Demo checks: "+juce::String(checks)+", failures: "+juce::String(failures));done(failures);}
    Ignite::IgniteEngine& engine;std::function<void(int)> done;
    IgniteDesktop::Media::Job job;juce::File root;
    int checks=0,failures=0,stem=-1;
    int phase=0;uint64_t before=0;bool verifyMode=false;
    double deadline=0;
};
