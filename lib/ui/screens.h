#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_TIMER_PAGE = 2,
    _SCREEN_ID_LAST = 2
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *timer_page;
    lv_obj_t *timerbt;
    lv_obj_t *battery_bar;
    lv_obj_t *battery_val;
    lv_obj_t *powerbt;
    lv_obj_t *power_label;
    lv_obj_t *robot_wakeup;
    lv_obj_t *timer_arc;
    lv_obj_t *time_val;
    lv_obj_t *time_set;
    lv_obj_t *timer_numval;
    lv_obj_t *hour_display;
    lv_obj_t *minute_display;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_timer_page();
void tick_screen_timer_page();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/