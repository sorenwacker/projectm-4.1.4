/**
* projectM -- Milkdrop-esque visualisation SDK
* Copyright (C)2003-2019 projectM Team
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
* See 'LICENSE.txt' included within this release
*
* projectM-sdl
* This is an implementation of projectM using libSDL2
*
* pmSDL.cpp
* Authors: Created by Mischa Spiegelmock on 2017-09-18.
*
*
* experimental Stereoscopic SBS driver functionality by
*	RobertPancoast77@gmail.com
*/

#include "pmSDL.hpp"

#include <vector>
#ifndef _WIN32
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
static struct termios orig_termios;
#endif
#include <fstream>

projectMSDL::projectMSDL(SDL_GLContext glCtx, const std::string& presetPath)
    : _openGlContext(glCtx)
    , _projectM(projectm_create())
    , _playlist(projectm_playlist_create(_projectM))
    , _presetsBasePath(presetPath)
    , _favoritesPath(presetPath + "/favorites")
    , _deletedPath(presetPath + "/deleted")
{
    projectm_get_window_size(_projectM, &_width, &_height);
    projectm_playlist_set_preset_switched_event_callback(_playlist, &projectMSDL::presetSwitchedEvent, static_cast<void*>(this));

    // Create favorites directory if it doesn't exist
#if defined _MSC_VER
    _mkdir(_favoritesPath.c_str());
    _mkdir(_deletedPath.c_str());
#else
    mkdir(_favoritesPath.c_str(), 0755);
    mkdir(_deletedPath.c_str(), 0755);
#endif

    // Load presets based on current mode
    reloadPlaylist();
    projectm_playlist_set_shuffle(_playlist, _shuffle);
}

projectMSDL::~projectMSDL()
{
    projectm_playlist_destroy(_playlist);
    _playlist = nullptr;
    projectm_destroy(_projectM);
    _projectM = nullptr;
}

/* Stretch projectM across multiple monitors */
void projectMSDL::stretchMonitors()
{
    int displayCount = SDL_GetNumVideoDisplays();
    if (displayCount >= 2)
    {
        std::vector<SDL_Rect> displayBounds;
        for (int i = 0; i < displayCount; i++)
        {
            displayBounds.push_back(SDL_Rect());
            SDL_GetDisplayBounds(i, &displayBounds.back());
        }

        int mostXLeft = 0;
        int mostXRight = 0;
        int mostYUp = 0;
        int mostYDown = 0;

        for (int i = 0; i < displayCount; i++)
        {
            if (displayBounds[i].x < mostXLeft)
            {
                mostXLeft = displayBounds[i].x;
            }
            if ((displayBounds[i].x + displayBounds[i].w) > mostXRight)
            {
                mostXRight = displayBounds[i].x + displayBounds[i].w;
            }
        }
        for (int i = 0; i < displayCount; i++)
        {
            if (displayBounds[i].y < mostYUp)
            {
                mostYUp = displayBounds[i].y;
            }
            if ((displayBounds[i].y + displayBounds[i].h) > mostYDown)
            {
                mostYDown = displayBounds[i].y + displayBounds[i].h;
            }
        }

        int mostWide = abs(mostXLeft) + abs(mostXRight);
        int mostHigh = abs(mostYUp) + abs(mostYDown);

        SDL_SetWindowPosition(_sdlWindow, mostXLeft, mostYUp);
        SDL_SetWindowSize(_sdlWindow, mostWide, mostHigh);
    }
}

/* Moves projectM to the next monitor */
void projectMSDL::nextMonitor()
{
    int displayCount = SDL_GetNumVideoDisplays();
    int currentWindowIndex = SDL_GetWindowDisplayIndex(_sdlWindow);
    if (displayCount >= 2)
    {
        std::vector<SDL_Rect> displayBounds;
        int nextWindow = currentWindowIndex + 1;
        if (nextWindow >= displayCount)
        {
            nextWindow = 0;
        }

        for (int i = 0; i < displayCount; i++)
        {
            displayBounds.push_back(SDL_Rect());
            SDL_GetDisplayBounds(i, &displayBounds.back());
        }
        SDL_SetWindowPosition(_sdlWindow, displayBounds[nextWindow].x, displayBounds[nextWindow].y);
        SDL_SetWindowSize(_sdlWindow, displayBounds[nextWindow].w, displayBounds[nextWindow].h);
    }
}

