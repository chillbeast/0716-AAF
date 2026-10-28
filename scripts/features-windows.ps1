param([ValidateSet('Release','Debug','RelWithDebInfo')][string]$Configuration='Release', [string]$BuildDirectory='win', [int]$TimeoutSeconds=120)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$build=Join-Path $repo 'Source\Application\build'
$exe=Join-Path $build "$BuildDirectory\apps\ignite\Ignite_artefacts\$Configuration\Ignite.exe"
$runRoot=Join-Path $build ('features\'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
$font=Join-Path $runRoot 'custom-bank-42.sf2'
python (Join-Path $repo 'Source\Application\tests\make_test_soundfont.py') $font 42 7
if ($LASTEXITCODE -ne 0) { throw 'Could not generate the custom bank fixture.' }
$settingsPath=Join-Path $build 'desktop-settings.json'
$initialFont=if(Test-Path -LiteralPath $settingsPath){(Get-Content -LiteralPath $settingsPath -Raw -Encoding UTF8|ConvertFrom-Json).SoundFont}else{Join-Path $build "$BuildDirectory\tests\test-tone.sf2"}
$previous=@{}
foreach($name in @('IGNITE_DATA_DIR','IGNITE_SOUNDFONT','IGNITE_LOG','IGNITE_SETTINGS_SNAPSHOT','IGNITE_TEST_SOUNDFONT')){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
$results=@()
try {
    $env:IGNITE_DATA_DIR=Join-Path $runRoot 'data'
    $env:IGNITE_TEST_SOUNDFONT=$font
    New-Item -ItemType Directory -Path $env:IGNITE_DATA_DIR -Force|Out-Null
    foreach($phase in @('test','verify')) {
        $env:IGNITE_SOUNDFONT=if($phase -eq 'test'){$initialFont}else{$font}
        $env:IGNITE_LOG=Join-Path $runRoot "$phase.log"
        $env:IGNITE_SETTINGS_SNAPSHOT=Join-Path $runRoot "$phase-settings.png"
        $process=Start-Process -FilePath $exe -ArgumentList "--feature-$phase" -WorkingDirectory (Split-Path -Parent $exe) -WindowStyle Hidden -PassThru
        $exited=$process.WaitForExit($TimeoutSeconds*1000)
        $result=[ordered]@{phase=$phase;exited=$exited;exitCode=$null;log=$env:IGNITE_LOG}
        if($exited){$result.exitCode=$process.ExitCode}else{Stop-Process -Id $process.Id -Force}
        $results+=[pscustomobject]$result
        $result|ConvertTo-Json -Compress|Write-Output
    }
    $results|ConvertTo-Json|Set-Content -LiteralPath (Join-Path $runRoot 'results.json')
    if($results.Where({!$_.exited -or $_.exitCode -ne 0}).Count){throw "Desktop feature failures: $runRoot"}
} finally {foreach($name in $previous.Keys){[Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')}}
