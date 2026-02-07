#!/bin/bash

# Tess Web Installer
# This script is intended to be hosted at https://tess.sh/install

set -e

REPO_URL="https://github.com/egirlst/tess.git"
TEMP_DIR=$(mktemp -d)

echo "Tess Language Installer"
echo "======================="

# Function to cleanup on exit
cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

# 1. Download/Clone source
echo "Fetching latest version..."
if command -v git >/dev/null 2>&1; then
    git clone --depth 1 "$REPO_URL" "$TEMP_DIR/tess"
else
    echo "Error: git is required to install from source."
    exit 1
fi

cd "$TEMP_DIR/tess"

# 2. Detect OS and Run Installer
OS="$(uname -s)"
echo "Detected OS: $OS"

case "$OS" in
    Linux*)
        echo "Running Linux installer..."
        bash installers/unix/install.sh
        ;;
    Darwin*)
        echo "Running macOS installer..."
        # We use the unix install script which builds and installs to /usr/local/bin
        bash installers/unix/install.sh
        ;;
    MINGW*|CYGWIN*|MSYS*)
        echo "Detected Windows environment."
        echo "Please use the Windows installers provided in the 'installers/windows' directory."
        echo "You can run: powershell installers/windows/install_portable.ps1"
        exit 1
        ;;
    *)
        echo "Unsupported OS: $OS"
        exit 1
        ;;
esac

echo ""
echo "Installation complete!"
echo "You may need to restart your shell."
