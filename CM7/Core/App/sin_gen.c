/*
 * sin_gen.c
 *
 *  Created on: Jan 26, 2021
 *      Author: Karol Witusik
 */

#include "sin_gen.h"

//todo
extern uint16_t buf[48000];

//----------------------------------------------------------------------

struct sin_gen_voices voices_tab[VOICES_COUNT] =
{
        [0] = {.status = gen_status_OFF, .freq = 100},
        [1] = {.status = gen_status_OFF, .freq = 100},
        [2] = {.status = gen_status_OFF, .freq = 100},
        [3] = {.status = gen_status_OFF, .freq = 100},
        [4] = {.status = gen_status_OFF, .freq = 100},
        [5] = {.status = gen_status_OFF, .freq = 100},
        [6] = {.status = gen_status_OFF, .freq = 100},
        [7] = {.status = gen_status_OFF, .freq = 100},
        [8] = {.status = gen_status_OFF, .freq = 100},
        [9] = {.status = gen_status_OFF, .freq = 100}
};

struct sin_gen ctx = {.voices_tab = voices_tab, .dma_flag = true};

//----------------------------------------------------------------------

static void calc_len(uint8_t voice_index);
static void sin_gen_set_freq(uint8_t voice_index);

//----------------------------------------------------------------------

float note_freq[12] =
{
    [note_C]    = 16.35,
    [note_Db]   = 17.32,
    [note_D]    = 18.35,
    [note_Eb]   = 19.45,
    [note_E]    = 20.60,
    [note_F]    = 21.83,
    [note_Gb]   = 23.12,
    [note_G]    = 24.50,
    [note_Ab]   = 25.96,
    [note_A]    = 27.50,
    [note_Bb]   = 29.14,
    [note_B]    = 30.87
};

//----------------------------------------------------------------------

void sin_gen_init(void)
{
    ctx.table = ctx.table_1;

    for(uint8_t i = 0; i < VOICES_COUNT; i++)
        calc_len(i);
}

//----------------------------------------------------------------------

void sin_gen_process(void)
{
    int32_t max_len = 0;
    uint32_t tmp_len = 0;
    int32_t scaling_factor = 0;

    /*calc max len*/
    for(uint8_t i = 0; i < VOICES_COUNT; i++)
    {
        if(ctx.voices_tab[i].len > max_len)
        {
            max_len = ctx.voices_tab[i].len;
        }
    }

    if(ctx.table == ctx.table_1)
    {
        ctx.table = ctx.table_2;
    }
    else
    {
        ctx.table = ctx.table_1;
    }

    /*clear table */
    for(uint32_t i = 0; i < COUNT_OF_SAMPLE_SEND_ARR; i++)
    {
        ctx.table[i] = 0;
    }

    /*calc scaling factor*/
//    for(uint8_t i = 0; i < VOICES_COUNT; i++)
//    {
//        if(ctx.voices_tab[i].status == gen_status_ON || ctx.voices_tab[i].status == gen_status_SUSTAIN) scaling_factor++;
//    }
    scaling_factor = VOICES_COUNT;
    //TODO
    scaling_factor = 1;

    for(uint8_t i = 0; i < VOICES_COUNT; i++)
    {
        while(max_len - tmp_len) //todo sprawdz se
        {
            switch(ctx.voices_tab[i].status)
            {
            case gen_status_ON:
                ctx.table[tmp_len * 2] += wavetable[ctx.voices_tab[i].sample_index] / scaling_factor;
                break;
            case gen_status_SUSTAIN:
                ctx.table[tmp_len * 2] += ((max_len - (int32_t)tmp_len) * wavetable[ctx.voices_tab[i].sample_index] / max_len) / scaling_factor;
                break;
            case gen_status_OFF:
                ctx.voices_tab[i].sample_index = 0;
                ctx.table[tmp_len * 2] += 0;
                break;
            }

            ctx.table[tmp_len * 2 + 1] = 0; //second channel
            ctx.voices_tab[i].sample_index += ctx.voices_tab[i].freq;
            if(ctx.voices_tab[i].sample_index > (SAMPLE_COUNT - 1)) ctx.voices_tab[i].sample_index = ctx.voices_tab[i].sample_index % SAMPLE_COUNT;
            tmp_len++;
        }
        tmp_len = 0;
        if(ctx.voices_tab[i].status == gen_status_SUSTAIN)
        {
            ctx.voices_tab[i].status = gen_status_OFF;
        }
    }

    while(!ctx.dma_flag);
    HAL_I2S_Transmit_DMA(&hi2s1, ctx.table, max_len * 2);
//    HAL_I2S_Transmit_DMA(&hi2s1, buf, 48000);
    ctx.dma_flag = false;
    //HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
}

//----------------------------------------------------------------------

static void sin_gen_set_freq(uint8_t voice_index)
{
    uint8_t key_in_oct = ctx.voices_tab[voice_index].key_number % 12;
    uint8_t octave = ctx.voices_tab[voice_index].key_number / 12;
    ctx.voices_tab[voice_index].freq = note_freq[key_in_oct] * pow(2, octave);

    calc_len(voice_index);
}

//----------------------------------------------------------------------

void sin_gen_set_play(bool flag, uint8_t key_number)
{
    if(flag)
    {
        for(uint8_t i = 0; i < VOICES_COUNT; i++)
        {
            if(ctx.voices_tab[i].status == gen_status_OFF)
            {
                ctx.voices_tab[i].status = gen_status_ON;
                ctx.voices_tab[i].key_number = key_number;
                sin_gen_set_freq(i);

                return;
            }
        }
    }
    else
    {
        for(uint8_t i = 0; i < VOICES_COUNT; i++)
        {
            if(ctx.voices_tab[i].key_number == key_number && ctx.voices_tab[i].status == gen_status_ON)
            {
                ctx.voices_tab[i].status = gen_status_SUSTAIN;
                return;
            }
        }
    }
}

//----------------------------------------------------------------------

static void calc_len(uint8_t voice_index)
{
    ctx.voices_tab[voice_index].len = BASIC_FREQ * (SAMPLE_COUNT) / ctx.voices_tab[voice_index].freq;
}

//----------------------------------------------------------------------

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    ctx.dma_flag = true;
}

//----------------------------------------------------------------------

//void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
//{
//    dma_flag = 1;
//}

bool get_flag(void)
{
    return ctx.dma_flag;
}

void set_flag(bool flag)
{
    ctx.dma_flag = flag;
}
