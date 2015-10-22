//===============================================================================
// THE CALCULATION AND COMPUTE ENGINE
//===============================================================================

// Add programmability here. Nice!!

#include "pebble.h"
#include "store.h"
#include "double.h"

extern unsigned char maze[];//maze size -- EXTERN!!
extern unsigned char con[];//console size -- EXTERN!!
extern int seconds;
extern unsigned char get_map(unsigned char * ptr, int x, int y, int mod);
extern void put_map(unsigned char * ptr, int x, int y, int mod, int val);
extern bool pause;
extern unsigned char mode;

double value;//output value
bool error = false;
static bool hold = false;

static bool expo = false;

typedef struct {
  float x;
} loader;

static loader xx;

void load_basik() {
  value = zero;
  if(persist_exists(BAS_VAL)) {
  	persist_read_data(BAS_VAL, &xx, sizeof(xx));
	value = xx.x;
	expo = persist_read_bool(BAS_EXPO);
	hold = persist_read_bool(BAS_HOLD);
  }
}

void save_basik() {
  xx.x = value;
  persist_write_data(BAS_VAL, &xx, sizeof(xx));
  persist_write_bool(BAS_EXPO, expo);
  persist_write_bool(BAS_HOLD, hold);
}

void tick_basik() {

}

void click_basik(ButtonId b, bool single) {

}

static void ac() {
  hold = false;
  value = zero;
  expo = false;
}

void click_value(ButtonId b, bool single) {
  if(hold && error) {
	ac();
  }
  if(error) {
	hold = true;
	return;
  }
  if(hold && !error) {
	hold = false;
  	value = zero;
  	expo = false;
  }
  if(b == BUTTON_ID_SELECT) {
	expo = !expo;
	return;//exponent
  }
  if(expo) {
	if(b == BUTTON_ID_UP) {
		if(single) value = value * ten;
		else value = value * tenten;// ^ 10
	} else {
		if(single) value = value * tenth;
		else value = value * tententh;// rt(10)
	}
  } else {// edit the value on sreen
	if(b == BUTTON_ID_UP) {
		if(single) value = value + one;
		else value = value * ten;
	} else {
		if(single) value = value - one;
		else value = value * tenth;
	}
  }
}
