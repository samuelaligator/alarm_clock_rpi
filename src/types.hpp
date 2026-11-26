#pragma once

#include <array>
#include <chrono>

enum class ButtonEvent {
    NONE,
    SHORT_PRESS,
    LONG_PRESS,
    REPEAT
};

enum class Mode {
    CLOCK,
    VIEW_ALARM,
    EDIT_ALARM,
    VIEW_VOLUME,
    EDIT_VOLUME,
    ALARM_RINGING
};

struct ButtonState {
    bool is_pressed = false;
    bool long_triggered = false;
    bool ignore_repeat = false;
    std::chrono::steady_clock::time_point press_time;
};

struct DisplayData {
    std::array<int, 4> digits = {0, 0, 0, 0};
    std::array<bool, 4> dps = {false, false, false, false};
};

struct ModeContext {
    Mode& current_mode;
    ButtonEvent action1;
    ButtonEvent action2;
    DisplayData& display_data;
    std::chrono::steady_clock::time_point last_activity;
};