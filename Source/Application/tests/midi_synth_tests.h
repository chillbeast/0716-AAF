// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <memory>
#include <numeric>
#include <vector>

template<class Check> void midiSynthTests(tsf* font,Check check)
{
    auto synth=[&]
    {
        std::unique_ptr<tsf,decltype(&tsf_close)> f(tsf_copy(font),tsf_close);
        tsf_set_output(f.get(),TSF_MONO,48000,0); tsf_set_max_voices(f.get(),8);
        tsf_channel_set_presetindex(f.get(),0,0); return f;
    };
    auto render=[](tsf* f,int samples=48000)
    { std::vector<float> audio(size_t(samples),0); tsf_render_float(f,audio.data(),samples,0); return audio; };
    auto rms=[](const std::vector<float>& audio)
    { double sum=0;for(float value:audio) sum+=double(value)*value;return std::sqrt(sum/audio.size()); };
    auto periods=[](const std::vector<float>& audio)
    {
        std::vector<double> intervals; double previous=-1;
        for(size_t i=2001;i<audio.size();++i) if(audio[i-1]<=0 && audio[i]>0)
        {
            const auto position=double(i-1)-audio[i-1]/double(audio[i]-audio[i-1]);
            if(previous>=0) intervals.push_back(position-previous); previous=position;
        }
        double mean=0,variance=0;
        if(!intervals.empty())
        {
            mean=std::accumulate(intervals.begin(),intervals.end(),0.0)/intervals.size();
            for(double interval:intervals) variance+=(interval-mean)*(interval-mean);
            variance=std::sqrt(variance/intervals.size());
        }
        return std::make_pair(mean,variance);
    };
    {
        auto f=synth(); tsf_channel_note_on(f.get(),0,57,0.8f);
        tsf_channel_midi_control(f.get(),0,64,127); tsf_channel_note_off(f.get(),0,57);
        check(rms(render(f.get()))>0.001 && tsf_active_voice_count(f.get())==1,"sustain holds released notes audibly");
        tsf_channel_note_on(f.get(),0,57,0.8f);
        tsf_channel_midi_control(f.get(),0,64,0); render(f.get());
        check(tsf_active_voice_count(f.get())==1,"pedal release preserves a repeated note that is still held");
        tsf_channel_note_off(f.get(),0,57); render(f.get());
        check(tsf_active_voice_count(f.get())==0,"released notes finish after the pedal is lifted");
    }
    {
        auto f=synth(); tsf_channel_note_on(f.get(),0,57,0.8f);
        const auto reference=periods(render(f.get()));
        tsf_channel_set_pitchwheel(f.get(),0,16383);
        const auto bent=periods(render(f.get()));
        check(reference.first>0 && std::abs(reference.first/bent.first-std::pow(2.0,2.0/12))<0.004,"pitch wheel changes rendered pitch by the default two semitones");
        tsf_channel_midi_control(f.get(),0,101,0); tsf_channel_midi_control(f.get(),0,100,0);
        tsf_channel_midi_control(f.get(),0,6,12); tsf_channel_midi_control(f.get(),0,38,0);
        const auto octave=periods(render(f.get()));
        check(std::abs(reference.first/octave.first-2.0)<0.004,"RPN pitch sensitivity changes the audible bend range");
        tsf_channel_midi_control(f.get(),0,121,0);
        const auto centred=periods(render(f.get()));
        check(std::abs(reference.first/centred.first-1.0)<0.004 && tsf_channel_get_pitchrange(f.get(),0)==12,"controller reset centres pitch without resetting its configured range");
    }
    {
        auto f=synth(); tsf_channel_note_on(f.get(),0,57,0.8f);
        const auto steady=periods(render(f.get()));
        tsf_channel_midi_control(f.get(),0,1,127);
        check(periods(render(f.get())).second>steady.second+0.3,"modulation wheel produces audible vibrato on an existing voice");
        tsf_channel_midi_control(f.get(),0,121,0);
        check(periods(render(f.get())).second<0.1,"controller reset removes modulation");
        tsf_channel_set_pressure(f.get(),0,127);
        check(periods(render(f.get())).second>0.3,"channel pressure changes audible vibrato");
        tsf_channel_set_pressure(f.get(),0,0); tsf_channel_set_key_pressure(f.get(),0,57,127);
        check(periods(render(f.get())).second>0.3,"polyphonic pressure affects the addressed note");
    }
    {
        auto f=synth(); tsf_channel_note_on(f.get(),0,57,0.8f);
        const auto reference=rms(render(f.get()));
        tsf_channel_midi_control(f.get(),0,7,64);
        const auto reduced=rms(render(f.get()));
        check(reduced<reference*0.2 && reduced>reference*0.08,"volume controller changes the rendered level");
        tsf_channel_midi_control(f.get(),0,11,0);
        check(rms(render(f.get()))<0.000001,"expression controller can silence active notes");
        tsf_channel_midi_control(f.get(),0,10,16); tsf_channel_midi_control(f.get(),0,0,1); tsf_channel_midi_control(f.get(),0,32,1);
        const auto volume=tsf_channel_get_volume(f.get(),0),pan=tsf_channel_get_pan(f.get(),0);
        tsf_channel_midi_control(f.get(),0,121,0);
        check(std::abs(rms(render(f.get()))-reduced)<reference*0.01 && tsf_channel_get_pan(f.get(),0)==pan && tsf_channel_get_preset_bank(f.get(),0)==129,
              "controller reset restores expression while retaining volume, pan and selected bank");
        (void)volume;
    }
    {
        auto f=synth(); tsf_channel_note_on(f.get(),0,57,0.8f);
        tsf_channel_midi_control(f.get(),0,64,127); tsf_channel_midi_control(f.get(),0,123,0);
        check(rms(render(f.get()))>0.001,"All Notes Off honors a held sustain pedal");
        tsf_channel_midi_control(f.get(),0,120,0);
        check(tsf_active_voice_count(f.get())==0 && rms(render(f.get()))==0,"All Sound Off silences sustained voices immediately");
        tsf_channel_note_on(f.get(),0,57,0.8f); tsf_channel_note_off(f.get(),0,57);
        check(rms(render(f.get()))>0.001,"All Sound Off retains pedal state for subsequent notes");
        tsf_channel_midi_control(f.get(),0,121,0); render(f.get());
        check(tsf_active_voice_count(f.get())==0,"controller reset releases pedal-held notes");
    }
}
