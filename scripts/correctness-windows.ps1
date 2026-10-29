param([ValidateSet('Release','Debug','RelWithDebInfo')][string]$Configuration='Release',[string]$BuildDirectory='win',[int]$TimeoutSeconds=120)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$build=Join-Path $repo 'Source\Application\build'
$exe=Join-Path $build "$BuildDirectory\apps\ignite\Ignite_artefacts\$Configuration\Ignite.exe"
$root=Join-Path $build ('correctness\'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $root -Force|Out-Null
$font=Join-Path $root 'midi-test.sf2'
python (Join-Path $repo 'Source\Application\tests\make_test_soundfont.py') $font --midi
if($LASTEXITCODE -ne 0){throw 'MIDI fixture generation failed.'}
$previous=@{}
foreach($name in @('IGNITE_DATA_DIR','IGNITE_SOUNDFONT','IGNITE_LOG')){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
$results=@()
try {
    $env:IGNITE_DATA_DIR=Join-Path $root 'data'
    $env:IGNITE_SOUNDFONT=$font
    foreach($phase in @('test','verify')) {
        $env:IGNITE_LOG=Join-Path $root "$phase.log"
        $process=Start-Process -FilePath $exe -ArgumentList "--correctness-$phase" -WorkingDirectory (Split-Path -Parent $exe) -WindowStyle Hidden -PassThru
        $exited=$process.WaitForExit($TimeoutSeconds*1000)
        $result=[ordered]@{phase=$phase;exited=$exited;exitCode=$null;log=$env:IGNITE_LOG}
        if($exited){$result.exitCode=$process.ExitCode}else{Stop-Process -Id $process.Id -Force}
        $results+=[pscustomobject]$result;$result|ConvertTo-Json -Compress|Write-Output
    }
    $results|ConvertTo-Json|Set-Content (Join-Path $root 'results.json') -Encoding UTF8
    if($results.Where({!$_.exited -or $_.exitCode -ne 0}).Count){throw "Correctness tests failed: $root"}
} finally {foreach($name in $previous.Keys){[Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')}}
