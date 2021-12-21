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

#include "select_mode_page.h"
#include "envelope_generator_page.h"
#include "osc1_page.h"
#include "osc2_page.h"
#include "fm_synth_page.h"

//----------------------------------------------------------------------

/* Array of structure with pointer to functions associate with appropriate page*/
static page page_tab[Menu_pages_end] =
{
    [SELECT_MODE_PAGE]                 = {.init_fun = Select_mode_page_init, .draw_fun = Select_mode_page_draw, .encoder_button_enter_press = Select_mode_encoder_enter_press_fun, .exit_fun = Select_mode_page_exit},
    [OSC1_PAGE]                        = {.init_fun = Osc1_page_init, .draw_fun = Osc1_page_draw, .encoder_button_enter_press = Osc1_encoder_enter_press_fun, .exit_fun = Osc1_page_exit},
    [OSC2_PAGE]                        = {.init_fun = Osc2_page_init, .draw_fun = Osc2_page_draw, .encoder_button_enter_press = Osc2_encoder_enter_press_fun, .exit_fun = Osc2_page_exit},
    [FM_PAGE]                          = {.init_fun = FMSynth_page_init, .draw_fun = FMSynth_page_draw, .encoder_button_enter_press = FMSynth_encoder_enter_press_fun, .exit_fun = FMSynth_page_exit},
    [ENVELOPE_GENERATOR_PAGE]          = {.init_fun = EnvelopeGenerator_page_init, .draw_fun = EnvelopeGenerator_page_draw, .encoder_button_enter_press = EnvelopeGenerator_encoder_enter_press_fun, .exit_fun = EnvelopeGenerator_page_exit},
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
