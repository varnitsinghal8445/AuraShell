# AuraShell Build Instructions

## Problem Fixed
The "explain" command was giving a "CreateProcess failed (2)" error because:
1. The executable you were running was built from an older version of the code
2. The "explain" command is now properly registered as a builtin command
3. Improved error handling has been added to provide better error messages

## Changes Made
1. **Improved error handling in `process.c`**: Better error messages when commands are not found
2. **Fallback to cmd.exe**: If a command isn't found directly, it now tries to run it through cmd.exe (similar to Windows cmd.exe behavior)

## How to Build

### Option 1: Using Visual Studio (Recommended)
1. Open `build_no_debug\AuraShell.sln` in Visual Studio
2. Select "Release" configuration and "x64" platform
3. Build the solution (Build > Build Solution or Ctrl+Shift+B)
4. The executable will be at: `build_no_debug\Release\AuraShell.exe`

### Option 2: Using Build Script
1. Run `build.bat` from the AuraShell directory
2. This will automatically set up the build environment and compile

### Option 3: Using CMake (if CMake is installed)
```bash
cd AuraShell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Testing the Fix
After rebuilding, test the explain command:
```
AuraShell > explain open
```

This should now display the explanation for the "open" command instead of the CreateProcess error.

## The "explain" Command
The `explain` command provides detailed information about builtin commands:
- Usage: `explain <command>`
- Example: `explain open` - Shows detailed help for the open command
- Example: `explain dir` - Shows detailed help for the dir command

Available commands to explain:
- exit, cd, help, history, open, theme, newwin, alias, welcome, search
- mkdir, rmdir, dir, pwd, copy, move, del, type, echo

