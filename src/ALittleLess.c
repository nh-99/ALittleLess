/* Created by Christopher Lewicki
 * This work is licensed under the MIT license
 * but please don't sell it :( */

/* TODO: --Fix battery_peek so that battery status is loaded on init rather than just on battery state change */

#include <pebble.h>

static Window *s_MainWindow;
static TextLayer *s_TimeLayer;
static TextLayer *s_DateLayer;
static TextLayer *s_DayLayer;
static TextLayer *s_BlackLayer;
static TextLayer *batteryLayer;
// static char s_batteryBuffer[16] /* I don't know if this is supposed to go here */

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_TimeLayer, buffer);
}

static void update_date(){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char dateBuffer[] = "Jan 31";
    strftime(dateBuffer, sizeof("January 31 1970"),  "%B %e", tick_time); // reserving an extra-large buffer to get around some weird memory issue
    text_layer_set_text(s_DateLayer, dateBuffer);

    static char dayBuffer[] = "Wednesday";
    strftime(dayBuffer, sizeof("Wednesday"), "%A", tick_time);
    text_layer_set_text(s_DayLayer, dayBuffer);
}

//BatteryChargeState charge_state = battery_state_service_peek();
static void battery_handler(BatteryChargeState charge_state){
      static char s_battery_buffer[16];

        if (charge_state.is_charging) {
                snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%++", charge_state.charge_percent);
                  } else {
                          snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
                            }
          text_layer_set_text(batteryLayer, s_battery_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
    update_date();
}

static void mainWindowLoad(Window *window) {
	// create a black base layer that covers the entire screen
	s_BlackLayer = text_layer_create(GRect(0,0,144,168));
	text_layer_set_background_color(s_BlackLayer, GColorBlack);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_BlackLayer));
	
	// creates a white layer over s_BlackLayer to create the pattern, as well as display the time
    s_TimeLayer = text_layer_create(GRect(0,60,144,50));
	text_layer_set_background_color(s_TimeLayer, GColorWhite);
	text_layer_set_text_color(s_TimeLayer, GColorBlack);
	
    // create the date layer. Note that it overlaps slightly with s_TimeLayer. This is a workaround to fix a weird padding issue.
    s_DateLayer = text_layer_create(GRect(0,105,139,34));
    text_layer_set_background_color(s_DateLayer, GColorBlack);
    text_layer_set_text_color(s_DateLayer, GColorWhite);

    // create a clear layer over s_DateLayer to show the day of the week without messing with spacing
    s_DayLayer = text_layer_create(GRect(0,105,139,34));
    text_layer_set_background_color(s_DayLayer, GColorClear);
    text_layer_set_text_color(s_DayLayer, GColorWhite);

    // create a battery layer, I don't know how well this will work, but uh
    // we're gonna try a thing here
    batteryLayer = text_layer_create(GRect(0,32,144,34));
    text_layer_set_background_color(batteryLayer, GColorClear);
    text_layer_set_text_color(batteryLayer, GColorWhite);
    //text_layer_set_text(batteryLayer, "%d%%", charge_state.charge_percent);

    // fun with fonts
	text_layer_set_font(s_TimeLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(s_TimeLayer, GTextAlignmentCenter);

    text_layer_set_font(s_DateLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(s_DateLayer, GTextAlignmentRight);
	
    text_layer_set_font(s_DayLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(s_DayLayer, GTextAlignmentLeft);

    text_layer_set_font(batteryLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(batteryLayer, GTextAlignmentLeft);

    // push things onto the root window, update time on change
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_DateLayer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_TimeLayer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_DayLayer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(batteryLayer));
    update_time();
    update_date();
    battery_state_service_peek();
}

static void mainWindowUnload(Window *window){
	text_layer_destroy(s_TimeLayer);
    text_layer_destroy(s_DateLayer);
    text_layer_destroy(batteryLayer);
    text_layer_destroy(s_BlackLayer);
}

static void init() {
	s_MainWindow = window_create();
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_handler);
    window_set_window_handlers(s_MainWindow, (WindowHandlers) {
		.load = mainWindowLoad,
		.unload = mainWindowUnload
	});
	
    battery_state_service_peek();
	window_stack_push(s_MainWindow, true);
}

static void deinit() {
	window_destroy(s_MainWindow);
    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
