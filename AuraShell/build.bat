@echo off
echo Building AuraShell...
echo.

REM Try to find and use Visual Studio Developer Command Prompt
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    goto :build
)

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    goto :build
)

if exist "%ProgramFiles%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    goto :build
)

echo Visual Studio not found. Please build manually using Visual Studio.
echo Open AuraShell.sln in Visual Studio and build the Release configuration.
pause
exit /b 1

:build
echo Compiling AuraShell...
cd build_no_debug
if exist "AuraShell.sln" (
    msbuild AuraShell.sln /p:Configuration=Release /p:Platform=x64 /m
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo Build successful!
        echo Executable: build_no_debug\Release\AuraShell.exe
    ) else (
        echo.
        echo Build failed!
    )
) else (
    echo Solution file not found!
    echo Please run CMake first to generate the solution file.
)
cd ..
pause

