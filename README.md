# projectM 4.1.4 - Modified Build

This is a modified version of [projectM](https://github.com/projectM-visualizer/projectm), the advanced open-source music visualizer.

## About projectM

projectM is an open-source project that reimplements the esteemed [Winamp Milkdrop](https://en.wikipedia.org/wiki/MilkDrop) by Geiss in a more modern, cross-platform reusable library. It transforms music into psychedelic and mesmerizing visuals by detecting tempo and rendering advanced equations into a limitless array of user-contributed visualizations.

**Original Project:** https://github.com/projectM-visualizer/projectm

## Modifications

This fork includes the following changes to improve cross-platform builds and distribution:

### Build System Improvements

- **Windows NSIS Installer**: Automated Windows installer generation with Start Menu and Desktop shortcuts
- **Milkdrop Presets Bundling**: Automatic download and inclusion of official Milkdrop presets in all platform builds
- **macOS Standalone Build**: Self-contained macOS application bundle with SDL2 library included
- **Platform Compatibility**: Added Windows compatibility fixes for POSIX-specific terminal code

### User Experience

- **Audio Setup Documentation**: Comprehensive audio input configuration instructions for macOS, Windows, and Linux
- **Installer Shortcuts**: Automated shortcut creation for Windows installations
- **Preset Integration**: Official presets automatically included in installers

### Technical Changes

- Modified `.github/workflows/build_windows.yml` to add NSIS installer generation
- Modified `.github/workflows/build_osx.yml` to bundle SDL2 library
- Modified `.github/workflows/push_release.yml` to include installers in releases
- Added platform guards in `src/sdl-test-ui/pmSDL.cpp` for Windows compatibility
- Added install target in `src/sdl-test-ui/CMakeLists.txt`
- Enhanced `CMakeLists.txt` with CPack NSIS configuration

## Downloads

Pre-built installers are available in the [Releases](https://github.com/sorenwacker/projectm-4.1.4/releases) section:

- **Windows**: `.exe` NSIS installer with shortcuts
- **macOS**: `.dmg` disk images for Intel and Apple Silicon
- **Linux**: `.tar.gz` archive

## Audio Input Setup

projectM visualizes audio from your microphone or system audio output. Configuration varies by platform:

### macOS

1. Grant microphone permissions when prompted, or enable in **System Settings > Privacy & Security > Microphone**
2. For system audio visualization, install [BlackHole](https://github.com/ExistentialAudio/BlackHole) and create a Multi-Output Device in Audio MIDI Setup

### Windows

1. Install [VB-Audio Cable](https://vb-audio.com/Cable/) for system audio routing
2. Set the virtual cable as your default playback device
3. Enable "Listen to this device" on the virtual cable, outputting to your speakers
4. Select the virtual cable as input in projectM

### Linux

Use PulseAudio, PipeWire, or ALSA to route audio:

**PulseAudio/PipeWire:**
```bash
# List sources
pactl list sources

# Use pavucontrol for graphical audio routing
sudo apt install pavucontrol
```

## Building from Source

Refer to the original projectM documentation:
- [BUILDING.md](BUILDING.md) - General build instructions
- [BUILDING-cmake.md](BUILDING-cmake.md) - CMake-specific details

## License

This software is licensed under the GNU Lesser General Public License v2.1 (LGPL-2.1), the same license as the original projectM project. See [LICENSE.txt](LICENSE.txt) for the full license text.

### License Compliance

This modified version complies with LGPL 2.1 requirements:
- Source code modifications are documented in git history
- Original copyright notices are preserved
- Modified source code is available at this repository
- License file is included and unchanged

## Credits

- **Original projectM Team**: https://github.com/projectM-visualizer/projectm
- **Winamp Milkdrop**: Created by Geiss
- **Modifications**: Build system and platform compatibility improvements

## Community

Join the projectM community:
- [Discord](https://discord.gg/mMrxAqaa3W)
- [Original Repository](https://github.com/projectM-visualizer/projectm)

## Disclaimer

This is an unofficial fork created for improved build automation and cross-platform distribution. For the official projectM project, visit https://github.com/projectM-visualizer/projectm
