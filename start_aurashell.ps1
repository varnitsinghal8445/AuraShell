# AuraShell Launcher for VS Code Terminal
# This ensures you're running the correct executable with the explain command

$exePath = Join-Path $PSScriptRoot "build\Release\AuraShell.exe"

if (Test-Path $exePath) {
    Write-Host "Starting AuraShell from: $exePath" -ForegroundColor Green
    Write-Host "The 'explain' command should work now!" -ForegroundColor Green
    Write-Host ""
    & $exePath
} else {
    Write-Host "Error: AuraShell.exe not found at: $exePath" -ForegroundColor Red
    Write-Host "Please build the project first using:" -ForegroundColor Yellow
    Write-Host "  msbuild build\AuraShell.sln /p:Configuration=Release /p:Platform=x64" -ForegroundColor Yellow
    exit 1
}

