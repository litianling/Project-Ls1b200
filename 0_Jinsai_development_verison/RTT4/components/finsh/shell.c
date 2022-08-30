/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-04-30     Bernard      the first version for FinSH
 * 2006-05-08     Bernard      change finsh thread stack to 2048
 * 2006-06-03     Bernard      add support for skyeye
 * 2006-09-24     Bernard      remove the code related with hardware
 * 2010-01-18     Bernard      fix down then up key bug.
 * 2010-03-19     Bernard      fix backspace issue and fix device read in shell.
 * 2010-04-01     Bernard      add prompt output when start and remove the empty history
 * 2011-02-23     Bernard      fix variable section end issue of finsh shell
 *                             initialization when use GNU GCC compiler.
 * 2016-11-26     armink       add password authentication
 * 2018-07-02     aozima       add custom prompt support.
 */

#include "rthw.h"
#include "bsp.h"
#ifdef RT_USING_FINSH

#include "finsh.h"
#include "shell.h"
#include "../../../gui/gui_desktop.h"

#ifdef FINSH_USING_MSH
#include "msh.h"
#endif

#ifdef _WIN32
#include <stdio.h> /* for putchar */
#endif

/* finsh thread */
#ifndef RT_USING_HEAP
static struct rt_thread finsh_thread;
ALIGN(RT_ALIGN_SIZE)
static char finsh_thread_stack[FINSH_THREAD_STACK_SIZE];
struct finsh_shell _shell;
#endif

/* finsh symtab */
#ifdef FINSH_USING_SYMTAB
struct finsh_syscall *_syscall_table_begin  = NULL;
struct finsh_syscall *_syscall_table_end    = NULL;
struct finsh_sysvar *_sysvar_table_begin    = NULL;
struct finsh_sysvar *_sysvar_table_end      = NULL;
#endif

struct finsh_shell *shell;
static char *finsh_prompt_custom = RT_NULL;

#define USE_PICTURE_SOC 1
#define x_max   800
#define y_max   480
#define devide_lie      4   // 800/4 = 200
#define devide_hang     5   // 480/5 = 96
int show_background = 1;

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__))
struct finsh_syscall* finsh_syscall_next(struct finsh_syscall* call)
{
    unsigned int *ptr;
    ptr = (unsigned int*) (call + 1);
    while ((*ptr == 0) && ((unsigned int*)ptr < (unsigned int*) _syscall_table_end))
        ptr ++;

    return (struct finsh_syscall*)ptr;
}

struct finsh_sysvar* finsh_sysvar_next(struct finsh_sysvar* call)
{
    unsigned int *ptr;
    ptr = (unsigned int*) (call + 1);
    while ((*ptr == 0) && ((unsigned int*)ptr < (unsigned int*) _sysvar_table_end))
        ptr ++;

    return (struct finsh_sysvar*)ptr;
}
#endif /* defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__)) */

#ifdef RT_USING_HEAP
int finsh_set_prompt(const char * prompt)
{
    if(finsh_prompt_custom)
    {
        rt_free(finsh_prompt_custom);
        finsh_prompt_custom = RT_NULL;
    }

    /* strdup */
    if(prompt)
    {
        finsh_prompt_custom = (char *)rt_malloc(strlen(prompt)+1);
        if(finsh_prompt_custom)
        {
            strcpy(finsh_prompt_custom, prompt);
        }
    }

    return 0;
}
#endif /* RT_USING_HEAP */

#if defined(RT_USING_DFS)
#include "dfs_posix.h"
#endif /* RT_USING_DFS */

const char *finsh_get_prompt()
{
#define _MSH_PROMPT "msh "
#define _PROMPT     "finsh "
    static char finsh_prompt[RT_CONSOLEBUF_SIZE + 1] = {0};

    /* check prompt mode */
    if (!shell->prompt_mode)
    {
        finsh_prompt[0] = '\0';
        return finsh_prompt;
    }

    if(finsh_prompt_custom)
    {
        strncpy(finsh_prompt, finsh_prompt_custom, sizeof(finsh_prompt)-1);
        return finsh_prompt;
    }

#ifdef FINSH_USING_MSH
    if (msh_is_used()) strcpy(finsh_prompt, _MSH_PROMPT);
    else
#endif
        strcpy(finsh_prompt, _PROMPT);

#if defined(RT_USING_DFS) && defined(DFS_USING_WORKDIR)
    /* get current working directory */
    getcwd(&finsh_prompt[rt_strlen(finsh_prompt)], RT_CONSOLEBUF_SIZE - rt_strlen(finsh_prompt));
#endif

    strcat(finsh_prompt, ">");

    return finsh_prompt;
}

