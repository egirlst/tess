# Tess Installers

This directory contains scripts to build installers and packages for Windows, macOS, and Linux.

## Windows (.msi)

**Requirements:**
- [WiX Toolset v3](http://wixtoolset.org/)
- MinGW (GCC and mingw32-make)

**Usage:**
Run `installers\windows\build_msi.bat` from Command Prompt or PowerShell.
This will generate `installers\windows\TessInstaller.msi`.

## macOS (.pkg)

**Requirements:**
- macOS
- Xcode Command Line Tools (`pkgbuild`)

**Usage:**
Run `installers/macos/build_pkg.sh` from Terminal.
This will generate `TessInstaller.pkg`.

## Linux (.tar.gz)

**Requirements:**
- Linux (or compatible Unix-like environment)
- GCC, Make

**Usage:**
Run `installers/unix/package.sh`.
This will generate `dist/tess-1.0.0.tar.gz`.

## Linux (.deb)

**Requirements:**
- Debian/Ubuntu based Linux
- `dpkg-deb`

**Usage:**
Run `installers/linux/build_deb.sh`.
This will generate a `.deb` package in `installers/linux/build_deb/`.

## Automated Builds (GitHub Actions)

The project includes a GitHub Actions workflow (`.github/workflows/release.yml`) that automatically builds all these installers when you push a tag (e.g., `v1.0.0`) or push to `main`.
