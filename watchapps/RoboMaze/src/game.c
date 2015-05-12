#include "pebble.h"

#define MAP_STORE 0

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern bool pause;
int32_t score = 0;

static uint16_t compact[32];

static void blank() {
  for(int i = 0; i < 32; i++) compact[i] = 0;//fill with nothing
}

static int loadwall(int x, int y) {
  if(!persist_exists(MAP_STORE)) blank();
  persist_read_data(MAP_STORE, compact, sizeof(compact));
  return 0;
}

void load() {
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
  for(int i = 0; i < 34-1; i++)
    for(int j = 8; j < 34+8-1; j++) {
      if(i%2 == 1 && j%2 == 1) continue;//main stay
      else if(i%2 == 0 && j%2 == 0) continue;//blank!! An active square not wall
      else if((get_map(maze, i, j, 35)&31) < 16) savewall((i-2)/2, (j-8)/2);
  }
  persist_write_data(MAP_STORE, compact, sizeof(compact));
}

void tick() {

}

void click(ButtonId b, bool single) {

}
