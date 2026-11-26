#include <array>
#include <chrono>
#include <gpiod.hpp>

#include "types.hpp"
#include "buttons.hpp"
#include "display.hpp"
#include "modes.hpp"

int main() {
    auto rpi = gpiod::chip("/dev/gpiochip0");
    auto request = rpi.prepare_request().set_consumer("watch")
    .add_line_settings(
        {DISPLAY_PIN[0], DISPLAY_PIN[1], DISPLAY_PIN[2], DISPLAY_PIN[3], SER, RCLK, SRCLK},
        gpiod::line_settings()
            .set_direction(gpiod::line::direction::OUTPUT)
            .set_output_value(gpiod::line::value::INACTIVE)
    )
    .add_line_settings(
        {BTN_1, BTN_2},
        gpiod::line_settings()
            .set_direction(gpiod::line::direction::INPUT)
            .set_bias(gpiod::line::bias::PULL_UP)
    )
    .do_request();

    ButtonState btn1_state;
    ButtonState btn2_state;
    Mode current_mode = Mode::CLOCK;
    DisplayData display_data;
    auto last_activity = std::chrono::steady_clock::now();

    bool alarm_enabled = false;
    std::array<int, 4> alarm_value = {0, 0, 0, 0};
    int volume_value = 6;

    while (true) {
        ButtonEvent action1 = check_button(BTN_1, btn1_state, request);
        ButtonEvent action2 = check_button(BTN_2, btn2_state, request);

        if (action1 != ButtonEvent::NONE || action2 != ButtonEvent::NONE) {
            last_activity = std::chrono::steady_clock::now();
        }

        ModeContext ctx = {current_mode, action1, action2, display_data, last_activity};
        Mode previous_mode = current_mode;

        switch (current_mode) {
        case Mode::CLOCK:
            clock(ctx, alarm_enabled);
            break;

        case Mode::VIEW_VOLUME:
            view_volume(ctx, volume_value);
            break;

        case Mode::EDIT_VOLUME:
            edit_volume(ctx, volume_value);
            break;

        case Mode::VIEW_ALARM:
            view_alarm(ctx, alarm_value, alarm_enabled);
            break;

        case Mode::EDIT_ALARM:
            edit_alarm(ctx, alarm_value, alarm_enabled);
            break;

        case Mode::ALARM_RINGING:
            break;
        }

        if (current_mode != previous_mode) {
            btn1_state.ignore_repeat = true;
            btn2_state.ignore_repeat = true;
        }

        display_numbers(display_data.digits, display_data.dps, request);
    }

    return 0;
}