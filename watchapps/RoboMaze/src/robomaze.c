//===============================================================================
// THE MAIN AND IO ABSTRACTION
//===============================================================================

// You should not edit this.

#include "pebble.h"
#include "store.h"

#ifdef BULK_BUILD

static Window *s_main_window;
static GBitmap *map;

unsigned char con[(((32*24) * 3) >> 2) + 1];//console size -- EXTERN!!
unsigned char maze[(((35*41) * 3) >> 2) + 1];//maze size -- EXTERN!!
static GBitmap *(maze_gfx[31]);//blank = 31
static GBitmap *(char_gfx[60]);
static GBitmap *logo;
static bool slogo = false;

extern void load();
extern void save();
extern void tick();
extern void click(ButtonId b, bool single);

extern void load_basik();
extern void save_basik();
extern void tick_basik();
extern void click_basik(ButtonId b, bool single);
extern void click_display(ButtonId b, bool single);

extern void load_clock();
extern void save_clock();
extern void tick_clock(struct tm *tick_time, bool stop);
extern void click_clock(ButtonId b, bool single);

extern bool pause;
extern unsigned char mode;
extern bool vidmode;
extern int get_at(int x, int y);

int8_t seconds = 0;//EXTERN!!

static Layer *layer;

static int get_64(unsigned char * ptr, int x, int y, int mod) {//get 6 bit field
  int idx = ((x + y * mod) * 3) >> 2;
  return ptr[idx] + (ptr[idx + 1] << 8);
}

static int get_shift(int x, int y, int mod) {
  int shift = ((x + y * mod) << 1)%8;
  if((shift & 2) != 0) shift ^= 4;//0, 6, 4, 2 pattern
  return shift;
}

unsigned char get_map(unsigned char * ptr, int x, int y, int mod) {
  return (get_64(ptr, x, y, mod) >> get_shift(x, y, mod)) & 63;//mask 64
}

void put_map(unsigned char * ptr, int x, int y, int mod, int val) {
  uint16_t shift = get_shift(x, y, mod);
  uint16_t tmp = get_64(ptr, x, y, mod);
  uint16_t andmsk = 63 << shift;
  val <<= shift;
  tmp = (tmp & ~andmsk) | (val & andmsk);
  int idx = ((x + y * mod) * 3) >> 2;
  ptr[idx] = (unsigned char)(tmp & 255);
  ptr[idx + 1] = (unsigned char)(tmp >> 8);
}

static int reduce_map(int x, int y) {
  int tmp = get_map(maze, x, y, 35)&32;
  if(x-1 >= 0 && (get_map(maze, x-1, y, 35)&31) < 16) tmp += 1;//left
  if(x+1 <= 35-1 && (get_map(maze, x+1, y, 35)&31) < 16) tmp += 2;//right
  if(y-1 >= 0 && (get_map(maze, x, y-1, 35)&31) < 16) tmp += 4;//up
  if(y+1 <= 41-1 && (get_map(maze, x, y+1, 35)&31) < 16) tmp += 8;//down
  return tmp;
}

void showlogo() {
  slogo = true;
}

static void layer_draw(Layer *layer, GContext *ctx) {//the main gfx layer update routine
  GRect bounds = layer_get_bounds(layer);

  // Draw a black filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  if(slogo) {
    slogo = false;
    graphics_draw_bitmap_in_rect(ctx, logo, GRect(6 + 16 * 4 - 14, 10 + 12 * 5 - 20, 28, 41));
    return;
  }

  // Draw Console
  if(pause)
    for(int i = 0; i < 32; i++)
      for(int j = 0; j < 24; j++) {
        int x = (vidmode)?get_at(i, j):get_map(con, i, j, 32);
        if(x >= 60) {//cursors
          int z = seconds + i + j;
          if(x == 60) x = z % 10;//number
          if(x == 61) x = z % 26 + 10;//letter
          if(x == 62) x = z % 23 + 36;//punc
          x -= (seconds & 1 == 0)?5:6;//generic
        }
        graphics_draw_bitmap_in_rect(ctx, char_gfx[x], GRect(i * 4 + 6, j * 6 + 10, 3, 5));//draw map
  } else

  // Draw map
  for(int i = 0; i < 35; i++)
    for(int j = 0; j < 41; j++) {
      int x = get_map(maze, i, j, 35);
      int d = 0;
      if(x >= 32) {
	//flashing
        x -= 32;
        if((seconds & 1) == 0) x = 31;//show as blank
      }
      if(x == 31) continue;//don't draw
      if(x < 16) {
        x = reduce_map(i, j);
        put_map(maze, i, j, 35, x);//write back
      }
      if(x >= 16 && x <= 20) {
        d = 1;
      }
      graphics_draw_bitmap_in_rect(ctx, maze_gfx[x], GRect(i * 4 - d, j * 4 - d, 4, 4));//draw map
  }
}

