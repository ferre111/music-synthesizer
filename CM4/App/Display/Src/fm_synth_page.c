/*
 * fm_synth_page.c
 *
 *  Created on: Dec 12, 2021
 *      Author: User
 */

#include <stdio.h>
#include "fm_synth_page.h"
#include "menu.h"
#include "buttons.h"
#include "encoder.h"
#include "synthcom.h"
#include "OLED.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

#define MAX_SHAPE           3U
#define MAX_MOD_INDEX       100U
#define MAX_FREQ_MODE       7U
#define MAX_FREQ_MOD_INDEP  20000U
#define MAX_VOLUME          100U


#define DEF_SHAPE           0U
#define DEF_MOD_INDEX       10U
#define DEF_FREQ_MODE       0U
#define DEF_FREQ            3U
#define DEF_VOLUME          100U

/*------------------------------------------------------------------------------------------------------------------------------*/

static char shape_txt[4][NUMBER_OF_LETTERS_IN_LINE] =
{
        "sine",
        "square",
        "triangle",
        "saw"
};

static char freq_mode_txt[8][NUMBER_OF_LETTERS_IN_LINE] =
{
        "indep",
        "key fx0.25",
        "key fx0.33",
        "key fx0.5",
        "key fx1",
        "key fx2",
        "key fx3",
        "key fx4"
};

typedef enum current_setting_T
{
    CURRENT_SETTING_CARRIER_SHAPE_OSC,
    CURRENT_SETTING_MODU_SHAPE_OSC,
    CURRENT_SETTING_MOD_INDEX,
    CURRENT_SETTING_FREQ_MODE,
    CURRENT_SETTING_FREQ,
    CURRENT_SETTING_VOLUME,

    Current_setting_end
} current_setting;

typedef enum frequency_mode_T
{
    FREQUENCY_MODE_INDEPENDENT,
    FREQUENCY_MODE_DEPENDENT
} frequency_mode;

typedef struct setting_data_T
{
    uint16_t data;
    uint16_t max_value;
    uint8_t setting_id;
    char setting_txt[NUMBER_OF_LETTERS_IN_LINE];
} setting_data;

typedef struct fm_synth_page_T
{
    uint8_t heading_id;
    char heading_txt[NUMBER_OF_LETTERS_IN_LINE];
    current_setting current_setting;
    setting_data    settings_data[Current_setting_end];
} fm_synth_page;

static fm_synth_page ctx = {.current_setting = CURRENT_SETTING_CARRIER_SHAPE_OSC,
                                     .settings_data =
                                       {
                                           [CURRENT_SETTING_CARRIER_SHAPE_OSC]    = {.max_value = MAX_SHAPE, .data = DEF_SHAPE},
                                           [CURRENT_SETTING_MODU_SHAPE_OSC]    = {.max_value = MAX_SHAPE, .data = DEF_SHAPE},
                                           [CURRENT_SETTING_MOD_INDEX]      = {.max_value = MAX_MOD_INDEX, .data = DEF_MOD_INDEX},
                                           [CURRENT_SETTING_FREQ_MODE]      = {.max_value = MAX_FREQ_MODE, .data = DEF_FREQ_MODE},
                                           [CURRENT_SETTING_FREQ]           = {.max_value = MAX_FREQ_MOD_INDEP, .data = DEF_FREQ},
                                           [CURRENT_SETTING_VOLUME]         = {.max_value = MAX_VOLUME, .data = DEF_VOLUME},
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

void FMSynth_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&ctx.heading_id, 52U, 0U, ctx.heading_txt, 2U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_CARRIER_SHAPE_OSC].setting_id, 0U, 16U, ctx.settings_data[CURRENT_SETTING_CARRIER_SHAPE_OSC].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_MODU_SHAPE_OSC].setting_id, 0U, 24U, ctx.settings_data[CURRENT_SETTING_MODU_SHAPE_OSC].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_MOD_INDEX].setting_id, 0U, 32U, ctx.settings_data[CURRENT_SETTING_MOD_INDEX].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_FREQ_MODE].setting_id, 0U, 40U, ctx.settings_data[CURRENT_SETTING_FREQ_MODE].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_FREQ].setting_id, 0U, 48U, ctx.settings_data[CURRENT_SETTING_FREQ].setting_txt, 1U, false);
    OLED_createTextField(&ctx.settings_data[CURRENT_SETTING_VOLUME].setting_id, 0U, 56U, ctx.settings_data[CURRENT_SETTING_VOLUME].setting_txt, 1U, false);
    snprintf(ctx.heading_txt, NUMBER_OF_LETTERS_IN_LINE, "FM");
}

