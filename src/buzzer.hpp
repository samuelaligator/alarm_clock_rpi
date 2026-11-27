#pragma once
#include <gpiod.hpp>

inline constexpr unsigned int BUZZER_PIN = 19;

void buzzer_on(gpiod::line_request& request);
void buzzer_off(gpiod::line_request& request);