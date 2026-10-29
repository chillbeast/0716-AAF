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
