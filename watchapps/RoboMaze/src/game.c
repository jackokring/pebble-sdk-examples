//===============================================================================
// THE GAME CODE
//===============================================================================

// Edit this to change the game. Remember a lot of the processing is abstracted
// such as the score is auto displayed from the variable score, and such.

#include "pebble.h"
#include "store.h"

#ifdef BULK_BUILD

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int8_t seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern void show_lvl();
extern void showlogo();

void reset();
static void account();
static void makemaze();
static void makechar();
static void ready();
static void move();
static void respond();
static void evaluate();
static void delay();
static void xpand();

static uint16_t compact[31];
static uint16_t chaf[16];

static void blank(int with) {
  for(int i = 0; i < 31; i++) compact[i] = with;//fill with nothing
}

static void blankc(int with) {
  for(int i = 0; i < 16; i++) chaf[i] = with;//fill with nothing
}

bool pause = false;
bool gAB;
bool slong;
int32_t score;
int32_t hiscore;
uint32_t level;
int8_t direction;

static mover ting[8];

static bool success;
static void (*oldstate)() = reset;
static int countdown;
static void (*state)() = reset;
static void (*(fn[]))() = { reset, account, makemaze, makechar, ready, move, respond, evaluate, delay, xpand };//end on initial state
static int8_t dxy[] = { -35, 1, 35, -1 };//rotary
static int8_t offset[] = { -16, 1, 16, -1 };//for location update

static void loadchaf(int x, int y) {
  put_map(maze, 2 * x + 2, 2 * y + 8, 35, (((chaf[x]>>y)&1)==1)?30:31);
}

static void loadwall(int x, int y) {
  put_map(maze, x + 2, 2 * y + 8 + (x + 1)%2, 35,(((compact[x]>>y)&1)==1)?0:31);
}

static void savewall(int x, int y) {
  compact[x] |= 1<<y;
}

static void savechaf(int x, int y) {
  chaf[x] |= 1<<y;
}

static void delay() {
  score += level * 12 * countdown;
  if((--countdown)==0)
    state = oldstate;
}

static void do_delay(int sec, void (*newstate)()) {
  countdown = sec;
  oldstate = newstate;
  state = delay;
}

void reset() {
  showlogo();
  score = 0;
  direction = 2;
  level = 0;
  show_lvl(6);//hiscore
  do_delay(6, account);
} 

static void destroywall(int x, int y) {
  if(rand()%32 != 0) return;
  int hv = ((x%2)==0)?1:0;
  for(int i = hv; i <= hv + 2; i+=2) {
    int k = 0;
    for(int j = 0; j < 4; j++) 
      if((get_map(maze, x + 2 + dxy[i] + dxy[j], 2 * y + 8 + (x + 1)%2, 35)&31) < 16) k++;//wall
    if(k <= 1) return;//single wall
  }
  compact[x] &= ~(1<<y);
}

static int xygrid(int x, int y) {
  return x * 2 + 2 + 35 * (y * 2 + 8);
}

static void activechaf(int x, int y) {//16*16
  int q = xygrid(x, y);
  int z = get_map(maze, q, 0, 0)&31;//get grid
  if(z == 31) {// on blank
    success = false;
    for(int j = 0; j < 4; j++) 
      if((get_map(maze, q, dxy[j], 1)&31) >= 16) {
        if((get_map(maze, q, 2 * dxy[j], 1)&31) == 30) {
          savechaf(x, y);
        }
    }
  }
}