void projectMSDL::toggleFullScreen()
{
    if (_isFullScreen)
    {
        SDL_SetWindowFullscreen(_sdlWindow, 0);
        _isFullScreen = false;
        SDL_ShowCursor(true);
    }
    else
    {
        SDL_ShowCursor(false);
        SDL_SetWindowFullscreen(_sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        _isFullScreen = true;
    }
}

void projectMSDL::printKeyboardShortcuts()
{
    printf("\n========================================\n");
    printf("projectM Keyboard Shortcuts\n");
    printf("========================================\n");
    printf("\nPresets:\n");
    printf("  LEFT/RIGHT Arrow  - Navigate presets\n");
    printf("  R                 - Random preset\n");
    printf("  SPACE             - Lock/unlock preset\n");
    printf("  Y                 - Toggle shuffle\n");
    printf("  F                 - Move preset to/from favorites\n");
    printf("  T                 - Toggle favorites-only mode\n");
    printf("  CMD+Delete        - Move preset to deleted folder\n");
    printf("  Mouse Scroll      - Change presets\n");
    printf("\nPreset Duration:\n");
    printf("  1-9               - Set duration (1=5s, 2=10s ... 9=120s)\n");
    printf("  0                 - Toggle random mode (5-120s per preset)\n");
    printf("\nAudio:\n");
    printf("  CMD+I             - Cycle audio input devices\n");
    printf("  CMD+UP/DOWN       - Adjust beat sensitivity\n");
    printf("\nDisplay:\n");
    printf("  CMD+F             - Toggle fullscreen\n");
    printf("  CMD+M             - Change monitor\n");
    printf("  CMD+S             - Stretch across monitors\n");
    printf("  A                 - Toggle aspect correction\n");
    printf("\nTime Control:\n");
    printf("  UP/DOWN Arrow     - Adjust time scale (0.01x - 2.0x, adaptive increments)\n");
    printf("  S                 - Toggle slow motion (0.1x/1.0x)\n");
    printf("\nOther:\n");
    printf("  H                 - Show this help\n");
    printf("  CMD+Q             - Quit\n");
    printf("\n* Type 'help' in terminal for command-line control\n");
    printf("========================================\n\n");
}

void projectMSDL::setTerminalMode()
{
#ifndef _WIN32
    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);

    // Set terminal to raw mode (character-by-character input)
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    raw.c_cc[VMIN] = 0;  // Non-blocking read
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
#endif
}

void projectMSDL::restoreTerminalMode()
{
#ifndef _WIN32
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
#endif
}

void projectMSDL::printTerminalHelp()
{
    printf("\n========================================\n");
    printf("Terminal Command Center\n");
    printf("========================================\n");
    printf("\nSingle-key commands (no ENTER needed):\n\n");
    printf("Preset Navigation:\n");
    printf("  → or N           - Next preset\n");
    printf("  ← or P           - Previous preset\n");
    printf("  R                - Random preset\n");
    printf("  L                - Lock/unlock current preset\n");
    printf("  Y                - Toggle shuffle\n");
    printf("\nPreset Duration (seconds per preset):\n");
    printf("  1                - 5s (rapid browsing)\n");
    printf("  2                - 10s (fast)\n");
    printf("  3                - 15s\n");
    printf("  4                - 20s\n");
    printf("  5                - 30s (medium)\n");
    printf("  6                - 45s\n");
    printf("  7                - 60s (1 min)\n");
    printf("  8                - 90s (1.5 min)\n");
    printf("  9                - 120s (2 min, enjoy)\n");
    printf("  0                - Toggle random mode (5-120s per preset)\n");
    printf("\nFavorites & Organization:\n");
    printf("  F                - Add/remove preset to favorites\n");
    printf("  T                - Toggle favorites-only mode\n");
    printf("  D                - Move preset to deleted folder\n");
    printf("\nTime & Audio Control:\n");
    printf("  ↑ or +           - Increase time scale\n");
    printf("  ↓ or -           - Decrease time scale\n");
    printf("  S                - Toggle slow motion (0.1x/1.0x)\n");
    printf("  ]                - Increase beat sensitivity\n");
    printf("  [                - Decrease beat sensitivity\n");
    printf("\nDisplay:\n");
    printf("  M                - Toggle fullscreen\n");
    printf("  A                - Toggle aspect correction\n");
    printf("\nOther:\n");
    printf("  I                - Show current status\n");
    printf("  H or ?           - Show this help\n");
    printf("  Q                - Quit projectM\n");
    printf("\n========================================\n");
    printf("\nPress any key to control projectM...\n\n");
    fflush(stdout);
}

