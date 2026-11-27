#include <array>
#include <chrono>
#include <gpiod.hpp>

#include "types.hpp"
#include "buttons.hpp"
#include "display.hpp"
#include "modes.hpp"
#include "buzzer.hpp"

int main() {
    auto rpi = gpiod::chip("/dev/gpiochip0");
    auto request = rpi.prepare_request().set_consumer("watch")
    .add_line_settings(
        {DISPLAY_PIN[0], DISPLAY_PIN[1], DISPLAY_PIN[2], DISPLAY_PIN[3], SER, RCLK, SRCLK, BUZZER_PIN},
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

    // Context variables
    ButtonState btn1_state;
    ButtonState btn2_state;
    Mode current_mode = Mode::CLOCK;
    DisplayData display_data;
    auto last_activity = std::chrono::steady_clock::now();
    

    std::array<int, 4> current_time;
    bool alarm_enabled = false;
    bool alarm_armed = true;
    std::array<int, 4> alarm_time = {0, 0, 0, 0};
    int volume_value = 1;

    while (true) {
        ButtonEvent action1 = check_button(BTN_1, btn1_state, request);
        ButtonEvent action2 = check_button(BTN_2, btn2_state, request);

        if (action1 != ButtonEvent::NONE || action2 != ButtonEvent::NONE) {
            last_activity = std::chrono::steady_clock::now();
        }

        ModeContext ctx = {current_mode, action1, action2, display_data, last_activity};
        Mode previous_mode = current_mode;
        current_time = get_current_time();

        if (alarm_enabled && alarm_armed && (current_time == alarm_time)) {
            alarm_armed = false;
            last_activity = std::chrono::steady_clock::now();
            current_mode = Mode::ALARM_RINGING;
        } else if (!alarm_armed && current_time != alarm_time) {
            alarm_armed = true;
        }

        switch (current_mode) {
        case Mode::CLOCK:
            clock(ctx, current_time, alarm_enabled);
            break;

        case Mode::VIEW_VOLUME:
            view_volume(ctx, volume_value, request);
            break;

        case Mode::EDIT_VOLUME:
            edit_volume(ctx, volume_value);
            break;

        case Mode::VIEW_ALARM:
            view_alarm(ctx, alarm_time, alarm_enabled);
            break;

        case Mode::EDIT_ALARM:
            edit_alarm(ctx, alarm_time, alarm_enabled);
            break;

        case Mode::ALARM_RINGING:
            alarm_ringing(ctx, current_time, alarm_enabled, volume_value, request);
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