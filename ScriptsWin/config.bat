@echo off

rem Get the full path of the script
for %%A in ("%~dp0\..") do set "PROJECT_PATH=%%~fA"

rem Get the last directory in the PROJECT_PATH as PROJECT_NAME
for %%B in ("%PROJECT_PATH%") do set "PROJECT_NAME=%%~nxB"

rem Now PROJECT_PATH and PROJECT_NAME are available for the rest of the script

set "PLATFORM=Win64"

IF "%PROJECT_NAME%"=="%CONTAINER_DIR%" (
    for %%I in ("%PROJECT_PATH%\..") do set "PROJECT_NAME=%%~nxI"
)

set "PROJECT_CONFIG=%PROJECT_PATH%\%PROJECT_NAME%.uproject"
set "UE_VERSION="

If exist "%PROJECT_CONFIG%" (
    for /f "tokens=2 delims=:}" %%a in ('type "%PROJECT_CONFIG%" ^| findstr /C:"EngineAssociation"') do (
        set "UE_VERSION=%%~a"
    )
) else (
    echo Error: Project config file not found.
    exit /b 1
)

rem Remove space, double quotes, and comma from UE_VERSION
set "UE_VERSION=%UE_VERSION: =%"
set "UE_VERSION=%UE_VERSION:"=%"
set "UE_VERSION=%UE_VERSION:,=%"

set "UE_ROOT=C:\Program Files\Epic Games\UE_%UE_VERSION%\"
set "SCRIPTS_ROOT=%UE_ROOT%Engine\Build\BatchFiles\"
set "PROJECT_CONFIG=%PROJECT_PATH%\%PROJECT_NAME%.uproject"
