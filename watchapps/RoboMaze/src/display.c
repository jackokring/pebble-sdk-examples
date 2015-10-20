//===============================================================================
// THE CONSOLE ENTRY ABSTRACTION
//===============================================================================

// You should not edit this.

#include "pebble.h"
#include "store.h"

// 32 by 24 display

bool vidmode = false;
unsigned char selector = 127;

extern bool pause;

static unsigned char specials[8] = { /* BACK */ 11, 10, 12, 21, /* ENTR */ 14, 24, 30, 28 };

//TH HE AN RE ER IN ON AT ND ST ES EN OF TE ED OR TI HI AS TO
//AN AS AT ED EN ER ES HE HI IN ND OF ON OR RE ST TE TH TI TO
static unsigned char doubles[40] = {
	10, 24, //AN
	10, 29, //AS
	10, 30, //AT
	14, 13, //ED
	14, 24, //EN
	14, 28, //ER
	
	12, 29, //ES
	17, 14, //HE
	17, 18, //HI
	18, 24, //IN

	24, 13, //ND
	25, 15, //OF
	25, 24, //ON
	25, 28, //OR
	28, 14, //RE
	29, 30, //ST

	30, 14, //TE
	30, 17, //TH
	30, 18, //TI
	30, 25  //TO
};

void entry() {
  selector = 127;
  vidmode = true;
  pause = true;//automate indication
}

static int changed(int ch, int y, int x) {
  if(ch > 9 || y % 4 == 0) return ch;
  if(selector >= 6 && x > 27 && y > 3) {//last two specials rows
	return doubles[(y - 4) * 20 + (x%4)/2 + selector * 2];
  } else {
	return (ch + (y/4) * 10 < 64) ? ch + 36 + (y/4) * 10 : specials[x%4 + (ch % 64) * 4];
  }
}

int get_at(int x, int y) {
  if(x < 24) {
	return changed(x/4 + (y/4)*6, y, x);//main alpha display
  } else {
	if(x > 27 && selector < 6) return changed(selector * 6 + (y/4), y, x);
	if(x > 24 && x < 27) {
		int off = 0;
		if(y == 6 || y == 11 || y == 12 || y == 19) return 59;
		if(y > 6) off = 1;
		if(y > 11) off = 3;
		if(y > 19) off = 4;
		return doubles[(y - off) * 2 + x%2];
	} 
	if(x > 27 && selector >= 6 && selector < 36 + 6) return changed(selector, y/4, x);
	return 59;//blank space
  };
}

static void convert() {
  //TODO
  selector = 127;
}

void click_display(ButtonId b, bool single) {
  int x = 0;
  if(b == BUTTON_ID_DOWN) {
    x = 4;
  } else if(b == BUTTON_ID_SELECT) {
    x = 2;
  }
  if(!single) {
    x += 1;
  }
  if(selector == 127) selector = x;//row
  if(selector < 6) selector = 6 + selector * 6 + x;//column
  if(selector < 6 + 10) selector = 36 + 6 + (selector - 6) * x;//higher selector
  if(selector >= 36 + 6 + 6 * 10) convert();
}
