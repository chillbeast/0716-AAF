param([string]$DataDirectory='', [string]$SoundFont='')
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$exe=Join-Path $repo 'Source/Application/build/win/apps/ignite/Ignite_artefacts/Release/Ignite.exe'
if(!(Test-Path -LiteralPath $exe)){throw 'Build the Release simulator first.'}
if(!$DataDirectory){$DataDirectory=Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'IGNITE Demo'}
if(Test-Path -LiteralPath $DataDirectory){throw 'The demo directory already exists. Choose a new -DataDirectory to keep existing work.'}
if(!$SoundFont){
    foreach($settings in @((Join-Path ([Environment]::GetFolderPath('ApplicationData')) 'BehringerIgnite/desktop.json'),(Join-Path $repo 'Source/Application/build/desktop-settings.json'))) {
        if(Test-Path -LiteralPath $settings){$SoundFont=(Get-Content -LiteralPath $settings -Raw -Encoding UTF8|ConvertFrom-Json).SoundFont}
        if($SoundFont){break}
    }
}
if(!$SoundFont -or !(Test-Path -LiteralPath $SoundFont -PathType Leaf)){throw 'Supply a General MIDI SF2 using -SoundFont.'}
$SoundFont=(Resolve-Path -LiteralPath $SoundFont).Path
New-Item -ItemType Directory -Path $DataDirectory|Out-Null
$DataDirectory=(Resolve-Path -LiteralPath $DataDirectory).Path
@{SoundFont=$SoundFont;PanelLayout='compact';MonitorInput=$false}|ConvertTo-Json|Set-Content -LiteralPath (Join-Path $DataDirectory 'desktop.json') -Encoding UTF8
$previous=@{}
foreach($name in @('IGNITE_DATA_DIR','IGNITE_SOUNDFONT','IGNITE_LOG')){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
try {
    $env:IGNITE_DATA_DIR=$DataDirectory;$env:IGNITE_SOUNDFONT=$SoundFont;$env:IGNITE_LOG=Join-Path $DataDirectory 'install.log'
    $process=Start-Process -FilePath $exe -ArgumentList '--install-demo' -WorkingDirectory (Split-Path -Parent $exe) -WindowStyle Hidden -PassThru
    if(!$process.WaitForExit(120000)){Stop-Process -Id $process.Id -Force;throw 'Demo installation timed out.'}
    if($process.ExitCode -ne 0){throw "Demo installation failed. See $env:IGNITE_LOG"}
} finally {foreach($name in $previous.Keys){[Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')}}
Copy-Item -LiteralPath (Join-Path $repo 'Source/Application/assets/demo/First Light Demo.mid') -Destination $DataDirectory
Copy-Item -LiteralPath (Join-Path $repo 'Source/Application/assets/demo/README.txt') -Destination $DataDirectory
$shortcutPath=Join-Path ([Environment]::GetFolderPath('Desktop')) 'IGNITE Demo.lnk'
if(Test-Path -LiteralPath $shortcutPath){throw "Demo installed, but an existing shortcut was kept: $shortcutPath"}
$shortcut=(New-Object -ComObject WScript.Shell).CreateShortcut($shortcutPath)
$shortcut.TargetPath=Join-Path $env:SystemRoot 'System32/WindowsPowerShell/v1.0/powershell.exe'
$launcher=Join-Path $PSScriptRoot 'run-ignite.ps1'
$shortcut.Arguments="-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File `"$launcher`" -DataDirectory `"$DataDirectory`" -Song"
$shortcut.WorkingDirectory=$repo;$shortcut.IconLocation=$exe+',0'
$shortcut.Description='First Light demo project. Press Space / PLAY; Esc / STOP stops playback.'
$shortcut.Save()
Write-Output "Demo project: $DataDirectory"
Write-Output "Desktop shortcut: $shortcutPath"
