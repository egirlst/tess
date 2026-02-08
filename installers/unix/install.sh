#!/bin/bash
set -e

echo "Installing Tess Language..."

# Determine installation directory
INSTALL_DIR="/usr/local/bin"

# Check if we are in a distribution bundle (binaries exist in ./bin relative to script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIST_BIN="$SCRIPT_DIR/bin/tess"

if [ -f "$DIST_BIN" ]; then
    echo "Installing from binary distribution..."
    echo "Installing to $INSTALL_DIR (requires sudo)..."
    sudo cp "$SCRIPT_DIR/bin/tess" "$INSTALL_DIR/"
    if [ -f "$SCRIPT_DIR/bin/ts" ]; then
        sudo cp "$SCRIPT_DIR/bin/ts" "$INSTALL_DIR/"
    fi
else
    # We are likely in source repo structure, try to find root
    # If script is in installers/unix/, root is ../..
    # If script is in root (unlikely for source repo but possible), root is .
    
    if [ -f "$SCRIPT_DIR/Makefile" ]; then
        PROJECT_ROOT="$SCRIPT_DIR"
    elif [ -f "$SCRIPT_DIR/../../Makefile" ]; then
        PROJECT_ROOT="$SCRIPT_DIR/../.."
    else
        echo "Error: Could not find project root (Makefile)."
        exit 1
    fi
    
    echo "Building from source in $PROJECT_ROOT..."
    cd "$PROJECT_ROOT"
    
    # Build
    make
    
    echo "Installing to $INSTALL_DIR (requires sudo)..."
    sudo cp bin/tess "$INSTALL_DIR/"
    sudo cp bin/ts "$INSTALL_DIR/"
fi

echo "Verifying installation..."
if command -v tess >/dev/null 2>&1; then
    tess version
else
    echo "Warning: 'tess' not found in PATH yet. You may need to restart your shell."
    "$INSTALL_DIR/tess" version
fi

echo "Installation complete!"
