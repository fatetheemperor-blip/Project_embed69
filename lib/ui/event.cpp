#include "event.h"
#include <Arduino.h>

bool show_robot1 = true;
int input_step = 0;          
lv_obj_t *current_ta = NULL; 
lv_timer_t *countdown_timer = NULL;
int total_remaining_seconds = 0;
int initial_total_seconds = 0;

void event_handler(lv_event_t*e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if(obj==objects.powerbt && code == LV_EVENT_CLICKED)
    {
        if(show_robot1)
        {
            lv_img_set_src(objects.robot_wakeup, &img_robot);
            lv_label_set_text(objects.power_label,"STOP");
            lv_obj_set_style_bg_color(objects.powerbt, lv_color_hex(0xff0000), 0);
            show_robot1 = false;

            if (total_remaining_seconds > 0 && countdown_timer == NULL)
            {
                countdown_timer = lv_timer_create(timer_setup, 1000, NULL);
            }

        }
        else
        {
            lv_img_set_src(objects.robot_wakeup, &img_robot_sleep);
            lv_label_set_text(objects.power_label,"START");
            lv_obj_set_style_bg_color(objects.powerbt, lv_color_hex(0x22ff00), 0);
            show_robot1 = true;

            if (countdown_timer != NULL)
            {
                lv_timer_del(countdown_timer);
                countdown_timer = NULL;
            }
        }
    }
    else if (obj == objects.timerbt && code == LV_EVENT_CLICKED)
    {
        input_step = 0;
        current_ta = objects.hour_display;
        lv_textarea_set_text(objects.hour_display, "");
        lv_textarea_set_text(objects.minute_display, "");
        lv_scr_load(objects.timer_page);
    }
    else if (obj == objects.timer_numval)
    {
        const char *txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));
        if (txt == NULL) return;
        if (current_ta == NULL) current_ta = objects.hour_display;

        if (code == LV_EVENT_LONG_PRESSED)
        {
            if (strcmp(txt, "Delete") == 0)
            {
                lv_textarea_set_text(objects.hour_display, "");
                lv_textarea_set_text(objects.minute_display, "");

                input_step = 0;
                current_ta = objects.hour_display;
            }
        }
        else if (code == LV_EVENT_VALUE_CHANGED)
        {
            if (strcmp(txt, "Delete") == 0)
            {
                lv_textarea_del_char(current_ta);
            }
            else if (strcmp(txt, "Enter") == 0)
            {
                if (input_step == 0)
                {
                    input_step = 1;
                    current_ta = objects.minute_display;
                    lv_textarea_set_text(current_ta, "");
                }
                else if (input_step == 1)
                {
                    int hours = atoi(lv_textarea_get_text(objects.hour_display));
                    int mins = atoi(lv_textarea_get_text(objects.minute_display));

                    if (mins > 60) mins = 60;
                    if (hours == 0 && mins == 0) 
                    {
                        return;
                    }
                    countdoen(hours, mins);

                    input_step = 0;
                    current_ta = objects.hour_display;
                    lv_scr_load(objects.main);
                }
            }
            else
            {
                if (input_step == 1)
                {
                    String current_str = String(lv_textarea_get_text(current_ta)) + txt;
                    if (current_str.toInt() > 60) 
                    {
                        lv_textarea_set_text(current_ta, "60");
                        return;
                    }
                }
                lv_textarea_add_text(current_ta, txt);
            }
        }
    }
}

void timer_setup(lv_timer_t *timer)
{
    if (total_remaining_seconds > 0) 
    {
        total_remaining_seconds--;
        int h = total_remaining_seconds / 3600;
        int m = (total_remaining_seconds % 3600) / 60;
        int s = total_remaining_seconds % 60;
        char buf[32];
        snprintf(buf, sizeof(buf), "%02d : %02d : %02d", h, m, s);
        lv_label_set_text(objects.time_val, buf);
        if(initial_total_seconds > 0)
        {
            int arc_val = (total_remaining_seconds * 100) / initial_total_seconds;
            lv_arc_set_value(objects.timer_arc, arc_val);
            if(arc_val <= 10)
            {
                lv_obj_set_style_arc_color(objects.timer_arc, lv_color_hex(0xff0000), LV_PART_INDICATOR);
            }
        }
    }
    else if (total_remaining_seconds == 0)
    {
        lv_label_set_text(objects.time_val, "00 : 00 : 00");
        lv_arc_set_value(objects.timer_arc, 0);

        if (countdown_timer != NULL)
        {
            lv_timer_del(countdown_timer);
            countdown_timer = NULL;
        }

        show_robot1 = true;
        lv_img_set_src(objects.robot_wakeup, &img_robot_sleep);
        lv_label_set_text(objects.power_label, "START");
        lv_obj_set_style_bg_color(objects.powerbt, lv_color_hex(0x22ff00), 0);
    }
}

void countdoen(int hours, int minutes)
{
    if (minutes > 0) 
    {
        minutes -= 1;
    } 
    else if (hours > 0) 
    {
        hours -= 1;
        minutes = 59;
    }

    total_remaining_seconds = hours * 3600 + minutes * 60 + 60;
    initial_total_seconds = total_remaining_seconds;

    lv_obj_clear_flag(objects.timer_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_range(objects.timer_arc, 0, 100);
    lv_arc_set_value(objects.timer_arc, 100);
    lv_obj_set_style_arc_color(objects.timer_arc, lv_color_hex(0x2196F3), LV_PART_INDICATOR);

    int init_h = total_remaining_seconds / 3600;
    int init_m = (total_remaining_seconds % 3600) / 60;
    int init_s = total_remaining_seconds % 60;

    char buf[32];
    snprintf(buf, sizeof(buf), "%02d : %02d : %02d", init_h, init_m, init_s);
    lv_label_set_text(objects.time_val, buf);
    if (countdown_timer != NULL) 
    {
        lv_timer_del(countdown_timer);
        countdown_timer = NULL;
    }
    if (!show_robot1)
    {
        countdown_timer = lv_timer_create(timer_setup, 1000, NULL);
    }
}


