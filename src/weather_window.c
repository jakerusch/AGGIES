#include <pebble.h>
#include "weather_window.h"
#include "steps_window.h"
#include "clock_window.h"

#define KEY_CITY 0
#define KEY_TIME1 1
#define KEY_TEMP1 2
#define KEY_TIME2 3
#define KEY_TEMP2 4
#define KEY_TIME3 5
#define KEY_TEMP3 6
#define KEY_TEMP 7

static Window *s_main_window;
static TextLayer *s_layer1, *s_layer2;
static GFont s_font, s_small_font, s_small_font_bold;
static ActionBarLayer *s_action_bar;
static GBitmap *s_up_bitmap, *s_down_bitmap;
// static Layer *s_canvas_layer;
// static TextLayer *s_weather_layer;
static TextLayer *s_weather_time_1, *s_weather_temp_1, *s_weather_time_2, *s_weather_temp_2, 
  *s_weather_time_3, *s_weather_temp_3, *s_weather_layer_city;
static AppTimer *s_timer;

static void up_click() {
  show_steps_window();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather up_click");
}

static void down_click() {
  show_clock_window();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather down_click");
}

static void go_home() {
  show_clock_window();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather go_home");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_register(300, up_click, NULL); 
  if(s_timer) { app_timer_cancel(s_timer); }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather up_click_handler");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_register(300, down_click, NULL);
  if(s_timer) { app_timer_cancel(s_timer); }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather down_click_handler");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather click_config_provider");
}

// static void canvas_update_proc(Layer *layer, GContext *ctx) {
// //   GRect bounds = layer_get_bounds(layer);
//   ////////////////////////////
//   // draw lines around temp //
//   ////////////////////////////
// //   graphics_draw_line(ctx, GPoint(0, 1*bounds.size.h/7), GPoint(bounds.size.w, 1*bounds.size.h/7));
// //   graphics_draw_line(ctx, GPoint(0, 3*bounds.size.h/7), GPoint(bounds.size.w, 3*bounds.size.h/7));
// //   graphics_draw_line(ctx, GPoint(0, 5*bounds.size.h/7), GPoint(bounds.size.w, 5*bounds.size.h/7));
//   GPoint start = GPoint(0, 10);
//   GPoint end = GPoint(114, 10);
//   graphics_draw_line(ctx, start, end);
// }

static void load_icons(Window *window) {
  // Load icon bitmaps
  s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SHOE_WHITE_ICON);
  s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOCK_WHITE_ICON);
  
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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather load_icons");
}  

static void window_load(Window *window) {
  hide_steps_window();
  hide_clock_window();
  
  // get information about the window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorWhite);   
  
  /////////////////
  // create font //
  /////////////////
  s_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_small_font_bold = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  
  int header = 1;
  int sub_items = 9;
  int total_items = header + sub_items;
   
  // city
  s_weather_layer_city = text_layer_create(GRect(0, 0*bounds.size.h/total_items, bounds.size.w-ACTION_BAR_WIDTH, bounds.size.h/total_items*2));
//   text_layer_set_background_color(s_weather_layer_city, GColorBlack);
//   text_layer_set_text_color(s_weather_layer_city, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer_city, GTextAlignmentCenter);
  text_layer_set_font(s_weather_layer_city, s_font);
  text_layer_set_text(s_weather_layer_city, "Fetching...");
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer_city));
  
  // time 1
  s_weather_time_1 = text_layer_create(GRect(0, 2*bounds.size.h/total_items, bounds.size.w-ACTION_BAR_WIDTH, bounds.size.h/total_items));
//   text_layer_set_background_color(s_weather_time_1, GColorDarkGray);
  text_layer_set_text_alignment(s_weather_time_1, GTextAlignmentCenter);
  text_layer_set_font(s_weather_time_1, s_small_font_bold);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_time_1));      
  
  // temp and conditions 1
  s_weather_temp_1 = text_layer_create(GRect(0, 3*bounds.size.h/total_items, bounds.size.w-ACTION_BAR_WIDTH, bounds.size.h/total_items));
//   text_layer_set_background_color(s_weather_temp_1, GColorDarkGray);
  text_layer_set_text_alignment(s_weather_temp_1, GTextAlignmentCenter);
  text_layer_set_font(s_weather_temp_1, s_small_font);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_temp_1));      
     
  // time 2
  s_weather_time_2 = text_layer_create(GRect(0, 5*bounds.size.h/total_items, bounds.size.w-ACTION_BAR_WIDTH, bounds.size.h/total_items));
//   text_layer_set_background_color(s_weather_time_2, GColorBlack);
//   text_layer_set_text_color(s_weather_time_2, GColorWhite);
  text_layer_set_text_alignment(s_weather_time_2, GTextAlignmentCenter);
  text_layer_set_font(s_weather_time_2, s_small_font_bold);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_time_2));      
  
  // temp and conditions 2
  s_weather_temp_2 = text_layer_create(GRect(0, 6*bounds.size.h/total_items, bounds.size.w-ACTION_BAR_WIDTH, bounds.size.h/total_items));
//   text_layer_set_background_color(s_weather_temp_2, GColorBlack);
//   text_layer_set_text_color(s_weather_temp_2, GColorWhite);
  text_layer_set_text_alignment(s_weather_temp_2, GTextAlignmentCenter);
  text_layer_set_font(s_weather_temp_2, s_small_font);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_temp_2));         
  
  // time 3
  s_weather_time_3 = text_layer_create(GRect(0, 8*bounds.size.h/total_items, bounds.size.w-ACTION_BAR_WIDTH, bounds.size.h/total_items));
