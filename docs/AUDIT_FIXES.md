# Desktop reliability repairs, 2026-10-28

This pass uses the adjacent KICK audit as a reference for model lifetime, message-thread UI updates, safe persistence and sanitizer-backed workflow tests. IGNITE has its own engine; fixes were adapted to its source rather than copied wholesale. Community changes are published from `desktop-build`.

## Confirmed failures and repairs

| Area | Previous behavior | Repair and regression coverage |
|---|---|---|
| Note editing | Editing an existing note crashed in `Step::validateDelay`, through `Pattern::getTimeBase`, because two step constructors left their owning pattern null. | Initialise and rebind the owning pattern. Exercise the actual node-edit API, save the edit and reopen it in another process. |
| Message values | Copying a numeric `AnyType` changed its reported type to `UINT64`. Assigning a number over a string leaked the previous allocation. | Own the value in `std::variant`, preserve the type across copies and release replaced strings. The original Debug test failed three checks; the repaired test includes a CRT heap allocation comparison. |
| Copied patterns | Bars and steps could share owning data through implicit copies; owned data also leaked. | Deep copy and assignment for patterns, bars and steps, with destructors and repaired child references. Pattern rotation can now assign a replacement without sharing temporary data. |
| Copied songs | A copied song's selected scene pointed into the source song. Some song settings reverted to defaults. Scene and instrument assignments shared owning pointers. | Copy all settings and owned lists, then select children inside the destination. Check song/scene/instrument copies and assignments under AddressSanitizer. |
| Track delivery | The worker queue only dispatched complete groups of eight. Counts 1, 7 and 9 failed the clock regression; 8, 16 and 64 passed. | Windows visits tracks synchronously under the engine lock, eliminating queued pointers to removed tracks. The hardware queue branch also dispatches its trailing partial batch. All six counts pass on desktop. |
| Playback edits | The clock, MIDI, UI edits and synth could access model objects concurrently. | Protect desktop model entry points with a recursive mutex. Audio tries the lock and emits silence during contention. Desktop track synthesis is serial, avoiding unsequenced parallel work on mutable data. Stress note edits and repeated 1–64-track resizing with clock and rendering active. |
| Windows clock | A time-critical busy loop read tempo changes without synchronisation and adjusted intervals by subtracting timing error. | Use interruptible steady-clock deadlines. Pause, resume, interval changes, zero interval rejection and joined shutdown have regression coverage. Large scheduling stalls do not trigger an unlimited burst of old pulses. |
| Bar editing | Copy-and-append had a non-advancing iterator. Several edit paths shortened the exclusive end position by one. Copies could omit tie/direction state or leave notes with old step IDs. | Bounded append/duplicate/insert/remove/copy operations preserve independent data, sequential bar/step/note positions and the exclusive pattern length. Keep one bar and cap at 16. |
| Pattern Save All | Save inspected the oldest undo entry, which is the saved baseline, and skipped recent edits. The regression failed both the immediate disk check and fresh-process verification. | Inspect the newest undo entry. Save now persists the edited velocity, and a new process reads that value. |
| Scene-only Save All | Save All skipped the full song write when only scene settings were dirty, and cleared scene undo before confirming persistence. | Save All writes the whole active song regardless of song-parameter undo state. Scene save uses staged replacement; dirty state clears after a successful commit. Check separate scene Save and scene-only Save All, then reopen. |
| Interrupted saves | Song, scene and pattern saves deleted the old directory before writing the replacement. Track and instrument files were overwritten in place. | Stage complete song/scene/pattern directories, check writes, retain the previous directory as a recovery copy and recover interrupted directory swaps at startup. Replace track, instrument and song/scene metadata files through temporary files. Retain dirty/undo state on failed song/instrument saves. |
| UI notifications | Desktop UI notifications ran on a publisher thread under `MessageManagerLock`. Delayed scene messages could dereference a null selected song. | Drain desktop notifications on the message thread; reject missing/out-of-range scene selections. Publication never blocks the model on a full UI queue. |
| Device lifecycle | Opening audio ignored errors and marked the callback enabled even on failure. Stop/pause did not close the device. | Check/log opening errors, allow retry, close on pause/stop and reopen on resume. Exercise the normal transport commands against the actual Windows device. |
| Device sample rate | The headphones negotiated **44,100 Hz** despite a request for 48,000 Hz. Feeding the 48 kHz synth directly to them changed pitch and playback speed. | Resample the mono 48 kHz engine stream with JUCE's `ResamplingAudioSource` at the negotiated device rate, then duplicate to active output channels. Verify the source/output frame ratio in actual device callbacks. |
| MIDI UI bounds | A channel-use query lacked a return for missing ports. Channel helpers lacked an explicit lower bound. | Return false for missing ports and accept only channels 0–15. |

