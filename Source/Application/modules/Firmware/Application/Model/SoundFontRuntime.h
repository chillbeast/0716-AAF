// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "tsf.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#if IGNITE_FULL_SF2
#include <fluidsynth.h>
// Reset hooks from the pinned static engine (2.6.1). These clear DSP histories
// without a MIDI system reset, which would discard RPN/NRPN/channel state.
extern "C" int fluid_synth_reset_reverb(fluid_synth_t*);
extern "C" int fluid_synth_reset_chorus(fluid_synth_t*);
#endif

namespace Ignite::SF2
{
#if IGNITE_FULL_SF2
    struct Settings
    {
        fluid_settings_t* value=new_fluid_settings();
        Settings()
        {
            if(!value)throw std::bad_alloc();
            fluid_settings_setnum(value,"synth.sample-rate",48000);
            fluid_settings_setnum(value,"synth.gain",0.9);
            fluid_settings_setint(value,"synth.polyphony",32);
            fluid_settings_setint(value,"synth.midi-channels",16);
            fluid_settings_setint(value,"synth.cpu-cores",1);
            fluid_settings_setint(value,"synth.threadsafe-api",0);
            fluid_settings_setint(value,"synth.dynamic-sample-loading",0);
        }
        ~Settings(){delete_fluid_settings(value);}
        Settings(const Settings&)=delete;
    };
    struct Preset {int bank,program;std::string name;};
    struct Bank
    {
        Settings settings;
        fluid_synth_t* owner=nullptr;
        fluid_sfont_t* font=nullptr;
        std::vector<Preset> presets;
        ~Bank(){if(owner)delete_fluid_synth(owner);}
    };
    inline Bank* load(tsf_stream*,const std::string& path)
    {
        auto bank=std::make_unique<Bank>();
        bank->owner=new_fluid_synth(bank->settings.value);if(!bank->owner)return nullptr;
        const int id=fluid_synth_sfload(bank->owner,path.c_str(),0);if(id<0)return nullptr;
        bank->font=fluid_synth_get_sfont_by_id(bank->owner,id);if(!bank->font)return nullptr;
        fluid_sfont_iteration_start(bank->font);
        while(auto* preset=fluid_sfont_iteration_next(bank->font))
            bank->presets.push_back({fluid_preset_get_banknum(preset),fluid_preset_get_num(preset),fluid_preset_get_name(preset)});
        if(bank->presets.empty())return nullptr;
        return bank.release();
    }
    inline void closeBank(Bank* bank){delete bank;}
    inline int count(Bank* bank){return bank?int(bank->presets.size()):0;}
    inline int bankNumber(Bank* bank,int index){return bank->presets.at(size_t(index)).bank;}
    inline int programNumber(Bank* bank,int index){return bank->presets.at(size_t(index)).program;}
    inline const char* name(Bank* bank,int index){return bank->presets.at(size_t(index)).name.c_str();}
    inline int index(Bank* bank,int number,int program)
    {
        if(bank)for(size_t i=0;i<bank->presets.size();++i)
            if(bank->presets[i].bank==number && bank->presets[i].program==program)return int(i);
        return -1;
    }
    inline const char* presetName(Bank* bank,int number,int program)
    {const int i=index(bank,number,program);return i>=0?name(bank,i):nullptr;}

