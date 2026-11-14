#!/usr/bin/env pwsh
# ESP32 Deployment Script for TogEsp32 Project
# PowerShell version - works on Windows, Linux, and macOS
# Run from the esp32/ folder (deploy.bat does this for you on Windows).

param(
    [Parameter(Position=0)]
    [ValidateSet("build", "upload", "monitor", "clean", "all", "help")]
    [string]$Action = "all",

    # Serial port, e.g. COM7 or /dev/ttyUSB0. Omit to let PlatformIO auto-detect.
    [string]$Port = $env:ESP32_PORT
)

$ErrorActionPreference = "Stop"

function Show-Header {
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "ESP32 Deployment Script" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
}

function Show-Help {
    Write-Host "Usage: ./deploy.ps1 [command] [-Port <serial port>]" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Commands:"
    Write-Host "  build      - Build the project only"
    Write-Host "  upload     - Build and upload to ESP32"
    Write-Host "  monitor    - Connect to serial monitor"
    Write-Host "  clean      - Clean build files"
    Write-Host "  all        - Build, upload, and monitor (default)"
    Write-Host "  help       - Show this help message"
    Write-Host ""
    Write-Host "The serial port is auto-detected unless -Port (or the ESP32_PORT"
    Write-Host "environment variable) is set, e.g. ./deploy.ps1 upload -Port COM7"
    Write-Host ""
}

function Test-SecretsFile {
    if (-not (Test-Path "src/secrets.h")) {
        Write-Host "ERROR: src/secrets.h not found!" -ForegroundColor Red
        Write-Host "Please copy src/secrets.h.example to src/secrets.h and configure your credentials." -ForegroundColor Yellow
        Write-Host ""
        exit 1
    }
}

function Invoke-Build {
    Write-Host "Building project..." -ForegroundColor Green
    pio run
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "BUILD FAILED!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
}

function Invoke-Upload {
    Write-Host "Building and uploading to ESP32..." -ForegroundColor Green
    if ($Port) {
        pio run --target upload --upload-port $Port
    } else {
        pio run --target upload
    }
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "UPLOAD FAILED!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
    Write-Host "Upload successful!" -ForegroundColor Green
}

function Invoke-Monitor {
    Write-Host "Connecting to serial monitor..." -ForegroundColor Green
    Write-Host "Press Ctrl+C to exit" -ForegroundColor Yellow
    Write-Host ""
    # Baud rate comes from monitor_speed in platformio.ini
    if ($Port) {
        pio device monitor --port $Port
    } else {
        pio device monitor
    }
}

function Invoke-Clean {
    Write-Host "Cleaning build files..." -ForegroundColor Green
    pio run --target clean
    Write-Host ""
    Write-Host "Clean complete!" -ForegroundColor Green
}

function Invoke-All {
    Write-Host "Step 1/3" -ForegroundColor Cyan
    Invoke-Build
    Write-Host ""

    Write-Host "Step 2/3" -ForegroundColor Cyan
    Invoke-Upload
    Write-Host ""

    Write-Host "Step 3/3" -ForegroundColor Cyan
    Start-Sleep -Seconds 2
    Invoke-Monitor
}

# Main script execution
Show-Header

if ($Action -eq "help") {
    Show-Help
    exit 0
}

Test-SecretsFile

switch ($Action) {
    "build"   { Invoke-Build }
    "upload"  { Invoke-Upload }
    "monitor" { Invoke-Monitor }
    "clean"   { Invoke-Clean }
    "all"     { Invoke-All }
}
