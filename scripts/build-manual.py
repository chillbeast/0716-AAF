#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Render the Markdown manual as self-contained HTML and optionally PDF.

Requires Python-Markdown (`python -m pip install Markdown`). PDF output uses
an installed Chromium browser with a separate, disposable browser profile.
"""
import argparse
import base64
import mimetypes
from pathlib import Path
import re
import subprocess
import tempfile
from urllib.parse import unquote

import markdown


CSS = """
:root { color-scheme: light; --ink:#24272c; --accent:#a62e49; }
* { box-sizing:border-box; }
body { margin:0; background:#eceef0; color:var(--ink); font:16px/1.65 'Segoe UI',Arial,sans-serif; }
main { max-width:980px; margin:32px auto; padding:54px 64px; background:white; border-top:8px solid var(--accent); }
h1 { font-size:42px; line-height:1.15; letter-spacing:-1px; margin:0 0 24px; }
h2 { font-size:27px; margin:52px 0 18px; padding-top:12px; border-top:2px solid #d9dce0; }
h3 { font-size:19px; margin:28px 0 10px; }
a { color:#8b2540; text-decoration-thickness:1px; text-underline-offset:3px; }
img { display:block; max-width:100%; max-height:560px; height:auto; margin:24px auto; border:1px solid #d9dce0; }
table { width:100%; border-collapse:collapse; margin:22px 0; font-size:.92em; }
th,td { padding:10px 12px; border:1px solid #d8dce0; text-align:left; vertical-align:top; }
th { background:#f1f2f4; font-weight:650; }
tr:nth-child(even) td { background:#fafafb; }
code { font-family:Consolas,monospace; font-size:.9em; overflow-wrap:anywhere; }
p code,li code,td code { background:#f0f1f3; padding:1px 4px; border-radius:3px; }
pre { background:#f1f2f4; padding:14px 18px; white-space:pre-wrap; overflow-wrap:anywhere; }
li { margin:7px 0; }
@media(max-width:720px) { main { margin:0; padding:28px 20px; } h1 {font-size:32px;} table{font-size:.83em;} th,td{padding:7px;} }
@page {
  size:A4; margin:17mm 18mm 18mm;
  @bottom-left { content:'IGNITE Desktop — Community manual'; font:8pt 'Segoe UI',Arial,sans-serif; color:#697078; }
  @bottom-right { content:counter(page) ' / ' counter(pages); font:8pt 'Segoe UI',Arial,sans-serif; color:#697078; }
}
@media print {
  body { background:white; font-size:10.5pt; line-height:1.48; }
  main { max-width:none; margin:0; padding:0; border-top:5px solid var(--accent); }
  h1 { font-size:31pt; padding-top:7mm; }
  h2 { break-before:page; font-size:20pt; margin:0 0 6mm; padding-top:4mm; }
  h3 { font-size:13pt; break-after:avoid; }
  p,li { orphans:3; widows:3; }
  img { max-height:82mm; max-width:100%; break-inside:avoid; margin:6mm auto; }
  img[alt^='IGNITE simulator'] { max-height:105mm; }
  table { font-size:9pt; margin:5mm 0; }
  th,td { padding:2.3mm; }
  thead { display:table-header-group; }
  tr { break-inside:avoid; }
  pre { break-inside:avoid; }
  a { color:inherit; }
}
"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pdf", action="store_true")
    parser.add_argument("--browser", type=Path)
    parser.add_argument("--version", default="v0.3.3-desktop.1")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parent.parent
    manual = repo / "docs/manual/USER_MANUAL.md"
    source = manual.read_text(encoding="utf-8-sig")

    def embed_image(match):
        path = (manual.parent / unquote(match[2])).resolve(strict=True)
        if not path.is_relative_to(repo):
            raise ValueError("Manual images must be in the repository")
        mime = mimetypes.guess_type(path.name)[0] or "application/octet-stream"
        payload = base64.b64encode(path.read_bytes()).decode("ascii")
        return f"![{match[1]}](data:{mime};base64,{payload})"

    source = re.sub(r"!\[([^\]]*)\]\(([^)]+)\)", embed_image, source)

    def source_link(match):
        relative = match[2]
        if relative.startswith(("#", "https://", "http://", "data:")):
            return match[0]
        path = (manual.parent / relative).resolve(strict=True)
        relative = path.relative_to(repo).as_posix()
        return f"[{match[1]}](https://github.com/chillbeast/0716-AAF/blob/{args.version}/{relative})"

    source = re.sub(r"(?<!!)\[([^\]]+)\]\(([^)]+)\)", source_link, source)
    body = markdown.markdown(source, extensions=["tables", "fenced_code", "toc", "sane_lists"])
    output = manual.with_suffix(".html")
    output.write_text("<!doctype html>\n<html lang=\"en\"><head><meta charset=\"utf-8\">"
                      "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
                      "<title>IGNITE Desktop Simulator — User Manual</title>"
                      f"<style>{CSS}</style></head><body><main>{body}</main></body></html>\n", encoding="utf-8")
    print(f"HTML: {output}")
    if args.pdf:
        candidates = [args.browser] if args.browser else [
            Path("C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe"),
            Path("C:/Program Files/Google/Chrome/Application/chrome.exe")]
        browser = next((p for p in candidates if p and p.is_file()), None)
        if browser is None:
            raise RuntimeError("Pass --browser with a Chromium browser executable for PDF output")
        pdf = manual.with_suffix(".pdf")
        logs = repo / "Source/Application/build/manual"
        logs.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="ignite-manual-") as profile:
            with (logs / "browser.log").open("w", encoding="utf-8") as log:
                subprocess.run([str(browser), "--headless", "--disable-gpu", "--no-pdf-header-footer",
                                "--disable-extensions", "--allow-file-access-from-files",
                                f"--user-data-dir={profile}", f"--print-to-pdf={pdf}", output.as_uri()],
                               stdout=log, stderr=log, check=True, timeout=45,
                               creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        if not pdf.is_file() or pdf.stat().st_size < 10000:
            raise RuntimeError("The browser did not create a usable PDF")
        print(f"PDF: {pdf} ({pdf.stat().st_size:,} bytes)")


if __name__ == "__main__":
    main()