void projectMSDL::processTerminalCommand()
{
#ifndef _WIN32
    // Read a single character (non-blocking)
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0) return; // No input available

    // Check for escape sequences (arrow keys, etc.)
    if (c == 27) // ESC character
    {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return;

        if (seq[0] == '[')
        {
            switch (seq[1])
            {
                case 'A': // Up arrow
                    {
                        float currentScale = projectm_get_time_scale(_projectM);
                        float increment = (currentScale <= 0.09f) ? 0.01f : 0.1f;
                        float newScale = std::min(2.0f, currentScale + increment);
                        projectm_set_time_scale(_projectM, newScale);
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[↑] ⏩ Time Scale: %.2fx", newScale);
                    }
                    return;
                case 'B': // Down arrow
                    {
                        float currentScale = projectm_get_time_scale(_projectM);
                        float increment = (currentScale <= 0.1f) ? 0.01f : 0.1f;
                        float newScale = std::max(0.01f, currentScale - increment);
                        projectm_set_time_scale(_projectM, newScale);
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[↓] ⏪ Time Scale: %.2fx", newScale);
                    }
                    return;
                case 'C': // Right arrow
                    projectm_playlist_play_next(_playlist, true);
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[→] Next preset");
                    return;
                case 'D': // Left arrow
                    projectm_playlist_play_previous(_playlist, true);
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[←] Previous preset");
                    return;
            }
        }
        return;
    }

    // Convert to uppercase for consistency
    c = toupper(c);

    // Process single-key commands
    switch (c)
    {
        case 'H':
        case '?':
            printTerminalHelp();
            break;

        case 'N':
            projectm_playlist_play_next(_playlist, true);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[N] → Next preset");
            break;

        case 'P':
            projectm_playlist_play_previous(_playlist, true);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[P] ← Previous preset");
            break;

        case 'R':
            projectm_playlist_set_shuffle(_playlist, true);
            projectm_playlist_play_next(_playlist, true);
            projectm_playlist_set_shuffle(_playlist, _shuffle);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[R] 🎲 Random preset");
            break;

        case 'L':
            {
                bool newValue = !projectm_get_preset_locked(_projectM);
                projectm_set_preset_locked(_projectM, newValue);
                UpdateWindowTitle();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[L] 🔒 Preset Lock: %s", newValue ? "LOCKED" : "UNLOCKED");
            }
            break;

        case 'Y':
            _shuffle = !_shuffle;
            projectm_playlist_set_shuffle(_playlist, _shuffle);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[Y] 🔀 Shuffle: %s", _shuffle ? "ON" : "OFF");
            break;

        case 'F':
            if (_presetName.find("/favorites/") != std::string::npos)
                movePresetFromFavorites();
            else
                movePresetToFavorites();
            break;

        case 'T':
            toggleFavoritesMode();
            break;

        case 'D':
            movePresetToDeleted();
            break;

        case '+':
        case '=':
            {
                float currentScale = projectm_get_time_scale(_projectM);
                float increment = (currentScale <= 0.09f) ? 0.01f : 0.1f;
                float newScale = std::min(2.0f, currentScale + increment);
                projectm_set_time_scale(_projectM, newScale);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[+] ⏩ Time Scale: %.2fx", newScale);
            }
            break;

        case '-':
        case '_':
            {
                float currentScale = projectm_get_time_scale(_projectM);
                float increment = (currentScale <= 0.1f) ? 0.01f : 0.1f;
                float newScale = std::max(0.01f, currentScale - increment);
                projectm_set_time_scale(_projectM, newScale);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[-] ⏪ Time Scale: %.2fx", newScale);
            }
            break;

        case 'S':
            {
                float currentScale = projectm_get_time_scale(_projectM);
                float newScale = (currentScale == 1.0f) ? 0.1f : 1.0f;
                projectm_set_time_scale(_projectM, newScale);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[S] 🐌 Slow Motion: %s (%.1fx)",
                            newScale < 1.0f ? "ON" : "OFF", newScale);
            }
            break;

        case ']':
            {
                float currentSensitivity = projectm_get_beat_sensitivity(_projectM);
                float newSensitivity = std::min(2.0f, currentSensitivity + 0.1f);
                projectm_set_beat_sensitivity(_projectM, newSensitivity);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[]] 🎵 Beat Sensitivity: %.1f", newSensitivity);
            }
            break;

        case '[':
            {
                float currentSensitivity = projectm_get_beat_sensitivity(_projectM);
                float newSensitivity = std::max(0.0f, currentSensitivity - 0.1f);
                projectm_set_beat_sensitivity(_projectM, newSensitivity);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[[] 🎵 Beat Sensitivity: %.1f", newSensitivity);
            }
            break;

        case 'M':
            toggleFullScreen();
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[M] 🖥️  Fullscreen: %s", _isFullScreen ? "ON" : "OFF");
            break;

        case 'A':
            {
                bool newValue = !projectm_get_aspect_correction(_projectM);
                projectm_set_aspect_correction(_projectM, newValue);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[A] 📐 Aspect Correction: %s", newValue ? "ON" : "OFF");
            }
            break;

        case 'I':
            printf("\n========================================\n");
            printf("Current Status\n");
            printf("========================================\n");
            printf("Preset: %s\n", _presetName.c_str());
            printf("Locked: %s\n", projectm_get_preset_locked(_projectM) ? "YES" : "NO");
            printf("Favorites Mode: %s\n", _favoritesOnlyMode ? "ON" : "OFF");
            printf("Shuffle: %s\n", _shuffle ? "ON" : "OFF");
            if (_randomDurationMode)
            {
                printf("Preset Duration: RANDOM (5-120s each preset)\n");
            }
            else
            {
                printf("Preset Duration: %.0f seconds\n", projectm_get_preset_duration(_projectM));
            }
            printf("Time Scale: %.2fx\n", projectm_get_time_scale(_projectM));
            printf("Beat Sensitivity: %.1f\n", projectm_get_beat_sensitivity(_projectM));
            printf("Fullscreen: %s\n", _isFullScreen ? "YES" : "NO");
            printf("Playlist Size: %d presets\n", projectm_playlist_size(_playlist));
            printf("========================================\n\n");
            fflush(stdout);
            break;

        case 'Q':
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[Q] 👋 Quit requested from terminal");
            done = true;
            break;

        case '1':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 5.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[1] ⏱️  Preset Duration: 5 seconds (rapid)");
            break;
        case '2':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 10.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[2] ⏱️  Preset Duration: 10 seconds (fast)");
            break;
        case '3':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 15.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[3] ⏱️  Preset Duration: 15 seconds");
            break;
        case '4':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 20.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[4] ⏱️  Preset Duration: 20 seconds");
            break;
        case '5':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 30.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[5] ⏱️  Preset Duration: 30 seconds (medium)");
            break;
        case '6':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 45.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[6] ⏱️  Preset Duration: 45 seconds");
            break;
        case '7':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 60.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[7] ⏱️  Preset Duration: 60 seconds (1 min)");
            break;
        case '8':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 90.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[8] ⏱️  Preset Duration: 90 seconds (1.5 min)");
            break;
        case '9':
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 120.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[9] ⏱️  Preset Duration: 120 seconds (2 min)");
            break;
        case '0':
            {
                // Toggle random duration mode
                _randomDurationMode = !_randomDurationMode;
                if (_randomDurationMode)
                {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[0] 🎲 Random Duration Mode: ON (each preset gets random duration 5-120s)");
                }
                else
                {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[0] ⏱️  Random Duration Mode: OFF");
                }
            }
            break;

        default:
            // Ignore unknown keys silently
            break;
    }
