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
* pmSDL.hpp 
* Authors: Created by Mischa Spiegelmock on 2017-09-18.
*
*/

#pragma once

#include <SDL.h>

// Disable LOOPBACK and FAKE audio to enable microphone input
#ifdef _WIN32
#define WASAPI_LOOPBACK 1
#endif /** _WIN32 */
#define FAKE_AUDIO 0
// ----------------------------
#define TEST_ALL_PRESETS 0
#define STEREOSCOPIC_SBS 0

// projectM
#include <projectM-4/playlist.h>
#include <projectM-4/projectM.h>

// projectM SDL
#include "audioCapture.hpp"
#include "ControlWindow.hpp"
#include "loopback.hpp"
#include "opengl.h"
#include "setup.hpp"


#if defined _MSC_VER
#include <direct.h>
#endif

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <string>
#include <sys/stat.h>

#ifdef WASAPI_LOOPBACK
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <functiondiscoverykeys_devpkey.h>
#include <avrt.h>

#include <mmsystem.h>
#include <stdio.h>


#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) LOG(L"Error: " format, __VA_ARGS__)

#endif /** WASAPI_LOOPBACK */

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#include "SDL.h"
#else
#include <SDL.h>
#endif /** _WIN32 */


// DATADIR_PATH should be set by the root Makefile if this is being
// built with autotools.
#ifndef DATADIR_PATH
#ifdef DEBUG
#define DATADIR_PATH "."
#ifndef _WIN32
#warning "DATADIR_PATH is not defined - falling back to ./"
#else
#pragma warning "DATADIR_PATH is not defined - falling back to ./"
#endif /** _WIN32 */
#else
#define DATADIR_PATH "/usr/local/share/projectM"
#ifndef _WIN32
#warning "DATADIR_PATH is not defined - falling back to /usr/local/share/projectM"
#endif /** _WIN32 */
#endif
#endif

class projectMSDL
{

public:
    projectMSDL(SDL_GLContext glCtx, const std::string& presetPath);

    ~projectMSDL();

    void init(SDL_Window* window, const bool renderToTexture = false);
    int openAudioInput();
    int toggleAudioInput();
    int initAudioInput();
    void beginAudioCapture();
    void endAudioCapture();
    void stretchMonitors();
    void nextMonitor();
    void toggleFullScreen();
    void resize(unsigned int width, unsigned int height);
    void touch(float x, float y, int pressure, int touchtype = 0);
    void touchDrag(float x, float y, int pressure);
    void touchDestroy(float x, float y);
    void touchDestroyAll();
    void renderFrame();
    void pollEvent();
    void processTerminalCommand();
    void printTerminalHelp();
    void setTerminalMode();
    void restoreTerminalMode();
    void openControlWindow();
    void updateControlWindow();
    void renderControlWindow();
    bool keymod = false;
    std::string getActivePresetName();
    void addFakePCM();
    projectm_handle projectM();
    void setFps(size_t fps);
    size_t fps() const;

    bool done{false};
    bool mouseDown{false};
    bool wasapi{false};    // Used to track if wasapi is currently active. This bool will allow us to run a WASAPI app and still toggle to microphone inputs.
    bool fakeAudio{false}; // Used to track fake audio, so we can turn it off and on.
    bool stretch{false};   // used for toggling stretch mode

    SDL_GLContext _openGlContext{nullptr};

private:
    static void presetSwitchedEvent(bool isHardCut, uint32_t index, void* context);

    static void audioInputCallbackF32(void* userdata, unsigned char* stream, int len);

    void UpdateWindowTitle();

    void scrollHandler(SDL_Event*);
    void keyHandler(SDL_Event*);
    void printKeyboardShortcuts();
    void toggleFavoritesMode();
    void movePresetToDeleted();
    void reloadPlaylist();

    projectm_handle _projectM{nullptr};
    projectm_playlist_handle _playlist{nullptr};

    std::string _presetsBasePath; //!< Base path to presets directory
    std::string _deletedPath;     //!< Path to deleted subdirectory
    bool _favoritesOnlyMode{false}; //!< Whether to show only rating 9 presets

    SDL_Window* _sdlWindow{nullptr};
    bool _isFullScreen{false};
    size_t _width{0};
    size_t _height{0};
    size_t _fps{60};

    bool _shuffle{true};
    bool _randomDurationMode{false}; //!< Random duration for each preset

    // audio input device characteristics
    unsigned int _numAudioDevices{0};
    int _curAudioDevice{0}; // SDL's device indexes are 0-based, -1 means "system default"
    unsigned short _audioChannelsCount{0};
    SDL_AudioDeviceID _audioDeviceId{0};
    int _selectedAudioDevice{0};

    std::string _presetName; //!< Current preset name

    // Control window
    ControlWindow _controlWindow;

    // Preset history for back/forward navigation
    std::vector<uint32_t> _presetHistory;
    int _historyIndex{-1};
    static const size_t MAX_HISTORY_SIZE = 100;
    bool _navigatingHistory{false}; //!< Flag to prevent adding to history during navigation

    void addToHistory(uint32_t presetIndex);
    void navigateHistoryBack();
    void navigateHistoryForward();

    // Rating system
    std::map<std::string, int> _ratings; //!< Preset filename -> rating (1-9)
    std::string _ratingsFilePath;        //!< Path to ratings storage file
    void loadRatings();
    void saveRatings();
    void setPresetRating(int rating);
    int getPresetRating(const std::string& presetPath);
    void selectWeightedRandomPreset();
    void addToFavorites();  //!< Sets rating to 9 (max)
};
