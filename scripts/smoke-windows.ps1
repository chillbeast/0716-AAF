param([ValidateSet('Release', 'Debug')][string]$Configuration = 'Debug', [string]$SoundFont = '')
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$build = Join-Path $repo 'Source\Application\build'
$exe = Join-Path $build "win\apps\ignite\Ignite_artefacts\$Configuration\Ignite.exe"
if (!(Test-Path -LiteralPath $exe)) { throw "Missing build: $exe" }
if (!$SoundFont -and (Test-Path -LiteralPath (Join-Path $build 'desktop-settings.json'))) {
    $SoundFont = (Get-Content -LiteralPath (Join-Path $build 'desktop-settings.json') -Raw -Encoding UTF8 | ConvertFrom-Json).SoundFont
}
$runRoot = Join-Path $build ('smoke\' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
$previous = @{}
foreach ($name in @('IGNITE_SOUNDFONT', 'IGNITE_DATA_DIR', 'IGNITE_LOG', 'IGNITE_SNAPSHOT')) { $previous[$name] = [Environment]::GetEnvironmentVariable($name, 'Process') }
$results = @()
try {
    $cases = @(@{Name='no-font'; Font=(Join-Path $runRoot 'missing.sf2')})
    if ($SoundFont) { $cases += @{Name='soundfont'; Font=$SoundFont} }
    foreach ($case in $cases) {
        $env:IGNITE_DATA_DIR = Join-Path $runRoot $case.Name
        New-Item -ItemType Directory -Path $env:IGNITE_DATA_DIR -Force | Out-Null
        $env:IGNITE_SOUNDFONT = $case.Font
        $env:IGNITE_LOG = Join-Path $runRoot ($case.Name + '.log')
        $env:IGNITE_SNAPSHOT = Join-Path $runRoot ($case.Name + '.png')
        $process = Start-Process -FilePath $exe -ArgumentList '--self-test' -WorkingDirectory (Split-Path -Parent $exe) -WindowStyle Hidden -PassThru
        $exited = $process.WaitForExit(30000)
        $result = [ordered]@{case=$case.Name; exited=$exited; exitCode=$null; snapshot=(Test-Path -LiteralPath $env:IGNITE_SNAPSHOT); log=$env:IGNITE_LOG}
        if (!$exited) { Stop-Process -Id $process.Id -Force } else { $result.exitCode = $process.ExitCode }
        $log = if (Test-Path -LiteralPath $env:IGNITE_LOG) { Get-Content -LiteralPath $env:IGNITE_LOG -Raw } else { '' }
        $result.passed = $exited -and $result.exitCode -eq 0 -and $result.snapshot -and $log.Contains('Desktop self-test failures: 0')
        $results += [pscustomobject]$result
        $result | ConvertTo-Json -Compress | Write-Output
    }
    $results | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $runRoot 'results.json')
    if ($results.Where({!$_.passed}).Count) { throw "Smoke checks failed. Results: $runRoot" }
} finally {
    foreach ($name in $previous.Keys) { [Environment]::SetEnvironmentVariable($name, $previous[$name], 'Process') }
}
