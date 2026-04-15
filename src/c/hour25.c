//***********************************************************************************************100

#include <pebble.h>
#include <time.h>

#include "common.h"

//****************************************************************************80

static struct EdoUi
{
    Window* window;
    Layer* path_layer;
    //TextLayer* rise_text;
    TextLayer* time_text;
    TextLayer* info_text;
    TextLayer* battery_text;

    GBitmap* image;
    BitmapLayer* img_layer;

    AppTimer* timer;
    time_t last_js_request_time;
} ui;

//****************************************************************************80
// functions 

static void bell(int count)
{
    // Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
    static const uint32_t segments[] = { 200, 100, 400 };
    VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);
}

/*
tk = s
t = s/k
0.96 = k
*/
static char* currentTime()
{
    const time_t t = time(NULL);
    struct tm* lt = localtime(&t);
    static char text[] = "00:00";

    int seconds_now = (((lt->tm_hour * 60) + lt->tm_min) * 60) + lt->tm_sec;
    int ticks_now = seconds_now / 0.96;

    int minutes_25 = ticks_now / 60;
    int hours_25 = minutes_25 / 60;
    int minutes_25_mod = minutes_25 % 60;

    snprintf(text, sizeof(text), "%02d:%02d",
        clamp(hours_25, 0, 23),
        clamp(minutes_25_mod, 0, 59));

    return text;
}

static int currentBatteryValue()
{
    BatteryChargeState bcs = battery_state_service_peek();
    int cp = (int)bcs.charge_percent;
    return cp;
}

static char* currentBattery()
{
    int cp_value = currentBatteryValue();
    static char per[] = "100x";
    snprintf(per, sizeof(per), "%02d%%", clamp(cp_value, 0, 100));
    
    return per;
}

static char* currentDate()
{
    const time_t t = time(NULL);
    struct tm* lt = localtime(&t);
    static char text[] = "00000-00-00";

    snprintf(text, 2*sizeof(text), "%05d-%02d-%02d",
        lt->tm_year % 100 + 12000,
        lt->tm_mon + 1,
        lt->tm_mday);

    return text;
}

//****************************************************************************80
// timed events 

static void timer_callback(void* data)
{
    time_t current_time = time(NULL);
    struct tm* lt = localtime(&current_time);
    int min = lt->tm_min;
    if (current_time - ui.last_js_request_time >= (10 * 60))
    { // 10 minute interval
        if (min == 0)
        {
            bell(min);
        }
        ui.last_js_request_time = current_time;
    }

    text_layer_set_text(ui.time_text, currentTime());

    static char info[] = "12026-88-88";
    char* cd = currentDate();
    snprintf(info, sizeof(info), "%s", cd);
    text_layer_set_text(ui.info_text, info);

    char* cb = currentBattery();
    text_layer_set_text(ui.battery_text, cb);

    int cbv = currentBatteryValue();
    if (cbv < 10) {
        text_layer_set_text_color(ui.battery_text, GColorRed);
    } else if (cbv < 20) {
        text_layer_set_text_color(ui.battery_text, GColorYellow);
    }

    //calculate the amount of time it task to move one degree, then do it twice
    int durration = 30;
    int ms = durration * 1000;

    ui.timer = app_timer_register(ms, timer_callback, NULL);
}

//****************************************************************************80
// window events 

// called to setup the window's layers
static void window_load(Window *window)
{
    Layer* window_layer = window_get_root_layer(ui.window);
    GRect bounds = layer_get_bounds(window_layer);

    /**********************************/
    // time layer
    GRect text_time_bounds = (GRect)
    {
        .origin = { 0, bounds.size.h/2 - 49},
        .size = { bounds.size.w, 50 }
    };
    ui.time_text = text_layer_create(text_time_bounds);
    text_layer_set_font(ui.time_text ,fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_alignment(ui.time_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(ui.time_text));

    /**********************************/
    //info layer
    GRect text_info_bounds = (GRect)
    {
        .origin = { 0,(bounds.size.h)/2+0 },
        .size = {bounds.size.w, 30}
    };
    ui.info_text = text_layer_create(text_info_bounds);
    text_layer_set_font(ui.info_text,fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_alignment(ui.info_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(ui.info_text));
    text_layer_set_text(ui.info_text, "load");

    /**********************************/
    //info layer
    GRect text_battery_bounds = (GRect)
    {
        .origin = { 0,(bounds.size.h)/2+28 },
        .size = {bounds.size.w, 28}
    };
    ui.battery_text = text_layer_create(text_battery_bounds);
    text_layer_set_font(ui.battery_text,fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_alignment(ui.battery_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(ui.battery_text));
    text_layer_set_text(ui.battery_text, "load");
}

static void window_appear(Window* win)
{
    text_layer_set_text(ui.time_text, "00:00");
    ui.timer = app_timer_register(10, timer_callback, NULL);
}

static void window_unload(Window* w)
{
    text_layer_destroy(ui.time_text);
    text_layer_destroy(ui.info_text);
    text_layer_destroy(ui.battery_text);

    app_timer_cancel(ui.timer);

    //bitmap_layer_destroy(ui.img_layer);
}

//****************************************************************************80


/** called by framework to start process */
void hour_25_init(Window *window)
{
    ui.window = window_create();
    window_set_window_handlers(ui.window, (WindowHandlers)
    {
        .load = window_load
        , .unload = window_unload
        , .appear = window_appear
    });

    window_stack_pop(false);
    window_stack_push(ui.window, true);
}