The desktop lock follows the KICK repair's lifetime principle. It is a soft real-time solution: a long edit or save can briefly interrupt audio. The firmware's Linux scheduling and physical I/O still require separate testing.

## Reproduce

Build instructions are in [BUILD_WINDOWS.md](BUILD_WINDOWS.md). The workflow runner uses a fresh ignored data directory for each run, creates an arrangement through model commands, edits notes, saves, checks copied ownership, renders audio, tests bar operations and transport, then reopens the saved arrangement in a second process:

```powershell
.\scripts\workflow-windows.ps1 -Configuration Release -NodeEdit -AudioDevice -SimulateInterruptedSave
ctest --test-dir Source/Application/build/win -C Release --output-on-failure
ctest --test-dir Source/Application/build/win -C Debug --output-on-failure
```

`-AudioDevice` opens the current Windows output and can make sound. Without it the workflow renders offline and still exercises the live clock/editing path. `-SimulateInterruptedSave` moves only the runner's disposable saved song into its recovery directory; the second process must recover it. `-SoundFont` overrides the local configured bank; the generated test bank is a fallback when no local settings exist.

An AddressSanitizer build uses the same local JUCE checkout:

```powershell
cmake -S Source/Application -B Source/Application/build/asan -G "Visual Studio 17 2022" -A x64 -DCPM_JUCE_SOURCE=E:/BehringerKick/0716-AAF/Source/Application/build/deps/JUCE -DTIMESTAMP=OFF -DUSE_ASAN_IGNITE=ON
cmake --build Source/Application/build/asan --config RelWithDebInfo --target Ignite IgniteDesktopTests --parallel 4
$env:PATH='C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64;'+$env:PATH
$env:ASAN_OPTIONS='log_path=asan_workflow:halt_on_error=1:malloc_context_size=10'
.\scripts\workflow-windows.ps1 -Configuration RelWithDebInfo -BuildDirectory asan -NodeEdit -AudioDevice
```

Adjust the JUCE and MSVC paths on another machine. Sanitizer reports are written beside the test executable. Sanitizer UI construction is slower than Release; the workflow timeout defaults to 120 seconds per process.

## Validation evidence

The AddressSanitizer run at `Source/Application/build/workflow/20261028-135946` passed **58 create/edit checks and 49 fresh-process verification checks**, both with normal exit code 0 and no sanitizer report. This includes simulated interruption/recovery and real audio through the JIB TRUE XT 2 headphones at 44.1 kHz. The final Release workflow at `Source/Application/build/workflow/20261028-140225` repeats all **107 checks** successfully, including interrupted-save recovery. Release smoke cases with and without a bank pass at `Source/Application/build/smoke/20261028-140244`. The native suite passes **50 checks in Release/ASan and 51 in Debug**; the extra Debug check measures allocation release.

The earlier failing runs remain in the ignored workflow directory for comparison: `20261028-131916` demonstrates dropped track batches, `20261028-132459` demonstrates the copied-song scene reference, and `20261028-134254` demonstrates the skipped pattern Save command. The original note-edit sanitizer report is `build/asan/apps/ignite/Ignite_artefacts/RelWithDebInfo/asan_workflow.3444`; the later delayed UI scene-null report is `asan_message_thread.28980`. These build artifacts are local, not bundled repository assets.

## Save recovery and boundaries

Recovery copies live under `Ignite_folder/.song-save/backups`, `.scene-save/<song>/backups` and `.pattern-save/backups`, outside the visible libraries. A full song save supersedes its earlier scene recovery copies. A successful replacement retains the preceding version. On startup, a missing library directory is restored from its recovery copy. Explicit deletion/wipe removes the corresponding recovery copies, so deleted songs do not reappear.

Song replacement preserves the song's existing settings files while replacing its scene contents. Invalid song/scene names and incomplete directory writes fail before replacing the old song. The native tests inject failed writes and locked-file replacement failures.

These changes do not make Save All a single transaction across every library and settings file, or guarantee survival of every power-loss/storage failure. The desktop.3 pass below adds checked settings writers and a close-with-unsaved-changes prompt. Use one process per data directory.

Hardware CV/Gate, DIN and USB endpoints, external clock synchronisation, recording from physical devices, device firmware images and factory/update services remain unqualified. The inherited singleton UI still retains allocations in Debug; Release is the normal-use build. These tests establish the listed desktop workflows, not every screen and hardware combination.

## Desktop feature completion

