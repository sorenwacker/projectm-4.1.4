#!/bin/bash
# Script to create a DMG installer for projectM

set -e  # Exit on error

APP_NAME="projectM"
APP_BUNDLE="${APP_NAME}.app"
DMG_NAME="${APP_NAME}-v4.1.4-macOS.dmg"
DMG_TEMP="${APP_NAME}_dmg_temp"
DMG_VOLUME_NAME="projectM v4.1.4"

echo "Creating DMG installer for ${APP_NAME}..."

# Check if app bundle exists
if [ ! -d "${APP_BUNDLE}" ]; then
    echo "Error: ${APP_BUNDLE} not found. Please run ./create_macos_app.sh first."
    exit 1
fi

# Clean up old DMG and temp folder
rm -rf "${DMG_TEMP}"
rm -f "${DMG_NAME}"
rm -f "${DMG_NAME%.dmg}-temp.dmg"

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

# Create temporary DMG
echo "Creating temporary DMG..."
hdiutil create -volname "${DMG_VOLUME_NAME}" -srcfolder "${DMG_TEMP}" -ov -format UDRW "${DMG_NAME%.dmg}-temp.dmg"

# Mount the DMG
echo "Mounting DMG for customization..."
MOUNT_DIR="/Volumes/${DMG_VOLUME_NAME}"
hdiutil attach "${DMG_NAME%.dmg}-temp.dmg" -mountpoint "${MOUNT_DIR}"

# Wait for mount
sleep 2

# Set custom icon positions and window properties using AppleScript
echo "Setting window layout..."
osascript <<EOF
tell application "Finder"
    tell disk "${DMG_VOLUME_NAME}"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {100, 100, 740, 480}
        set viewOptions to the icon view options of container window
        set arrangement of viewOptions to not arranged
        set icon size of viewOptions to 128
        set background color of viewOptions to {255, 255, 255}

        delay 1

        -- Position app icon on the left
        set position of item "${APP_BUNDLE}" of container window to {160, 180}

        delay 1

        -- Position Applications symlink on the right
        set position of item "Applications" of container window to {480, 180}

        delay 1

        -- Hide documentation files (move them off-screen)
        try
            set position of item "README.txt" of container window to {1000, 1000}
        end try
        try
            set position of item "FEATURES.md" of container window to {1000, 1000}
        end try

        -- Close and reopen to save settings
        close
        delay 1
        open

        -- Update and ensure .DS_Store is written
        update without registering applications
        delay 3

        close
    end tell
end tell
EOF

# Ensure .DS_Store is written
echo "Ensuring layout is saved..."
sleep 3
sync
sync

# Unmount
echo "Unmounting DMG..."
hdiutil detach "${MOUNT_DIR}" || hdiutil detach "${MOUNT_DIR}" -force

# Convert to compressed read-only DMG
echo "Creating final compressed DMG..."
hdiutil convert "${DMG_NAME%.dmg}-temp.dmg" -format UDZO -o "${DMG_NAME}"

# Clean up
echo "Cleaning up..."
rm -rf "${DMG_TEMP}"
rm -f "${DMG_NAME%.dmg}-temp.dmg"

echo ""
echo "✅ DMG created successfully!"
echo "📦 Location: ${DMG_NAME}"
echo ""
echo "Features:"
echo "  - Drag & drop installation interface"
echo "  - App positioned on left, Applications on right"
echo "  - README and documentation included"
echo ""
echo "To distribute:"
echo "  The DMG file is ready for distribution"
echo ""
echo "To test installation:"
echo "  open ${DMG_NAME}"
echo ""
