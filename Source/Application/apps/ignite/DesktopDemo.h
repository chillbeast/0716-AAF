// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopMidiSong.h"

namespace IgniteDesktop::Demo
{
    inline juce::File midiFile()
    {return ui::Theme::getAssetsDirectory().getChildFile("demo/First Light Demo.mid");}

    inline std::string create(Ignite::IgniteEngine& engine)
    {
        auto lock=engine.lockDesktopState();auto& songs=*engine.songManager;
        const auto presets=songs.getDesktopSoundFontPresets(true);
        for(const auto& wanted:std::vector<std::pair<int,int>>{{0,4},{0,12},{0,33},{0,80},{128,0}})
            if(std::none_of(presets.begin(),presets.end(),[&](const auto& preset){return preset.bank==wanted.first && preset.program==wanted.second;}))
                throw std::runtime_error("Load a General MIDI SF2 in SoundFonts and choose Set as default SF2 first. The demo needs bank 0 programs 4, 12, 33, 80 and bank 128 program 0 (drums).");
        auto plan=Media::MidiSong::parse(midiFile());plan.name="Demo - First Light";
        const auto name=plan.apply(engine,"");
        // A local audio demo does not need an external MIDI output assignment.
        for(auto& scene:songs.activeSong->_songData->sceneList)for(auto& track:scene._sceneData->trackList)
            track.getTrackTxInstrument()->_instrumentData->midiPort=-1;
        if(!engine.SaveAllData())throw std::runtime_error("The demo is open but could not be saved. Check the data folder and use Save before closing.");
        return name;
    }

    inline void showSong()
    {
        ui::IgniteSong::getAppSongInstance()->updateDataFromEngine();
        ui::IgnitePattern::getAppPatternInstance()->updateDataFromEngine();
        ui::AppMainmode::getMainModeInstance()->HandleTabKey(mt::Ignite::tabs::SONG);
    }
}
