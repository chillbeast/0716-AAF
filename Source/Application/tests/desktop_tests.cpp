// SPDX-License-Identifier: AGPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
#include "FixedAudioBlockAdapter.h"
#include "Common/Os/CrossPlatform/CMsgQueue.h"
#include "Common/Modules/anytype.h"
#include "SafeFileIO.h"
#include "SoundFontProgram.h"
#ifdef _WIN32
#include "Common/Board/CHardwareTimer.h"
#endif
#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif
#define TSF_IMPLEMENTATION
#include "tsf.h"

int main(int argc, char** argv)
{
    int failures = 0;
    int checks = 0;
    auto check = [&](bool ok, const char* name)
    {
        ++checks;
        if (!ok) { ++failures; std::cerr << "FAIL: " << name << '\n'; }
    };
    {
        namespace fs = std::filesystem;
        namespace io = Ignite::SafeFileIO;
        const auto root = fs::temp_directory_path() / ("ignite-file-tests-" + io::uniqueSuffix());
        fs::create_directories(root);
        const auto file = root / "saved.txt";
        auto read = [](const fs::path& path) { std::ifstream input(path); return std::string(std::istreambuf_iterator<char>(input), {}); };
        check(io::writeFile(file, [](std::ostream& output) { output << "original"; }), "first atomic save succeeds");
        check(!io::writeFile(file, [](std::ostream& output) { output << "partial"; output.setstate(std::ios::badbit); }) &&
              read(file) == "original", "failed write preserves the previous file");
        check(io::writeFile(file, [](std::ostream& output) { output << "replacement"; }) && read(file) == "replacement",
              "atomic save replaces an existing file");
#ifdef _WIN32
        auto locked = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        check(locked != INVALID_HANDLE_VALUE && !io::writeFile(file, [](std::ostream& output) { output << "blocked"; }) &&
              read(file) == "replacement", "failed file replacement preserves the original");
        if (locked != INVALID_HANDLE_VALUE) CloseHandle(locked);
#endif
        const auto directory = root / "Song";
        const auto recovery = root / "recovery";
        fs::create_directory(directory);
        std::ofstream(directory / "obsolete") << "old";
        check(!io::writeDirectory(directory, recovery, [](const fs::path& pending)
              { std::ofstream(pending / "partial") << "incomplete"; return false; }) && fs::exists(directory / "obsolete"),
              "incomplete song staging preserves the previous directory");
        check(io::writeDirectory(directory, recovery, [](const fs::path& pending)
              { return io::writeFile(pending / "complete", [](std::ostream& output) { output << "new"; }); }) &&
              !fs::exists(directory / "obsolete") && read(directory / "complete") == "new" &&
              fs::exists(recovery / "backups" / "Song" / "obsolete"),
              "complete song replacement removes stale entries and keeps a recovery copy");
        check(!io::isLeafName("../outside") && !io::isLeafName("nested\\folder") && !io::isLeafName("..") &&
              io::isLeafName("My Song"), "save names stay within their library directory");
        fs::remove_all(root); // uniquely created test directory
    }
#ifdef _WIN32
    {
        std::atomic<int> pulses{0};
        Ignite::CHardwareTimer timer([](void* context) { ++*static_cast<std::atomic<int>*>(context); }, &pulses);
        check(!timer.start(0), "clock rejects zero interval");
        check(timer.start(5000), "clock starts");
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        check(pulses >= 8 && pulses <= 80, "clock advances at the requested rate within desktop scheduling tolerance");
        timer.stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // allow an in-flight callback to finish
        const auto paused = pulses.load();
        timer.changeInterval(1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        check(pulses == paused, "tempo change keeps a paused clock paused");
        check(timer.start(1000), "clock resumes");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        check(pulses - paused >= 30, "resumed clock uses the new tempo");
        timer.shutdownDesktop();
        const auto stopped = pulses.load();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        check(pulses == stopped, "shutdown joins the clock and ends callbacks");
    }
#endif
    {
        Ignite::AnyType original(uint8_t(7));
        Ignite::AnyType copied(original);
        check(copied.GetType() == Ignite::MTDataType::UINT8 && copied.get<uint8_t>() == 7,
              "copied messages preserve their numeric type");
        Ignite::AnyType assigned(std::string(128, 'x'));
        assigned = Ignite::AnyType(3.5f);
        check(assigned.GetType() == Ignite::MTDataType::FLOAT32 && assigned.get<float>() == 3.5f,
              "string-to-numeric message assignment preserves type and value");
        assigned = Ignite::AnyType(std::string("restored"));
        check(assigned.get<std::string>() == "restored", "message can change back to a string");
    }
#if defined(_MSC_VER) && defined(_DEBUG)
    {
        _CrtMemState before{}, after{}, difference{};
        _CrtMemCheckpoint(&before);
        for (int i = 0; i < 100; ++i)
        {
            Ignite::AnyType message(std::string(128, 'x'));
            message = Ignite::AnyType(int32_t(i));
        }
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        check(difference.lSizes[_NORMAL_BLOCK] == 0, "message type changes release the previous string allocation");
    }
#endif
    check(Ignite::SoundFontProgram::isCustom(Ignite::SoundFontProgram::encode(42,7)) &&
          Ignite::SoundFontProgram::bank(Ignite::SoundFontProgram::encode(42,7))==42 &&
          Ignite::SoundFontProgram::program(Ignite::SoundFontProgram::encode(42,7))==7,"custom SoundFont bank and program roundtrip");
    check(!Ignite::SoundFontProgram::isCustom(137) && Ignite::SoundFontProgram::bank(Ignite::SoundFontProgram::encode(65535,127))==65535,
          "legacy programs and full SoundFont bank range remain distinct");
    Ignite::FixedAudioBlockAdapter<960> adapter;
    int rendered = 0;
    int consumed = 0;
    auto render = [&](float* block) { for (int i = 0; i < 960; ++i) block[i] = static_cast<float>(rendered++); };
    for (int count : {0, 1, 64, 128, 256, 512, 960, 1024, 2048, 17})
    {
        std::vector<float> left(count + 2, -999.0f), right(count + 2, -999.0f);
        float* out[]{left.data() + 1, nullptr, right.data() + 1};
        adapter.process(out, 3, count, render);
        check(left.front() == -999 && left.back() == -999 && right.front() == -999 && right.back() == -999,
              "driver buffer guards intact");
        bool continuous = true;
        for (int i = 0; i < count; ++i)
            continuous &= left[i + 1] == static_cast<float>(consumed + i) && right[i + 1] == left[i + 1];
        check(continuous, "continuous samples across callbacks and identical stereo output");
        consumed += count;
    }
    adapter.reset();
    float sample = -1; float* output[]{ &sample };
    adapter.process(output, 1, 1, render);
    check(sample == static_cast<float>(rendered - 960), "device restart discards old partial block");
    check(tsf_load_filename("missing-font-for-ignite-test.sf2") == nullptr, "missing SoundFont is reported");
    Ignite::CMsgQueue<int> queue(1);
    int value = 0;
    check(!queue.get(value, Ignite::TimeOut::kNoWait), "empty queue read returns without blocking");
    check(queue.put(42, Ignite::TimeOut::kNoWait), "queue accepts item");
    check(!queue.put(43, Ignite::TimeOut::kNoWait), "full queue rejects item without blocking");
    check(queue.get(value, Ignite::TimeOut::kNoWait) && value == 42, "queue preserves item");
    tsf* font = argc > 1 ? tsf_load_filename(argv[1]) : nullptr;
    check(font != nullptr, "generated SoundFont loads in firmware TinySoundFont");
    if (font)
    {
        check(tsf_get_presetcount(font) == 1, "test preset found");
        check(tsf_get_presetbank(font,0)==0 && tsf_get_presetnumber(font,0)==0,"SoundFont metadata identifies bank and program");
        check(tsf_get_presetbank(font,-1)==-1 && tsf_get_presetnumber(font,2)==-1,"invalid preset metadata lookups are rejected");
        tsf_set_output(font, TSF_MONO, 48000, 0);
        tsf_set_max_voices(font, 8);
        tsf* track = tsf_copy(font);
        check(track != nullptr, "per-track SoundFont copy");
        if (track)
        {
            tsf_set_output(track, TSF_MONO, 48000, 0);
            tsf_set_max_voices(track, 8);
            tsf_channel_set_presetindex(track, 0, 0);
            tsf_channel_note_on(track, 0, 57, 0.8f);
            float block[960]{};
            tsf_render_float(track, block, 960, 0);
            float peak = 0;
            bool finite = true;
            for (auto value : block) { peak = std::max(peak, std::abs(value)); finite &= std::isfinite(value); }
            check(finite && peak > 0.001f, "firmware synth renders finite non-silent audio");
            tsf_channel_note_off(track, 0, 57);
            for (int i = 0; i < 200; ++i) tsf_render_float(track, block, 960, 0);
            check(tsf_active_voice_count(track) == 0, "note release ends the voice");
            tsf_close(track);
            check(tsf_get_presetcount(font) == 1, "closing track keeps shared font alive");
        }
        tsf_close(font);
    }
    std::atomic<bool> stopped{false};
    std::thread waiter([&] { int item = 0; stopped = !queue.get(item, Ignite::TimeOut::kWaitForever); });
    Ignite::CThread::requestDesktopShutdown();
    waiter.join();
    check(stopped.load(), "desktop shutdown releases blocked queue readers");
    std::cout << (checks - failures) << '/' << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
