param([ValidateSet('Release','Debug','RelWithDebInfo')][string]$Configuration='Release',[string]$BuildDirectory='win',[string]$SoundFont='',[int]$TimeoutSeconds=240)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$build=Join-Path $repo 'Source/Application/build'
$exe=Join-Path $build "$BuildDirectory/apps/ignite/Ignite_artefacts/$Configuration/Ignite.exe"
$root=Join-Path $build ('demo/'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $root -Force|Out-Null
$font=Join-Path $root 'demo-gm.sf2';$alternate=Join-Path $root 'incomplete.sf2'
python (Join-Path $repo 'Source/Application/tests/make_test_soundfont.py') $font --demo
if($LASTEXITCODE -ne 0){throw 'Demo fixture generation failed'}
python (Join-Path $repo 'Source/Application/tests/make_test_soundfont.py') $alternate --alternate
if($LASTEXITCODE -ne 0){throw 'Incomplete-bank fixture generation failed'}
if($SoundFont){$font=(Resolve-Path -LiteralPath $SoundFont).Path}
$previous=@{}
foreach($name in @('IGNITE_DATA_DIR','IGNITE_SOUNDFONT','IGNITE_LOG','IGNITE_DEMO_ALTERNATE')){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
$results=@()
try {
    $env:IGNITE_DEMO_ALTERNATE=$alternate
    foreach($phase in @('test','verify','incomplete','no-font')) {
        $env:IGNITE_DATA_DIR=Join-Path $root $(if($phase -in @('test','verify')){'data'}else{'data-'+$phase})
        $env:IGNITE_SOUNDFONT=if($phase -eq 'incomplete'){$alternate}elseif($phase -eq 'no-font'){Join-Path $root 'missing.sf2'}else{$font}
        $env:IGNITE_LOG=Join-Path $root "$phase.log"
        $argument=if($phase -in @('test','verify')){"--demo-$phase"}else{'--demo-unavailable'}
        $process=Start-Process -FilePath $exe -ArgumentList $argument -WorkingDirectory (Split-Path -Parent $exe) -WindowStyle Hidden -PassThru
        $exited=$process.WaitForExit($TimeoutSeconds*1000)
        $result=[ordered]@{phase=$phase;exited=$exited;exitCode=$null;log=$env:IGNITE_LOG}
        if($exited){$result.exitCode=$process.ExitCode}else{Stop-Process -Id $process.Id -Force}
        $results+=[pscustomobject]$result;$result|ConvertTo-Json -Compress|Write-Output
        $results|ConvertTo-Json|Set-Content (Join-Path $root 'results.json') -Encoding UTF8
        if(!$exited -or $result.exitCode -ne 0){throw "Demo tests failed: $root"}
    }
} finally {foreach($name in $previous.Keys){[Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')}}
