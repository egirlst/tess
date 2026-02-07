@echo off
echo Building Tess MSI Installer...

REM Check for WiX Toolset
where candle >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: WiX Toolset not found in PATH.
    echo Please install WiX Toolset v3.x from http://wixtoolset.org/
    echo or add it to your PATH.
    exit /b 1
)

REM Compile
echo Compiling Product.wxs...
candle Product.wxs -out Product.wixobj
if %errorlevel% neq 0 (
    echo Error compiling Product.wxs
    exit /b 1
)

REM Link
echo Linking...
light Product.wixobj -out TessInstaller.msi
if %errorlevel% neq 0 (
    echo Error linking MSI
    exit /b 1
)

echo.
echo Build successful! TessInstaller.msi created.
pause
