@echo off
title AuraShell Launcher
echo.
echo Welcome to AuraShell
echo.
echo Starting AuraShell...
echo.
REM Try to find the newest build with voice feature
set EXE=
if exist "%~dp0AuraShell\build_voice\Release\AuraShell.exe" (
    set "EXE=%~dp0AuraShell\build_voice\Release\AuraShell.exe"
    set "DIR=%~dp0AuraShell\build_voice\Release"
    echo Found AuraShell with Voice Feature!
) else if exist "%~dp0AuraShell\build_no_debug\Release\AuraShell.exe" (
    set "EXE=%~dp0AuraShell\build_no_debug\Release\AuraShell.exe"
    set "DIR=%~dp0AuraShell\build_no_debug\Release"
    echo Found AuraShell executable.
) else if exist "%~dp0build\Release\AuraShell.exe" (
    set "EXE=%~dp0build\Release\AuraShell.exe"
    set "DIR=%~dp0build\Release"
    echo Found AuraShell executable.
) else (
    echo Error: AuraShell.exe not found!
    echo Please build the project first.
    pause
    exit /b 1
)

start "AuraShell" cmd /k "cd /d %DIR% && echo AuraShell && echo. && echo Try: voice on && echo Then: echo Hello World && echo. && AuraShell.exe"
echo AuraShell started successfully!
echo.
pause

