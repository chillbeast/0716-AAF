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
