param([string]$Version='')
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$build=Join-Path $repo 'Source\Application\build'
$release=Join-Path $build 'win\apps\ignite\Ignite_artefacts\Release'
$exe=Join-Path $release 'Ignite.exe'
if(!(Test-Path -LiteralPath $exe)){throw 'Build the Release application first.'}
if($Version -and $Version -notmatch '^[0-9A-Za-z][0-9A-Za-z.+-]*$'){throw 'Version must contain only letters, numbers, dots, plus signs and hyphens.'}
$packageName=if($Version){"Ignite-Desktop-$Version-windows-x64"}else{'Ignite-Windows-x64-'+(Get-Date -Format 'yyyyMMdd-HHmmss')}
$destination=Join-Path $build ('packages\'+$packageName)
if(Test-Path -LiteralPath $destination){throw "Package directory already exists: $destination"}
New-Item -ItemType Directory -Path $destination -Force|Out-Null
Copy-Item -LiteralPath $exe -Destination $destination
Copy-Item -LiteralPath (Join-Path $release 'assets') -Destination $destination -Recurse
Copy-Item -LiteralPath (Join-Path $repo 'LICENSE') -Destination $destination
Copy-Item -LiteralPath (Join-Path $repo 'docs') -Destination $destination -Recurse
Copy-Item -LiteralPath (Join-Path $repo 'RELEASE_NOTES.md') -Destination $destination
$source=Join-Path $destination 'source'
New-Item -ItemType Directory -Path $source -Force|Out-Null
$files=& git -C $repo ls-files --cached --others --exclude-standard -- Source/Application ComProtocol AudIO scripts docs README.md RELEASE_NOTES.md LICENSE .gitignore
if($LASTEXITCODE -ne 0){throw 'Could not enumerate corresponding source files.'}
foreach($relative in $files) {
    if([IO.Path]::GetExtension($relative) -ieq '.sf2'){continue}
    $origin=Join-Path $repo $relative
    if(!(Test-Path -LiteralPath $origin -PathType Leaf)){continue}
    $target=Join-Path $source $relative
    New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force|Out-Null
    Copy-Item -LiteralPath $origin -Destination $target
}
@'
IGNITE desktop simulator

Run Ignite.exe. Choose DESKTOP SETTINGS (or Ctrl+,) for audio, MIDI routing,
SoundFont loading/preview, WAV/stem and MIDI export, MIDI clip import,
and project backup ZIP export/restore. Stop playback before media export.
Supply your own .sf2 bank; no private sample libraries are included.
The app stores user data under %APPDATA%\BehringerIgnite.

Read docs/manual/USER_MANUAL.html or docs/manual/USER_MANUAL.pdf.
The Markdown manual source is docs/manual/USER_MANUAL.md.
The editable faceplate is assets/ignite-panel.svg.

Corresponding application and library source is included under source/.
Build commands are in source/scripts/build-windows.ps1 and docs/BUILD_WINDOWS.md.
JUCE 7.0.12 is fetched by CMake and the included multitouch patch is applied.
Upstream: https://github.com/MusicTribeOpenSource/0716-AAF
Community fork: https://github.com/chillbeast/0716-AAF
This is a local community desktop build, not a device firmware image.
'@ | Set-Content -LiteralPath (Join-Path $destination 'START-HERE.txt') -Encoding UTF8
$sourceCommit=& git -C $repo rev-parse HEAD
if($LASTEXITCODE -ne 0){throw 'Could not identify the source commit.'}
$localChanges=[bool](& git -C $repo status --porcelain --untracked-files=normal)
$metadata=[ordered]@{createdUtc=(Get-Date).ToUniversalTime().ToString('o');version=$Version;firmwareVersion='0.3.3';repository='https://github.com/chillbeast/0716-AAF';upstream='https://github.com/MusicTribeOpenSource/0716-AAF';upstreamCommit='0371a3ea22fc530e23e2321068a0b8835e1e6502';sourceCommit=$sourceCommit;localChanges=$localChanges;executableSha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash;configuration='Release';includesSoundFont=$false;sourceDirectory='source'}
$metadata|ConvertTo-Json|Set-Content -LiteralPath (Join-Path $destination 'BUILD.json') -Encoding UTF8
$zip=$destination+'.zip'
& python (Join-Path $PSScriptRoot 'package_zip.py') $destination $zip
if($LASTEXITCODE -ne 0){throw 'Package archive creation or validation failed.'}
Write-Output "Package: $zip"
Write-Output "Unpacked: $destination"
