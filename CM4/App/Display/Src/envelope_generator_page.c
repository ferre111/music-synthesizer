/*
 * envelope_generator.c
 *
 *  Created on: Nov 18, 2021
 *      Author: karol
 */

#include <stdbool.h>
#include "envelope_generator_page.h"
#include "menu.h"
#include "buttons.h"
#include "encoder.h"
#include "synthcom.h"

//----------------------------------------------------------------------

#define NUMBER_OF_LETTERS_IN_LINE 22U

#define MAX_SUSTAIN_LEVEL   100U
#define MAX_ATTACK_TIME     1000U
#define MAX_DECAY_TIME      1000U
#define MAX_RELEASE_TIME    1000U

/* WARNING! THIS DEFINES MUST BE THE SAME AS IN THE FILE sin_gen.c*/
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
    uint8_t envelop_generator_setting_id;
    char envelop_generator_setting_txt[NUMBER_OF_LETTERS_IN_LINE];
} setting_data;

typedef struct envelop_generator_page_T
{
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
static void encoder_hold_fun(void);
static void encoder_inc_browsing_fun(void);
static void encoder_dec_browsing_fun(void);
static void encoder_inc_data_fun(void);
static void encoder_dec_data_fun(void);

//----------------------------------------------------------------------

void envelope_generator_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].envelop_generator_setting_id, 0U, 0U, ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].envelop_generator_setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].envelop_generator_setting_id, 0U, 8U, ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].envelop_generator_setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_DECAY_TIME].envelop_generator_setting_id, 0U, 16U, ctx.settings_data[CURRENT_SETTING_DECAY_TIME].envelop_generator_setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].envelop_generator_setting_id, 0U, 24U, ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].envelop_generator_setting_txt, 1U, false);
}

//----------------------------------------------------------------------

void envelope_generator_page_draw(void)
{
    /* update text strings printed on display with actual envelope generator data */
    snprintf(ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].envelop_generator_setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Sustain level: %*u", 6U, ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].data);
    snprintf(ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].envelop_generator_setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Attack time: %*u", 8U, ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].data);
    snprintf(ctx.settings_data[CURRENT_SETTING_DECAY_TIME].envelop_generator_setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Decay time: %*u", 9U, ctx.settings_data[CURRENT_SETTING_DECAY_TIME].data);
    snprintf(ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].envelop_generator_setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Release time: %*u", 7U, ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].data);
}

//----------------------------------------------------------------------

void envelope_generator_encoder_enter_press_fun(void)
{
    Encoder_set_callback_increment_fun(encoder_inc_browsing_fun);
    Encoder_set_callback_decrement_fun(encoder_dec_browsing_fun);
    Buttons_set_callback_press_function(BUTTON_TYPES_ENCODER, encoder_press_fun);
    Buttons_set_callback_hold_function(BUTTON_TYPES_ENCODER, encoder_hold_fun);
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].envelop_generator_setting_id, true);
}

//----------------------------------------------------------------------

void envelope_generator_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_SUSTAIN_LEVEL].envelop_generator_setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_ATTACK_TIME].envelop_generator_setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_DECAY_TIME].envelop_generator_setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_RELEASE_TIME].envelop_generator_setting_id);
}
//----------------------------------------------------------------------


static void encoder_press_fun(void)
{
    static bool data_edited_flag;

    data_edited_flag ^= true;

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
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].envelop_generator_setting_id, false);

    if (CURRENT_SETTING_RELEASE_TIME == ctx.current_setting)
    {
        ctx.current_setting = CURRENT_SETTING_SUSTAIN_LEVEL;
    }
    else
    {
        ctx.current_setting++;
    }

    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].envelop_generator_setting_id, true);
}

//----------------------------------------------------------------------

static void encoder_dec_browsing_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].envelop_generator_setting_id, false);

    if (CURRENT_SETTING_SUSTAIN_LEVEL == ctx.current_setting)
    {
        ctx.current_setting = CURRENT_SETTING_RELEASE_TIME;
    }
    else
    {
        ctx.current_setting--;
    }

    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].envelop_generator_setting_id, true);
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
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].envelop_generator_setting_id, false);
    Menu_set_main_encoder_callbacks();
}

