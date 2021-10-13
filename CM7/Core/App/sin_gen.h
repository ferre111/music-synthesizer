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

#define PACKED_SIZE 4800 //size of data (in words) sends by one DMA transfer
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

typedef enum gen_status_T
{
    gen_status_ON,
    gen_status_SUSTAIN,
    gen_status_OFF
} gen_status;

struct sin_gen_voices
{
    enum gen_status status;
    uint32_t freq;
    uint32_t sample_multiple;
    uint8_t key_number;
    uint16_t sample_offset;
};

struct sin_gen
{
    volatile bool dma_flag;
    int16_t table[PACKED_SIZE * 2];     //make one "double" array, with this we can use DMA in circular mode
                                        //and it is not required to again set transmission when whole buffer has been sent
    int16_t *table_ptr;                 //pointer to currently edited "virtual" buffer
    struct sin_gen_voices *voices_tab;  //

    bool buff_ready;//todo
    uint32_t max_len;
};

float note_freq[12];

void sin_gen_process(void);
void sin_gen_init(void);
void sin_gen_set_play(bool flag, uint8_t key_number);

bool get_flag(void);
void set_flag(bool flag);

