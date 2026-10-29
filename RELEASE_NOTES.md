# IGNITE Desktop v0.3.3-desktop.10

Fixes disabled playback for imported/demo songs and desktop transport reliability.

- Songs with 1-15 tracks now enable Play correctly. Partial groups of 16 round
  up in the firmware UI; the actual arrangement keeps its original track count.
  The five-track First Light demo plays through both the visible Play control
  and Space, including after reopening.
- Play, Pause and Rewind safely ignore a missing song or a song with no scenes.
- A pending Windows rewind is cancelled by Stop, Pause, switching songs or
  shutdown. It can no longer restart playback a second after Stop.
- Rewind's 250 ms button cooldown now yields to the UI instead of blocking it.
  Song readiness checks inspect existing tracks by reference without copying
  their metadata or indexing beyond scene data.
- The demo desktop shortcut opens the saved Song view. Other launches can opt
  in with `Ignite.exe --song` or `scripts/run-ignite.ps1 -Song`.
- Expanded the demo regression to 123 checks, including real firmware buttons,
  panel keyboard input, non-silent device output, fresh-process playback,
  rewind/Stop cancellation, empty-song commands and the startup view. Updated
  manual and corresponding source are included; supply your own SF2 banks.

Release, Debug and AddressSanitizer builds and both native CTest targets pass.
Release and ASan pass 123 demo, 131 MIDI song, 75 feature and 107 device/recovery
workflow checks. Release also passes 107 editor/MIDI correctness and 14 smoke
checks. The demo is verified with both an original generated bank and the local
GM bank. Final sanitizer runs report no address errors.

# IGNITE Desktop v0.3.3-desktop.9

A ready-to-play demo song and a clearer route to hearing the sequencer.

- **Projects > Create demo song** adds, selects and saves **Demo - First Light**,
  preserving existing songs. Set a compatible General MIDI SF2 as default first,
  close Settings after creating the demo, then press Space / PLAY.
- Original 16-bar arrangement at 112 BPM: Intro, Groove, Lift and Outro scenes;
  drums, finger bass, electric piano, marimba arpeggio and square lead; twenty
  assigned patterns. Demo instruments use internal audio without external MIDI
  output. The included CC0 MIDI contains no samples.
- Unsaved edits and missing/incomplete default banks are checked before adding a
  song. An unrelated bank in the browser does not replace the demo's default bank.
- Source users can install a separate Documents demo project and Desktop shortcut
  with `scripts/install-demo-windows.ps1`. Existing project directories are kept.
- Updated manual and Projects screenshot. The new 86-check demo suite covers
  live Windows output, full mix and all five stems, save/reopen and error cases.
  No SoundFont samples or rendered sample-library audio are distributed.

# IGNITE Desktop v0.3.3-desktop.8

Full SF2 synthesis and a third, touchscreen-focused display mode.

- **Touchscreen** in the toolbar hides all 69 panel controls and fits the live
  firmware display to the window. Keyboard and mapped inputs remain available.
  The small mode/settings toolbar lets you return to Hardware or Compact.
  Layout choices persist; `--touchscreen` or launcher `-Layout Touchscreen`
  provides a temporary override.
- Windows synthesis now uses **FluidSynth 2.6.1**: instrument/preset modulators,
  global/local/default overrides, source curves and amount sources, pressure,
  native reverb/chorus sends, linked stereo and 24-bit SF2 samples.
- Shared banks retain independent track voices/controllers, saved per-instrument
  assignments, release controls, polyphony settings and the existing panel effects.
  Engine defaults can change levels/timbre compared with older versions; check
  saved mixes after upgrading. SF3 compressed banks are not enabled.
- Fixed active-voice termination/controller preservation and synth shutdown order
  found during regression testing. Added 31 native SF2 compatibility checks and
  expanded display-mode coverage to 111 checks.
- Updated manual, screenshots and source/license packaging. Release archives include
  corresponding FluidSynth LGPL source and rebuild/relink instructions. Supply
  your own SF2 banks; no sample libraries are included.

