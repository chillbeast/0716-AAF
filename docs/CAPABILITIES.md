# IGNITE capability map

Baseline: upstream `0371a3ea22fc530e23e2321068a0b8835e1e6502`, application version **0.3.3**, inspected 2026-10-28. This distinguishes source capabilities from tested behavior; it is not a claim that every hardware feature has been validated.

The desktop simulator offers **Compact** and **Hardware** layouts. Compact gives the live firmware touchscreen most of the window, with all 69 original panel controls retained. The toolbar switches layouts and remembers the choice; startup sizing fits the monitor work area. See the [manual](manual/USER_MANUAL.md).

## Sequencing

| Capability | Source evidence / interpretation |
|---|---|
| Song → scene → track → pattern | `Common/Modules/{CSong,Scene,Track,Pattern}` and firmware managers implement the hierarchy. Patterns and instruments also have independent file libraries. |
| Up to 64 tracks | `SongManager` registers track-count 0–64; mixer indices are 0–63. New songs default to 16 tracks. This does not mean 64 MIDI channels on one port. |
| 16 steps/bar; 16 bars/pattern | `ModelParams.h`: `MAX_STEPS_IN_PATTERN_BAR`, `MAX_NUM_OF_BARS_IN_PATTERN`; up to 256 step positions. |
| Scene length up to 200 bars | `MAX_SCENE_LEN_IN_NUM_BARS`; scene gBar up to 16 steps. |
| 96 PPQN | `SET_PPQN`. Pattern timing supports straight and triplet time bases from 1 to 32T. |
| Tempo 20–360 BPM | Registered song parameter; some older UI constants have different ranges, so do not infer all limits from one constants file. |
| Notes, velocity, length, tick offsets, auxiliary data | `Step`, `Bar`, `Pattern`, `SequenceManager` and the list/step/grid editors. |
| Real-time recording and event filtering | Step and live recording pass internal MIDI tests, including channel filtering, velocity/duration and persistence. Physical input remains unverified. |
| Main/fill patterns, solo/skip, gang, sculpt | Dedicated front-panel/UI paths. Sculpt data includes relative, absolute, random-relative and random-absolute note/velocity operations. |
| Transpose and scales | Built-in scale map, per-instrument transforms, transpose control, ten user-defined scales with twelve nodes each. |
| Undo | Constants allocate 50 general, 16 pattern and 16 settings undo entries; individual operations still need coverage. |

## Internal sound engine

The running application uses a modified **TinySoundFont** implementation in `modules/Firmware/Application/Model/tsf.h`, instantiated by `SongManager` and copied by `Common/Modules/Track.cpp`. It does **not** link the bundled `AudIO` library in its main target. The older `CSoundFontEngine` wrapper is compiled as stubs; finding a feature in AudIO does not establish that IGNITE exposes it.

| Property | Current source behavior |
|---|---|
| Bank format | SoundFont 2 (`.sf2`), loaded from a file. SF3 support is not established. |
| Voice limit | Windows default: 32 voices per track, configurable to 8/16/32/64/128. Presets can use multiple voices per note; maximum aggregate polyphony is not a real-time performance guarantee. |
| Audio format | Windows stereo synthesis/effects at 48 kHz, 32-bit float, resampled to the device rate. Stereo channel histories remain independent; mono output uses a downmix. |
| Render quantum | Live rendering uses 960 samples / 20 ms. The desktop adapter preserves samples across arbitrary driver buffer sizes; offline export also supports shorter blocks for exact timing. |
| Program selection | Original GM programs plus a desktop browser enumerating actual preset names, banks and programs, with preview and saved custom assignments. A default SF2 and per-instrument SF2 assignments coexist, with sample data shared by path. |
| Controls | Release, cutoff, resonance, volume, reverb, chorus, EQ high and EQ low; alternate functions and track-value transforms are present. |
| Effects | JUCE reverb, chorus, EQ/filter/gain processing in `Track`; five effect-order slots. Track effect values save with songs and restore after reopening/synth reconstruction. The much larger Airwindows collection in AudIO is not automatically available in the IGNITE UI. |
| Missing bank | This branch disables internal sound safely; the public baseline dereferenced failed font loads. |

The local library scan found **1,094 SF2 files**. `CT4MGM.SF2` has **329 presets**, all **128 GM bank-0 programs**, and **10 bank-128 drum presets**. A native probe rendered piano, fingered bass, strings, square lead and a standard-kit kick to finite, non-silent audio. The bank is loaded from the user's sample library and is not included in the repository.

