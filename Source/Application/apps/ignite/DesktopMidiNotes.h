// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <deque>
#include <map>
#include <vector>

namespace IgniteDesktop::Media
{
    // Keep physical note duration separate from the lifetime of a pedal-held note.
    // Pattern events retain the physical note-off; only a later scene needs a
    // replacement note for a voice that the previous scene cannot carry over.
    class MidiNotes
    {
    public:
        struct Note {int pitch,velocity,start,end,soundEnd;};
        std::vector<Note> notes;
        int unmatched=0;

        void accept(const juce::MidiMessage& message,int tick)
        {
            if(message.isNoteOn())
            {
                const int pitch=message.getNoteNumber();
                keys[pitch].push_back(notes.size());
                notes.push_back({pitch,message.getVelocity(),tick,-1,-1});
            }
            else if(message.isNoteOff())
            {
                const int pitch=message.getNoteNumber();auto& held=keys[pitch];
                if(!held.empty()){release(held.front(),tick);held.pop_front();}
                else if(modeReleased[pitch]>0)--modeReleased[pitch];
                else ++unmatched;
            }
            else if(message.isController())
            {
                const int cc=message.getControllerNumber();
                if(cc==64)
                {
                    sustain=message.getControllerValue()>=64;
                    if(!sustain)releasePedal(tick);
                }
                else if(cc==121){sustain=false;releasePedal(tick);}
                else if(cc==120 || (cc>=123 && cc<=127))
                {
                    const bool kill=cc==120;
                    for(auto& entry:keys)
                    {
                        for(const auto index:entry.second)
                        {
                            release(index,tick,kill);++modeReleased[entry.first];
                        }
                        entry.second.clear();
                    }
                    if(kill)releasePedal(tick);
                }
            }
        }

        void finish(int tick)
        {
            for(auto& entry:keys)
            {
                for(const auto index:entry.second){release(index,tick);++unmatched;}
                entry.second.clear();
            }
            releasePedal(tick);
        }
    private:
        void release(size_t index,int tick,bool kill=false)
        {
            auto& note=notes[index];note.end=std::max(note.start+1,tick);
            if(sustain && !kill)pedal.push_back(index);
            else note.soundEnd=note.end;
        }
        void releasePedal(int tick)
        {
            for(const auto index:pedal)notes[index].soundEnd=std::max(notes[index].end,tick);
            pedal.clear();
        }
        bool sustain=false;
        std::map<int,std::deque<size_t>> keys;
        std::map<int,int> modeReleased;
        std::vector<size_t> pedal;
    };
}