    struct Synth
    {
        Settings settings;
        Bank* bank;
        fluid_synth_t* engine=nullptr;
        int fontId=-1,presetIndex=-1,pendingBank=0,releaseMs=0;
        float gain=0.9f;
        int pressure=0;
        std::array<int,128> keyPressure{};
        struct Release {fluid_voice_t* voice=nullptr;unsigned int id=0;float original=0;};
        std::vector<fluid_voice_t*> voices;
        std::vector<Release> releases;
        explicit Synth(Bank* source):bank(source)
        {
            voices.resize(32);releases.resize(32);
            engine=new_fluid_synth(settings.value);if(!engine)throw std::bad_alloc();
            for(int channel=0;channel<16;++channel)fluid_synth_unset_program(engine,channel);
            // Virtual font proxies share the immutable loaded bank, including its
            // samples and modulators. Track::desktopFontSource owns that bank and
            // outlives this synth. Presets returned by the loader are per-channel.
            auto* proxy=new_fluid_sfont(
                [](fluid_sfont_t* font){return fluid_sfont_get_name(static_cast<Bank*>(fluid_sfont_get_data(font))->font);},
                [](fluid_sfont_t* font,int bank,int program){return fluid_sfont_get_preset(static_cast<Bank*>(fluid_sfont_get_data(font))->font,bank,program);},
                [](fluid_sfont_t*){},[](fluid_sfont_t*)->fluid_preset_t*{return nullptr;},
                [](fluid_sfont_t* font){return delete_fluid_sfont(font);});
            if(!proxy){delete_fluid_synth(engine);engine=nullptr;throw std::bad_alloc();}
            fluid_sfont_set_data(proxy,bank);fontId=fluid_synth_add_sfont(engine,proxy);
            if(fontId<0){delete_fluid_sfont(proxy);delete_fluid_synth(engine);engine=nullptr;throw std::bad_alloc();}
            // Keep unrequested MIDI effect sends dry. SF2 generators/modulators
            // and incoming CC91/93 still drive the engine's reverb/chorus sends.
            fluid_synth_cc(engine,0,91,0);fluid_synth_cc(engine,0,93,0);
        }
        ~Synth(){if(engine)delete_fluid_synth(engine);}
        void updateRelease()
        {
            std::fill(voices.begin(),voices.end(),nullptr);
            fluid_synth_get_voicelist(engine,voices.data(),int(voices.size()),-1);
            for(auto* voice:voices)if(voice)
            {
                const auto id=fluid_voice_get_id(voice);
                auto found=std::find_if(releases.begin(),releases.end(),[&](const auto& entry){return entry.voice==voice && entry.id==id;});
                if(releaseMs>0)
                {
                    if(found==releases.end())
                    {
                        found=std::find_if(releases.begin(),releases.end(),[&](const auto& entry)
                        {return !entry.voice || std::find(voices.begin(),voices.end(),entry.voice)==voices.end() || fluid_voice_get_id(entry.voice)!=entry.id;});
                        if(found==releases.end())continue;
                        *found={voice,id,fluid_voice_gen_get(voice,GEN_VOLENVRELEASE)};
                    }
                    fluid_voice_gen_set(voice,GEN_VOLENVRELEASE,float(1200.0*std::log2(double(releaseMs)/1000.0)));
                }
                else if(found!=releases.end())fluid_voice_gen_set(voice,GEN_VOLENVRELEASE,found->original);
                fluid_voice_update_param(voice,GEN_VOLENVRELEASE);
            }
            if(!releaseMs)std::fill(releases.begin(),releases.end(),Release{});
        }
        void flushReset()
        {
            float left[128]{},right[128]{};
            fluid_synth_write_float(engine,128,left,0,1,right,0,1);
            std::fill(releases.begin(),releases.end(),Release{});
        }
    };
    inline Synth* copy(Bank* bank){if(!bank)return nullptr;try{return new Synth(bank);}catch(...){return nullptr;}}
    inline void close(Synth* synth){delete synth;}
    inline void setOutput(Synth*,TSFOutputMode,int,float){}
    inline void setVoices(Synth* s,int number)
    {if(s){fluid_synth_set_polyphony(s->engine,number);s->voices.resize(size_t(number));s->releases.resize(size_t(number));}}
    inline int activeVoices(Synth* s){return s?fluid_synth_get_active_voice_count(s->engine):0;}
    inline void setVolume(Synth* s,int,float value){s->gain=value;fluid_synth_set_gain(s->engine,value);}
    inline float volume(Synth* s,int){return s->gain;}
    inline int setBankPreset(Synth* s,int,int bank,int program)
    {
        const int i=index(s->bank,bank,program);if(i<0)return 0;
        if(fluid_synth_program_select(s->engine,0,s->fontId,bank,program)!=FLUID_OK)return 0;
        s->pendingBank=bank;s->presetIndex=i;return 1;
    }
    inline int setPresetIndex(Synth* s,int,int i)
    {return s && i>=0 && i<count(s->bank)?setBankPreset(s,0,bankNumber(s->bank,i),programNumber(s->bank,i)):0;}
    inline int presetIndex(Synth* s,int){return s->presetIndex;}
    inline int bank(Synth* s,int){return s->pendingBank;}
    inline void setBank(Synth* s,int,int value)
    {s->pendingBank=value;fluid_synth_cc(s->engine,0,0,(value>>7)&127);fluid_synth_cc(s->engine,0,32,value&127);}
    inline void reset(Synth* s)
    {fluid_synth_all_sounds_off(s->engine,0);s->flushReset();fluid_synth_system_reset(s->engine);s->pressure=0;s->keyPressure.fill(0);s->flushReset();}
    inline void allNotesOff(Synth* s,int){fluid_synth_all_notes_off(s->engine,0);}
    inline void allSoundsOff(Synth* s,int)
    {
        // Drain queued voice termination before clearing the effect histories.
        fluid_synth_all_sounds_off(s->engine,0);s->flushReset();
        fluid_synth_reset_reverb(s->engine);fluid_synth_reset_chorus(s->engine);
        s->flushReset();
    }

