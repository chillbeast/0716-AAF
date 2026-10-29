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

## SoundFonts

![Actual SoundFont preset browser with a generated non-GM test bank](images/desktop-soundfonts.png)

Choose **Load SoundFont...** to load an SF2 from your sample library. The chooser starts at `D:\Old Samples` on this machine. The bank is loaded directly from its existing location; it is not copied into the repository or project backup. Loading stops playback and replaces the shared bank for every internal instrument.

The browser lists the actual preset names and **bank:program** numbers, starting at zero, including non-GM banks. Search the list, select a row and press **Preview C4**, or double-click a row. Preview plays a short note while stopped. To use it in a sequence, select an existing instrument below the list and press **Assign to instrument**. This enables internal sound and saves that instrument's selection. Create instruments using the firmware UI first.

Custom bank/program choices survive project saves and restarts. The firmware's original GM picker remains available for conventional GM banks. A missing preset stays silent. A failed replacement load retains the current bank. SF3 support is not established, and the synth renders mono with eight voices allocated per track.

Startup selection is an explicit `IGNITE_SOUNDFONT` override, the saved `SoundFont` preference, an adjacent `Ignite.sf2`, then the original platform fallback. The local PowerShell launcher also uses the ignored build settings as a fallback. An invalid explicit environment override disables internal sound for diagnostic runs.

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
