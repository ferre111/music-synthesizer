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
#include "i2s.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

#define VOICES_COUNT 10

/* default envelop generator values */
/* WARNING! THIS DEFINES MUST BE THE SAME AS IN THE FILE envelope_generator_page.c*/
#define DEF_SUSTAIN_LEVEL   75U
#define DEF_ATTACK_TIME     250U
#define DEF_DECAY_TIME      100U
#define DEF_RELEASE_TIME    250U

#define NUMBER_OF_SAMPLES_IN_MILISECOND 48U

/*------------------------------------------------------------------------------------------------------------------------------*/

/* array with "voices", one voice - one wave (e.g. sine) on the output */
static sin_gen_voice voices_tab[VOICES_COUNT] =
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
static int16_t sin_gen_envelope_generator(int16_t val, sin_gen_voice* sin_gen_voice);
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
    uint32_t table_current_sample = 0U;

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

        /* go through all voices */
        for (uint8_t i = 0U; i < VOICES_COUNT; i++)
        {
            /* if voice status is different than Off then fill table */
            if (voice_status_Off != ctx.voices_tab[i].voice_status)
            {
                /* fill all samples in table */
                do
                {
                    /* write sample after envelope generator edit it */
                    sin_gen_write_one_sample(table_current_sample, sin_gen_envelope_generator(wavetable[ctx.voices_tab[i].current_sample], &ctx.voices_tab[i]) / VOICES_COUNT);

                    /* set next sample from wavetable */
                    ctx.voices_tab[i].current_sample += ctx.voices_tab[i].freq;

                    /* check if current sample variable is bigger than wavetable length */
                    if (ctx.voices_tab[i].current_sample > (SAMPLE_COUNT - 1U))
                    {
                        /* if yes perform modulo */
                        ctx.voices_tab[i].current_sample = ctx.voices_tab[i].current_sample % SAMPLE_COUNT;
                    }

                    /* in one iteration two samples are write to table */
                    table_current_sample += 2U;
                } while (0U != (PACKED_SIZE - table_current_sample));
                table_current_sample = 0U;
            }
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
    eg_ctx.attack_time = attack_time * NUMBER_OF_SAMPLES_IN_MILISECOND;
    eg_ctx.decay_time = decay_time * NUMBER_OF_SAMPLES_IN_MILISECOND;
    eg_ctx.release_time = release_time * NUMBER_OF_SAMPLES_IN_MILISECOND;

    sin_gen_update_envelop_generator_coef_update();
}

/*------------------------------------------------------------------------------------------------------------------------------*/


void sin_gen_set_voice_start_play(uint8_t key_number)
{
    /* go through all voices */
    for (uint8_t i = 0; i < VOICES_COUNT; i++)
    {
        /* search for a free voice */
        if (voice_status_Off == ctx.voices_tab[i].voice_status)
        {
            /* set voice status as Attack */
            ctx.voices_tab[i].voice_status = voice_status_Attack;
            /* set key number */
            ctx.voices_tab[i].key_number = key_number;
            /* set other required variables */
            sin_gen_set_freq(i);

            /* return from function */
            return;
        }
    }
}
/*------------------------------------------------------------------------------------------------------------------------------*/

void sin_gen_set_voice_stop_play(uint8_t key_number)
{
    /* go through all voices */
    for (uint8_t i = 0; i < VOICES_COUNT; i++)
    {
        /* search for a voice with same key number as function argument, voice status can't be Off or Release */
        if (ctx.voices_tab[i].key_number == key_number && voice_status_Off != ctx.voices_tab[i].voice_status && voice_status_Release != ctx.voices_tab[i].voice_status)
        {
            /* if voice status is Attack then... */
            if (voice_status_Attack == ctx.voices_tab[i].voice_status)
            {
                /* get max amplitude level */
                eg_ctx.max_ampl_release_transition = eg_ctx.attack_coef * ctx.voices_tab[i].attack_counter;
                /* reset attack counter */
                ctx.voices_tab[i].attack_counter = 0U;
            }
            /* if voice status is Decay then... */
            else if (voice_status_Decay == ctx.voices_tab[i].voice_status)
            {
                /* get max amplitude level */
                eg_ctx.max_ampl_release_transition = eg_ctx.decay_coef * ctx.voices_tab[i].decay_counter + 1.0;
                /* reset decay counter */
                ctx.voices_tab[i].decay_counter = 0U;
            }
            /* if voice status is Sustain then... */
            else if (voice_status_Sustain == ctx.voices_tab[i].voice_status)
            {
                /* max amplitude level will be equal to sustain coefficient */
                eg_ctx.max_ampl_release_transition = eg_ctx.sustain_coef;
            }

            /* compute release coefficient */
            eg_ctx.release_coef = -(double)eg_ctx.max_ampl_release_transition / (double)eg_ctx.release_time;
            /* set voice status as Release */
            ctx.voices_tab[i].voice_status = voice_status_Release;

            /* return from function */
            return;
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void sin_gen_set_freq(uint8_t voice_index)
{
    uint8_t key_in_oct = ctx.voices_tab[voice_index].key_number % 12;
    uint8_t octave = ctx.voices_tab[voice_index].key_number / 12;
    ctx.voices_tab[voice_index].freq = note_freq[key_in_oct] * pow(2, octave);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void sin_gen_update_envelop_generator_coef_update(void)
{
    eg_ctx.attack_coef = 1.0 / (double)eg_ctx.attack_time;
    eg_ctx.decay_coef = ((double)eg_ctx.sustain_level / 100.0 - 1.0) / (double)eg_ctx.decay_time;
    eg_ctx.sustain_coef = (double)eg_ctx.sustain_level / 100.0;
    eg_ctx.max_ampl_release_transition = eg_ctx.sustain_coef;
    eg_ctx.release_coef = -(double)eg_ctx.max_ampl_release_transition / (double)eg_ctx.release_time;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static int16_t sin_gen_envelope_generator(int16_t val, sin_gen_voice* sin_gen_voice)
{
    switch(sin_gen_voice->voice_status)
    {
        case voice_status_Attack:
            val = (int16_t)(eg_ctx.attack_coef * (double)sin_gen_voice->attack_counter * (double)val);

            if (++sin_gen_voice->attack_counter == eg_ctx.attack_time)
            {
                sin_gen_voice->voice_status = voice_status_Decay;
                sin_gen_voice->attack_counter = 0U;
            }
            break;
        case voice_status_Decay:
            val = (int16_t)((eg_ctx.decay_coef * (double)sin_gen_voice->decay_counter + 1.0) * (double)val);

            if (++sin_gen_voice->decay_counter == eg_ctx.decay_time)
            {
                sin_gen_voice->voice_status = voice_status_Sustain;
                sin_gen_voice->decay_counter = 0U;
            }
            break;
        case voice_status_Sustain:
            val = (int16_t)(eg_ctx.sustain_coef * (double)val);
            break;
        case voice_status_Release:
            val = (int16_t)((eg_ctx.release_coef * (double)sin_gen_voice->release_counter + eg_ctx.max_ampl_release_transition) * (double)val);

            if (++sin_gen_voice->release_counter == eg_ctx.release_time)
            {
                sin_gen_voice->voice_status = voice_status_Off;
                sin_gen_voice->release_counter = 0U;
            }
            break;
        case voice_status_Off:
            break;
    }
    return val;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/* Since it is monophonic devices write same data to both channel */
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
        utility_ErrLedOn();
    }
}
