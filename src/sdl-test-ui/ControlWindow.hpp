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
    std::function<void()> onHistoryBack;
    std::function<void()> onHistoryForward;
    std::function<void()> onDelete;
    std::function<void(double)> onSetDuration;
    std::function<void(float)> onSetSpeed;
    std::function<void(float)> onSetBeatSensitivity;
    std::function<void(int)> onSetRating;
    std::function<void()> onToggleRandomDuration;

    // State updates
    void setPresetName(const std::string& name);
    void setShuffleState(bool enabled);
    void setLockState(bool locked);
    void setFavoritesMode(bool enabled);
    void setDuration(double seconds);
    void setPresetIndex(uint32_t current, uint32_t total);
    void setHistoryPosition(int current, int total);
    void setRating(int rating);
    void setSpeed(float speed);
    void setBeatSensitivity(float sensitivity);
    void setRandomDurationMode(bool enabled);

private:
    void createButtons();
    void renderButton(const Button& btn);
    void renderButton(const Button& btn, bool isActive);
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
    int _historyPos{0};
    int _historyTotal{0};
    int _rating{0};
    float _speed{1.0f};
    float _beatSensitivity{1.0f};
    bool _randomDurationMode{false};

    // Slider state
    bool _draggingDurationSlider{false};
    bool _draggingSpeedSlider{false};
    bool _draggingBeatSlider{false};
    int _draggingRating{0};
    SDL_Rect _durationSliderRect;
    SDL_Rect _speedSliderRect;
    SDL_Rect _beatSliderRect;
    SDL_Rect _ratingRect;

    // Layout constants (base values, multiplied by scale)
    static const int BASE_WINDOW_WIDTH = 420;
    static const int BASE_WINDOW_HEIGHT = 340;
    static const int BASE_BUTTON_HEIGHT = 40;
    static const int BASE_BUTTON_MARGIN = 10;
    static const int BASE_BUTTON_WIDTH = 80;

    // Scale factor (1.0 = normal, 2.0 = double size)
    float _scale{1.5f};

    // Scaled values
    int WINDOW_WIDTH() const { return (int)(BASE_WINDOW_WIDTH * _scale); }
    int WINDOW_HEIGHT() const { return (int)(BASE_WINDOW_HEIGHT * _scale); }
    int BUTTON_HEIGHT() const { return (int)(BASE_BUTTON_HEIGHT * _scale); }
    int BUTTON_MARGIN() const { return (int)(BASE_BUTTON_MARGIN * _scale); }
    int BUTTON_WIDTH() const { return (int)(BASE_BUTTON_WIDTH * _scale); }
    int FONT_SCALE() const { return (int)_scale; }
};