    inline void control(Synth* s,int,int cc,int value)
    {
        if(cc==120){allSoundsOff(s,0);return;}
        if(cc==0)s->pendingBank=(s->pendingBank&127)|(value<<7);
        if(cc==32)s->pendingBank=(s->pendingBank&0x3f80)|value;
        if(cc==121){s->pressure=0;s->keyPressure.fill(0);}
        fluid_synth_cc(s->engine,0,cc,value);
    }
    inline void pitchWheel(Synth* s,int,int value){fluid_synth_pitch_bend(s->engine,0,value);}
    inline void pressure(Synth* s,int,int value){s->pressure=value;fluid_synth_channel_pressure(s->engine,0,value);}
    inline void keyPressure(Synth* s,int,int key,int value){s->keyPressure[size_t(key)]=value;fluid_synth_key_pressure(s->engine,0,key,value);}
    inline void noteOn(Synth* s,int,int key,float velocity)
    {fluid_synth_noteon(s->engine,0,key,std::clamp(int(std::lround(velocity*127)),1,127));if(s->releaseMs)s->updateRelease();}
    inline void noteOff(Synth* s,int,int key){fluid_synth_noteoff(s->engine,0,key);}
    inline void releaseTime(Synth* s,int value){s->releaseMs=value;s->updateRelease();}
    inline void render(Synth* s,float* buffer,int frames,int)
    {fluid_synth_write_float(s->engine,frames,buffer,0,1,buffer+frames,0,1);}
#else
    using Bank=tsf;using Synth=tsf;
    inline Bank* load(tsf_stream* stream,const std::string&){return tsf_load(stream);}
    inline void closeBank(Bank* bank){tsf_close(bank);}
    inline int count(Bank* bank){return tsf_get_presetcount(bank);}
    inline int bankNumber(Bank* bank,int i){return tsf_get_presetbank(bank,i);}
    inline int programNumber(Bank* bank,int i){return tsf_get_presetnumber(bank,i);}
    inline const char* name(Bank* bank,int i){return tsf_get_presetname(bank,i);}
    inline int index(Bank* bank,int b,int p){return tsf_get_presetindex(bank,b,p);}
    inline const char* presetName(Bank* bank,int b,int p){return tsf_bank_get_presetname(bank,b,p);}
    inline Synth* copy(Bank* bank){return tsf_copy(bank);}
    inline void close(Synth* s){tsf_close(s);}
    inline void setOutput(Synth* s,TSFOutputMode m,int rate,float gain){tsf_set_output(s,m,rate,gain);}
    inline void setVoices(Synth* s,int n){tsf_set_max_voices(s,n);}
    inline int activeVoices(Synth* s){return tsf_active_voice_count(s);}
    inline void setVolume(Synth* s,int c,float v){tsf_channel_set_volume(s,c,v);}
    inline float volume(Synth* s,int c){return tsf_channel_get_volume(s,c);}
    inline int setBankPreset(Synth* s,int c,int b,int p){return tsf_channel_set_bank_preset(s,c,b,p);}
    inline int setPresetIndex(Synth* s,int c,int p){return tsf_channel_set_presetindex(s,c,p);}
    inline int presetIndex(Synth* s,int c){return tsf_channel_get_preset_index(s,c);}
    inline int bank(Synth* s,int c){return tsf_channel_get_preset_bank(s,c);}
    inline void setBank(Synth* s,int c,int b){tsf_channel_set_bank(s,c,b);}
    inline void reset(Synth* s){tsf_reset(s);}
    inline void allNotesOff(Synth* s,int c){tsf_channel_note_off_all(s,c);}
    inline void allSoundsOff(Synth* s,int c){tsf_channel_sounds_off_all(s,c);}
    inline void control(Synth* s,int c,int cc,int v){tsf_channel_midi_control(s,c,cc,v);}
    inline void pitchWheel(Synth* s,int c,int v){tsf_channel_set_pitchwheel(s,c,v);}
    inline void pressure(Synth* s,int c,int v){tsf_channel_set_pressure(s,c,v);}
    inline void keyPressure(Synth* s,int c,int k,int v){tsf_channel_set_key_pressure(s,c,k,v);}
    inline void noteOn(Synth* s,int c,int k,float v){tsf_channel_note_on(s,c,k,v);}
    inline void noteOff(Synth* s,int c,int k){tsf_channel_note_off(s,c,k);}
    inline void releaseTime(Synth* s,int v){tsf_set_noteReleaseTimeMs(s,v);}
    inline void render(Synth* s,float* out,int n,int mix){tsf_render_float(s,out,n,mix);}
#endif
}
