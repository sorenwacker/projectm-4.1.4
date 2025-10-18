#!/bin/bash
# Script to create a DMG installer for projectM

set -e  # Exit on error

APP_NAME="projectM"
APP_BUNDLE="${APP_NAME}.app"
DMG_NAME="${APP_NAME}-v4.1.4-macOS.dmg"
DMG_TEMP="temp_dmg"

echo "Creating DMG installer for ${APP_NAME}..."

# Check if app bundle exists
if [ ! -d "${APP_BUNDLE}" ]; then
    echo "Error: ${APP_BUNDLE} not found. Please run ./create_macos_app.sh first."
    exit 1
fi

# Clean up old DMG and temp folder
rm -rf "${DMG_TEMP}"
rm -f "${DMG_NAME}"

# Create temporary directory
echo "Creating temporary directory..."
mkdir -p "${DMG_TEMP}"

# Copy app bundle to temp directory
echo "Copying app bundle..."
cp -R "${APP_BUNDLE}" "${DMG_TEMP}/"

# Create Applications symlink
echo "Creating Applications symlink..."
ln -s /Applications "${DMG_TEMP}/Applications"

# Create README
echo "Creating README..."
cat > "${DMG_TEMP}/README.txt" << 'EOF'
projectM v4.1.4 - Music Visualizer (Custom Build)
==================================================

Installation:
1. Drag projectM.app to the Applications folder
2. Open projectM from Applications
3. Grant microphone access when prompted (for audio visualization)

New Features:
- ⏱️  Time scale control (0.01x - 2.0x slow motion/fast forward)
- 🎚️  Beat sensitivity adjustment (0.0 - 2.0)
- ⭐ Favorites system for organizing presets
- 🗑️  Safe delete (presets moved to deleted folder, recoverable)
- 🖥️  Fullscreen and multi-monitor support

Quick Start Keyboard Shortcuts:
- H: Show complete help menu
- F: Add/remove preset to favorites
- T: Toggle favorites-only mode
- CMD+Delete: Move preset to deleted folder (recoverable)
- UP/DOWN: Adjust time scale (slow motion/fast forward)
- CMD+UP/DOWN: Adjust beat sensitivity
- S: Toggle slow motion (0.1x/1.0x)
- SPACE: Lock/unlock current preset

Preset Management:
Your presets are organized in folders:
- favorites/ - Press F to add/remove presets
- deleted/ - Press CMD+Delete to safely remove presets
- All presets can be recovered from the deleted folder

For complete documentation, see FEATURES.md in the app bundle.

Enjoy the visualizations! 🎵✨

Based on projectM v4.1.4 - https://github.com/projectM-visualizer/projectm
EOF

# Copy FEATURES.md if it exists
if [ -f "FEATURES.md" ]; then
    echo "Copying FEATURES.md..."
    cp FEATURES.md "${DMG_TEMP}/"
fi

# Create DMG
echo "Creating DMG..."
hdiutil create -volname "${APP_NAME}" -srcfolder "${DMG_TEMP}" -ov -format UDZO "${DMG_NAME}"

# Clean up
echo "Cleaning up..."
rm -rf "${DMG_TEMP}"

echo ""
echo "✅ DMG created successfully!"
echo "📦 Location: ${DMG_NAME}"
echo ""
echo "To distribute:"
echo "  The DMG file is ready for distribution"
echo ""
echo "To test installation:"
echo "  open ${DMG_NAME}"
echo ""
