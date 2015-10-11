//===============================================================================
// THE CONSOLE ENTRY ABSTRACTION
//===============================================================================

// You should not edit this.

#include "pebble.h"
#include "store.h"

// 32 by 24 display

unsigned char vidmode = 0;
unsigned char selector = 100;

extern int seconds;

static unsigned char specials[16] = { /* BACK */ 11, 10, 12, 21, /* ENTR */ 14, 24, 30, 28,
				      /* LOAD */ 22, 25, 10, 14, /* SAVE */ 29, 10, 32, 15
 };

static int changed(int ch, int y, int x) {
  if(ch > 9 || y % 4 == 0) return ch;
  if(selector >= 6 && x > 27 && y > 3) {
	return specials[y * 4 + x % 4 - 8];
  } else {
	return (ch + (y/4) * 10 < 64) ? ch + 36 + (y/4) * 10 : specials[x%4 + (ch % 64) * 4];
  }
}

int get_at(int x, int y) {
  if(x < 24) {
	return changed(x/4 + (y/4)*6, y, x);//main alpha display
  } else {
	if(x > 27 && selector < 6) return changed(selector * 6 + (y/4), y, x);
	if(x > 27 && selector >= 6 && selector < 36) return changed(selector, y/4, x);
	return 59;//blank space
  };
}
