#!/bin/bash

# Tess Installer for macOS and Linux

set -e

echo "Installing Tess Language..."

# Navigate to project root
cd "$(dirname "$0")/../.."

# Build
echo "Building..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS specific
    echo "Detected macOS"
    make
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux specific
    echo "Detected Linux"
    make
else
    echo "Unknown OS, attempting generic build..."
    make
fi

# Install
INSTALL_DIR="/usr/local/bin"
echo "Installing to $INSTALL_DIR (requires sudo)..."

sudo cp bin/tess "$INSTALL_DIR/"
sudo cp bin/ts "$INSTALL_DIR/"

echo "Verifying installation..."
tess version || "$INSTALL_DIR/tess" version

echo "Installation complete!"
