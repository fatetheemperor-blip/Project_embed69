#ifndef EVENT_H
#define EVENT_H

#include <lvgl.h>
#include "ui.h"
#include "images.h"
#include <ESP32Servo.h>
#if defined(EEZ_FOR_LVGL)
#include <eez/flow/lvgl_api.h>
#endif
#if !defined(EEZ_FOR_LVGL)
#include "screens.h"
#endif
#ifdef __cplusplus
extern "C"
{
#endif

    void event_handler(lv_event_t*e);
    void timer_setup(lv_timer_t *timer);
    void countdoen(int hours, int minutes);
    void motor_stop();
    void motor_start();
    void gear_motor_on();
    void gear_motor_off();
    void init_servo();
    void read_barrier_obj();
    void gear_motor_turn_right();
    void gear_motor_turn_left();
    void barrier_obj();
    void robot_stop_all();

#ifdef __cplusplus
}
#endif
#endif
