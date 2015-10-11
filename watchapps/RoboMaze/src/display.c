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

int get_at(int x, int y) {
  switch(vidmode) {
	case 1: return 59;
	case 2: if(x < 24) {
		return x/4 + (y/4)*6;//main alpha display
	} else {
		if(x > 27 && selector < 6) return selector * 6 + (y/4);
		if(x > 27 && selector >= 6 && selector < 36 && selector%6 == y/4) return (selector%6) * 6 + (y/4);
		return 59;//blank space
	};
	default: return 59;
  }
}
