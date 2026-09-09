#ifndef EVENT_H
#define EVENT_H

#include <lvgl.h>
#include "ui.h"
#include "images.h"
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
    void battery_status(lv_timer_t *timer);

#ifdef __cplusplus
}
#endif
#endif
