@echo off
echo ========================================
echo AuraShell - Build and Run Script
echo ========================================
echo.

REM Close any running AuraShell instances
echo Checking for running AuraShell...
taskkill /F /IM AuraShell.exe 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Closed running AuraShell instance.
) else (
    echo No running AuraShell found.
)
echo.

REM Try to setup Visual Studio environment
echo Setting up build environment...
set "VS_FOUND=0"

if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=1"
    goto :build
)

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=1"
    goto :build
)

if exist "%ProgramFiles%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=1"
    goto :build
)

echo.
echo ERROR: Visual Studio build tools not found!
echo.
echo Please install Visual Studio 2019 or 2022 with C++ development tools.
echo Or open the project in Visual Studio and build manually.
echo.
pause
exit /b 1

:build
echo Build environment ready!
echo.

cd /d "%~dp0"

REM Create fresh build directory
if exist "build_fresh" rmdir /s /q "build_fresh"
mkdir "build_fresh"
cd "build_fresh"

REM Try CMake first
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Configuring with CMake...
    cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
    if errorlevel 1 (
        cmake .. -DCMAKE_BUILD_TYPE=Release
    )
    
    if exist "AuraShell.sln" (
        echo.
        echo Building project...
        msbuild "AuraShell.sln" /p:Configuration=Release /p:Platform=x64 /m /v:minimal
        if %ERRORLEVEL% EQU 0 (
            echo.
            echo ========================================
            echo ✅ BUILD SUCCESSFUL!
            echo ========================================
            echo.
            cd ..
            if exist "build_fresh\Release\AuraShell.exe" (
                echo Running AuraShell...
                echo.
                start "AuraShell" "build_fresh\Release\AuraShell.exe"
                echo.
                echo AuraShell started! Test with: explain open
            ) else if exist "build_fresh\x64\Release\AuraShell.exe" (
                echo Running AuraShell...
                echo.
                start "AuraShell" "build_fresh\x64\Release\AuraShell.exe"
                echo.
                echo AuraShell started! Test with: explain open
            ) else (
                echo Executable not found in expected location.
                echo Please check build_fresh directory.
            )
        ) else (
            echo.
            echo ========================================
            echo ❌ BUILD FAILED
            echo ========================================
        )
    ) else (
        echo CMake failed to generate solution file.
    )
) else (
    echo CMake not found. Please install CMake or use Visual Studio GUI.
    echo.
    echo To build manually:
    echo 1. Open Visual Studio
    echo 2. File -^> Open -^> CMake...
    echo 3. Select: CMakeLists.txt
    echo 4. Build -^> Build All
)

cd ..
pause

