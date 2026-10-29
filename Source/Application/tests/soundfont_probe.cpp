// SPDX-License-Identifier: AGPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#define TSF_IMPLEMENTATION
#include "SoundFontRuntime.h"
using namespace Ignite;

int main(int argc, char** argv)
{
    if (argc != 3) { std::cerr << "Usage: IgniteSoundFontProbe bank.sf2 output.wav\n"; return 2; }
    std::ifstream input(argv[1], std::ios::binary);
    tsf_stream source{&input, [](void* p, void* b, unsigned n) { auto& f=*static_cast<std::ifstream*>(p); f.read(static_cast<char*>(b),n); return int(f.gcount()); }, [](void* p,unsigned n) { auto& f=*static_cast<std::ifstream*>(p); f.seekg(n,std::ios::cur); return f.good()?1:0; }};
    auto* fontBank = SF2::load(&source,argv[1]);
    auto* font = SF2::copy(fontBank);
    if (!font) { SF2::closeBank(fontBank); std::cerr << "Cannot load SoundFont\n"; return 1; }
    SF2::setOutput(font, TSF_STEREO_UNWEAVED, 48000, -6);
    SF2::setVolume(font,0,0.45f);
    SF2::setVoices(font, 8);
    int gm = 0, drums = 0;
    for (int p = 0; p < 128; ++p)
    {
        gm += SF2::index(fontBank, 0, p) >= 0;
        drums += SF2::index(fontBank, 128, p) >= 0;
    }
    std::cout << "Presets=" << SF2::count(fontBank) << " GM=" << gm << "/128 Drums=" << drums << '\n';
    std::vector<float> audio;
    int nonSilent = 0;
    for (auto program : {0, 33, 48, 80, 128})
    {
        const int bank = program == 128 ? 128 : 0;
        const int preset = program == 128 ? 0 : program;
        const int index = SF2::index(fontBank, bank, preset);
        if (index < 0) continue;
        SF2::reset(font);
        SF2::setBankPreset(font, 0, bank, preset);
        SF2::noteOn(font, 0, bank == 128 ? 36 : 60, 0.8f);
        float peak = 0;
        for (int block = 0; block < 40; ++block)
        {
            if (block == 20) SF2::allNotesOff(font, 0);
            float samples[1920]{};
            SF2::render(font, samples, 960, 0);
            for (int i=0;i<960;++i) for(int channel=0;channel<2;++channel)
            {
                const auto sample=samples[channel*960+i];
                if (!std::isfinite(sample)) { SF2::close(font); SF2::closeBank(fontBank); return 1; }
                peak = std::max(peak, std::abs(sample));
                audio.push_back(sample);
            }
        }
        std::cout << "Bank=" << bank << " Program=" << preset << " Name=" << SF2::name(fontBank, index)
                  << " Peak=" << peak << '\n';
        nonSilent += peak > 0.001f;
    }
    SF2::close(font); SF2::closeBank(fontBank);
    std::ofstream stream(argv[2], std::ios::binary);
    auto u16 = [&](uint16_t value) { char b[]{char(value & 255), char(value >> 8)}; stream.write(b, 2); };
    auto u32 = [&](uint32_t value) { u16(uint16_t(value)); u16(uint16_t(value >> 16)); };
    stream.write("RIFF", 4); u32(36 + uint32_t(audio.size()) * 2); stream.write("WAVEfmt ", 8);
    u32(16); u16(1); u16(2); u32(48000); u32(192000); u16(4); u16(16);
    stream.write("data", 4); u32(uint32_t(audio.size()) * 2);
    for (auto sample : audio) u16(static_cast<uint16_t>(static_cast<int16_t>(std::clamp(sample, -1.0f, 1.0f) * 32767)));
    std::cout << "Non-silent programs=" << nonSilent << " Samples=" << audio.size() << '\n';
    return stream.good() && nonSilent > 0 ? 0 : 1;
}
