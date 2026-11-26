#pragma once

#include <array>
#include <vector>
#include <gpiod.hpp>

inline constexpr unsigned int SER = 17;
inline constexpr unsigned int RCLK = 22;
inline constexpr unsigned int SRCLK = 27;

inline const std::vector<unsigned int> DISPLAY_PIN = {23, 24, 25, 6};

inline const std::vector<std::vector<int>> SEGMENT_PATTERNS = {
    {0, 0, 0, 0, 0, 0, 1}, // 0
    {1, 0, 0, 1, 1, 1, 1}, // 1
    {0, 0, 1, 0, 0, 1, 0}, // 2
    {0, 0, 0, 0, 1, 1, 0}, // 3
    {1, 0, 0, 1, 1, 0, 0}, // 4
    {0, 1, 0, 0, 1, 0, 0}, // 5
    {0, 1, 0, 0, 0, 0, 0}, // 6
    {0, 0, 0, 1, 1, 1, 1}, // 7
    {0, 0, 0, 0, 0, 0, 0}, // 8
    {0, 0, 0, 0, 1, 0, 0}, // 9
    {1, 1, 1, 1, 1, 1, 1}, // empty (10)
    {0, 1, 0, 0, 1, 0, 0}, // S (11)
    {0, 0, 0, 1, 0, 0, 0}, // A (12)
    {0, 0, 0, 1, 0, 0, 1}  // M (13)
};

void update_register(unsigned int pin, gpiod::line_request& request);
void display_numbers(const std::array<int, 4>& digits, const std::array<bool, 4>& dps, gpiod::line_request& request);