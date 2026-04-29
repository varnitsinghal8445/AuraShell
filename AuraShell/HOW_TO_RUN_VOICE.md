# How to Run the Voice Command in AuraShell

## Step 1: Build the Project

### Option A: Using Visual Studio (Recommended)

1. **Open Visual Studio 2022**
   - If you don't have it, download Visual Studio Community (free) from Microsoft

2. **Open the Project**
   - Click **File** → **Open** → **CMake...**
   - Navigate to: `D:\AuraShell__\AuraShell\CMakeLists.txt`
   - Click **Open**

3. **Build the Project**
   - Click **Build** → **Build All** (or press **F7**)
   - Wait for "Build succeeded" message
   - The executable will be in: `out\build\x64-Release\AuraShell.exe` or similar

### Option B: Using Command Line

1. **Open Developer Command Prompt for VS 2022**
   - Search "Developer Command Prompt" in Start Menu
   - Or open PowerShell/CMD in the AuraShell directory

2. **Build using MSBuild**
   ```cmd
   cd D:\AuraShell__\AuraShell
   mkdir build_voice
   cd build_voice
   cmake .. -DCMAKE_BUILD_TYPE=Release
   msbuild AuraShell.sln /p:Configuration=Release /p:Platform=x64
   ```

## Step 2: Run AuraShell

### Option A: From Visual Studio
- Press **F5** to build and run
- Or find the executable in the build folder and double-click it

### Option B: From Command Line
```cmd
cd D:\AuraShell__\AuraShell\build_voice\Release
AuraShell.exe
```

### Option C: Using the Launcher
```cmd
cd D:\AuraShell__\AuraShell
run_aurashell.bat
```

## Step 3: Use the Voice Command

Once AuraShell is running, use these commands:

### Enable Voice:
```
AuraShell > voice on
```
**Output:** "Voice enabled. Command outputs will be spoken." (and it will speak "Voice enabled")

### Test Voice Output:
```
AuraShell > echo Hello World
```
**Output:** Prints "Hello World" and speaks it

```
AuraShell > pwd
```
**Output:** Prints current directory and speaks it

```
AuraShell > cd Documents
```
**Output:** On success, speaks "Directory changed"
**Output:** On error, speaks "Directory not found" or appropriate error

```
AuraShell > mkdir TestFolder
```
**Output:** On success, speaks "Directory created"
**Output:** On error, speaks "Directory already exists" or error message

```
AuraShell > copy file.txt backup.txt
```
**Output:** On success, speaks "File copied successfully"
**Output:** On error, speaks appropriate error message

### Check Voice Status:
```
AuraShell > voice status
```
**Output:** Shows "Voice: ON (enabled)" or "Voice: OFF"

### Disable Voice:
```
AuraShell > voice off
```
**Output:** "Voice disabled."

### Get Help:
```
AuraShell > help
```
Shows all commands including voice

```
AuraShell > explain voice
```
Shows detailed explanation of the voice command

## Quick Test Example

```
AuraShell > voice on
Voice enabled. Command outputs will be spoken.
[Speaks: "Voice enabled"]

AuraShell > echo Testing voice feature
Testing voice feature
[Speaks: "Testing voice feature"]

AuraShell > pwd
D:\AuraShell__
[Speaks: "D:\AuraShell__"]

AuraShell > voice status
Voice: ON (enabled)

AuraShell > voice off
Voice disabled.
```

## Troubleshooting

### "voice: Failed to initialize voice system"
- Make sure Windows Speech API is available
- Check Windows Settings → Speech → Speech Recognition
- Try running as administrator
- Restart AuraShell

### Voice not speaking
- Check Windows volume settings
- Check if voice is enabled: `voice status`
- Make sure you ran `voice on` first
- Verify Windows has speech voices installed

### Build Errors
- Make sure Visual Studio 2022 is installed
- Ensure Windows SDK is installed
- Check that all source files are present
- Verify CMakeLists.txt includes voice.c and links sapi library

## Notes

- Voice is **disabled by default** - you must run `voice on` to enable
- Voice state is **not saved** - you need to enable it each session
- Voice output is **asynchronous** - it won't block commands
- Only **text outputs** are spoken, not binary data