/*------------------------------------------------------------------------------------------------------------------------------*/


void FMSynth_page_draw(void)
{
    /* update text strings printed on display with actual envelope generator data */
    snprintf(ctx.settings_data[CURRENT_SETTING_CARRIER_SHAPE_OSC].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Osc C shape: %*s", 8U, shape_txt[ctx.settings_data[CURRENT_SETTING_CARRIER_SHAPE_OSC].data]);
    snprintf(ctx.settings_data[CURRENT_SETTING_MODU_SHAPE_OSC].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Osc M shape: %*s", 8U, shape_txt[ctx.settings_data[CURRENT_SETTING_MODU_SHAPE_OSC].data]);
    snprintf(ctx.settings_data[CURRENT_SETTING_MOD_INDEX].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Modulation index: %u.%u", ctx.settings_data[CURRENT_SETTING_MOD_INDEX].data / 10, ctx.settings_data[CURRENT_SETTING_MOD_INDEX].data % 10);
    snprintf(ctx.settings_data[CURRENT_SETTING_FREQ_MODE].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Freq mode: %*s", 10U, freq_mode_txt[ctx.settings_data[CURRENT_SETTING_FREQ_MODE].data]);
    snprintf(ctx.settings_data[CURRENT_SETTING_FREQ].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Frequency: %*u", 10U, ctx.settings_data[CURRENT_SETTING_FREQ].data);
    snprintf(ctx.settings_data[CURRENT_SETTING_VOLUME].setting_txt, NUMBER_OF_LETTERS_IN_LINE, "Volume: %*u", 13U, ctx.settings_data[CURRENT_SETTING_VOLUME].data);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void FMSynth_encoder_enter_press_fun(void)
{
    Encoder_set_callback_increment_fun(encoder_inc_browsing_fun);
    Encoder_set_callback_decrement_fun(encoder_dec_browsing_fun);
    Buttons_set_callback_press_function(BUTTON_TYPES_ENCODER, encoder_press_fun);
    Buttons_set_callback_hold_function(BUTTON_TYPES_ENCODER, encoder_hold_fun);
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, true);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void FMSynth_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(ctx.heading_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_CARRIER_SHAPE_OSC].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_MODU_SHAPE_OSC].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_MOD_INDEX].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_FREQ_MODE].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_FREQ].setting_id);
    OLED_deleteObject(ctx.settings_data[CURRENT_SETTING_VOLUME].setting_id);
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

    if (Current_setting_end - 1U == ctx.current_setting)
    {
        ctx.current_setting = CURRENT_SETTING_CARRIER_SHAPE_OSC;
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

    if (CURRENT_SETTING_CARRIER_SHAPE_OSC == ctx.current_setting)
    {
        ctx.current_setting = Current_setting_end - 1U;
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

    SynthCom_transmit(SYNTHCOM_FM_SYNTHESIS_DATA, data_tmp);

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

    SynthCom_transmit(SYNTHCOM_FM_SYNTHESIS_DATA, data_tmp);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void encoder_hold_fun(void)
{
    OLED_textFieldSetReverse(ctx.settings_data[ctx.current_setting].setting_id, false);
    Menu_set_main_encoder_callbacks();
}
