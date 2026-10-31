@echo off
rem Windows launcher for ue.py: runs it with the project's engine bundled Python
setlocal

set "UE_PYTHON="
if defined UE_ROOT set "UE_PYTHON=%UE_ROOT%\Engine\Binaries\ThirdParty\Python3\Win64\python.exe"
if defined UE_PYTHON if exist "%UE_PYTHON%" goto run

rem Resolve the engine install from the uproject EngineAssociation: Epic launcher manifest, then registry
set "UE_FIND=$p = Get-ChildItem '%~dp0..\*.uproject' | Select-Object -First 1; $v = (Get-Content $p.FullName -Raw | ConvertFrom-Json).EngineAssociation; $m = Join-Path $env:ProgramData 'Epic\UnrealEngineLauncher\LauncherInstalled.dat'; $r = $null; if (Test-Path $m) { $r = ((Get-Content $m -Raw | ConvertFrom-Json).InstallationList | Where-Object AppName -eq ('UE_' + $v) | Select-Object -First 1).InstallLocation }; if (-not $r) { $r = (Get-ItemProperty ('HKLM:\SOFTWARE\EpicGames\Unreal Engine\' + $v) -ErrorAction SilentlyContinue).InstalledDirectory }; if (-not $r) { $r = (Get-ItemProperty 'HKCU:\Software\Epic Games\Unreal Engine\Builds' -ErrorAction SilentlyContinue).$v }; if ($r) { Join-Path $r 'Engine\Binaries\ThirdParty\Python3\Win64\python.exe' }"
for /f "usebackq delims=" %%P in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "%UE_FIND%"`) do set "UE_PYTHON=%%P"

if not defined UE_PYTHON goto missing
if not exist "%UE_PYTHON%" goto missing

:run
"%UE_PYTHON%" -I "%~dp0ue.py" %*
exit /b %ERRORLEVEL%

:missing
echo error: engine Python not found, set UE_ROOT to the engine root dir 1>&2
exit /b 1
