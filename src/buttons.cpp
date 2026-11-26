#include "buttons.hpp"

ButtonEvent check_button(unsigned int pin, ButtonState& state, gpiod::line_request& request) {
    auto btn_value = request.get_value(pin);
    auto now = std::chrono::steady_clock::now();

    if (btn_value == gpiod::line::value::INACTIVE) { // INACTIVE = pressed
        if (!state.is_pressed) {
            state.is_pressed = true;
            state.long_triggered = false;
            state.ignore_repeat = false;
            state.press_time = now;
        }
        else if (!state.long_triggered) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.press_time).count();
            if (elapsed >= 600) {
                state.long_triggered = true;
                state.press_time = now;
                return ButtonEvent::LONG_PRESS;
            }
        }
        else if (!state.ignore_repeat) {
            auto since_last = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.press_time).count();
            if (since_last >= 150) {
                state.press_time = now;
                return ButtonEvent::REPEAT;
            }
        }
    }
    else {
        if (state.is_pressed) {
            state.is_pressed = false;
            state.ignore_repeat = false;
            if (!state.long_triggered) {
                return ButtonEvent::SHORT_PRESS;
            }
        }
    }
    return ButtonEvent::NONE;
}