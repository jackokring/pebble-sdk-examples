#include "pebble.h"
#include "store.h"

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern void show_lvl();

static void reset();
static void account();
static void makemaze();
static void makechar();
static void ready();
static void move();
static void respond();
static void evaluate();
static void reset();
static void delay();

static void null() {};//NULL function

bool pause = false;
bool gAB;
int32_t score;
int32_t hiscore;
int32_t level;
int8_t direction;

static void (*oldstate)() = reset;
static int countdown;
static void (*state)() = reset;
static void (*(fn[]))() = { reset, account, makemaze, makechar, ready, move, respond, evaluate, delay };//end on initial state

static void delay() {
  if((--countdown)==0)
    state = oldstate;
}

static void do_delay(int sec, void (*newstate)()) {
  countdown = sec;
  oldstate = newstate;
  state = delay;
}

static void reset() {
  score = 0;
  direction = 3;
  level = 0;
  show_lvl(6);//hiscore
  do_delay(6, account);
} 

static uint16_t compact[32];

static void account() {
  //calculate bonus based on level, except when level 0
  
  level++;
  state = makemaze;
}

static void makemaze() {

  state = makechar;
}

static void makechar() {

  state = ready;
}

static void ready() {
  show_lvl(5);//show level
  do_delay(6, move);
}

static void move() {

  state = respond;
}

static void respond() {

  state = evaluate;
}

static void evaluate() {

  state = move;
}

/* MAIN FUNCTIONS - ABOVE IS STATES in fn */

void tick() {
  (*state)();
}

void click(ButtonId b, bool single) {

}

static void blank(int with) {
  for(int i = 0; i < 32; i++) compact[i] = with;//fill with nothing
}

static int loadwall(int x, int y) {
  return (((compact[x]>>y)&1)==1)?0:31;
}

void load() {
  pause = true;
  reset();
  blank(0);
  if(persist_exists(MAP_STORE)) {
    persist_read_data(MAP_STORE, compact, sizeof(compact));
    score = persist_read_int(SCORE);
    hiscore = persist_read_int(HISCORE);
    level = persist_read_int(LEVEL);
    direction = persist_read_int(DIRECTION);
    gAB = persist_read_bool(GAME_AB);
    if(persist_exists(PROCESS)) {
      state = fn[persist_read_int(PROCESS)];
    } else {
      state = reset;//account for missing states
    }
  } else {
    score = 0;
    hiscore = 0;
    level = 0;//incremented later
    direction = 2;//auto
    gAB = false;//A
    state = reset;
  }
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
  pause = false;
}

static void savewall(int x, int y) {
  compact[x] |= 1<<y;
}

void save() {
  pause = true;//still
  blank(0);
  int i;
  for(i = 3; i <= 35-3; i++)
    for(int j = 8; j <= 32+8; j++) {
      if(i%2 == 1 && j%2 == 1) continue;//main stay
      else if(i%2 == 0 && j%2 == 0) continue;//blank!! An active square not wall
      else if((get_map(maze, i, j, 35)&31) < 16) savewall((i-2)/2, (j-8)/2);
  }
  for(i = 0; ((unsigned int)i) < sizeof(fn)/sizeof(state); i++)
    if(state == fn[i]) {
      persist_write_int(PROCESS, i);
      break;
    }
  persist_write_data(MAP_STORE, compact, sizeof(compact));
  persist_write_int(SCORE, score);
  persist_write_int(HISCORE, hiscore);
  persist_write_int(LEVEL, level);
  persist_write_int(DIRECTION, direction);
  persist_write_bool(GAME_AB, gAB);
}
