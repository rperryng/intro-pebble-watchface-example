#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_step_layer;

#if defined(PBL_HEALTH)
static void update_steps() {
  HealthMetric metric = HealthMetricStepCount;

  time_t start = time_start_of_today();
  time_t end = time(NULL);

  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);

  if (mask & HealthServiceAccessibilityMaskAvailable) {
    static char steps_text[sizeof("steps: 99999")];
    snprintf(steps_text, sizeof(steps_text), "steps %d", (int) health_service_sum_today(metric));
    text_layer_set_text(s_step_layer, steps_text);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Data unavailable!");
  }
}
#endif

#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
  switch (event) {
    case HealthEventSignificantUpdate:
      // No implementation
      break;

    case HealthEventMovementUpdate:
      update_steps();
      break;

    case HealthEventSleepUpdate:
      // No implementatio
      break;
  }
}
#endif

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

#if defined(PBL_HEALTH)
  update_steps();
#endif
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_time_layer = text_layer_create(GRect(0, 58, bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorJazzberryJam);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_layer, "00:00");

  s_step_layer = text_layer_create(GRect(20, 38, bounds.size.w, 30));
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_text_color(s_step_layer, GColorBlack);
  text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentLeft);
  text_layer_set_text(s_step_layer, "steps  ");

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_step_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_step_layer);
}

static void init() {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

#if defined(PBL_HEALTH)
  if (!health_service_events_subscribe(health_handler, NULL)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }

#else
  APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
#endif

  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
