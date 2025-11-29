/**
 * ControlWindow - Separate SDL2 window for projectM controls
 */

#include "ControlWindow.hpp"
#include <algorithm>

// 5x7 bitmap font data for ASCII 32-127 (space to ~)
// Each character is 5 pixels wide, 7 pixels tall
static const unsigned char font5x7[96][7] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // space
    {0x04,0x04,0x04,0x04,0x00,0x04,0x00}, // !
    {0x0A,0x0A,0x00,0x00,0x00,0x00,0x00}, // "
    {0x0A,0x1F,0x0A,0x0A,0x1F,0x0A,0x00}, // #
    {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04}, // $
    {0x19,0x19,0x02,0x04,0x08,0x13,0x13}, // %
    {0x08,0x14,0x14,0x08,0x15,0x12,0x0D}, // &
    {0x04,0x04,0x00,0x00,0x00,0x00,0x00}, // '
    {0x02,0x04,0x08,0x08,0x08,0x04,0x02}, // (
    {0x08,0x04,0x02,0x02,0x02,0x04,0x08}, // )
    {0x00,0x04,0x15,0x0E,0x15,0x04,0x00}, // *
    {0x00,0x04,0x04,0x1F,0x04,0x04,0x00}, // +
    {0x00,0x00,0x00,0x00,0x04,0x04,0x08}, // ,
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}, // -
    {0x00,0x00,0x00,0x00,0x00,0x04,0x00}, // .
    {0x01,0x01,0x02,0x04,0x08,0x10,0x10}, // /
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, // 0
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}, // 1
    {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F}, // 2
    {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E}, // 3
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, // 4
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}, // 5
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, // 6
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, // 7
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, // 8
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}, // 9
    {0x00,0x04,0x00,0x00,0x04,0x00,0x00}, // :
    {0x00,0x04,0x00,0x00,0x04,0x04,0x08}, // ;
    {0x02,0x04,0x08,0x10,0x08,0x04,0x02}, // <
    {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00}, // =
    {0x08,0x04,0x02,0x01,0x02,0x04,0x08}, // >
    {0x0E,0x11,0x01,0x02,0x04,0x00,0x04}, // ?
    {0x0E,0x11,0x17,0x15,0x17,0x10,0x0E}, // @
    {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}, // A
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}, // B
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}, // C
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}, // D
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, // E
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}, // F
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F}, // G
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}, // H
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}, // I
    {0x07,0x02,0x02,0x02,0x02,0x12,0x0C}, // J
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11}, // K
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}, // L
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}, // M
    {0x11,0x11,0x19,0x15,0x13,0x11,0x11}, // N
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}, // O
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, // P
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}, // Q
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}, // R
    {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E}, // S
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}, // T
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}, // U
    {0x11,0x11,0x11,0x11,0x11,0x0A,0x04}, // V
    {0x11,0x11,0x11,0x15,0x15,0x15,0x0A}, // W
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}, // X
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}, // Y
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}, // Z
    {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E}, // [
    {0x10,0x10,0x08,0x04,0x02,0x01,0x01}, // backslash
    {0x0E,0x02,0x02,0x02,0x02,0x02,0x0E}, // ]
    {0x04,0x0A,0x11,0x00,0x00,0x00,0x00}, // ^
    {0x00,0x00,0x00,0x00,0x00,0x00,0x1F}, // _
    {0x08,0x04,0x00,0x00,0x00,0x00,0x00}, // `
    {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F}, // a
    {0x10,0x10,0x1E,0x11,0x11,0x11,0x1E}, // b
    {0x00,0x00,0x0E,0x11,0x10,0x11,0x0E}, // c
    {0x01,0x01,0x0F,0x11,0x11,0x11,0x0F}, // d
    {0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E}, // e
    {0x06,0x09,0x08,0x1C,0x08,0x08,0x08}, // f
    {0x00,0x00,0x0F,0x11,0x0F,0x01,0x0E}, // g
    {0x10,0x10,0x16,0x19,0x11,0x11,0x11}, // h
    {0x04,0x00,0x0C,0x04,0x04,0x04,0x0E}, // i
    {0x02,0x00,0x06,0x02,0x02,0x12,0x0C}, // j
    {0x10,0x10,0x12,0x14,0x18,0x14,0x12}, // k
    {0x0C,0x04,0x04,0x04,0x04,0x04,0x0E}, // l
    {0x00,0x00,0x1A,0x15,0x15,0x11,0x11}, // m
    {0x00,0x00,0x16,0x19,0x11,0x11,0x11}, // n
    {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E}, // o
    {0x00,0x00,0x1E,0x11,0x1E,0x10,0x10}, // p
    {0x00,0x00,0x0F,0x11,0x0F,0x01,0x01}, // q
    {0x00,0x00,0x16,0x19,0x10,0x10,0x10}, // r
    {0x00,0x00,0x0E,0x10,0x0E,0x01,0x1E}, // s
    {0x08,0x08,0x1C,0x08,0x08,0x09,0x06}, // t
    {0x00,0x00,0x11,0x11,0x11,0x13,0x0D}, // u
    {0x00,0x00,0x11,0x11,0x11,0x0A,0x04}, // v
    {0x00,0x00,0x11,0x11,0x15,0x15,0x0A}, // w
    {0x00,0x00,0x11,0x0A,0x04,0x0A,0x11}, // x
    {0x00,0x00,0x11,0x11,0x0F,0x01,0x0E}, // y
    {0x00,0x00,0x1F,0x02,0x04,0x08,0x1F}, // z
    {0x02,0x04,0x04,0x08,0x04,0x04,0x02}, // {
    {0x04,0x04,0x04,0x04,0x04,0x04,0x04}, // |
    {0x08,0x04,0x04,0x02,0x04,0x04,0x08}, // }
    {0x00,0x00,0x08,0x15,0x02,0x00,0x00}, // ~
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // DEL
};