The desktop additions are described in [DESKTOP_FEATURES.md](DESKTOP_FEATURES.md). The feature runner builds an arrangement, injects internal MIDI through the Windows router, switches to an original generated SF2 at bank 42/program 7, records notes, saves and restores libraries, then verifies them in a fresh process. It also captures all four settings tabs. No physical MIDI input devices were available; internal MIDI tests were selected by the user.

| Area | Repair and verification |
|---|---|
| Record event filters | Several lookups used `count(parameter != 0)`, querying a boolean key instead of the requested parameter. Use the actual parameter ID. |
| Step recording | Ignore other channels before advancing the chord/step counters, reject a missing pattern, and preserve correct note step IDs, note-off state and time-base tick lengths. Test two accepted notes and a rejected channel. |
| Live recording | Reject unarmed tracks, guard two-byte program messages, flush pending note-off events before the Record Stop transition and save, and regenerate sequences synchronously on Windows. Test velocity/duration and immediate stop/save/reopen. |
| Partial track banks | Saved scenes with eight tracks triggered an ASan container overflow when the UI consumed a rounded-up sixteen-track response. Bound consumption by the actual response length and clamp selection. Fresh-process feature verification now passes. |
| Missing-font mix | Initialise each track's audio-buffer view before the no-font early return, so mixing a silent track has a valid buffer. |
| Bank selection | Enumerate real SF2 preset metadata, preserve explicit bank/program selection in the existing instrument integer, and suppress stale preset playback after a failed program lookup. Invalid replacement banks retain the previous bank. |
| Preview | Use a guarded model entry point and the actual loaded font's preset count. Test audible rendering without a sequence assignment. Timed callbacks use a safe component pointer and preview generation. |
| Backup/restore | Stage Windows firmware backups and ZIP restores, retain recovery copies, and validate saved XML before replacement. JUCE accepts some truncated opening tags as an element; additionally require a known saved-data root and complete closing tag. Test corrupt-data rejection, traversal rejection, valid restore and reopen. |
| MIDI lifecycle | Release the model lock before closing MIDI inputs because close joins callbacks. Clear the output closure under the model lock before destroying its router. Give the five desktop host slots distinct names. |
| Supporting libraries | Repair stale AudIO parameter/callback signatures and duplicate definitions so its three static libraries build. Packet-protocol tests caught an uninitialised command return and empty-payload parser transition; both are repaired. These separate libraries are not linked by the simulator. |

Reproduce with `scripts/features-windows.ps1 -Configuration Release`; for sanitised checks add `-Configuration RelWithDebInfo -BuildDirectory asan` and the MSVC sanitizer runtime directory to PATH. The native suite now contains 54 checks (55 in Debug). The feature run adds 21 checks plus four fresh-process checks to its 50-check arrangement setup.

Final evidence for this pass, relative to `Source/Application/build`: Release `features/20261028-151408` (75 checks), `workflow/20261028-151350` (107 checks) and `smoke/20261028-151425` (seven checks per case); ASan `features/20261028-151101` (75), `workflow/20261028-151204` (107, real audio and recovery) and `workflow/20261028-151308` (91, no font). All passed with normal exits, and the final ASan runs produced no sanitizer report. Native Release/ASan pass 54/54, Debug passes 55/55, and packet-protocol checks pass 5/5.


## Desktop.3 correctness pass

- Internal MIDI: audible tests for bank/program selection (including bank 129), missing presets, pitch bend/RPN range, expression, sustain with repeated pitches, modulation/pressure vibrato, reset controllers and both all-notes/all-sound-off paths. The router is exercised on channel 16 as well as channel filtering. Panic rebuilds the synth channel after reset so subsequent notes sound.
- Save/close: pending changes cover normal model editing and direct note edits; three-way close policy has lifetime-safe asynchronous callbacks. Full save includes all loaded library members and preserves per-song settings. Fifteen settings writers, Sculpt, main info and pattern index updates use checked temporary-file replacement. A real locked destination tests failure, preservation of the previous file, staying open, and successful retry. Export aborts on save failure. Already-written individual saves are not rolled back by Discard.
- Editor: direct step-data updates now find the requested pattern rather than always stopping after the first library entry. The previously stubbed step-insertion API now shifts independent step data across bars, fixes IDs/pointers, notifies sequence listeners and refuses overflow. Ctrl+Insert exposes it in Pattern view. Undo baseline replacement removes duplicate defaults; empty pattern undo is safe. MIDI and Sculpt history captured pre-edit state but popped it before applying it; undo now restores the immediately previous value.
- Performance: Sculpt uses signed arithmetic and clamping, preserves velocity 127, applies random-relative offsets to existing values and rebuilds pitch maps separately to avoid iterator invalidation. Its UI enable command targets the Sculpt module. Force-to-scale wraps pitch classes correctly at octave boundaries and uses a wide intermediate before clamping. Scene-length fill compares the current scene to the captured scene ID, rather than a bar ID.

