"""Inspect SF2 preset headers without loading sample data or changing the library."""
import argparse
import json
from pathlib import Path
import struct


def presets(path):
    with path.open('rb') as stream:
        if stream.read(4) != b'RIFF':
            raise ValueError('Not RIFF')
        size = struct.unpack('<I', stream.read(4))[0]
        if stream.read(4) != b'sfbk':
            raise ValueError('Not SF2')
        end = min(size + 8, path.stat().st_size)
        while stream.tell() + 8 <= end:
            tag, length = struct.unpack('<4sI', stream.read(8))
            data_start = stream.tell()
            if data_start + length > end:
                raise ValueError('Truncated chunk')
            if tag == b'LIST' and length >= 4 and stream.read(4) == b'pdta':
                while stream.tell() + 8 <= data_start + length:
                    child, count = struct.unpack('<4sI', stream.read(8))
                    if stream.tell() + count > data_start + length:
                        raise ValueError('Truncated preset chunk')
                    if child == b'phdr':
                        if count % 38:
                            raise ValueError('Invalid preset record size')
                        data = stream.read(count)
                        return [dict(name=n.split(b'\0')[0].decode('latin1'), program=p, bank=b)
                                for n, p, b, *_ in struct.iter_unpack('<20sHHHIII', data)][:-1]
                    stream.seek(count + (count % 2), 1)
            stream.seek(data_start + length + (length % 2))
    raise ValueError('No preset headers')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('root', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    paths = sorted((p for p in args.root.rglob('*') if p.is_file() and p.suffix.lower() == '.sf2'), key=str)
    results = []
    for path in paths:
        try:
            entries = presets(path)
            gm = len({e['program'] for e in entries if e['bank'] == 0 and e['program'] < 128})
            drums = sum(e['bank'] == 128 for e in entries)
            results.append(dict(path=str(path), bytes=path.stat().st_size, presets=len(entries),
                                gm_programs=gm, drum_presets=drums, entries=entries))
        except (OSError, ValueError, struct.error) as exc:
            results.append(dict(path=str(path), error=str(exc)))
    results.sort(key=lambda r: (-r.get('gm_programs', 0), -r.get('drum_presets', 0), r.get('bytes', 0)))
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(results, indent=2), encoding='utf-8')
    print(f'{len(results)} SF2 files; {sum("error" in r for r in results)} unreadable')
    for result in results[:20]:
        print(f"GM {result.get('gm_programs', 0):3}/128, drums {result.get('drum_presets', 0):3}, "
              f"{result.get('bytes', 0)/1048576:.1f} MiB: {result['path']}")


if __name__ == '__main__':
    main()
