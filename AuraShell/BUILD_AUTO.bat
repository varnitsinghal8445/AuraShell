@echo off
title Building AuraShell with Voice - Automatic
echo ========================================
echo Building AuraShell with Voice Feature
echo ========================================
echo.

REM Set up Visual Studio environment if available
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
)

REM Find MSBuild
set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if not exist "%MSBUILD%" (
    set MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe
)
if not exist "%MSBUILD%" (
    echo ERROR: MSBuild not found. Please install Visual Studio 2022.
    exit /b 1
)

REM Create build directory
cd /d "%~dp0"
if not exist "build_voice" mkdir build_voice
cd build_voice

REM Find CMake
set CMAKE=cmake
where cmake >nul 2>&1
if errorlevel 1 (
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set "CMAKE=C:\Program Files\CMake\bin\cmake.exe"
    ) else (
        echo CMake not found. Trying to build without regenerating solution...
        goto :build_existing
    )
)

REM Generate solution
if not exist "AuraShell.sln" (
    echo Generating solution files...
    "%CMAKE%" .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64 >nul 2>&1
    if errorlevel 1 (
        "%CMAKE%" .. -DCMAKE_BUILD_TYPE=Release >nul 2>&1
    )
)

:build_existing
REM Build the project
if exist "AuraShell.sln" (
    echo Building AuraShell...
    "%MSBUILD%" AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:q
    if %ERRORLEVEL% EQU 0 (
        if exist "Release\AuraShell.exe" (
            echo.
            echo ========================================
            echo BUILD SUCCESSFUL!
            echo ========================================
            echo Executable: %CD%\Release\AuraShell.exe
            echo.
            echo Copying to build_no_debug\Release...
            if not exist "..\build_no_debug\Release" mkdir "..\build_no_debug\Release"
            copy /Y "Release\AuraShell.exe" "..\build_no_debug\Release\AuraShell.exe" >nul
            echo.
            echo READY! Run: ..\build_no_debug\Release\AuraShell.exe
            echo Then try: voice on
            cd ..
            exit /b 0
        )
    ) else (
        echo BUILD FAILED - Check errors above
        cd ..
        exit /b 1
    )
) else (
    echo Solution file not found. Please open in Visual Studio:
    echo   1. File -^> Open -^> CMake...
    echo   2. Select: %CD%\..\CMakeLists.txt
    echo   3. Build -^> Build All
    cd ..
    exit /b 1
)

cd ..

