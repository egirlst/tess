#!/bin/bash

# Create a distribution tarball

VERSION="1.0.0"
DIST_NAME="tess-${VERSION}"
DIST_DIR="dist/${DIST_NAME}"

cd "$(dirname "$0")/../.."

echo "Creating distribution package..."

# Clean and build
make clean
make

# Create directory structure
mkdir -p "$DIST_DIR/bin"
mkdir -p "$DIST_DIR/include"
mkdir -p "$DIST_DIR/lib"

# Copy files
cp bin/tess "$DIST_DIR/bin/"
cp bin/ts "$DIST_DIR/bin/"
cp include/*.h "$DIST_DIR/include/"
cp installers/unix/install.sh "$DIST_DIR/"

# Create archive
tar -czf "${DIST_NAME}.tar.gz" -C dist "${DIST_NAME}"

echo "Package created: ${DIST_NAME}.tar.gz"
