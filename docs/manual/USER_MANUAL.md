# IGNITE Desktop Simulator

**User manual · Community release v0.3.3-desktop.4 · 28 October 2026**

IGNITE is a MIDI sequencer with an internal SoundFont synthesizer. This Windows edition combines the released firmware application with compact and hardware layouts, plus desktop audio, MIDI, SoundFont and backup controls. Both layouts run the actual firmware touchscreen and the same panel controls.

This is an independent community build of Music Tribe's open-source release. It is not an official Behringer manual or a flashable device image. The firmware application reports version 0.3.3; `desktop.4` identifies this community release.

![IGNITE simulator compact layout with a large live touchscreen](../images/ignite-compact.png)

## Contents

1. [Install and launch](#1-install-and-launch)
2. [The panel and keyboard](#2-the-panel-and-keyboard)
3. [Songs, scenes, tracks and patterns](#3-songs-scenes-tracks-and-patterns)
4. [Make your first sequence](#4-make-your-first-sequence)
5. [Patterns and note editing](#5-patterns-and-note-editing)
6. [Playback and performance](#6-playback-and-performance)
7. [SoundFonts and internal instruments](#7-soundfonts-and-internal-instruments)
8. [Windows audio](#8-windows-audio)
9. [MIDI connections and routing](#9-midi-connections-and-routing)
10. [Step and live recording](#10-step-and-live-recording)
11. [Save, export and restore](#11-save-export-and-restore)
12. [Troubleshooting](#12-troubleshooting)
13. [Feature status and limits](#13-feature-status-and-limits)
14. [Developer and release reference](#14-developer-and-release-reference)

## 1. Install and launch

Download the Windows x64 ZIP from the [GitHub release](https://github.com/chillbeast/0716-AAF/releases/tag/v0.3.3-desktop.4). Extract the complete folder, then run **Ignite.exe**. Keep its **assets** directory beside the executable. Launch from the extracted folder rather than opening the executable inside the ZIP.

The package is portable and does not require an installer, Visual Studio or a Python installation to run. Development tools are needed only to rebuild it. A working Windows audio output is required to hear internal sound. Supply your own `.sf2` SoundFont; sample banks are not included.

For a shortcut, right-click the extracted `Ignite.exe` and choose **Show more options → Send to → Desktop (create shortcut)**. Moving the extracted folder later requires updating that shortcut. An existing source-checkout shortcut may instead launch `scripts/run-ignite.ps1`.

Open **Desktop settings** in the top toolbar, or press **Ctrl+,**. This is separate from the firmware's own Settings screen in MENU. Desktop Settings contains **Audio**, **MIDI**, **SoundFonts** and **Projects** tabs.

For a larger touchscreen without the surrounding panel, launch `Ignite.exe --touchscreen` from PowerShell or add `--touchscreen` to a shortcut's target arguments. Ctrl+, still opens Desktop Settings.

## 2. The panel and keyboard

Use **Compact** or **Hardware** in the top toolbar to choose the layout. Compact is the default: most of the window is the touchscreen, with closely spaced controls on the right and a strip of step encoders/pads below. Hardware restores the vector product faceplate. The layout choice is remembered for the next launch; switching keeps the same screen, song and controls active.

The initial window fits the monitor's usable desktop area. Resize or maximise it to give the screen more room. Compact preserves the touchscreen's aspect ratio and adapts the control spacing to smaller windows, including common laptop desktop sizes. The layout tests cover 960×540, 960×600, 1280×720 and 1366×768 content sizes. On very small or heavily scaled displays, touchscreen text will still be smaller than its native size.

![IGNITE hardware layout, selectable without restarting](../images/ignite-simulator.png)

Most panel buttons depend on the current screen and selected object. Start with the mode buttons, then select the song, scene, track or pattern you want to edit.

| Control | Mouse operation |
|---|---|
| Touchscreen | Click a control; drag where the firmware supports dragging. |
| Buttons and 16 pads | Press and release. Hold the mouse button to send the firmware's hold action. |
| 16 step encoders | Drag vertically or scroll to turn; click without dragging to push. |
| Knobs A–H | Drag vertically or scroll to turn. |
| ROW and BAR | Drag or scroll to change the current row/bar as defined by the active screen; click to push. |
| Jog / Enter | Drag vertically or scroll to navigate; click to select. |
| SHIFT | Click to toggle the firmware Shift state; click again to release it. |
| Return, Copy, − and + | Send the corresponding firmware navigation/edit action. |

EQ High and EQ Low control the matching high/low shelf. Alternate gain is **-12 to +12 dB**; **0 dB** bypasses that band. Older nonzero saved EQ gains are now interpreted as dB, so review saved EQ settings after upgrading. Release time is independent per synth, with 0 using the preset release. Reverb applies width/dry controls immediately, chorus processes both channels, and stereo filter/EQ histories remain independent.

| Keyboard | Action |
|---|---|
| F1 | MENU |
| F2 | SONG |
| F3 | SCENE |
| F4 | TRACK |
| F5 | PATTERN |
| Space | PLAY |
| Esc | STOP |
| Ctrl+S | SAVE |
| Ctrl+, | Desktop Settings |

Hover over a panel control for its tooltip. When typing in a text field, the panel avoids intercepting its normal typing keys. If a mode shortcut seems inactive, leave the text field and click the panel first.

This simulator does not turn the PC keyboard into a MIDI piano. Use a MIDI input for performed notes, or enter events in the pattern editors. The panel pads have screen-specific sequencer functions; do not assume KICK drum-pad behavior.

## 3. Songs, scenes, tracks and patterns

| Object | Role |
|---|---|
| Song | Holds scenes and song-level sequencing settings. |
| Scene | Holds an arrangement of tracks and scene-level timing/performance settings. |
| Track | Connects patterns to instruments; also holds track transforms, mute and recording state. |
| Pattern | Contains bars, steps, note events and auxiliary events. Patterns have their own library. |
| Instrument | Describes a MIDI port/channel and/or an internal sound selection. Instruments have their own library. |
| Receive instrument | Specifies the input port and channel a track listens to. |
| Output instrument | Specifies where the track sends events and which internal preset it uses. |

Creating a song does not create a ready-to-play scene, pattern and sound assignment. An empty pattern or an unassigned output instrument can be perfectly valid and silent.

The source supports up to **64 tracks**, **16 step positions per bar**, **16 bars per pattern**, scene lengths up to **200 bars**, **96 PPQN** timing and a registered tempo range of **20–360 BPM**. These are software limits, not a claim that 64 heavily layered instruments can play without dropouts.

Patterns and instruments can be reused. Editing a reused library object may affect more than one assignment. Copy it before making a variation that should remain independent. Save after the change.

## 4. Make your first sequence

This walkthrough uses internal audio and does not require a MIDI controller. The underlying create/edit/save/play workflow is covered by model tests; the test harness does not click every control in these screens.

1. **Choose a default bank.** Open Desktop Settings → SoundFonts → **Browse SF2...** and select an SF2, then press **Set as default SF2**. A complete GM bank makes the original firmware preset names useful; a single-instrument bank also works with the desktop browser.
2. **Create a song.** Press SONG / F2. Use the empty-list creation control or **+ Add New Song**, then select the new song.
3. **Create a scene.** Press SCENE / F3. Use the scene creation control or **+ Add New Scene**, then select it. Tracks belong to this scene.
4. **Create an instrument.** Open MENU / F1 → Instrument and use its creation control. Note the instrument's name.
5. **Choose its sound.** Return to Desktop Settings → SoundFonts. Select that instrument in the lower selector first, browse a different SF2 if desired, select a preset, and press **Assign SF2 + preset**. This enables internal sound and saves the instrument selection.
6. **Create a pattern.** Open PATTERN / F5 and use its creation control or **+ Add New Pattern**. Choose a polyphonic pattern if you want chords; a monophonic pattern is sufficient for a simple melody.
7. **Connect the track.** Open TRACK / F4, select a track, and assign your pattern and output instrument. Ensure the track is not muted. A receive instrument is needed for MIDI recording, but not for entering notes with the mouse.
8. **Enter a few notes.** Open the assigned pattern. Use its step/event editor or grid tools to add notes, with nonzero velocity and duration, and enable the relevant steps. For a short test, use a one-bar sixteenth-note pattern and put notes on positions 1, 5, 9 and 13.
9. **Play.** Press PLAY or Space. If silent, check the track assignments, mute state and audio output before changing more settings.
10. **Save and reopen.** Press SAVE / Ctrl+S, stop playback and close Ignite. Launch again and select the song and scene to confirm your arrangement is there.

Use the larger touchscreen mode if small firmware fields are difficult to read. Loading a SoundFont stops playback; after changing banks, choose presets that actually exist in the new bank.

## 5. Patterns and note editing

The firmware includes pattern list, bar, step/event and grid views. These are views of the same pattern data. Use the screen's current selection and parameter labels to determine what the pads, encoders, ROW and BAR controls will change.

A note event includes pitch, velocity, length in steps/ticks and a tick offset. Multiple notes can occupy a step in a polyphonic pattern. Velocity zero, a disabled step, a muted track or an unavailable preset can each explain a silent note.

| Parameter | Musical effect |
|---|---|
| Pitch / note | Selects the note played by the output instrument. |
| Velocity | Controls note strength; how strongly it changes the tone depends on the preset. |
| Length in steps | Sets the coarse duration. |
| Length in ticks | Adds finer duration control. |
| Offset | Moves the event relative to the step boundary. |
| Time base | Changes the rhythmic spacing of the pattern's step positions. |

Bar operations include copying, appending, duplicating, inserting and removing. The desktop regression suite covers independent copies, correct note/step positions, the 16-bar cap and retaining at least one bar. Save a backup before a large rearrangement.

In the Pattern view, **Ctrl+Insert** inserts one blank step before the selected step. Later events shift across bar boundaries; the active length and bar count grow as needed, up to 256 steps. Undo restores the previous pattern. The hardware INSERT button retains its view-specific note/event behavior.

Auxiliary events, triplet time bases and the broader editing tools exist in the source. Coverage varies by operation; see the feature-status table rather than assuming every combination was exercised.

## 6. Playback and performance

PLAY starts or resumes the current arrangement; STOP ends the current transport action. The firmware decides the precise behavior from its current transport state. Pause/resume, tempo changes and normal stop commands are covered by desktop workflow tests.

The front panel also exposes NEXT/TIE, FILL/X, SOLO/SKIP, GANG and SCULPT. These depend on the active view and track/pattern configuration. The source contains main/fill patterns, grouped operations, scale/transpose transforms and sculpt changes to notes and velocities. Regression coverage includes gang velocity selection, signed relative and absolute Sculpt edits, random relative offsets, pitch moves, MIDI/Sculpt settings undo, fill mute/release rules, scale snapping across octave boundaries, transform priority and MIDI-range clamping. The tests exercise the real engine and selected UI editing paths; every performance combination and external timing behavior has not been qualified.

Start with the internal clock. Source settings also cover MIDI clock, MIDI Machine Control, remote control and sync connections. Timing accuracy against external equipment has not been measured. Selecting an external clock source without providing that clock can leave playback waiting.

Knobs A–H have the front-panel labels **Release, Cutoff, Resonance, Volume, Reverb, Chorus, EQ High and EQ Low**. Their actual response depends on the active firmware context and instrument/effect setup. Effects and transforms can change loudness; use moderate output levels while exploring.

## 7. SoundFonts and internal instruments

![SoundFont browser showing a generated bank 42, program 7 test preset](../images/desktop-soundfonts.png)

Use **Browse SF2...** to open a bank from your sample collection, including Unicode paths. Browsing and previewing do not change instrument assignments. Different instruments can use different SF2 files simultaneously. Tracks sharing a bank share its sample memory and keep their own voices, MIDI controllers and effects.

The preset list shows **bank:program** followed by the preset name. Both numbers start at zero. Search by name or number. This lists the actual presets in the loaded file, so banks with custom numbering are usable without remapping their samples.

Select a row and press **Preview C4**, or double-click the row, to hear a short note while stopped. Preview does not require a track assignment. A drum preset may have no sample on that particular note; silence on C4 alone does not establish that the bank is invalid.

To choose a sound for a sequence, select an existing instrument below the list first, browse the desired SF2, select a preset, then press **Assign SF2 + preset**. Ignite saves the file path and preset with that instrument and stops playback for the change. Assign the instrument to the track's output. The **Assigned** line shows its bank.

**Set as default SF2** saves the browsed bank as the startup default. Explicit instrument assignments retain their own banks. **Use default SF2** clears the selected instrument's explicit path; it keeps a matching preset where available, otherwise selects the first preset in the default bank. The original firmware GM picker remains available for conventional GM banks.

The default bank path is remembered in Windows preferences. Explicit instrument paths and bank/program assignments survive saved arrangements and project backups; SF2 samples remain external. If an assigned file is missing, that instrument stays silent while instruments with available banks continue playing. Browse an available file and reassign its preset to repair the path. A missing default bank does not prevent explicit instrument banks from playing.

| Engine property | Current behavior |
|---|---|
| Bank format | SF2. SF3 support is not established. |
| Bank assignment | One default SF2 plus optional per-instrument SF2 paths; sample data is cached and shared by path. |
| Voices | 32 per track by default. Choose 8, 16, 32, 64 or 128 in the SoundFonts tab. Layered/stereo presets can consume several voices per note. The setting applies to every track and changing it stops playback. |
| Synthesis | Stereo, 48 kHz, including MIDI pan and stereo effects. Resampled to the Windows device rate; a mono device receives a downmix. |
| Missing preset | Silent instead of reusing a previous valid preset. |
| Failed replacement load | Retains the previously loaded bank. |

No commercial SoundFonts are bundled. Use files whose license permits your intended use. This release also does not expose the entire separate AudIO/Airwindows effect collection through the firmware UI.

## 8. Windows audio

Open Desktop Settings → **Audio**, choose the output device, sample rate and buffer size, then press **Save audio settings**. Windows may negotiate 44.1 kHz even when the engine requests 48 kHz; the desktop output adapter converts between the engine and device rates.

Play, input monitoring and preset preview can open the audio device. Stop, pause or the end of a preview can close it, so seeing a closed device while idle is expected. If an output becomes unavailable, reconnect it and retry playback. The public build has ASIO disabled.

**Panic** clears sounding internal notes. It is useful after interrupted input or when testing a preset with a long release. It does not promise to reset every external MIDI instrument.

The audio path uses a 960-sample internal render block, then adapts to the driver's callback size. Smaller Windows buffers do not remove all engine or device latency. Editing or saving can briefly interrupt audio because the desktop protects model changes with a shared lock. Bluetooth devices can add substantial delay.

## 9. MIDI connections and routing

![Windows MIDI input/output routing for the firmware's logical ports](../images/desktop-midi.png)

The MIDI tab maps Windows MIDI device names to the firmware's **five DIN, six USB-device and five USB-host logical ports**. These are software routes: mapping a controller to DIN 1 does not create a physical DIN jack on the PC.

1. Connect your controller or install an appropriate virtual MIDI port outside Ignite.
2. Open Desktop Settings → MIDI and press **Refresh devices**.
3. Choose its Windows input on a logical port row, such as USB device 1.
4. Set the track's receive instrument to that same logical port and MIDI channel in the firmware UI.
5. Assign an output instrument with internal sound enabled if you want Ignite to make the sound.
6. Leave **Hear incoming notes through the selected track** enabled for monitoring.

Input and output choices are independent. Outputs initially use **None**, preventing unexpected duplication through the Windows software synth. To drive an external device, select its Windows MIDI output on the logical row used by the track's output instrument.

The interface uses channels 1–16; internal data stores them as 0–15. A channel mismatch can prevent both monitoring and recording. Multiple logical rows can use the same device, which may duplicate messages; map only the rows you need.

Routes and the monitoring option save automatically. If startup reports a missing device, reconnect it, refresh and select the route again. Avoid routing an output back to its own input unless you deliberately want that loop and have configured filtering.

The internal synth handles sustain (CC64), channel volume (CC7/39), expression (CC11/43), bank select (CC0/32) plus program change, pitch bend and RPN bend range, modulation wheel, channel pressure and polyphonic pressure. Modulation and pressure add a basic vibrato mapping; this is not a complete SoundFont modulator-matrix implementation. Controllers are scoped to the routed track. CC121 resets expression, sustain, bend position, modulation and pressure while retaining volume, pan and the chosen bank/program. CC123 releases notes subject to sustain; CC120 stops sound immediately and clears the track effects. Panic clears notes and leaves the synth playable.

No MIDI input devices were connected for release testing. Internal messages exercised the same router entry point and all 16 logical port IDs; physical controllers, driver behavior, external output and external-clock timing remain unverified.

## 10. Step and live recording

Both methods need a selected song, scene and track, an assigned pattern, a matching receive instrument and enabled note recording in the firmware record settings. Enable internal monitoring if you want to hear the performed notes.

### Step entry while stopped

1. Stop playback and select the pattern/track you want to record.
2. Set the input port/channel and note-event recording filter.
3. Press REC to enter step recording.
4. Play and release notes on the matching input. Notes on other channels should be ignored.
5. Stop recording, then inspect the pattern's events and save.

The internal test records two accepted notes, rejects another channel, saves and reopens them in a fresh process. Step recording is MIDI note entry; it is not an audio recorder.

### Live recording during playback

1. Assign the track's receive instrument and pattern, then enable the track's recording state.
2. Start playback and press REC to enter live recording.
3. Play the notes, including note releases.
4. Stop recording before closing or changing the arrangement, then save.
5. Inspect note velocity and duration in the pattern editor.

The test checks an armed track, captured velocity/duration, stopping immediately after note-off and reopening the saved result. Unarmed tracks should not record. This verifies the internal event path, not the latency or timing of a physical controller.

## 11. Save, export and restore

Press **SAVE / Ctrl+S** to save the active song, all loaded patterns and instruments, and the firmware settings at their selected global or song location. This includes library items edited before switching to another item.

Closing a window or quitting with pending edits offers **Save**, **Discard**, or **Cancel**. Save closes only after all required writes succeed. A failed save keeps Ignite open with the pending changes available for retry; fix the file permissions or storage problem and save again. Discard leaves the saved files as they are; Cancel keeps the session open. Writes already completed by automatic or individual saves remain on disk. Undo may conservatively leave a pending-change indication even after returning to an earlier value.

Settings files use temporary-file replacement. Save All spans several files and is not one transaction: a failure can leave some items saved while others still need a retry. Backup export stops if the preceding save fails.

Windows data is stored separately from the extracted application:

| Location | Contents |
|---|---|
| `%APPDATA%\BehringerIgnite\Ignite_folder` | Songs, patterns, instruments and firmware settings. |
| `%APPDATA%\BehringerIgnite\desktop.json` | Windows audio settings, MIDI routes, monitoring preference and SoundFont path. |
| Your chosen SF2 location | The sample bank; it remains outside project storage. |

Upgrading the extracted app does not require copying the data into its folder. Export a backup before upgrading. Run only one Ignite process against a given data directory.

![Project backup export and restore controls](../images/desktop-projects.png)

**Export backup...** saves the libraries and writes an Ignite ZIP containing songs, patterns, instruments, firmware settings and the library index. **Restore backup...** validates and stages a compatible backup before replacing the saved collection. After a successful restore, close Ignite using the completion message, then launch it again.

Restoring replaces the collection rather than merging it. Export the current collection first if you need to retain it. The archive does **not** contain SF2 sample files or Windows device preferences, so keep the bank available and review audio/MIDI selections on another PC.

Song, scene, pattern and desktop restore replacements retain recovery directories. A missing library from an interrupted directory swap is recovered at startup. Recovery copies are not a substitute for an exported backup, and Save All is not one transaction across all possible settings files.

## 12. Troubleshooting

| Symptom | Checks |
|---|---|
| App opens with missing artwork | Extract the whole ZIP and keep `assets` beside `Ignite.exe`. |
| Preview works, sequence is silent | Select a song and scene; check the track's pattern, output instrument, mute state, enabled steps and note velocity. |
| Nothing makes sound | Load a valid SF2, select an existing preset, check the Windows output and retry Play or Preview. |
| A custom bank is silent with GM names | Use the desktop SoundFont browser's actual bank/program list and assign that preset to the instrument. |
| Preview is silent only on a drum preset | C4 may be unmapped; test appropriate drum notes in a pattern. |
| No MIDI devices appear | Connect the device, close other apps that may own it, then Refresh. Ignite does not create virtual drivers. |
| MIDI arrives but is not heard/recorded | Match the logical port and channel, assign a receive instrument, enable monitoring/note filters, and arm the track for live recording. |
| Playback waits or does not advance | Check the selected clock source; use internal clock for the first test. |
| Audio crackles or feels delayed | Try a wired output and another buffer size; reduce active voices/effects and avoid large edits during playback. |
| Old notes keep sounding | Use Desktop Settings → Panic and check note-off input/routing. |
| A restored project looks unchanged | Close and reopen Ignite; restore replaces saved files, then startup reloads the libraries. |
| Desktop shortcut stops working | Confirm the extracted/source folder has not moved and update the shortcut target. |
| Debug takes a long time to exit | Use Release for normal operation; inherited UI allocations produce a long Debug CRT leak report. |

For a file log, set `IGNITE_LOG` to a writable path before launch. Source-checkout launches through `scripts/run-ignite.ps1` log to `Source/Application/build/win/ignite.log`. Logs can include local paths, so review them before posting publicly.

## 13. Feature status and limits

| Status | Release coverage |
|---|---|
| Tested on Windows | Build and startup, vector panel navigation, model creation/editing/copying, save/reopen, staged-save recovery, track counts through 64, playback edits, device audio and rate conversion. |
| Tested with internal MIDI | Logical input/output routing, channel-filtered step recording, live velocity/duration, saved recordings and fresh-process verification. |
| Tested desktop services | Actual SF2 preset enumeration, non-GM assignment and preview, failed-bank retention, preferences and backup/restore round trips, unsafe/corrupt archive rejection. |
| Present in source; coverage varies | All editor combinations, auxiliary events, main/fill performance, gang/sculpt, scales/transpose, remote control and external clock options. |
| Requires physical testing | MIDI controllers and external destinations, DIN/USB hardware endpoints, CV/Gate voltages, sync jacks, calibration, updates and hotplug behavior. |
| Incomplete device distribution | MCU firmware, Wi-Fi firmware and the proprietary PC transfer service were omitted upstream. A complete flashable image is not supplied. |
| Known desktop limitation | Debug retains UI allocations and reports them slowly at exit. The shared model lock can cause brief audio gaps during edits/saves. |

The synth has a configurable voice limit; complete SoundFont modulator-matrix support is still incomplete. This simulator is not an emulation of the MCU, electrical I/O or device storage hardware. The optional VST source is a template, not an Ignite plugin supplied by this release.

## 14. Developer and release reference

Source: [chillbeast/0716-AAF](https://github.com/chillbeast/0716-AAF), branch **desktop-build**. Upstream: [MusicTribeOpenSource/0716-AAF](https://github.com/MusicTribeOpenSource/0716-AAF), base commit `0371a3ea22fc530e23e2321068a0b8835e1e6502`.

To build the simulator, supporting libraries and helpers, run from the repository root:

```powershell
.\scripts\build-windows.ps1 -DebugBuild -Package
```

See [Windows build instructions](../BUILD_WINDOWS.md) for Visual Studio, CMake, JUCE and Python requirements. Reproduce the expanded desktop feature checks with:

```powershell
.\scripts\features-windows.ps1 -Configuration Release
.\scripts\correctness-windows.ps1 -Configuration Release
.\scripts\sound-windows.ps1 -Configuration Release
.\scripts\workflow-windows.ps1 -Configuration Release -NodeEdit -AudioDevice -SimulateInterruptedSave
```

Tests use disposable directories under the ignored build tree. The release was checked with **84 native Release/ASan checks**, **85 native Debug checks**, **107 arrangement workflow checks**, **75 desktop feature/setup/reopen checks**, **107 MIDI/save/editor correctness checks**, **61 layout checks**, **99 stereo/effects/multi-bank checks**, **91 no-font workflow checks under ASan**, **seven smoke checks per font/no-font case**, and **five packet-protocol checks**. These counts describe overlapping suites, not a count of every supported feature. Final sanitizer runs reported no address errors.

The separate AudIO, DAW and Airwindows libraries also build, but the main app uses its firmware TinySoundFont path. Detailed evidence and remaining audit items are in [AUDIT_FIXES.md](../AUDIT_FIXES.md), [CAPABILITIES.md](../CAPABILITIES.md) and [DEVELOPMENT.md](../DEVELOPMENT.md).

Application and community code retain the upstream AGPL-3.0-or-later license and third-party notices. Behringer, IGNITE and Music Tribe names remain their owners' trademarks. See the repository LICENSE and upstream [trademark policy](https://github.com/MusicTribeOpenSource/.github/blob/main/TRADEMARKS.md).
