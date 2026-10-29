# Windows desktop features

Open **Desktop settings** in the top toolbar, or press **Ctrl+,**. The firmware touchscreen and its normal editing controls remain available alongside these Windows controls.

## Monitor layouts

The **Compact / Hardware** toolbar switches between a large touchscreen with closely spaced controls and the SVG hardware faceplate. Compact is the default; your choice is saved as `PanelLayout` in `desktop.json`. The same touchscreen and all 69 controls stay active across switches, including held/latched SHIFT state. The initial window fits the monitor work area, and Compact keeps the touchscreen's original aspect ratio when resized.

![Compact layout with more room for the firmware screen](images/ignite-compact.png)

Use `Ignite.exe --compact` or `--hardware` for a temporary launch override. The source launcher accepts `scripts/run-ignite.ps1 -Layout Compact` or `-Layout Hardware`. These overrides leave the saved choice alone. `--touchscreen` still opens the screen on its own.

## Audio

The Audio tab selects a Windows output, sample rate and buffer size. **Save audio settings** remembers the selection for the next launch. The synth runs at 48 kHz; the output adapter converts to the device rate. Play and MIDI monitoring open the output; stopping transport or finishing a preset preview can close it. **Panic** clears sounding internal notes.

## MIDI routing and recording

![Windows MIDI device mapping](images/desktop-midi.png)

The MIDI tab maps Windows device names to the firmware's five DIN, six USB-device and five USB-host logical ports. Each row has independent input and output choices. Routes and the selected-track monitoring option are saved automatically. Use **Refresh devices** after connecting a controller, then choose its input. Outputs start at **None** to avoid unwanted routing to the Windows software synth.

These are software routes to Windows MIDI devices; selecting a DIN row does not create a physical DIN connector. Multiple logical rows may use one device. If startup reports a missing device, reconnect it, refresh the list and select the route again.

Assign the matching input port and channel to a track's receive instrument, and assign an output instrument with internal sound enabled to hear the selected track. The firmware displays channels 1–16 and stores them as 0–15. Start Record while stopped for step entry. For live recording, start playback, enable recording on the intended track, then start Record. Stop Record commits pending notes before saving.

The automated tests inject MIDI into the same router used by device callbacks. They check all 16 logical input/output IDs, channel filtering, step entry, live note velocity/duration, saving and reopening. No MIDI input devices were available during this pass, so physical controller input, Windows driver behavior, external clock timing and external hardware output remain unverified.

Internal synthesis now responds to sustain, pitch bend/RPN range, volume/expression, bank/program, modulation and pressure. Modulation/pressure use a basic vibrato mapping. All Notes Off honors sustain; All Sound Off clears the track voices/effects. Panic permits subsequent notes without reloading the bank. Custom SF2 modulator matrices are not fully implemented.

## Saving and editor fixes

Closing with pending edits offers Save, Discard and Cancel. Save All writes every loaded pattern/instrument plus the active song and firmware settings, preserving song-scoped settings. Failed writes keep pending changes and stop close/backup export. Settings use atomic file replacement; the complete save is still several independent file operations.

In Pattern view, Ctrl+Insert inserts a blank step and shifts later notes across bars, with undo and a 256-step limit. Gang velocity selection, signed Sculpt modes, pitch-map changes, MIDI/Sculpt settings undo, scale snapping and fill release have regression coverage. Other view-specific hardware key behavior remains available.

## SoundFonts

![Actual SoundFont preset browser with a generated non-GM test bank](images/desktop-soundfonts.png)

Choose **Browse SF2...** to open a bank from your sample library. The chooser starts at `D:\Old Samples` on this machine. Browsing leaves assignments intact; select a preset, choose an instrument and press **Assign SF2 + preset** to save that file and program for the instrument. Different instruments can play different SF2 files simultaneously. Sample data is shared by path, with independent track voices and effects.

The browser lists actual preset names and zero-based **bank:program** numbers, including non-GM banks. Search, use **Preview C4**, or double-click a row. Preview uses the browsed bank independently of the default. Create instruments in the firmware UI first. The Assigned line displays the instrument's bank path.

**Set as default SF2** saves the browsed file as the startup default, retaining explicit instrument assignments. **Use default SF2** returns the selected instrument to the default, preserving a matching preset or choosing the first available preset. Assigning a bank or changing the default stops playback. Missing assigned files stay silent without substituting another bank; other instruments continue working. Backups retain paths/programs, with SF2 samples kept outside the archive.

Windows rendering is stereo at 48 kHz, with independent stereo EQ/filter/chorus and reverb. MIDI pan reaches the actual output. **Voices per track** offers 8, 16, 32, 64 and 128; the default is 32. The limit applies to all tracks; changing it stops playback and sounding notes. Layered presets use multiple voices per note. The audio adapter preserves stereo across driver block sizes and downmixes for mono output. SF3 and complete SoundFont modulators remain unqualified.

EQ gain now uses -12 to +12 dB, with 0 dB bypass. Check existing nonzero saved EQ gains after upgrading: old values were used as linear multipliers and the shelves were reversed. Release time now applies independently to each synth.

The startup default is an explicit `IGNITE_SOUNDFONT` override, the saved `SoundFont` preference, an adjacent `Ignite.sf2`, then the original platform fallback. The launcher also uses ignored build settings as a fallback. An invalid explicit override disables the default for diagnostic runs; instruments with valid explicit paths still work.

## Backups

![Project backup export and restore](images/desktop-projects.png)

**Projects → Export backup...** saves the current libraries and writes a ZIP containing songs, patterns, instruments, settings and the library index. **Restore backup...** validates and stages a compatible Ignite backup before replacing the saved libraries. Ignite closes after a successful restore; launch it again to load the restored state. Restoring replaces the saved collection, so export the current collection first if you want to retain it.

SoundFont files and Windows device preferences are separate from these project archives. Keep the required SF2 files available at their selected paths. A project backup does not freeze the state of external MIDI instruments.

The importer rejects unsafe archive paths, unsupported formats and malformed saved XML before replacing data. Directory replacements retain a recovery copy. This does not make all library/settings writes one transaction or cover every storage failure. Use one Ignite process per data directory.

## Data, builds and limits

Windows preferences live in `%APPDATA%\BehringerIgnite\desktop.json`; the firmware libraries live in the adjacent `Ignite_folder`. `IGNITE_DATA_DIR` changes their common parent. Test scripts always use disposable directories under `Source/Application/build`.

Build the simulator, native checks, SoundFont probe and standalone supporting libraries with:

```powershell
.\scripts\build-windows.ps1 -DebugBuild -Package
```

Run the additional desktop feature round trip with `scripts/features-windows.ps1`. See [BUILD_WINDOWS.md](BUILD_WINDOWS.md) for tools and [AUDIT_FIXES.md](AUDIT_FIXES.md) for repairs and verification evidence.

Release is the normal-use build. Debug still has an inherited retained UI graph that produces a long CRT leak report during exit. AudIO and the packet protocol build independently; the main app uses its firmware TinySoundFont path. The optional VST source is a template, not a functioning Ignite plugin. Device images, MCU firmware, CV/Gate, sync jacks, Wi-Fi transfer and physical calibration need separate hardware work; omitted upstream components prevent a complete device-image build from this checkout alone.
