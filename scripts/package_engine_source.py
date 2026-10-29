#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Copy the embedded engine's source, retaining submodules and excluding banks."""
from pathlib import Path
import shutil
import sys

source = Path(sys.argv[1]).resolve(strict=True)
target = Path(sys.argv[2]).resolve(strict=True)
if target == source or target.is_relative_to(source):
    raise ValueError("Source and package destination must be separate")
count = 0
for path in source.rglob("*"):
    relative = path.relative_to(source)
    if ".git" in relative.parts or path.suffix.lower() in {".sf2", ".sf3", ".dls"}:
        continue
    if path.is_file():
        destination = target / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, destination)
        count += 1
print(f"Packaged {count} SoundFont engine source files")
