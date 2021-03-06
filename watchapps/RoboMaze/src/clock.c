//===============================================================================
// THE CLOCK EATURES UNCTIONALITY
//===============================================================================

// You should not edit this.

#include "pebble.h"
#include "store.h"
#include "double.h"

#ifdef BULK_BUILD

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int8_t seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern int32_t score;
extern int32_t hiscore;
extern uint32_t level;
extern int8_t direction;
extern double value;
extern void click_basik(ButtonId b, bool single);
extern void click_value(ButtonId b, bool single);
extern void click(ButtonId b, bool single);
extern bool gAB;
extern bool slong;
extern bool error;
extern void reset();

unsigned char dp = 0;//dp col
unsigned char mode = 0;//clock,date,stopwatch,value,score

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
  0b001001001101111,//M 0
  0b010101101101010,//O 1
  0b101101101101111,//N 2
  0b010010010010111,//T 3
  0b010101101101101,//U 4
  0b111001111001111,//E 5
  0b111101101001001,//W 6
  0b011101101101011,//D 7
  0b101101111101101,//H 8
  0b001001111001111,// 9
  0b101101011101011,//R 0
  0b111010010010111,//I 1
  0b011100010001110,//S 2
  0b101101111101010,//A 3
  0b010101100100100,//J 4
  0b011101011101011,//B 5
  0b001001011101011,//P 6
  0b010010010101101,//Y 7
  0b111001001001001,//L 8
  0b110101101001110,//G 9
  0b110001001001110,//C 0
  0b010010101101101,//V 1
  0b000000111000000//-
};

static int8_t days[] = { 22, 14, 12, 10, 11, 12, 13, 14, 15, 16, 15, 17, 13, 18, 14, 19, 20, 21, 22, 23, 13 /* };
static int8_t months[] = {*/, 24, 23, 12, 19, 15, 25, 10, 23, 20,
                           23, 26, 20, 10, 23, 27, 24, 14, 12,
                           24, 14, 28, 23, 14, 29, 22, 15, 26,
                           11, 30, 13, 12, 11, 31, 17, 15, 30 };

static void clear() {
  for(int8_t j = 0; j < 5; j++)
    for(int8_t i = 0; i < 4 * 8 + 2; i++) {
      put_map(maze, i + 1, j + 1, 35, 31);
  }
}

static void draw(int digit, int location) {
  int16_t pix = pics[digit%32];
  for(int8_t j = 0; j < 5; j++)
    for(int8_t i = 0; i < 3; i++) {
      put_map(maze, i + 1 + (location * 4) + ((location >= dp)?2:0), j + 1, 35, (((pix & 1)==0)?31:(digit&32)));
      pix >>= 1;
  }
}

static void draw_dp(int bits, int location) {
  dp = location;//remember
  int8_t tmp = bits;
  for(int8_t j = 0; j < 5; j++) {
    put_map(maze, 1 + location * 4, j + 1, 35, (((bits & 1)==0)?31:(tmp&32)));//top to bottom from lsb to msb
    bits >>= 1;
  }
}

static int16_t sw_butt = 0;
static struct tm lap, reg;

void save_clock() {
  time_t now = time(NULL);
#ifdef PBL_SDK_2
  struct tm *t = localtime(&now);
#else
  struct tm *t = gmtime(&now);
#endif
  persist_write_data(SW_STORE, &reg, sizeof(reg));
  persist_write_data(SW_LAP, &lap, sizeof(lap));
  persist_write_data(SW_STOP, t, sizeof(reg));
  persist_write_int(SW_BUTT, sw_butt);
}

static int8_t button_sec = 0;
static int8_t delays[] = { 0, 0, 2, 3, 4, 4, 4 };//2nd has delay switch to first
static bool keys[] = { false, false, true, true, true, false, false };

