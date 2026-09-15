#include "event.h"
#include <Arduino.h>
#include <ESP32Servo.h>

bool show_robot1 = true;
int input_step = 0;
lv_obj_t *current_ta = NULL;
lv_timer_t *countdown_timer = NULL;
int total_remaining_seconds = 0;
int initial_total_seconds = 0;
float duration, cm;

Servo scan_servo;

// *หมายเหตุ: ถ้าขา 13 ชนกับจอ ให้แก้เป็นขาอื่นเช่น 27 นะครับ
#define SERVO_PIN 13 
#define SERVO_STOP 89
#define SERVO_RIGHT 20  
#define SERVO_LEFT 160

void init_servo()
{
    // จอง Timer แค่ตัวเดียวพอ ป้องกันไปทับ Timer ของ Ultrasonic
    ESP32PWM::allocateTimer(0);

    scan_servo.setPeriodHertz(50);
    if (!scan_servo.attached()) scan_servo.attach(SERVO_PIN, 500, 2400);
    scan_servo.write(SERVO_STOP);
    delay(300);
    if (scan_servo.attached()) scan_servo.detach();
}

void robot_stop_all()
{
    // มอเตอร์หลัก
    digitalWrite(17, LOW);
    digitalWrite(16, LOW);

    // มอเตอร์เกียร์/เลี้ยว
    digitalWrite(21, LOW);
    digitalWrite(14, LOW);
    digitalWrite(25, LOW);
    digitalWrite(33, LOW);

    // ตัดสัญญาณ Servo อย่างปลอดภัย
    if (scan_servo.attached()) scan_servo.detach();
}

