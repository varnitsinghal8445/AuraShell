@echo off
title AuraShell Voice Feature - Quick Start
echo.
echo ========================================
echo   AuraShell Voice Feature - Quick Start
echo ========================================
echo.

REM Find and run AuraShell executable
set AURASHELL=
if exist "build_voice\Release\AuraShell.exe" (
    set AURASHELL=build_voice\Release\AuraShell.exe
) else if exist "build_no_debug\Release\AuraShell.exe" (
    set AURASHELL=build_no_debug\Release\AuraShell.exe
) else if exist "build\Release\AuraShell.exe" (
    set AURASHELL=build\Release\AuraShell.exe
) else if exist "..\build\Release\AuraShell.exe" (
    set AURASHELL=..\build\Release\AuraShell.exe
) else (
    echo Error: AuraShell.exe not found!
    echo.
    echo Please build the project first:
    echo   1. Open Visual Studio
    echo   2. File -^> Open -^> CMake...
    echo   3. Select: %CD%\CMakeLists.txt
    echo   4. Build -^> Build All (F7)
    echo.
    echo Or run: BUILD_VOICE.bat
    echo.
    pause
    exit /b 1
)

echo Found AuraShell: %AURASHELL%
echo.
echo Starting AuraShell...
echo.
echo ========================================
echo   VOICE COMMAND INSTRUCTIONS
echo ========================================
echo.
echo Once AuraShell starts, try these commands:
echo.
echo   1. Enable voice:
echo      voice on
echo.
echo   2. Test voice output:
echo      echo Hello World
echo      pwd
echo      cd Documents
echo.
echo   3. Check voice status:
echo      voice status
echo.
echo   4. Disable voice:
echo      voice off
echo.
echo   5. Get help:
echo      explain voice
echo.
echo ========================================
echo.
pause
start "AuraShell" cmd /k "cd /d %~dp0 && %AURASHELL%"