static void tock(struct tm *tick_time, bool max, bool t4hr) {
    if(max) draw(tick_time->tm_hour/100, 0);
    int16_t tmp = tick_time->tm_hour;
    if(t4hr) {
      draw((tmp/10)%10, 1);
      draw(tmp%10, 2);
    } else {
      tmp %= 12;//12 hr
      if(tmp == 0) tmp = 12;
      if(tmp/10 != 0)
        draw(1, 1);
      draw(tmp%10, 2);
    }
    draw(tick_time->tm_min/10, 3);
    draw(tick_time->tm_min%10, 4);
    draw(tick_time->tm_sec/10, 6);
    draw(tick_time->tm_sec%10, 7);
}

static void sw_tick() {
  if(++reg.tm_sec == 60) {
    reg.tm_sec = 0;
    if(++reg.tm_min == 60) {
      reg.tm_min = 0;
      if(++reg.tm_hour == 960)
        reg.tm_hour = 0;
    }
  }
}

static void printint(int x, int pos, bool zeros) {
  bool neg = false;
  if(x < 0) {
    x = -x;
    neg = true;
  }
  int i;
  for(i = pos; i >= neg?1:0; i--) {//leave space for negative sign
    draw(x%10, i);
    x /= 10;
    if(x == 0) {
      if(zeros) {
        if(pos < dp - 1) break;//zero fill
      }
      else break;//exit
    }    
  }
  if(neg) draw(32, --i);//negate
}

static void sci(bool neg, double x);

static void printreal(double x, bool ex) {
  bool neg = false;
  if(x < zero) {
    neg = true;
    x = -x;
  }
  if(x == zero|| (!neg && x < 0.0001e-99) || (neg && x < 0.001e-99)) {
    draw_dp(0b10000, 8);
    draw(0, 7);
    return;//Zero rounding
  }
  if((!neg && x > 9.9999e+99) || (neg && x > 9.999e+99) || x != x) {//NaN test too 
    error = true;
    draw(15, 7);
    return;//Overflow
  }
  double mask = ((ex)?100000.0:100000000.0);//use exponent
  mask = ((!neg)?mask:(mask * tenth));//leave space for negative sign
  if(x >= mask || x <= one/mask) {//sci notation as too big
    sci(neg, x);
    return;
  }
  dp = (ex?5:8);//set zero placed deciaml
  //either defered as sci exponent, zero or error printed so far!!
  bool zeros = (x < one);
  int check = 0;
  while((zeros)?(dp > (neg?2:1)):(x < mask)) {
    check = (int)x;
    if((double)check == x) break;//exact
    --dp;
    x *= 10;
  }
  draw_dp(0b10000, dp);
  printint(neg?-check:check, ex?4:7, zeros);//integer part drawn
}

static void sci(bool neg, double x) {
  int ex = 0;
  while(x > ten) {
    x /= ten;
    ex++;
  }
  while(x < one) {
    x *= ten;
    ex--;
  }
  if(neg) x = -x;
  printreal(x, true);
  printint(ex, 7, false);//no zeros
}

void show_lvl(int mode_to) {
  button_sec = 0;//restore
  if(mode > 3) mode = mode_to;//show if not watch
}

void tick_clock(struct tm *tick_time, bool stop) {
  clear();
  if((sw_butt&4)==4) { 
    reg.tm_hour = 0;
    reg.tm_min = 0;
    reg.tm_sec = 0;
    sw_butt ^= 4;//reset
  }
  if(!stop && (sw_butt&1)==1) sw_tick();
  if(++button_sec == 3) mode = delays[mode];//change mode
  switch(mode) {
  case 0: draw_dp(0b101010, 3);//time
    if (!clock_is_24h_style()) {
      if(tick_time->tm_hour > 11)
        draw(26, 0);//PM
      else
        draw(23, 0);//AM
    }
    tock(tick_time, false, clock_is_24h_style());
    break;
  case 1: draw_dp(0b100, 3);//date
    draw(days[tick_time->tm_mon * 3 + 21], 5);
    draw(days[tick_time->tm_mon * 3 + 22], 6);
    draw(days[tick_time->tm_mon * 3 + 23], 7);
    draw(tick_time->tm_mday/10, 3);
    draw(tick_time->tm_mday%10, 4);
    draw(days[tick_time->tm_wday * 3], 0);
    draw(days[tick_time->tm_wday * 3 + 1], 1);
    draw(days[tick_time->tm_wday * 3 + 2], 2);
    break;
  case 2: draw_dp(0b1010 + (((sw_butt&1)==1)?32:0) + (((sw_butt&2)==2)?0b10001:0), 3);//stopwatch
    if((sw_butt&2)==2) {
      tock(&lap, true, true);
    } else {
      tock(&reg, true, true);
    }
    break;
  case 3: printreal(value, false);
    break;
  case 4: draw_dp(1<<(direction%5), 8);//score
    printint(score, 7, false);
    break;
  case 5: draw_dp(1<<(direction%5), 8);//level
    printint(level, 7, false);
    draw(28, 0);//L
    break;
  case 6: draw_dp(1<<(direction%5), 8);//hiscore
    printint(hiscore, 7, false);
    draw(18, 0);//H
    break;
  default: break;
  }
}

