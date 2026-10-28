// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <JuceHeader.h>
#include <cmath>

// Exercises the same model commands used by the touchscreen, in disposable data.
class DesktopWorkflowTest
{
public:
    static int run(Ignite::IgniteEngine& engine, bool verify, bool nodeEdit, bool audioDevice)
    {
        using namespace Ignite;
        int failures = 0, checks = 0;
        auto check = [&](bool pass, const juce::String& message)
        {
            ++checks;
            juce::Logger::writeToLog(juce::String(pass ? "PASS: " : "FAIL: ") + message);
            if (!pass) ++failures;
            return pass;
        };
        auto command = [&](ModuleName module, auto param, AnyType value)
        {
            return engine.SetCommand(moduleId_t(ModuleName::moduleUI), moduleId_t(module), paramId_t(param), value);
        };
        auto& songs = *engine.songManager;
        juce::Logger::writeToLog(verify ? "WORKFLOW: verify persisted arrangement" : "WORKFLOW: create arrangement");
        if (!verify)
        {
            check(static_cast<const SongManager&>(songs).getSongCount() == 0, "fresh data directory contains no songs");
            command(ModuleName::moduleSong, SongParam::paramAddSongCmd, uint8_t(0));
        }
        command(ModuleName::moduleSong, SongParam::paramActiveSong, int32_t(0));
        if (!check(songs.activeSong != nullptr, "song selected")) return failures;
        if (!verify) command(ModuleName::moduleScene, SceneParam::paramAddSceneCmd, int32_t(0));
        command(ModuleName::moduleScene, SceneParam::paramActiveScene, int32_t(0));
        if (!check(songs.activeSong->editScene != nullptr, "scene selected")) return failures;
        command(ModuleName::moduleTrack, TrackParam::paramActiveTrack, uint8_t(0));
        if (!check(songs.activeSong->editScene->editTrack != nullptr, "track selected")) return failures;
        if (!verify)
        {
            auto instrumentName = engine.instrumentManager->addInstrument();
            auto* instrument = engine.instrumentManager->getInstrument(instrumentName);
            if (!check(instrument != nullptr, "instrument created")) return failures;
            instrument->setMidiPort(0);
            instrument->setMidiChannel(0);
            instrument->setInternalSound(true);
            instrument->setInternalSoundPgm(0);
            engine.instrumentManager->SaveInstrument(instrumentName);
            const auto patternName = engine.patternManager->addPattern();
            auto* pattern = engine.patternManager->GetPattern(patternName);
            if (!check(pattern != nullptr, "pattern created")) return failures;
            pattern->setTimeBase(TimeBase::Base_16);
            for (int step : {0, 4, 8, 12})
            {
                auto data = engine.EngineGetPatternData(patternName).BarDataList.at(0).StepDataList.at(uint16_t(step));
                data.enable = true;
                ClientPatternNodeData node{};
                node.nodeId = node.note = uint8_t(60 + step / 4);
                node.velocity = 100;
                node.lenInSteps = 1;
                node.lenInTicks = 0;
                node.offset = 0;
                node.onState = true;
                node.step_id = uint16_t(step);
                data.nodeList[node.note][0] = node;
                data.nodeCount = 1;
                check(engine.EngineSetStepData(moduleId_t(ModuleName::moduleUI), patternName, 0, uint16_t(step), data),
                      "pattern step " + juce::String(step + 1) + " edited");
            }
            command(ModuleName::moduleTrack, TrackParam::paramTrackOutInstrument, instrumentName);
            command(ModuleName::moduleTrack, TrackParam::paramTrackPattern, patternName);
            command(ModuleName::moduleTrack, TrackParam::paramTrackMute, uint8_t(0));
            engine.patternManager->SavePattern(pattern, false);
            songs.saveSong();
            juce::Logger::writeToLog("WORKFLOW: saved arrangement");
            if (nodeEdit)
            {
                auto node = engine.EngineGetPatternData(patternName).BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0);
                node.velocity = 110;
                juce::Logger::writeToLog("WORKFLOW: editing an existing note through the node API");
                check(engine.EngineAddNodeData(patternName, 0, 0, node), "note API accepts velocity edit");
                command(ModuleName::modulePattern, PatternParam::paramSaveAll, int8_t(0));
                const auto saved = engine.fileHandler->GetpatternBarStepData(patternName, 0, 16);
                check(!saved.empty() && saved.front().nodeList.at(60).at(0).velocity == 110,
                      "Save command writes the latest pattern edit");
            }
        }
        auto* track = songs.activeSong->editScene->editTrack;
        auto* pattern = track->getTrackMainPattern();
        auto* instrument = track->getTrackTxInstrument();
        check(!track->getTrackMuteStatus(), "track mute state preserved");
        if (!check(pattern && instrument, "pattern and instrument assignments preserved")) return failures;
        if (!verify)
        {
            command(ModuleName::moduleScene, SceneParam::paramSceneSwing, uint8_t(3));
            command(ModuleName::moduleScene, SceneParam::paramSaveSceneCmd, int8_t(0));
            check(engine.fileHandler->GetSceneInfo(songs.activeSong->getName(), songs.activeSong->editScene->getName()).sceneSwing == 3,
                  "scene Save command preserves scene edits on disk");
            songs.ClearUndoList();
            command(ModuleName::moduleScene, SceneParam::paramSceneSwing, uint8_t(4));
            command(ModuleName::moduleScene, SceneParam::paramSaveAll, int8_t(0));
            command(ModuleName::moduleSong, SongParam::paramSaveAll, int8_t(0));
        }
        check(engine.fileHandler->GetSceneInfo(songs.activeSong->getName(), songs.activeSong->editScene->getName()).sceneSwing == 4,
              "Save All preserves scene-only edits when song parameters are clean");
        check(instrument->getInternalSound(), "internal sound enabled");
        check(instrument->getInternalSoundPgm() == 0, "program selection preserved");
        const auto data = engine.EngineGetPatternData(pattern->getName());
        check(data.numBars == 1 && data.patternLastStepId == 16 && data.BarDataList.at(0).StepDataList.size() == 16,
              "one-bar pattern contains all 16 step positions");
        int notes = 0;
        for (const auto& step : data.BarDataList.at(0).StepDataList)
            for (const auto& pitch : step.second.nodeList) notes += int(pitch.second.size());
        check(notes == 4, "four edited notes preserved");
        if (verify && nodeEdit)
            check(data.BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0).velocity == 110,
                  "Save command preserves note edits across a process restart");
        {
            CSong copy(*songs.activeSong);
            check(copy.editScene != songs.activeSong->editScene && copy.editScene != nullptr &&
                  copy.editScene == &copy._songData->sceneList.front(), "copied song selects its own scene");
            check(copy._songData->gTrackCount == songs.activeSong->_songData->gTrackCount,
                  "copied song retains its track count");
            copy._songData->gTrackCount = 7;
            copy._songData->applySongTempoToScenes = false;
            CSong assigned(99, "copy target", 100, 0, 0, &songs);
            assigned = copy;
            check(assigned._songData != copy._songData && assigned.editScene != copy.editScene &&
                  assigned.editScene == &assigned._songData->sceneList.front() &&
                  assigned._songData->gTrackCount == 7 && !assigned._songData->applySongTempoToScenes,
                  "song assignment owns its scenes and retains settings");
            Scene scene(99, "scene target", 100, 0, 0, 0, &songs);
            scene = *copy.editScene;
            check(scene._sceneData != copy.editScene->_sceneData && scene.editTrack != copy.editScene->editTrack &&
                  scene.editTrack == &scene._sceneData->trackList.front(), "scene assignment owns its selected track");
            Instrument assignedInstrument(*instrument);
            assignedInstrument = *instrument;
            assignedInstrument.setInternalSoundPgm(1);
            check(assignedInstrument._instrumentData != instrument->_instrumentData && instrument->getInternalSoundPgm() == 0,
                  "instrument assignment owns its settings");
        }
        {
            auto invalid = *songs.activeSong->_songData;
            invalid.sceneList.front()._sceneData->sceneName = "nested\\invalid scene";
            check(engine.fileHandler->CreateSong(invalid) == FILE_ERROR, "invalid replacement song reports a save failure");
            const auto retained = engine.fileHandler->GetSongInfo(songs.activeSong->getName());
            check(retained.numberOfScenes == songs.activeSong->getNumOfScenes() &&
                  engine.fileHandler->GetSceneList(songs.activeSong->getName(), 0, 65535).size() == 1,
                  "failed song replacement retains the previously saved arrangement");
        }
        {
            Pattern copy(*pattern);
            check(copy._patternData != pattern->_patternData, "pattern copy owns its data");
            check(copy._patternData->barList.begin()->second._BarData != pattern->_patternData->barList.begin()->second._BarData,
                  "pattern copy owns independent bars");
            copy.AddBar(false, 0);
            check(copy.getTotalBars() == 2 && copy.getLastStepId() == 32, "appending a bar preserves every step position");
            copy.AddBar(true, 0);
            check(copy.getTotalBars() == 3 && copy._patternData->barList.count(2) == 1,
                  "duplicating a bar completes with sequential bar IDs");
            copy.insertBar(1);
            check(copy.getTotalBars() == 4 && copy.getLastStepId() == 64,
                  "inserting a bar preserves the full pattern length");
            bool sequential = true;
            int barId = 0, stepId = 0, copiedNotes = 0;
            for (const auto& bar : copy._patternData->barList)
            {
                sequential &= bar.first == barId && bar.second._BarData->BarId == barId;
                for (const auto& step : bar.second._BarData->stepList)
                {
                    sequential &= step.first == stepId && step.second._stepData->stepId == stepId;
                    for (const auto& pitch : step.second._stepData->nodeList)
                        for (const auto& node : pitch.second)
                        {
                            ++copiedNotes;
                            sequential &= node.second.step_id == stepId;
                        }
                    ++stepId;
                }
                ++barId;
            }
            check(sequential && copiedNotes == 8, "bar edits preserve notes and renumber bars, steps and note positions");
            check(copy.RemoveBar(1) && copy.getTotalBars() == 3 && copy.getLastStepId() == 48,
                  "removing a bar updates the complete pattern length");
            check(copy.CopyBar(0, 1) && copy._patternData->barList.find(1)->second._BarData !=
                  copy._patternData->barList.find(0)->second._BarData, "bar overwrite owns an independent copy");
            check(!copy.CopyBar(99, 0), "copying a missing source bar reports failure");
            copy.addBars(100);
            check(copy.getTotalBars() == MAX_NUM_OF_BARS_IN_PATTERN, "bar count is capped at the firmware limit");
            while (copy.getTotalBars() > 1) copy.RemoveBar(copy.getTotalBars() - 1);
            check(!copy.RemoveBar(0) && copy.getTotalBars() == 1, "the final pattern bar cannot be deleted");
        }
        engine.sequenceManager->initSequence();
        songs.initPlayScene(0);
        songs._state = SYSTEM_STATE::PLAY;
        float peak = 0;
        for (int pulse = 0; pulse < 384; ++pulse)
        {
            track->pulse(0, int16_t(pulse / 24));
            float block[INTERNAL_AUDIO_20MS_BLOCK_SIZE]{};
            songs.renderTracksAndMix(block);
            for (float sample : block) peak = std::max(peak, std::abs(sample));
        }
        check(std::isfinite(peak), "sequenced mix remains finite");
        check(songs.get_tsf_instance() == nullptr ? peak == 0 : peak > 0.0001f, "sequenced arrangement produces expected audio");
        track->deInitTrackPlaySetting();
        songs._state = SYSTEM_STATE::IDLE;

        for (int count : {1, 7, 8, 9, 16, 64})
        {
            command(ModuleName::moduleSong, SongParam::paramTrackCountCmd, int8_t(count));
            songs.initPlayScene(0);
            songs._state = SYSTEM_STATE::PLAY;
            std::vector<std::pair<Track*, uint64_t>> pending;
            for (auto& t : songs.activeSong->playScene->_sceneData->trackList)
                pending.emplace_back(&t, t.completedPulses.load(std::memory_order_acquire));
            songs.pulse(0);
            bool complete = false;
            for (int retry = 0; retry < 100 && !complete; ++retry)
            {
                complete = std::all_of(pending.begin(), pending.end(), [](const auto& item)
                    { return item.first->completedPulses.load(std::memory_order_acquire) > item.second; });
                if (!complete) std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            check(complete, "clock delivers pulses to every track at track count " + juce::String(count));
            songs._state = SYSTEM_STATE::IDLE;
        }
        // Keep audio rendering active while UI commands resize the model and edit notes.
        songs.initPlayScene(0);
        songs._state = SYSTEM_STATE::PLAY;
        auto* clockTrack = &songs.activeSong->playScene->_sceneData->trackList.front();
        const auto beforeClock = clockTrack->completedPulses.load();
        std::atomic<bool> rendering{true};
        std::atomic<bool> finiteAudio{true};
        std::thread audio([&]
        {
            while (rendering)
            {
                float block[INTERNAL_AUDIO_20MS_BLOCK_SIZE]{};
                songs.renderTracksAndMix(block);
                for (float sample : block) if (!std::isfinite(sample)) finiteAudio = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        });
        auto& clock = CMidiClock::getInstance();
        clock.setTempo(130);
        clock.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        check(clockTrack->completedPulses.load() > beforeClock + 5, "live clock advances the arrangement");
        for (int count : {1, 64, 7, 16, 9, 1, 64, 8})
        {
            command(ModuleName::moduleSong, SongParam::paramTrackCountCmd, int8_t(count));
            songs.initPlayScene(0);
            auto edited = engine.EngineGetPatternData(pattern->getName()).BarDataList.at(0).StepDataList.at(0).nodeList.at(60).at(0);
            edited.velocity = uint8_t(80 + count % 30);
            engine.EngineAddNodeData(pattern->getName(), 0, 0, edited);
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        clock.pause();
        const auto paused = clockTrack->completedPulses.load();
        clock.setTempo(180);
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        check(clockTrack->completedPulses.load() == paused, "pause and tempo changes keep the playhead still");
        clock.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        check(clockTrack->completedPulses.load() > paused + 5, "playback resumes after edits and tempo change");
        clock.stop();
        rendering = false;
        audio.join();
        check(finiteAudio, "concurrent playback edits render finite audio");
        songs._state = SYSTEM_STATE::IDLE;
        if (audioDevice)
        {
            const auto beforeAudio = songs.desktopAudioCallbacks.load();
            const auto beforeSource = songs.desktopSourceSamples.load();
            const auto beforeOutput = songs.desktopOutputSamples.load();
            songs.desktopAudioPeak = 0;
            command(ModuleName::moduleSong, SongParam::paramPlaySongCmd, uint8_t(0));
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            check(clock.isPlaying() && songs.getSystemState() == SYSTEM_STATE::PLAY,
                  "transport command starts live playback");
            check(songs.desktopAudioCallbacks.load() > beforeAudio + 2,
                  "Windows output device receives audio callbacks");
            check(songs.get_tsf_instance() == nullptr || songs.desktopAudioPeak.load() > 0.0001f,
                  "sequenced SoundFont audio reaches the Windows output callback");
            command(ModuleName::moduleSong, SongParam::paramPauseSongCmd, uint8_t(0));
            check(!clock.isPlaying() && songs.getSystemState() == SYSTEM_STATE::PAUSE,
                  "transport command pauses playback");
            const auto pausedCallbacks = songs.desktopAudioCallbacks.load();
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
            check(songs.desktopAudioCallbacks.load() == pausedCallbacks, "pause closes the audio device");
            const auto sourceFrames = songs.desktopSourceSamples.load() - beforeSource;
            const auto outputFrames = songs.desktopOutputSamples.load() - beforeOutput;
            const auto expected = double(outputFrames) * INTERNAL_AUDIO_SAMPLE_RATE / songs.desktopHostSampleRate.load();
            check(outputFrames > 0 && std::abs(double(sourceFrames) - expected) < 128,
                  "audio conversion preserves the synth sample rate at the negotiated device rate");
            command(ModuleName::moduleSong, SongParam::paramPlaySongCmd, uint8_t(0));
            std::this_thread::sleep_for(std::chrono::milliseconds(120));
            check(clock.isPlaying() && songs.desktopAudioCallbacks.load() > pausedCallbacks,
                  "resume reopens the audio device");
            command(ModuleName::moduleSong, SongParam::paramStopSongCmd, uint8_t(0));
            if (songs.getSystemState() != SYSTEM_STATE::IDLE)
                command(ModuleName::moduleSong, SongParam::paramStopSongCmd, uint8_t(0)); // safe-stop option
            check(!clock.isPlaying() && songs.getSystemState() == SYSTEM_STATE::IDLE,
                  "transport command stops playback");
        }
        juce::Logger::writeToLog("Workflow checks: " + juce::String(checks) + ", failures: " + juce::String(failures));
        return failures;
    }
};
