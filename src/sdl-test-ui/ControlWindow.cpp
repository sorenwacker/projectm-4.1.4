/**
 * ControlWindow - Separate SDL2 window for projectM controls
 */

#include "ControlWindow.hpp"
#include <algorithm>

ControlWindow::ControlWindow() {
    _sliderRect = {BUTTON_MARGIN, 200, WINDOW_WIDTH - 2 * BUTTON_MARGIN, 30};
}

ControlWindow::~ControlWindow() {
    close();
}

bool ControlWindow::init(int x, int y) {
    _window = SDL_CreateWindow(
        "projectM Controls",
        x, y,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!_window) {
        SDL_Log("Failed to create control window: %s", SDL_GetError());
        return false;
    }

    _windowID = SDL_GetWindowID(_window);

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(_window);
        _window = nullptr;
        return false;
    }

    createButtons();
    return true;
}

void ControlWindow::close() {
    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }
    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
}

void ControlWindow::createButtons() {
    _buttons.clear();

    int row1_y = 80;
    int row2_y = 130;
    int row3_y = 180;

    // Row 1: Playback controls
    int x = BUTTON_MARGIN;

    _buttons.push_back({
        {x, row1_y, BUTTON_WIDTH, BUTTON_HEIGHT},
        "< Prev",
        [this]() { if (onPrev) onPrev(); }
    });
    x += BUTTON_WIDTH + BUTTON_MARGIN;

    _buttons.push_back({
        {x, row1_y, BUTTON_WIDTH, BUTTON_HEIGHT},
        "Random",
        [this]() { if (onRandom) onRandom(); }
    });
    x += BUTTON_WIDTH + BUTTON_MARGIN;

    _buttons.push_back({
        {x, row1_y, BUTTON_WIDTH, BUTTON_HEIGHT},
        "Next >",
        [this]() { if (onNext) onNext(); }
    });
    x += BUTTON_WIDTH + BUTTON_MARGIN;

    _buttons.push_back({
        {x, row1_y, BUTTON_WIDTH, BUTTON_HEIGHT},
        "Shuffle",
        [this]() { if (onToggleShuffle) onToggleShuffle(); }
    });

    // Row 2: Mode controls
    x = BUTTON_MARGIN;

    _buttons.push_back({
        {x, row2_y, BUTTON_WIDTH, BUTTON_HEIGHT},
        "Lock",
        [this]() { if (onToggleLock) onToggleLock(); }
    });
    x += BUTTON_WIDTH + BUTTON_MARGIN;

    _buttons.push_back({
        {x, row2_y, BUTTON_WIDTH + 20, BUTTON_HEIGHT},
        "+ Favorite",
        [this]() { if (onAddFavorite) onAddFavorite(); }
    });
    x += BUTTON_WIDTH + 20 + BUTTON_MARGIN;

    _buttons.push_back({
        {x, row2_y, BUTTON_WIDTH + 20, BUTTON_HEIGHT},
        "Favs Only",
        [this]() { if (onToggleFavorites) onToggleFavorites(); }
    });
    x += BUTTON_WIDTH + 20 + BUTTON_MARGIN;

    _buttons.push_back({
        {x, row2_y, BUTTON_WIDTH, BUTTON_HEIGHT},
        "Fullscr",
        [this]() { if (onFullscreen) onFullscreen(); }
    });
}

void ControlWindow::render() {
    if (!_renderer) return;

    // Dark background
    SDL_SetRenderDrawColor(_renderer, 30, 30, 35, 255);
    SDL_RenderClear(_renderer);

    // Preset name header
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {150, 150, 150, 255};
    SDL_Color accent = {100, 180, 255, 255};

    // Draw preset info area
    SDL_SetRenderDrawColor(_renderer, 40, 40, 45, 255);
    SDL_Rect infoRect = {BUTTON_MARGIN, BUTTON_MARGIN, WINDOW_WIDTH - 2 * BUTTON_MARGIN, 60};
    SDL_RenderFillRect(_renderer, &infoRect);

    // Preset name (truncate if too long)
    std::string displayName = _presetName;
    if (displayName.length() > 45) {
        displayName = displayName.substr(0, 42) + "...";
    }
    renderText(displayName, BUTTON_MARGIN + 10, BUTTON_MARGIN + 10, white);

    // Preset index
    std::string indexText = "Preset " + std::to_string(_currentPreset + 1) + " / " + std::to_string(_totalPresets);
    renderText(indexText, BUTTON_MARGIN + 10, BUTTON_MARGIN + 35, gray);

    // Render buttons
    for (auto& btn : _buttons) {
        renderButton(btn);
    }

    // Duration slider label
    std::string durationLabel = "Duration: " + std::to_string((int)_duration) + "s";
    renderText(durationLabel, BUTTON_MARGIN, 230, gray);

    // Duration slider
    renderSlider(_sliderRect.x, _sliderRect.y + 30, _sliderRect.w, 20, _duration, 5.0, 120.0);

    // Status indicators at bottom
    int statusY = WINDOW_HEIGHT - 30;
    std::string status = "";
    if (_shuffleEnabled) status += "[Shuffle] ";
    if (_lockEnabled) status += "[Locked] ";
    if (_favoritesMode) status += "[Favorites] ";
    if (!status.empty()) {
        renderText(status, BUTTON_MARGIN, statusY, accent);
    }

    SDL_RenderPresent(_renderer);
}

