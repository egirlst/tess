#!/bin/bash

# Tess Installer Script
# Detects OS and installs the appropriate version of Tess

REPO="egirlst/tess"
LATEST_RELEASE_URL="https://api.github.com/repos/$REPO/releases/latest"

echo "Tess Installer"
echo "=============="

# Detect OS
OS="$(uname -s)"
case "${OS}" in
    Linux*)     OS_TYPE=Linux;;
    Darwin*)    OS_TYPE=Mac;;
    CYGWIN*|MINGW*|MSYS*) OS_TYPE=Windows;;
    *)          OS_TYPE="UNKNOWN:${OS}"
esac

echo "Detected OS: $OS_TYPE"

# Function to download and install
install_linux() {
    echo "Fetching latest release info..."
    # Check if we can use apt (Debian/Ubuntu)
    if command -v dpkg >/dev/null 2>&1; then
        echo "Detected Debian/Ubuntu system."
        DOWNLOAD_URL=$(curl -s $LATEST_RELEASE_URL | grep "browser_download_url.*deb" | cut -d : -f 2,3 | tr -d \")
        
        if [ -z "$DOWNLOAD_URL" ]; then
            echo "Error: Could not find .deb package in latest release."
            exit 1
        fi

        echo "Downloading $DOWNLOAD_URL..."
        curl -L -o tess.deb "$DOWNLOAD_URL"
        
        echo "Installing..."
        sudo dpkg -i tess.deb
        rm tess.deb
        echo "Done! Type 'tess' to start."
    else
        # Fallback to tarball
        echo "Using generic tarball installation."
        DOWNLOAD_URL=$(curl -s $LATEST_RELEASE_URL | grep "browser_download_url.*tar.gz" | cut -d : -f 2,3 | tr -d \")
        
        if [ -z "$DOWNLOAD_URL" ]; then
            echo "Error: Could not find .tar.gz package in latest release."
            exit 1
        fi

        echo "Downloading $DOWNLOAD_URL..."
        curl -L -o tess.tar.gz "$DOWNLOAD_URL"
        
        echo "Extracting..."
        tar -xzf tess.tar.gz
        cd tess-* 
        # Assuming the tarball contains an install script or binaries
        if [ -f "install.sh" ]; then
            sudo ./install.sh
        else
            echo "Manual installation required from extracted directory."
        fi
        cd ..
        rm tess.tar.gz
        echo "Done."
    fi
}

install_mac() {
    echo "Fetching latest release info..."
    DOWNLOAD_URL=$(curl -s $LATEST_RELEASE_URL | grep "browser_download_url.*pkg" | cut -d : -f 2,3 | tr -d \")
    
    if [ -z "$DOWNLOAD_URL" ]; then
        echo "Error: Could not find .pkg installer in latest release."
        exit 1
    fi

    echo "Downloading $DOWNLOAD_URL..."
    curl -L -o tess.pkg "$DOWNLOAD_URL"
    
    echo "Installing..."
    sudo installer -pkg tess.pkg -target /
    rm tess.pkg
    echo "Done! Type 'tess' to start."
}

install_windows() {
    echo "Fetching latest release info..."
    DOWNLOAD_URL=$(curl -s $LATEST_RELEASE_URL | grep "browser_download_url.*msi" | cut -d : -f 2,3 | tr -d \")
    
    if [ -z "$DOWNLOAD_URL" ]; then
        echo "Error: Could not find .msi installer in latest release."
        exit 1
    fi

    echo "Downloading $DOWNLOAD_URL..."
    curl -L -o tess_installer.msi "$DOWNLOAD_URL"
    
    echo "Launching installer..."
    cmd.exe /c "start tess_installer.msi"
    echo "Follow the prompts to complete installation."
}

# Run Installation
if [ "$OS_TYPE" == "Linux" ]; then
    install_linux
elif [ "$OS_TYPE" == "Mac" ]; then
    install_mac
elif [ "$OS_TYPE" == "Windows" ]; then
    install_windows
else
    echo "Unsupported operating system: $OS"
    exit 1
fi
