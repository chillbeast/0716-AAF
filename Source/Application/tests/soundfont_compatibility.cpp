// SPDX-License-Identifier: AGPL-3.0-or-later
#include "SoundFontRuntime.h"
#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>

using namespace Ignite;
namespace
{
    int checks=0,failures=0;
    void check(bool passed,const char* name)
    {++checks;if(!passed)++failures;std::cout<<(passed?"PASS: ":"FAIL: ")<<name<<'\n';}
    using SynthPtr=std::unique_ptr<SF2::Synth,decltype(&SF2::close)>;
    struct Audio
    {
        std::vector<float> data;
        int frames;
        explicit Audio(int count):data(size_t(count)*2),frames(count){}
        double rms(int channel=0) const
        {
            double sum=0;for(int i=0;i<frames;++i){const double sample=data[size_t(channel*frames+i)];sum+=sample*sample;}
            return std::sqrt(sum/frames);
        }
        double frequency(int channel=0) const
        {
            int count=0;for(int i=1;i<frames;++i)if(data[size_t(channel*frames+i-1)]<=0 && data[size_t(channel*frames+i)]>0)++count;
            return count*48000.0/frames;
        }
        double difference() const
        {double sum=0;for(int i=0;i<frames;++i){const double d=data[size_t(i)]-data[size_t(i+frames)];sum+=d*d;}return std::sqrt(sum/frames);}
        bool finite() const {return std::all_of(data.begin(),data.end(),[](float v){return std::isfinite(v);});}
    };
    Audio render(SF2::Synth* synth,int frames=16384)
    {Audio result(frames);SF2::render(synth,result.data.data(),frames,0);return result;}
    SynthPtr create(SF2::Bank* bank,int program)
    {
        SynthPtr result(SF2::copy(bank),SF2::close);
        if(!result || !SF2::setBankPreset(result.get(),0,0,program))throw std::runtime_error("Cannot create a test synth/preset");
        return result;
    }
    Audio tone(SF2::Bank* bank,int program,std::initializer_list<std::pair<int,int>> cc={},float velocity=1.0f)
    {
        auto synth=create(bank,program);for(const auto& value:cc)SF2::control(synth.get(),0,value.first,value.second);
        SF2::noteOn(synth.get(),0,69,velocity);render(synth.get(),4800);return render(synth.get());
    }
}
int main(int argc,char** argv)
{
    try
    {
        if(argc<2)throw std::runtime_error("Expected fixture directory");
        const std::filesystem::path root=argv[1];
        std::unique_ptr<SF2::Bank,decltype(&SF2::closeBank)> matrix(SF2::load(nullptr,(root/"matrix.sf2").u8string()),SF2::closeBank);
        if(!matrix)throw std::runtime_error("Cannot load original SF2 matrix fixture");
        check(SF2::count(matrix.get())==19,"all original matrix presets enumerate");
        const auto baseline=tone(matrix.get(),0);
        check(baseline.finite() && baseline.rms()>0.001,"baseline SF2 renders finite audible audio");
        check(std::abs(baseline.frequency()-440)<5,"baseline sample pitch is preserved");
        check(tone(matrix.get(),0,{{11,0}}).rms()<baseline.rms()*0.0001,"standard expression modulator provides at least 80 dB attenuation at zero");
        auto difference=[](const Audio& a,const Audio& b)
        {double sum=0;for(size_t i=0;i<a.data.size();++i){const double d=a.data[i]-b.data[i];sum+=d*d;}return std::sqrt(sum/a.data.size());};
        check(difference(tone(matrix.get(),0,{{1,127}}),baseline)>0.001,"standard mod-wheel vibrato modulator affects the rendered voice");
        check(difference(tone(matrix.get(),18,{{1,0}}),tone(matrix.get(),18,{{1,127}}))<0.000001,"bank can disable standard mod-wheel vibrato with a zero-amount override");
        const auto pitch=tone(matrix.get(),1,{{21,127}});
        check(pitch.frequency()>baseline.frequency()*1.95 && pitch.frequency()<baseline.frequency()*2.05,"instrument CC modulator changes pitch by an octave");
        const auto noAmount=tone(matrix.get(),2,{{22,127},{23,0}}),withAmount=tone(matrix.get(),2,{{22,127},{23,127}});
        check(noAmount.rms(0)>noAmount.rms(1)*0.9 && noAmount.rms(1)>noAmount.rms(0)*0.9,"zero amount-source leaves the modulator neutral");
        check(withAmount.rms(1)>withAmount.rms(0)*20,"amount-source controller scales the pan modulator");
        const auto soft=tone(matrix.get(),0,{},32.0f/127),overrideSoft=tone(matrix.get(),3,{},32.0f/127);
        check(overrideSoft.rms()>soft.rms()*4,"a zero-amount instrument modulator overrides the default velocity attenuation");
        const auto local=tone(matrix.get(),4,{{20,127}}),add=tone(matrix.get(),5,{{20,127}}),global=tone(matrix.get(),14,{{20,127}}),presetLocal=tone(matrix.get(),15,{{20,127}});
        check(global.rms()>local.rms()*1.5,"instrument local modulator replaces an identical global modulator");
        check(std::abs(local.rms()/add.rms()-1)<0.05,"preset and instrument modulator amounts add");
        check(std::abs(presetLocal.rms()/add.rms()-1)<0.05,"preset local modulator replaces the preset global amount");
        const auto negativeZero=tone(matrix.get(),6,{{22,0}}),negativeFull=tone(matrix.get(),6,{{22,127}});
        check(negativeZero.rms(1)>negativeZero.rms(0)*20 && std::abs(negativeFull.rms(0)/negativeFull.rms(1)-1)<0.05,"negative source direction reverses controller response");
        const auto bipolarLeft=tone(matrix.get(),7,{{22,0}}),bipolarRight=tone(matrix.get(),7,{{22,127}});
        check(bipolarLeft.rms(0)>bipolarLeft.rms(1)*20 && bipolarRight.rms(1)>bipolarRight.rms(0)*20,"bipolar source spans both pan directions");
        const auto linear=tone(matrix.get(),8,{{24,64}}),concave=tone(matrix.get(),9,{{24,64}}),convex=tone(matrix.get(),10,{{24,64}});
        check(concave.rms()>linear.rms()*2 && linear.rms()>convex.rms()*2,"linear, concave and convex source curves have distinct SF2 responses");
        const auto switchLow=tone(matrix.get(),11,{{24,20}}),switchHigh=tone(matrix.get(),11,{{24,100}});
        check(switchLow.rms()>switchHigh.rms()*10,"switch source curve changes state across its threshold");
        auto pressure=create(matrix.get(),16);SF2::noteOn(pressure.get(),0,69,1);SF2::keyPressure(pressure.get(),0,69,127);render(pressure.get(),4800);const auto keyPressure=render(pressure.get());
        check(keyPressure.rms(1)>keyPressure.rms(0)*20,"polyphonic pressure drives a custom SoundFont pan modulator");
        auto channel=create(matrix.get(),17);SF2::pressure(channel.get(),0,127);SF2::noteOn(channel.get(),0,69,1);render(channel.get(),4800);const auto channelPressure=render(channel.get());
        check(channelPressure.frequency()>baseline.frequency()*1.95 && channelPressure.frequency()<baseline.frequency()*2.05,"channel pressure drives custom pitch modulation");
        auto reverb=create(matrix.get(),12);SF2::noteOn(reverb.get(),0,69,1);render(reverb.get(),48000);SF2::noteOff(reverb.get(),0,69);render(reverb.get(),48000);const auto tail=render(reverb.get());
        check(tail.finite() && tail.rms()>0.000001,"SoundFont reverb-send generator produces a release tail");
        const auto chorus=tone(matrix.get(),13);
        check(chorus.finite() && chorus.difference()>baseline.difference()+0.0001,"SoundFont chorus-send generator produces stereo modulation");
        auto ccSend=create(matrix.get(),0);SF2::control(ccSend.get(),0,91,127);SF2::noteOn(ccSend.get(),0,69,1);render(ccSend.get(),48000);SF2::noteOff(ccSend.get(),0,69);render(ccSend.get(),48000);
        check(render(ccSend.get()).rms()>0.000001,"default MIDI reverb-send modulator responds to CC91");
        SF2::allSoundsOff(reverb.get(),0);const auto silence=render(reverb.get());
        check(silence.rms()<0.000001,"All Sound Off clears preset-effect tails");
        auto first=create(matrix.get(),0),second=create(matrix.get(),1);SF2::control(first.get(),0,21,0);SF2::control(second.get(),0,21,127);SF2::noteOn(first.get(),0,69,1);SF2::noteOn(second.get(),0,69,1);render(first.get(),4800);render(second.get(),4800);
        check(render(second.get()).frequency()>render(first.get()).frequency()*1.9,"synths sharing one bank retain independent MIDI and voice state");
        auto controller=create(matrix.get(),0);
        SF2::control(controller.get(),0,101,0);SF2::control(controller.get(),0,100,0);SF2::control(controller.get(),0,6,12);
        SF2::pitchWheel(controller.get(),0,16383);SF2::noteOn(controller.get(),0,69,1);render(controller.get(),4800);
        const auto bent=render(controller.get());
        check(bent.frequency()>baseline.frequency()*1.95,"RPN pitch-bend sensitivity changes the audible bend range");
        SF2::control(controller.get(),0,11,32);render(controller.get(),4800);const auto quiet=render(controller.get());
        SF2::allSoundsOff(controller.get(),0);SF2::noteOn(controller.get(),0,69,1);render(controller.get(),4800);const auto retained=render(controller.get());
        check(std::abs(retained.rms()/quiet.rms()-1)<0.05 && std::abs(retained.frequency()/bent.frequency()-1)<0.02,"All Sound Off retains expression, bend and parameter-controller state for subsequent notes");
        SF2::control(controller.get(),0,121,0);render(controller.get(),4800);const auto restored=render(controller.get());
        check(restored.rms()>quiet.rms()*4 && std::abs(restored.frequency()/baseline.frequency()-1)<0.02,"Reset Controllers restores expression and bend position");
        SF2::allSoundsOff(controller.get(),0);const auto stopped=render(controller.get());
        check(stopped.rms()<0.000001,"All Sound Off immediately silences an active held voice");
        const auto unicodePath=root/std::filesystem::u8path(u8"caf\u00e9-bank.sf2");
        std::filesystem::copy_file(root/"matrix.sf2",unicodePath,std::filesystem::copy_options::overwrite_existing);
        std::unique_ptr<SF2::Bank,decltype(&SF2::closeBank)> unicode(SF2::load(nullptr,unicodePath.u8string()),SF2::closeBank);
        check(unicode && tone(unicode.get(),0).rms()>0.001,"SoundFont loads and sounds from a UTF-8 Windows filename");
        std::unique_ptr<SF2::Bank,decltype(&SF2::closeBank)> low24(SF2::load(nullptr,(root/"low24.sf2").u8string()),SF2::closeBank);
        check(low24 && tone(low24.get(),0).rms()>0.000000001,"SF2 24-bit low sample bytes contribute to floating-point audio");
        std::unique_ptr<SF2::Bank,decltype(&SF2::closeBank)> linked(SF2::load(nullptr,(root/"linked.sf2").u8string()),SF2::closeBank);
        if(linked)
        {
            const auto stereo=tone(linked.get(),0);
            check(stereo.finite() && stereo.frequency(1)>stereo.frequency(0)*1.9,"linked left/right SF2 samples retain independent stereo content");
        }
        else check(false,"linked stereo SoundFont loads");
        std::cout<<checks<<" SoundFont compatibility checks, "<<failures<<" failures\n";
    }
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';++failures;}
    return failures?1:0;
}
