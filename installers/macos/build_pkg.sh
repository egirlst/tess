#!/bin/bash

# Script to build a macOS .pkg installer
# Run this on macOS

APP_NAME="Tess"
VERSION="1.0.0"
IDENTIFIER="com.tess.language"
INSTALL_LOCATION="/usr/local/bin"

# Directories
BUILD_DIR="build_macos"
PAYLOAD_DIR="${BUILD_DIR}/payload"
OUTPUT_PKG="TessInstaller.pkg"

# 1. Clean and Build
echo "Building Tess..."
cd "$(dirname "$0")/../.."
make clean
make

# 2. Prepare Payload (The files to install)
echo "Preparing payload..."
mkdir -p "${PAYLOAD_DIR}"
cp bin/tess "${PAYLOAD_DIR}/"
cp bin/ts "${PAYLOAD_DIR}/"

# 3. Build the Package
echo "Building .pkg..."
# We use --root to specify the content, and --install-location to specify where it goes.
# Note: /usr/local/bin might require sudo to install, so the installer will ask for password.

if ! command -v pkgbuild &> /dev/null; then
    echo "Error: pkgbuild not found. Are you running this on macOS?"
    exit 1
fi

pkgbuild --root "${PAYLOAD_DIR}" \
         --identifier "${IDENTIFIER}" \
         --version "${VERSION}" \
         --install-location "${INSTALL_LOCATION}" \
         "${OUTPUT_PKG}"

echo "Done! Created ${OUTPUT_PKG}"
echo "This installer will place 'tess' and 'ts' into ${INSTALL_LOCATION}, which is in your PATH."