/**
 * @ingroup finsh
 *
 * This function get the prompt mode of finsh shell.
 *
 * @return prompt the prompt mode, 0 disable prompt mode, other values enable prompt mode.
 */
rt_uint32_t finsh_get_prompt_mode(void)
{
    RT_ASSERT(shell != RT_NULL);
    return shell->prompt_mode;
}

/**
 * @ingroup finsh
 *
 * This function set the prompt mode of finsh shell.
 *
 * The parameter 0 disable prompt mode, other values enable prompt mode.
 *
 * @param prompt the prompt mode
 */
void finsh_set_prompt_mode(rt_uint32_t prompt_mode)
{
    RT_ASSERT(shell != RT_NULL);
    shell->prompt_mode = prompt_mode;
}

static int finsh_getchar(void)
{
#if defined(RT_USING_DEVICE) && defined(RT_USING_SERIAL)
  #ifdef RT_USING_POSIX
    return getchar();
  #else
    char ch = 0;

    RT_ASSERT(shell != RT_NULL);
    while (rt_device_read(shell->device, -1, &ch, 1) != 1)
        rt_sem_take(&shell->rx_sem, RT_WAITING_FOREVER);

    return (int)ch;
  #endif
#else
    extern char rt_hw_console_getchar(void);
    return rt_hw_console_getchar();
#endif
}

#if !defined(RT_USING_POSIX) && defined(RT_USING_DEVICE)
static rt_err_t finsh_rx_ind(rt_device_t dev, rt_size_t size)
{
    RT_ASSERT(shell != RT_NULL);

    /* release semaphore to let finsh thread rx data */
    rt_sem_release(&shell->rx_sem);

    return RT_EOK;
}

/**
 * @ingroup finsh
 *
 * This function sets the input device of finsh shell.
 *
 * @param device_name the name of new input device.
 */
void finsh_set_device(const char *device_name)
{
    rt_device_t dev = RT_NULL;
    rt_uint16_t flag = RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM;
    
    RT_ASSERT(shell != RT_NULL);
    dev = rt_device_find(device_name);
    if (dev == RT_NULL)
    {
        rt_kprintf("finsh: can not find device: %s\n", device_name);
        return;
    }

    /* check whether it's a same device */
    if (dev == shell->device) return;

    /**************************************************************************
     * FIXME maybe serial is open with poll mode. 2020.12.16 Bian
     */
    if (dev->flag & RT_DEVICE_FLAG_INT_RX)
        flag |= RT_DEVICE_FLAG_INT_RX;
        
    /* open this device and set the new device in finsh shell */
    if (rt_device_open(dev, flag) == RT_EOK)
    {
        if (shell->device != RT_NULL)
        {
            /* close old finsh device */
            rt_device_close(shell->device);
            rt_device_set_rx_indicate(shell->device, RT_NULL);
        }

        /* clear line buffer before switch to new device */
        memset(shell->line, 0, sizeof(shell->line));
        shell->line_curpos = shell->line_position = 0;

        shell->device = dev;
        rt_device_set_rx_indicate(dev, finsh_rx_ind);
    }
}

/**
 * @ingroup finsh
 *
 * This function returns current finsh shell input device.
 *
 * @return the finsh shell input device name is returned.
 */
const char *finsh_get_device()
{
    RT_ASSERT(shell != RT_NULL);
    return shell->device->parent.name;
}
#endif

/**
 * @ingroup finsh
 *
 * This function set the echo mode of finsh shell.
 *
 * FINSH_OPTION_ECHO=0x01 is echo mode, other values are none-echo mode.
 *
 * @param echo the echo mode
 */
void finsh_set_echo(rt_uint32_t echo)
{
    RT_ASSERT(shell != RT_NULL);
    shell->echo_mode = (rt_uint8_t)echo;
}