#endif
}

void projectMSDL::reloadPlaylist()
{
    // Clear current playlist
    projectm_playlist_clear(_playlist);

    // Load presets based on mode
    const char* pathToLoad = _favoritesOnlyMode ? _favoritesPath.c_str() : _presetsBasePath.c_str();
    projectm_playlist_add_path(_playlist, pathToLoad, true, false);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loaded %d presets from %s",
                projectm_playlist_size(_playlist),
                _favoritesOnlyMode ? "favorites" : "all presets");
}

void projectMSDL::toggleFavoritesMode()
{
    _favoritesOnlyMode = !_favoritesOnlyMode;
    reloadPlaylist();

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Favorites-Only Mode: %s", _favoritesOnlyMode ? "ON" : "OFF");
    UpdateWindowTitle();
}

void projectMSDL::movePresetToFavorites()
{
    if (_presetName.empty())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No preset currently loaded");
        return;
    }

    // Check if preset is already in favorites
    if (_presetName.find("/favorites/") != std::string::npos)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "⭐ Already in favorites");
        return;
    }

    // Get just the filename from the full path
    size_t lastSlash = _presetName.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? _presetName.substr(lastSlash + 1) : _presetName;

    // Build destination path
    std::string destPath = _favoritesPath + "/" + filename;

    // Check if source file exists
    std::ifstream srcFile(_presetName);
    if (!srcFile.good())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Preset file not found (may have been moved): %s", filename.c_str());
        return;
    }
    srcFile.close();

    // Move file using system command (cross-platform)
