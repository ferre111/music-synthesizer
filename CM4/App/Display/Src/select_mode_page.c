/*
 * select_mode_page.c
 *
 *  Created on: Dec 21, 2021
 *      Author: User
 */

#include <stdio.h>
#include "select_mode_page.h"
#include "menu.h"
#include "buttons.h"
#include "encoder.h"
#include "synthcom.h"
#include "OLED.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

#define MAX_SELECT_MODE     2U

#define DEF_SELECT_MODE       0U

/*------------------------------------------------------------------------------------------------------------------------------*/

static char select_mode_txt[3][NUMBER_OF_LETTERS_IN_LINE] =
{
        "None",
        "Wavetable",
        "FM"
};

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef enum current_setting_T
{
    CURRENT_SETTING_MODE,

    Current_setting_end
} current_setting;

typedef struct setting_data_T
{
    uint8_t data;
    uint8_t max_value;
    uint8_t setting_id;
    char setting_txt[NUMBER_OF_LETTERS_IN_LINE];
} setting_data;

typedef struct select_mode_page_T
{
    uint8_t heading_id;
    uint8_t mode_image_id;
    char heading_txt[NUMBER_OF_LETTERS_IN_LINE];
    current_setting current_setting;
    setting_data    settings_data[Current_setting_end];
} select_mode_page;

/*------------------------------------------------------------------------------------------------------------------------------*/

static const unsigned char mode_wavetable [] = {
    0x48, 0x18,
    0xFF, 0x01, 0x01, 0x01, 0x01, 0x81, 0xC1, 0xE1, 0x71, 0xF9, 0xF9, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x61, 0x71, 0x19, 0x19, 0x19, 0x19,
    0xF1, 0xE1, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x80,
    0x80, 0xFF, 0xFF, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xF0, 0xB8, 0x9C, 0x8E, 0x87, 0x83, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x0F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0F
};

/*------------------------------------------------------------------------------------------------------------------------------*/

static const unsigned char mode_fm [] = {
    0x48, 0x18,
    0xFF, 0x01, 0x01, 0x01, 0x01, 0x81, 0xC1, 0xE1, 0x71, 0xF9, 0xF9, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0xFF, 0x00, 0x80, 0xC0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x61, 0x71, 0x19, 0x19, 0x19, 0x19,
    0xF1, 0xE1, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x80,
    0x80, 0xFF, 0xFF, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x0F, 0x1F, 0x3F, 0x7F,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xF0, 0xB8, 0x9C, 0x8E, 0x87, 0x83, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x0F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0F
};

/*------------------------------------------------------------------------------------------------------------------------------*/

static select_mode_page ctx = {.current_setting = CURRENT_SETTING_MODE, .mode_image_id = 0x55,
                                     .settings_data =
                                       {
                                           [CURRENT_SETTING_MODE] = {.max_value = MAX_SELECT_MODE, .data = DEF_SELECT_MODE},
                                       }
};

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_hold_fun(void);
static void encoder_inc_data_fun(void);
static void encoder_dec_data_fun(void);
static void set_image(void);

/*------------------------------------------------------------------------------------------------------------------------------*/

void Select_mode_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&ctx.heading_id, 40U, 0U, ctx.heading_txt, 2U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_MODE].setting_id, 0U, 16U, ctx.settings_data[CURRENT_SETTING_MODE].setting_txt, 1U, false);
    snprintf(ctx.heading_txt, NUMBER_OF_LETTERS_IN_LINE, "MODE");
//    OLED_createImage(&ctx.mode_image_id, 28U, 14U, mode_none);
    set_image();
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Select_mode_page_draw(void)
{
    /* update text strings printed on display with actual envelope generator data */
    snprintf(ctx.settings_data[CURRENT_SETTING_MODE].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Mode: %*s", 15U, select_mode_txt[ctx.settings_data[CURRENT_SETTING_MODE].data]);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Select_mode_encoder_enter_press_fun(void)
{
    Encoder_set_callback_increment_fun(encoder_inc_data_fun);
    Encoder_set_callback_decrement_fun(encoder_dec_data_fun);
//    Buttons_set_callback_press_function(BUTTON_TYPES_ENCODER, encoder_press_fun);
    Buttons_set_callback_hold_function(BUTTON_TYPES_ENCODER, encoder_hold_fun);
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, true);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Select_mode_page_exit(void)
{
    OLED_deleteObject(ctx.heading_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_MODE].setting_id);
    if (0U != ctx.settings_data[CURRENT_SETTING_MODE].data)
    {
        OLED_deleteObject(ctx.mode_image_id);
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

//static void encoder_press_fun(void)
//{
//    /* start editing data */
//    Encoder_set_callback_increment_fun(encoder_inc_data_fun);
//    Encoder_set_callback_decrement_fun(encoder_dec_data_fun);
//}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_inc_data_fun(void)
{
    uint8_t data_tmp;

    if (ctx.settings_data[ctx.current_setting].max_value == ctx.settings_data[ctx.current_setting].data)
    {
        ctx.settings_data[ctx.current_setting].data = 0U;
    }
    else
    {
        ctx.settings_data[ctx.current_setting].data++;
    }

    set_image();

    data_tmp = ctx.settings_data[CURRENT_SETTING_MODE].data;
    SynthCom_transmit(SYNTHCOM_SET_MODE, &data_tmp);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_dec_data_fun(void)
{
    uint8_t data_tmp;

    if (0U == ctx.settings_data[ctx.current_setting].data)
    {
        ctx.settings_data[ctx.current_setting].data = ctx.settings_data[ctx.current_setting].max_value;
    }
    else
    {
        ctx.settings_data[ctx.current_setting].data--;
    }

    set_image();

    data_tmp = ctx.settings_data[CURRENT_SETTING_MODE].data;
    SynthCom_transmit(SYNTHCOM_SET_MODE, &data_tmp);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_hold_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, false);
    Menu_set_main_encoder_callbacks();
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void set_image(void)
{
    OLED_deleteObject(ctx.mode_image_id);

    if (0U == ctx.settings_data[CURRENT_SETTING_MODE].data)
    {
        /* no image */
    }
    else if (1U == ctx.settings_data[CURRENT_SETTING_MODE].data)
    {
        OLED_createImage(&ctx.mode_image_id, 28U, 37U, mode_wavetable);
    }
    else if (2U == ctx.settings_data[CURRENT_SETTING_MODE].data)
    {
        OLED_createImage(&ctx.mode_image_id, 28U, 37U, mode_fm);
    }
}
