/*
 * envelope_generator.c
 *
 *  Created on: Nov 18, 2021
 *      Author: karol
 */

#include <stdbool.h>
#include "envelope_generator_page.h"
#include "menu.h"

//----------------------------------------------------------------------

#define ENVELOP_GENERATOR_SETTINGS_COUNT 4U
#define NUMBER_OF_LETTERS_IN_LINE 20U

//----------------------------------------------------------------------

struct
{
    /* sustain level, 1 - 100%, 0.5 - 50% */
    uint8_t sustain_level;
    /* time in ms */
    uint8_t attack_time;
    uint8_t decay_time;
    uint8_t release_time;
} envelop_generator_data;

//----------------------------------------------------------------------

/* variables to store drawable object IDs and char arrays to store strings for TextFields */
static uint8_t envelop_generator_settings_id[ENVELOP_GENERATOR_SETTINGS_COUNT];
static char envelop_generator_settings_txt[ENVELOP_GENERATOR_SETTINGS_COUNT][NUMBER_OF_LETTERS_IN_LINE];

//----------------------------------------------------------------------

void envelope_generator_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&envelop_generator_settings_id[0], 0U, 0U, envelop_generator_settings_txt[0], 1U, true);
    OLED_createTextField(&envelop_generator_settings_id[1], 0U, 8U, envelop_generator_settings_txt[1], 1U, true);
    OLED_createTextField(&envelop_generator_settings_id[2], 0U, 16U, envelop_generator_settings_txt[2], 1U, true);
    OLED_createTextField(&envelop_generator_settings_id[3], 0U, 24U, envelop_generator_settings_txt[3], 1U, true);
    snprintf(envelop_generator_settings_txt[0], NUMBER_OF_LETTERS_IN_LINE, "Sustain level: ");
    snprintf(envelop_generator_settings_txt[1], NUMBER_OF_LETTERS_IN_LINE, "Attack time: ");
    snprintf(envelop_generator_settings_txt[2], NUMBER_OF_LETTERS_IN_LINE, "Decay time: ");
    snprintf(envelop_generator_settings_txt[3], NUMBER_OF_LETTERS_IN_LINE, "Release time: ");
}

//----------------------------------------------------------------------

void envelope_generator_page_draw(void)
{
    /* update text strings printed on display with actual envelope generator data */
    snprintf(envelop_generator_settings_txt[0], NUMBER_OF_LETTERS_IN_LINE, "Sustain level: ");
    snprintf(envelop_generator_settings_txt[1], NUMBER_OF_LETTERS_IN_LINE, "attack time: ");
    snprintf(envelop_generator_settings_txt[2], NUMBER_OF_LETTERS_IN_LINE, "decay time: ");
    snprintf(envelop_generator_settings_txt[3], NUMBER_OF_LETTERS_IN_LINE, "release time: ");
}

//----------------------------------------------------------------------

void envelope_generator_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(envelop_generator_settings_id[0]);
    OLED_deleteObject(envelop_generator_settings_id[1]);
    OLED_deleteObject(envelop_generator_settings_id[2]);
    OLED_deleteObject(envelop_generator_settings_id[3]);
}
