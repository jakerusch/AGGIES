#include <pebble.h>
#include "weather_window.h"
#include "steps_window.h"
#include "clock_window.h"

static Window *s_main_window;
static TextLayer *s_layer;
static GFont s_font;
static ActionBarLayer *s_action_bar;
static GBitmap *s_up_bitmap, *s_down_bitmap;
static AppTimer *s_timer;

// static double step_count;
// static int step_goal=10000;
// static char *char_current_steps;

static void load_weather() {
  show_weather_window();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps load_weather");
}

static void load_clock() {
  show_clock_window();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps load_clock");
}

static void go_home() {
  show_clock_window();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps go_home");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_register(300, load_clock, NULL);
  if(s_timer) { app_timer_cancel(s_timer); }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps up_click_handler");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_register(300, load_weather, NULL);
  if(s_timer) { app_timer_cancel(s_timer); }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps down_click_handler");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps click_config_provider");
}

static void load_icons(Window *window) {
  // Load icon bitmaps
  s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOCK_WHITE_ICON);
  s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_WHITE_ICON);
  
  // Create ActionBarLayer
  s_action_bar = action_bar_layer_create();  
  
  // Set the icons
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_up_bitmap);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_down_bitmap);
  
  // animate button press
  action_bar_layer_set_icon_press_animation(s_action_bar, BUTTON_ID_UP, ActionBarLayerIconPressAnimationMoveDown);
  action_bar_layer_set_icon_press_animation(s_action_bar, BUTTON_ID_DOWN, ActionBarLayerIconPressAnimationMoveUp);  
  
  // Add to Window
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps load_icons");
}  

static void window_load(Window *window) {
  hide_clock_window();
  hide_weather_window();
  
  // get information about the window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorWhite);
  
  /////////////////
  // create font //
  /////////////////
  s_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
   
  /////////////////////////////////
  // create text layer for steps //
  /////////////////////////////////
  s_layer = text_layer_create(GRect(0, 120, bounds.size.w-ACTION_BAR_WIDTH, 30));
  text_layer_set_background_color(s_layer, GColorClear);
  text_layer_set_text_color(s_layer, GColorBlack);
  text_layer_set_text_alignment(s_layer, GTextAlignmentCenter);
  text_layer_set_font(s_layer, s_font);
  text_layer_set_text(s_layer, "-");
  layer_add_child(window_layer, text_layer_get_layer(s_layer));  
  
  load_icons(window);
  
  s_timer = app_timer_register(10000, go_home, NULL); 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps s_timer created");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps window_load");
}

// registers health update events
static void health_handler(HealthEventType event, void *context) {
  if(event==HealthEventMovementUpdate) {
//     step_count = (double)health_service_sum_today(HealthMetricStepCount);
    
    // write to char_current_steps variable
    static char health_buf[32];
    snprintf(health_buf, sizeof(health_buf), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    APP_LOG(APP_LOG_LEVEL_INFO, "%d", (int)health_service_sum_today(HealthMetricStepCount));
//     char_current_steps = health_buf;
    
    // force update to circle
    text_layer_set_text(s_layer, health_buf);
    
    APP_LOG(APP_LOG_LEVEL_INFO, "health_handler completed");
  }
}

static void window_unload(Window *window) {
  app_timer_cancel(s_timer);
  text_layer_destroy(s_layer);
  action_bar_layer_destroy(s_action_bar);
  gbitmap_destroy(s_up_bitmap);
  gbitmap_destroy(s_down_bitmap);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps window_unload");
}

void show_steps_window(void) {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(s_main_window, true);
  
  // subscribe to health events 
  health_service_events_subscribe(health_handler, NULL); 
  // force initial update
  health_handler(HealthEventMovementUpdate, NULL); 
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps show_steps_window");
}

void hide_steps_window(void) {
  window_stack_remove(s_main_window, true);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Steps hide_steps_window");
}
