@echo off

call "%~dp0\config.bat"

set PYTHON_PATH="%UE_ROOT%Engine\Binaries\ThirdParty\Python3\Win64\python.exe"

%PYTHON_PATH% %*