ControlWindow::ControlWindow() {
    // Slider rects will be set in createButtons() after scale is known
}

ControlWindow::~ControlWindow() {
    close();
}

bool ControlWindow::init(int x, int y) {
    _window = SDL_CreateWindow(
        "projectM Controls",
        x, y,
        WINDOW_WIDTH(), WINDOW_HEIGHT(),
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

    int margin = BUTTON_MARGIN();
    int windowW = WINDOW_WIDTH();
    int usableW = windowW - 2 * margin;

    int row1_y = (int)(70 * _scale);
    int row2_y = (int)(110 * _scale);
    int btnHeight = (int)(32 * _scale);
    int btnSpacing = (int)(8 * _scale);

    // Row 1: Navigation - 5 equal buttons
    int navBtnW = (usableW - 4 * btnSpacing) / 5;
    int x = margin;

    _buttons.push_back({
        {x, row1_y, navBtnW, btnHeight},
        "<<",
        [this]() { if (onHistoryBack) onHistoryBack(); }
    });
    x += navBtnW + btnSpacing;

    _buttons.push_back({
        {x, row1_y, navBtnW, btnHeight},
        "Prev",
        [this]() { if (onPrev) onPrev(); }
    });
    x += navBtnW + btnSpacing;

    _buttons.push_back({
        {x, row1_y, navBtnW, btnHeight},
        "Random",
        [this]() { if (onRandom) onRandom(); }
    });
    x += navBtnW + btnSpacing;

    _buttons.push_back({
        {x, row1_y, navBtnW, btnHeight},
        "Next",
        [this]() { if (onNext) onNext(); }
    });
    x += navBtnW + btnSpacing;

    _buttons.push_back({
        {x, row1_y, navBtnW, btnHeight},
        ">>",
        [this]() { if (onHistoryForward) onHistoryForward(); }
    });

    // Row 2: Mode controls - 5 equal buttons
    int modeBtnW = (usableW - 4 * btnSpacing) / 5;
    x = margin;

    _buttons.push_back({
        {x, row2_y, modeBtnW, btnHeight},
        "Shuffle",
        [this]() { if (onToggleShuffle) onToggleShuffle(); }
    });
    x += modeBtnW + btnSpacing;

    _buttons.push_back({
        {x, row2_y, modeBtnW, btnHeight},
        "Lock",
        [this]() { if (onToggleLock) onToggleLock(); }
    });
    x += modeBtnW + btnSpacing;

    _buttons.push_back({
        {x, row2_y, modeBtnW, btnHeight},
        "Favs",
        [this]() { if (onToggleFavorites) onToggleFavorites(); }
    });
    x += modeBtnW + btnSpacing;

    _buttons.push_back({
        {x, row2_y, modeBtnW, btnHeight},
        "RndTime",
        [this]() { if (onToggleRandomDuration) onToggleRandomDuration(); }
    });
    x += modeBtnW + btnSpacing;

    _buttons.push_back({
        {x, row2_y, modeBtnW, btnHeight},
        "Full",
        [this]() { if (onFullscreen) onFullscreen(); }
    });

    // Set slider and rating rects
    _durationSliderRect = {margin, (int)(175 * _scale), usableW, (int)(16 * _scale)};
    int halfWidth = (usableW - btnSpacing) / 2;
    _speedSliderRect = {margin, (int)(220 * _scale), halfWidth, (int)(16 * _scale)};
    _beatSliderRect = {margin + halfWidth + btnSpacing, (int)(220 * _scale), halfWidth, (int)(16 * _scale)};
    _ratingRect = {margin, (int)(270 * _scale), usableW, (int)(30 * _scale)};
}

void ControlWindow::render() {
    if (!_renderer) return;

    int margin = BUTTON_MARGIN();
    int s = FONT_SCALE();
    if (s < 1) s = 1;
    int usableW = WINDOW_WIDTH() - 2 * margin;
    int btnSpacing = (int)(8 * _scale);

    // Dark background
    SDL_SetRenderDrawColor(_renderer, 30, 30, 35, 255);
    SDL_RenderClear(_renderer);

    // Colors
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {150, 150, 150, 255};
    SDL_Color yellow = {255, 220, 100, 255};
    SDL_Color green = {100, 200, 100, 255};

    // Draw preset info area
    SDL_SetRenderDrawColor(_renderer, 40, 40, 45, 255);
    SDL_Rect infoRect = {margin, margin, usableW, (int)(55 * _scale)};
    SDL_RenderFillRect(_renderer, &infoRect);

    // Preset name (truncate from beginning if too long, show end of path)
    std::string displayName = _presetName;
    int charWidthPx = 6;
    int availableWidth = usableW - (int)(20 * _scale);
    int maxChars = availableWidth / charWidthPx;
    if ((int)displayName.length() > maxChars) {
        displayName = "..." + displayName.substr(displayName.length() - maxChars + 3);
    }
    renderText(displayName, margin + (int)(8 * _scale), margin + (int)(6 * _scale), white);

    // Second line: Preset index, history, and rating
    std::string infoLine = std::to_string(_currentPreset + 1) + "/" + std::to_string(_totalPresets);
    if (_historyTotal > 0) {
        infoLine += "  H:" + std::to_string(_historyPos) + "/" + std::to_string(_historyTotal);
    }
    infoLine += "  Rating: ";
    for (int i = 1; i <= 9; i++) {
        infoLine += (i <= _rating) ? "*" : ".";
    }
    if (_rating == 9) {
        infoLine += " FAV";
    }
    renderText(infoLine, margin + (int)(8 * _scale), margin + (int)(22 * _scale), _rating == 9 ? yellow : gray);

    // Third line: Active modes
    std::string modes = "";
    if (_shuffleEnabled) modes += "Shuffle ";
    if (_lockEnabled) modes += "Locked ";
    if (_favoritesMode) modes += "FavsOnly ";
    if (_randomDurationMode) modes += "RndTime ";
    if (!modes.empty()) {
        renderText(modes, margin + (int)(8 * _scale), margin + (int)(38 * _scale), green);
    }

    // Render buttons with active state colors
    for (size_t i = 0; i < _buttons.size(); i++) {
        Button& btn = _buttons[i];
        bool isActive = false;

        if (btn.label == "Shuffle" && _shuffleEnabled) isActive = true;
        if (btn.label == "Lock" && _lockEnabled) isActive = true;
        if (btn.label == "Favs" && _favoritesMode) isActive = true;
        if (btn.label == "RndTime" && _randomDurationMode) isActive = true;

        renderButton(btn, isActive);
    }

    // Duration slider
    std::string durationLabel = "Duration: " + std::to_string((int)_duration) + "s";
    if (_randomDurationMode) durationLabel += " (RND)";
    renderText(durationLabel, margin, (int)(150 * _scale), gray);
    renderSlider(_durationSliderRect.x, _durationSliderRect.y, _durationSliderRect.w, _durationSliderRect.h, _duration, 5.0, 120.0);

    // Speed and Beat sliders side by side
    char speedBuf[32], beatBuf[32];
    snprintf(speedBuf, sizeof(speedBuf), "Speed: %.2fx", _speed);
    snprintf(beatBuf, sizeof(beatBuf), "Beat: %.1f", _beatSensitivity);
    renderText(speedBuf, _speedSliderRect.x, (int)(200 * _scale), gray);
    renderText(beatBuf, _beatSliderRect.x, (int)(200 * _scale), gray);
    renderSlider(_speedSliderRect.x, _speedSliderRect.y, _speedSliderRect.w, _speedSliderRect.h, _speed, 0.01, 2.0);
    renderSlider(_beatSliderRect.x, _beatSliderRect.y, _beatSliderRect.w, _beatSliderRect.h, _beatSensitivity, 0.0, 2.0);

    // Rating buttons - full width, 9 equal buttons
    int ratingY = (int)(250 * _scale);
    int ratingBtnW = (usableW - 8 * btnSpacing) / 9;
    int ratingBtnH = (int)(28 * _scale);
    renderText("Rating:", margin, ratingY - (int)(14 * _scale), gray);

    for (int i = 1; i <= 9; i++) {
        int ratingX = margin + (i - 1) * (ratingBtnW + btnSpacing);
        SDL_Rect ratingBox = {ratingX, ratingY, ratingBtnW, ratingBtnH};

        if (i == _rating) {
            SDL_SetRenderDrawColor(_renderer, 80, 160, 80, 255);
        } else if (i == 9) {
            SDL_SetRenderDrawColor(_renderer, 60, 50, 70, 255);  // Fav color hint
        } else {
            SDL_SetRenderDrawColor(_renderer, 45, 45, 55, 255);
        }
        SDL_RenderFillRect(_renderer, &ratingBox);

        SDL_SetRenderDrawColor(_renderer, 80, 80, 90, 255);
        SDL_RenderDrawRect(_renderer, &ratingBox);

        std::string numStr = std::to_string(i);
        int charW = 6 * s;
        int charH = 7 * s;
        int textX = ratingBox.x + (ratingBox.w - charW) / 2;
        int textY = ratingBox.y + (ratingBox.h - charH) / 2;
        SDL_Color numColor = (i == _rating) ? white : ((i == 9) ? yellow : gray);
        renderText(numStr, textX, textY, numColor);
    }

    // Hotkeys - compact
    int hotkeyY = (int)(295 * _scale);
    int lineH = (int)(11 * _scale);
    SDL_Color dimGray = {100, 100, 100, 255};
    renderText("Keys: B/W=History  N/P=Nav  R=Random  1-9=Rate", margin, hotkeyY, dimGray);
    hotkeyY += lineH;
    renderText("Y=Shuf L=Lock F=Fav T=FavMode M=Full C=Panel", margin, hotkeyY, dimGray);
    hotkeyY += lineH;
    renderText("Arrows=Speed S=Slow []=Beat  Cmd+Del=Delete", margin, hotkeyY, dimGray);

    SDL_RenderPresent(_renderer);
}

void ControlWindow::renderButton(const Button& btn, bool isActive) {
    // Button background - green when active, normal otherwise
    if (btn.pressed) {
        SDL_SetRenderDrawColor(_renderer, 80, 140, 200, 255);
    } else if (isActive) {
        SDL_SetRenderDrawColor(_renderer, 60, 130, 80, 255);  // Green for active
    } else if (btn.hovered) {
        SDL_SetRenderDrawColor(_renderer, 70, 70, 80, 255);
    } else {
        SDL_SetRenderDrawColor(_renderer, 50, 50, 60, 255);
    }
    SDL_RenderFillRect(_renderer, &btn.rect);

    // Button border - brighter when active
    if (isActive) {
        SDL_SetRenderDrawColor(_renderer, 100, 180, 100, 255);
    } else {
        SDL_SetRenderDrawColor(_renderer, 100, 100, 110, 255);
    }
    SDL_RenderDrawRect(_renderer, &btn.rect);

    // Button label (centered)
    SDL_Color white = {255, 255, 255, 255};
    int s = FONT_SCALE();
    if (s < 1) s = 1;
    int charWidth = 6 * s;
    int charHeight = 7 * s;
    int textWidth = (int)btn.label.length() * charWidth;
    int textX = btn.rect.x + (btn.rect.w - textWidth) / 2;
    int textY = btn.rect.y + (btn.rect.h - charHeight) / 2;
    renderText(btn.label, textX, textY, white);
}

void ControlWindow::renderButton(const Button& btn) {
    renderButton(btn, false);
}

void ControlWindow::renderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);

    int s = FONT_SCALE();
    if (s < 1) s = 1;

    int cursorX = x;
    for (size_t i = 0; i < text.length(); i++) {
        unsigned char c = text[i];
        if (c < 32 || c > 127) c = '?';
        int idx = c - 32;

        // Draw the character pixel by pixel, scaled
        for (int row = 0; row < 7; row++) {
            unsigned char rowData = font5x7[idx][row];
            for (int col = 0; col < 5; col++) {
                if (rowData & (0x10 >> col)) {
                    SDL_Rect pixel = {cursorX + col * s, y + row * s, s, s};
                    SDL_RenderFillRect(_renderer, &pixel);
                }
            }
        }
        cursorX += 6 * s; // 5px char width + 1px spacing, scaled
    }
}