/**
 * @ingroup finsh
 *
 * This function gets the echo mode of finsh shell.
 *
 * @return the echo mode
 */
rt_uint32_t finsh_get_echo()
{
    RT_ASSERT(shell != RT_NULL);

    return shell->echo_mode;
}

#ifdef FINSH_USING_AUTH
/**
 * set a new password for finsh
 *
 * @param password new password
 *
 * @return result, RT_EOK on OK, -RT_ERROR on the new password length is less than
 *  FINSH_PASSWORD_MIN or greater than FINSH_PASSWORD_MAX
 */
rt_err_t finsh_set_password(const char *password) {
    rt_ubase_t level;
    rt_size_t pw_len = rt_strlen(password);

    if (pw_len < FINSH_PASSWORD_MIN || pw_len > FINSH_PASSWORD_MAX)
        return -RT_ERROR;

    level = rt_hw_interrupt_disable();
    rt_strncpy(shell->password, password, FINSH_PASSWORD_MAX);
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}

/**
 * get the finsh password
 *
 * @return password
 */
const char *finsh_get_password(void)
{
    return shell->password;
}

static void finsh_wait_auth(void)
{
    int ch;
    rt_bool_t input_finish = RT_FALSE;
    char password[FINSH_PASSWORD_MAX] = { 0 };
    rt_size_t cur_pos = 0;
    /* password not set */
    if (rt_strlen(finsh_get_password()) == 0) return;

    while (1)
    {
        rt_kprintf("Password for login: ");
        while (!input_finish)
        {
            while (1)
            {
                /* read one character from device */
                ch = finsh_getchar();
                if (ch < 0)
                {
                    continue;
                }

                if (ch >= ' ' && ch <= '~' && cur_pos < FINSH_PASSWORD_MAX)
                {
                    /* change the printable characters to '*' */
                    rt_kprintf("*");
                    password[cur_pos++] = ch;
                }
                else if (ch == '\b' && cur_pos > 0)
                {
                    /* backspace */
                    cur_pos--;
                    password[cur_pos] = '\0';
                    rt_kprintf("\b \b");
                }
                else if (ch == '\r' || ch == '\n')
                {
                    rt_kprintf("\n");
                    input_finish = RT_TRUE;
                    break;
                }
            }
        }
        if (!rt_strncmp(shell->password, password, FINSH_PASSWORD_MAX)) return;
        else
        {
            /* authentication failed, delay 2S for retry */
            rt_thread_delay(2 * RT_TICK_PER_SECOND);
            rt_kprintf("Sorry, try again.\n");
            cur_pos = 0;
            input_finish = RT_FALSE;
            rt_memset(password, '\0', FINSH_PASSWORD_MAX);
        }
    }
}
#endif /* FINSH_USING_AUTH */

static void shell_auto_complete(char *prefix)
{

    rt_kprintf("\n");
#ifdef FINSH_USING_MSH
    if (msh_is_used() == RT_TRUE)
    {
        msh_auto_complete(prefix);
    }
    else
#endif
    {
#ifndef FINSH_USING_MSH_ONLY
        extern void list_prefix(char * prefix);
        list_prefix(prefix);
#endif
    }

    rt_kprintf("%s%s", FINSH_PROMPT, prefix);
}

#ifndef FINSH_USING_MSH_ONLY
void finsh_run_line(struct finsh_parser *parser, const char *line)
{
    const char *err_str;

    if(shell->echo_mode)
        rt_kprintf("\n");
    finsh_parser_run(parser, (unsigned char *)line);

    /* compile node root */
    if (finsh_errno() == 0)
    {
        finsh_compiler_run(parser->root);
    }
    else
    {
        err_str = finsh_error_string(finsh_errno());
        rt_kprintf("%s\n", err_str);
    }

    /* run virtual machine */
    if (finsh_errno() == 0)
    {
        char ch;
        finsh_vm_run();

        ch = (unsigned char)finsh_stack_bottom();
        if (ch > 0x20 && ch < 0x7e)
        {
            rt_kprintf("\t'%c', %d, 0x%08x\n",
                       (unsigned char)finsh_stack_bottom(),
                       (unsigned int)finsh_stack_bottom(),
                       (unsigned int)finsh_stack_bottom());
        }
        else
        {
            rt_kprintf("\t%d, 0x%08x\n",
                       (unsigned int)finsh_stack_bottom(),
                       (unsigned int)finsh_stack_bottom());
        }
    }

    finsh_flush(parser);
}
#endif

