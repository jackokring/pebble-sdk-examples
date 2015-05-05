#include "pebble.h"

#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 3
#define NUM_SECOND_MENU_ITEMS 1

#define MAP_STORE 0

static Window *s_menu_window, *s_main_window;
static SimpleMenuLayer *s_simple_menu_layer;
static SimpleMenuSection s_menu_sections[NUM_MENU_SECTIONS];
static SimpleMenuItem s_first_menu_items[NUM_FIRST_MENU_ITEMS];
static SimpleMenuItem s_second_menu_items[NUM_SECOND_MENU_ITEMS];
static GBitmap *s_menu_icon_image;
static GBitmap *map;

static bool s_special_flag = false;
static int s_hit_count = 0;

static unsigned char maze[(((35*41) * 3) >> 2) + 1];//maze size
static GBitmap *(maze_gfx[31]);//blank = 32

static unsigned int get_64(unsigned char * ptr, int x, int y, int mod) {//get 6 bit field
  int idx = ((x + y * mod) * 3) >> 2;
  return ptr[idx] + (ptr[idx + 1] << 8);
}

static int get_shift(int x, int y, int mod) {
  int shift = ((x + y * mod) << 1)%8;
  if((shift & 2) != 0) shift ^= 4;//0, 6, 4, 2 pattern
  return shift;
}

static unsigned char get_map(unsigned char * ptr, int x, int y, int mod) {
  return (get_64(ptr, x, y, mod) >> get_shift(x, y, mod)) & 63;//mask 64
}

static void put_map(unsigned char * ptr, int x, int y, int mod, int val) {
  int shift = get_shift(x, y, mod);
  int tmp = get_64(ptr, x, y, mod);
  int andmsk = 63 << shift;
  val <<= shift;
  tmp = (tmp & ~andmsk) | (val & andmsk);
  int idx = ((x + y * mod) * 3) >> 2;
  ptr[idx] = (unsigned char)(tmp & 255);
  ptr[idx + 1] = (unsigned char)(tmp >> 8);
}

static int reduce_map(int x, int y) {
  int tmp = 0;
  if(x-1 >= 0 && get_map(maze, x-1, y, 35) < 16) tmp += 1;//left
  if(x+1 <= 35-1 && get_map(maze, x+1, y, 35) < 16) tmp += 2;//right
  if(y-1 >= 0 && get_map(maze, x, y-1, 35) < 16) tmp += 4;//up
  if(y+1 <= 41-1 && get_map(maze, x, y+1, 35) < 16) tmp += 8;//down
  return tmp;
}

static void layer_draw(Layer *layer, GContext *ctx) {//the main gfx layer update routine
  GRect bounds = layer_get_bounds(layer);

  // Draw a black filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw map
  for(int i = 0; i < 35; i++)
    for(int j = 0; j < 41; j++) {
      int x;
      if(get_map(maze, i, j, 35) < 16) {
        x = reduce_map(i, j);
        put_map(maze, i, j, 35, x);//write back
        graphics_draw_bitmap_in_rect(ctx, maze_gfx[x], GRect(i * 4, j * 4, 4, 4));//draw map
      }
  }
}

static void show_menu() {
  window_stack_push(s_menu_window, true);
}

static void menu_select_callback(int index, void *ctx) {
  s_first_menu_items[index].subtitle = "You've hit select here!";
  layer_mark_dirty(simple_menu_layer_get_layer(s_simple_menu_layer));
}

static void special_select_callback(int index, void *ctx) {
  // Of course, you can do more complicated things in a menu item select callback
  // Here, we have a simple toggle
  s_special_flag = !s_special_flag;

  SimpleMenuItem *menu_item = &s_second_menu_items[index];

  if (s_special_flag) {
    menu_item->subtitle = "Okay, it's not so special.";
  } else {
    menu_item->subtitle = "Well, maybe a little.";
  }

  if (++s_hit_count > 5) {
    menu_item->title = "Very Special Item";
  }

  layer_mark_dirty(simple_menu_layer_get_layer(s_simple_menu_layer));
}

