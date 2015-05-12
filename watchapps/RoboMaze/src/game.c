#include "pebble.h"

#define MAP_STORE 0
#define PROCESS 4

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
bool pause = false;
bool gAB = false;
int32_t score = 0;
int8_t direction;

static int8_t state = 0;

static uint16_t compact[32];

static void account() {

}

static void makemaze() {

}

static void makechar() {

}

static void ready() {

}

static void move() {

}

static void respond() {

}

static void evaluate() {

}

static void reset() {
  state = 0;
} 

static void (*(fn[]))() = { reset, account, makemaze, makechar, ready, move, respond, evaluate, reset };//end on initial state

void tick() {
  (fn[state])();
}

void click(ButtonId b, bool single) {

}

static void blank() {
  for(int i = 0; i < 32; i++) compact[i] = 0;//fill with nothing
}

static int loadwall(int x, int y) {
  return (((compact[x]>>y)&1)==1)?0:31;
}

void load() {
  blank();
  if(persist_exists(MAP_STORE))
    persist_read_data(MAP_STORE, compact, sizeof(compact));
  //draw maze
  for(int i = 0; i < 35; i++)
    for(int j = 0; j < 41; j++) {
      unsigned char x;//null by default
      if(j < 7 || j == 41-1 || i == 0 || i == 35-1) x = 31;//blank
      else if(j == 7 || j == 40-1) x = 0;//top or bottom row
      else if(i%2 == 1 && j%2 == 1) x = 0;//main stay
      else if(i == 1 || i == 34-1) x = 0;//left or right col
      else if(i%2 == 0 && j%2 == 0) x = 31;//blank!!
      else x = loadwall((i-2)/2, (j-8)/2);
      put_map(maze, i, j, 35, x);//display wall
  }
}

static void savewall(int x, int y) {
  compact[x] |= 1<<y;
}

void save() {
  blank();
  for(int i = 3; i <= 35-3; i++)
    for(int j = 8; j <= 32+8; j++) {
      if(i%2 == 1 && j%2 == 1) continue;//main stay
      else if(i%2 == 0 && j%2 == 0) continue;//blank!! An active square not wall
      else if((get_map(maze, i, j, 35)&31) < 16) savewall((i-2)/2, (j-8)/2);
  }
  persist_write_data(MAP_STORE, compact, sizeof(compact));
}
