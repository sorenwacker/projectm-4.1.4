#!/bin/bash
# Script to create a macOS .app bundle for projectM

set -e  # Exit on error

APP_NAME="projectM"
BUILD_DIR="build"
APP_BUNDLE="${APP_NAME}.app"
CONTENTS_DIR="${APP_BUNDLE}/Contents"
MACOS_DIR="${CONTENTS_DIR}/MacOS"
RESOURCES_DIR="${CONTENTS_DIR}/Resources"

echo "Creating macOS .app bundle for ${APP_NAME}..."

# Clean up old bundle if it exists
if [ -d "${APP_BUNDLE}" ]; then
    echo "Removing old app bundle..."
    rm -rf "${APP_BUNDLE}"
fi

# Create app bundle structure
echo "Creating bundle structure..."
mkdir -p "${MACOS_DIR}"
mkdir -p "${RESOURCES_DIR}"
mkdir -p "${CONTENTS_DIR}/Frameworks"

# Copy executable
echo "Copying executable..."
if [ -f "${BUILD_DIR}/src/sdl-test-ui/projectM-Test-UI" ]; then
    cp "${BUILD_DIR}/src/sdl-test-ui/projectM-Test-UI" "${MACOS_DIR}/${APP_NAME}"
    chmod +x "${MACOS_DIR}/${APP_NAME}"
else
    echo "Error: Executable not found. Please build the project first."
    exit 1
fi

# Copy required dylibs
echo "Copying dynamic libraries..."
FRAMEWORKS_DIR="${CONTENTS_DIR}/Frameworks"
if [ -f "${BUILD_DIR}/src/libprojectM/libprojectM-4.4.1.4.dylib" ]; then
    cp "${BUILD_DIR}/src/libprojectM/libprojectM-4.4.1.4.dylib" "${FRAMEWORKS_DIR}/"
    ln -sf "libprojectM-4.4.1.4.dylib" "${FRAMEWORKS_DIR}/libprojectM-4.4.dylib"
fi
if [ -f "${BUILD_DIR}/src/playlist/libprojectM-4-playlist.4.1.4.dylib" ]; then
    cp "${BUILD_DIR}/src/playlist/libprojectM-4-playlist.4.1.4.dylib" "${FRAMEWORKS_DIR}/"
    ln -sf "libprojectM-4-playlist.4.1.4.dylib" "${FRAMEWORKS_DIR}/libprojectM-4-playlist.4.dylib"
fi

# Copy presets from Homebrew installation
echo "Copying presets..."
HOMEBREW_PRESETS="/opt/homebrew/Cellar/projectm/3.1.12/share/projectM/presets"
if [ -d "${HOMEBREW_PRESETS}" ]; then
    cp -R "${HOMEBREW_PRESETS}" "${RESOURCES_DIR}/presets"
    echo "Copied presets from Homebrew installation"
else
    echo "Warning: Homebrew presets not found at ${HOMEBREW_PRESETS}"
    echo "Creating empty presets directory..."
    mkdir -p "${RESOURCES_DIR}/presets"
fi

# Create favorites folder
echo "Creating favorites folder..."
mkdir -p "${RESOURCES_DIR}/presets/favorites"

# Copy config if it exists
if [ -f "config.inp" ]; then
    echo "Copying config.inp..."
    cp config.inp "${RESOURCES_DIR}/"
fi

# Copy documentation
if [ -f "FEATURES.md" ]; then
    echo "Copying documentation..."
    cp FEATURES.md "${RESOURCES_DIR}/"
fi

# Create Info.plist
echo "Creating Info.plist..."
cat > "${CONTENTS_DIR}/Info.plist" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>projectM</string>
    <key>CFBundleIdentifier</key>
    <string>com.projectm.visualizer</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>projectM</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>4.1.4</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSMicrophoneUsageDescription</key>
    <string>projectM needs access to the microphone to visualize audio input.</string>
</dict>
</plist>
EOF

# Create a helper script that will be run in Terminal
echo "Creating run script..."
cat > "${MACOS_DIR}/run.sh" << 'EOF'
#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
RESOURCES_DIR="${DIR}/../Resources"
export DATADIR_PATH="${RESOURCES_DIR}"

echo "========================================"
echo "projectM Music Visualizer v4.1.4"
echo "========================================"
echo ""
echo "Press H for help menu"
echo "Press CMD+Q to quit"
echo ""

"${DIR}/projectM-bin"

echo ""
echo "projectM exited. Press any key to close this window..."
read -n 1
EOF

chmod +x "${MACOS_DIR}/run.sh"

# Create the main launcher that opens Terminal
echo "Creating launcher script..."
cat > "${MACOS_DIR}/${APP_NAME}-launcher" << 'EOF'
#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

open -a Terminal.app "${DIR}/run.sh"
EOF

# Make launcher executable and rename
chmod +x "${MACOS_DIR}/${APP_NAME}-launcher"
mv "${MACOS_DIR}/${APP_NAME}" "${MACOS_DIR}/${APP_NAME}-bin"
mv "${MACOS_DIR}/${APP_NAME}-launcher" "${MACOS_DIR}/${APP_NAME}"

# Fix library paths for the executable and dylibs
echo "Fixing library paths..."
install_name_tool -add_rpath "@executable_path/../Frameworks" "${MACOS_DIR}/${APP_NAME}-bin" 2>/dev/null || true

# Fix inter-library dependencies
if [ -f "${FRAMEWORKS_DIR}/libprojectM-4-playlist.4.dylib" ]; then
    install_name_tool -change "@rpath/libprojectM-4.4.dylib" "@rpath/libprojectM-4.4.dylib" "${FRAMEWORKS_DIR}/libprojectM-4-playlist.4.1.4.dylib" 2>/dev/null || true
    install_name_tool -id "@rpath/libprojectM-4-playlist.4.dylib" "${FRAMEWORKS_DIR}/libprojectM-4-playlist.4.1.4.dylib" 2>/dev/null || true
fi
if [ -f "${FRAMEWORKS_DIR}/libprojectM-4.4.dylib" ]; then
    install_name_tool -id "@rpath/libprojectM-4.4.dylib" "${FRAMEWORKS_DIR}/libprojectM-4.4.1.4.dylib" 2>/dev/null || true
fi

# Code sign the app (ad-hoc signature)
echo "Code signing the application..."
codesign --force --deep --sign - "${APP_BUNDLE}" 2>/dev/null || {
    echo "Warning: Code signing failed. The app may trigger Gatekeeper warnings."
}

# Remove quarantine attribute to help with distribution
echo "Removing quarantine attributes..."
xattr -cr "${APP_BUNDLE}" 2>/dev/null || true

echo ""
echo "✅ macOS app bundle created successfully!"
echo "📦 Location: ${APP_BUNDLE}"
echo ""
echo "To test the app:"
echo "  open ${APP_BUNDLE}"
echo ""
echo "To create a DMG for distribution:"
echo "  ./create_dmg.sh"
echo ""

# Count presets
PRESET_COUNT=$(find "${RESOURCES_DIR}/presets" -name "*.milk" -o -name "*.prjm" | wc -l)
echo "📊 Packaged ${PRESET_COUNT} presets"
echo "📁 Favorites folder ready at: ${RESOURCES_DIR}/presets/favorites/"
