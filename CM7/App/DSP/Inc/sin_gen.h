/*
 * sin_gen.h
 *
 *  Created on: Jan 26, 2021
 *      Author: Karol Witusik
 */

#ifndef __SIN_GEN_H
#define __SIN_GEN_H

#include "wavetable.h"
#include "main.h"
#include "stdbool.h"

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

typedef struct sin_gen_voice_T
{
    voice_status voice_status;
    uint32_t freq;
    uint32_t current_sample;
    uint8_t key_number;
    uint8_t velocity;

    uint32_t attack_counter;
    uint32_t decay_counter;
    uint32_t release_counter;
} sin_gen_voice;

typedef struct sin_gen_T
{
    /* if this flag is set sin_gen_process should fill buffer */
    volatile bool dma_flag;
    /* make one "double" array, with this we can use DMA in circular mode, and it is not required to again set transmission when whole buffer has been sent */
    int16_t table[PACKED_SIZE * 2];
    /* pointer to currently edited "virtual" buffer */
    int16_t *table_ptr;
    /* pointer to array with structures specifying actual playing waves */
    sin_gen_voice *voices_tab;
    /* with this flag we can check whether current "virtual" buffer has been filled */
    volatile bool buff_ready;
} sin_gen;

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

typedef struct sin_gen_envelop_generator_T
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
    /* max amplitude during transition to Release state, this maximal amplitude value results from a linear equation related with ADSR envelope generator */
    double max_ampl_release_transition;
} sin_gen_envelop_generator;

/*------------------------------------------------------------------------------------------------------------------------------*/

void sin_gen_process(void);
void sin_gen_init(void);
void sin_gen_set_voice_start_play(uint8_t key_number);
void sin_gen_set_voice_stop_play(uint8_t key_number);
void sin_gen_set_envelop_generator(uint8_t sustain_level, uint32_t attack_time, uint32_t decay_time, uint32_t release_time);

#endif