void event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if(obj == objects.powerbt && code == LV_EVENT_CLICKED)
    {
        if(show_robot1 && total_remaining_seconds <= 0)
        {
            return;
        }

        if(show_robot1)
        {
            show_robot1 = false;

            lv_img_set_src(objects.robot_wakeup, &img_robot);
            lv_label_set_text(objects.power_label, "STOP");
            lv_obj_set_style_bg_color(
                objects.powerbt,
                lv_color_hex(0xff0000),
                0
            );

            // เริ่มมอเตอร์ทั้งหมด
            motor_start();
            gear_motor_on();

            // เริ่มนับเวลา ถ้ายังไม่มี timer
            if(countdown_timer == NULL && total_remaining_seconds > 0)
            {
                countdown_timer = lv_timer_create(timer_setup, 1000, NULL);
            }
        }
        else
        {
            show_robot1 = true;

            // หยุดทุกอย่างทันที
            robot_stop_all();

            // ลบ countdown timer
            if(countdown_timer != NULL)
            {
                lv_timer_del(countdown_timer);
                countdown_timer = NULL;
            }

            lv_img_set_src(objects.robot_wakeup, &img_robot_sleep);
            lv_label_set_text(objects.power_label, "START");
            lv_obj_set_style_bg_color(
                objects.powerbt,
                lv_color_hex(0x22ff00),
                0
            );
        }
    }

    else if (obj == objects.mainbt && code == LV_EVENT_CLICKED)
    {
        lv_scr_load(objects.main);
    }

    else if (obj == objects.timerbt && code == LV_EVENT_CLICKED)
    {
        input_step = 0;
        current_ta = objects.hour_display;
        lv_textarea_set_text(objects.hour_display, "");
        lv_textarea_set_text(objects.minute_display, "");
        lv_scr_load(objects.timer_page);
    }

    else if(obj == objects.hour_display && code == LV_EVENT_CLICKED)
    {
        current_ta = objects.hour_display;
        input_step = 0;
    }

    else if(obj == objects.minute_display && code == LV_EVENT_CLICKED)
    {
        current_ta = objects.minute_display;
        input_step = 1;
    }

    else if (obj == objects.timer_numval)
    {
        const char *txt =
            lv_btnmatrix_get_btn_text(
                obj,
                lv_btnmatrix_get_selected_btn(obj)
            );

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
                int hours = atoi(
                    lv_textarea_get_text(objects.hour_display)
                );

                int mins = atoi(
                    lv_textarea_get_text(objects.minute_display)
                );

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

            else
            {
                if (input_step == 1)
                {
                    String current_str =
                        String(lv_textarea_get_text(current_ta)) + txt;

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

        snprintf(
            buf,
            sizeof(buf),
            "%02d : %02d : %02d",
            h,
            m,
            s
        );

        lv_label_set_text(objects.time_val, buf);

        if(initial_total_seconds > 0)
        {
            int arc_val =
                (total_remaining_seconds * 100)
                / initial_total_seconds;

            lv_arc_set_value(objects.timer_arc, arc_val);

            if(arc_val <= 10)
            {
                lv_obj_set_style_arc_color(
                    objects.timer_arc,
                    lv_color_hex(0xff0000),
                    LV_PART_INDICATOR
                );
            }
        }
    }

    else if (total_remaining_seconds == 0)
    {
        show_robot1 = true;

        robot_stop_all();

        if (countdown_timer != NULL)
        {
            lv_timer_del(countdown_timer);
            countdown_timer = NULL;
        }

        lv_label_set_text(objects.time_val, "00 : 00 : 00");
        lv_arc_set_value(objects.timer_arc, 0);

        lv_img_set_src(
            objects.robot_wakeup,
            &img_robot_sleep
        );

        lv_label_set_text(objects.power_label, "START");

        lv_obj_set_style_bg_color(
            objects.powerbt,
            lv_color_hex(0x22ff00),
            0
        );
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

    total_remaining_seconds =
        hours * 3600 + minutes * 60 + 60;

    initial_total_seconds =
        total_remaining_seconds;

    lv_obj_clear_flag(
        objects.timer_arc,
        LV_OBJ_FLAG_CLICKABLE
    );

    lv_arc_set_range(objects.timer_arc, 0, 100);
    lv_arc_set_value(objects.timer_arc, 100);

    lv_obj_set_style_arc_color(
        objects.timer_arc,
        lv_color_hex(0x2196F3),
        LV_PART_INDICATOR
    );

    int init_h = total_remaining_seconds / 3600;
    int init_m = (total_remaining_seconds % 3600) / 60;
    int init_s = total_remaining_seconds % 60;

    char buf[32];

    snprintf(
        buf,
        sizeof(buf),
        "%02d : %02d : %02d",
        init_h,
        init_m,
        init_s
    );

    lv_label_set_text(objects.time_val, buf);

    if (countdown_timer != NULL)
    {
        lv_timer_del(countdown_timer);
        countdown_timer = NULL;
    }

    if (!show_robot1)
    {
        motor_start();
        gear_motor_on();

        countdown_timer =
            lv_timer_create(timer_setup, 1000, NULL);
    }
}

/*
 * มอเตอร์หลัก
 */
void motor_stop()
{
    digitalWrite(17, LOW);
    digitalWrite(16, LOW);
}

void motor_start()
{
    digitalWrite(17, HIGH);
    digitalWrite(16, LOW);
}

/*
 * มอเตอร์เกียร์
 */
void gear_motor_on()
{
    digitalWrite(21, LOW);
    digitalWrite(14, HIGH);
    digitalWrite(25, LOW);
    digitalWrite(33, HIGH);
}

void gear_motor_off()
{
    digitalWrite(21, LOW);
    digitalWrite(14, LOW);
    digitalWrite(25, LOW);
    digitalWrite(33, LOW);

    if (scan_servo.attached()) scan_servo.detach();
}

/*
 * อ่าน HC-SR04
 */
void read_barrier_obj()
{
    digitalWrite(22, LOW);
    delayMicroseconds(2);

    digitalWrite(22, HIGH);
    delayMicroseconds(10);

    digitalWrite(22, LOW);

    pinMode(35, INPUT);

    duration = pulseIn(35, HIGH, 25000);

    if (duration == 0.0)
    {
        cm = 999.0;
    }
    else
    {
        cm = (duration / 29.0) / 2.0;
    }
}

/*
 * หมุนหุ่นไปทางขวา
 */
void gear_motor_turn_right()
{
    digitalWrite(21, LOW);
    digitalWrite(14, HIGH);
    digitalWrite(25, HIGH);
    digitalWrite(33, LOW);
}

/*
 * หมุนหุ่นไปทางซ้าย
 */
void gear_motor_turn_left()
{
    digitalWrite(21, HIGH);
    digitalWrite(14, LOW);
    digitalWrite(25, HIGH);
    digitalWrite(33, LOW);
}

/*
 * ตรวจสิ่งกีดขวาง
 */
void barrier_obj()
{
    // ถ้าหน้าจอแสดงว่าเป็น STOP หรือ ยังไม่ได้ตั้งเวลา จะหยุดหุ่น
    if (show_robot1 || countdown_timer == NULL)
    {
        robot_stop_all();
        return;
    }

    read_barrier_obj();

    /*
     * พบสิ่งกีดขวางในระยะ <= 50 cm
     */
    if (cm <= 50.0)
    {
        gear_motor_off();

        if (show_robot1 || countdown_timer == NULL) { robot_stop_all(); return; }

        // ============================================================
        // 1. หมุนทิศทางที่ 1 (125 องศา, 260ms) -> หยุดอ่านค่า
        // ============================================================
        if (!scan_servo.attached()) scan_servo.attach(SERVO_PIN, 500, 2400);
        scan_servo.write(125);
        vTaskDelay(pdMS_TO_TICKS(260));

        if (scan_servo.attached()) scan_servo.detach();
        vTaskDelay(pdMS_TO_TICKS(150));

        read_barrier_obj();
        float left_cm = cm;

        // ============================================================
        // 2. หมุนดึงกลับมาที่เดิม (65 องศา, 300ms) -> หยุดตรงกลาง
        // ============================================================
        if (!scan_servo.attached()) scan_servo.attach(SERVO_PIN, 500, 2400);
        scan_servo.write(65);
        vTaskDelay(pdMS_TO_TICKS(300));

        if (scan_servo.attached()) scan_servo.detach();
        vTaskDelay(pdMS_TO_TICKS(200));

        if (show_robot1 || countdown_timer == NULL) { robot_stop_all(); return; }

        // ============================================================
        // 3. หมุนทิศทางที่ 2 (65 องศา, 250ms) -> หยุดอ่านค่า
        // ============================================================
        if (!scan_servo.attached()) scan_servo.attach(SERVO_PIN, 500, 2400);
        scan_servo.write(65);
        vTaskDelay(pdMS_TO_TICKS(250));

        if (scan_servo.attached()) scan_servo.detach();
        vTaskDelay(pdMS_TO_TICKS(150));

        read_barrier_obj();
        float right_cm = cm;

        // ============================================================
        // 4. หมุนดึงกลับมาที่เดิม (125 องศา, 310ms) -> หยุดตรงกลาง
        // ============================================================
        if (!scan_servo.attached()) scan_servo.attach(SERVO_PIN, 500, 2400);
        scan_servo.write(125);
        vTaskDelay(pdMS_TO_TICKS(310));

        if (scan_servo.attached()) scan_servo.detach();
        vTaskDelay(pdMS_TO_TICKS(200));

        if (show_robot1 || countdown_timer == NULL) { robot_stop_all(); return; }

        // ============================================================
        // เปรียบเทียบระยะทางเพื่อเลี้ยว
        // ============================================================
        if (right_cm > 50.0 && right_cm >= left_cm)
        {
            gear_motor_turn_right();
            vTaskDelay(pdMS_TO_TICKS(650));
        }
        else if (left_cm > 50.0 && left_cm > right_cm)
        {
            gear_motor_turn_left();
            vTaskDelay(pdMS_TO_TICKS(650));
        }
        else
        {
            // ทางตันทั้งสองฝั่ง -> กลับตัว
            gear_motor_turn_right();
            vTaskDelay(pdMS_TO_TICKS(1200));
        }

        gear_motor_off();
        vTaskDelay(pdMS_TO_TICKS(50));

        if (!show_robot1 && countdown_timer != NULL)
        {
            gear_motor_on();
        }
        else
        {
            robot_stop_all();
        }
    }
    else
    {
        if (!show_robot1 && countdown_timer != NULL)
        {
            gear_motor_on();
        }
        else
        {
            robot_stop_all();
        }
    }
}