#ifdef FINSH_USING_HISTORY
static rt_bool_t shell_handle_history(struct finsh_shell *shell)
{
#if defined(_WIN32)
    int i;
    rt_kprintf("\r");

    for (i = 0; i <= 60; i++)
        putchar(' ');
    rt_kprintf("\r");

#else
    rt_kprintf("\r");//rt_kprintf("\033[2K\r");
#endif
    rt_kprintf("%s%s", FINSH_PROMPT, shell->line);
    return RT_FALSE;
}

static void shell_push_history(struct finsh_shell *shell)
{
    if (shell->line_position != 0)
    {
        /* push history */
        if (shell->history_count >= FINSH_HISTORY_LINES)
        {
            /* if current cmd is same as last cmd, don't push */
            if (memcmp(&shell->cmd_history[FINSH_HISTORY_LINES - 1], shell->line, FINSH_CMD_SIZE))
            {
                /* move history */
                int index;
                for (index = 0; index < FINSH_HISTORY_LINES - 1; index ++)
                {
                    memcpy(&shell->cmd_history[index][0],
                           &shell->cmd_history[index + 1][0], FINSH_CMD_SIZE);
                }
                memset(&shell->cmd_history[index][0], 0, FINSH_CMD_SIZE);
                memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

                /* it's the maximum history */
                shell->history_count = FINSH_HISTORY_LINES;
            }
        }
        else
        {
            /* if current cmd is same as last cmd, don't push */
            if (shell->history_count == 0 || memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, FINSH_CMD_SIZE))
            {
                shell->current_history = shell->history_count;
                memset(&shell->cmd_history[shell->history_count][0], 0, FINSH_CMD_SIZE);
                memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

                /* increase count and set current history position */
                shell->history_count ++;
            }
        }
    }
    shell->current_history = shell->history_count;
}
#endif

#if (USE_PICTURE_SOC == 1)
extern void fb_drawrect_1(int x1, int y1, int x2, int y2);
extern void fb_put_string_center_1(int x, int y, char *str);
void show_back_ground(void)
{
    int i,j,x1,x2,y1,y2;
    fb_cons_clear();                        // 清楚画布
    for(i=0;i<devide_lie;i++)
    {
        for(j=0;j<devide_hang;j++)
        {
            x1 = (x_max/devide_lie)*i;
            x2 = (x_max/devide_lie)*(i+1) - 1;
            y1 = (y_max/devide_hang)*j;
            y2 = (y_max/devide_hang)*(j+1) - 1;
            fb_drawrect_1( x1,  y1,  x2,  y2);
        }
    }
    fb_put_string_center_1(100, 48, "set_color");
    fb_put_string_center_1(300, 48, "read_disk");
    fb_put_string_center_1(500, 48, "push_box");
    fb_put_string_center_1(700, 48, "mouse_draw");
    
    fb_put_string_center_1(100,144, "SG180_");
    fb_put_string_center_1(300,144, "air_quality");
    fb_put_string_center_1(500,144, "human_sensor");
    fb_put_string_center_1(700,144, "Temperature");
    
    fb_put_string_center_1(100,240, "L610_Check");
    fb_put_string_center_1(300,240, "L610_Ctrl");
    fb_put_string_center_1(500,240, "L610_ShortMessage");
    fb_put_string_center_1(700,240, "L610_TXcloud_safety");
    
    fb_put_string_center_1(100,336, "L610_Telephone");
    fb_put_string_center_1(300,336, "L610_Position");
}
#endif

