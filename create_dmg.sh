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
projectM v4.1.4 - Music Visualizer
===================================

Installation:
1. Drag projectM.app to the Applications folder
2. Open projectM from Applications
3. Grant microphone access when prompted (for audio visualization)

Features:
- Time scale control (slow motion/fast forward)
- Beat sensitivity adjustment
- Favorites system for presets
- Fullscreen mode
- Multi-monitor support

Keyboard Shortcuts:
- H: Show help menu
- F: Add/remove preset to favorites
- T: Toggle favorites-only mode
- SPACE: Lock/unlock preset
- UP/DOWN: Adjust time scale
- CMD+UP/DOWN: Adjust beat sensitivity

For more info: https://github.com/projectM-visualizer/projectm

Enjoy the visualizations! 🎵✨
EOF

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
