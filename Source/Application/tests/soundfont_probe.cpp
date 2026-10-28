// SPDX-License-Identifier: AGPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#define TSF_IMPLEMENTATION
#include "tsf.h"

int main(int argc, char** argv)
{
    if (argc != 3) { std::cerr << "Usage: IgniteSoundFontProbe bank.sf2 output.wav\n"; return 2; }
    tsf* font = tsf_load_filename(argv[1]);
    if (!font) { std::cerr << "Cannot load SoundFont\n"; return 1; }
    tsf_set_output(font, TSF_MONO, 48000, -6);
    tsf_set_max_voices(font, 8);
    int gm = 0, drums = 0;
    for (int p = 0; p < 128; ++p)
    {
        gm += tsf_get_presetindex(font, 0, p) >= 0;
        drums += tsf_get_presetindex(font, 128, p) >= 0;
    }
    std::cout << "Presets=" << tsf_get_presetcount(font) << " GM=" << gm << "/128 Drums=" << drums << '\n';
    std::vector<float> audio;
    int nonSilent = 0;
    for (auto program : {0, 33, 48, 80, 128})
    {
        const int bank = program == 128 ? 128 : 0;
        const int preset = program == 128 ? 0 : program;
        const int index = tsf_get_presetindex(font, bank, preset);
        if (index < 0) continue;
        tsf_reset(font);
        tsf_channel_set_bank_preset(font, 0, bank, preset);
        tsf_channel_note_on(font, 0, bank == 128 ? 36 : 60, 0.8f);
        float peak = 0;
        for (int block = 0; block < 40; ++block)
        {
            if (block == 20) tsf_channel_note_off_all(font, 0);
            float samples[960]{};
            tsf_render_float(font, samples, 960, 0);
            for (auto sample : samples)
            {
                if (!std::isfinite(sample)) { tsf_close(font); return 1; }
                peak = std::max(peak, std::abs(sample));
                audio.push_back(sample);
            }
        }
        std::cout << "Bank=" << bank << " Program=" << preset << " Name=" << tsf_get_presetname(font, index)
                  << " Peak=" << peak << '\n';
        nonSilent += peak > 0.001f;
    }
    tsf_close(font);
    std::ofstream stream(argv[2], std::ios::binary);
    auto u16 = [&](uint16_t value) { char b[]{char(value & 255), char(value >> 8)}; stream.write(b, 2); };
    auto u32 = [&](uint32_t value) { u16(uint16_t(value)); u16(uint16_t(value >> 16)); };
    stream.write("RIFF", 4); u32(36 + uint32_t(audio.size()) * 2); stream.write("WAVEfmt ", 8);
    u32(16); u16(1); u16(1); u32(48000); u32(96000); u16(2); u16(16);
    stream.write("data", 4); u32(uint32_t(audio.size()) * 2);
    for (auto sample : audio) u16(static_cast<uint16_t>(static_cast<int16_t>(std::clamp(sample, -1.0f, 1.0f) * 32767)));
    std::cout << "Non-silent programs=" << nonSilent << " Samples=" << audio.size() << '\n';
    return stream.good() && nonSilent > 0 ? 0 : 1;
}