Release/Debug/AddressSanitizer builds and native checks pass, including 31 new
SF2 compatibility checks per build. Release and ASan application suites pass:
111 layout, 99 sound, 107 MIDI/save/editor, 131 MIDI song, 93 media, 75 features
and 107 workflow checks. The packaged engine source rebuilds and passes its tests.

# IGNITE Desktop v0.3.3-desktop.7

MIDI sustain, channel-mode and routing fidelity fixes.

- Pedal-held notes are recreated across imported scene boundaries until their pedal release. Physical key releases retain their original timing. Envelopes still retrigger at scene changes.
- All Notes Off and channel-mode note releases respect sustain. All Sound Off and Reset Controllers release pedal-held voices appropriately. Later physical note-offs do not revive terminated notes in another scene.
- RPN/NRPN selector/data-entry sequences retain their order across scenes, including pitch-bend range, deselection and Reset Controllers behavior.
- External-only MIDI parts retain logical port metadata even without program initialization. Unrouted internal parts use USB device 1 metadata. All 16 MIDI routes retain their actual firmware IDs (DIN 0-4, USB 13-23); CV IDs 5-12 and unsupported imported ports produce a remapping error instead of silent rerouting.
- Clip import preserves overlapping same-pitch note pairing. Both import paths reject zero tempo and malformed port metadata before adding patterns or songs.
- Updated manual and audio/MIDI regression coverage. Re-import earlier MIDI sources to apply the new note conversion.

Release/Debug/AddressSanitizer builds succeed. The MIDI song suite passes 131 checks and media passes 93 in Release and ASan, alongside the sound, correctness, feature, layout and device/recovery regressions. Native checks remain 84 Release/ASan and 85 Debug; Release font/no-font smoke and ASan no-font workflow pass. Counts include repeated setup. Final sanitizer runs produce no address-error report.

Download **Ignite-Desktop-v0.3.3-desktop.7-windows-x64.zip**, extract it and run **Ignite.exe**. The illustrated manual and corresponding source are included; supply your own SF2 files. Scene boundaries still round to sixteenths; release/effect tails are not continuous across scenes. SysEx, complete SF2 modulators, an Ignite VST and physical MIDI/CV/clock qualification remain outside this community preview.

# IGNITE Desktop v0.3.3-desktop.6

Import arranged MIDI songs with tempo and meter changes.

- **Projects → Import MIDI song...** opens type 0/1 PPQ MIDI as a new song with named tracks, assigned patterns/internal instruments, scene tempos, time signatures, marker names and bank/program selections. Channel 10 defaults to SF2 percussion bank 128.
- Save current edits and stop playback before importing. Existing songs stay available; save the imported song and its new library items with normal Save All. MIDI clip import remains available independently.
- MIDI export now includes saved time signatures. Fractional scene tempos survive reopening, and song/scene XML retains float precision.
- Carries controller/bend state into later scenes, retains silent parts and trailing duration, and reports conversion adjustments. Tempo/meter/marker boundaries round to sixteenths; note/event timing uses 48 ticks/quarter. Notes held across scenes retrigger. Long/fine meters keep their MIDI signature with a normalised internal global-bar grid. SysEx is omitted.
- Validates file/event/part/scene/pattern/library limits before song creation, rejects type 2/SMPTE and bad tempos, and checks malformed tempo/meter payloads before JUCE reads them. Clip import shares the metadata checks.
- Updated illustrated manual and generated MIDI/stereo-SF2 tests covering MIDI/WAV round trips, fractional tempo and meter persistence, drum banks, program changes, unsaved work and retention of previous songs.

Release, Debug and AddressSanitizer builds succeed. The 103-check MIDI song suite passes in Release and ASan, alongside the 88 media, 99 sound, 107 MIDI/save/editor, 75 desktop feature, 61 layout and 107 device/recovery checks. Native checks remain 84 Release/ASan and 85 Debug; font/no-font smoke passes in Release. Counts include repeated setup. Final sanitizer runs produce no address-error report.

