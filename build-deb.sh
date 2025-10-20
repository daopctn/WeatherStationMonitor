#!/bin/bash
# Build script for creating Weather Station Monitor .deb package

set -e  # Exit on any error

echo "==========================================="
echo "  Weather Station Monitor - DEB Builder"
echo "==========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ] || [ ! -d "debian" ]; then
    echo -e "${RED}Error: This script must be run from the project root directory${NC}"
    echo "Make sure you have both CMakeLists.txt and debian/ directory"
    exit 1
fi

# Check for required tools
echo "Checking for required build tools..."
MISSING_TOOLS=""

for tool in dpkg-buildpackage debhelper cmake; do
    if ! command -v $tool &> /dev/null; then
        MISSING_TOOLS="$MISSING_TOOLS $tool"
    fi
done

if [ -n "$MISSING_TOOLS" ]; then
    echo -e "${YELLOW}Missing tools:$MISSING_TOOLS${NC}"
    echo ""
    echo "Installing required build tools..."
    sudo apt-get update
    sudo apt-get install -y build-essential debhelper cmake \
        qtbase5-dev libqt5charts5-dev libqt5sql5-mysql \
        python3-dev python3-pip
fi

# Check for Qt5
echo "Checking for Qt5 development packages..."
if ! dpkg -l | grep -q qtbase5-dev; then
    echo -e "${YELLOW}Qt5 development packages not found. Installing...${NC}"
    sudo apt-get install -y qtbase5-dev libqt5charts5-dev libqt5sql5-mysql
fi

echo -e "${GREEN}All build dependencies satisfied!${NC}"
echo ""

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf build/
rm -rf debian/weather-station-monitor/
rm -f ../weather-station-monitor_*.deb
rm -f ../weather-station-monitor_*.buildinfo
rm -f ../weather-station-monitor_*.changes
rm -f ../weather-station-monitor_*.tar.xz

echo ""
echo "==========================================="
echo "  Building .deb package..."
echo "==========================================="
echo ""

# Build the package
dpkg-buildpackage -us -uc -b

echo ""
echo "==========================================="
echo -e "${GREEN}  Build Complete!${NC}"
echo "==========================================="
echo ""

# Find the generated .deb file
DEB_FILE=$(ls -t ../*.deb 2>/dev/null | head -1)

if [ -n "$DEB_FILE" ]; then
    echo -e "${GREEN}Package created successfully:${NC}"
    echo "  $DEB_FILE"
    echo ""
    echo "Package size: $(du -h "$DEB_FILE" | cut -f1)"
    echo ""
    echo "To install:"
    echo "  sudo dpkg -i $DEB_FILE"
    echo "  sudo apt-get install -f  # Fix any dependency issues"
    echo ""
    echo "To inspect package contents:"
    echo "  dpkg -c $DEB_FILE"
    echo ""
    echo "To get package info:"
    echo "  dpkg -I $DEB_FILE"
    echo ""
else
    echo -e "${RED}Error: .deb file not found!${NC}"
    exit 1
fi
