# projectM v4.1.4 - Custom Features Documentation

This is a customized build of projectM with enhanced features for better preset management and visualization control.

## New Features

### 🖥️ Terminal Command Center

Control projectM entirely from the Terminal with instant single-keypress commands (no ENTER needed).

**How it works:**
- Terminal runs in raw mode for instant single-character input
- All commands work immediately without pressing ENTER
- Real-time feedback with emoji indicators
- Works alongside SDL window controls

**Quick Commands:**
- **Arrow keys** (↑↓←→): Time scale and preset navigation
- **Number keys** (0-9): Preset duration control
- **Letters**: All other controls (N/P for next/prev, F for favorites, etc.)

See "Terminal Commands" section below for complete list.

### ⏱️ Preset Duration Control

Control how long each preset displays before automatically switching to the next.

- **Keys 1-9**: Set fixed duration
  - **1** = 5 seconds (rapid browsing)
  - **2** = 10 seconds (fast)
  - **3** = 15 seconds
  - **4** = 20 seconds
  - **5** = 30 seconds (medium, good default)
  - **6** = 45 seconds
  - **7** = 60 seconds (1 minute)
  - **8** = 90 seconds (1.5 minutes)
  - **9** = 120 seconds (2 minutes, enjoy mode)

- **Key 0**: Toggle random duration mode
  - When ON: Each preset gets a random duration (5-120 seconds)
  - When OFF: Uses last set fixed duration
  - Creates unpredictable, natural flow

**Use Cases:**
- Quick browsing: Press `1` for 5-second rapid flipping
- Finding favorites: Use `1` to browse fast, `F` to favorite, `L` to lock
- Party mode: Press `0` for random unpredictable timing
- Meditation/focus: Press `9` for long 2-minute displays

**Default**: 10 seconds (configurable in config.inp)

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

## Terminal Commands

All commands work instantly in the Terminal (no ENTER needed). Press **H** or **?** in Terminal for help.

### Preset Navigation
- **→** or **N**: Next preset
- **←** or **P**: Previous preset
- **R**: Random preset
- **L**: Lock/unlock current preset
- **Y**: Toggle shuffle

### Preset Duration
- **1**: 5 seconds (rapid browsing)
- **2**: 10 seconds (fast)
- **3**: 15 seconds
- **4**: 20 seconds
- **5**: 30 seconds (medium)
- **6**: 45 seconds
- **7**: 60 seconds (1 minute)
- **8**: 90 seconds (1.5 minutes)
- **9**: 120 seconds (2 minutes)
- **0**: Toggle random mode (5-120s per preset)

### Favorites & Organization
- **F**: Add/remove preset to favorites
- **T**: Toggle favorites-only mode
- **D**: Move preset to deleted folder

### Time & Audio Control
- **↑** or **+**: Increase time scale
- **↓** or **-**: Decrease time scale
- **S**: Toggle slow motion (0.1x/1.0x)
- **]**: Increase beat sensitivity
- **[**: Decrease beat sensitivity

### Display
- **M**: Toggle fullscreen
- **A**: Toggle aspect correction

### Other
- **I**: Show current status
- **H** or **?**: Show help
- **Q**: Quit projectM

## Complete Keyboard Shortcuts (SDL Window)

### Presets
- **LEFT/RIGHT Arrow**: Navigate presets
- **R**: Random preset
- **SPACE**: Lock/unlock preset
- **Y**: Toggle shuffle
- **F**: Move preset to/from favorites
- **T**: Toggle favorites-only mode
- **CMD+Delete**: Move preset to deleted folder
- **Mouse Scroll**: Change presets
- **1-9**: Set preset duration (5s to 120s)
- **0**: Toggle random duration mode

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

### Terminal Command Center Implementation
- Uses termios for raw terminal mode (character-by-character input)
- Non-blocking read with VMIN=0, VTIME=0
- Escape sequence detection for arrow keys (ESC + '[' + direction)
- Terminal mode automatically restored on exit
- Works alongside SDL window input (dual control)

### Preset Duration Control
- Uses projectM API: `projectm_set_preset_duration()` and `projectm_get_preset_duration()`
- Random mode: callback on `presetSwitchedEvent` generates new duration (5-120s)
- Pressing any fixed duration key (1-9) automatically disables random mode
- Default duration: 10 seconds (from config.inp)

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
- Path tracking: `_presetName` updated after file moves to prevent errors

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
- **Custom Features**:
  - Terminal command center with instant single-keypress control
  - Preset duration control (1-9 keys + random mode)
  - Time scale control (0.01x-2.0x)
  - Beat sensitivity control (0.0-2.0)
  - Favorites system with toggle mode
  - Safe delete with recovery
- **Build Date**: 2025-10-18
- **Platform**: macOS (cross-platform compatible)

## Credits

Custom features implemented with Claude Code assistance.
Based on projectM v4.1.4 by the projectM Team.

---

**For Help**: Press **H** in the app to see all keyboard shortcuts.

**For Issues**: Check the console output for error messages and debugging info.
