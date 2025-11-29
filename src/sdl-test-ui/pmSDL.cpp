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
#include <sstream>
#include <cstdlib>
#include <random>
#ifndef _WIN32
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <pwd.h>
static struct termios orig_termios;
#endif
#include <fstream>

projectMSDL::projectMSDL(SDL_GLContext glCtx, const std::string& presetPath)
    : _openGlContext(glCtx)
    , _projectM(projectm_create())
    , _playlist(projectm_playlist_create(_projectM))
    , _presetsBasePath(presetPath)
    , _deletedPath(presetPath + "/deleted")
{
    projectm_get_window_size(_projectM, &_width, &_height);
    projectm_playlist_set_preset_switched_event_callback(_playlist, &projectMSDL::presetSwitchedEvent, static_cast<void*>(this));

    // Create deleted directory if it doesn't exist
#if defined _MSC_VER
    _mkdir(_deletedPath.c_str());
#else
    mkdir(_deletedPath.c_str(), 0755);
#endif

    // Set up ratings file path (~/.projectM/ratings.txt)
#ifdef _WIN32
    char* appdata = getenv("APPDATA");
    if (appdata) {
        _ratingsFilePath = std::string(appdata) + "\\projectM\\ratings.txt";
        _mkdir((std::string(appdata) + "\\projectM").c_str());
    } else {
        _ratingsFilePath = "ratings.txt";
    }
#else
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
    if (home) {
        std::string projectMDir = std::string(home) + "/.projectM";
        mkdir(projectMDir.c_str(), 0755);
        _ratingsFilePath = projectMDir + "/ratings.txt";
    } else {
        _ratingsFilePath = "ratings.txt";
    }
#endif
    loadRatings();

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
    printf("  S                 - Toggle shuffle\n");
    printf("  T                 - Toggle favorites-only mode\n");
    printf("  Mouse Scroll      - Change presets\n");
    printf("\nPreset Duration:\n");
    printf("  1-9               - Set duration (1=5s, 2=10s ... 9=120s)\n");
    printf("  0                 - Toggle random mode (5-120s per preset)\n");
    printf("\nAudio:\n");
    printf("  CMD+I             - Cycle audio input devices\n");
    printf("  CMD+UP/DOWN       - Adjust beat sensitivity\n");
    printf("\nDisplay:\n");
    printf("  M                 - Toggle fullscreen\n");
    printf("  CMD+M             - Change monitor\n");
    printf("  CMD+S             - Stretch across monitors\n");
    printf("  A                 - Toggle aspect correction\n");
    printf("\nTime Control:\n");
    printf("  UP/DOWN Arrow     - Adjust time scale (0.01x - 2.0x, adaptive increments)\n");
    printf("  Y                 - Toggle slow motion (0.1x/1.0x)\n");
    printf("\nOther:\n");
    printf("  C                 - Open control window\n");
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
    printf("  N or ->          - Next preset\n");
    printf("  P or <-          - Previous preset\n");
    printf("  R                - Random preset\n");
    printf("  B                - History back\n");
    printf("  W                - History forward\n");
    printf("  L                - Lock/unlock current preset\n");
    printf("  S                - Toggle shuffle\n");
    printf("\nRating (1-9):\n");
    printf("  1-9              - Set preset rating (higher = more likely)\n");
    printf("  0                - Clear rating\n");
    printf("\nFavorites:\n");
    printf("  T                - Toggle favorites-only mode\n");
    printf("\nTime & Audio Control:\n");
    printf("  up/+             - Increase time scale\n");
    printf("  down/-           - Decrease time scale\n");
    printf("  Y                - Toggle slow motion (0.1x/1.0x)\n");
    printf("  ]                - Increase beat sensitivity\n");
    printf("  [                - Decrease beat sensitivity\n");
    printf("\nDisplay:\n");
    printf("  M                - Toggle fullscreen\n");
    printf("  A                - Toggle aspect correction\n");
    printf("  C                - Open control window\n");
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
            selectWeightedRandomPreset();
            break;

        case 'L':
            {
                bool newValue = !projectm_get_preset_locked(_projectM);
                projectm_set_preset_locked(_projectM, newValue);
                UpdateWindowTitle();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[L] 🔒 Preset Lock: %s", newValue ? "LOCKED" : "UNLOCKED");
            }
            break;

        case 'S':
            _shuffle = !_shuffle;
            projectm_playlist_set_shuffle(_playlist, _shuffle);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[S] Shuffle: %s", _shuffle ? "ON" : "OFF");
            break;

        case 'F':
            addToFavorites();
            break;

        case 'T':
            toggleFavoritesMode();
            break;

        case 'B':
            navigateHistoryBack();
            break;

        case 'W':
            navigateHistoryForward();
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

        case 'Y':
            {
                float currentScale = projectm_get_time_scale(_projectM);
                float newScale = (currentScale == 1.0f) ? 0.1f : 1.0f;
                projectm_set_time_scale(_projectM, newScale);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[Y] Slow Motion: %s (%.1fx)",
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
            setPresetRating(1);
            break;
        case '2':
            setPresetRating(2);
            break;
        case '3':
            setPresetRating(3);
            break;
        case '4':
            setPresetRating(4);
            break;
        case '5':
            setPresetRating(5);
            break;
        case '6':
            setPresetRating(6);
            break;
        case '7':
            setPresetRating(7);
            break;
        case '8':
            setPresetRating(8);
            break;
        case '9':
            setPresetRating(9);
            break;
        case '0':
            setPresetRating(0); // Clear rating
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

    // Always load from the base path
    projectm_playlist_add_path(_playlist, _presetsBasePath.c_str(), true, false);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loaded %d presets%s",
                projectm_playlist_size(_playlist),
                _favoritesOnlyMode ? " (Favs mode: only rating 9 presets will play)" : "");
}

