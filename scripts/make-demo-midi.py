#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Write the original CC0 First Light demo; no MIDI library or samples needed."""
from pathlib import Path
import struct

PPQ = 480
BAR = 4 * PPQ
END = 16 * BAR


def vlq(value):
    result = [value & 127]
    while value >> 7:
        value >>= 7
        result.insert(0, 128 | (value & 127))
    return bytes(result)


def meta(kind, text):
    data = text.encode("ascii")
    return bytes([255, kind]) + vlq(len(data)) + data


def track(events):
    data = bytearray()
    previous = 0
    for tick, order, message in sorted(events):
        data += vlq(tick - previous) + message
        previous = tick
    data += vlq(END - previous) + b"\xff\x2f\x00"
    return b"MTrk" + struct.pack(">I", len(data)) + data


def make_demo():
    conductor = [(0, 0, meta(3, "First Light - IGNITE Demo")),
                 (0, 1, b"\xff\x51\x03" + round(60_000_000 / 112).to_bytes(3, "big")),
                 (0, 2, b"\xff\x58\x04\x04\x02\x18\x08")]
    for bar, name in [(0, "01 Intro"), (4, "02 Groove"), (8, "03 Lift"), (12, "04 Outro")]:
        conductor.append((bar * BAR, 3, meta(6, name)))
    parts = []
    # Name, channel, GM program (zero based), volume, pan, native reverb send.
    specs = [("Drums", 9, 0, 90, 64, 0), ("Finger Bass", 0, 33, 88, 64, 0),
             ("Electric Piano", 1, 4, 83, 53, 24), ("Marimba Arp", 2, 12, 78, 83, 16),
             ("Square Lead", 3, 80, 68, 70, 12)]
    for name, channel, program, volume, pan, reverb in specs:
        events = [(0, 0, meta(3, name))]
        for cc, value in [(0, 1 if channel == 9 else 0), (32, 0), (7, volume), (10, pan), (91, reverb)]:
            events.append((0, 1, bytes([176 | channel, cc, value])))
        events.append((0, 2, bytes([192 | channel, program])))
        parts.append(events)

    def note(part, bar, step, pitch, length, velocity):
        channel = specs[part][1]
        tick = round(bar * BAR + step * PPQ / 4)
        end = min(END - 1, tick + round(length * PPQ / 4))
        assert 0 <= tick < end <= END
        parts[part] += [(tick, 4, bytes([144 | channel, pitch, velocity])),
                        (end, 3, bytes([128 | channel, pitch, 0]))]

    roots = [45, 41, 36, 43]
    chords = [[57, 60, 64, 67], [53, 57, 60, 64], [52, 55, 59, 64], [55, 59, 62, 65]]
    motifs = [[76, 74, 72, 69, 72, 74], [72, 69, 67, 65, 69, 72],
              [67, 71, 72, 76, 74, 72], [74, 71, 67, 69, 71, 74]]
    for bar in range(16):
        section = bar // 4
        harmony = 0 if bar == 15 else bar % 4
        root, chord = roots[harmony], chords[harmony]
        sparse = section == 0 or bar >= 14
        for step in ([0, 8] if sparse else [0, 4, 8, 12]):
            note(0, bar, step, 36, 0.7, 102 if step in [0, 8] else 89)
        for step in [4, 12]:
            if bar != 15:
                note(0, bar, step, 38, 0.7, 78)
                if section == 2:
                    note(0, bar, step, 39, 0.7, 53)
        for step in range(0, 16, 2 if sparse else 1):
            if bar == 15 and step >= 8:
                continue
            note(0, bar, step, 42, 0.4, 59 if step % 4 == 0 else 38 + (step % 3) * 6)
        if section in [1, 2]:
            note(0, bar, 14, 46, 1.3, 48)
        if bar in [3, 7, 11]:
            for step in [14, 15]:
                note(0, bar, step, 38, 0.6, 50 + (step - 14) * 12)
        for step, pitch, length in ([(0, root, 6), (8, root, 6)] if sparse else
                                    [(0, root, 2.5), (3, root, 1), (6, root + 12, 1.5),
                                     (8, root, 2.5), (11, root + 7, 1), (14, root + 12, 1.5)]):
            note(1, bar, step, pitch, length, 89 if step % 8 == 0 else 76)
        for step, length in ([(0, 13)] if sparse else [(0, 5.5), (7, 2.5), (12, 3.5)]):
            for pitch in chord:
                note(2, bar, step, pitch, length, 67 if sparse else 74)
        if 4 <= bar < 14:
            for step in range(0, 16, 2):
                pitch = chord[(step // 2 + bar) % 4] + 12
                note(3, bar, step, pitch, 1.2, 68 if step % 4 == 0 else 55)
        if 8 <= bar < 12:
            for step, pitch in zip([0, 3, 6, 8, 11, 14], motifs[harmony]):
                note(4, bar, step, pitch, 1.7, 64 if step % 8 == 0 else 57)
        elif bar == 15:
            note(4, bar, 0, 69, 12, 54)
    tracks = [track(conductor)] + [track(events) for events in parts]
    return b"MThd" + struct.pack(">IHHH", 6, 1, len(tracks), PPQ) + b"".join(tracks)


if __name__ == "__main__":
    destination = Path(__file__).resolve().parents[1] / "Source/Application/assets/demo/First Light Demo.mid"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(make_demo())
    print(destination)