//   text_layer_set_background_color(s_weather_time_3, GColorDarkGray);
  text_layer_set_text_alignment(s_weather_time_3, GTextAlignmentCenter);
  text_layer_set_font(s_weather_time_3, s_small_font_bold);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_time_3));      
  
  // temp and conditions 3
  s_weather_temp_3 = text_layer_create(GRect(0, 9*bounds.size.h/total_items, bounds.size.w-ACTION_BAR_WIDTH, bounds.size.h/total_items));
//   text_layer_set_background_color(s_weather_temp_3, GColorDarkGray);
  text_layer_set_text_alignment(s_weather_temp_3, GTextAlignmentCenter);
  text_layer_set_font(s_weather_temp_3, s_small_font);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_temp_3));       

//   ///////////////////////////////////
//   // create canvas layer for lines //
//   ///////////////////////////////////
//   s_canvas_layer = layer_create(bounds);
//   layer_set_update_proc(s_canvas_layer, canvas_update_proc);
//   layer_add_child(window_get_root_layer(window), s_canvas_layer);   

  load_icons(window);
  
  s_timer = app_timer_register(10000, go_home, NULL); 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather s_timer created");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather window_load");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather tick_handler");
}

static void window_unload(Window *window) {
  text_layer_destroy(s_layer1);
  text_layer_destroy(s_layer2);
  action_bar_layer_destroy(s_action_bar);
  gbitmap_destroy(s_up_bitmap);
  gbitmap_destroy(s_down_bitmap);  
  window_destroy(s_main_window);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather window_unload");  
}

// for weather calls
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char city_buf[32];
  
  static char time_buf_1[32];
  static char temp_buf_1[32];
  
  static char time_buf_2[32];
  static char temp_buf_2[32];
  
  static char time_buf_3[32];
  static char temp_buf_3[32];
  
  static char city_layer_buf[32];
  
  static char time_layer_buf_1[32];
  static char temp_layer_buf_1[32];

  static char time_layer_buf_2[32];
  static char temp_layer_buf_2[32];

  static char time_layer_buf_3[32];
  static char temp_layer_buf_3[32];
  
  // Read tuples for data
  Tuple *city_tuple = dict_find(iterator, KEY_CITY);
  
  Tuple *time_tuple_1 = dict_find(iterator, KEY_TIME1);
  Tuple *temp_tuple_1 = dict_find(iterator, KEY_TEMP1);

  Tuple *time_tuple_2 = dict_find(iterator, KEY_TIME2);
  Tuple *temp_tuple_2 = dict_find(iterator, KEY_TEMP2);

  Tuple *time_tuple_3 = dict_find(iterator, KEY_TIME3);
  Tuple *temp_tuple_3 = dict_find(iterator, KEY_TEMP3);
  
  // If all data is available, use it
  if(city_tuple && time_tuple_1 && temp_tuple_1 && time_tuple_2 && temp_tuple_2 && time_tuple_3 && temp_tuple_3) {
    
    snprintf(city_buf, sizeof(city_buf), "%s", city_tuple->value->cstring);
    
    snprintf(time_buf_1, sizeof(time_buf_1), "%s", time_tuple_1->value->cstring);
    snprintf(temp_buf_1, sizeof(temp_buf_1), "%s", temp_tuple_1->value->cstring);
    
    snprintf(time_buf_2, sizeof(time_buf_2), "%s", time_tuple_2->value->cstring);
    snprintf(temp_buf_2, sizeof(temp_buf_2), "%s", temp_tuple_2->value->cstring);
    
    snprintf(time_buf_3, sizeof(time_buf_3), "%s", time_tuple_3->value->cstring);
    snprintf(temp_buf_3, sizeof(temp_buf_3), "%s", temp_tuple_3->value->cstring);
        
    //////////
    // city //
    //////////
    snprintf(city_layer_buf, sizeof(city_layer_buf), "%s", city_buf);
    text_layer_set_text(s_weather_layer_city, city_layer_buf);
    
    ////////////
    // time 1 //
    ////////////
    snprintf(time_layer_buf_1, sizeof(time_layer_buf_1), "%s", time_buf_1);
    text_layer_set_text(s_weather_time_1, time_layer_buf_1);
    
    ////////////
    // temp 1 //
    ////////////
    snprintf(temp_layer_buf_1, sizeof(temp_layer_buf_1), "%s", temp_buf_1);
    text_layer_set_text(s_weather_temp_1, temp_layer_buf_1);
    
    ////////////
    // time 2 //
    ////////////
    snprintf(time_layer_buf_2, sizeof(time_layer_buf_2), "%s", time_buf_2);
    text_layer_set_text(s_weather_time_2, time_layer_buf_2);
    
    ////////////
    // temp 2 //
    ////////////
    snprintf(temp_layer_buf_2, sizeof(temp_layer_buf_2), "%s", temp_buf_2);
    text_layer_set_text(s_weather_temp_2, temp_layer_buf_2);

    ////////////
    // time 3 //
    ////////////
    snprintf(time_layer_buf_3, sizeof(time_layer_buf_3), "%s", time_buf_3);
    text_layer_set_text(s_weather_time_3, time_layer_buf_3);
    
    ////////////
    // temp 3 //
    ////////////
    snprintf(temp_layer_buf_3, sizeof(temp_layer_buf_3), "%s", temp_buf_3);
    text_layer_set_text(s_weather_temp_3, temp_layer_buf_3);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Weather inbox_received_callback");
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void show_weather_window(void) {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(s_main_window, true);
    
  // register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler); 
  
  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);  
  
  // Open AppMessage for weather callbacks
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);   
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather show_weather_window");
}

void hide_weather_window(void) {
  window_stack_remove(s_main_window, true);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather hide_weather_window");
}