#ifdef _WIN32
    std::string command = "move \"" + _presetName + "\" \"" + destPath + "\"";
#else
    std::string command = "mv \"" + _presetName + "\" \"" + destPath + "\"";
#endif

    int result = system(command.c_str());

    if (result == 0)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "⭐ Added to favorites: %s", filename.c_str());
        // Update the preset name to reflect new location
        _presetName = destPath;
        // Reload playlist to reflect changes
        reloadPlaylist();
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to move preset to favorites (file may not exist)");
    }
}

void projectMSDL::movePresetFromFavorites()
{
    if (_presetName.empty())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No preset currently loaded");
        return;
    }

    // Check if preset is in favorites
    if (_presetName.find("/favorites/") == std::string::npos)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Not in favorites");
        return;
    }

    // Get just the filename from the full path
    size_t lastSlash = _presetName.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? _presetName.substr(lastSlash + 1) : _presetName;

    // Build destination path (back to main presets folder)
    std::string destPath = _presetsBasePath + "/" + filename;

    // Move file using system command (cross-platform)
#ifdef _WIN32
    std::string command = "move \"" + _presetName + "\" \"" + destPath + "\"";
#else
    std::string command = "mv \"" + _presetName + "\" \"" + destPath + "\"";
#endif

    int result = system(command.c_str());

    if (result == 0)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "☆ Removed from favorites: %s", filename.c_str());
        // Update the preset name to reflect new location
        _presetName = destPath;
        // Reload playlist to reflect changes
        reloadPlaylist();
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to move preset from favorites");
    }
}

void projectMSDL::movePresetToDeleted()
{
    if (_presetName.empty())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No preset currently loaded");
        return;
    }

    // Don't delete if already in deleted folder
    if (_presetName.find("/deleted/") != std::string::npos)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Preset is already in deleted folder");
        return;
    }

    // Get just the filename from the full path
    size_t lastSlash = _presetName.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? _presetName.substr(lastSlash + 1) : _presetName;

    // Build destination path
    std::string destPath = _deletedPath + "/" + filename;

    // Move file using system command (cross-platform)
#ifdef _WIN32
    std::string command = "move \"" + _presetName + "\" \"" + destPath + "\"";
#else
    std::string command = "mv \"" + _presetName + "\" \"" + destPath + "\"";
#endif

    int result = system(command.c_str());

    if (result == 0)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "🗑️  Moved to deleted: %s", filename.c_str());
        // Update the preset name to reflect new location
        _presetName = destPath;
        // Reload playlist to reflect changes
        reloadPlaylist();
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to move preset to deleted");
    }
}

void projectMSDL::copyPresetToFavorites()
{
    // Kept for backward compatibility, but now just calls movePresetToFavorites
    movePresetToFavorites();
}

void projectMSDL::scrollHandler(SDL_Event* sdl_evt)
{
    // handle mouse scroll wheel - up++
    if (sdl_evt->wheel.y > 0)
    {
        projectm_playlist_play_previous(_playlist, true);
    }
    // handle mouse scroll wheel - down--
    if (sdl_evt->wheel.y < 0)
    {
        projectm_playlist_play_next(_playlist, true);
    }
}

void projectMSDL::keyHandler(SDL_Event* sdl_evt)
{
    SDL_Keymod sdl_mod = (SDL_Keymod) sdl_evt->key.keysym.mod;
    SDL_Keycode sdl_keycode = sdl_evt->key.keysym.sym;

    // Left or Right Gui or Left Ctrl
    if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
    {
        keymod = true;
    }

    // handle keyboard input (for our app first, then projectM)
    switch (sdl_keycode)
    {
        case SDLK_h:
            printKeyboardShortcuts();
            break;

        case SDLK_a:
            {
                bool newValue = !projectm_get_aspect_correction(_projectM);
                projectm_set_aspect_correction(_projectM, newValue);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Aspect Correction: %s", newValue ? "ON" : "OFF");
            }
            break;

        case SDLK_q:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // cmd/ctrl-q = quit
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit requested");
                done = 1;
                return;
            }
            break;

        case SDLK_i:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                toggleAudioInput();
                return; // handled
            }
            break;

        case SDLK_s:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // command-s: [s]tretch monitors
                // Stereo requires fullscreen
