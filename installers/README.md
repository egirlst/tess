# Tess Installers

This directory contains installer scripts and configuration for Windows, macOS, and Linux.

## Windows

### Option 1: Portable Install (Recommended)
1. Run `windows/install_portable.ps1` with PowerShell.
2. It automatically adds `tess` to your User PATH.
3. Restart your terminal to use `tess` and `ts`.

### Option 2: MSI Installer (Native)
To build the standard `.msi` installer file (requires **WiX Toolset**):
1. Run `windows/build_msi.bat`.
2. This generates `TessInstaller.msi`.
3. Running this file installs Tess and automatically adds it to the System PATH.

## macOS

### Option 1: Native .pkg Installer
To build a double-clickable `.pkg` installer file (run this on a Mac):
1. Run:
   ```bash
   bash installers/macos/build_pkg.sh
   ```
2. This generates `TessInstaller.pkg`.
3. Installing this puts `tess` in `/usr/local/bin` (automatically in your PATH).

### Option 2: Shell Install
Run:
```bash
bash installers/unix/install.sh
```

## Linux

### Shell Install
Run:
```bash
bash installers/unix/install.sh
```
This builds the project and copies binaries to `/usr/local/bin` (standard PATH location).

### Distribution
To create a tarball for distribution:
```bash
bash installers/unix/package.sh
```
