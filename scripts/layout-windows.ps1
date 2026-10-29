param([ValidateSet('Release','Debug','RelWithDebInfo')][string]$Configuration='Release',[string]$BuildDirectory='win',[int]$TimeoutSeconds=120)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$build=Join-Path $repo 'Source\Application\build'
$exe=Join-Path $build "$BuildDirectory\apps\ignite\Ignite_artefacts\$Configuration\Ignite.exe"
$runRoot=Join-Path $build ('layout\'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
$data=Join-Path $runRoot 'data'
New-Item -ItemType Directory -Path $data -Force|Out-Null
$font=Join-Path $build "$BuildDirectory\tests\test-tone.sf2"
@{PanelLayout='hardware';LayoutSentinel='preserve-other-settings';SoundFont=$font;MonitorInput=$true}|ConvertTo-Json|Set-Content (Join-Path $data 'desktop.json') -Encoding UTF8
$previous=@{}
foreach($name in @('IGNITE_DATA_DIR','IGNITE_SOUNDFONT','IGNITE_LOG','IGNITE_EXPECT_LAYOUT','IGNITE_LAYOUT_SNAPSHOTS')){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
$results=@()
try {
    $env:IGNITE_DATA_DIR=$data
    $env:IGNITE_SOUNDFONT=$font
    foreach($phase in @('test','verify','override','compact-override','touchscreen-override')) {
        $env:IGNITE_LOG=Join-Path $runRoot "$phase.log"
        $env:IGNITE_LAYOUT_SNAPSHOTS=Join-Path $runRoot 'snapshots'
        $env:IGNITE_EXPECT_LAYOUT=if($phase -eq 'verify' -or $phase -eq 'touchscreen-override'){'touchscreen'}elseif($phase -eq 'compact-override'){'compact'}else{'hardware'}
        $arguments=if($phase -eq 'test'){'--layout-test'}elseif($phase -eq 'verify'){'--layout-verify'}elseif($phase -eq 'compact-override'){'--layout-verify --compact'}elseif($phase -eq 'touchscreen-override'){'--layout-verify --touchscreen'}else{'--layout-verify --hardware'}
        $process=Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory (Split-Path -Parent $exe) -WindowStyle Hidden -PassThru
        $exited=$process.WaitForExit($TimeoutSeconds*1000)
        $result=[ordered]@{phase=$phase;exited=$exited;exitCode=$null;log=$env:IGNITE_LOG}
        if($exited){$result.exitCode=$process.ExitCode}else{Stop-Process -Id $process.Id -Force}
        $results+=[pscustomobject]$result
        $result|ConvertTo-Json -Compress|Write-Output
    }
    $results|ConvertTo-Json|Set-Content (Join-Path $runRoot 'results.json') -Encoding UTF8
    if($results.Where({!$_.exited -or $_.exitCode -ne 0}).Count){throw "Layout tests failed: $runRoot"}
} finally {foreach($name in $previous.Keys){[Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')}}
