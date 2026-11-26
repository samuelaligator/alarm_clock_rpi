#include <gpiod.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <chrono>
#include <ctime>
#include <thread>


//GPIO functions
const unsigned int SER = 17;
const unsigned int RCLK = 22;
const unsigned int SRCLK = 27;

const unsigned int BTN_1 = 21;
const unsigned int BTN_2 = 20;

const std::vector<unsigned int> DISPLAY_PIN = {23, 24, 25, 6};


const std::vector<std::vector<int>> SEGMENT_PATTERNS = {
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

enum class ButtonEvent {
    NONE, 
    SHORT_PRESS, 
    LONG_PRESS, 
    REPEAT
};

enum class Mode {
    CLOCK,
    VIEW_ALARM,
    EDIT_ALARM,
    VIEW_VOLUME,
    EDIT_VOLUME,
    ALARM_RINGING
};

struct ButtonState {
    bool is_pressed = false;
    bool long_triggered = false;
    bool ignore_repeat = false;
    std::chrono::steady_clock::time_point press_time;
};

struct DisplayData {
    std::array<int, 4> digits = {0, 0, 0, 0};
    std::array<bool, 4> dps = {false, false, false, false};
};

struct ModeContext {
    Mode& current_mode;
    ButtonEvent action1;
    ButtonEvent action2;
    DisplayData& display_data;
    std::chrono::steady_clock::time_point last_activity;
};

ButtonEvent check_button(unsigned int pin, ButtonState& state, gpiod::line_request& request) {
    auto btn_value = request.get_value(pin);
    auto now = std::chrono::steady_clock::now();

    if (btn_value == gpiod::line::value::INACTIVE) { // INACTIVE = pressed
        if (!state.is_pressed) {
            state.is_pressed = true;
            state.long_triggered = false;
            state.ignore_repeat = false;
            state.press_time = now;
        } 
        else if (!state.long_triggered) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.press_time).count();
            if (elapsed >= 600) {
                state.long_triggered = true;
                state.press_time = now;
                return ButtonEvent::LONG_PRESS;
            }
        } 
        else if (!state.ignore_repeat) {
            auto since_last = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.press_time).count();
            if (since_last >= 150) {
                state.press_time = now;
                return ButtonEvent::REPEAT;
            }
        }
    } 
    else {
        if (state.is_pressed) {
            state.is_pressed = false;
            state.ignore_repeat = false;
            if (!state.long_triggered) {
                return ButtonEvent::SHORT_PRESS;
            }
        }
    }
    return ButtonEvent::NONE;
}

void update_register(unsigned int pin, gpiod::line_request& request) {
    request.set_value(pin, gpiod::line::value::ACTIVE);
    request.set_value(pin, gpiod::line::value::INACTIVE);
}

void display_numbers(const std::array<int, 4>& digits, const std::array<bool, 4>& dps, gpiod::line_request& request) {
    
    for (int d = 0; d < 4; d++) {
        if (dps[d]){
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

bool is_blink_on(int interval_ms = 300) {
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

void edit_alarm(ModeContext& ctx, std::array<int, 4>& time, bool& alarm_enabled){
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
    
     if (!is_timeout(ctx.last_activity, 500) || is_blink_on()){
        ctx.display_data.digits = {time[0], time[1], time[2], time[3]};
    } else{
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


    if (!is_timeout(ctx.last_activity, 500) || is_blink_on()){
        ctx.display_data.digits = {10, 10, 10, volume};
    } else {
        ctx.display_data.digits = {10, 10, 10, 10};
    }   
    ctx.display_data.dps = {false, false, false, false};
}

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
        }

        if (current_mode != previous_mode) {
        btn1_state.ignore_repeat = true;
        btn2_state.ignore_repeat = true;
        }

        display_numbers(display_data.digits, display_data.dps, request);
    }

    return 0;
}