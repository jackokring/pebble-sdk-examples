#include "pebble.h"
#include "store.h"

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern bool pause;

double value = 0;//output value

void load_basik() {

}

void save_basik() {

}

void tick_basik() {

}

void click_basik(ButtonId b, bool single) {
  if(pause) {//console

  } else {//value

  }
}
