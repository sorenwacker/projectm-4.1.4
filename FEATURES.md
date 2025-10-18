# projectM v4.1.4 - Custom Features Documentation

This is a customized build of projectM with enhanced features for better preset management and visualization control.

## New Features

### 🎵 Time Scale Control (Slow Motion / Fast Forward)

Control the animation speed independently from preset switching timing.

- **UP/DOWN Arrow**: Adjust time scale (0.01x - 2.0x)
  - Adaptive increments: 0.01x steps below 0.1x, then 0.1x steps
  - Example: 0.01x → 0.02x → ... → 0.09x → 0.1x → 0.2x → ... → 2.0x
- **S**: Toggle slow motion (switches between 0.1x and 1.0x)

**Note**: Time scale affects animation speed but NOT preset switching timing. This means:
- Slow motion (0.1x) = animations move 10x slower
- Fast forward (2.0x) = animations move 2x faster
- Presets still switch at their normal duration regardless of time scale

### 🎚️ Beat Sensitivity

Adjust how strongly the visualizer responds to audio beats.

- **CMD+UP/DOWN**: Adjust beat sensitivity (0.0 - 2.0)
  - 0.0 = No response to beats
  - 1.0 = Normal response (default)
  - 2.0 = Maximum response to beats

### ⭐ Favorites System

Organize your favorite presets in a dedicated folder.

- **F**: Move current preset to/from favorites
  - If preset is in main folder → moves to favorites
  - If preset is in favorites → moves back to main folder
- **T**: Toggle favorites-only mode
  - Shows only presets in the favorites folder
  - Window title shows `[favorites]` when active
  - Press T again to show all presets

**Folder Structure**:
```
presets/
├── favorites/      # Your favorite presets
└── [all other presets and subfolders]
```

### 🗑️ Safe Delete System

Delete presets safely with ability to recover them.

- **CMD+Delete** (or **CMD+Backspace**): Move current preset to deleted folder
  - Presets are moved to `presets/deleted/` folder
  - Not permanently deleted - can be recovered manually
  - Playlist automatically reloads after deletion

**Recovery**: Navigate to `presets/deleted/` folder and manually move presets back to recover them.

### 📁 Preset Folder Management

Three special folders are automatically created:
```
presets/
├── favorites/      # Press F to add/remove presets
├── deleted/        # Press CMD+Delete to move presets here
└── [all other presets]
```

All folders are created on startup if they don't exist.

## Complete Keyboard Shortcuts

### Presets
- **LEFT/RIGHT Arrow**: Navigate presets
- **R**: Random preset
- **SPACE**: Lock/unlock preset
- **Y**: Toggle shuffle
- **F**: Move preset to/from favorites
- **T**: Toggle favorites-only mode
- **CMD+Delete**: Move preset to deleted folder
- **Mouse Scroll**: Change presets

### Audio
- **CMD+I**: Cycle audio input devices
- **CMD+UP/DOWN**: Adjust beat sensitivity (0.0 - 2.0)

### Display
- **CMD+F**: Toggle fullscreen
- **CMD+M**: Change monitor
- **CMD+S**: Stretch across monitors
- **A**: Toggle aspect correction

### Time Control
- **UP/DOWN Arrow**: Adjust time scale (0.01x - 2.0x, adaptive increments)
- **S**: Toggle slow motion (0.1x/1.0x)

### Other
- **H**: Show keyboard shortcuts help
- **CMD+Q**: Quit

## Building and Packaging

### Build from Source
```bash
# Build the project
cmake --build build -j8

# Run directly
./build/src/sdl-test-ui/projectM-Test-UI
```

### Create macOS App Bundle
```bash
# Create .app bundle for testing
./create_macos_app.sh

# Test the app
open projectM.app
```

### Create DMG Installer
```bash
# Create DMG for distribution
./create_dmg.sh

# Result: projectM-v4.1.4-macOS.dmg
```

## Technical Details

### Time Scale Implementation
- Dual time tracking: scaled time (for animations) and real time (for preset switching)
- Time scale affects `m_currentTime` but not `m_realTime`
- Preset progress and switching use `m_realTime` exclusively
- Allows independent control of animation speed and preset timing

### Beat Sensitivity Implementation
- Multiplier applied to all audio data fields before passing to presets
- Affects: bass, mid, treb, vol (and their attenuated versions)
- Range: 0.0 (no response) to 2.0 (maximum response)

### Favorites System
- Uses move operations (not copy) to prevent duplicates
- Playlist automatically reloads when presets are moved
- Window title shows current mode: `[favorites]` or `[locked]` or both

### Delete System
- Soft delete: moves presets to `deleted/` folder
- Allows recovery if deleted by accident
- Cross-platform: uses `mv` on Unix, `move` on Windows

## Pre-populating Favorites for Distribution

When preparing a distribution package:

1. Run projectM and navigate to presets you want as defaults
2. Press **F** to move them to the favorites folder
3. The `presets/favorites/` folder now contains your curated favorites
4. Package the entire `presets/` folder (including `favorites/` subfolder)
5. Ship the .app bundle or DMG with pre-populated favorites

Users will have access to:
- Your curated favorites (can add/remove with F key)
- Empty deleted folder (automatically created on first run)
- All other presets in the main collection

## Cross-Platform Support

All features work on:
- **macOS**: Full support with setup scripts
- **Linux**: Full support (build from source)
- **Windows**: Full support (build from source)

## Performance Notes

- Time scale does not affect FPS or render quality
- Preset switching timing is independent of time scale
- Favorites mode improves performance (fewer presets to shuffle through)
- Beat sensitivity has negligible performance impact

## Version Information

- **Base Version**: projectM v4.1.4
- **Custom Features**: Time scale, beat sensitivity, favorites, safe delete
- **Build Date**: 2025-10-18
- **Platform**: macOS (cross-platform compatible)

## Credits

Custom features implemented with Claude Code assistance.
Based on projectM v4.1.4 by the projectM Team.

---

**For Help**: Press **H** in the app to see all keyboard shortcuts.

**For Issues**: Check the console output for error messages and debugging info.
