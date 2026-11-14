@echo off
REM ESP32 Deployment Script for TogEsp32 Project
REM Thin wrapper around deploy.ps1 so there is one script to maintain.
REM Usage: deploy.bat [build^|upload^|monitor^|clean^|all^|help] [-Port COM7]

pushd "%~dp0"
where pwsh >nul 2>nul
if %errorlevel%==0 (
    pwsh -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" %*
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" %*
)
set RESULT=%errorlevel%
popd
exit /b %RESULT%
