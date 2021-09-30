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

#define COUNT_OF_SAMPLE_SEND_ARR 1000
#define VOICES_COUNT 10


enum note
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
};

enum gen_status
{
    gen_status_ON,
    gen_status_SUSTAIN,
    gen_status_OFF
};

struct sin_gen_voices
{
    uint32_t freq;
    uint32_t len;
    enum gen_status status;
    uint32_t sample_index;
    uint8_t key_number;
};

struct sin_gen
{
    bool dma_flag;
    int16_t table_1[COUNT_OF_SAMPLE_SEND_ARR];
    int16_t table_2[COUNT_OF_SAMPLE_SEND_ARR];
    int16_t *table;
    struct sin_gen_voices *voices_tab;
};

float note_freq[12];

void sin_gen_process(void);
void sin_gen_init(void);
void sin_gen_set_play(bool flag, uint8_t key_number);

bool get_flag(void);
void set_flag(bool flag);