void projectMSDL::toggleFavoritesMode()
{
    _favoritesOnlyMode = !_favoritesOnlyMode;
    reloadPlaylist();

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Favorites-Only Mode: %s", _favoritesOnlyMode ? "ON" : "OFF");
    UpdateWindowTitle();

    // If we just enabled favorites mode and current preset isn't a favorite,
    // immediately switch to a random favorite
    if (_favoritesOnlyMode)
    {
        int currentRating = getPresetRating(_presetName);
        if (currentRating != 9)
        {
            selectWeightedRandomPreset();
        }
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

        case SDLK_c:
            // C (without modifier): Open control window
            if (!(sdl_mod & KMOD_LGUI) && !(sdl_mod & KMOD_RGUI) && !(sdl_mod & KMOD_LCTRL))
            {
                openControlWindow();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Control window opened");
                return;
            }
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
                // s without modifier: shuffle toggle
                _shuffle = !_shuffle;
                projectm_playlist_set_shuffle(_playlist, _shuffle);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shuffle: %s", _shuffle ? "ON" : "OFF");
            }
            break;

        case SDLK_y:
            {
                // y: slow motion toggle
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
            else
            {
                // m without modifier: toggle fullscreen
#if !STEREOSCOPIC_SBS
                toggleFullScreen();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fullscreen: %s", _isFullScreen ? "ON" : "OFF");
#endif
            }
            break;

        case SDLK_t:
            // t: toggle favorites-only mode
            toggleFavoritesMode();
            break;

        case SDLK_r:
            // Use weighted random selection based on ratings
            selectWeightedRandomPreset();
            break;

        case SDLK_b:
            navigateHistoryBack();
            break;

        case SDLK_w:
            navigateHistoryForward();
            break;

        case SDLK_LEFT:
            navigateHistoryBack();
            break;

        case SDLK_RIGHT:
            navigateHistoryForward();
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

        // Rating controls (number keys 1-9, 0 to clear)
        case SDLK_1:
            setPresetRating(1);
            break;
        case SDLK_2:
            setPresetRating(2);
            break;
        case SDLK_3:
            setPresetRating(3);
            break;
        case SDLK_4:
            setPresetRating(4);
            break;
        case SDLK_5:
            setPresetRating(5);
            break;
        case SDLK_6:
            setPresetRating(6);
            break;
        case SDLK_7:
            setPresetRating(7);
            break;
        case SDLK_8:
            setPresetRating(8);
            break;
        case SDLK_9:
            setPresetRating(9);
            break;
        case SDLK_0:
            setPresetRating(0); // Clear rating
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
        // Handle control window events first
        if (_controlWindow.isOpen()) {
            _controlWindow.handleEvent(evt);
        }

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
                    case SDL_WINDOWEVENT_MOVED:
                    case SDL_WINDOWEVENT_DISPLAY_CHANGED:
                        // When window moves to different display, update resolution
                        // This fixes HiDPI issues when moving between Retina and non-Retina displays
                        resize(w, h);
                        break;
                    case SDL_WINDOWEVENT_EXPOSED:
                        // Refresh when window is exposed (e.g., after being occluded)
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

    // In favorites mode, check if this preset is a favorite (rating 9)
    // If not, immediately switch to a random favorite instead
    if (app->_favoritesOnlyMode && !app->_navigatingHistory)
    {
        int rating = app->getPresetRating(presetName);
        if (rating != 9)
        {
            projectm_playlist_free_string(presetName);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset not a favorite (rating %d), switching to random favorite", rating);
            app->selectWeightedRandomPreset();
            return;  // selectWeightedRandomPreset will trigger another callback
        }
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Displaying preset: %s\n", presetName);

    app->_presetName = presetName;
    projectm_playlist_free_string(presetName);

    // Add to history if not navigating through history
    if (!app->_navigatingHistory)
    {
        app->addToHistory(index);
    }
    app->_navigatingHistory = false;

    // Apply random duration if random mode is enabled
    if (app->_randomDurationMode)
    {
        double randomDuration = 5.0 + (rand() % 116);  // 5-120 seconds
        projectm_set_preset_duration(app->_projectM, randomDuration);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Random Duration: %.0f seconds", randomDuration);
    }

    app->UpdateWindowTitle();
}

void projectMSDL::addToHistory(uint32_t presetIndex)
{
    // If we're not at the end of history, truncate future entries
    if (_historyIndex >= 0 && _historyIndex < (int)_presetHistory.size() - 1)
    {
        _presetHistory.resize(_historyIndex + 1);
    }

    // Add new entry
    _presetHistory.push_back(presetIndex);

    // Limit history size
    if (_presetHistory.size() > MAX_HISTORY_SIZE)
    {
        _presetHistory.erase(_presetHistory.begin());
    }

    _historyIndex = (int)_presetHistory.size() - 1;
}

void projectMSDL::navigateHistoryBack()
{
    if (_historyIndex > 0)
    {
        _historyIndex--;
        _navigatingHistory = true;
        projectm_playlist_set_position(_playlist, _presetHistory[_historyIndex], true);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "History back (%d/%d)", _historyIndex + 1, (int)_presetHistory.size());
    }
    else
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "At beginning of history");
    }
}