static void menu_window_load(Window *window) {
  s_menu_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON);

  // Although we already defined NUM_FIRST_MENU_ITEMS, you can define
  // an int as such to easily change the order of menu items later
  int num_a_items = 0;

  s_first_menu_items[num_a_items++] = (SimpleMenuItem) {
    .title = "First Item",
    .callback = menu_select_callback,
  };
  s_first_menu_items[num_a_items++] = (SimpleMenuItem) {
    .title = "Second Item",
    .subtitle = "Here's a subtitle",
    .callback = menu_select_callback,
  };
  s_first_menu_items[num_a_items++] = (SimpleMenuItem) {
    .title = "Third Item",
    .subtitle = "This has an icon",
    .callback = menu_select_callback,
    .icon = s_menu_icon_image,
  };

  s_second_menu_items[0] = (SimpleMenuItem) {
    .title = "Special Item",
    .callback = special_select_callback,
  };

  s_menu_sections[0] = (SimpleMenuSection) {
    .num_items = NUM_FIRST_MENU_ITEMS,
    .items = s_first_menu_items,
  };
  s_menu_sections[1] = (SimpleMenuSection) {
    .title = "Yet Another Section",
    .num_items = NUM_SECOND_MENU_ITEMS,
    .items = s_second_menu_items,
  };

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_simple_menu_layer = simple_menu_layer_create(bounds, window, s_menu_sections, NUM_MENU_SECTIONS, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(s_simple_menu_layer));
}

static void menu_window_unload(Window *window) {
  simple_menu_layer_destroy(s_simple_menu_layer);
  gbitmap_destroy(s_menu_icon_image);
}

static Layer *layer;

static void main_window_load(Window *window) {
#ifdef PBL_SDK_2
  window_set_fullscreen(window, true);
#endif
  map = gbitmap_create_with_resource(RESOURCE_ID_MAP);
  for(int i = 0; i < 16; i++) {
    maze_gfx[i] = gbitmap_create_as_sub_bitmap(map, GRect( (i%4)*4, (i/4)*4, 4, 4 ));//fill maze_gfx
  }
  uint16_t compact[33];
  //if(persist_exists(MAP_STORE)) {
    //persist_read_data(MAP_STORE, compact, sizeof(compact));
  //} else
    for(int i = 0; i < 33; i++) compact[i] = ~0;//fill with walls
  //draw maze
  for(int i = 0; i < 35; i++)
    for(int j = 0; j < 41; j++) {
      unsigned char x;//null by default
      if(j < 8) x = 31;//blank
      else if(j == 8 || j == 41-1) x = 0;//top or bottom row
      else if(i%2 == 0 && j%2 == 0) x = 0;//main stay
      else if(i == 0 || i == 35-1) x = 0;//left or right col
      else if(i%2 == 1 && j%2 == 1) x = 31;//blank!!
      else {//only walls left
        int col = compact[i-1];//get active col
        int y = (j - 9 - i%2) >> 1;  
        if( ((col >> y) & 1) == 1) x = 0; else x = 31;
      }
      put_map(maze, i, j, 35, x);//display wall
  }

  Layer * lay = window_get_root_layer(window);
  layer_set_bounds(lay, GRect(0, 0, 144, 168));
  layer = layer_create(GRect(2, 2, 140, 164));
  layer_set_bounds(layer, GRect(0, 0, 140, 164));
  layer_set_update_proc(layer, layer_draw);
  layer_add_child(lay, layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(layer);
  gbitmap_destroy(map);
  uint16_t compact[33];
  int col;
  for(int i = 9; i < 35-1; i++)
    for(int j = 1; j < 41-1; j++) {
      if(i%2 == 0 && j%2 == 0) continue;//main stay
      else if(i%2 == 1 && j%2 == 1) continue;//blank!!
      else {//onlt walls left
        col = compact[i-1];//get active col
        int y = (j - 9 - i%2) >> 1;  
        if(get_map(maze, i, j, 35))
          col |= (1 << y) ; else col &= ~(1 << y);
      }
      compact[i-1] = col;//write description
  }
  persist_write_data(MAP_STORE, compact, sizeof(compact)); 
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  s_menu_window = window_create();
  window_set_window_handlers(s_menu_window, (WindowHandlers) {
    .load = menu_window_load,
    .unload = menu_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_menu_window);
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