The correctness runner creates its own four-preset synthetic SF2 and disposable libraries. It tests real model writes, audio output, selected UI editor paths and reopening in a new process. Physical MIDI/CV/clock qualification, every combination of performance controls and full SF2 modulator compatibility remain outside this verification. At desktop.3 the remaining limits included one shared mono SF2, eight voices per track, no MIDI file import/export, no WAV bounce and no Ignite VST. Desktop.4 addresses stereo, polyphony and separate banks below.


## Desktop.4 audio and SoundFonts

Corrects reversed EQ shelves and effect-order slots, replaces shared mono IIR state with per-channel processors, fixes out-of-bounds `chorusStatus[1]`, applies reverb parameters to the processor, and replaces global/incorrectly scaled release overrides with per-synth timing. EQ exposes -12 to +12 dB and the model accepts cuts. Zero dB bypasses; review older saved nonzero gains because their semantics changed.

Windows tracks render planar stereo through effects and mixing. A two-channel block adapter preserves partial driver blocks; resampling/output preserve channels and mono output downmixes. A compatibility mono render remains for existing firmware/offline callers. Voice limits are 8/16/32/64/128, default 32; changes stop transport and clear notes.

Instrument XML stores an optional UTF-8 SF2 path, empty meaning default. Construction, copying, undo and save/reload preserve it. Sample data is cached by path while track copies keep separate voices/controllers/effects. Browser/preview banks are independent of assignments, and assignments validate presets first. Backups store paths/programs with samples external. Missing assigned/default banks are tested in fresh processes.

Run `scripts/sound-windows.ps1` for original stereo/alternate sine fixtures and disposable data. Coverage includes rendered pitches, pan, concurrent banks, preview, voice allocation, EQ frequency response, stereo isolation, chorus, reverb tails, release timing, control ranges, block adaptation, undo and backup round trips. Complete SF2 modulators, every bank format and maximum aggregate real-time polyphony remain outside this qualification.

## Desktop.5 media and persistence

`DesktopMediaSnapshot` serializes current patterns, assigned instruments, scene timing, transforms/scales, mute/solo state and track effects under the model lock. `DesktopMediaJob` creates a private temporary workspace and a child process with a scoped data directory. The parent remains responsive, retains its unsaved edits and never changes its process environment. The child uses inline sequencer pulses, finite scene order and variable-size audio blocks without the live clock, MIDI routing, monitor mix, CV generation or performance recording.

WAV output is 48 kHz stereo float32, with sample positions rounded from each pulse's tempo and a final two-second tail. MIDI is type 1, 960 PPQ with a conductor tempo/marker track. Output is written beside the destination under a unique pending name, validated and atomically replaced only after child success. Missing fonts, cancellation, child failure and an actual Windows destination lock preserve the old output. Destruction cancels only the owned child and removes its temporary data.

`DesktopMidiImport` parses and bounds the complete file before adding independent library patterns, using track/channel groups and 16-bar chunks with tick offsets. It rejects SMPTE/invalid/oversized material, retains final controller events, repairs unmatched note edges and reports coincident-note merging. Import does not assign tracks or alter tempo, and normal Save All persists the new patterns. Full arrangement/tempo-map import remains outstanding.

The media review exposed three existing engine issues. Transpose was changing auxiliary controller numbers and pitch-bend data; it now transforms only note and poly-pressure pitches. SongManager stopped notes at every global bar; it now releases them at scene boundaries/manual scene changes. Effects lived only in DSP objects; canonical values now travel with TrackData, track copies and song XML, restore after synth reconstruction, and mark song edits dirty. Files without the new effect flag retain the previous default behavior.

The 88-check runner combines 50 arrangement setup, 33 media checks and five fresh-process checks. It generates original MIDI/SF2 fixtures, verifies rendered scene/song frame counts and stereo format, multi-tempo MIDI, an unsaved snapshot distinct from later parent edits, controller/pressure transpose, held notes across bars, stems, failure/cancellation/locked-output protection, clip persistence and saved effects. Run `scripts/media-windows.ps1`; sanitizer use follows the sound suite. Export covers main patterns once per scene and excludes live fills, Sculpt gestures and incoming MIDI. Random directions generate a fresh pass, and only the final WAV scene receives an extra tail.