static void processmaze(void (*active)(int x, int y), void (*wall)(int x, int y), bool mains) {
  for(int i = 2; i <= 2 + 30; i++)
    for(int j = 8; j <= 8 + 30; j++) {
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
  for(int i = 0; i < 10; i++) {//quick draw
    processmaze(activechaf, destroywall, false);
    drawmaze();
  }
  if(!success) return;
  //state = makechar; ?????????  <--- no characters yet
  state = account;
}

static void makemaze() {
  blank(-1);//fill maze
  blankc(0);//no chaf
  savechaf(0, 0);
  drawmaze();//single percolation chaf
  state = xpand;
}

static uint8_t orders[4][4] = { { 3, 0, 1, 2 },//0 LURD
                                { 0, 3, 1, 2 },//1 ULRD
                                { 0, 1, 3, 2 },//2 URLD
                                { 1, 0, 3, 2 } };//3 RULD

static uint8_t dir[] = { 0, 1, 1, 2, 3 };//element 2 is auto &dir[2]
static uint8_t defis[] = { 0, 1, 1, 1, 2, 2, 3, 3 };
static uint8_t defchar[4][5] = { { 18, 17, 19, 16, 0 },//0 player (directions and anim frames)
                                { 22, 21, 23, 20, 0 },//1 missile
                                { 28, 28, 28, 28, 3 },//2 fill dot
                                { 24, 24, 24, 24, 4 } };//3 boomerang

static void makechar() {
  for(uint8_t i = 0; i < sizeof(ting)/sizeof(mover); i++) {
    ting[i].pos = rand()%256;//on 16 by 16 grid
    ting[i].dpos = 0;//filled in by motion
    ting[i].dir = rand()%4;//index into dxy[]
    ting[i].mode = rand()%4;//index into orders
    ting[i].is = defis[i];
  }
  state = ready;
}

static int xydxy(mover x) {
  return xygrid(x.pos%16, x.pos/16) + x.dpos;
}

static void drawchars() {
  drawmaze();//before chars overlay
  for(uint8_t i = 0; i < sizeof(ting)/sizeof(mover); i++) {
    int q = xydxy(ting[i]);
    put_map(maze, q, 0, 0, defchar[ting[i].is][ting[i].dir] + seconds%defchar[ting[i].is][4]);//anim
  }
}

void ready() {
  show_lvl(5);//show level
  drawchars();
  do_delay(6, move);
}

static void move() {
  state = respond;
  for(uint8_t i = 0; i < sizeof(ting)/sizeof(mover); i++) {
    if(ting[i].is == 0) ting[i].mode = dir[direction];
    for(uint8_t j = 0; j < 4; j++) {
      ting[i].dpos = dxy[(ting[i].dir + orders[ting[i].mode][j])%4];
      int q = get_map(maze, xydxy(ting[i]), 0, 0);//what is at the intended location?
      if((q & 31) < 16) continue;//wall so no go
      ting[i].dir = (ting[i].dir + orders[ting[i].mode][j])%4;
      break;//good direction
    }
  }
  drawchars();
}

void save();

static void respond() {
  state = move;
  for(uint8_t i = 0; i < sizeof(ting)/sizeof(mover); i++) {
    ting[i].dpos *= 2;
    int q = get_map(maze, xydxy(ting[i]), 0, 0);//what is at the intended location?
    if((q & 31) == 30 && ting[i].is == 0) {
      put_map(maze, xydxy(ting[i]), 0, 0, 31);
      score++;
      save();
      int x = 0;
      for(uint8_t k = 0; k < 16; k++) x += chaf[k];
      if(x != 0) state = evaluate;
    }
    ting[i].pos += offset[ting[i].dir];
  }
}

static void evaluate() {
  do_delay(level / 4 + 1, account);
}

/* MAIN FUNCTIONS - ABOVE IS STATES in fn */

void tick() {
  (*state)();
}

void click(ButtonId b, bool single) {
  if(!single) {
    if(b == BUTTON_ID_UP) direction = 0;
    if(b == BUTTON_ID_DOWN) direction = 4;
    if(b == BUTTON_ID_SELECT) direction = 2;
    return;
  }
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
  for(i = 2; i <= 2 + 30; i++)
    for(int j = 8; j <= 8 + 30; j++) {
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

#endif