Download **Ignite-Desktop-v0.3.3-desktop.6-windows-x64.zip**, extract it and run **Ignite.exe**. The manual and corresponding source are included; supply your own SF2 files. Complete SF2 modulators, a working Ignite VST and physical MIDI/CV/clock qualification remain outstanding. This is a community preview.

# IGNITE Desktop v0.3.3-desktop.5

WAV/stem export, MIDI file interchange and saved track effects.

- Projects exports the current song or scene once as stereo 48 kHz, 32-bit floating-point WAV, with effects and a two-second final tail. Choose the full mix or one track for stems. Required SoundFonts stay external.
- Export type 1 MIDI at 960 PPQ with scene tempos/markers, notes, supported controllers/pressure, and bank/program setup. Export works without MIDI devices or a live audio output.
- Import PPQ-timed MIDI as new pattern-library clips grouped by source track/channel, in chunks of up to 16 bars. Assign clips to tracks and save them. The current song, tempo and assignments are unchanged; complete arrangement/tempo-map import is not included.
- Exports snapshot current unsaved edits in an isolated background process. Keep editing while it renders. Cancellation, render failure and locked destinations preserve an existing output file. Closing Desktop Settings cancels an active export.
- Track effects now save with songs and restore after reopening or synth reconstruction. Corrected controller transposition and notes being cut at every bar boundary.
- Updated illustrated HTML/PDF manual, Projects controls and reproducible media tests covering stereo duration, multi-scene tempos, unsaved snapshots, clip persistence, track effects and output failure/cancellation.

Release, Debug and AddressSanitizer builds succeed. Verification includes 88 media/setup/reopen checks, 84 native Release/ASan checks (85 Debug), 99 sound, 107 MIDI/save/editor, 75 desktop feature, 61 layout, 107 device/recovery workflow and 14 font/no-font smoke checks. Media, sound, correctness, feature, layout and workflow suites also pass under ASan, with a further 91-check missing-font workflow. Counts include repeated setup; final sanitizer runs produce no address-error report.

Download **Ignite-Desktop-v0.3.3-desktop.5-windows-x64.zip**, extract it and run **Ignite.exe**. Open **Desktop Settings → Projects** for the new controls. The package includes the illustrated manual and corresponding source; supply your own SF2 files. Exports cover main patterns, not live fill/Sculpt/input performances; random directions generate a fresh pass. Physical MIDI/CV/clock qualification, complete SF2 modulators and an Ignite VST remain outstanding. This is a community preview.

# IGNITE Desktop v0.3.3-desktop.4

Stereo sound, configurable voice limits and separate SoundFonts for each instrument.

- Windows synthesis, mixing, effects, resampling and output preserve stereo. MIDI pan is audible; mono devices receive a downmix.
- Choose 8/16/32/64/128 voices per track in SoundFonts, default 32. Layered presets can consume several voices per note. Changing the limit stops playback.
- Browse/preview without replacing assignments. Assign an SF2 and preset to each instrument or use the default bank. Paths/programs survive backup/restore and a fresh launch. Missing files silence affected instruments; samples remain external.
- Corrected high/low EQ shelves and effect ordering, stereo filter histories, chorus bounds, reverb updates and per-synth release timing. EQ gain now uses -12 to +12 dB with zero bypass; review older nonzero saved EQ gains after upgrading.
- Updated manual and synthetic audio tests for stereo, polyphony, effects, simultaneous banks, preview, undo, save/restore and missing-bank behavior.

Release/Debug builds pass. Verification includes 84 native Release/ASan checks (85 Debug), 99 stereo/effects/multi-bank checks, 107 MIDI/save/editor, 75 desktop feature, 61 layout, 107 device/recovery workflow and 14 font/no-font smoke checks. The sound, correctness, feature, layout and workflow suites also pass under AddressSanitizer; a further 91-check missing-font workflow passes there. Counts include repeated setup. Final sanitizer runs produce no address-error report.

