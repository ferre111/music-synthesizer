/*
 * menu.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include <stdbool.h>
#include "main.h"

#include "envelope_generator_page.h"

//----------------------------------------------------------------------

#define TIME_BETWEEN_PAGE_CHANGE    500 //in ms
#define GYRO_REG_VAL_TO_CHANGE_PAGE 15360

//----------------------------------------------------------------------

enum menu_axes
{
    x_axis,
    y_axis,
    z_axis
};

//----------------------------------------------------------------------

typedef enum menu_pages_T
{
    Menu_pages_start,

    ENVELOPE_GENERATOR_PAGE,

    Menu_pages_end
} menu_pages;

//----------------------------------------------------------------------

typedef struct page_T
{
    void (*init_fun)(void);
    void (*draw_fun)(void);
    void (*encoder_button_enter_press)(void);
    void (*exit_fun)(void);
} page;

typedef struct menu_ctx_T
{
    menu_pages page;
    bool next_page_flag;
    bool prev_page_flag;

    page *page_tab;
} menu_ctx;

//----------------------------------------------------------------------

void Menu_init(void);

//----------------------------------------------------------------------

/*
 * @brief   This function should be insert in main loop.
 */
void Menu_process(void);

//----------------------------------------------------------------------

void Menu_set_next_page_flag(void);

//----------------------------------------------------------------------

void Menu_set_prev_page_flag(void);

//----------------------------------------------------------------------

void Menu_set_main_encoder_callbacks(void);