#if !STEREOSCOPIC_SBS
                if (!this->stretch)
                { // if stretching is not already enabled, enable it.
                    stretchMonitors();
                    this->stretch = true;
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Stretch monitors: ON");
                }
                else
                {
                    toggleFullScreen(); // else, just toggle full screen so we leave stretch mode.
                    this->stretch = false;
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Stretch monitors: OFF");
                }
#endif
                return; // handled
            }
            else
            {
                // s without modifier: slow motion toggle
                float currentScale = projectm_get_time_scale(_projectM);
                float newScale = (currentScale == 1.0f) ? 0.1f : 1.0f;
                projectm_set_time_scale(_projectM, newScale);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Slow Motion: %s (time scale: %.1fx)",
                            newScale < 1.0f ? "ON" : "OFF", newScale);
            }
            break;

        case SDLK_m:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // command-m: change [m]onitor
                // Stereo requires fullscreen
#if !STEREOSCOPIC_SBS
                nextMonitor();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Switched to next monitor");
#endif
                this->stretch = false; // if we are switching monitors, ensure we disable monitor stretching.
                return;                // handled
            }

        case SDLK_f:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // command-f: fullscreen
                // Stereo requires fullscreen
#if !STEREOSCOPIC_SBS
                toggleFullScreen();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fullscreen: %s", _isFullScreen ? "ON" : "OFF");
#endif
                this->stretch = false; // if we are toggling fullscreen, ensure we disable monitor stretching.
                return;                // handled
            }
            else
            {
                // f without modifier: move preset to/from favorites
                if (_presetName.find("/favorites/") != std::string::npos)
                {
                    movePresetFromFavorites();
                }
                else
                {
                    movePresetToFavorites();
                }
            }
            break;

        case SDLK_t:
            // t: toggle favorites-only mode
            toggleFavoritesMode();
            break;

        case SDLK_r:
            // Use playlist shuffle to randomize.
            projectm_playlist_set_shuffle(_playlist, true);
            projectm_playlist_play_next(_playlist, true);
            projectm_playlist_set_shuffle(_playlist, _shuffle);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Random preset selected");
            break;

        case SDLK_y:
            _shuffle = !_shuffle;
            projectm_playlist_set_shuffle(_playlist, _shuffle);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shuffle: %s", _shuffle ? "ON" : "OFF");
            break;

        case SDLK_LEFT:
            projectm_playlist_play_previous(_playlist, true);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Previous preset");
            break;

        case SDLK_RIGHT:
            projectm_playlist_play_next(_playlist, true);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Next preset");
            break;

        case SDLK_UP:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // CMD+UP: Increase beat sensitivity (max 2.0)
                float currentSensitivity = projectm_get_beat_sensitivity(_projectM);
                float newSensitivity = std::min(2.0f, currentSensitivity + 0.1f);
                projectm_set_beat_sensitivity(_projectM, newSensitivity);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Beat Sensitivity: %.1f", newSensitivity);
            }
            else
            {
                // UP: Increase time scale (speed up)
                float currentScale = projectm_get_time_scale(_projectM);
                // Use 0.01 increments up to 0.09, then switch to 0.1 increments
                float increment = (currentScale <= 0.09f) ? 0.01f : 0.1f;
                float newScale = std::min(2.0f, currentScale + increment);
                projectm_set_time_scale(_projectM, newScale);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Time Scale: %.2fx", newScale);
            }
            break;

        case SDLK_DOWN:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // CMD+DOWN: Decrease beat sensitivity (min 0.0)
                float currentSensitivity = projectm_get_beat_sensitivity(_projectM);
                float newSensitivity = std::max(0.0f, currentSensitivity - 0.1f);
                projectm_set_beat_sensitivity(_projectM, newSensitivity);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Beat Sensitivity: %.1f", newSensitivity);
            }
            else
            {
                // DOWN: Decrease time scale (slow down)
                float currentScale = projectm_get_time_scale(_projectM);
                // Use 0.01 increments below 0.1, otherwise 0.1 increments
                float increment = (currentScale <= 0.1f) ? 0.01f : 0.1f;
                float newScale = std::max(0.01f, currentScale - increment);
                projectm_set_time_scale(_projectM, newScale);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Time Scale: %.2fx", newScale);
            }
            break;

        case SDLK_SPACE:
            {
                bool newValue = !projectm_get_preset_locked(_projectM);
                projectm_set_preset_locked(_projectM, newValue);
                UpdateWindowTitle();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Lock: %s", newValue ? "LOCKED" : "UNLOCKED");
            }
            break;

        case SDLK_DELETE:
        case SDLK_BACKSPACE:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // CMD+Delete: move preset to deleted folder
                movePresetToDeleted();
            }
            break;

        // Preset duration controls (number keys 0-9)
        case SDLK_1:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 5.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 5 seconds (rapid)");
            break;
        case SDLK_2:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 10.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 10 seconds (fast)");
            break;
        case SDLK_3:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 15.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 15 seconds");
            break;
        case SDLK_4:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 20.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 20 seconds");
            break;
        case SDLK_5:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 30.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 30 seconds (medium)");
            break;
        case SDLK_6:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 45.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 45 seconds");
            break;
        case SDLK_7:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 60.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 60 seconds (1 min)");
            break;
        case SDLK_8:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 90.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 90 seconds (1.5 min)");
            break;
        case SDLK_9:
            _randomDurationMode = false;
            projectm_set_preset_duration(_projectM, 120.0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset Duration: 120 seconds (2 min)");
            break;
        case SDLK_0:
            {
                // Toggle random duration mode
                _randomDurationMode = !_randomDurationMode;
                if (_randomDurationMode)
                {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Random Duration Mode: ON (each preset gets random 5-120s)");
                }
                else
                {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Random Duration Mode: OFF");
                }
            }
            break;

    }
}

