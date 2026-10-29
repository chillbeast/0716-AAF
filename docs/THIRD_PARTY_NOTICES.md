# Desktop third-party components

The application and community changes are AGPL-3.0-or-later; see the root LICENSE.
Bundled components retain their own copyright and license notices.

The Windows desktop sound engine uses **FluidSynth 2.6.1**, commit
`71e85b2ca6bf48641ba7e2261d8f2640ae473773`, under **LGPL-2.1-or-later**.
Copyright belongs to the FluidSynth authors named in its source files and AUTHORS.
Upstream: <https://github.com/FluidSynth/fluidsynth>.

Release packages include `FluidSynth-LICENSE.txt` and the corresponding source at
`source/Source/Application/third_party/fluidsynth`. The integration disables unused
drivers/formats and guards upstream's standalone test/doc registration; no synth
algorithm changes are applied. CMake automatically uses this packaged source.
To modify the library and relink, edit that copy and run
`source/scripts/build-windows.ps1 -SkipChecks` using the prerequisites in
[BUILD_WINDOWS.md](BUILD_WINDOWS.md). The complete application source and build
scripts are included. No SoundFont sample banks are distributed.

JUCE 7.0.12 retains its upstream licensing and the included multitouch patch.
TinySoundFont (MIT) remains in the firmware/non-Windows path and legacy native
tests. Its license is in `Source/Application/modules/Firmware/Application/Model/tsf.h`.
Other bundled libraries retain the notices accompanying their sources.


The original First Light demo MIDI (`assets/demo/First Light Demo.mid`) is CC0-1.0.
It contains note/controller data only. SoundFont sample libraries remain external
and retain their own licenses; rendered demo audio is not included in releases.
