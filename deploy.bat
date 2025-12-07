@echo off
REM ESP32 Deployment Script for TogEsp32 Project
REM This script builds, uploads, and monitors the ESP32

echo ========================================
echo ESP32 Deployment Script
echo ========================================
echo.

REM Check if secrets.h exists
if not exist "src\secrets.h" (
    echo ERROR: src\secrets.h not found!
    echo Please copy src\secrets.h.example to src\secrets.h and configure your credentials.
    echo.
    pause
    exit /b 1
)

REM Parse command line arguments
set ACTION=%1
if "%ACTION%"=="" set ACTION=all

if "%ACTION%"=="build" goto BUILD
if "%ACTION%"=="upload" goto UPLOAD
if "%ACTION%"=="monitor" goto MONITOR
if "%ACTION%"=="clean" goto CLEAN
if "%ACTION%"=="all" goto ALL
if "%ACTION%"=="help" goto HELP

echo Unknown command: %ACTION%
echo.
goto HELP

:HELP
echo Usage: deploy.bat [command]
echo.
echo Commands:
echo   build      - Build the project only
echo   upload     - Build and upload to ESP32
echo   monitor    - Connect to serial monitor
echo   clean      - Clean build files
echo   all        - Build, upload, and monitor (default)
echo   help       - Show this help message
echo.
goto END

:CLEAN
echo Cleaning build files...
pio run --target clean
echo.
echo Clean complete!
goto END

:BUILD
echo Building project...
pio run
if errorlevel 1 (
    echo.
    echo BUILD FAILED!
    pause
    exit /b 1
)
echo.
echo Build successful!
goto END

:UPLOAD
echo Building and uploading to ESP32...
pio run --target upload
if errorlevel 1 (
    echo.
    echo UPLOAD FAILED!
    pause
    exit /b 1
)
echo.
echo Upload successful!
goto END

:MONITOR
echo Connecting to serial monitor...
echo Press Ctrl+C to exit
echo.
pio device monitor --baud 115200 --port COM7
goto END

:ALL
echo Step 1/3: Building project...
pio run
if errorlevel 1 (
    echo.
    echo BUILD FAILED!
    pause
    exit /b 1
)
echo.
echo Build successful!
echo.

echo Step 2/3: Uploading to ESP32...
pio run --target upload
if errorlevel 1 (
    echo.
    echo UPLOAD FAILED!
    pause
    exit /b 1
)
echo.
echo Upload successful!
echo.

echo Step 3/3: Starting serial monitor...
echo Press Ctrl+C to exit
echo.
timeout /t 2 /nobreak >nul
pio device monitor --baud 115200 --port COM7
goto END

:END
