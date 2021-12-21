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

#define NUMBER_OF_LETTERS_IN_LINE 22U

//----------------------------------------------------------------------

typedef enum menu_pages_T
{
    Menu_pages_start,

    SELECT_MODE_PAGE,
    OSC1_PAGE,
    OSC2_PAGE,
    FM_PAGE,
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
