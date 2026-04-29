# Running AuraShell

## Quick Start

To run AuraShell with the `explain` command working:

### Option 1: Use the Launcher (Recommended)
Double-click `run_aurashell.bat` in the root directory, or run:
```cmd
run_aurashell.bat
```

### Option 2: Run Directly
```cmd
cd build\Release
AuraShell.exe
```

### Option 3: From AuraShell Directory
```cmd
cd AuraShell
run_aurashell.bat
```

## VS Code Integration

If you're using VS Code:

1. **Press F5** to build and run AuraShell
2. Or use **Terminal > Run Task > run-aurashell**
3. Or open the integrated terminal and run: `build\Release\AuraShell.exe`

## Important Notes

- The executable with the `explain` command is located at: `build\Release\AuraShell.exe`
- Make sure to close any old AuraShell windows before running a new one
- If `explain` doesn't work, you may be running an old executable - use the launcher above

## Testing the Explain Command

Once AuraShell is running, test it:
```
AuraShell > explain open
AuraShell > explain dir
AuraShell > explain copy
```

You should see detailed help instead of "CreateProcess failed" errors.