Download **Ignite-Desktop-v0.3.3-desktop.4-windows-x64.zip**, extract it and run **Ignite.exe**. The package includes the manual and corresponding source. Supply your own SF2 files. This remains a community preview: physical MIDI/CV/clock qualification, complete SF2 modulators, MIDI file interchange, WAV/stem export and an Ignite VST remain outstanding.

# IGNITE Desktop v0.3.3-desktop.3

MIDI controller handling, protection for unsaved changes and editor/performance corrections.

- Internal synth: sustain, pitch bend and RPN bend range, volume/expression, bank/program changes, basic modulation/pressure vibrato, controller reset and all-notes/all-sound-off handling. Panic leaves the synth ready for new notes.
- Close with pending edits: Save / Discard / Cancel. Failed saves stay open for retry. Save All includes every loaded pattern/instrument and preserves global versus song-scoped settings. Checked atomic settings writes; backup export stops on save failure.
- Ctrl+Insert in Pattern view inserts a blank step, shifts later notes across bars and supports undo, within the 256-step limit.
- Corrected MIDI/Sculpt undo, signed and random-relative Sculpt arithmetic, pitch-map iteration, scale snapping at octave boundaries and scene-length fill release.
- Updated manual and reproducible correctness tests with synthetic multi-preset audio, locked-file failure/retry and fresh-process persistence checks.

Release and Debug builds succeed. Verification: **82 native Release/ASan checks; 83 Debug; 107 MIDI/save/editor; 75 desktop feature; 61 layout; 107 workflow/audio/recovery; 14 font/no-font smoke; 5 packet-protocol checks**. Correctness, features, layout and workflow also pass under AddressSanitizer. Counts include repeated setup. No address-error report was generated.

Download **Ignite-Desktop-v0.3.3-desktop.3-windows-x64.zip**, extract it and run **Ignite.exe**. Both Compact and Hardware layouts are included, with corresponding source and the illustrated manual. Supply your own SF2. This remains a community preview; physical MIDI/CV/clock behavior is unverified. The synth still uses one shared mono SF2, eight voices per track and basic modulation mappings. MIDI file interchange, song WAV bounce and an Ignite VST are not provided.

# IGNITE Desktop v0.3.3-desktop.2

Compact layout gives the firmware touchscreen most of the window, with the original buttons, knobs and pads arranged closely along the right and bottom. Use the **Compact / Hardware** toolbar to switch without restarting or changing the selected song. Compact is the default and the choice is remembered.

- Initial sizing fits the monitor work area; resizing keeps the compact screen's original aspect ratio. Small content sizes are covered down to 960×540 for scaled desktops.
- Both layouts share all 69 controls and the original touchscreen instance. Keyboard shortcuts remain available after using the toolbar.
- `--compact` / `--hardware` and the launcher's `-Layout` option provide temporary overrides without changing the saved preference.
- Desktop Settings is accessible from the top toolbar in either layout. The illustrated Markdown/HTML/PDF manual now covers both layouts.
- Layout regression checks exercise toolbar switching, repeated changes, geometry, selected-song/screen lifetime, navigation, SHIFT and saved/overridden preferences across processes. Existing audio, MIDI, SoundFont and backup features are retained.

Release and Debug simulator builds succeeded. The three-process layout suite passes **61 checks in Release and under AddressSanitizer**. The 75-check desktop feature suite is rerun with the new default layout, covering internal MIDI recording, custom SoundFonts and backup/restore. No physical MIDI or CV validation is claimed.

Download **Ignite-Desktop-v0.3.3-desktop.2-windows-x64.zip**, extract it and run **Ignite.exe**. The package includes corresponding source and the manual. Supply your own SF2. This remains a community preview with the hardware and Debug limitations listed below for desktop.1.

# IGNITE Desktop v0.3.3-desktop.1

