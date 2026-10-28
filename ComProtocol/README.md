# ComProtocol

`PktMgr` is the framed packet protocol (header, type, direction, ID, payload and CRC) used between the
application and the device's MCU for buttons, LEDs, pads, knobs, sliders, MIDI and CV. It is a small C++
class with no external dependencies.

Build and test the standalone library on Windows:

```powershell
cmake -S ComProtocol -B Source/Application/build/protocol -G 'Visual Studio 17 2022' -A x64
cmake --build Source/Application/build/protocol --config Release
ctest --test-dir Source/Application/build/protocol -C Release --output-on-failure
```

`IgniteProtocolTests` exercises framing, command lookup, payload round trips, CRC rejection and empty-payload packets. These tests do not establish physical MCU compatibility or a complete parser audit.

## License

GNU Affero General Public License v3.0 or later, see [LICENSE](LICENSE).
Copyright (c) MUSIC Tribe.
