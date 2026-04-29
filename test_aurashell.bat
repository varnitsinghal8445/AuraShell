@echo off
echo Testing AuraShell explain command...
echo.
if not exist "D:\AuraShell__\build\Release\AuraShell.exe" (
    echo Error: AuraShell.exe not found in build\Release\
    echo Please build the project first.
    pause
    exit /b 1
)
echo Found AuraShell.exe
echo.
echo ========================================
echo Starting AuraShell for testing...
echo ========================================
echo.
echo Instructions:
echo   1. Type 'explain open' to test the explain command
echo   2. Type 'explain dir' to test another command
echo   3. Type 'exit' to close AuraShell
echo.
pause
start "AuraShell Test" cmd /k "cd /d D:\AuraShell__\build\Release && AuraShell.exe"
