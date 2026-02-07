#!/bin/bash

# Script to build a Debian package (.deb)
# Run this on Linux (Ubuntu/Debian)

APP_NAME="tess"
VERSION="1.0.0"
ARCH="amd64" # Change if building on ARM/etc
MAINTAINER="Tess Project <maintainer@example.com>"
DESC="The Tess Programming Language"

BUILD_DIR="build_deb"
PKG_DIR="${APP_NAME}_${VERSION}_${ARCH}"
DEBIAN_DIR="${BUILD_DIR}/${PKG_DIR}/DEBIAN"
USR_DIR="${BUILD_DIR}/${PKG_DIR}/usr/local"

echo "Building Debian Package..."

# 1. Clean and Build
cd "$(dirname "$0")/../.."
make clean
make

# 2. Create Directory Structure
mkdir -p "${DEBIAN_DIR}"
mkdir -p "${USR_DIR}/bin"
mkdir -p "${USR_DIR}/include/tess"

# 3. Copy Files
cp bin/tess "${USR_DIR}/bin/"
cp bin/ts "${USR_DIR}/bin/"
cp compiler/include/*.h "${USR_DIR}/include/tess/"

# 4. Create Control File
cat > "${DEBIAN_DIR}/control" <<EOF
Package: ${APP_NAME}
Version: ${VERSION}
Section: devel
Priority: optional
Architecture: ${ARCH}
Depends: libc6 (>= 2.14)
Maintainer: ${MAINTAINER}
Description: ${DESC}
 Tess is a simple, embeddable scripting language.
EOF

# 5. Build Package
dpkg-deb --build "${BUILD_DIR}/${PKG_DIR}"

echo "Done! Package created: ${BUILD_DIR}/${PKG_DIR}.deb"
