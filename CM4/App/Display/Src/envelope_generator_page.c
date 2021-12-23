/*
 * envelope_generator.c
 *
 *  Created on: Nov 18, 2021
 *      Author: karol
 */

#include <stdio.h>
#include <stdbool.h>
#include "envelope_generator_page.h"
#include "menu.h"
#include "buttons.h"
#include "encoder.h"
#include "synthcom.h"
#include "OLED.h"

//----------------------------------------------------------------------

#define MAX_SUSTAIN_LEVEL   100U
#define MAX_ATTACK_TIME     1000U
#define MAX_DECAY_TIME      1000U
#define MAX_RELEASE_TIME    1000U

#warning THIS DEFINES MUST BE THE SAME AS IN THE FILE envelope_generator_page.c
#define DEF_SUSTAIN_LEVEL   75U
#define DEF_ATTACK_TIME     250U
#define DEF_DECAY_TIME      100U
#define DEF_RELEASE_TIME    250U

//----------------------------------------------------------------------

typedef enum current_setting_T
{
    CURRENT_SETTING_SUSTAIN_LEVEL,
    CURRENT_SETTING_ATTACK_TIME,
    CURRENT_SETTING_DECAY_TIME,
    CURRENT_SETTING_RELEASE_TIME,

    Current_setting_end
} current_setting;

typedef struct setting_data_T
{
    uint16_t data;
    uint16_t max_value;
    uint8_t setting_id;
    char setting_txt[NUMBER_OF_LETTERS_IN_LINE];
} setting_data;

typedef struct envelop_generator_page_T
{
    uint8_t heading_id;
    char heading_txt[NUMBER_OF_LETTERS_IN_LINE];
    current_setting current_setting;
    setting_data    settings_data[Current_setting_end];
} envelop_generator_page;

//----------------------------------------------------------------------

static envelop_generator_page ctx = {.current_setting = CURRENT_SETTING_SUSTAIN_LEVEL,
                                     .settings_data =
                                       {
                                           [CURRENT_SETTING_SUSTAIN_LEVEL] = {.max_value = MAX_SUSTAIN_LEVEL, .data = DEF_SUSTAIN_LEVEL},
                                           [CURRENT_SETTING_ATTACK_TIME] = {.max_value = MAX_ATTACK_TIME, .data = DEF_ATTACK_TIME},
                                           [CURRENT_SETTING_DECAY_TIME] = {.max_value = MAX_DECAY_TIME, .data = DEF_DECAY_TIME},
                                           [CURRENT_SETTING_RELEASE_TIME] = {.max_value = MAX_RELEASE_TIME, .data = DEF_RELEASE_TIME}
                                       }
};

//----------------------------------------------------------------------

static void encoder_press_fun(void);
static void encoder_inc_browsing_fun(void);
static void encoder_dec_browsing_fun(void);
static void encoder_inc_data_fun(void);
static void encoder_dec_data_fun(void);
static void encoder_hold_fun(void);

//----------------------------------------------------------------------

void EnvelopeGenerator_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&ctx.heading_id, 22U, 0U, ctx.heading_txt, 2U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].setting_id, 0U, 16U, ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].setting_id, 0U, 24U, ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_DECAY_TIME].setting_id, 0U, 32U, ctx.settings_data[CURRENT_SETTING_DECAY_TIME].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].setting_id, 0U, 40U, ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].setting_txt, 1U, false);
    snprintf(ctx.heading_txt, NUMBER_OF_LETTERS_IN_LINE, "AMPL EG");
}

//----------------------------------------------------------------------

void EnvelopeGenerator_page_draw(void)
{
    /* update text strings printed on display with actual envelope generator data */
    snprintf(ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Sustain level: %*u", 6U, ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].data);
    snprintf(ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Attack time: %*ums", 6U, ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].data);
    snprintf(ctx.settings_data[CURRENT_SETTING_DECAY_TIME].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Decay time: %*ums", 7U, ctx.settings_data[CURRENT_SETTING_DECAY_TIME].data);
    snprintf(ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Release time: %*ums", 5U, ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].data);
}

//----------------------------------------------------------------------

void EnvelopeGenerator_encoder_enter_press_fun(void)
{
    Encoder_set_callback_increment_fun(encoder_inc_browsing_fun);
    Encoder_set_callback_decrement_fun(encoder_dec_browsing_fun);
    Buttons_set_callback_press_function(BUTTON_TYPES_ENCODER, encoder_press_fun);
    Buttons_set_callback_hold_function(BUTTON_TYPES_ENCODER, encoder_hold_fun);
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, true);
}

//----------------------------------------------------------------------

void EnvelopeGenerator_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(ctx.heading_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_DECAY_TIME].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].setting_id);
}

//----------------------------------------------------------------------


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

//----------------------------------------------------------------------

static void encoder_inc_browsing_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, false);

    if (CURRENT_SETTING_RELEASE_TIME == ctx.current_setting)
    {
        ctx.current_setting = CURRENT_SETTING_SUSTAIN_LEVEL;
    }
    else
    {
        ctx.current_setting++;
    }

    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, true);
}

//----------------------------------------------------------------------

static void encoder_dec_browsing_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, false);

    if (CURRENT_SETTING_SUSTAIN_LEVEL == ctx.current_setting)
    {
        ctx.current_setting = CURRENT_SETTING_RELEASE_TIME;
    }
    else
    {
        ctx.current_setting--;
    }

    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, true);
}

//----------------------------------------------------------------------

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

    SynthCom_transmit(SYNTHCOM_ENVELOPE_GENERATOR_DATA, data_tmp);
}

//----------------------------------------------------------------------

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

    SynthCom_transmit(SYNTHCOM_ENVELOPE_GENERATOR_DATA, data_tmp);
}

//----------------------------------------------------------------------

static void encoder_hold_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, false);
    Menu_set_main_encoder_callbacks();
}

