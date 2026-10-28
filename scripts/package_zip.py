#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Create and validate the Windows package without PowerShell archive quirks."""
import os
from pathlib import Path
import sys
import zipfile


def main():
    folder = Path(sys.argv[1]).resolve(strict=True)
    destination = Path(sys.argv[2]).resolve()
    if destination.is_relative_to(folder):
        raise ValueError("The archive must be outside the packaged directory")
    files = sorted(path for path in folder.rglob("*") if path.is_file())
    if any(path.suffix.lower() == ".sf2" for path in files):
        raise ValueError("SoundFonts must not be included in this package")
    pending = destination.with_suffix(destination.suffix + ".tmp")
    with zipfile.ZipFile(pending, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path in files:
            archive.write(path, path.relative_to(folder.parent).as_posix())
    with zipfile.ZipFile(pending) as archive:
        if len(archive.infolist()) != len(files) or archive.testzip() is not None:
            raise RuntimeError("Package archive validation failed")
    os.replace(pending, destination)
    print(f"Verified ZIP: {len(files)} files, {destination.stat().st_size:,} bytes")


if __name__ == "__main__":
    main()