First community Windows x64 preview of the IGNITE firmware application, based on upstream `0371a3ea22fc530e23e2321068a0b8835e1e6502`. The application identifies itself as firmware version 0.3.3. This desktop release is independent of Behringer and Music Tribe.

## Download and run

Download **Ignite-Desktop-v0.3.3-desktop.1-windows-x64.zip**, extract the complete folder and run **Ignite.exe**. Keep `assets` beside the executable. Supply your own SF2 bank under **DESKTOP SETTINGS → SoundFonts**. No SoundFonts or private sample libraries are bundled.

The package includes the illustrated [user manual](docs/manual/USER_MANUAL.md) in Markdown, self-contained HTML and printable PDF, plus corresponding application/support-library source. The PDF and HTML are also separate release downloads. `SHA256SUMS.txt` records asset hashes; `BUILD.json` identifies the source commit and executable hash.

## Included

- Interactive vector faceplate around the actual firmware touchscreen, including buttons, pads, encoders, knobs, ROW/BAR and jog controls. A touchscreen-only mode and keyboard shortcuts are available.
- Windows audio selection, saved device preferences, 48 kHz engine output converted to the actual device rate, callback block adaptation and internal-note Panic.
- Windows MIDI device mapping for 16 logical input/output ports and selected-track monitoring.
- Actual SoundFont preset names and bank/program enumeration, search, C4 preview, custom-bank assignment and saved selections.
- Repaired step/live MIDI recording, channel filtering, note-off duration handling and immediate stop/save persistence.
- Project backup ZIP export/restore, validation before replacement and recovery copies for interrupted directory saves.
- Repairs to copied model ownership, undo/save behavior, track iteration, playback editing, desktop timer/queue shutdown and UI publication.
- Reproducible native, workflow, internal MIDI, SoundFont, archive and packet-protocol checks; standalone AudIO/DAW/Airwindows and packet-protocol builds.

## Verification

Windows x64, MSVC 19.44, Windows SDK 10.0.26100, CMake 4.3 and JUCE 7.0.12. Release and Debug application/helper builds and the supporting static libraries succeeded.

| Check | Result |
|---|---|
| Native regression suite | 54/54 Release and AddressSanitizer; 55/55 Debug. |
| Arrangement workflow | 58 create/edit and 49 fresh-process checks, Release and ASan, including real Windows audio and interrupted-save recovery. |
| Desktop feature workflow | 50 setup, 21 feature and four reopen checks, Release and ASan. |
| Missing-font workflow | 50 create/edit and 41 reopen checks under ASan. |
| Application smoke | Seven checks with a bank and seven without; normal exits and screenshots. |
| Packet protocol | Five checks. |
| Packaged application | Launch/self-test passed; executable matched the tested Release build. |

Final ASan runs produced no address-error report. See [AUDIT_FIXES.md](docs/AUDIT_FIXES.md) and [DEVELOPMENT.md](docs/DEVELOPMENT.md) for the exact coverage and local run evidence. Counts overlap and do not establish every screen or hardware combination.

## Known limits

- Physical MIDI controllers, external MIDI destinations, external clock timing, CV/Gate, sync jacks, calibration and device updates have not been qualified. MIDI recording/routing tests used internal messages because no MIDI input devices were connected.
- A complete flashable firmware image is not included. Upstream omits MCU firmware, Wi-Fi firmware and the proprietary PC transfer service.
- The synth renders mono, with eight voices allocated per track and one shared SF2 bank. Layered presets can consume multiple voices per note. SF3 is not established.
- Debug retains inherited UI allocations and can spend a long time reporting them at exit. Use **Release** for normal operation. Large edits or saves can briefly interrupt audio.
- Save before closing; there is no automatic unsaved-changes prompt. Run one process per data directory. Restore replaces the saved collection and requires a restart.
- The optional VST source is a template, not a working Ignite plugin supplied here.

Application/community code remains AGPL-3.0-or-later with upstream and third-party notices preserved. Product names and marks remain their owners' trademarks.
