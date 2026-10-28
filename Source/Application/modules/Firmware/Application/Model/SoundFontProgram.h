// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
namespace Ignite::SoundFontProgram
{
    constexpr int customFlag = 0x40000000;
    constexpr int encode(int bank, int program) { return customFlag | ((bank & 0xffff) << 7) | (program & 127); }
    constexpr bool isCustom(int value) { return value >= 0 && (value & customFlag) != 0; }
    constexpr int bank(int value) { return (value >> 7) & 0xffff; }
    constexpr int program(int value) { return value & 127; }
}
