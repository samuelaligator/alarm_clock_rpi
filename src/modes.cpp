#include "modes.hpp"
#include "buzzer.hpp"
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

void clock(ModeContext& ctx, std::array<int, 4>& current_time, bool& alarm_enabled) {
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

    ctx.display_data.digits = current_time;
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

void view_volume(ModeContext& ctx, int volume, gpiod::line_request& request) {
    if (ctx.action1 == ButtonEvent::SHORT_PRESS || ctx.action2 == ButtonEvent::SHORT_PRESS || is_timeout(ctx.last_activity, 4000)) {
        ctx.current_mode = Mode::CLOCK;
        buzzer_off(request);
        return;
    } else if (ctx.action2 == ButtonEvent::LONG_PRESS) {
        ctx.current_mode = Mode::EDIT_VOLUME;
        buzzer_off(request);
        return;
    }

    if (volume == 1) {
        buzzer_on(request);
    } else {
        buzzer_off(request);
    }

    ctx.display_data.digits = {10, 10, 10, volume};
    ctx.display_data.dps = {false, false, false, false};
}

void edit_volume(ModeContext& ctx, int& volume) {
    if (is_timeout(ctx.last_activity, 4000)) {
        ctx.current_mode = Mode::CLOCK;
        return;
    } else if (ctx.action1 == ButtonEvent::SHORT_PRESS || ctx.action1 == ButtonEvent::REPEAT) {
        volume = (volume - 1 + 2) % 2;
    } else if (ctx.action2 == ButtonEvent::SHORT_PRESS || ctx.action2 == ButtonEvent::REPEAT) {
        volume = (volume + 1) % 2;
    }

    if (!is_timeout(ctx.last_activity, 500) || is_blink_on()) {
        ctx.display_data.digits = {10, 10, 10, volume};
    } else {
        ctx.display_data.digits = {10, 10, 10, 10};
    }
    ctx.display_data.dps = {false, false, false, false};
}

void alarm_ringing(ModeContext& ctx, std::array<int, 4>& current_time, bool& alarm_enabled, int& volume_value, gpiod::line_request& request) {
    if (ctx.action1 == ButtonEvent::SHORT_PRESS || ctx.action2 == ButtonEvent::SHORT_PRESS || is_timeout(ctx.last_activity, 180000)) {
        ctx.current_mode = Mode::CLOCK;
        buzzer_off(request);
        return;
    }

    if (is_blink_on(500)) {
        ctx.display_data.digits = {current_time[0], current_time[1], current_time[2], current_time[3]};
        volume_value == 1 ? buzzer_on(request) : buzzer_off(request);
    } 
    else {
        ctx.display_data.digits = {10, 10, 10, 10};
        buzzer_off(request);
    }
    ctx.display_data.dps = {false, true, false, alarm_enabled};
}