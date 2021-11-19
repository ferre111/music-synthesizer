/*
 * menu.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */
#include "menu.h"
#include "envelope_generator_page.h"

//----------------------------------------------------------------------

/* Array of structure with pointer to functions associate with appropriate page*/
static page page_tab[Menu_pages_end] =
{
    [ENVELOPE_GENERATOR_PAGE]          = {.init_fun = envelope_generator_page_init,           .draw_fun = envelope_generator_page_draw,          .exit_fun = envelope_generator_page_exit},
};

static menu_ctx ctx = {.page = Menu_pages_start + 1, .page_tab = page_tab};

//----------------------------------------------------------------------

static void draw_current_page(void);
static void check_page_flag(void);

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
}

//----------------------------------------------------------------------

void Menu_process(void)
{
    draw_current_page();    //draw actual set page
    check_page_flag();
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

