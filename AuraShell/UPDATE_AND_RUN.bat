@echo off
title Update AuraShell with Voice Feature
echo.
echo ========================================
echo Updating AuraShell Executable
echo ========================================
echo.
echo Please CLOSE all AuraShell windows first!
echo.
pause

cd /d "%~dp0"

REM Build if needed
if not exist "build_voice\Release\AuraShell.exe" (
    echo Building project...
    if exist "build_voice\AuraShell.sln" (
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" build_voice\AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:q
        if errorlevel 1 (
            echo Build failed!
            pause
            exit /b 1
        )
    ) else (
        echo Solution not found. Please build in Visual Studio first.
        pause
        exit /b 1
    )
)

REM Copy new executable
if exist "build_voice\Release\AuraShell.exe" (
    echo Copying new executable...
    if not exist "build_no_debug\Release" mkdir "build_no_debug\Release"
    copy /Y "build_voice\Release\AuraShell.exe" "build_no_debug\Release\AuraShell.exe"
    if errorlevel 1 (
        echo.
        echo ERROR: Could not copy executable!
        echo Make sure all AuraShell windows are closed.
        pause
        exit /b 1
    )
    
    echo.
    echo ========================================
    echo SUCCESS! Executable updated!
    echo ========================================
    echo.
    echo Starting AuraShell with voice feature...
    echo.
    start "AuraShell - Voice" cmd /k "cd /d %~dp0build_no_debug\Release && echo AuraShell with Voice Feature && echo. && echo Try these commands: && echo   voice on && echo   echo Hello World && echo   pwd && echo   voice status && echo. && AuraShell.exe"
) else (
    echo ERROR: New executable not found!
    pause
    exit /b 1
)

pause


