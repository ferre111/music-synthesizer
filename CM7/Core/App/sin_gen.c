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

/* array with "voices", one voice - one wave (e.g. sine) on the output */
static struct sin_gen_voices voices_tab[VOICES_COUNT] =
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

static struct sin_gen ctx = {.voices_tab = voices_tab, .dma_flag = true};
static struct sin_gen_envelop_generator eg_ctx = {.attack_time = 2000, .decay_time = 2000, .release_time = 2000, .sustain_level = 50};

/*------------------------------------------------------------------------------------------------------------------------------*/

static void sin_gen_set_freq(uint8_t voice_index);
inline static void sin_gen_write_one_sample(uint32_t current_sample, int16_t value);

/*------------------------------------------------------------------------------------------------------------------------------*/

/* Basic notes array */
const static float note_freq[12] =
{
    [note_C]    = 16.35f,
    [note_Db]   = 17.32f,
    [note_D]    = 18.35f,
    [note_Eb]   = 19.45f,
    [note_E]    = 20.60f,
    [note_F]    = 21.83f,
    [note_Gb]   = 23.12f,
    [note_G]    = 24.50f,
    [note_Ab]   = 25.96f,
    [note_A]    = 27.50f,
    [note_Bb]   = 29.14f,
    [note_B]    = 30.87f
};

/*------------------------------------------------------------------------------------------------------------------------------*/

void sin_gen_init(void)
{
    ctx.table_ptr = ctx.table;

    HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*)ctx.table_ptr, PACKED_SIZE * 2);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

#define SIN_VALUE 32767.0 * (double)arm_sin_f32(6.28f * (float)ctx.voices_tab[i].freq * current_sample / 48000.0)
//#define SIN_VALUE (double)wavetable[ctx.voices_tab[i].sample_multiple]

void sin_gen_process(void)
{
    uint32_t current_sample = 0;

    double attack_coef = 1.0 / (double)eg_ctx.attack_time;
    double decay_coef = ((double)eg_ctx.sustain_level / 100.0 - 1.0) / (double)eg_ctx.decay_time;
    double sustain_coef = (double)eg_ctx.sustain_level / 100.0;
    double release_coef = -((double)eg_ctx.sustain_level / 100.0) / (double)eg_ctx.release_time;

    if (ctx.dma_flag)
    {
        /* Choose buffer that is not currently shipped */
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

        utility_TimeMeasurmentsSetHigh(); //todo delete this
        for (uint8_t i = 0; i < VOICES_COUNT; i++)
        {
            do
            {
                switch(ctx.voices_tab[i].voice_status)
                {
                case voice_status_Attack:
                    sin_gen_write_one_sample(current_sample, (int16_t)(attack_coef * (double)ctx.voices_tab[i].attack_counter * SIN_VALUE / (double)VOICES_COUNT));

                    if (ctx.voices_tab[i].attack_counter++ > eg_ctx.attack_time)
                    {
                        ctx.voices_tab[i].voice_status = voice_status_Decay;
                        ctx.voices_tab[i].attack_counter = 0U;
                    }
                    break;
                case voice_status_Decay:
                    sin_gen_write_one_sample(current_sample, (int16_t)((decay_coef * (double)ctx.voices_tab[i].decay_counter + 1.0) * SIN_VALUE / (double)VOICES_COUNT));

                    if (ctx.voices_tab[i].decay_counter++ > eg_ctx.decay_time)
                    {
                        ctx.voices_tab[i].voice_status = voice_status_Sustain;
                        ctx.voices_tab[i].decay_counter = 0U;
                    }
                    break;
                case voice_status_Sustain:
                    sin_gen_write_one_sample(current_sample, (int16_t)(sustain_coef * SIN_VALUE / (double)VOICES_COUNT));
                    break;
                case voice_status_Release:
                    sin_gen_write_one_sample(current_sample, (int16_t)((release_coef * (double)ctx.voices_tab[i].release_counter + (double)eg_ctx.sustain_level / 100.0) * SIN_VALUE / (double)VOICES_COUNT));

                    if (ctx.voices_tab[i].release_counter++ > eg_ctx.release_time)
                    {
                        ctx.voices_tab[i].voice_status = voice_status_Off;
                        ctx.voices_tab[i].release_counter = 0;
                    }
                    break;
                case voice_status_Off:
                    break;
                }
                    //ctx.table_ptr[currnet_sample] += 32767.0 * sin((6.28 * (double)ctx.voices_tab[i].freq * (double)current_sample) / 48000.0) / (double)VOICES_COUNT;
                /*release*/
//                    ctx.table_ptr[current_sample] += ((int32_t)(PACKED_SIZE - current_sample) * (int32_t)wavetable[ctx.voices_tab[i].sample_multiple] / PACKED_SIZE) / VOICES_COUNT;


                ctx.voices_tab[i].sample_multiple += ctx.voices_tab[i].freq;
                if (ctx.voices_tab[i].sample_multiple > (SAMPLE_COUNT - 1)) ctx.voices_tab[i].sample_multiple = ctx.voices_tab[i].sample_multiple % SAMPLE_COUNT;

                current_sample += 2;
            } while (0 != (PACKED_SIZE - current_sample));
            current_sample = 0;
        }

    }
    utility_TimeMeasurmentsSetLow(); //todo delete this

    ctx.dma_flag = false;
    ctx.buff_ready = true;
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
        else
        {
            utility_ErrLedOff();
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
        else
        {
            utility_ErrLedOff();
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