void finsh_thread_entry(void *parameter)
{
    int ch,gui_is_begin = 1;
    extern int get_scheduler_lock();
    if(get_scheduler_lock())
    {
        rt_thread_sleep(1000);
    }

    /* normal is echo mode */
#ifndef FINSH_ECHO_DISABLE_DEFAULT
    shell->echo_mode = 1;
#else
    shell->echo_mode = 0;
#endif

#ifndef FINSH_USING_MSH_ONLY
    finsh_init(&shell->parser);
#endif

#if !defined(RT_USING_POSIX) && defined(RT_USING_DEVICE)
    /* 将控制台设备设置为外壳设备 */
    if (shell->device == RT_NULL)
    {
        rt_device_t console = rt_console_get_device();
        if (console)
        {
            finsh_set_device(console->parent.name);
        }
    }
#endif

#ifdef FINSH_USING_AUTH
    /* set the default password when the password isn't setting */
    if (rt_strlen(finsh_get_password()) == 0)
    {
        if (finsh_set_password(FINSH_DEFAULT_PASSWORD) != RT_EOK)
        {
            rt_kprintf("Finsh password set failed.\n");
        }
    }
    /* waiting authenticate success */
    finsh_wait_auth();
#endif

    rt_kprintf(FINSH_PROMPT);           // 为了main函数中更长的初始化操作

    while (1)
    {
#if (USE_PICTURE_SOC == 1)
        rt_thread_sleep(5);
#else
        rt_thread_sleep(100);           /* 让出 CPU 时间 FIXME 2021.4.15 Bian */
#endif
        while(get_scheduler_lock())
        {
            rt_thread_sleep(1000);
        }


#if (USE_PICTURE_SOC)

        extern int mouse_x,mouse_y,mouse_color;
        if(show_background == 1)
        {
#if (GUI_SOC)
            if(gui_is_begin == 1)
            {
                show_desktop_gui();
                gui_is_begin = 0;
            }
            else
            {
                desktop_gui_show();
            }
#else
            show_back_ground();
#endif
            save_old_mouse_graph(mouse_x,mouse_y);
            show_mouse_graph(mouse_x,mouse_y,mouse_color);
            show_background++;
        }
        
        int ch = 0;
        if (usb_mouse_testc())              // 检测是否有输入
        {
            ch = usb_mouse_getc();          // 读取鼠标输入，返回输入字符
            //printf(" %d \n",ch);          // ch=1鼠标左键，ch=2鼠标右键，ch=4鼠标滚轮
            if(ch == 1)
            {
#if (GUI_SOC)
/*
                if(click_in_button(1))
                    msh_exec("set_color",9);
                else if(click_in_button(2))
                    msh_exec("mouse_draw",10);
                else if(click_in_button(3))
                    msh_exec("read_disk",9);
                else if(click_in_button(4))
                    msh_exec("push_box",8);
                else if(click_in_button(5))
                    msh_exec("SG180_",6);
                else if(click_in_button(6))
                    msh_exec("human_sensor",12);
                else if(click_in_button(7))
                    msh_exec("air_quality",11);
                else if(click_in_button(8))
                    msh_exec("Temperature",11);
                else if(click_in_button(9))
                    msh_exec("L610_Ctrl",9);
                else if(click_in_button(10))
                    msh_exec("L610_Check",10);
                else if(click_in_button(11))
                    msh_exec("L610_Telephone",14);
                else if(click_in_button(12))
                    msh_exec("L610_ShortMessage",17);
                else if(click_in_button(13))
                    msh_exec("L610_Position",13);
                else if(click_in_button(14))
                    msh_exec("L610_TXcloud_safety",19);
                else ;
*/
                int button_x = (int)((mouse_x-540)/110 +1);
                int button_y = (int)((mouse_y-30)/60);
                if(((button_x == 1)||(button_x == 2)) && ((button_y>=0)&&(button_y<=6)))
                {
                    int select_app_num = button_y*2 + button_x;
                    switch(select_app_num)
                    {
                        case 1: msh_exec("set_color",9);break;
                        case 2: msh_exec("mouse_draw",10);break;
                        case 3: msh_exec("read_disk",9);break;
                        case 4: msh_exec("push_box",8);break;
                        case 5: msh_exec("SG180_",6);break;
                        case 6: msh_exec("human_sensor",12);break;
                        case 7: msh_exec("air_quality",11);break;
                        case 8: msh_exec("Temperature",11);break;
                        case 9: msh_exec("L610_Ctrl",9);break;
                        case 10: msh_exec("L610_Check",10);break;
                        case 11: msh_exec("L610_Telephone",14);break;
                        case 12: msh_exec("L610_ShortMessage",17);break;
                        case 13: msh_exec("L610_Position",13);break;
                        case 14: msh_exec("L610_TXcloud_safety",19);break;
                        default:    break;
                    }
                }
#else
                if(mouse_y<96)
                {
                    if(mouse_x<200)
                        msh_exec("set_color",9);
                    else if(mouse_x<400)
                        msh_exec("read_disk",9);
                    else if(mouse_x<600)
                        msh_exec("push_box",8);
                    else
                        msh_exec("mouse_draw",10);
                }
                else if(mouse_y<192)
                {
                    if(mouse_x<200)
                        msh_exec("SG180_",6);
                    else if(mouse_x<400)
                        msh_exec("air_quality",11);
                    else if(mouse_x<600)
                        msh_exec("human_sensor",12);
                    else
                        msh_exec("Temperature",11);
                }
                else if(mouse_y<288)
                {
                    if(mouse_x<200)
                        msh_exec("L610_Check",10);
                    else if(mouse_x<400)
                        msh_exec("L610_Ctrl",9);
                    else if(mouse_x<600)
                        msh_exec("L610_ShortMessage",17);
                    else
                        msh_exec("L610_TXcloud_safety",19);
                }
                else if(mouse_y<384)
                {
                    if(mouse_x<200)
                        msh_exec("L610_Telephone",14);
                    else if(mouse_x<400)
                        msh_exec("L610_Position",13);
                    else ;
                }
                else;
#endif
            }
        }

#else
        ch = finsh_getchar();
        if (ch < 0)
        {
            continue;
        }

        /*
         * handle control key
         * up key  : 0x1b 0x5b 0x41
         * down key: 0x1b 0x5b 0x42
         * right key:0x1b 0x5b 0x43
         * left key: 0x1b 0x5b 0x44
         */
        if (ch == 0x1b)
        {
            shell->stat = WAIT_SPEC_KEY;
            continue;
        }
        else if (shell->stat == WAIT_SPEC_KEY)
        {
            if (ch == 0x5b)
            {
                shell->stat = WAIT_FUNC_KEY;
                continue;
            }

            shell->stat = WAIT_NORMAL;
        }
        else if (shell->stat == WAIT_FUNC_KEY)
        {
            shell->stat = WAIT_NORMAL;

            if (ch == 0x41) /* up key */
            {
#ifdef FINSH_USING_HISTORY
                /* prev history */
                if (shell->current_history > 0)
                    shell->current_history --;
                else
                {
                    shell->current_history = 0;
                    continue;
                }

                /* copy the history command */
                memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                       FINSH_CMD_SIZE);
                shell->line_curpos = shell->line_position = strlen(shell->line);
                shell_handle_history(shell);
#endif
                continue;
            }
            else if (ch == 0x42) /* down key */
            {
#ifdef FINSH_USING_HISTORY
                /* next history */
                if (shell->current_history < shell->history_count - 1)
                    shell->current_history ++;
                else
                {
                    /* set to the end of history */
                    if (shell->history_count != 0)
                        shell->current_history = shell->history_count - 1;
                    else
                        continue;
                }

                memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                       FINSH_CMD_SIZE);
                shell->line_curpos = shell->line_position = strlen(shell->line);
                shell_handle_history(shell);
#endif
                continue;
            }
            else if (ch == 0x44) /* left key */
            {
                if (shell->line_curpos)
                {
                    rt_kprintf("\b");
                    shell->line_curpos --;
                }

                continue;
            }
            else if (ch == 0x43) /* right key */
            {
                if (shell->line_curpos < shell->line_position)
                {
                    rt_kprintf("%c", shell->line[shell->line_curpos]);
                    shell->line_curpos ++;
                }

                continue;
            }
        }

        /* received null or error */
        if (ch == '\0' || ch == 0xFF) continue;
        /* handle tab key */
        else if (ch == '\t')
        {
            int i;
            /* move the cursor to the beginning of line */
            for (i = 0; i < shell->line_curpos; i++)
                rt_kprintf("\b");

            /* auto complete */
            shell_auto_complete(&shell->line[0]);
            /* re-calculate position */
            shell->line_curpos = shell->line_position = strlen(shell->line);

            continue;
        }
        /* handle backspace key */
        else if (ch == 0x7f || ch == 0x08)
        {
            /* note that shell->line_curpos >= 0 */
            if (shell->line_curpos == 0)
                continue;

            shell->line_position--;
            shell->line_curpos--;

            if (shell->line_position > shell->line_curpos)
            {
                int i;

                rt_memmove(&shell->line[shell->line_curpos],
                           &shell->line[shell->line_curpos + 1],
                           shell->line_position - shell->line_curpos);
                shell->line[shell->line_position] = 0;

                rt_kprintf("\b%s  \b", &shell->line[shell->line_curpos]);

                /* move the cursor to the origin position */
                for (i = shell->line_curpos; i <= shell->line_position; i++)
                    rt_kprintf("\b");
            }
            else
            {
                rt_kprintf("\b \b");
                shell->line[shell->line_position] = 0;
            }

            continue;
        }

        /* handle end of line, break */
        if (ch == '\r' || ch == '\n')                       // 按下回车命令行的响应
        {
#ifdef FINSH_USING_HISTORY
            shell_push_history(shell);                      // 更新历史命令
#endif

#ifdef FINSH_USING_MSH
            if (msh_is_used() == RT_TRUE)
            {
                if (shell->echo_mode)
                    rt_kprintf("\n");
                msh_exec(shell->line, shell->line_position);    // 启动相应线程
            }
            else
#endif
            {
#ifndef FINSH_USING_MSH_ONLY
                /* add ';' and run the command line */
                shell->line[shell->line_position] = ';';

                if (shell->line_position != 0) finsh_run_line(&shell->parser, shell->line);
                else
                    if (shell->echo_mode) rt_kprintf("\n");
#endif
            }

            rt_kprintf(FINSH_PROMPT);
            memset(shell->line, 0, sizeof(shell->line));
            shell->line_curpos = shell->line_position = 0;
            continue;
        }

        /* it's a large line, discard it */
        if (shell->line_position >= FINSH_CMD_SIZE)
            shell->line_position = 0;

        /* normal character */
        if (shell->line_curpos < shell->line_position)
        {
            int i;

            rt_memmove(&shell->line[shell->line_curpos + 1],
                       &shell->line[shell->line_curpos],
                       shell->line_position - shell->line_curpos);
            shell->line[shell->line_curpos] = ch;
            if (shell->echo_mode)
                rt_kprintf("%s", &shell->line[shell->line_curpos]);

            /* move the cursor to new position */
            for (i = shell->line_curpos; i < shell->line_position; i++)
                rt_kprintf("\b");
        }
        else
        {
            shell->line[shell->line_position] = ch;             // 输入指令的响应
            if (shell->echo_mode)
                rt_kprintf("%c", ch);
        }

        ch = 0;
        shell->line_position ++;
        shell->line_curpos++;
        if (shell->line_position >= FINSH_CMD_SIZE)
        {
            /* clear command line */
            shell->line_position = 0;
            shell->line_curpos = 0;
        }
