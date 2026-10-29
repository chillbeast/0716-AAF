"""Generate an original sine-wave SF2 fixture. This is a test tone, not a GM bank."""
import math
from pathlib import Path
import struct
import sys


def chunk(tag, data):
    return tag + struct.pack('<I', len(data)) + data + (b'\0' if len(data) % 2 else b'')


def name(text):
    return text.encode('ascii').ljust(20, b'\0')


def make_font(bank=0, program=0, presets=None, stereo=False):
    rate = 22050
    count = rate
    pcm = [int(16000 * math.sin(2 * math.pi * 220 * i / rate)) for i in range(count)] + [0] * 46
    info = chunk(b'LIST', b'INFO' + chunk(b'ifil', struct.pack('<HH', 2, 1))
                 + chunk(b'INAM', b'Ignite test tone\0') + chunk(b'ISFT', b'Ignite desktop tests\0'))
    sdta = chunk(b'LIST', b'sdta' + chunk(b'smpl', struct.pack(f'<{len(pcm)}h', *pcm)))
    specs = presets if presets is not None else [(bank, program, 0)]
    phdr = b''.join(struct.pack('<20sHHHIII', name('Test sine' if len(specs) == 1 else f'Tone {b}:{p}'), p, b, i, 0, 0, 0)
                    for i, (b, p, shift) in enumerate(specs))
    phdr += struct.pack('<20sHHHIII', name('EOP'), 0, 0, len(specs), 0, 0, 0)
    pbag = b''.join(struct.pack('<HH', i * 2, 0) for i in range(len(specs) + 1))
    pgen = b''.join(struct.pack('<HHHH', 51, shift & 0xffff, 41, 0) for b, p, shift in specs) + bytes(4)
    inst = struct.pack('<20sH', name('Test sine'), 0) + struct.pack('<20sH', name('EOI'), 2 if stereo else 1)
    shdr = struct.pack('<20sIIIIIBbHH', name('Sine A3'), 0, count, 0, count, rate, 57, 0, 0, 1)
    shdr += struct.pack('<20sIIIIIBbHH', name('EOS'), count, count, count, count, rate, 0, 0, 0, 1)
    igen = struct.pack('<HHHHHHHH', 54, 1, 38, (-1200) & 0xffff, 53, 0, 0, 0)
    ibag = struct.pack('<HHHH', 0, 0, 3, 0)
    if stereo:
        # Two independently panned regions with different pitches reveal any
        # channel copying or effect-state sharing in the stereo signal path.
        igen = b''.join(struct.pack('<HHHHHHHHHH', 17, pan & 0xffff, 51, shift, 54, 1, 38, (-1200) & 0xffff, 53, 0)
                        for pan, shift in [(-500, 0), (500, 12)]) + bytes(4)
        ibag = struct.pack('<HHHHHH', 0, 0, 5, 0, 10, 0)
    pdta = chunk(b'LIST', b'pdta'
                 + chunk(b'phdr', phdr)
                 + chunk(b'pbag', pbag)
                 + chunk(b'pmod', bytes(10))
                 + chunk(b'pgen', pgen)
                 + chunk(b'inst', inst)
                 + chunk(b'ibag', ibag)
                 + chunk(b'imod', bytes(10))
                 + chunk(b'igen', igen)
                 + chunk(b'shdr', shdr))
    return chunk(b'RIFF', b'sfbk' + info + sdta + pdta)


if __name__ == '__main__':
    target = Path(sys.argv[1])
    target.parent.mkdir(parents=True, exist_ok=True)
    if len(sys.argv) > 2 and sys.argv[2] == '--stereo':
        target.write_bytes(make_font(stereo=True))
    elif len(sys.argv) > 2 and sys.argv[2] == '--alternate':
        target.write_bytes(make_font(presets=[(0, 0, 12)]))
    elif len(sys.argv) > 2 and sys.argv[2] == '--midi':
        target.write_bytes(make_font(presets=[(0, 0, 0), (0, 1, 12), (42, 7, -12), (129, 5, 7)]))
    else:
        target.write_bytes(make_font(int(sys.argv[2]) if len(sys.argv) > 2 else 0, int(sys.argv[3]) if len(sys.argv) > 3 else 0))
