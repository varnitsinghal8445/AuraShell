@echo off
echo ========================================
echo AuraShell Rebuild Script
echo ========================================
echo.
echo This script will rebuild AuraShell with the latest code changes.
echo.
echo IMPORTANT: Close AuraShell if it's currently running!
echo.
pause

REM Try to find and setup Visual Studio environment
echo.
echo Setting up build environment...

REM Try Visual Studio 2022 Community
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    goto :build
)

REM Try Visual Studio 2022 in Program Files (x86)
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    goto :build
)

REM Try Visual Studio 2019
if exist "%ProgramFiles%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    goto :build
)

echo.
echo ERROR: Visual Studio not found!
echo.
echo Please do ONE of the following:
echo.
echo Option 1: Open Visual Studio
echo   1. Open: build_no_debug\AuraShell.sln
echo   2. Select "Release" and "x64"
echo   3. Press Ctrl+Shift+B to build
echo.
echo Option 2: Use Developer Command Prompt
echo   1. Open "Developer Command Prompt for VS 2022"
echo   2. Navigate to: D:\AuraShell__\AuraShell\build_no_debug
echo   3. Run: msbuild AuraShell.sln /p:Configuration=Release /p:Platform=x64
echo.
pause
exit /b 1

:build
echo Build environment setup complete!
echo.

REM Check if we need to regenerate CMake files
if not exist "build_no_debug\AuraShell.sln" (
    echo Regenerating CMake project files...
    cd ..
    if exist "build_no_debug" rmdir /s /q build_no_debug
    mkdir build_no_debug
    cd build_no_debug
    cmake .. -DCMAKE_BUILD_TYPE=Release
    if errorlevel 1 (
        echo CMake failed. Trying to use existing project files...
        cd ..
        cd build_no_debug
    )
)

cd build_no_debug
echo.
echo Building AuraShell...
echo.

if exist "AuraShell.sln" (
    msbuild AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ========================================
        echo BUILD SUCCESSFUL!
        echo ========================================
        echo.
        echo The new executable is at:
        echo build_no_debug\Release\AuraShell.exe
        echo.
        echo You can now test: explain open
        echo.
    ) else (
        echo.
        echo ========================================
        echo BUILD FAILED!
        echo ========================================
        echo.
        echo Please check the error messages above.
        echo.
    )
) else (
    echo ERROR: AuraShell.sln not found!
    echo Please regenerate the project files using CMake.
)

pause

