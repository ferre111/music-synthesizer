/*
 * sin_gen.h
 *
 *  Created on: Jan 26, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "wavetable.h"
#include "main.h"
#include "stdbool.h"
#include "i2s.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

/* size of data (in words) sends by one DMA transfer */
#define PACKED_SIZE 4800

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef enum note_T
{
    note_C,
    note_Db,
    note_D,
    note_Eb,
    note_E,
    note_F,
    note_Gb,
    note_G,
    note_Ab,
    note_A,
    note_Bb,
    note_B
} note;

typedef enum voice_status_T
{
    voice_status_Attack,
    voice_status_Decay,
    voice_status_Sustain,
    voice_status_Release,
    voice_status_Off
} voice_status;

/*------------------------------------------------------------------------------------------------------------------------------*/

struct sin_gen_voices
{
    voice_status voice_status;
    uint32_t freq;
    uint32_t sample_multiple;
    uint8_t key_number;

    uint32_t attack_counter;
    uint32_t decay_counter;
    uint32_t release_counter;
};

struct sin_gen
{
    /* if this flag is set sin_gen_process should fill buffer */
    volatile bool dma_flag;
    /* make one "double" array, with this we can use DMA in circular mode, and it is not required to again set transmission when whole buffer has been sent */
    int16_t table[PACKED_SIZE * 2];
    /* pointer to currently edited "virtual" buffer */
    int16_t *table_ptr;
    /* pointer to array with structures specifying actual playing waves */
    struct sin_gen_voices *voices_tab;
    /* with this flag we can check whether current "virtual" buffer has been filled */
    volatile bool buff_ready;
};

/*
 *            /|\
 *           / | \
 *          /  |  \
 *         /   |   \
 *        /    |    \
 *       /     |     \________________  Sustain level
 *      /      |     |                |\
 *     /       |     |                | \
 *    /        |     |                |  \
 * __/         |     |                |   \__
 *     Attack             Sustain
 *              Decay                  Release
 */

struct sin_gen_envelop_generator
{
    /* sustain level, 1 - 100%, 0.5 - 50% */
    uint8_t sustain_level;
    /* time as a 1/480000s */
    uint32_t attack_time;
    uint32_t decay_time;
    uint32_t release_time;
    /* coefficients */
    double attack_coef;
    double decay_coef;
    double sustain_coef;
    double release_coef;
};

/*------------------------------------------------------------------------------------------------------------------------------*/

void sin_gen_process(void);
void sin_gen_init(void);
void sin_gen_set_play(bool flag, uint8_t key_number);
void sin_gen_set_envelop_generator(double sustain_level, uint32_t attack_time, uint32_t decay_time, uint32_t release_time);
