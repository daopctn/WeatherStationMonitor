# Debian Package Build Guide

This guide explains how to build a `.deb` package for Weather Station Monitor.

## Quick Build

The easiest way to build the package:

```bash
./build-deb.sh
```

This automated script will:
1. Check and install build dependencies
2. Clean previous builds
3. Build the .deb package
4. Show you where the package was created

## Manual Build Process

If you prefer to build manually or need more control:

### Step 1: Install Build Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    debhelper \
    cmake \
    qtbase5-dev \
    qtcharts5-dev \
    libqt5sql5-mysql \
    python3-dev \
    python3-pip
```

### Step 2: Clean Previous Builds

```bash
# Clean the build directory
rm -rf build/

# Clean debian temporary files
rm -rf debian/weather-station-monitor/
rm -f debian/files
rm -rf debian/.debhelper/

# Clean previous packages (in parent directory)
rm -f ../*.deb ../*.buildinfo ../*.changes
```

### Step 3: Build the Package

```bash
# Build unsigned package (for local use)
dpkg-buildpackage -us -uc -b

# Or build with signatures (if you have GPG key set up)
dpkg-buildpackage -b
```

**Flags explained:**
- `-us` - Do not sign the source package
- `-uc` - Do not sign the .changes file
- `-b` - Binary-only build (don't build source package)

### Step 4: Find Your Package

The .deb file will be created in the parent directory:

```bash
ls -lh ../*.deb
```

Expected output:
```
weather-station-monitor_1.0.0_amd64.deb
```

## Testing the Package

### Install Locally

```bash
# Install the package
sudo dpkg -i ../weather-station-monitor_*.deb

# Fix any dependency issues
sudo apt-get install -f
```

### Inspect Package Contents

```bash
# List all files in the package
dpkg -c ../weather-station-monitor_*.deb

# Show package information
dpkg -I ../weather-station-monitor_*.deb

# Show package size
du -h ../weather-station-monitor_*.deb
```

### Verify Installation

```bash
# Check if package is installed
dpkg -l | grep weather-station-monitor

# List installed files
dpkg -L weather-station-monitor

# Check the executable
which weather-station-monitor

# Try running it
weather-station-monitor --version
```

## Package Contents

The .deb package includes:

### Executable
- `/usr/bin/weather-station-monitor` - Main application

### Shared Data
- `/usr/share/weather-station-monitor/python/` - Python scripts
- `/usr/share/weather-station-monitor/config/` - Configuration examples
- `/usr/share/weather-station-monitor/database/` - SQL schema files
- `/usr/share/weather-station-monitor/ui/` - UI files
- `/usr/share/weather-station-monitor/resources/` - Icons and resources

### Documentation
- `/usr/share/doc/weather-station-monitor/README.md`
- `/usr/share/doc/weather-station-monitor/database/README.md`

### Desktop Integration
- `/usr/share/applications/weather-station-monitor.desktop`

### Configuration
- `/etc/weather-station-monitor/config.json` (created during installation)

## Troubleshooting Build Issues

### CMake Errors

**Error:** `Could not find Qt5`
```bash
sudo apt-get install qtbase5-dev qtcharts5-dev
```

**Error:** `Could not find Python3`
```bash
sudo apt-get install python3-dev
```

### Dependency Issues

**Error:** `Unmet dependencies`
```bash
# Update package lists
sudo apt-get update

# Install dependencies automatically
sudo apt-get install -f
```

### Build Fails with "debhelper" Errors

```bash
# Ensure debhelper is installed
sudo apt-get install debhelper

# Check debhelper version
dpkg -l | grep debhelper
# Should be version 11 or higher
```

### Permission Errors

```bash
# Ensure debian/rules is executable
chmod +x debian/rules

# Ensure debian/postinst is executable
chmod +x debian/postinst
```

## Customizing the Package

### Change Package Version

Edit `debian/changelog`:
```bash
nano debian/changelog
```

Update the version number on the first line:
```
weather-station-monitor (1.1.0) jammy; urgency=medium
```

### Change Package Maintainer

Edit `debian/control`:
```bash
nano debian/control
```

Update the Maintainer field:
```
Maintainer: Your Name <your.email@example.com>
```

### Add Dependencies

Edit `debian/control` and add to the `Depends:` section:
```
Depends: ${shlibs:Depends},
         ${misc:Depends},
         your-new-dependency
```

## Distribution

### Create a Repository

For distributing your package, you can:

1. **GitHub Releases** - Upload the .deb file to GitHub releases
2. **PPA (Personal Package Archive)** - Create a Launchpad PPA
3. **Local Repository** - Set up your own apt repository

### GitHub Release Example

```bash
# Create a git tag
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0

# Upload the .deb file to GitHub releases page
```

## Clean Up

After building and testing:

```bash
# Remove the installed package
sudo apt-get remove weather-station-monitor

# Clean build artifacts
./build-deb.sh  # Has cleanup built-in
# or
rm -rf build/ debian/weather-station-monitor/
```

## Advanced: Building for Multiple Architectures

The package is currently built for `amd64`. To build for other architectures:

```bash
# For arm64
dpkg-buildpackage -a arm64 -us -uc -b

# For i386
dpkg-buildpackage -a i386 -us -uc -b
```

Note: You'll need cross-compilation tools installed.

## References

- [Debian New Maintainers' Guide](https://www.debian.org/doc/manuals/maint-guide/)
- [Debian Policy Manual](https://www.debian.org/doc/debian-policy/)
- [Ubuntu Packaging Guide](https://packaging.ubuntu.com/html/)

---

**Need Help?**
- Check the build logs in `debian/` directory
- Review CMake output in the build process
- Ensure all dependencies are correctly specified in `debian/control`
