/*
 * sin_gen.c
 *
 *  Created on: Jan 26, 2021
 *      Author: Karol Witusik
 */

#include "sin_gen.h"
#include "utility.h"
#include "string.h"
#include "arm_math.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

#define VOICES_COUNT 10

/* default envelop generator values */
#define DEF_SUSTAIN_LEVEL 50
#define DEF_ATTACK_TIME 2000
#define DEF_DECAY_TIME 2000
#define DEF_RELEASE_TIME 2000

/*------------------------------------------------------------------------------------------------------------------------------*/

/* array with "voices", one voice - one wave (e.g. sine) on the output */
static sin_gen_voices voices_tab[VOICES_COUNT] =
{
        [0] = {.voice_status = voice_status_Off},
        [1] = {.voice_status = voice_status_Off},
        [2] = {.voice_status = voice_status_Off},
        [3] = {.voice_status = voice_status_Off},
        [4] = {.voice_status = voice_status_Off},
        [5] = {.voice_status = voice_status_Off},
        [6] = {.voice_status = voice_status_Off},
        [7] = {.voice_status = voice_status_Off},
        [8] = {.voice_status = voice_status_Off},
        [9] = {.voice_status = voice_status_Off}
};

static sin_gen ctx __attribute((section(".sin_gen_ctx"))) = {.voices_tab = voices_tab, .dma_flag = true};
static sin_gen_envelop_generator eg_ctx;

/*------------------------------------------------------------------------------------------------------------------------------*/

static void sin_gen_set_freq(uint8_t voice_index);
static void sin_gen_update_envelop_generator_coef_update(void);
inline static void sin_gen_write_one_sample(uint32_t current_sample, int16_t value);

/*------------------------------------------------------------------------------------------------------------------------------*/

/* Basic notes array */
const static float note_freq[12] =
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

/*------------------------------------------------------------------------------------------------------------------------------*/

