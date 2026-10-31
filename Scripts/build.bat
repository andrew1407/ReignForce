@echo off
rem Compiles the project: build.bat [editor|game] [-c CONFIG] [--if-needed]
call "%~dp0ue.bat" build %*
exit /b %ERRORLEVEL%
