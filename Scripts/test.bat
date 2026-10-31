@echo off
rem Runs the project automation tests: test.bat [FILTER] [--build auto|always|never]
call "%~dp0ue.bat" test %*
exit /b %ERRORLEVEL%