#endif
    } /* end of device read */
}

void finsh_system_function_init(const void *begin, const void *end)
{
    _syscall_table_begin = (struct finsh_syscall *) begin;
    _syscall_table_end = (struct finsh_syscall *) end;
}

void finsh_system_var_init(const void *begin, const void *end)
{
    _sysvar_table_begin = (struct finsh_sysvar *) begin;
    _sysvar_table_end = (struct finsh_sysvar *) end;
}

#if defined(__ICCARM__) || defined(__ICCRX__)               /* for IAR compiler */
#ifdef FINSH_USING_SYMTAB
#pragma section="FSymTab"
#pragma section="VSymTab"
#endif
#elif defined(__ADSPBLACKFIN__) /* for VisaulDSP++ Compiler*/
#ifdef FINSH_USING_SYMTAB
extern "asm" int __fsymtab_start;
extern "asm" int __fsymtab_end;
extern "asm" int __vsymtab_start;
extern "asm" int __vsymtab_end;
#endif
#elif defined(_MSC_VER)
#pragma section("FSymTab$a", read)
const char __fsym_begin_name[] = "__start";
const char __fsym_begin_desc[] = "begin of finsh";
__declspec(allocate("FSymTab$a")) const struct finsh_syscall __fsym_begin =
{
    __fsym_begin_name,
    __fsym_begin_desc,
    NULL
};

