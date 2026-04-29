@echo off
title Building AuraShell with Voice Feature
echo.
echo ========================================
echo Building AuraShell with Voice Feature
echo ========================================
echo.

REM Try to find MSBuild
set MSBUILD=
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)

if "%MSBUILD%"=="" (
    echo Error: MSBuild not found!
    echo Please install Visual Studio 2022 Build Tools or Community Edition
    echo Or open the project in Visual Studio and build manually.
    pause
    exit /b 1
)

echo Found MSBuild: %MSBUILD%
echo.

REM Try to find CMake
set CMAKE=
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set CMAKE=cmake
) else (
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set "CMAKE=C:\Program Files\CMake\bin\cmake.exe"
    )
)

if "%CMAKE%"=="" (
    echo Warning: CMake not found in PATH
    echo You may need to regenerate solution files manually in Visual Studio
    echo.
)

REM Create build directory if needed
if not exist "build_voice" mkdir build_voice
cd build_voice

REM Generate solution if CMake is available
if not "%CMAKE%"=="" (
    if not exist "AuraShell.sln" (
        echo Generating Visual Studio solution...
        "%CMAKE%" .. -DCMAKE_BUILD_TYPE=Release
        if errorlevel 1 (
            echo CMake generation failed!
            echo Please generate solution files manually in Visual Studio
            pause
            exit /b 1
        )
    )
)

REM Build the project
if exist "AuraShell.sln" (
    echo Building AuraShell...
    "%MSBUILD%" AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal
    if errorlevel 1 (
        goto :build_failed
    )
    if not errorlevel 1 (
        goto :build_success
    )
    :build_success
        echo.
        echo ========================================
        echo Build Successful!
        echo ========================================
        echo.
        echo Executable location: build_voice\Release\AuraShell.exe
        echo.
        echo To test the voice feature:
        echo   1. Run: build_voice\Release\AuraShell.exe
        echo   2. Type: voice on
        echo   3. Try commands like: echo Hello, pwd, cd Documents
        echo.
        if exist "Release\AuraShell.exe" (
            echo Copying executable to build_no_debug\Release\...
            if not exist "..\build_no_debug\Release" mkdir "..\build_no_debug\Release"
            copy /Y "Release\AuraShell.exe" "..\build_no_debug\Release\AuraShell.exe"
            echo Executable also copied to: build_no_debug\Release\AuraShell.exe
        )
        goto :build_end
    )
    :build_failed
    echo.
    echo ========================================
    echo Build Failed!
    echo ========================================
    echo.
    echo Please check the error messages above.
    echo Make sure:
    echo   - Windows SDK is installed
    echo   - SAPI (Speech API) is available
    echo   - All source files are present
    :build_end
) else (
    echo Solution file not found!
    echo.
    echo Please do one of the following:
    echo   1. Open Visual Studio
    echo   2. File -^> Open -^> CMake...
    echo   3. Select: %CD%\..\CMakeLists.txt
    echo   4. Build -^> Build All (or F7)
    echo.
)

cd ..
pause

