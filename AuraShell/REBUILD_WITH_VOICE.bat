@echo off
title Rebuilding AuraShell with Voice Feature
echo.
echo ========================================
echo Rebuilding AuraShell with Voice Feature
echo ========================================
echo.

set MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe

if not exist "%MSBUILD%" (
    set MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe
)

if not exist "%MSBUILD%" (
    echo Error: MSBuild not found!
    echo.
    echo Please open Visual Studio and build manually:
    echo   1. File -^> Open -^> CMake...
    echo   2. Select: %CD%\CMakeLists.txt
    echo   3. Build -^> Build All (F7)
    echo.
    pause
    exit /b 1
)

echo Found MSBuild: %MSBUILD%
echo.

REM Create build directory
if not exist "build_voice" mkdir build_voice
cd build_voice

REM Check if solution exists, if not need CMake
if not exist "AuraShell.sln" (
    echo Solution file not found.
    echo.
    echo Please generate solution using Visual Studio:
    echo   1. Open Visual Studio
    echo   2. File -^> Open -^> CMake...
    echo   3. Select: %CD%\..\CMakeLists.txt
    echo   4. Visual Studio will generate the solution
    echo   5. Then run this script again
    echo.
    echo OR if CMake is in PATH:
    echo   cmake .. -DCMAKE_BUILD_TYPE=Release
    echo.
    cd ..
    pause
    exit /b 1
)

echo Building AuraShell with voice feature...
echo.
"%MSBUILD%" AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build Successful!
    echo ========================================
    echo.
    if exist "Release\AuraShell.exe" (
        echo Executable: %CD%\Release\AuraShell.exe
        echo.
        echo Copying to build_no_debug\Release...
        if not exist "..\build_no_debug\Release" mkdir "..\build_no_debug\Release"
        copy /Y "Release\AuraShell.exe" "..\build_no_debug\Release\AuraShell.exe"
        echo.
        echo ========================================
        echo Ready to test!
        echo ========================================
        echo.
        echo Run: ..\build_no_debug\Release\AuraShell.exe
        echo Then try: voice on
        echo.
    )
) else (
    echo.
    echo ========================================
    echo Build Failed!
    echo ========================================
    echo.
    echo Please check the error messages above.
)

cd ..
pause

