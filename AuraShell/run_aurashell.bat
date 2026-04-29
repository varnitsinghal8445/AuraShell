@echo off
title AuraShell Launcher
echo.
echo Welcome to AuraShell
echo.
echo Starting AuraShell in a new window...
echo.
start "AuraShell" cmd /k "cd /d %~dp0..\build\Release && AuraShell.exe"
echo AuraShell window opened successfully!
echo.
pause
