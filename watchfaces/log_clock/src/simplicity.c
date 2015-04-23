#include "pebble.h"

double sqrt(double x) {
  return x;//for now
}

double PI = 3.141592658;

double powi(double x, int y) {
        int p = y;//should be in range
        double mul = x;
        double acc = 1;
        for(int j=0;j<32;j++) {
            if((p&1)==1) acc *= mul;
            p >>= 1;
            mul *= mul;
        }
        return acc;
}

double log(double x) {
        int flip = 1;
        if(x>1) { x = 1/x; flip = -1;}
        int sc = 2;
        double mul;
        //smaller range for quadratic gain, without too
        //much  loss of precission
        while(x<0.875) {
            x = sqrt(x);
            sc <<= 1;
        }
        double acc;
        acc = x = (mul = (x-1)/(x+1));
        x *= x;
        for(int i = 3;i < 16;i+=2) {
            acc += mul/i;
            mul *= x;
        }
        return sc*flip*acc;//an accurate log
}

double atan(double x) {
        int flip = 0;
        if(x>1||x<-1) { flip = -1; x = 1/x; }//cos/sin??+angle??
        int sc = 1;
        while(x<-0.125||x>0.125) {
            x = x/(1+sqrt(1+x*x));
            sc <<= 1;
        }
        double mul = x;
        double acc = x;
        x *= -x;
        for(int i = 3;i < 16;i+=2) {
            acc += mul/i;
            mul *= x;
        }
        acc *= sc;
        //+/-45deg
        if(flip) {
            acc = ((acc>0) ? PI/2-acc : -PI/2+acc);
        }
        return acc;
}

double ln2 = 0.69314718056;

double exp(double x) {
        int flip = 0;
        if(x<0) { flip = -1; x = -x;}//all positive
        int digits = (int)(log(x)*ln2);
        double po = powi(2,digits);
        x = x/po;//residual
        int raz = 1;
        //focus series for gain
        while(x>0.125) {
            raz <<= 1;
            x /= 2;
        }
        //the exp
        double acc = 1;//first term
        double fac = 1;
        double mul = 1;
        for(int i = 1;i < 8;i++) {
            fac *= i;
            mul *= x;
            acc += mul/fac;
        }
        x = powi(acc, (int)(po*raz));
        if(flip) x = 1/x;
        return x;//the actual exp
}

static Window *s_main_window;
static TextLayer *s_date_layer, *s_time_layer;
static Layer *s_line_layer;

static void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char s_time_text[] = "00:00";
  static char s_date_text[] = "Xxxxxxxxx 00";
  static char log_text[] = "xxxxxxxxxx";//10

  float s = (float)tick_time->tm_sec;//get seconds
  s = log(s)/log(60.0);

  strftime(s_date_text, sizeof(s_date_text), "     %S", tick_time);

  //insert log
  memmove(s_date_text, &log_text[2], 4);//4 digit after dp

  text_layer_set_text(s_date_layer, s_date_text);

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

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_date_layer = text_layer_create(GRect(8, 68, 136, 100));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_time_layer = text_layer_create(GRect(7, 92, 137, 76));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  GRect line_frame = GRect(8, 97, 139, 2);
  s_line_layer = layer_create(line_frame);
  layer_set_update_proc(s_line_layer, line_layer_update_callback);
  layer_add_child(window_layer, s_line_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
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

  tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);
  
  // Prevent starting blank
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  handle_minute_tick(t, SECOND_UNIT);
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