void ControlWindow::renderButton(const Button& btn) {
    // Button background
    if (btn.pressed) {
        SDL_SetRenderDrawColor(_renderer, 80, 140, 200, 255);
    } else if (btn.hovered) {
        SDL_SetRenderDrawColor(_renderer, 70, 70, 80, 255);
    } else {
        SDL_SetRenderDrawColor(_renderer, 50, 50, 60, 255);
    }
    SDL_RenderFillRect(_renderer, &btn.rect);

    // Button border
    SDL_SetRenderDrawColor(_renderer, 100, 100, 110, 255);
    SDL_RenderDrawRect(_renderer, &btn.rect);

    // Button label (centered approximation)
    SDL_Color white = {255, 255, 255, 255};
    int textX = btn.rect.x + (btn.rect.w - btn.label.length() * 8) / 2;
    int textY = btn.rect.y + (btn.rect.h - 16) / 2;
    renderText(btn.label, textX, textY, white);
}

void ControlWindow::renderText(const std::string& text, int x, int y, SDL_Color color) {
    // Simple text rendering using rectangles (no SDL_ttf dependency)
    // Each character is an 8x16 block
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);

    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        if (c == ' ') continue;

        // Simple block representation
        SDL_Rect charRect = {x + (int)(i * 8), y, 6, 12};
        SDL_RenderFillRect(_renderer, &charRect);
    }
}

void ControlWindow::renderSlider(int x, int y, int width, int height, double value, double min, double max) {
    // Slider track
    SDL_SetRenderDrawColor(_renderer, 60, 60, 70, 255);
    SDL_Rect track = {x, y, width, height};
    SDL_RenderFillRect(_renderer, &track);

    // Slider fill
    double normalized = (value - min) / (max - min);
    int fillWidth = (int)(width * normalized);
    SDL_SetRenderDrawColor(_renderer, 80, 150, 220, 255);
    SDL_Rect fill = {x, y, fillWidth, height};
    SDL_RenderFillRect(_renderer, &fill);

    // Slider handle
    SDL_SetRenderDrawColor(_renderer, 200, 200, 210, 255);
    SDL_Rect handle = {x + fillWidth - 5, y - 2, 10, height + 4};
    SDL_RenderFillRect(_renderer, &handle);
}

void ControlWindow::handleEvent(SDL_Event& event) {
    if (!_window) return;

    // Only handle events for this window
    if (event.type == SDL_WINDOWEVENT && event.window.windowID == _windowID) {
        if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
            close();
            return;
        }
    }

    if (event.window.windowID != _windowID) return;

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    switch (event.type) {
        case SDL_MOUSEMOTION:
            // Update button hover states
            for (auto& btn : _buttons) {
                btn.hovered = SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &btn.rect);
            }

            // Handle slider dragging
            if (_draggingSlider) {
                SDL_Rect sliderArea = {_sliderRect.x, _sliderRect.y + 30, _sliderRect.w, 20};
                double normalized = (double)(mouseX - sliderArea.x) / sliderArea.w;
                normalized = std::max(0.0, std::min(1.0, normalized));
                _duration = 5.0 + normalized * (120.0 - 5.0);
                if (onSetDuration) onSetDuration(_duration);
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                // Check buttons
                for (auto& btn : _buttons) {
                    if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &btn.rect)) {
                        btn.pressed = true;
                    }
                }

                // Check slider
                SDL_Rect sliderArea = {_sliderRect.x, _sliderRect.y + 30, _sliderRect.w, 24};
                if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &sliderArea)) {
                    _draggingSlider = true;
                    double normalized = (double)(mouseX - sliderArea.x) / sliderArea.w;
                    normalized = std::max(0.0, std::min(1.0, normalized));
                    _duration = 5.0 + normalized * (120.0 - 5.0);
                    if (onSetDuration) onSetDuration(_duration);
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                _draggingSlider = false;

                for (auto& btn : _buttons) {
                    if (btn.pressed && SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &btn.rect)) {
                        if (btn.onClick) btn.onClick();
                    }
                    btn.pressed = false;
                }
            }
            break;
    }
}

void ControlWindow::setPresetName(const std::string& name) {
    _presetName = name;
}

void ControlWindow::setShuffleState(bool enabled) {
    _shuffleEnabled = enabled;
}

void ControlWindow::setLockState(bool locked) {
    _lockEnabled = locked;
}

void ControlWindow::setFavoritesMode(bool enabled) {
    _favoritesMode = enabled;
}

void ControlWindow::setDuration(double seconds) {
    _duration = seconds;
}

void ControlWindow::setPresetIndex(uint32_t current, uint32_t total) {
    _currentPreset = current;
    _totalPresets = total;
}
