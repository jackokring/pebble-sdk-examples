//===============================================================================
// THE CONSOLE ENTRY ABSTRACTION
//===============================================================================

// You should not edit this.

#include "pebble.h"
#include "store.h"

#ifdef BULK_BUILD

// 32 by 24 display

bool vidmode = false;
int selector = 127;
unsigned char buffer[8];
static int cur = 0;

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
  selector = 127;//superflewass!!
  for(int i = 0; i < 8; ++i) {
    buffer[i] = 60;
  }
  cur = 0;
  vidmode = true;
  pause = true;//automate indication
}

int valid() {
  return vidmode?0:cur;
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
	if(y > 21 && x >= 7 && x <= 8 + 8) {
		if(y == 23 && x >= 8 && x < 8 + 8) {
			return buffer[x - 8];//the input string
		}
		else return 59;
	} else return changed(x/4 + (y/4)*6, y, x);//main alpha display
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
	if(x > 27 && selector >= 6 && selector < 10 + 6) return changed(selector, y/4, x);
	return 59;//blank space
  };
}

static void convert() {
  int sel = selector - 6;
  if(sel >= 36) {
    if(sel - 36 < 10) sel -= 36;//number
    else sel -= 10;//symbol
  }
  if(sel == 64) {//BACK
    buffer[cur--] = (unsigned char)60;
    if(cur < 0) cur = 0;//delete back
    return;
  }
  if(sel == 65) {//ENTR
    vidmode = false;
    return;
  }
  buffer[cur++] = sel;
  if(sel >= 66) {//duals
    --cur;
    buffer[cur++] = doubles[(sel - 66) * 2];//first
    if(cur < 8) buffer[cur++] = doubles[(sel - 66) * 2 + 1];//second
  } 
  if(cur >= 8) cur = 7;
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
  if(selector < 6) selector = 6 + selector * 6 + x;//column
  else if(selector < 6 + 10) selector = 36 + 6 + (selector - 6) + 10 * x;//higher selector
  if(selector == 127) selector = x;//row
  if(selector >= 10 + 6) convert();
}

#endif
