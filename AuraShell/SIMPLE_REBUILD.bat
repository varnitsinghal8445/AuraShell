@echo off
echo ========================================
echo AuraShell - Simple Rebuild Script
echo ========================================
echo.
echo This will rebuild AuraShell with the explain command fix.
echo.
echo STEP 1: Close AuraShell if it's running!
echo.
pause

echo.
echo STEP 2: Setting up build environment...
echo.

REM Try to find and setup Visual Studio
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
echo ========================================
echo Visual Studio not found automatically!
echo ========================================
echo.
echo Please do ONE of the following:
echo.
echo OPTION 1 - Visual Studio GUI (EASIEST):
echo   1. Open Visual Studio
echo   2. File -^> Open -^> CMake...
echo   3. Navigate to: D:\AuraShell__\AuraShell\CMakeLists.txt
echo   4. Visual Studio will generate project files automatically
echo   5. Select "Release" and "x64"
echo   6. Build -^> Build All (Ctrl+Shift+B)
echo.
echo OPTION 2 - Developer Command Prompt:
echo   1. Open "Developer Command Prompt for VS 2022"
echo   2. Run these commands:
echo      cd D:\AuraShell__\AuraShell
echo      mkdir build_new
echo      cd build_new
echo      cmake .. -DCMAKE_BUILD_TYPE=Release
echo      cmake --build . --config Release
echo.
pause
exit /b 1

:build
echo Build environment ready!
echo.

REM Check if we need to regenerate CMake files
cd /d "%~dp0"
if not exist "build_no_debug\AuraShell.sln" (
    echo Regenerating CMake project files...
    if exist "build_no_debug" rmdir /s /q "build_no_debug"
    mkdir "build_no_debug"
    cd "build_no_debug"
    cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
    if errorlevel 1 (
        echo CMake failed. Trying without generator specification...
        cmake .. -DCMAKE_BUILD_TYPE=Release
    )
    cd ..
)

cd "build_no_debug"
if exist "AuraShell.sln" (
    echo.
    echo Building AuraShell...
    echo.
    msbuild "AuraShell.sln" /p:Configuration=Release /p:Platform=x64 /m /v:minimal
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ========================================
        echo ✅ BUILD SUCCESSFUL!
        echo ========================================
        echo.
        echo Your new executable is at:
        echo   build_no_debug\Release\AuraShell.exe
        echo.
        echo You can now test:
        echo   explain open
        echo.
    ) else (
        echo.
        echo ========================================
        echo ❌ BUILD FAILED
        echo ========================================
        echo.
        echo Please check the error messages above.
        echo Try building manually in Visual Studio.
        echo.
    )
) else (
    echo.
    echo ERROR: AuraShell.sln not found!
    echo Please regenerate using CMake (see instructions above).
    echo.
)

pause

