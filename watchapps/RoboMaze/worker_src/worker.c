#include <pebble_worker.h>

#define WORKER_TICKS 0

//travel functions

static void RevTravel(bool val) {

}

static void ForTravel(bool val) {

}

static bool stateLong() {
  return false;
}

//code functions

static void RevTravel0() {
  RevTravel(false);
}

static void ForTravel0() {
  ForTravel(false);
}

static void RevTravel1() {
  RevTravel(true);
}

static void ForTravel1() {
  ForTravel(true);
}

//test functions

static bool For0StrictShort() {
  //test??
  bool x = !stateLong();
  ForTravel0();
  RevTravel1();
  x &= stateLong();//strict short
  ForTravel1();
  RevTravel0();//back at start
  return x;
}

static bool Rev0StrictShort() {
  RevTravel0();
  //test??
  bool x = For0StrictShort();
  ForTravel0();
  return x;
}

static bool For1StrictLong() {
  //test??
  bool x = stateLong();
  ForTravel1();
  RevTravel0();
  x &= !stateLong();//strict long
  ForTravel0();
  RevTravel1();//back at start
  return x;
}

static bool Rev1StrictLong() {
  RevTravel1();
  //test??
  bool x = For1StrictLong();
  ForTravel1();
  return x;
}

//data functions

static bool absorb() {
  return false;
}

static void emit(bool val) {

}

static void decompress() {
    if(Rev0StrictShort()) { 
        RevTravel0();
        if(For1StrictLong()) { emit(false); ForTravel0(); return; }
        ForTravel0();
        if(Rev1StrictLong()) { 
            RevTravel1();
            if(!For0StrictShort()) { emit(true); return; }
            else { ForTravel1(); RevTravel0(); return; }
        }
    } else { RevTravel0(); return; }
}

static void compress() {
    if(For1StrictLong()) { 
        if(For0StrictShort()) {
            if(absorb()) { ForTravel1(); return;}
            else { ForTravel0(); return; }
        } else {
            ForTravel1(); return;
        }
    } else { ForTravel0(); return; }
}   


static uint16_t s_ticks = 0;

static void tick_handler(struct tm *tick_timer, TimeUnits units_changed) {
  // Update value
  s_ticks++;

  // Construct a data packet
  AppWorkerMessage msg_data = {
    .data0 = s_ticks
  };

  // Send the data to the foreground app
  app_worker_send_message(WORKER_TICKS, &msg_data);
}

static void worker_init() {
  // Use the TickTimer Service as a data source
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void worker_deinit() {
  // Stop using the TickTimerService
  tick_timer_service_unsubscribe();
}

int main(void) {
  worker_init();
  worker_event_loop();
  worker_deinit();
}

