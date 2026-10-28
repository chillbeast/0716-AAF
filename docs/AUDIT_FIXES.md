# Desktop reliability repairs, 2026-10-28

This pass uses the adjacent KICK audit as a reference for model lifetime, message-thread UI updates, safe persistence and sanitizer-backed workflow tests. IGNITE has its own engine; fixes were adapted to its source rather than copied wholesale. All changes remain local on `desktop-build`.

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

These changes do not make Save All a single transaction across every library and settings file, or guarantee survival of every power-loss/storage failure. Some inherited settings writers still need an audit. The app has no new close-with-unsaved-changes prompt. Use one process per data directory.

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
