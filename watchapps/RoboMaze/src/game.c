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
static void delay();
static void xpand();

static uint16_t compact[32];
static uint16_t chaf[16];

static void blank(int with) {
  for(int i = 0; i < 32; i++) compact[i] = with;//fill with nothing
}

static void blankc(int with) {
  for(int i = 0; i < 16; i++) chaf[i] = with;//fill with nothing
}

bool pause = false;
bool gAB;
int32_t score;
int32_t hiscore;
uint32_t level;
int8_t direction;

static bool success;
static void (*oldstate)() = reset;
static int countdown;
static void (*state)() = reset;
static void (*(fn[]))() = { reset, account, makemaze, makechar, ready, move, respond, evaluate, delay, xpand };//end on initial state

static void loadchaf(int x, int y) {
  put_map(maze, 2 * x + 2, 2 * y + 8, 35, (((chaf[x]>>y)&1)==1)?30:31);
}

static void null(int x, int y) {
  return;
}

static void loadwall(int x, int y) {
  put_map(maze, x + 2, 2 * y + 8 + (~x)%2, 35,(((compact[x]>>y)&1)==1)?0:31);
}

static void savewall(int x, int y) {
  compact[x] |= 1<<y;
}

static void savechaf(int x, int y) {
  chaf[x] |= 1<<y;
}

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

static void destroywall(int x, int y) {
  if(rand()%16 != 0) return;
  compact[x] &= ~(1<<y);
}

static int8_t dxy[] = { -1, 1, -35, 35 };

static int xygrid(int x, int y) {
  return x * 2 + 3 + 35 * (y * 2 + 8);
}

static void activechaf(int x, int y) {//16*16
  int q = xygrid(x, y);
  int z = get_map(maze, q, 0, 0)&31;//get grid
  if(z == 31) {// on blank
    for(int j = 0; j < 4; j++) 
      if((get_map(maze, q, dxy[j], 1)&31) >= 16) {
        if((get_map(maze, q, 2 * dxy[j], 1)&31) == 30) {
          success = false;
          put_map(maze, q, 0, 0, 30);//blank spread
        }
    }
  }
}

static void processmaze(void (*active)(int x, int y), void (*wall)(int x, int y), bool mains) {
  for(int i = 2; i <= 35 - 2; i++)
    for(int j = 2 + 6; j <= 35 - 2 + 6; j++) {
      if(i%2 == 1 && j%2 == 1) {
        if(mains) put_map(maze, i, j, 35, 0);//main stay
      } else if(i%2 == 0 && j%2 == 0) active((i-2)/2, (j-8)/2);//blank!! An active square not wall
      else wall((i-2), (j-8)/2);
  }
}

static void drawmaze() {
  //draw maze
  for(int i = 1; i <= 1 + 32; i++) {
    put_map(maze, i, 7, 35, 0);
    put_map(maze, i, 7 + 32, 35, 0);
  }
  for(int i = 7; i <= 7 + 32; i++) {
    put_map(maze, 1, i, 35, 0);
    put_map(maze, 1 + 32, i, 35, 0);
  }
  processmaze(loadchaf, loadwall, true);
}

static void account() {
  //calculate bonus based on level, except when level 0
  
  level++;
  srand(level);//seed level
  state = makemaze;
}

static void xpand() {
  success = true;
  //processmaze(activechaf, destroywall, false);
  processmaze(null, destroywall, false);
  drawmaze();
  if(!success) return;
  state = makechar;
}

static void makemaze() {
  blank(-1);//fill maze
  blankc(0);//no chaf
  savechaf(0, 0);
  drawmaze();//single percolation chaf
  state = xpand;
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

  state = /* move */ reset;//testing
}

/* MAIN FUNCTIONS - ABOVE IS STATES in fn */

void tick() {
  (*state)();
}

void click(ButtonId b, bool single) {
  if(b == BUTTON_ID_UP) direction--;
  if(b == BUTTON_ID_DOWN) direction++;
  if(direction > 4) direction = 4;
  if(direction < 0) direction = 0;
}

void load() {
  pause = true;
  reset();
  blank(0);
  if(persist_exists(CHAF)) {
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
    persist_read_data(CHAF, chaf, sizeof(chaf));
  } else {
    score = 0;
    hiscore = 0;
    level = 0;//incremented later
    direction = 2;//auto
    gAB = false;//A
    state = reset;
  }
  for(int i = 0; i < 35; i++)
    for(int j = 0; j < 41; j++)
      put_map(maze, i, j, 35, 31);//clear screen
  drawmaze();
  pause = false;
}

void save() {
  pause = true;//still
  blank(0);
  int i;
  for(i = 2; i <= 2 + 32 - 1; i++)
    for(int j = 8; j <= 8 + 32 - 1; j++) {
      if(i%2 == 1 && j%2 == 1) continue;//main stay
      else if(i%2 == 0 && j%2 == 0) {
        if((get_map(maze, i, j, 35)&31) == 30) savechaf((i-2)/2, (j-8)/2);
      }//blank!! An active square not wall
      else if((get_map(maze, i, j, 35)&31) < 16) savewall((i-2), (j-8)/2);
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
  persist_write_data(CHAF, chaf, sizeof(chaf));
}