static void reset_sw() {
  sw_butt = 0;
  memset(&reg, 0, sizeof(reg));
  memset(&lap, 0, sizeof(lap));
}

void load_clock() {
  time_t now = time(NULL);
#ifdef PBL_SDK_2
  struct tm *t = localtime(&now);
#else
  struct tm *t = gmtime(&now);
#endif
  tick_clock(t, true);
  if(persist_exists(SW_BUTT)) {
    sw_butt = persist_read_int(SW_BUTT);
    persist_read_data(SW_STOP, &reg, sizeof(reg));
    lap.tm_hour = t->tm_hour - reg.tm_hour + 960;
    lap.tm_hour += (t->tm_yday - reg.tm_yday) * 24;//days;
    if(reg.tm_year != t->tm_year) {
      lap.tm_hour += 365 * 24;//happy new year (close enough)
      if((reg.tm_year%4 == 0) && ((reg.tm_year%100 != 0) || (reg.tm_year%400 == 0)))
        lap.tm_hour += 24;//leap year day
      if(t->tm_year - reg.tm_year > 1) {//greater than 1 year
        reset_sw();//get a life!!
        return;
      } 
    }
    lap.tm_min = t->tm_min - reg.tm_min + 60;
    lap.tm_sec = t->tm_sec - reg.tm_sec + 60;
    persist_read_data(SW_STORE, &reg, sizeof(reg));
    if((sw_butt&1)==1) {//add extra running time
      reg.tm_hour += lap.tm_hour;
      reg.tm_hour %= 960;//40 day timer
      reg.tm_min += lap.tm_min;
      reg.tm_min %= 60;
      reg.tm_sec += lap.tm_sec;
      reg.tm_sec %= 60;
    }
    persist_read_data(SW_LAP, &lap, sizeof(lap));
    return;
  }
  reset_sw();
}

static bool clickback = false;

void click_clock(ButtonId b, bool single) {
  button_sec = 0;//restore
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
      if(mode==2) {
        switch(b) {
        case BUTTON_ID_UP://lap/reset
          if((sw_butt&1)==0) {//not running
            if((sw_butt&2)==2) //showing lap
              sw_butt ^= 2;
            else //showing counter
              sw_butt = 4;//reset
          } else { //running
            sw_butt ^= 2;
            memcpy(&lap, &reg, sizeof(lap));//get lap buffer
          }
          break;
        case BUTTON_ID_DOWN://start/stop
          sw_butt ^= 1;
          break;
        default: break;
        }
        if(!single) sw_butt ^= 4;//reset
      }
      if(mode==3) {
        click_value(b, single);//send
      }
      if(mode==4) {
        click(b, single);
      }
    } else {
      mode = 4;//show score
      if(b == BUTTON_ID_SELECT) {
        reset();//long select
        return;
      }
      if(b == BUTTON_ID_UP) gAB = false;
      if(b == BUTTON_ID_DOWN) gAB = true;
      slong = single;
    }
  }
}

#endif
