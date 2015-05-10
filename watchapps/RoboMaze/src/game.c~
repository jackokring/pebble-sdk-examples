#include "pebble.h"

#define MAP_STORE 0

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern bool pause;

void load() {
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
}

void save() {
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

void tick() {

}

void click(ButtonId b, bool single) {

}
