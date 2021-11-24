/*
 * menu.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */
#include "menu.h"
#include "encoder.h"
#include "buttons.h"
#include "utility.h"
#include "envelope_generator_page.h"

//----------------------------------------------------------------------

/* Array of structure with pointer to functions associate with appropriate page*/
static page page_tab[Menu_pages_end] =
{
    [ENVELOPE_GENERATOR_PAGE]          = {.init_fun = envelope_generator_page_init, .draw_fun = envelope_generator_page_draw, .encoder_button_enter_press = envelope_generator_encoder_enter_press_fun, .exit_fun = envelope_generator_page_exit},
};

static menu_ctx ctx = {.page = Menu_pages_start + 1, .page_tab = page_tab};

//----------------------------------------------------------------------

static void draw_current_page(void);
static void check_page_flag(void);
static void menu_encoder_button_press(void);

//----------------------------------------------------------------------

void Menu_set_next_page_flag(void)
{
    ctx.next_page_flag = true;
}

//----------------------------------------------------------------------

void Menu_set_prev_page_flag(void)
{
    ctx.prev_page_flag = true;
}

//----------------------------------------------------------------------

void Menu_init(void)
{
    ctx.page_tab[ctx.page].init_fun();  //init first page
    Encoder_set_callback_increment_fun(Menu_set_next_page_flag);
    Encoder_set_callback_decrement_fun(Menu_set_prev_page_flag);
    Buttons_set_callback_press_function(BUTTON_TYPES_ENCODER, menu_encoder_button_press);
}

//----------------------------------------------------------------------

void Menu_process(void)
{
    draw_current_page();    //draw actual set page
    check_page_flag();
}

//----------------------------------------------------------------------

void Menu_set_main_encoder_callbacks(void)
{
    Encoder_set_callback_increment_fun(Menu_set_next_page_flag);
    Encoder_set_callback_decrement_fun(Menu_set_prev_page_flag);
    Buttons_set_callback_press_function(BUTTON_TYPES_ENCODER, menu_encoder_button_press);
    Buttons_set_callback_hold_function(BUTTON_TYPES_ENCODER, utility_BlankFun);
}

//----------------------------------------------------------------------

static void draw_current_page(void)
{
    ctx.page_tab[ctx.page].draw_fun();
}

//----------------------------------------------------------------------

static void check_page_flag(void)
{
    if(ctx.next_page_flag)
    {
        ctx.page_tab[ctx.page].exit_fun();                                  //execute function to deinit page

        if(++ctx.page == Menu_pages_end) ctx.page = Menu_pages_start + 1U;   //increment page

        ctx.page_tab[ctx.page].init_fun();                                  //execute function to init new page

        ctx.next_page_flag = false;
    }

    if(ctx.prev_page_flag)
    {
        ctx.page_tab[ctx.page].exit_fun();                                  //execute function to deinit page

        if(--ctx.page == Menu_pages_start) ctx.page = Menu_pages_end - 1U;   //increment page

        ctx.page_tab[ctx.page].init_fun();                                  //execute function to init new page

        ctx.prev_page_flag = false;
    }
}

//----------------------------------------------------------------------

static void menu_encoder_button_press(void)
{
    ctx.page_tab[ctx.page].encoder_button_enter_press();
}