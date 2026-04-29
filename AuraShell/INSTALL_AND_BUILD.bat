@echo off
title Installing Dependencies and Building AuraShell with Voice
echo.
echo ========================================
echo Installing Dependencies and Building
echo ========================================
echo.
echo This will:
echo   1. Set up Visual Studio build environment
echo   2. Generate CMake solution files  
echo   3. Build AuraShell with voice feature
echo   4. Make it ready to run
echo.
pause

REM Try to set up Visual Studio environment
set VCVARS=
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)

if not "%VCVARS%"=="" (
    echo Setting up Visual Studio environment...
    call "%VCVARS%"
    echo Environment ready!
    echo.
)

REM Find MSBuild
set MSBUILD=
where msbuild >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /f "tokens=*" %%i in ('where msbuild') do set MSBUILD=%%i
    goto :found_msbuild
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    goto :found_msbuild
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    goto :found_msbuild
)

echo ERROR: MSBuild not found!
echo Please install Visual Studio 2022 Build Tools
pause
exit /b 1

:found_msbuild
echo Found MSBuild: %MSBUILD%
echo.

REM Find CMake
set CMAKE=
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /f "tokens=*" %%i in ('where cmake') do set CMAKE=%%i
    goto :found_cmake
)
if exist "C:\Program Files\CMake\bin\cmake.exe" (
    set "CMAKE=C:\Program Files\CMake\bin\cmake.exe"
    goto :found_cmake
)

echo Warning: CMake not found. Will try Visual Studio integration...
set CMAKE=

:found_cmake
if not "%CMAKE%"=="" (
    echo Found CMake: %CMAKE%
    echo.
)

REM Create build directory
if not exist "build_voice" mkdir build_voice
cd build_voice

REM Generate solution if CMake available
if not "%CMAKE%"=="" (
    if not exist "AuraShell.sln" (
        echo Generating Visual Studio solution...
        "%CMAKE%" .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
        if errorlevel 1 (
            echo Trying without generator specification...
            "%CMAKE%" .. -DCMAKE_BUILD_TYPE=Release
        )
    )
)

REM Build the project
if exist "AuraShell.sln" (
    echo.
    echo ========================================
    echo Building AuraShell with Voice Feature
    echo ========================================
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
        echo BUILD FAILED
        echo ========================================
        echo.
        echo Please check error messages above.
        cd ..
        pause
        exit /b 1
    )
) else (
    echo.
    echo ========================================
    echo SOLUTION FILE NOT FOUND
    echo ========================================
    echo.
    echo Please generate solution using Visual Studio:
    echo   1. Open Visual Studio
    echo   2. File -^> Open -^> CMake...
    echo   3. Select: %CD%\..\CMakeLists.txt
    echo   4. Visual Studio will generate solution
    echo   5. Then run: %MSBUILD% AuraShell.sln /p:Configuration=Release /p:Platform=x64
    echo.
    cd ..
    pause
    exit /b 1
)

cd ..
pause