## MIDI, CV and system features

| Feature | Source support and test boundary |
|---|---|
| DIN MIDI | Five input/output pairs handled via the MCU. Requires hardware. |
| USB device MIDI | Six logical device MIDI ports in the source, despite one physical USB-C connector. |
| USB host MIDI | Five host-device slots. Device hotplug integration is Linux-specific. |
| Routing | Instrument input/output ports and channels, thru, program/bank messages and track-value CC mapping. Desktop settings map Windows devices onto 16 logical MIDI input/output ports. Internal routing is tested; physical drivers/endpoints remain unverified. |
| Clock / transport | Internal clock, MIDI clock input/output, sync port, MIDI Machine Control and remote control settings. Desktop timing has not been benchmarked against external equipment. |
| CV/Gate | Eight CV and eight Gate channels, calibration and envelope-related code. No desktop electrical emulation or voltage validation. |
| Wi-Fi / PlayTribe | Settings, connection manager and transfer-related application code exist; the proprietary Linux `pcmode` service was omitted. End-to-end transfer is not established. |
| Updates | Linux SWUpdate/image scripts and application update UI. MCU hex is absent. No update or flashing tests performed. |

## Useful next investigations

1. Benchmark sustained polyphony and audio/timing under heavy load. Deterministic create/edit/save/reopen, device audio and playback resizing through 64 tracks now have regression coverage; see [AUDIT_FIXES.md](AUDIT_FIXES.md).
2. Verify live desktop MIDI routing with a loopback port and a controller.
3. Exercise more varied SoundFonts and sustained preset preview/monitoring alongside editing. The bank-aware browser and saved non-GM assignments now have regression coverage.
4. Extend coverage to detached MIDI tasks, hardware-specific writers and full singleton UI ownership. Step/live recording, ZIP backup round trips and firmware backup/restore now have internal regression coverage; see [DESKTOP_FEATURES.md](DESKTOP_FEATURES.md).
5. Establish hardware acceptance tests for the MCU, CV/Gate, sync, hotplug, calibration and update paths.

Source paths above are relative to [`Source/Application/modules`](../Source/Application/modules).

The desktop.3 pass adds MIDI controller support, checked settings saves and a close prompt, step insertion, and editor/performance regressions. See [AUDIT_FIXES.md](AUDIT_FIXES.md) for the specific fixes and verified limits.

Desktop.4 adds stereo/effects, configurable polyphony and persistent per-instrument SF2 assignments.

## Desktop file interchange (desktop.5)

| Operation | Implemented behavior / boundary |
|---|---|
| WAV / stems | Current song or scene once, full mix or one track, stereo 48 kHz float32 with effects and a final two-second tail. Internal instruments only; required SF2s stay external. Maximum 30-minute arrangement. |
| MIDI export | Type 1 SMF, 960 PPQ; scene tempos/markers, notes, supported controllers/pressure/pitch bend, bank/program setup and logical-port metadata. Routing may need adjustment in another app. |
| MIDI clip import | Additive library patterns by source track/channel, up to 16 bars each, fixed 4/4 sixteenth grid with 48 ticks/quarter resolution. Current song/tempo/assignments are unchanged. Arranged song import is available in desktop.6 below. |
| Export isolation | Snapshot includes unsaved edits; a child engine renders without live clock, audio-device or MIDI access. Cancellation, child failure and locked destination preserve an existing output. |
| Performance boundary | Main patterns only; live fill/Sculpt/input actions are not captured. Random directions generate a fresh pass. Scene boundaries release notes; only the final scene has a WAV tail. |

Complete SoundFont modulator compatibility, a working Ignite VST and physical MIDI/CV/clock qualification remain outstanding.

## Arranged MIDI songs (desktop.6)

Type 0/1 PPQ MIDI imports as a new song, with assigned named parts, tempo/meter/marker scenes, bank/program selections and GM percussion. Existing work must be saved before switching; existing songs are retained. Imported patterns and instruments save through normal Save All. Fractional scene BPM now survives reopening, and MIDI export includes the saved time signatures.

Notes/events round to 48 ticks/quarter; scene boundaries round to sixteenths and crossing notes retrigger. Meters that exceed the global-bar model retain MIDI metadata with a normalised internal bar grid. SysEx, SMPTE and type 2 independent sequences are not imported. Resource and scene/library limits are validated before song creation. The 103-check setup/song/round-trip/reopen suite covers generated MIDI/SF2 fixtures and actual exported MIDI/audio; see [DEVELOPMENT.md](DEVELOPMENT.md).
