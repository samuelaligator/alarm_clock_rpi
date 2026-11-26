#include "modes.hpp"
#include <chrono>
#include <ctime>


bool is_blink_on(int interval_ms) {
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    return (now_ms / interval_ms) % 2 == 0;
}

bool is_timeout(std::chrono::steady_clock::time_point start_time, int duration_ms) {
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time
    ).count();
    return elapsed_ms >= duration_ms;
}

void clock(ModeContext& ctx, bool& alarm_enabled) {
    if (ctx.action1 == ButtonEvent::SHORT_PRESS) {
        ctx.current_mode = Mode::VIEW_ALARM;
        return;
    } else if (ctx.action1 == ButtonEvent::LONG_PRESS) {
        ctx.current_mode = Mode::EDIT_ALARM;
        return;
    } else if (ctx.action2 == ButtonEvent::SHORT_PRESS) {
        ctx.current_mode = Mode::VIEW_VOLUME;
        return;
    } else if (ctx.action2 == ButtonEvent::LONG_PRESS) {
        ctx.current_mode = Mode::EDIT_VOLUME;
        return;
    }

    std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* local_time = std::localtime(&current_time);

    int hours = local_time->tm_hour;
    int minutes = local_time->tm_min;

    ctx.display_data.digits[0] = hours / 10;
    ctx.display_data.digits[1] = hours % 10;
    ctx.display_data.digits[2] = minutes / 10;
    ctx.display_data.digits[3] = minutes % 10;

    ctx.display_data.dps = {false, true, false, alarm_enabled};
}

void view_alarm(ModeContext& ctx, std::array<int, 4>& time, bool& alarm_enabled) {
    if (ctx.action1 == ButtonEvent::SHORT_PRESS) {
        alarm_enabled = !alarm_enabled;
    } else if (ctx.action2 == ButtonEvent::SHORT_PRESS || is_timeout(ctx.last_activity, 4000)) {
        ctx.current_mode = Mode::CLOCK;
        return;
    } else if (ctx.action1 == ButtonEvent::LONG_PRESS) {
        ctx.current_mode = Mode::EDIT_ALARM;
        return;
    }

    ctx.display_data.digits = {time[0], time[1], time[2], time[3]};
    ctx.display_data.dps = {false, false, false, alarm_enabled};
}

void edit_alarm(ModeContext& ctx, std::array<int, 4>& time, bool& alarm_enabled) {
    if (is_timeout(ctx.last_activity, 4000)) {
        ctx.current_mode = Mode::CLOCK;
        return;
    } else if (ctx.action1 == ButtonEvent::SHORT_PRESS || ctx.action1 == ButtonEvent::REPEAT) {
        int hour = time[0] * 10 + time[1];
        hour = (hour + 1) % 24;
        time[0] = hour / 10;
        time[1] = hour % 10;
    } else if (ctx.action2 == ButtonEvent::SHORT_PRESS || ctx.action2 == ButtonEvent::REPEAT) {
        int min = time[2] * 10 + time[3];
        min = (min + 1) % 60;
        time[2] = min / 10;
        time[3] = min % 10;
    }

    if (!is_timeout(ctx.last_activity, 500) || is_blink_on()) {
        ctx.display_data.digits = {time[0], time[1], time[2], time[3]};
    } else {
        ctx.display_data.digits = {10, 10, 10, 10};
    }
    ctx.display_data.dps = {false, false, false, alarm_enabled};
}

void view_volume(ModeContext& ctx, int volume) {
    if (ctx.action1 == ButtonEvent::SHORT_PRESS || ctx.action2 == ButtonEvent::SHORT_PRESS || is_timeout(ctx.last_activity, 4000)) {
        ctx.current_mode = Mode::CLOCK;
        return;
    } else if (ctx.action2 == ButtonEvent::LONG_PRESS) {
        ctx.current_mode = Mode::EDIT_VOLUME;
    }

    ctx.display_data.digits = {10, 10, 10, volume};
    ctx.display_data.dps = {false, false, false, false};
}

void edit_volume(ModeContext& ctx, int& volume) {
    if (is_timeout(ctx.last_activity, 4000)) {
        ctx.current_mode = Mode::CLOCK;
        return;
    } else if (ctx.action1 == ButtonEvent::SHORT_PRESS || ctx.action1 == ButtonEvent::REPEAT) {
        volume = (volume - 1 + 10) % 10;
    } else if (ctx.action2 == ButtonEvent::SHORT_PRESS || ctx.action2 == ButtonEvent::REPEAT) {
        volume = (volume + 1) % 10;
    }

    if (!is_timeout(ctx.last_activity, 500) || is_blink_on()) {
        ctx.display_data.digits = {10, 10, 10, volume};
    } else {
        ctx.display_data.digits = {10, 10, 10, 10};
    }
    ctx.display_data.dps = {false, false, false, false};
}