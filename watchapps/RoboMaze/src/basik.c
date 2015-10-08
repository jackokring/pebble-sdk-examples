//===============================================================================
// THE CALCULATION AND COMPUTE ENGINE
//===============================================================================

// Add programmability here. Nice!!

#include "pebble.h"
#include "store.h"

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern bool pause;
extern unsigned char mode;

double value = 0;//output value

void load_basik() {
  if(persist_exists(BAS_VAL)) {
  	persist_read_data(BAS_VAL, &value, sizeof(double));
  }
}

void save_basik() {
  persist_write_data(BAS_VAL, &value, sizeof(double));
}

void tick_basik() {

}

void click_basik(ButtonId b, bool single) {

}

void click_value(ButtonId b, bool single) {

}
