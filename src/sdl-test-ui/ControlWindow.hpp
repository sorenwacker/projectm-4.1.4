/**
 * ControlWindow - Separate SDL2 window for projectM controls
 */

#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include <functional>

struct Button {
    SDL_Rect rect;
    std::string label;
    std::function<void()> onClick;
    bool hovered{false};
    bool pressed{false};
};

class ControlWindow {
public:
    ControlWindow();
    ~ControlWindow();

    bool init(int x = SDL_WINDOWPOS_UNDEFINED, int y = SDL_WINDOWPOS_UNDEFINED);
    void close();
    void render();
    void handleEvent(SDL_Event& event);
    bool isOpen() const { return _window != nullptr; }
    Uint32 getWindowID() const { return _windowID; }

    // Callbacks for control actions
    std::function<void()> onPrev;
    std::function<void()> onNext;
    std::function<void()> onRandom;
    std::function<void()> onToggleShuffle;
    std::function<void()> onToggleLock;
    std::function<void()> onToggleFavorites;
    std::function<void()> onAddFavorite;
    std::function<void()> onFullscreen;
    std::function<void(double)> onSetDuration;

    // State updates
    void setPresetName(const std::string& name);
    void setShuffleState(bool enabled);
    void setLockState(bool locked);
    void setFavoritesMode(bool enabled);
    void setDuration(double seconds);
    void setPresetIndex(uint32_t current, uint32_t total);

private:
    void createButtons();
    void renderButton(const Button& btn);
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void renderSlider(int x, int y, int width, int height, double value, double min, double max);

    SDL_Window* _window{nullptr};
    SDL_Renderer* _renderer{nullptr};
    Uint32 _windowID{0};

    std::vector<Button> _buttons;

    // State
    std::string _presetName{"No preset"};
    bool _shuffleEnabled{true};
    bool _lockEnabled{false};
    bool _favoritesMode{false};
    double _duration{30.0};
    uint32_t _currentPreset{0};
    uint32_t _totalPresets{0};

    // Slider state
    bool _draggingSlider{false};
    SDL_Rect _sliderRect;

    // Layout constants
    static const int WINDOW_WIDTH = 400;
    static const int WINDOW_HEIGHT = 300;
    static const int BUTTON_HEIGHT = 40;
    static const int BUTTON_MARGIN = 10;
    static const int BUTTON_WIDTH = 80;
};
