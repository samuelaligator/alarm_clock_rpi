#pragma once

#include <gpiod.hpp>
#include "types.hpp"

inline constexpr unsigned int BTN_1 = 21;
inline constexpr unsigned int BTN_2 = 20;

ButtonEvent check_button(unsigned int pin, ButtonState& state, gpiod::line_request& request);