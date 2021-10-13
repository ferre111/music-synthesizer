/*
 * sin_gen.c
 *
 *  Created on: Jan 26, 2021
 *      Author: Karol Witusik
 */

#include "sin_gen.h"
#include "utility.h"

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

volatile struct sin_gen ctx = {.voices_tab = voices_tab, .dma_flag = true};

//----------------------------------------------------------------------

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
    ctx.table_ptr = ctx.table;

    HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*)ctx.table_ptr, PACKED_SIZE * 2);
}

//----------------------------------------------------------------------

void sin_gen_process(void)
{
    uint32_t currnet_sample = 0;
    int32_t scaling_factor = 0;

    if (ctx.dma_flag)
    {
        if (ctx.table_ptr == ctx.table)
        {
            ctx.table_ptr = &ctx.table[PACKED_SIZE];
        }
        else
        {
            ctx.table_ptr = ctx.table;
        }

        /* clear table */
        for (uint32_t i = 0; i < PACKED_SIZE; i++)
        {
            ctx.table_ptr[i] = 0;
        }

        /*calc scaling factor*/
    //    for(uint8_t i = 0; i < VOICES_COUNT; i++)
    //    {
    //        if(ctx.voices_tab[i].status == gen_status_ON || ctx.voices_tab[i].status == gen_status_SUSTAIN) scaling_factor++;
    //    }
        scaling_factor = VOICES_COUNT;
        //TODO
    //   scaling_factor = 1;

        for (uint8_t i = 0; i < VOICES_COUNT; i++)
        {
            do
            {
                switch(ctx.voices_tab[i].status)
                {
                case gen_status_ON:
                    ctx.table_ptr[currnet_sample * 2] += wavetable[ctx.voices_tab[i].sample_multiple] / scaling_factor;
                    break;
                case gen_status_SUSTAIN:
                    ctx.table_ptr[currnet_sample * 2] += ((int32_t)(PACKED_SIZE - (currnet_sample * 2)) * (int32_t)wavetable[ctx.voices_tab[i].sample_multiple] / PACKED_SIZE) / scaling_factor;
                    break;
                case gen_status_OFF:
                    break;
                }

                ctx.voices_tab[i].sample_multiple += ctx.voices_tab[i].freq;
                if (ctx.voices_tab[i].sample_multiple > (SAMPLE_COUNT - 1)) ctx.voices_tab[i].sample_multiple = ctx.voices_tab[i].sample_multiple % SAMPLE_COUNT;

                ctx.table_ptr[currnet_sample * 2 + 1] = 0; //second channel

                currnet_sample++;
            } while (0 != (PACKED_SIZE - currnet_sample * 2)); //multiple by two because there are two channels


            currnet_sample = 0;
            if(ctx.voices_tab[i].status == gen_status_SUSTAIN)
            {
                ctx.voices_tab[i].status = gen_status_OFF;
            }
        }

        ctx.dma_flag = false;
        ctx.buff_ready = true;
        //HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
    }
}

//----------------------------------------------------------------------

static void sin_gen_set_freq(uint8_t voice_index)
{
    uint8_t key_in_oct = ctx.voices_tab[voice_index].key_number % 12;
    uint8_t octave = ctx.voices_tab[voice_index].key_number / 12;
    ctx.voices_tab[voice_index].freq = note_freq[key_in_oct] * pow(2, octave);
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

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (&hi2s1 == hi2s)
    {
        if (false == ctx.buff_ready)
        {
            utility_ErrLedOn();
        }
        else
        {
            utility_ErrLedOff();
        }
        ctx.dma_flag = true;    //we need to change and fill buffer twice in one DMA transmission
        ctx.buff_ready = false;
    }
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (&hi2s1 == hi2s)
    {
        if (false == ctx.buff_ready)
        {
            utility_ErrLedOn();
        }
        else
        {
            utility_ErrLedOff();
        }
        ctx.dma_flag = true;    //we need to change and fill buffer twice in one DMA transmission
        ctx.buff_ready = false;
    }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    if(&hi2s1 == hi2s)
    {
        __NOP();
    }
}

//----------------------------------------------------------------------

bool get_flag(void)
{
    return ctx.dma_flag;
}

void set_flag(bool flag)
{
    ctx.dma_flag = flag;
}
