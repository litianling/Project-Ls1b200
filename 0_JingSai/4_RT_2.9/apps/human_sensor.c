/*
 * human_sensor.c
 *
 * created: 2022/5/27
 *  author: 
 */

#include "bsp.h"
#include <rtthread.h>
#include "ls1b_gpio.h"

#define printf  printk
#define getch   usb_kbd_getc

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        50
static rt_thread_t human_sensor1 = RT_NULL;

void human_sensor_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    fb_cons_clear();

    int Buzzer_Mode = 0;
    gpio_enable(BUZZER,DIR_OUT);
    gpio_enable(TOUCH,DIR_IN);
    gpio_enable(MAN_S,DIR_IN);

    printf("Alarm when someone is active within 3 meters!\r\n");
    printf("Touch the capacitive sensor to exit the program.\r\n");
    gpio_write(BUZZER,Buzzer_Mode);
    delay_ms(10);
    while(!(gpio_read(TOUCH)))
    {
        Buzzer_Mode = gpio_read(MAN_S);
        gpio_write(BUZZER,Buzzer_Mode);
        delay_ms(10);
    }
    gpio_write(BUZZER,0);

    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
}

int human_sensor(void)
{
    human_sensor1 = rt_thread_create("human_sensor",
                            human_sensor_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(human_sensor1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(human_sensor, human sensor);
