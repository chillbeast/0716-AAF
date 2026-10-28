param([switch]$DebugBuild,[switch]$Asan,[switch]$SkipChecks,[switch]$Package,[int]$Jobs=4)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$build=Join-Path $repo 'Source\Application\build'
New-Item -ItemType Directory -Path $build -Force|Out-Null
function Invoke-CMakeLogged([string[]]$Arguments,[string]$Log) {
    # Windows PowerShell treats native stderr warnings as errors when redirected.
    # CMake's exit code determines success; retain warnings in the log.
    $ErrorActionPreference='Continue'
    & cmake @Arguments *> $Log
    if ($LASTEXITCODE -ne 0) {Get-Content -LiteralPath $Log -Tail 35;throw "CMake failed. Log: $Log"}
}
$tree=if($Asan){'asan'}else{'win'}
$configuration=if($Asan){'RelWithDebInfo'}else{'Release'}
$directory=Join-Path $build $tree
$configure=@('-S',(Join-Path $repo 'Source\Application'),'-B',$directory,'-G','Visual Studio 17 2022','-A','x64','-DTIMESTAMP=OFF',"-DUSE_ASAN_IGNITE=$(if($Asan){'ON'}else{'OFF'})")
$juce=Join-Path $build 'deps\JUCE'
if(Test-Path -LiteralPath $juce){$configure+='-DCPM_JUCE_SOURCE='+$juce.Replace('\','/')}
Write-Output "Configuring Ignite $configuration"
Invoke-CMakeLogged $configure (Join-Path $build "configure-$tree.log")
Invoke-CMakeLogged @('--build',$directory,'--config',$configuration,'--target','Ignite','IgniteDesktopTests','IgniteSoundFontProbe','--parallel',"$Jobs") (Join-Path $build "build-$tree-$configuration.log")
if($DebugBuild -and !$Asan){Invoke-CMakeLogged @('--build',$directory,'--config','Debug','--target','Ignite','IgniteDesktopTests','IgniteSoundFontProbe','--parallel',"$Jobs") (Join-Path $build 'build-win-Debug.log')}
if(!$Asan) {
    Write-Output 'Building standalone AudIO and packet-protocol libraries'
    $audio=Join-Path $build 'audio'
    Invoke-CMakeLogged @('-S',(Join-Path $repo 'AudIO'),'-B',$audio,'-G','Visual Studio 17 2022','-A','x64','-DAUDIO_BUILD_EXAMPLES=OFF') (Join-Path $build 'audio-configure.log')
    Invoke-CMakeLogged @('--build',$audio,'--config','Release','--parallel',"$Jobs") (Join-Path $build 'audio-build.log')
    $protocol=Join-Path $build 'protocol'
    Invoke-CMakeLogged @('-S',(Join-Path $repo 'ComProtocol'),'-B',$protocol,'-G','Visual Studio 17 2022','-A','x64') (Join-Path $build 'protocol-configure.log')
    Invoke-CMakeLogged @('--build',$protocol,'--config','Release','--parallel',"$Jobs") (Join-Path $build 'protocol-build.log')
}
if(!$SkipChecks) {
    $previousPath=$env:PATH
    try {
        if($Asan) {
            $runtime=Get-ChildItem -Path "$env:ProgramFiles\Microsoft Visual Studio\2022\*\VC\Tools\MSVC\*\bin\Hostx64\x64\clang_rt.asan_dynamic-x86_64.dll" -File -ErrorAction SilentlyContinue|Sort-Object LastWriteTime -Descending|Select-Object -First 1
            if(!$runtime){throw 'The MSVC AddressSanitizer runtime was not found. Install the C++ AddressSanitizer component in Visual Studio.'}
            $env:PATH=$runtime.DirectoryName+';'+$env:PATH
        }
        & ctest --test-dir $directory -C $configuration --output-on-failure
        if($LASTEXITCODE -ne 0){throw 'Native regression checks failed.'}
        if($DebugBuild -and !$Asan) {
            & ctest --test-dir $directory -C Debug --output-on-failure
            if($LASTEXITCODE -ne 0){throw 'Debug native regression checks failed.'}
        }
    } finally {$env:PATH=$previousPath}
    if(!$Asan){
        & ctest --test-dir (Join-Path $build 'protocol') -C Release --output-on-failure
        if($LASTEXITCODE -ne 0){throw 'Packet protocol checks failed.'}
        & (Join-Path $PSScriptRoot 'workflow-windows.ps1') -Configuration Release -NodeEdit -AudioDevice -SimulateInterruptedSave
        & (Join-Path $PSScriptRoot 'features-windows.ps1') -Configuration Release
        & (Join-Path $PSScriptRoot 'smoke-windows.ps1') -Configuration Release
    }
}
if($Package){if($Asan){throw 'Package the normal Release build.'}; & (Join-Path $PSScriptRoot 'package-windows.ps1')}
Write-Output "Build complete: $directory"