void projectMSDL::addFakePCM()
{
    int i;
    int16_t pcm_data[2 * 512];
    /** Produce some fake PCM data to stuff into projectM */
    for (i = 0; i < 512; i++)
    {
        if (i % 2 == 0)
        {
            pcm_data[2 * i] = (float) (rand() / ((float) RAND_MAX) * (pow(2, 14)));
            pcm_data[2 * i + 1] = (float) (rand() / ((float) RAND_MAX) * (pow(2, 14)));
        }
        else
        {
            pcm_data[2 * i] = (float) (rand() / ((float) RAND_MAX) * (pow(2, 14)));
            pcm_data[2 * i + 1] = (float) (rand() / ((float) RAND_MAX) * (pow(2, 14)));
        }
        if (i % 2 == 1)
        {
            pcm_data[2 * i] = -pcm_data[2 * i];
            pcm_data[2 * i + 1] = -pcm_data[2 * i + 1];
        }
    }

    /** Add the waveform data */
    projectm_pcm_add_int16(_projectM, pcm_data, 512, PROJECTM_STEREO);
}

void projectMSDL::resize(unsigned int width_, unsigned int height_)
{
    _width = width_;
    _height = height_;

    // Hide cursor if window size equals desktop size
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0)
    {
        SDL_ShowCursor(_isFullScreen ? SDL_DISABLE : SDL_ENABLE);
    }

    projectm_set_window_size(_projectM, _width, _height);
}

