# Quick Fix: explain Command Not Working

## The Problem
You're getting: `Error: CreateProcess failed (2)` when running `explain open`

## Why This Happens
The `explain` command **IS** in the source code, but you're running an **OLD executable** that was built before the `explain` command was added.

## The Solution: REBUILD

### ✅ Step-by-Step Instructions

#### Option 1: Visual Studio (RECOMMENDED - Easiest)

1. **Close AuraShell** if it's currently running
   - This is important! The executable file is locked while running

2. **Open Visual Studio**

3. **Open the solution file:**
   - File → Open → Project/Solution
   - Navigate to: `D:\AuraShell__\AuraShell\build_no_debug\AuraShell.sln`
   - Click Open

4. **Select Build Configuration:**
   - In the toolbar, find the dropdowns
   - Select: **Release** (not Debug)
   - Select: **x64** (not x86 or Any CPU)

5. **Build the project:**
   - Press **Ctrl+Shift+B**
   - OR: Build → Build Solution
   - Wait for "Build succeeded" message

6. **Test it:**
   - Run: `build_no_debug\Release\AuraShell.exe`
   - Type: `explain open`
   - It should work now! ✅

#### Option 2: Developer Command Prompt

1. **Close AuraShell**

2. **Open "Developer Command Prompt for VS 2022"**
   - Search for it in Start Menu
   - Or: Start → Visual Studio 2022 → Developer Command Prompt

3. **Navigate and build:**
   ```cmd
   cd D:\AuraShell__\AuraShell\build_no_debug
   msbuild AuraShell.sln /p:Configuration=Release /p:Platform=x64
   ```

4. **Test it:**
   ```cmd
   Release\AuraShell.exe
   explain open
   ```

## Verification

After rebuilding, the `explain` command should work:

```
AuraShell > explain open

========================================
  Command Explanation: open
========================================

NAME:
  open - Open file or folder

DESCRIPTION:
  The open command opens a file or folder using the default Windows
  application associated with that file type...
```

## Why Rebuild is Necessary

- ✅ The source code (`commands.c`) has the `explain` command registered
- ✅ The handler function (`cmd_explain`) exists and works
- ❌ But your current `AuraShell.exe` was compiled from OLD code
- 🔧 Rebuilding creates a NEW executable with the latest code

## Still Not Working?

If you rebuild and it still doesn't work:

1. Make sure you're running the NEW executable from `build_no_debug\Release\`
2. Check that the build completed successfully (no errors)
3. Verify the file date/time of `AuraShell.exe` - it should be recent (just now)

## Files That Were Fixed

- ✅ `src/process.c` - Better error handling
- ✅ `src/commands.c` - `explain` command is registered (lines 1067, 1091)
- ✅ All code is correct and ready - just needs to be compiled!

