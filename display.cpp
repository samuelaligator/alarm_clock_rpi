#include "display.hpp"
#include <thread>
#include <chrono>

void update_register(unsigned int pin, gpiod::line_request& request) {
    request.set_value(pin, gpiod::line::value::ACTIVE);
    request.set_value(pin, gpiod::line::value::INACTIVE);
}

void display_numbers(const std::array<int, 4>& digits, const std::array<bool, 4>& dps, gpiod::line_request& request) {
    for (int d = 0; d < 4; d++) {
        if (dps[d]) {
            request.set_value(SER, gpiod::line::value::INACTIVE);
        }
        else {
            request.set_value(SER, gpiod::line::value::ACTIVE);
        }
        update_register(SRCLK, request);

        for (int i = 6; i >= 0; i--) {
            if (SEGMENT_PATTERNS[digits[d]][i] == 1) {
                request.set_value(SER, gpiod::line::value::ACTIVE);
            }
            else {
                request.set_value(SER, gpiod::line::value::INACTIVE);
            }
            update_register(SRCLK, request);
        }
        update_register(RCLK, request);

        request.set_value(DISPLAY_PIN[d], gpiod::line::value::ACTIVE);
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
        request.set_value(DISPLAY_PIN[d], gpiod::line::value::INACTIVE);
    }
}