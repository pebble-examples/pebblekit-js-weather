#include "pebble.h"

static Window *s_main_window;

static TextLayer *s_temperature_layer;
static TextLayer *s_city_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;

static AppSync s_sync;
static uint8_t s_sync_buffer[64];

typedef enum {
  WeatherKeyIcon = 0,
  WeatherKeyTemperature,
  WeatherKeyCity,       
} WeatherKey;

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN,    // 0
  RESOURCE_ID_IMAGE_CLOUD,  // 1
  RESOURCE_ID_IMAGE_RAIN,   // 2
  RESOURCE_ID_IMAGE_SNOW    // 3
};

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case WeatherKeyIcon:
      if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }
      s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
      break;

    case WeatherKeyTemperature:
      // App Sync keeps new_tuple in s_sync_buffer, so we may use it directly
      text_layer_set_text(s_temperature_layer, new_tuple->value->cstring);
      break;

    case WeatherKeyCity:
      text_layer_set_text(s_city_layer, new_tuple->value->cstring);
      break;
  }
}

static void request_weather(void) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  const int dummy_key = 1;
  const int dummy_value = 1;
  dict_write_int(iter, dummy_key, &dummy_value, sizeof(int), true);

  dict_write_end(iter);
  app_message_outbox_send();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_icon_layer = bitmap_layer_create(GRect(0, 10, bounds.size.w, 80));
#ifdef PBL_SDK_3
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
#endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_temperature_layer = text_layer_create(GRect(0, 90, bounds.size.w, 32));
  text_layer_set_text_color(s_temperature_layer, PBL_IF_COLOR_ELSE(gcolor_legible_over(GColorIndigo), GColorWhite));
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));

  s_city_layer = text_layer_create(GRect(0, 122, bounds.size.w, 32));
  text_layer_set_text_color(s_city_layer, PBL_IF_COLOR_ELSE(gcolor_legible_over(GColorIndigo), GColorWhite));
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_city_layer));

  Tuplet initial_values[] = {
    TupletInteger(WeatherKeyIcon, (uint8_t) 1),
    TupletCString(WeatherKeyTemperature, "1234\u00B0C"),
    TupletCString(WeatherKeyCity, "St Pebblesburg"),
  };

  app_message_open(64, 64);
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  request_weather();
}

static void window_unload(Window *window) {
  if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }

  text_layer_destroy(s_city_layer);
  text_layer_destroy(s_temperature_layer);
  bitmap_layer_destroy(s_icon_layer);
}

static void init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorIndigo, GColorBlack));

#ifdef PBL_SDK_2
  window_set_fullscreen(s_main_window, true);
#endif

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(s_main_window, true);

  app_message_open(64, 64);
}

static void deinit(void) {
  window_destroy(s_main_window);

  app_sync_deinit(&s_sync);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
