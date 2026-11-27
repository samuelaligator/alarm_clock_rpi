#include "buzzer.hpp"

void buzzer_on(gpiod::line_request& request) {
    request.set_value(BUZZER_PIN, gpiod::line::value::ACTIVE);
}

void buzzer_off(gpiod::line_request& request) {
    request.set_value(BUZZER_PIN, gpiod::line::value::INACTIVE);
}