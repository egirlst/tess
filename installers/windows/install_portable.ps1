Write-Host "Setting up Tess (Portable Mode)..."

$BinPath = Join-Path (Get-Location).Path "..\..\bin"
$BinPath = [System.IO.Path]::GetFullPath($BinPath)

Write-Host "Adding $BinPath to User PATH..."

$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")

if ($UserPath -notlike "*$BinPath*") {
    $NewPath = "$UserPath;$BinPath"
    [Environment]::SetEnvironmentVariable("Path", $NewPath, "User")
    Write-Host "Success! Please restart your terminal/PowerShell to use 'tess' command."
} else {
    Write-Host "Path already exists."
}

Write-Host "Setup complete."
Pause
