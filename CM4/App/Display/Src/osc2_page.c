/*
 * osc2_page.c
 *
 *  Created on: Nov 30, 2021
 *      Author: karol
 */

#include <stdio.h>
#include "osc2_page.h"
#include "menu.h"
#include "buttons.h"
#include "encoder.h"
#include "synthcom.h"
#include "OLED.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

#define MAX_ACTIVATED       1U
#define MAX_SHAPE           2U
#define MAX_OCTAVE_OFFSET   4U

#define DEF_ACTIVATED       0U
#define DEF_SHAPE           0U
#define DEF_OCTAVE_OFFSET   2U

/*------------------------------------------------------------------------------------------------------------------------------*/

static char activated_txt[2][NUMBER_OF_LETTERS_IN_LINE] =
{
        "no",
        "yes",
};


static char shape_txt[3][NUMBER_OF_LETTERS_IN_LINE] =
{
        "sine",
        "rectangle",
        "saw",
};

static char octave_offset_txt[5][NUMBER_OF_LETTERS_IN_LINE] =
{
        "-2",
        "-1",
        "0",
        "+1",
        "+2",
};

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef enum current_setting_T
{
    CURRENT_SETTING_ACTIVATED,
    CURRENT_SETTING_SHAPE,
    CURRENT_SETTING_OCTAVE_OFFSET,

    Current_setting_end
} current_setting;

typedef struct setting_data_T
{
    uint16_t data;
    uint16_t max_value;
    uint8_t setting_id;
    char setting_txt[NUMBER_OF_LETTERS_IN_LINE];
} setting_data;

typedef struct osc2_page_T
{
    uint8_t heading_id;
    char heading_txt[NUMBER_OF_LETTERS_IN_LINE];
    current_setting current_setting;
    setting_data    settings_data[Current_setting_end];
} osc2_page;

/*------------------------------------------------------------------------------------------------------------------------------*/

static osc2_page ctx = {.current_setting = CURRENT_SETTING_ACTIVATED,
                                     .settings_data =
                                       {
                                           [CURRENT_SETTING_ACTIVATED] = {.max_value = MAX_ACTIVATED, .data = DEF_ACTIVATED},
                                           [CURRENT_SETTING_SHAPE] = {.max_value = MAX_SHAPE, .data = DEF_SHAPE},
                                           [CURRENT_SETTING_OCTAVE_OFFSET] = {.max_value = MAX_OCTAVE_OFFSET, .data = DEF_OCTAVE_OFFSET},
                                       }
};

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_press_fun(void);
static void encoder_hold_fun(void);
static void encoder_inc_browsing_fun(void);
static void encoder_dec_browsing_fun(void);
static void encoder_inc_data_fun(void);
static void encoder_dec_data_fun(void);

/*------------------------------------------------------------------------------------------------------------------------------*/

void Osc2_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&ctx.heading_id, 34U, 0U, ctx.heading_txt, 2U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_ACTIVATED].setting_id, 0U, 16U, ctx.settings_data[CURRENT_SETTING_ACTIVATED].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_SHAPE].setting_id, 0U, 24U, ctx.settings_data[CURRENT_SETTING_SHAPE].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_OCTAVE_OFFSET].setting_id, 0U, 32U, ctx.settings_data[CURRENT_SETTING_OCTAVE_OFFSET].setting_txt, 1U, false);
    snprintf(ctx.heading_txt, NUMBER_OF_LETTERS_IN_LINE, "OSC 2");
}

void Osc2_page_draw(void)
{
    /* update text strings printed on display with actual envelope generator data */
    snprintf(ctx.settings_data[CURRENT_SETTING_ACTIVATED].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Activated: %*s", 10U, activated_txt[ctx.settings_data[CURRENT_SETTING_ACTIVATED].data]);
    snprintf(ctx.settings_data[CURRENT_SETTING_SHAPE].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Shape: %*s", 14U, shape_txt[ctx.settings_data[CURRENT_SETTING_SHAPE].data]);
    snprintf(ctx.settings_data[CURRENT_SETTING_OCTAVE_OFFSET].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Octave offset: %*s", 6U, octave_offset_txt[ctx.settings_data[CURRENT_SETTING_OCTAVE_OFFSET].data]);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Osc2_encoder_enter_press_fun(void)
{
    Encoder_set_callback_increment_fun(encoder_inc_browsing_fun);
    Encoder_set_callback_decrement_fun(encoder_dec_browsing_fun);
    Buttons_set_callback_press_function(BUTTON_TYPES_ENCODER, encoder_press_fun);
    Buttons_set_callback_hold_function(BUTTON_TYPES_ENCODER, encoder_hold_fun);
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, true);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Osc2_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(ctx.heading_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_ACTIVATED].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_SHAPE].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_OCTAVE_OFFSET].setting_id);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_press_fun(void)
{
    static bool data_edited_flag;

    data_edited_flag = !data_edited_flag;

    if (data_edited_flag)
    {
        /* start editing data */
        Encoder_set_callback_increment_fun(encoder_inc_data_fun);
        Encoder_set_callback_decrement_fun(encoder_dec_data_fun);
    }
    else
    {
        /* end editing data */
        Encoder_set_callback_increment_fun(encoder_inc_browsing_fun);
        Encoder_set_callback_decrement_fun(encoder_dec_browsing_fun);
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/


static void encoder_inc_browsing_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, false);

    if (CURRENT_SETTING_OCTAVE_OFFSET == ctx.current_setting)
    {
        ctx.current_setting = CURRENT_SETTING_ACTIVATED;
    }
    else
    {
        ctx.current_setting++;
    }

    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, true);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_dec_browsing_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, false);

    if (CURRENT_SETTING_ACTIVATED == ctx.current_setting)
    {
        ctx.current_setting = CURRENT_SETTING_OCTAVE_OFFSET;
    }
    else
    {
        ctx.current_setting--;
    }

    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, true);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_inc_data_fun(void)
{
    uint16_t data_tmp[Current_setting_end];

    if (ctx.settings_data[ctx.current_setting].max_value == ctx.settings_data[ctx.current_setting].data)
    {
        ctx.settings_data[ctx.current_setting].data = 0U;
    }
    else
    {
        ctx.settings_data[ctx.current_setting].data++;
    }

    for (uint8_t i = 0U; i < Current_setting_end; i++)
    {
        data_tmp[i] = ctx.settings_data[i].data;
    }
    //todo synthcom
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_dec_data_fun(void)
{
    uint16_t data_tmp[Current_setting_end];

    if (0U == ctx.settings_data[ctx.current_setting].data)
    {
        ctx.settings_data[ctx.current_setting].data = ctx.settings_data[ctx.current_setting].max_value;
    }
    else
    {
        ctx.settings_data[ctx.current_setting].data--;
    }

    for (uint8_t i = 0U; i < Current_setting_end; i++)
    {
        data_tmp[i] = ctx.settings_data[i].data;
    }
    //todo
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_hold_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, false);
    Menu_set_main_encoder_callbacks();
}
