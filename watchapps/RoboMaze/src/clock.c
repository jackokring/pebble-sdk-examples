#include "pebble.h"

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern int32_t score;
extern double value;

static unsigned char dp = 0;//dp col
static unsigned char mode = 0;//clock,date,stopwatch,value,score

static const int16_t pics[] = {
  0b111101101101111,//0
  0b100100100100100,//1
  0b111001111100111,//2
  0b111100111100111,//3
  0b100100111101101,//4
  0b111100111001111,//5
  0b111101111001111,//6
  0b100100100100111,//7
  0b111101111101111,//8
  0b111100111101111,//9
  0 //more!!
};

static void clear() {
  for(int j = 0; j < 5; j++)
    for(int i = 0; i < 4 * 8 + 2; i++) {
      put_map(maze, i + 1, j + 1, 35, 31);
  }
}

static void draw(int digit, int location) {
  int16_t pix = pics[digit%32];
  for(int j = 0; j < 5; j++)
    for(int i = 0; i < 3; i++) {
      put_map(maze, i + 1 + (location * 4) + ((location >= dp)?2:0), j + 1, 35, (digit&32) + ((pix & 1)==0)?31:0);
      pix >>= 1;
  }
}

static void draw_dp(int bits, int location) {
  dp = location;//remember
  bool flash = bits&32;
  for(int j = 0; j < 5; j++) {
    put_map(maze, 1 + location * 4, j + 1, 35, flash + ((bits & 1)==0)?31:0);//top to bottom from lsb to msb
    bits >>= 1;
  }
}

void save_clock() {
  //null
}

static int8_t button_sec = 0;
static int8_t delays[] = { 0, 0, 2, 3, 4 };//2nd has delay switch to first
static bool keys[] = { false, false, true, true, false };

void tick_clock(struct tm *tick_time) {
  clear();
  if(++button_sec == 3) mode = delays[mode];//change mode
  switch(mode) {
  case 0: draw_dp(0b1010, 2);
    draw(tick_time->tm_hour/10, 0);
    draw(tick_time->tm_hour%10, 1);
    draw(tick_time->tm_min/10, 2);
    draw(tick_time->tm_min%10, 3);
    draw(tick_time->tm_sec/10, 6);
    draw(tick_time->tm_sec%10, 7);
    break;
  default: break;
  }
}

void load_clock() {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  tick_clock(t);
}

static bool clickback = false;

bool click_clock(ButtonId b, bool single) {//never gets select button
  button_sec = 0;//restore
  bool tmp = keys[mode];
  if(b == BUTTON_ID_BACK) {
    if(!clickback) {
      mode++;
      mode %= 4;//four mode loop 
    } else {
      mode = 0;//back to clock
      clickback = false;
    }
  } else {
    clickback = true;//botch
    if(keys[mode]) {
      //own custom actions
    } else {
      mode = 4;//show score
    }
  }
  load_clock();//easy fast action
  return tmp;//handled?
}
