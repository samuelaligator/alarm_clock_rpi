#pragma once

#include <array>
#include <chrono>
#include <gpiod.hpp>
#include "types.hpp"

bool is_blink_on(int interval_ms = 300);
bool is_timeout(std::chrono::steady_clock::time_point start_time, int duration_ms);

void clock(ModeContext& ctx, std::array<int, 4>& current_time, bool& alarm_enabled);
void view_alarm(ModeContext& ctx, std::array<int, 4>& time, bool& alarm_enabled);
void edit_alarm(ModeContext& ctx, std::array<int, 4>& time, bool& alarm_enabled);
void view_volume(ModeContext& ctx, int volume, gpiod::line_request& request);
void edit_volume(ModeContext& ctx, int& volume);
void alarm_ringing(ModeContext& ctx, std::array<int, 4>& current_time, bool& alarm_enabled, int& volume_value, gpiod::line_request& request);