#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Original tiny SF2 banks that exercise synthesis semantics, not private samples."""
from pathlib import Path
import math
import struct
import sys
from make_test_soundfont import chunk, name


def bank(presets, low24=False, linked=False):
    rate, count = 22050, 22050
    wave = [int(16000 * math.sin(2 * math.pi * 220 * i / rate)) for i in range(count)] + [0] * 46
    if low24:
        pcm = [0] * len(wave)
        low = bytes(int(127 + 126 * math.sin(2 * math.pi * 220 * i / rate)) for i in range(count)) + bytes(46)
    else:
        pcm = wave
        low = b''
    if linked:
        pcm += [int(16000 * math.sin(2 * math.pi * 440 * i / rate)) for i in range(count)] + [0] * 46
    info = chunk(b'LIST', b'INFO' + chunk(b'ifil', struct.pack('<HH', 2, 4 if low24 else 1))
                 + chunk(b'INAM', b'IGNITE SF2 compatibility fixtures\0') + chunk(b'ISFT', b'IGNITE original tests\0'))
    sdta = chunk(b'LIST', b'sdta' + chunk(b'smpl', struct.pack(f'<{len(pcm)}h', *pcm))
                 + (chunk(b'sm24', low) if low24 else b''))
    phdr, pbag, pgen, pmod, inst, ibag, igen, imod = [], [], [], [], [], [], [], []

    def generators(values):
        return [struct.pack('<HH', op, amount & 0xffff) for op, amount in values]

    def modulators(values):
        return [struct.pack('<HHhHH', *value) for value in values]

    for program, spec in enumerate(presets):
        phdr.append(struct.pack('<20sHHHIII', name(spec['name']), program, 0, len(pbag), 0, 0, 0))
        if spec.get('preset_global'):
            pbag.append(struct.pack('<HH', len(pgen), len(pmod)))
            pmod += modulators(spec['preset_global'])
        pbag.append(struct.pack('<HH', len(pgen), len(pmod)))
        pmod += modulators(spec.get('preset', []))
        pgen += generators([(41, program)])
        inst.append(struct.pack('<20sH', name(spec['name']), len(ibag)))
        if spec.get('global'):
            ibag.append(struct.pack('<HH', len(igen), len(imod)))
            imod += modulators(spec['global'])
        for sample in range(2 if linked else 1):
            ibag.append(struct.pack('<HH', len(igen), len(imod)))
            imod += modulators(spec.get('mods', []))
            values = [(54, 1), (38, -1200)] + spec.get('generators', [])
            if linked:
                values += [(17, -500 if sample == 0 else 500)]
            igen += generators(values + [(53, sample)])
    phdr.append(struct.pack('<20sHHHIII', name('EOP'), 0, 0, len(pbag), 0, 0, 0))
    pbag.append(struct.pack('<HH', len(pgen), len(pmod)))
    pgen.append(bytes(4)); pmod.append(bytes(10))
    inst.append(struct.pack('<20sH', name('EOI'), len(ibag)))
    ibag.append(struct.pack('<HH', len(igen), len(imod)))
    igen.append(bytes(4)); imod.append(bytes(10))
    shdr = struct.pack('<20sIIIIIBbHH', name('Left' if linked else 'Tone'), 0, count, 0, count, rate, 57, 0, 1 if linked else 0, 4 if linked else 1)
    if linked:
        start = count + 46
        shdr += struct.pack('<20sIIIIIBbHH', name('Right'), start, start + count, start, start + count, rate, 57, 0, 0, 2)
    shdr += struct.pack('<20sIIIIIBbHH', name('EOS'), len(pcm), len(pcm), len(pcm), len(pcm), rate, 0, 0, 0, 1)
    tables = [(b'phdr', phdr), (b'pbag', pbag), (b'pmod', pmod), (b'pgen', pgen),
              (b'inst', inst), (b'ibag', ibag), (b'imod', imod), (b'igen', igen)]
    pdta = chunk(b'LIST', b'pdta' + b''.join(chunk(tag, b''.join(values)) for tag, values in tables) + chunk(b'shdr', shdr))
    return chunk(b'RIFF', b'sfbk' + info + sdta + pdta)


def main():
    root = Path(sys.argv[1]); root.mkdir(parents=True, exist_ok=True)
    cc = lambda number, curve=0: 128 | number | curve
    att = lambda amount: (cc(20), 48, amount, 0, 0)
    specs = [
        {'name': 'Baseline'},
        {'name': 'CC pitch', 'mods': [(cc(21), 51, 12, 0, 0)]},
        {'name': 'Amount source pan', 'mods': [(cc(22), 17, 500, cc(23), 0)]},
        {'name': 'Velocity override', 'mods': [(0x0502, 48, 0, 0, 0)]},
        {'name': 'Local overrides global', 'global': [att(120)], 'mods': [att(240)]},
        {'name': 'Preset adds instrument', 'mods': [att(120)], 'preset': [att(120)]},
        {'name': 'Negative source', 'mods': [(cc(22, 0x100), 17, 500, 0, 0)]},
        {'name': 'Bipolar source', 'mods': [(cc(22, 0x200), 17, 500, 0, 0)]},
        {'name': 'Linear curve', 'mods': [(cc(24), 48, 600, 0, 0)]},
        {'name': 'Concave curve', 'mods': [(cc(24, 0x400), 48, 600, 0, 0)]},
        {'name': 'Convex curve', 'mods': [(cc(24, 0x800), 48, 600, 0, 0)]},
        {'name': 'Switch curve', 'mods': [(cc(24, 0xc00), 48, 600, 0, 0)]},
        {'name': 'Preset reverb', 'generators': [(16, 1000)]},
        {'name': 'Preset chorus', 'generators': [(15, 1000)]},
        {'name': 'Instrument global', 'global': [att(120)]},
        {'name': 'Preset local overrides', 'mods': [att(120)], 'preset_global': [att(60)], 'preset': [att(120)]},
        {'name': 'Poly pressure pan', 'mods': [(10, 17, 500, 0, 0)]},
        {'name': 'Channel pressure pitch', 'mods': [(13, 51, 12, 0, 0), (13, 6, 0, 0, 0)]},
        {'name': 'Wheel default override', 'mods': [(cc(1), 6, 0, 0, 0)]},
    ]
    (root / 'matrix.sf2').write_bytes(bank(specs))
    (root / 'low24.sf2').write_bytes(bank([{'name': '24 bit low samples'}], low24=True))
    (root / 'linked.sf2').write_bytes(bank([{'name': 'Linked stereo'}], linked=True))
    print('Created original SF2 matrix, linked stereo and 24-bit fixtures:', root)


if __name__ == '__main__':
    main()