void sin_gen_init(void)
{
    ctx.table_ptr = ctx.table;

    sin_gen_set_envelop_generator(DEF_SUSTAIN_LEVEL, DEF_ATTACK_TIME, DEF_DECAY_TIME, DEF_RELEASE_TIME);
    HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*)ctx.table_ptr, PACKED_SIZE * 2);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void sin_gen_process(void)
{
    uint32_t current_sample = 0;

    if (ctx.dma_flag)
    {
        utility_TimeMeasurmentsSetHigh();

        /* Choose buffer that is not currently send */
        if (ctx.table_ptr == ctx.table)
        {
            ctx.table_ptr = &ctx.table[PACKED_SIZE];
        }
        else
        {
            ctx.table_ptr = ctx.table;
        }

        /* clear table, multiple by two due to uint16_t */
        memset(ctx.table_ptr, 0U, PACKED_SIZE * 2U);

        for (uint8_t i = 0; i < VOICES_COUNT; i++)
        {
            do
            {
                switch(ctx.voices_tab[i].voice_status)
                {
                case voice_status_Attack:
                    sin_gen_write_one_sample(current_sample, (int16_t)(eg_ctx.attack_coef * (double)ctx.voices_tab[i].attack_counter * (double)wavetable[ctx.voices_tab[i].sample_multiple] / (double)VOICES_COUNT));

                    if (ctx.voices_tab[i].attack_counter++ > eg_ctx.attack_time)
                    {
                        ctx.voices_tab[i].voice_status = voice_status_Decay;
                        ctx.voices_tab[i].attack_counter = 0U;
                    }
                    break;
                case voice_status_Decay:
                    sin_gen_write_one_sample(current_sample, (int16_t)((eg_ctx.decay_coef * (double)ctx.voices_tab[i].decay_counter + 1.0) * (double)wavetable[ctx.voices_tab[i].sample_multiple] / (double)VOICES_COUNT));

                    if (ctx.voices_tab[i].decay_counter++ > eg_ctx.decay_time)
                    {
                        ctx.voices_tab[i].voice_status = voice_status_Sustain;
                        ctx.voices_tab[i].decay_counter = 0U;
                    }
                    break;
                case voice_status_Sustain:
                    sin_gen_write_one_sample(current_sample, (int16_t)(eg_ctx.sustain_coef * (double)wavetable[ctx.voices_tab[i].sample_multiple] / (double)VOICES_COUNT));
                    break;
                case voice_status_Release:
                    sin_gen_write_one_sample(current_sample, (int16_t)((eg_ctx.release_coef * (double)ctx.voices_tab[i].release_counter + (double)eg_ctx.sustain_level / 100.0) * (double)wavetable[ctx.voices_tab[i].sample_multiple] / (double)VOICES_COUNT));

                    if (ctx.voices_tab[i].release_counter++ > eg_ctx.release_time)
                    {
                        ctx.voices_tab[i].voice_status = voice_status_Off;
                        ctx.voices_tab[i].release_counter = 0U;
                    }
                    break;
                case voice_status_Off:
                    break;
                }

                ctx.voices_tab[i].sample_multiple += ctx.voices_tab[i].freq;
                if (ctx.voices_tab[i].sample_multiple > (SAMPLE_COUNT - 1)) ctx.voices_tab[i].sample_multiple = ctx.voices_tab[i].sample_multiple % SAMPLE_COUNT;

                current_sample += 2;
            } while (0 != (PACKED_SIZE - current_sample));
            current_sample = 0;
        }
        /* Clean DCache after filling whole table */
//        SCB_CleanDCache_by_Addr((uint32_t*)ctx.table_ptr, PACKED_SIZE * 2);
        ctx.dma_flag = false;
        ctx.buff_ready = true;
        utility_TimeMeasurmentsSetLow();
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void sin_gen_set_envelop_generator(uint8_t sustain_level, uint32_t attack_time, uint32_t decay_time, uint32_t release_time)
{
    eg_ctx.sustain_level = sustain_level;
    eg_ctx.attack_time = attack_time;
    eg_ctx.decay_time = decay_time;
    eg_ctx.release_time = release_time;

    sin_gen_update_envelop_generator_coef_update();
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void sin_gen_update_envelop_generator_coef_update(void)
{
    eg_ctx.attack_coef = 1.0 / (double)eg_ctx.attack_time;
    eg_ctx.decay_coef = ((double)eg_ctx.sustain_level / 100.0 - 1.0) / (double)eg_ctx.decay_time;
    eg_ctx.sustain_coef = (double)eg_ctx.sustain_level / 100.0;
    eg_ctx.release_coef = -(double)eg_ctx.sustain_level / 100.0 / (double)eg_ctx.release_time;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void sin_gen_set_freq(uint8_t voice_index)
{
    uint8_t key_in_oct = ctx.voices_tab[voice_index].key_number % 12;
    uint8_t octave = ctx.voices_tab[voice_index].key_number / 12;
    ctx.voices_tab[voice_index].freq = note_freq[key_in_oct] * pow(2, octave);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void sin_gen_set_play(bool flag, uint8_t key_number)
{
    if (flag)
    {
        for (uint8_t i = 0; i < VOICES_COUNT; i++)
        {
            if (voice_status_Off == ctx.voices_tab[i].voice_status)
            {
                ctx.voices_tab[i].voice_status = voice_status_Attack;
                ctx.voices_tab[i].key_number = key_number;
                sin_gen_set_freq(i);

                return;
            }
        }
    }
    else
    {
        for (uint8_t i = 0; i < VOICES_COUNT; i++)
        {
            if (ctx.voices_tab[i].key_number == key_number && voice_status_Off != ctx.voices_tab[i].voice_status)
            {
                ctx.voices_tab[i].voice_status = voice_status_Release;
                return;
            }
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/* Since it is monophonic devices we write same data to both channel */
inline static void sin_gen_write_one_sample(uint32_t current_sample, int16_t value)
{
    ctx.table_ptr[current_sample] += value;
    ctx.table_ptr[current_sample + 1] += value;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (&hi2s1 == hi2s)
    {
        if (false == ctx.buff_ready)
        {
            utility_ErrLedOn();
        }

        /* we need to change and fill buffer twice in one DMA transmission */
        ctx.dma_flag = true;
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

        /* we need to change and fill buffer twice in one DMA transmission */
        ctx.dma_flag = true;
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
