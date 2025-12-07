#!/usr/bin/env pwsh
# ESP32 Deployment Script for TogEsp32 Project
# PowerShell version - works on Windows, Linux, and macOS

param(
    [Parameter(Position=0)]
    [ValidateSet("build", "upload", "monitor", "clean", "all", "help")]
    [string]$Action = "all"
)

$ErrorActionPreference = "Stop"

function Show-Header {
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "ESP32 Deployment Script" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
}

function Show-Help {
    Write-Host "Usage: ./deploy.ps1 [command]" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Commands:"
    Write-Host "  build      - Build the project only"
    Write-Host "  upload     - Build and upload to ESP32"
    Write-Host "  monitor    - Connect to serial monitor"
    Write-Host "  clean      - Clean build files"
    Write-Host "  all        - Build, upload, and monitor (default)"
    Write-Host "  help       - Show this help message"
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
    pio run --target upload
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
    pio device monitor --baud 115200 --port COM7
}

function Invoke-Clean {
    Write-Host "Cleaning build files..." -ForegroundColor Green
    pio run --target clean
    Write-Host ""
    Write-Host "Clean complete!" -ForegroundColor Green
}

function Invoke-All {
    Write-Host "Step 1/3: Building project..." -ForegroundColor Green
    pio run
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "BUILD FAILED!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host ""

    Write-Host "Step 2/3: Uploading to ESP32..." -ForegroundColor Green
    pio run --target upload
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "UPLOAD FAILED!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
    Write-Host "Upload successful!" -ForegroundColor Green
    Write-Host ""

    Write-Host "Step 3/3: Starting serial monitor..." -ForegroundColor Green
    Write-Host "Press Ctrl+C to exit" -ForegroundColor Yellow
    Write-Host ""
    Start-Sleep -Seconds 2
    pio device monitor --baud 115200 --port COM7
}

# Main script execution
Show-Header
Test-SecretsFile

switch ($Action) {
    "build"   { Invoke-Build }
    "upload"  { Invoke-Upload }
    "monitor" { Invoke-Monitor }
    "clean"   { Invoke-Clean }
    "all"     { Invoke-All }
    "help"    { Show-Help }
}