#pragma section("FSymTab$z", read)
const char __fsym_end_name[] = "__end";
const char __fsym_end_desc[] = "end of finsh";
__declspec(allocate("FSymTab$z")) const struct finsh_syscall __fsym_end =
{
    __fsym_end_name,
    __fsym_end_desc,
    NULL
};
#endif

/*
 * @ingroup finsh
 *
 * This function will initialize finsh shell
 */
int finsh_system_init(void)
{
    rt_err_t result = RT_EOK;
    rt_thread_t tid;

#ifdef FINSH_USING_SYMTAB
#if defined(__CC_ARM) || defined(__CLANG_ARM)          /* ARM C Compiler */
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    extern const int VSymTab$$Base;
    extern const int VSymTab$$Limit;
    finsh_system_function_init(&FSymTab$$Base, &FSymTab$$Limit);
#ifndef FINSH_USING_MSH_ONLY
    finsh_system_var_init(&VSymTab$$Base, &VSymTab$$Limit);
#endif
#elif defined (__ICCARM__) || defined(__ICCRX__)      /* for IAR Compiler */
    finsh_system_function_init(__section_begin("FSymTab"),
                               __section_end("FSymTab"));
    finsh_system_var_init(__section_begin("VSymTab"),
                          __section_end("VSymTab"));
#elif defined (__GNUC__) || defined(__TI_COMPILER_VERSION__)
    /* GNU GCC Compiler and TI CCS */
    extern const int __fsymtab_start;
    extern const int __fsymtab_end;
    extern const int __vsymtab_start;
    extern const int __vsymtab_end;
    finsh_system_function_init(&__fsymtab_start, &__fsymtab_end);
    finsh_system_var_init(&__vsymtab_start, &__vsymtab_end);
#elif defined(__ADSPBLACKFIN__) /* for VisualDSP++ Compiler */
    finsh_system_function_init(&__fsymtab_start, &__fsymtab_end);
    finsh_system_var_init(&__vsymtab_start, &__vsymtab_end);
#elif defined(_MSC_VER)
    unsigned int *ptr_begin, *ptr_end;
		
    if(shell)
    {
        rt_kprintf("finsh shell already init.\n");
        return RT_EOK;
    }

    ptr_begin = (unsigned int *)&__fsym_begin;
    ptr_begin += (sizeof(struct finsh_syscall) / sizeof(unsigned int));
    while (*ptr_begin == 0) ptr_begin ++;

    ptr_end = (unsigned int *) &__fsym_end;
    ptr_end --;
    while (*ptr_end == 0) ptr_end --;

    finsh_system_function_init(ptr_begin, ptr_end);
#endif
#endif

#ifdef RT_USING_HEAP
    /* 创建或设置外壳结构 */
    shell = (struct finsh_shell *)rt_calloc(1, sizeof(struct finsh_shell));
    if (shell == RT_NULL)
    {
        rt_kprintf("no memory for shell\n");
        return -1;
    }
#if (USE_PICTURE_SOC == 1)
    tid = rt_thread_create(FINSH_THREAD_NAME,
                           finsh_thread_entry, RT_NULL,
                           FINSH_THREAD_STACK_SIZE, FINSH_THREAD_PRIORITY, 50);
#else
    tid = rt_thread_create(FINSH_THREAD_NAME,
                           finsh_thread_entry, RT_NULL,
                           FINSH_THREAD_STACK_SIZE, FINSH_THREAD_PRIORITY, 10);
#endif
#else
    shell = &_shell;
    tid = &finsh_thread;
    result = rt_thread_init(&finsh_thread,
                            FINSH_THREAD_NAME,
                            finsh_thread_entry, RT_NULL,
                            &finsh_thread_stack[0], sizeof(finsh_thread_stack),
                            FINSH_THREAD_PRIORITY, 10);
#endif /* RT_USING_HEAP */

    rt_sem_init(&(shell->rx_sem), "shrx", 0, 0);
    finsh_set_prompt_mode(1);

    if (tid != NULL && result == RT_EOK)
        rt_thread_startup(tid);
    return 0;
}
INIT_APP_EXPORT(finsh_system_init);

#endif /* RT_USING_FINSH */