void projectMSDL::navigateHistoryForward()
{
    if (_historyIndex < (int)_presetHistory.size() - 1)
    {
        _historyIndex++;
        _navigatingHistory = true;
        projectm_playlist_set_position(_playlist, _presetHistory[_historyIndex], true);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "History forward (%d/%d)", _historyIndex + 1, (int)_presetHistory.size());
    }
    else
    {
        // At end of history, go to next preset in playlist
        projectm_playlist_play_next(_playlist, true);
    }
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

void projectMSDL::openControlWindow()
{
    if (_controlWindow.isOpen()) {
        return; // Already open
    }

    // Position control window to the right of the main window
    int mainX, mainY;
    SDL_GetWindowPosition(_sdlWindow, &mainX, &mainY);

    if (!_controlWindow.init(mainX + 50, mainY + 50)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open control window");
        return;
    }

    // Set up callbacks
    _controlWindow.onPrev = [this]() {
        projectm_playlist_play_previous(_playlist, true);
    };

    _controlWindow.onNext = [this]() {
        projectm_playlist_play_next(_playlist, true);
    };

    _controlWindow.onRandom = [this]() {
        selectWeightedRandomPreset();
    };

    _controlWindow.onToggleShuffle = [this]() {
        _shuffle = !_shuffle;
        projectm_playlist_set_shuffle(_playlist, _shuffle);
        _controlWindow.setShuffleState(_shuffle);
    };

    _controlWindow.onToggleLock = [this]() {
        bool newValue = !projectm_get_preset_locked(_projectM);
        projectm_set_preset_locked(_projectM, newValue);
        _controlWindow.setLockState(newValue);
        UpdateWindowTitle();
    };

    _controlWindow.onAddFavorite = [this]() {
        addToFavorites();
    };

    _controlWindow.onToggleFavorites = [this]() {
        toggleFavoritesMode();
        _controlWindow.setFavoritesMode(_favoritesOnlyMode);
    };

    _controlWindow.onFullscreen = [this]() {
        toggleFullScreen();
    };

    _controlWindow.onSetDuration = [this](double duration) {
        _randomDurationMode = false;
        projectm_set_preset_duration(_projectM, duration);
    };

    _controlWindow.onHistoryBack = [this]() {
        navigateHistoryBack();
    };

    _controlWindow.onHistoryForward = [this]() {
        navigateHistoryForward();
    };

    _controlWindow.onDelete = [this]() {
        movePresetToDeleted();
    };

    _controlWindow.onSetSpeed = [this](float speed) {
        projectm_set_time_scale(_projectM, speed);
    };

    _controlWindow.onSetBeatSensitivity = [this](float sensitivity) {
        projectm_set_beat_sensitivity(_projectM, sensitivity);
    };

    _controlWindow.onSetRating = [this](int rating) {
        setPresetRating(rating);
    };

    _controlWindow.onToggleRandomDuration = [this]() {
        _randomDurationMode = !_randomDurationMode;
        _controlWindow.setRandomDurationMode(_randomDurationMode);
        if (_randomDurationMode) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Random Duration Mode: ON");
        } else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Random Duration Mode: OFF");
        }
    };

    // Initial state
    updateControlWindow();
}

