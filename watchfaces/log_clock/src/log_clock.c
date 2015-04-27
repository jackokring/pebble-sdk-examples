#include "pebble.h"

static double log(double x) {
        double mul;
        double acc;
        double tmp;
        acc = x = (mul = (x-1)/(x+1));
        x *= x;
        for(int32_t i = 3;i < 256;i+=2) {
	    tmp = mul/i;
	    if(tmp < 0.000001) break;
            acc += tmp;
            mul *= x;
        }
        return 2*acc;//an accurate log
}

static Window *s_main_window;
static TextLayer *s_log_layer, *s_time_layer, *s_0_layer, *s_1_layer, *s_2_layer, *s_eight_layer;
static Layer *s_line_layer;

static void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    static char s_time_text[] = "00:00";
    char *time_format;
    if (clock_is_24h_style()) {
      time_format = "%R";
    } else {
      time_format = "%I:%M";
    }
    strftime(s_time_text, sizeof(s_time_text), time_format, tick_time);
    // Handle lack of non-padded hour format string for twelve hour clock.
    if (!clock_is_24h_style() && (s_time_text[0] == '0')) {
      memmove(s_time_text, &s_time_text[1], sizeof(s_time_text) - 1);
    }
    text_layer_set_text(s_time_layer, s_time_text);
}

static int round_it(double s) {
  int i = (int)s;
  s -= i;
  int v = (int)(s*10000)%10000;//converted
  return v;
}

static char s_long_text[] = " GPS CONNECT ";//13 per line
static char s_lat_text[] = " GPS CONNECT ";//13 per line

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
    // Need to be static because they're used by the system later.
    if(units_changed & MINUTE_UNIT) handle_minute_tick(tick_time, units_changed);
    static char s_log_text[] = "1234567890123";//13 per line
    static char s_eight_text[] = "1234567890123";//13 per line
    double s = (double)(tick_time->tm_sec);//get seconds
    double j = ((double)(tick_time->tm_min&3)) / 4.0;//4 min cycle
    double p = s + j;//0.25 increments
    if(p < 1) p += 60.0;//prevent errors by wrap
    double log60 = 1/log(60);
    s = log(p) * log60;//scale to base 60
    double plus = log(p + 0.125) * log60;
    if(p - 0.125 < 1) p += 60;
    double minus = log(p - 0.125) * log60;
    int v = round_it(s);
    int w = round_it(p / 100.0);
    snprintf(s_log_text, sizeof(s_log_text), "%04d LOG %04d",w , v);
    //insert log
    v = round_it(plus);
    w = round_it(minus);
    snprintf(s_eight_text, sizeof(s_eight_text), "%04d -/+ %04d",w , v);

    text_layer_set_text(s_log_layer, s_log_text);//log
    char * longlat = ((tick_time->tm_sec & 4)==0)?s_lat_text:s_long_text;
    //extra layers of 13 chars
    text_layer_set_text(s_0_layer, longlat);//gps co-ordinates? alternating N/W
    text_layer_set_text(s_1_layer, s_log_text);
    text_layer_set_text(s_2_layer, s_log_text);
    text_layer_set_text(s_eight_layer, s_eight_text);// 1/8th offset giving log
}

#define LAT 1
#define LONG 0

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void received_callback(DictionaryIterator *iterator, void *context) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch (t->key) {
      case LAT:
	memcpy(s_lat_text, (char *)t->value->cstring, sizeof(s_lat_text));
	APP_LOG(APP_LOG_LEVEL_INFO, s_lat_text, 0);
        break;
      case LONG:
        memcpy(s_long_text, (char *)t->value->cstring, sizeof(s_long_text));
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static Layer *window_layer;

static void layer(TextLayer **ptr, int offset) {
  *ptr = text_layer_create(GRect(4, offset * 24, 144 - 8, 100));
  text_layer_set_text_color(*ptr, GColorWhite);
  text_layer_set_background_color(*ptr, GColorClear);
  text_layer_set_font(*ptr, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(*ptr));
}

static void main_window_load(Window *window) {
  window_layer = window_get_root_layer(window);

  layer(&s_0_layer, 0);
  layer(&s_1_layer, 1);
  layer(&s_2_layer, 2);
  layer(&s_eight_layer, 3);

  layer(&s_log_layer, 4);

  s_time_layer = text_layer_create(GRect(8, 115, 144 - 16, 76));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  GRect line_frame = GRect(8, 120, 144 - 16, 2);
  s_line_layer = layer_create(line_frame);
  layer_set_update_proc(s_line_layer, line_layer_update_callback);
  layer_add_child(window_layer, s_line_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_log_layer);
  text_layer_destroy(s_time_layer);

  layer_destroy(s_line_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  //tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);//NO!!

  app_message_register_inbox_received(received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Prevent starting blank
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  handle_minute_tick(t, MINUTE_UNIT);
  handle_second_tick(t, SECOND_UNIT);
}

static void deinit() {
  window_destroy(s_main_window);

  tick_timer_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
