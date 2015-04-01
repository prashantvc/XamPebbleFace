#include <pebble.h>

#define ANTIALIASING true

static Window *s_main_window;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;

static Layer *s_hands_layer;

static void hands_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	int16_t second_hand_length = bounds.size.w / 2;

	graphics_context_set_antialiased(ctx, ANTIALIASING);

	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	int16_t minute_hand_lenght = 65;
	int32_t minute_angle =  TRIG_MAX_ANGLE * t -> tm_min / 60;
	GPoint minute_hand = {
		.x = (int16_t)(sin_lookup(minute_angle) * (int32_t) minute_hand_lenght/TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(minute_angle) * (int32_t) minute_hand_lenght/TRIG_MAX_RATIO) + center.y
	};

	graphics_context_set_stroke_width(ctx, 4);
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, minute_hand, center);

	int16_t hour_hand_lenght =  45;
	int32_t hour_angle =  (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
	GPoint hour_hand = {
		.x = (int16_t)(sin_lookup(hour_angle) * (int32_t) hour_hand_lenght/TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t) hour_hand_lenght/TRIG_MAX_RATIO) + center.y
	};

	graphics_draw_line(ctx, hour_hand, center);
	
}

static void main_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds (window_layer);
	s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HEX_IMAGE);

	s_bitmap_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
  	layer_add_child(window_layer, s_hands_layer);
}


static void main_window_unload(Window *window){
	bitmap_layer_destroy(s_bitmap_layer);
	gbitmap_destroy(s_bitmap);
	layer_destroy(s_hands_layer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_main_window));
}

static void init() {
	s_main_window = window_create();
	window_set_fullscreen(s_main_window, true);
	window_set_background_color(s_main_window, GColorOxfordBlue);
	window_set_window_handlers (s_main_window, (WindowHandlers){
		.load = main_window_load,
		.unload = main_window_unload
	});

	window_stack_push(s_main_window, true);
	tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick);
}


static void deinit() {
  	tick_timer_service_unsubscribe();
	window_destroy(s_main_window);
}

int main(void) {
	init();
  	app_event_loop();
  	deinit();
}