void projectMSDL::updateControlWindow()
{
    if (!_controlWindow.isOpen()) return;

    _controlWindow.setPresetName(_presetName);
    _controlWindow.setShuffleState(_shuffle);
    _controlWindow.setLockState(projectm_get_preset_locked(_projectM));
    _controlWindow.setFavoritesMode(_favoritesOnlyMode);
    _controlWindow.setDuration(projectm_get_preset_duration(_projectM));
    _controlWindow.setPresetIndex(
        projectm_playlist_get_position(_playlist),
        projectm_playlist_size(_playlist)
    );
    _controlWindow.setHistoryPosition(_historyIndex + 1, (int)_presetHistory.size());
    _controlWindow.setRating(getPresetRating(_presetName));
    _controlWindow.setSpeed(projectm_get_time_scale(_projectM));
    _controlWindow.setBeatSensitivity(projectm_get_beat_sensitivity(_projectM));
    _controlWindow.setRandomDurationMode(_randomDurationMode);
}

void projectMSDL::renderControlWindow()
{
    if (!_controlWindow.isOpen()) return;

    updateControlWindow();
    _controlWindow.render();
}

void projectMSDL::loadRatings()
{
    _ratings.clear();
    std::ifstream file(_ratingsFilePath);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Format: rating|filename
        size_t sep = line.find('|');
        if (sep != std::string::npos && sep > 0) {
            int rating = std::stoi(line.substr(0, sep));
            std::string filename = line.substr(sep + 1);
            if (rating >= 1 && rating <= 9) {
                _ratings[filename] = rating;
            }
        }
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loaded %zu preset ratings", _ratings.size());
}

void projectMSDL::saveRatings()
{
    std::ofstream file(_ratingsFilePath);
    if (!file.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to save ratings to %s", _ratingsFilePath.c_str());
        return;
    }

    for (const auto& pair : _ratings) {
        file << pair.second << "|" << pair.first << "\n";
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Saved %zu preset ratings", _ratings.size());
}

void projectMSDL::setPresetRating(int rating)
{
    if (_presetName.empty()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "No preset loaded to rate");
        return;
    }

    // Extract just the filename from the full path
    size_t lastSlash = _presetName.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? _presetName.substr(lastSlash + 1) : _presetName;

    if (rating == 0) {
        // Remove rating
        _ratings.erase(filename);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Cleared rating for: %s", filename.c_str());
    } else if (rating >= 1 && rating <= 9) {
        _ratings[filename] = rating;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Set rating %d for: %s", rating, filename.c_str());
    }

    saveRatings();
    _controlWindow.setRating(rating);
}

int projectMSDL::getPresetRating(const std::string& presetPath)
{
    // Extract just the filename from the full path
    size_t lastSlash = presetPath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? presetPath.substr(lastSlash + 1) : presetPath;

    auto it = _ratings.find(filename);
    if (it != _ratings.end()) {
        return it->second;
    }
    return 5; // Default rating (unrated presets get middle rating)
}

void projectMSDL::selectWeightedRandomPreset()
{
    uint32_t playlistSize = projectm_playlist_size(_playlist);
    if (playlistSize == 0) return;

    // Build weighted list
    // In Favs mode: only include presets with rating 9
    // Otherwise: use rating as weight (default 5 for unrated)
    std::vector<std::pair<uint32_t, int>> weightedPresets;
    int totalWeight = 0;

    for (uint32_t i = 0; i < playlistSize; i++) {
        auto presetPath = projectm_playlist_item(_playlist, i);
        int rating = getPresetRating(presetPath);
        projectm_playlist_free_string(presetPath);

        if (_favoritesOnlyMode) {
            // In Favs mode, only include favorites (rating 9)
            if (rating == 9) {
                weightedPresets.push_back({i, 1});  // Equal weight for all favs
                totalWeight += 1;
            }
        } else {
            // Weight is the rating (1-9, default 5)
            weightedPresets.push_back({i, rating});
            totalWeight += rating;
        }
    }

    if (totalWeight == 0) {
        if (_favoritesOnlyMode) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "No favorites found (no presets with rating 9)");
        }
        return;
    }

    // Random selection based on weights
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, totalWeight);
    int randomValue = dis(gen);

    int cumulative = 0;
    for (const auto& preset : weightedPresets) {
        cumulative += preset.second;
        if (randomValue <= cumulative) {
            _navigatingHistory = false;
            projectm_playlist_set_position(_playlist, preset.first, true);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Weighted random selected preset %d%s",
                        preset.first, _favoritesOnlyMode ? " (favorite)" : "");
            return;
        }
    }
}

void projectMSDL::addToFavorites()
{
    // Adding to favorites just sets rating to 9 (max)
    setPresetRating(9);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Added to favorites (rating 9)");
}
