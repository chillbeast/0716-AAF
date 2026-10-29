// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "Firmware/Application/Model/SettingsManager/SettingsSculptModeCnfg.h"
extern std::unique_ptr<Ignite::SettingsSculptModeCnfg> _settingsSculptModeCnfg;

// Runs against the real engine and UI, exclusively in the disposable test data tree.
class DesktopEditorTest
{
public:
    template<class Check,class Command>
    static void run(Ignite::IgniteEngine& engine,Check check,Command command)
    {
        using namespace Ignite;
        auto& manager=*engine.patternManager;
        auto& songs=*engine.songManager;
        auto* track=songs.activeSong->editScene->editTrack;
        const auto name=manager.addPattern();
        auto add=[&](const std::string& target,int step,int pitch,int velocity=100)
        {
            ClientPatternNodeData node{};node.note=node.nodeId=uint8_t(pitch);node.velocity=uint8_t(velocity);
            node.onState=true;node.lenInSteps=1;node.step_id=uint16_t(step);
            return engine.EngineAddNodeData(target,uint16_t(step/16),uint16_t(step),node);
        };
        add(name,0,60);add(name,15,72);
        manager.SavePattern(manager.GetPattern(name),true);
        check(engine.EngineInsertPatternStep(name,0,2,0,{}),"blank step insertion is implemented through the public engine API");
        auto inserted=engine.EngineGetPatternData(name);
        check(inserted.numBars==2 && inserted.patternLastStepId==18 && inserted.BarDataList.at(0).StepDataList.at(2).nodeList.count(60)
              && inserted.BarDataList.at(1).StepDataList.at(1).nodeList.at(72).at(0).step_id==17,
              "step insertion shifts notes across bars and updates note IDs and active length");
        check(engine.EngineInsertPatternStep(name,0,1,4,{}) && engine.EngineGetPatternData(name).numBars==2,
              "repeated insertion reuses spare bar capacity");
        command(ModuleName::modulePattern,PatternParam::paramUndoCmd,uint8_t(0));
        check(engine.EngineGetPatternData(name).patternLastStepId==18,"Undo reverses exactly one inserted step");
        command(ModuleName::modulePattern,PatternParam::paramUndoCmd,uint8_t(0));
        auto restored=engine.EngineGetPatternData(name);
        check(restored.numBars==1 && restored.patternLastStepId==16 && restored.BarDataList.at(0).StepDataList.at(15).nodeList.count(72),
              "Undo restores the original bar count and notes");
        auto payload=restored.BarDataList.at(0).StepDataList.at(0);
        check(engine.EngineInsertPatternStep(name,0,1,8,{payload})
              && engine.EngineGetPatternData(name).BarDataList.at(0).StepDataList.at(8).nodeList.at(60).at(0).step_id==8,
              "step payload insertion creates independent notes with corrected IDs");
        const auto history=manager.PatternGetUndoList().size();
        check(!engine.EngineInsertPatternStep(name,99,1,0,{}) && !engine.EngineInsertPatternStep(name,0,1,17,{})
              && !engine.EngineInsertPatternStep(name,0,2,0,{payload}) && manager.PatternGetUndoList().size()==history,
              "invalid insertion requests leave the pattern and undo history intact");
        const auto full=manager.addPattern();
        for(int i=1;i<16;++i) engine.EngineInsertPatternBar(full,uint16_t(i));
        add(full,255,84);
        check(!engine.EngineInsertPatternStep(full,0,1,0,{})
              && engine.EngineGetPatternData(full).BarDataList.at(15).StepDataList.at(15).nodeList.count(84),
              "insertion at the 256-step limit refuses overflow without losing the final note");

        command(ModuleName::moduleTrack,TrackParam::paramTrackPattern,name);
        songs.initPlayScene(0);
        track->_trackPlayStepId=0;track->_trackBarId=0;
        auto* pattern=manager.GetPattern(name);
        auto& sculpt=_settingsSculptModeCnfg->_SettingsSculptModeCnfgData;
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptButtonOnState,true);
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptParameterSelection,uint8_t(1));
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptMode,int8_t(SculptMode::relative));
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptRelativeVelocity,int8_t(-120));
        track->sculptUpdate(pattern);
        check(engine.EngineGetPatternData(name).BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).velocity==0,
              "relative Sculpt accepts negative offsets and clamps velocity at zero");
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptMode,int8_t(SculptMode::absolute));
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptAbsoluteVelocity,int8_t(127));
        track->sculptUpdate(pattern);
        check(engine.EngineGetPatternData(name).BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).velocity==127,
              "absolute Sculpt preserves maximum MIDI velocity");
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptMode,int8_t(SculptMode::randomRelative));
        sculpt.sculptRandomRelativeVelocityStart=sculpt.sculptRandomRelativeVelocityEnd=-7;
        track->sculptUpdate(pattern);
        check(engine.EngineGetPatternData(name).BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).velocity==120,
              "random relative Sculpt applies a signed offset to the original velocity");
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptParameterSelection,uint8_t(SculptParameterSelect::note));
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptMode,int8_t(SculptMode::relative));
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptRelativeNote,int8_t(2));
        track->sculptUpdate(pattern);
        check(engine.EngineGetPatternData(name).BarDataList.at(0).StepDataList.at(0).nodeList.count(62)==1,
              "pitch Sculpt moves each note exactly once without invalidating the note map");
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptButtonOnState,false);

        _settingsSculptModeCnfg->saveForDesktop();
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptAbsoluteVelocity,int8_t(91));
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::sculptAbsoluteVelocity,int8_t(92));
        command(ModuleName::moduleSettingsSculptModeCnfg,SettingsSculptModeCnfgParam::paramUndoCmd,int8_t(1));
        check(sculpt.sculptAbsoluteVelocity==91,"Sculpt settings Undo restores the immediately preceding edit");
        command(ModuleName::moduleMidiCnfg,SettingsMidiCnfgParam::paramConfigMidiActivePort,int8_t(0));
        engine.settingsMidiCnfg->saveForDesktop();
        command(ModuleName::moduleMidiCnfg,SettingsMidiCnfgParam::paramConfigMidiDestChannel,int8_t(4));
        command(ModuleName::moduleMidiCnfg,SettingsMidiCnfgParam::paramConfigMidiDestChannel,int8_t(5));
        command(ModuleName::moduleMidiCnfg,SettingsMidiCnfgParam::paramUndoCmd,int8_t(1));
        check(engine.settingsMidiCnfg->getMidiDestChannel()==4,"MIDI settings Undo restores the immediately preceding edit");

        auto* scene=songs.activeSong->playScene;
        scene->setFTSRootNote(0);scene->setFTSScale(int8_t(ScaleList::ScaleMajor));scene->setTransposeValue(0);
        bool scaleOK=true;
        const auto scale=songs.generateScaleNotes(0,int8_t(ScaleList::ScaleMajor));
        for(int i=0;i<128;++i)
        {
            uint8_t note=uint8_t(i);songs.applyFtsAndTranspose(note,false,false);
            scaleOK &= note<=127 && std::find(scale.begin(),scale.end(),note%12)!=scale.end();
        }
        check(scaleOK,"force-to-scale keeps all 128 MIDI pitches in scale across octave boundaries");
        scene->setTransposeValue(1);
        scene->setTransposeFTSPriority(int8_t(TransposeFTSPriority::TransposePreFTS));
        uint8_t pre=60;songs.applyFtsAndTranspose(pre,false,false);
        scene->setTransposeFTSPriority(int8_t(TransposeFTSPriority::TransposePostFTS));
        uint8_t post=60;songs.applyFtsAndTranspose(post,false,false);
        check(pre==62 && post==61,"transpose before or after scale snapping produces the selected priority");
        uint8_t defeated=61;songs.applyFtsAndTranspose(defeated,true,true);
        check(defeated==61,"instrument transpose and scale defeat preserve the original pitch");
        scene->setFTSScale(-1);scene->setTransposeValue(12);
        uint8_t high=126;songs.applyFtsAndTranspose(high,false,false);
        scene->setTransposeValue(-12);uint8_t low=1;songs.applyFtsAndTranspose(low,false,false);
        check(high==127 && low==0,"live transpose clamps at both MIDI pitch limits");
        scene->setTransposeValue(0);

        songs.setFillModeRelease(int8_t(FillReleaseMode::key));songs.setFillModeSwitchOnState(true);
        songs.setFillModeType(int8_t(FillModeType::automatic));track->setTrackMuteStatus(0);
        check(track->shouldPlayFillPattern(),"automatic fill selects the fill on an unmuted track");
        track->setTrackMuteStatus(1);
        check(!track->shouldPlayFillPattern(),"automatic fill respects track mute");
        songs.setFillModeType(int8_t(FillModeType::active));
        check(track->shouldPlayFillPattern(),"active fill can play a muted track");
        songs.setFillModeSwitchOnState(false);
        check(!track->shouldPlayFillPattern(),"releasing a key-length fill returns to the main pattern");
        track->setTrackMuteStatus(0);
        // Scene 1 and bar 0 deliberately differ, catching the old scene/bar mixup.
        command(ModuleName::moduleScene,SceneParam::paramAddSceneCmd,int32_t(1));
        songs.activeSong->setPlayScene(1);songs.setFillModeRelease(int8_t(FillReleaseMode::scene));
        check(songs.getFillModeTriggerState(),"scene-length fill remains active in its originating scene");
        songs.activeSong->setPlayScene(0);
        check(!songs.getFillModeTriggerState(),"scene-length fill ends when the play scene changes");

        auto* ui=ui::IgnitePattern::getAppPatternInstance();ui->updateDataFromEngine();
        auto names=engine.EngineGetPatternList();auto it=std::find(names.begin(),names.end(),name);
        ui->setSelected(int(std::distance(names.begin(),it)));
        ui->resetGangStepState();ui->setGangStep(0,true);
        ui->setPresetValueForSteps(mt::Ignite::nodeDataIndex::VELO,std::vector<int>(16,77),false);
        auto gang=engine.EngineGetPatternData(name);
        check(gang.BarDataList.at(0).StepDataList.at(0).nodeList.at(62).at(0).velocity==77
              && gang.BarDataList.at(0).StepDataList.at(8).nodeList.at(60).at(0).velocity==100,
              "gang velocity edits affect selected steps and preserve other notes");
        check(ui->insertBlankStep() && engine.EngineGetPatternData(name).BarDataList.at(0).StepDataList.at(0).nodeList.empty(),
              "desktop blank-step insertion refreshes the real pattern UI");
        check(engine.SaveAllData() && !engine.desktopHasUnsavedChanges(),"the audited editor state saves completely");
        auto file=SongManager::desktopSettingsFile().getParentDirectory().getChildFile("correctness.json");
        auto manifest=juce::JSON::parse(file);manifest.getDynamicObject()->setProperty("inserted",juce::String(name));
        file.replaceWithText(juce::JSON::toString(manifest));
    }
};
