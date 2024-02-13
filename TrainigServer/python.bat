@echo off

set UE_ROOT=C:\Program Files\Epic Games
set UE_VERSION=5.3
set PYTHON_PATH="%UE_ROOT%\UE_%UE_VERSION%\Engine\Binaries\ThirdParty\Python3\Win64\python.exe"

%PYTHON_PATH% %*
