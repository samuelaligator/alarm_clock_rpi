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

inline std::array<int, 4> get_current_time() {
    std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* local_time = std::localtime(&current_time);

    int hours = local_time->tm_hour;
    int minutes = local_time->tm_min;

    return {
        hours / 10,
        hours % 10,
        minutes / 10,
        minutes % 10
    };
}