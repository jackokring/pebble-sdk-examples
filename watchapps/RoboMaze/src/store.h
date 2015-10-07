//=================================================================================
// MAIN DEFINES AND INCLUDES
//=================================================================================

// Put your common numbers here.

#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 3
#define NUM_SECOND_MENU_ITEMS 1

#define MAP_STORE 0
#define SW_STORE 1
#define SW_BUTT 2
#define SW_LAP 3
#define PROCESS 4
#define SW_STOP 5

#define SCORE 6
#define HISCORE 7
#define LEVEL 8
#define DIRECTION 9
#define GAME_AB 10
#define CHAF 11

typedef struct {
  uint8_t pos;//xy position
  uint8_t dir;//direction faced, or wanted to move
  uint8_t dpos;//actual move direction
  //note that dir is clockwise/anticlockwise, and dpos is xy relative
  uint8_t is;//a flag to decide type
  uint8_t mode;//actual movement mode
} mover;
