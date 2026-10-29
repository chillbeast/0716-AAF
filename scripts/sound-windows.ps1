param([ValidateSet('Release','Debug','RelWithDebInfo')][string]$Configuration='Release',[string]$BuildDirectory='win',[int]$TimeoutSeconds=120)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$build=Join-Path $repo 'Source\Application\build'
$exe=Join-Path $build "$BuildDirectory\apps\ignite\Ignite_artefacts\$Configuration\Ignite.exe"
$root=Join-Path $build ('sound\'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $root -Force|Out-Null
$bankA=Join-Path $root 'stereo.sf2';$bankB=Join-Path $root 'alternate.sf2'
python (Join-Path $repo 'Source\Application\tests\make_test_soundfont.py') $bankA --stereo
if($LASTEXITCODE -ne 0){throw 'Stereo fixture generation failed.'}
python (Join-Path $repo 'Source\Application\tests\make_test_soundfont.py') $bankB --alternate
if($LASTEXITCODE -ne 0){throw 'Alternate fixture generation failed.'}
$previous=@{}
foreach($name in @('IGNITE_DATA_DIR','IGNITE_SOUNDFONT','IGNITE_LOG','IGNITE_TEST_BANK_B','IGNITE_TEST_BANK_A')){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
$results=@()
try {
    $env:IGNITE_DATA_DIR=Join-Path $root 'data';$env:IGNITE_SOUNDFONT=$bankA;$env:IGNITE_TEST_BANK_B=$bankB;$env:IGNITE_TEST_BANK_A=$bankA
    foreach($phase in @('test','verify','missing','no-default')) {
        if($phase -eq 'missing'){Move-Item -LiteralPath $bankB -Destination ($bankB+'.offline')}
        if($phase -eq 'no-default'){
            Move-Item -LiteralPath ($bankB+'.offline') -Destination $bankB
            $env:IGNITE_SOUNDFONT=Join-Path $root 'missing-default.sf2'
        }
        $argumentPhase=if($phase -eq 'no-default'){'verify'}else{$phase}
        $env:IGNITE_LOG=Join-Path $root "$phase.log"
        $process=Start-Process -FilePath $exe -ArgumentList "--sound-$argumentPhase" -WorkingDirectory (Split-Path -Parent $exe) -WindowStyle Hidden -PassThru
        $exited=$process.WaitForExit($TimeoutSeconds*1000)
        $result=[ordered]@{phase=$phase;exited=$exited;exitCode=$null;log=$env:IGNITE_LOG}
        if($exited){$result.exitCode=$process.ExitCode}else{Stop-Process -Id $process.Id -Force}
        $results+=[pscustomobject]$result;$result|ConvertTo-Json -Compress|Write-Output
        $results|ConvertTo-Json|Set-Content (Join-Path $root 'results.json') -Encoding UTF8
        if(!$exited -or $result.exitCode -ne 0){throw "Sound tests failed: $root"}
    }
} finally {
    if(Test-Path -LiteralPath ($bankB+'.offline')){Move-Item -LiteralPath ($bankB+'.offline') -Destination $bankB}
    foreach($name in $previous.Keys){[Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')}
}