void projectMSDL::pollEvent()
{
    SDL_Event evt;

    int mousex = 0;
    float mousexscale = 0;
    int mousey = 0;
    float mouseyscale = 0;
    int mousepressure = 0;
    while (SDL_PollEvent(&evt))
    {
        switch (evt.type)
        {
            case SDL_WINDOWEVENT:
                int h, w;
                SDL_GL_GetDrawableSize(_sdlWindow, &w, &h);
                switch (evt.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                        resize(w, h);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resize(w, h);
                        break;
                }
                break;
            case SDL_MOUSEWHEEL:
                scrollHandler(&evt);

            case SDL_KEYDOWN:
                keyHandler(&evt);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (evt.button.button == SDL_BUTTON_LEFT)
                {
                    // if it's the first mouse down event (since mouse up or since SDL was launched)
                    if (!mouseDown)
                    {
                        // Get mouse coorindates when you click.
                        SDL_GetMouseState(&mousex, &mousey);
                        // Scale those coordinates. libProjectM supports a scale of 0.1 instead of absolute pixel coordinates.
                        mousexscale = (mousex / (float) _width);
                        mouseyscale = ((_height - mousey) / (float) _height);
                        // Touch. By not supplying a touch type, we will default to random.
                        touch(mousexscale, mouseyscale, mousepressure);
                        mouseDown = true;
                    }
                }
                else if (evt.button.button == SDL_BUTTON_RIGHT)
                {
                    mouseDown = false;

                    // Keymod = Left or Right Gui or Left Ctrl. This is a shortcut to remove all waveforms.
                    if (keymod)
                    {
                        touchDestroyAll();
                        keymod = false;
                        break;
                    }

                    // Right Click
                    SDL_GetMouseState(&mousex, &mousey);

                    // Scale those coordinates. libProjectM supports a scale of 0.1 instead of absolute pixel coordinates.
                    mousexscale = (mousex / (float) _width);
                    mouseyscale = ((_height - mousey) / (float) _height);

                    // Destroy at the coordinates we clicked.
                    touchDestroy(mousexscale, mouseyscale);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                mouseDown = false;
                break;

            case SDL_QUIT:
                done = true;
                break;
        }
    }

    // Handle dragging your waveform when mouse is down.
    if (mouseDown)
    {
        // Get mouse coordinates when you click.
        SDL_GetMouseState(&mousex, &mousey);
        // Scale those coordinates. libProjectM supports a scale of 0.1 instead of absolute pixel coordinates.
        mousexscale = (mousex / (float) _width);
        mouseyscale = ((_height - mousey) / (float) _height);
        // Drag Touch.
        touchDrag(mousexscale, mouseyscale, mousepressure);
    }
}

// This touches the screen to generate a waveform at X / Y.
void projectMSDL::touch(float x, float y, int pressure, int touchtype)
{
#ifdef PROJECTM_TOUCH_ENABLED
    projectm_touch(_projectM, x, y, pressure, static_cast<projectm_touch_type>(touchtype));
#endif
}

// This moves the X Y of your existing waveform that was generated by a touch (only if you held down your click and dragged your mouse around).
void projectMSDL::touchDrag(float x, float y, int pressure)
{
    projectm_touch_drag(_projectM, x, y, pressure);
}

// Remove waveform at X Y
void projectMSDL::touchDestroy(float x, float y)
{
    projectm_touch_destroy(_projectM, x, y);
}

// Remove all waveforms
void projectMSDL::touchDestroyAll()
{
    projectm_touch_destroy_all(_projectM);
}

void projectMSDL::renderFrame()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(_projectM);

    SDL_GL_SwapWindow(_sdlWindow);
}

void projectMSDL::init(SDL_Window* window, const bool _renderToTexture)
{
    _sdlWindow = window;
    projectm_set_window_size(_projectM, _width, _height);

#ifdef WASAPI_LOOPBACK
    wasapi = true;
#endif
}

std::string projectMSDL::getActivePresetName()
{
    unsigned int index = projectm_playlist_get_position(_playlist);
    if (index)
    {
        auto presetName = projectm_playlist_item(_playlist, index);
        std::string presetNameString(presetName);
        projectm_playlist_free_string(presetName);
        return presetNameString;
    }
    return {};
}

void projectMSDL::presetSwitchedEvent(bool isHardCut, unsigned int index, void* context)
{
    auto app = reinterpret_cast<projectMSDL*>(context);
    auto presetName = projectm_playlist_item(app->_playlist, index);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Displaying preset: %s\n", presetName);

    app->_presetName = presetName;
    projectm_playlist_free_string(presetName);

    // Apply random duration if random mode is enabled
    if (app->_randomDurationMode)
    {
        double randomDuration = 5.0 + (rand() % 116);  // 5-120 seconds
        projectm_set_preset_duration(app->_projectM, randomDuration);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "🎲 Random Duration: %.0f seconds", randomDuration);
    }

    app->UpdateWindowTitle();
}

projectm_handle projectMSDL::projectM()
{
    return _projectM;
}

void projectMSDL::setFps(size_t fps)
{
    _fps = fps;
}

size_t projectMSDL::fps() const
{
    return _fps;
}

void projectMSDL::UpdateWindowTitle()
{
    std::string title = "projectM ➫ " + _presetName;
    if (projectm_get_preset_locked(_projectM))
    {
        title.append(" [locked]");
    }
    if (_favoritesOnlyMode)
    {
        title.append(" [favorites]");
    }
    SDL_SetWindowTitle(_sdlWindow, title.c_str());
}
