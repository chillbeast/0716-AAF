param([ValidateSet('Release','Debug','RelWithDebInfo')][string]$Configuration='Release', [string]$BuildDirectory='win', [switch]$NodeEdit, [switch]$AudioDevice, [switch]$SimulateInterruptedSave, [string]$SoundFont='', [int]$TimeoutSeconds=120)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$build=Join-Path $repo 'Source\Application\build'
$exe=Join-Path $build "$BuildDirectory\apps\ignite\Ignite_artefacts\$Configuration\Ignite.exe"
$runRoot=Join-Path $build ('workflow\'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
if (!$SoundFont) {
    $settingsPath=Join-Path $build 'desktop-settings.json'
    if (Test-Path -LiteralPath $settingsPath) { $SoundFont=(Get-Content -LiteralPath $settingsPath -Raw -Encoding UTF8 | ConvertFrom-Json).SoundFont }
    else { $SoundFont=Join-Path $build "$BuildDirectory\tests\test-tone.sf2" }
}
$previous=@{}
foreach($name in @('IGNITE_DATA_DIR','IGNITE_SOUNDFONT','IGNITE_LOG','IGNITE_SNAPSHOT')){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
$results=@()
try{
    $env:IGNITE_DATA_DIR=Join-Path $runRoot 'data'
    $env:IGNITE_SOUNDFONT=$SoundFont
    New-Item -ItemType Directory -Path $env:IGNITE_DATA_DIR -Force | Out-Null
    foreach($phase in @('test','verify')){
        $env:IGNITE_LOG=Join-Path $runRoot "$phase.log"
        $env:IGNITE_SNAPSHOT=Join-Path $runRoot "$phase.png"
        $arguments="--workflow-$phase" + $(if($NodeEdit){' --node-edit'}else{''}) + $(if($AudioDevice){' --audio-device'}else{''})
        $p=Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory (Split-Path -Parent $exe) -WindowStyle Hidden -PassThru
        $exited=$p.WaitForExit($TimeoutSeconds * 1000)
        $result=[ordered]@{phase=$phase;exited=$exited;exitCode=$null;log=$env:IGNITE_LOG}
        if($exited){$result.exitCode=$p.ExitCode}else{Stop-Process -Id $p.Id -Force}
        $results+=[pscustomobject]$result
        $result|ConvertTo-Json -Compress|Write-Output
        if ($phase -eq 'test' -and $exited -and $p.ExitCode -eq 0 -and $SimulateInterruptedSave) {
            $library=Join-Path $env:IGNITE_DATA_DIR 'Ignite_folder\Songs'
            $songs=@(Get-ChildItem -LiteralPath $library -Directory)
            if ($songs.Count -ne 1) { throw 'Recovery check requires the one disposable workflow song.' }
            $recovery=Join-Path $env:IGNITE_DATA_DIR 'Ignite_folder\.song-save'
            $backup=Join-Path (Join-Path $recovery 'backups') $songs[0].Name
            $prior=Join-Path $recovery 'prior-recovery-check'
            # Every move stays inside this script's freshly created test directory.
            $allowed=[IO.Path]::GetFullPath($runRoot)+[IO.Path]::DirectorySeparatorChar
            foreach ($path in @($songs[0].FullName,$backup,$prior)) {
                if (![IO.Path]::GetFullPath($path).StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)) { throw 'Recovery path escaped test directory.' }
            }
            New-Item -ItemType Directory -Path (Split-Path -Parent $backup) -Force | Out-Null
            if (Test-Path -LiteralPath $backup) { Move-Item -LiteralPath $backup -Destination $prior }
            Move-Item -LiteralPath $songs[0].FullName -Destination $backup
            Write-Output 'Simulated an interrupted song commit; verify must recover the saved song at startup.'
        }
    }
    $results|ConvertTo-Json|Set-Content -LiteralPath (Join-Path $runRoot 'results.json')
    if($results.Where({!$_.exited -or $_.exitCode -ne 0}).Count){throw "Workflow failures: $runRoot"}
}finally{foreach($name in $previous.Keys){[Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')}}