static struct tm *ticks;
static unsigned char last;

void mark_dirty() {
  if(seconds != ticks->tm_sec || mode != last) 
    tick_clock(ticks, true);
  layer_mark_dirty(layer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  seconds = tick_time->tm_sec;
  last = mode;
  tick_clock(ticks = tick_time, false);
  if(!pause) tick();
  tick_basik();
  mark_dirty();
}

static void clicks(ClickRecognizerRef recognizer, bool single) {
  ButtonId b = click_recognizer_get_button_id(recognizer);
  if(b == BUTTON_ID_BACK && pause) {
    pause = false;
    mark_dirty();
    return;
  }
  if(b == BUTTON_ID_SELECT && single) {
    if(!pause) {
      pause = true;
      mark_dirty();
      return;
    } 
  }
  if(pause) {
    if(vidmode) click_display(b, single);
    else click_basik(b, single);
  } else click_clock(b, single);
  mark_dirty();
}

static void single_click_handler(ClickRecognizerRef recognizer, void *context) {
  clicks(recognizer, true);
}

static void long_click_handler(ClickRecognizerRef recognizer, void *context) {
  clicks(recognizer, false);
}

static void config_provider(Window *window) {
  // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_UP, single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, single_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, single_click_handler);

  // long click config:
  window_long_click_subscribe(BUTTON_ID_UP, 0, long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, long_click_handler, NULL);
}

static void main_window_load(Window *window) {
#ifdef PBL_SDK_2
  window_set_fullscreen(window, true);
#endif
  map = gbitmap_create_with_resource(RESOURCE_ID_MAP);
  logo = gbitmap_create_as_sub_bitmap(map, GRect( 36, 0, 28, 41 ));//logo
  for(int i = 0; i < 16; i++) {
    maze_gfx[i] = gbitmap_create_as_sub_bitmap(map, GRect( (i%4)*4, (i/4)*4, 4, 4 ));//fill maze_gfx
  }
  for(int i = 0; i < 4; i++) {
    maze_gfx[i+16] = gbitmap_create_as_sub_bitmap(map, GRect( (i%2)*6 + 16, (i/2)*6, 6, 6 ));//bot
    maze_gfx[i+20] = gbitmap_create_as_sub_bitmap(map, GRect( i*4 + 16, 12, 4, 4 ));//missile
    maze_gfx[i+24] = gbitmap_create_as_sub_bitmap(map, GRect( 32, i*4, 4, 4 ));//bomerang
  }
  for(int i = 0; i < 3; i++) {
    maze_gfx[i+28] = gbitmap_create_as_sub_bitmap(map, GRect( 16 + 12, i*4, 4, 4 ));//aux fill, square, dot
  }
  //maze_gfx[31] = NULL;//black space draw with GRect.
  for(int i = 0; i < 60; i++) {
    char_gfx[i] = gbitmap_create_as_sub_bitmap(map, GRect( (i%12)*3, 16 + (i/12)*5, 3, 5 ));
  }

  load();
  load_clock();
  load_basik();

  Layer * lay = window_get_root_layer(window);
  layer_set_bounds(lay, GRect(0, 0, 144, 168));
  layer = layer_create(GRect(2, 2, 140, 164));
  layer_set_bounds(layer, GRect(0, 0, 140, 164));
  layer_set_update_proc(layer, layer_draw);
  layer_add_child(lay, layer);
  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);
}

static void main_window_unload(Window *window) {
  layer_destroy(layer);
  for(int i = 0; i < 16 + 15; i++) {
    gbitmap_destroy(maze_gfx[i]);//maze_gfx
  }
  for(int i = 0; i < 64; i++) {
    gbitmap_destroy(char_gfx[i]);//char_gfx
  }
  gbitmap_destroy(map);
  save();
  save_clock();
  save_basik();
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
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

#endif
