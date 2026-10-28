# Windows desktop build

The `desktop-build` branch runs the IGNITE application and its real firmware model on Windows x64. The simulator adds an interactive SVG hardware faceplate around the live touchscreen. The labels, knobs and controls remain sharp when resized. `--touchscreen` opens only the touchscreen.

## Requirements

- Visual Studio 2022 with Desktop development with C++ and a Windows SDK.
- CMake 3.20 or newer, Git, and Python 3 for the regression checks and generated test font.
- JUCE **7.0.12**, downloaded automatically. No private Music Tribe repository or ASIO SDK is needed.

From the repository root in PowerShell:

```powershell
.\scripts\build-windows.ps1 -DebugBuild -Package
```

This builds Release and Debug applications/helpers, the standalone AudIO libraries and packet protocol, runs the Release workflow/desktop feature/smoke checks and native/protocol checks, then packages Release with corresponding application/library source under `Source/Application/build/packages`. No private SoundFonts are included. Add `-SkipChecks` for an incremental build only. For application-only commands:

```powershell
cmake -S Source/Application -B Source/Application/build/win -G 'Visual Studio 17 2022' -A x64 -DTIMESTAMP=OFF
cmake --build Source/Application/build/win --config Release --target Ignite IgniteDesktopTests IgniteSoundFontProbe --parallel 4
ctest --test-dir Source/Application/build/win -C Release --output-on-failure
```

The first configure builds JUCE's `juceaide` tool. To use a dedicated existing JUCE checkout, add `-DCPM_JUCE_SOURCE=C:/path/to/JUCE`. Configuration applies the supplied multitouch patch to that checkout and checks whether it is already applied. Use a separate checkout if another project needs different patches.

The executable is `Source/Application/build/win/apps/ignite/Ignite_artefacts/Release/Ignite.exe`. Its adjacent `assets` folder is required. It is a desktop build, not a flashable device image.

## Run with a SoundFont

```powershell
.\scripts\run-ignite.ps1 -SoundFont 'D:\path\to\your-bank.sf2'
.\scripts\run-ignite.ps1 -Touchscreen
```

The runner also reads an optional, ignored `Source/Application/build/desktop-settings.json`:

```json
{"SoundFont": "D:\\path\\to\\your-bank.sf2"}
```

Runtime lookup is `IGNITE_SOUNDFONT`, the saved `SoundFont` preference in `desktop.json`, `Ignite.sf2` beside the executable, then the original platform path. An explicit invalid override disables internal sound; it does not silently load another bank. Sequencer/UI operation remains available without a bank. Open **DESKTOP SETTINGS → SoundFonts** for actual bank/program browsing, preview and instrument assignment, including non-GM fonts. See [DESKTOP_FEATURES.md](DESKTOP_FEATURES.md).

`-DIGNITE_SOUNDFONT=C:/path/bank.sf2` at configure time optionally copies a font to the output. Keep private sample libraries outside the source tree; the local runner can load them directly.

Windows data defaults to `%APPDATA%\BehringerIgnite`, with songs, instruments, patterns and settings under `Ignite_folder`. Override it with `IGNITE_DATA_DIR` or the runner's `-DataDirectory`. Set `IGNITE_LOG` for a file log and `IGNITE_SNAPSHOT` for a startup PNG.

## Workflow and repair checks

See [AUDIT_FIXES.md](AUDIT_FIXES.md) for the reliability repairs, AddressSanitizer setup, exact workflow coverage and remaining boundaries. Run the create/edit/save/reopen, playback and recovery checks with:

```powershell
.\scripts\workflow-windows.ps1 -Configuration Release -NodeEdit -AudioDevice -SimulateInterruptedSave
.\scripts\features-windows.ps1 -Configuration Release
```

The Windows output may negotiate 44.1 kHz even though the synth runs at 48 kHz. The desktop stream now converts between those rates. Pause and Stop close the output; Play/Resume opens it again. Audio startup failures and the negotiated device format are recorded in the log.

## Verification tools

```powershell
python scripts/soundfont_inventory.py 'D:\Old Samples' --output Source/Application/build/soundfont-inventory.json
.\scripts\smoke-windows.ps1 -Configuration Release -SoundFont 'D:\path\to\your-bank.sf2'
& Source/Application/build/win/tests/Release/IgniteSoundFontProbe.exe 'D:\path\to\your-bank.sf2' 'Source/Application/build/probe.wav'
```

The inventory reads preset headers without loading sample data. The probe renders five representative instruments/drums through the firmware's TinySoundFont header to a mono 48 kHz WAV. It does not measure physical audio latency. Smoke tests use fresh data directories, check panel navigation and a real firmware track, capture a screenshot, and require normal process exit. Results live under `Source/Application/build/smoke`.

Debug uses the same build tree with `--config Debug`. The inherited UI retains many heap-allocated singleton objects: MSVC Debug's per-allocation CRT leak report can make process exit extremely slow. Use Release for the application smoke test and normal use; memory ownership remains an audit item. No leak-report suppression is applied. `IGNITE_WARNINGS_AS_ERRORS=ON` restores upstream's MSVC Release warning policy; existing source warnings currently require repairs before using it. Set `IGNITE_DESKTOP_TESTS=OFF` when only the application is wanted; it defaults off when cross-compiling.

## Manual and release packaging

The manual source is `docs/manual/USER_MANUAL.md`. To regenerate its self-contained HTML and PDF, install Python-Markdown (`python -m pip install Markdown`), then run `python scripts/build-manual.py --pdf`. PDF rendering uses an installed Edge/Chrome browser in headless mode with a separate temporary profile; `--browser` selects another Chromium executable. These tools are only needed for documentation development.

After committing the release source, create the versioned Windows package with `scripts/package-windows.ps1 -Version v0.3.3-desktop.1`. It includes the tested Release executable, assets, manual, license, release notes and corresponding source. `BUILD.json` records the source commit, whether the checkout is dirty and the executable SHA-256. The ZIP builder checks its entries and CRCs and refuses included SF2 files.

## Device build boundary

The hardware uses an i.MX8M Mini/Cortex-A53 platform. See [Linux/README.md](../Linux/README.md) for Buildroot and SDK instructions. Device-image work requires a Linux host plus replacements for omitted MCU firmware, Wi-Fi firmware and the proprietary PC transfer service. No device image has been built or flashed in this pass. CV/Gate, DIN ports, sync jacks, firmware updates and hardware calibration need physical acceptance tests.
