# How to Build and Run AuraShell in Cursor AI

## Current Situation
- ✅ Your source code is CORRECT - `explain` command is in the code
- ❌ Your executable is OLD - needs to be rebuilt
- ⚠️ Build tools (CMake/Visual Studio) need to be set up

## Step-by-Step: Build in Cursor AI

### Option 1: Using Visual Studio (Recommended)

1. **Install Visual Studio** (if not installed):
   - Download from: https://visualstudio.microsoft.com/
   - Install "Desktop development with C++" workload
   - This includes CMake and MSBuild

2. **Open in Visual Studio**:
   - Open Visual Studio
   - **File → Open → CMake...**
   - Navigate to: `D:\AuraShell__\AuraShell\CMakeLists.txt`
   - Visual Studio will automatically configure the project

3. **Build**:
   - Select **Release** and **x64** in toolbar
   - Press **Ctrl+Shift+B** or **Build → Build All**
   - Wait for "Build succeeded"

4. **Run**:
   - Press **F5** or **Debug → Start Debugging**
   - Or find executable in: `out\build\x64-Release\AuraShell.exe`

### Option 2: Using Developer Command Prompt

1. **Open "Developer Command Prompt for VS 2022"**
   - Search in Start Menu

2. **Navigate and Build**:
   ```cmd
   cd D:\AuraShell__\AuraShell
   mkdir build_cursor
   cd build_cursor
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```

3. **Run**:
   ```cmd
   Release\AuraShell.exe
   ```

### Option 3: Using the Batch Script I Created

1. **Run the script**:
   ```cmd
   cd D:\AuraShell__\AuraShell
   build_and_run.bat
   ```

   This will:
   - Close any running AuraShell
   - Set up build environment
   - Build the project
   - Run the new executable

## Verify the Build Worked

After building, check:
1. **File date** of new `AuraShell.exe` should be TODAY
2. **Test the command**:
   ```
   AuraShell > explain open
   ```
   Should show explanation, not error!

## If Build Tools Are Missing

If you get "CMake not found" or "MSBuild not found":

1. **Install Visual Studio Community** (free):
   - https://visualstudio.microsoft.com/vs/community/
   - During installation, select:
     - ✅ Desktop development with C++
     - ✅ CMake tools for Windows

2. **Or install CMake separately**:
   - Download from: https://cmake.org/download/
   - Add to PATH during installation

## Quick Test After Building

Once you have the new executable:

```cmd
AuraShell > explain open
```

Should output:
```
========================================
  Command Explanation: open
========================================

NAME:
  open - Open file or folder

DESCRIPTION:
  The open command opens a file or folder using the default Windows
  application associated with that file type...
```

If you still get "CreateProcess failed (2)", you're running the OLD executable!

## Files I Created for You

- `build_and_run.bat` - Automated build and run script
- `BUILD_AND_RUN_IN_CURSOR.md` - This file
- `WHY_NOT_WORKING.md` - Explanation of the problem
- `QUICK_FIX.md` - Quick fix guide

## Summary

Your code is ready! You just need:
1. Visual Studio or CMake installed
2. Build the project (one of the methods above)
3. Run the NEW executable
4. Test: `explain open`

The `explain` command **IS** in your code - it just needs to be compiled!

