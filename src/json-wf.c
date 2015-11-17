#include "pebble.h"
#include "colors.h"

static Window *s_main_window;
static TextLayer *s_json_time_layer;
static TextLayer *s_json_date_layer;
static TextLayer *s_json_battery_layer;
static TextLayer *s_json_bt_layer;
static TextLayer *s_json_end_layer;
static GFont s_wf_font;

static void update_battery(BatteryChargeState charge_state) {
  char s_battery_buffer[] = "100";
  static char s_json_battery_buffer[14];

  if (charge_state.is_charging) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "\"~\"");
  } else {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d", charge_state.charge_percent);
  }

  snprintf(s_json_battery_buffer, sizeof(s_json_battery_buffer), "  \"ch\": %s,", s_battery_buffer);

  text_layer_set_text(s_json_battery_layer, s_json_battery_buffer);
}

static void update_bluetooth(bool connected) {
  static char s_json_bt_buffer[14];

  snprintf(s_json_bt_buffer, sizeof(s_json_bt_buffer), "  \"bt\": %s", connected ? "true" : "false");

  text_layer_set_text(s_json_bt_layer, s_json_bt_buffer);
}

static void update_time(struct tm *tick_time, TimeUnits units_changed) {
  char s_hrs_buffer[11];
  char s_min_buffer[11];
  char s_week_day_buffer[12];
  char s_day_buffer[11];
  char s_month_buffer[12];

  static char s_json_time_buffer[37];
  static char s_json_date_buffer[52];

  if (clock_is_24h_style() == true) {
    strftime(s_hrs_buffer, sizeof(s_hrs_buffer), "\"%%H\": \"%H\"", tick_time);
  } else {
    strftime(s_hrs_buffer, sizeof(s_hrs_buffer), "\"%%I\": \"%I\"", tick_time);
  }

  strftime(s_min_buffer, sizeof(s_min_buffer), "\"%%M\": \"%M\"", tick_time);
  strftime(s_week_day_buffer, sizeof(s_week_day_buffer), "\"%%a\": \"%a\"", tick_time);
  strftime(s_day_buffer, sizeof(s_day_buffer), "\"%%d\": \"%d\"", tick_time);
  strftime(s_month_buffer, sizeof(s_month_buffer), "\"%%b\": \"%b\"", tick_time);

  snprintf(
    s_json_time_buffer,
    sizeof(s_json_time_buffer),
    "{\n  %s,\n  %s,\n",
    s_hrs_buffer,
    s_min_buffer
  );

  snprintf(
    s_json_date_buffer,
    sizeof(s_json_date_buffer),
    "  %s,\n  %s,\n  %s,\n",
    s_week_day_buffer,
    s_day_buffer,
    s_month_buffer
  );

  text_layer_set_text(s_json_time_layer, s_json_time_buffer);
  text_layer_set_text(s_json_date_layer, s_json_date_buffer);

  update_battery(battery_state_service_peek());
}

static void main_window_load(Window *window) {
  window_set_background_color(s_main_window, GColorBlack);

  s_wf_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_VERA_MONO_BOLD_16));

  s_json_time_layer = text_layer_create(GRect(0, 8, 144, 51));
  text_layer_set_background_color(s_json_time_layer, GColorClear);
  text_layer_set_text_color(s_json_time_layer, COLOR_TIME);
  text_layer_set_font(s_json_time_layer, s_wf_font);

  s_json_date_layer = text_layer_create(GRect(0, 56, 144, 51));
  text_layer_set_background_color(s_json_date_layer, GColorClear);
  text_layer_set_text_color(s_json_date_layer, COLOR_DATE);
  text_layer_set_font(s_json_date_layer, s_wf_font);

  s_json_battery_layer = text_layer_create(GRect(0, 104, 144, 19));
  text_layer_set_background_color(s_json_battery_layer, GColorClear);
  text_layer_set_text_color(s_json_battery_layer, COLOR_SYSTEM);
  text_layer_set_font(s_json_battery_layer, s_wf_font);

  s_json_bt_layer = text_layer_create(GRect(0, 120, 144, 19));
  text_layer_set_background_color(s_json_bt_layer, GColorClear);
  text_layer_set_text_color(s_json_bt_layer, COLOR_SYSTEM);
  text_layer_set_font(s_json_bt_layer, s_wf_font);

  s_json_end_layer = text_layer_create(GRect(0, 136, 144, 19));
  text_layer_set_background_color(s_json_end_layer, GColorClear);
  text_layer_set_text_color(s_json_end_layer, COLOR_TIME);
  text_layer_set_font(s_json_end_layer, s_wf_font);
  text_layer_set_text(s_json_end_layer, "}");

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  update_time(current_time, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, update_time);

  battery_state_service_subscribe(update_battery);

  update_bluetooth(bluetooth_connection_service_peek());
  bluetooth_connection_service_subscribe(update_bluetooth);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_json_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_json_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_json_battery_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_json_bt_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_json_end_layer));
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  text_layer_destroy(s_json_time_layer);
  text_layer_destroy(s_json_date_layer);
  text_layer_destroy(s_json_battery_layer);
  text_layer_destroy(s_json_bt_layer);
  text_layer_destroy(s_json_end_layer);

  fonts_unload_custom_font(s_wf_font);
}

static void init() {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, false);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
