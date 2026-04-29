@echo off
echo Building and Running AuraShell with Voice Feature...
echo.

REM Use Visual Studio MSBuild directly
set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

REM Create build directory
cd /d "%~dp0"
if not exist "build_voice" mkdir build_voice
cd build_voice

REM Try to find CMake
set CMAKE=
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set CMAKE=cmake
)
if not "%CMAKE%"=="" (
    if not exist "AuraShell.sln" (
        echo Generating solution...
        cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64 2>nul || cmake .. -DCMAKE_BUILD_TYPE=Release 2>nul
    )
)

REM Build
if exist "AuraShell.sln" (
    echo Building...
    "%MSBUILD%" AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:q
    if %ERRORLEVEL% EQU 0 (
        if exist "Release\AuraShell.exe" (
            echo Build successful!
            echo.
            cd Release
            echo Starting AuraShell with voice feature...
            echo Try: voice on
            echo.
            start "AuraShell - Voice" cmd /k "AuraShell.exe"
            exit /b 0
        )
    )
)

echo.
echo Build failed or solution not found.
echo.
echo Please build manually in Visual Studio:
echo   1. Open Visual Studio
echo   2. File -^> Open -^> CMake...
echo   3. Select: %CD%\..\CMakeLists.txt
echo   4. Build -^> Build All (F7)
echo.
cd ..
pause