void ControlWindow::renderSlider(int x, int y, int width, int height, double value, double min, double max) {
    // Slider track
    SDL_SetRenderDrawColor(_renderer, 60, 60, 70, 255);
    SDL_Rect track = {x, y, width, height};
    SDL_RenderFillRect(_renderer, &track);

    // Slider fill
    double normalized = (value - min) / (max - min);
    normalized = std::max(0.0, std::min(1.0, normalized));
    int fillWidth = (int)(width * normalized);
    SDL_SetRenderDrawColor(_renderer, 80, 150, 220, 255);
    SDL_Rect fill = {x, y, fillWidth, height};
    SDL_RenderFillRect(_renderer, &fill);

    // Slider handle
    int handleWidth = (int)(10 * _scale);
    SDL_SetRenderDrawColor(_renderer, 200, 200, 210, 255);
    SDL_Rect handle = {x + fillWidth - handleWidth/2, y - (int)(2 * _scale), handleWidth, height + (int)(4 * _scale)};
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

    SDL_Point mousePoint = {mouseX, mouseY};

    // Define slider areas for hit testing (use stored rects with some extra height for easier clicking)
    SDL_Rect durationArea = {_durationSliderRect.x, _durationSliderRect.y, _durationSliderRect.w, _durationSliderRect.h + (int)(4 * _scale)};
    SDL_Rect speedArea = {_speedSliderRect.x, _speedSliderRect.y, _speedSliderRect.w, _speedSliderRect.h + (int)(4 * _scale)};
    SDL_Rect beatArea = {_beatSliderRect.x, _beatSliderRect.y, _beatSliderRect.w, _beatSliderRect.h + (int)(4 * _scale)};

    switch (event.type) {
        case SDL_MOUSEMOTION:
            // Update button hover states
            for (auto& btn : _buttons) {
                btn.hovered = SDL_PointInRect(&mousePoint, &btn.rect);
            }

            // Handle slider dragging
            if (_draggingDurationSlider) {
                double normalized = (double)(mouseX - durationArea.x) / durationArea.w;
                normalized = std::max(0.0, std::min(1.0, normalized));
                _duration = 5.0 + normalized * (120.0 - 5.0);
                if (onSetDuration) onSetDuration(_duration);
            }
            if (_draggingSpeedSlider) {
                double normalized = (double)(mouseX - speedArea.x) / speedArea.w;
                normalized = std::max(0.0, std::min(1.0, normalized));
                _speed = 0.01f + (float)normalized * (2.0f - 0.01f);
                if (onSetSpeed) onSetSpeed(_speed);
            }
            if (_draggingBeatSlider) {
                double normalized = (double)(mouseX - beatArea.x) / beatArea.w;
                normalized = std::max(0.0, std::min(1.0, normalized));
                _beatSensitivity = (float)normalized * 2.0f;
                if (onSetBeatSensitivity) onSetBeatSensitivity(_beatSensitivity);
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                // Check buttons
                for (auto& btn : _buttons) {
                    if (SDL_PointInRect(&mousePoint, &btn.rect)) {
                        btn.pressed = true;
                    }
                }

                // Check duration slider
                if (SDL_PointInRect(&mousePoint, &durationArea)) {
                    _draggingDurationSlider = true;
                    double normalized = (double)(mouseX - durationArea.x) / durationArea.w;
                    normalized = std::max(0.0, std::min(1.0, normalized));
                    _duration = 5.0 + normalized * (120.0 - 5.0);
                    if (onSetDuration) onSetDuration(_duration);
                }

                // Check speed slider
                if (SDL_PointInRect(&mousePoint, &speedArea)) {
                    _draggingSpeedSlider = true;
                    double normalized = (double)(mouseX - speedArea.x) / speedArea.w;
                    normalized = std::max(0.0, std::min(1.0, normalized));
                    _speed = 0.01f + (float)normalized * (2.0f - 0.01f);
                    if (onSetSpeed) onSetSpeed(_speed);
                }

                // Check beat slider
                if (SDL_PointInRect(&mousePoint, &beatArea)) {
                    _draggingBeatSlider = true;
                    double normalized = (double)(mouseX - beatArea.x) / beatArea.w;
                    normalized = std::max(0.0, std::min(1.0, normalized));
                    _beatSensitivity = (float)normalized * 2.0f;
                    if (onSetBeatSensitivity) onSetBeatSensitivity(_beatSensitivity);
                }

                // Check rating boxes (1-9 only)
                int margin = BUTTON_MARGIN();
                int usableW = WINDOW_WIDTH() - 2 * margin;
                int btnSpacing = (int)(8 * _scale);
                int ratingY = (int)(250 * _scale);
                int ratingBtnW = (usableW - 8 * btnSpacing) / 9;
                int ratingBtnH = (int)(28 * _scale);
                for (int i = 1; i <= 9; i++) {
                    int ratingX = margin + (i - 1) * (ratingBtnW + btnSpacing);
                    SDL_Rect ratingBox = {ratingX, ratingY, ratingBtnW, ratingBtnH};
                    if (SDL_PointInRect(&mousePoint, &ratingBox)) {
                        _rating = i;
                        if (onSetRating) onSetRating(i);
                    }
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                _draggingDurationSlider = false;
                _draggingSpeedSlider = false;
                _draggingBeatSlider = false;

                for (auto& btn : _buttons) {
                    if (btn.pressed && SDL_PointInRect(&mousePoint, &btn.rect)) {
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

void ControlWindow::setHistoryPosition(int current, int total) {
    _historyPos = current;
    _historyTotal = total;
}

void ControlWindow::setRating(int rating) {
    _rating = rating;
}

void ControlWindow::setSpeed(float speed) {
    _speed = speed;
}

void ControlWindow::setBeatSensitivity(float sensitivity) {
    _beatSensitivity = sensitivity;
}

void ControlWindow::setRandomDurationMode(bool enabled) {
    _randomDurationMode = enabled;
}
