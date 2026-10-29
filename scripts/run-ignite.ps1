param(
    [ValidateSet('Release', 'Debug')][string]$Configuration = 'Release',
    [string]$SoundFont = '',
    [string]$DataDirectory = '',
    [switch]$Touchscreen,
    [ValidateSet('Hardware','Compact')][string]$Layout
)
$ErrorActionPreference = 'Stop'
$explicitSoundFont = [bool]$SoundFont
$repo = Split-Path -Parent $PSScriptRoot
$build = Join-Path $repo 'Source\Application\build\win'
$exe = Join-Path $build "apps\ignite\Ignite_artefacts\$Configuration\Ignite.exe"
if (!(Test-Path -LiteralPath $exe)) { throw "Build Ignite in $Configuration first. See docs/BUILD_WINDOWS.md." }
$preferenceRoot = if ($DataDirectory) { $DataDirectory } elseif ($env:IGNITE_DATA_DIR) { $env:IGNITE_DATA_DIR } else { Join-Path ([Environment]::GetFolderPath('ApplicationData')) 'BehringerIgnite' }
$preferencesPath = Join-Path $preferenceRoot 'desktop.json'
if (!$SoundFont -and (Test-Path -LiteralPath $preferencesPath)) {
    $SoundFont = (Get-Content -LiteralPath $preferencesPath -Raw -Encoding UTF8 | ConvertFrom-Json).SoundFont
}
$settingsPath = Join-Path $repo 'Source\Application\build\desktop-settings.json'
if (!$SoundFont -and (Test-Path -LiteralPath $settingsPath)) {
    $SoundFont = (Get-Content -LiteralPath $settingsPath -Raw -Encoding UTF8 | ConvertFrom-Json).SoundFont
}
if ($explicitSoundFont -and !(Test-Path -LiteralPath $SoundFont -PathType Leaf)) { throw "SoundFont does not exist: $SoundFont" }
$previous = @{}
foreach ($name in @('IGNITE_SOUNDFONT', 'IGNITE_DATA_DIR', 'IGNITE_LOG')) { $previous[$name] = [Environment]::GetEnvironmentVariable($name, 'Process') }
try {
    if ($SoundFont) {
        $env:IGNITE_SOUNDFONT = if (Test-Path -LiteralPath $SoundFont -PathType Leaf) { (Resolve-Path -LiteralPath $SoundFont).Path } else { [IO.Path]::GetFullPath($SoundFont) }
    }
    if ($DataDirectory) {
        New-Item -ItemType Directory -Path $DataDirectory -Force | Out-Null
        $env:IGNITE_DATA_DIR = (Resolve-Path -LiteralPath $DataDirectory).Path
    }
    $env:IGNITE_LOG = Join-Path $build 'ignite.log'
    $launch = @{ FilePath = $exe; WorkingDirectory = (Split-Path -Parent $exe); PassThru = $true }
    if ($Touchscreen) { $launch.ArgumentList = '--touchscreen' }
    elseif ($Layout) { $launch.ArgumentList = '--'+$Layout.ToLowerInvariant() }
    $process = Start-Process @launch
    Write-Output "IGNITE started (PID $($process.Id)). Log: $env:IGNITE_LOG"
} finally {
    foreach ($name in $previous.Keys) { [Environment]::SetEnvironmentVariable($name, $previous[$name], 'Process') }
}
