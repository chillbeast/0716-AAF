"""Generate an original sine-wave SF2 fixture. This is a test tone, not a GM bank."""
import math
from pathlib import Path
import struct
import sys


def chunk(tag, data):
    return tag + struct.pack('<I', len(data)) + data + (b'\0' if len(data) % 2 else b'')


def name(text):
    return text.encode('ascii').ljust(20, b'\0')


def make_font(bank=0, program=0):
    rate = 22050
    count = rate
    pcm = [int(16000 * math.sin(2 * math.pi * 220 * i / rate)) for i in range(count)] + [0] * 46
    info = chunk(b'LIST', b'INFO' + chunk(b'ifil', struct.pack('<HH', 2, 1))
                 + chunk(b'INAM', b'Ignite test tone\0') + chunk(b'ISFT', b'Ignite desktop tests\0'))
    sdta = chunk(b'LIST', b'sdta' + chunk(b'smpl', struct.pack(f'<{len(pcm)}h', *pcm)))
    phdr = struct.pack('<20sHHHIII', name('Test sine'), program, bank, 0, 0, 0, 0)
    phdr += struct.pack('<20sHHHIII', name('EOP'), 0, 0, 1, 0, 0, 0)
    inst = struct.pack('<20sH', name('Test sine'), 0) + struct.pack('<20sH', name('EOI'), 1)
    shdr = struct.pack('<20sIIIIIBbHH', name('Sine A3'), 0, count, 0, count, rate, 57, 0, 0, 1)
    shdr += struct.pack('<20sIIIIIBbHH', name('EOS'), count, count, count, count, rate, 0, 0, 0, 1)
    pdta = chunk(b'LIST', b'pdta'
                 + chunk(b'phdr', phdr)
                 + chunk(b'pbag', struct.pack('<HHHH', 0, 0, 1, 0))
                 + chunk(b'pmod', bytes(10))
                 + chunk(b'pgen', struct.pack('<HHHH', 41, 0, 0, 0))
                 + chunk(b'inst', inst)
                 + chunk(b'ibag', struct.pack('<HHHH', 0, 0, 3, 0))
                 + chunk(b'imod', bytes(10))
                 + chunk(b'igen', struct.pack('<HHHHHHHH', 54, 1, 38, (-1200) & 0xffff, 53, 0, 0, 0))
                 + chunk(b'shdr', shdr))
    return chunk(b'RIFF', b'sfbk' + info + sdta + pdta)


if __name__ == '__main__':
    target = Path(sys.argv[1])
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(make_font(int(sys.argv[2]) if len(sys.argv) > 2 else 0, int(sys.argv[3]) if len(sys.argv) > 3 else 0))
