#include <pebble.h>
#include "clock_window.h"

static Window *s_main_window;

static void init() {
  // create main window element and assign the pointer
  s_main_window = window_create();
  
  // show the window on the watch, with animated=true
  window_stack_push(s_main_window, true);  
  show_clock_window();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}