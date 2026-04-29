@echo off
title Auto-Building AuraShell with Voice Feature
echo.
echo ========================================
echo Auto-Building AuraShell with Voice
echo ========================================
echo.
echo This script will:
echo   1. Check for Visual Studio and build tools
echo   2. Generate CMake solution files
echo   3. Build the project with voice feature
echo   4. Copy executable to ready location
echo.
pause

REM Find MSBuild
set MSBUILD=
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    goto :found_msbuild
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    goto :found_msbuild
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    goto :found_msbuild
)

echo ERROR: MSBuild not found!
echo.
echo Please install Visual Studio 2022 Build Tools or Community Edition
echo Download from: https://visualstudio.microsoft.com/
echo Make sure to install "Desktop development with C++" workload
echo.
pause
exit /b 1

:found_msbuild
echo Found MSBuild: %MSBUILD%
echo.

REM Find CMake
set CMAKE=
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set CMAKE=cmake
    goto :found_cmake
)
if exist "C:\Program Files\CMake\bin\cmake.exe" (
    set "CMAKE=C:\Program Files\CMake\bin\cmake.exe"
    goto :found_cmake
)
if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" (
    set "CMAKE=C:\Program Files (x86)\CMake\bin\cmake.exe"
    goto :found_cmake
)

echo Warning: CMake not found in PATH
echo Will try to use Visual Studio's integrated CMake...
echo Or you can install CMake from: https://cmake.org/download/
echo.
echo For now, trying to build with existing solution files...
set CMAKE=
goto :build_existing

:found_cmake
echo Found CMake: %CMAKE%
echo.

REM Create build directory
if not exist "build_voice" mkdir build_voice
cd build_voice

REM Generate solution if needed
if not exist "AuraShell.sln" (
    echo Generating Visual Studio solution...
    "%CMAKE%" .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
    if errorlevel 1 (
        echo CMake generation failed, trying without generator...
        "%CMAKE%" .. -DCMAKE_BUILD_TYPE=Release
    )
    if errorlevel 1 (
        echo CMake failed. Will try to use existing solution files.
        cd ..
        goto :build_existing
    )
)

REM Build the project
if exist "AuraShell.sln" (
    echo.
    echo Building AuraShell with voice feature...
    echo.
    "%MSBUILD%" AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ========================================
        echo BUILD SUCCESSFUL!
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
            echo READY TO USE!
            echo ========================================
            echo.
            echo Run: ..\build_no_debug\Release\AuraShell.exe
            echo Then try: voice on
            echo.
            cd ..
            pause
            exit /b 0
        )
    ) else (
        echo.
        echo ========================================
        echo BUILD FAILED!
        echo ========================================
        echo.
        echo Please check error messages above.
        cd ..
        pause
        exit /b 1
    )
)

cd ..
goto :end

:build_existing
echo.
echo Trying to build from existing solution files...
if exist "build_no_debug\AuraShell.sln" (
    cd build_no_debug
    echo Building...
    "%MSBUILD%" AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal
    if %ERRORLEVEL% EQU 0 (
        if exist "Release\AuraShell.exe" (
            echo.
            echo ========================================
            echo BUILD SUCCESSFUL!
            echo ========================================
            echo.
            echo Executable: %CD%\Release\AuraShell.exe
            echo Ready to test: voice on
            cd ..
            pause
            exit /b 0
        )
    )
    cd ..
) else (
    echo.
    echo ========================================
    echo CANNOT BUILD WITHOUT CMAKE
    echo ========================================
    echo.
    echo Please do ONE of the following:
    echo.
    echo Option 1: Install CMake
    echo   1. Download from: https://cmake.org/download/
    echo   2. Install and add to PATH
    echo   3. Run this script again
    echo.
    echo Option 2: Use Visual Studio GUI
    echo   1. Open Visual Studio
    echo   2. File -^> Open -^> CMake...
    echo   3. Select: %CD%\CMakeLists.txt
    echo   4. Build -^> Build All (F7)
    echo.
    pause
    exit /b 1
)

:end
pause

