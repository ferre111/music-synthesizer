/*
 * sin_gen.c
 *
 *  Created on: Jan 26, 2021
 *      Author: Karol Witusik
 */

#include <string.h>
#include <stdio.h>
#include "synth.h"
#include "utility.h"
#include "arm_math.h"
#include "i2s.h"
#include "wavetable.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

/* default envelop generator values */
#warning THIS DEFINES MUST BE THE SAME AS IN THE FILE envelope_generator_page.c
#define DEF_SUSTAIN_LEVEL   75U
#define DEF_ATTACK_TIME     250U
#define DEF_DECAY_TIME      100U
#define DEF_RELEASE_TIME    250U

#define NUMBER_OF_SAMPLES_IN_MILISECOND 48U

/* this value results from the mechanics of how the GUI works */
#define OSC_OFFSET_COEF 2U

/*------------------------------------------------------------------------------------------------------------------------------*/

/* array with "voices", one voice - one wave (e.g. sine) on the output */
static synth_voice voices_tab[VOICE_COUNT] =
{
        [0] = {.voice_status = VOICE_STATUS_OFF},
        [1] = {.voice_status = VOICE_STATUS_OFF},
        [2] = {.voice_status = VOICE_STATUS_OFF},
        [3] = {.voice_status = VOICE_STATUS_OFF},
        [4] = {.voice_status = VOICE_STATUS_OFF},
        [5] = {.voice_status = VOICE_STATUS_OFF},
        [6] = {.voice_status = VOICE_STATUS_OFF},
        [7] = {.voice_status = VOICE_STATUS_OFF},
        [8] = {.voice_status = VOICE_STATUS_OFF},
        [9] = {.voice_status = VOICE_STATUS_OFF}
};

/* Basic notes array */
const static double note_freq[12] =
{
    [NOTE_C]    = 16.35,
    [NOTE_Db]   = 17.32,
    [NOTE_D]    = 18.35,
    [NOTE_Eb]   = 19.45,
    [NOTE_E]    = 20.60,
    [NOTE_F]    = 21.83,
    [NOTE_Gb]   = 23.12,
    [NOTE_G]    = 24.50,
    [NOTE_Ab]   = 25.96,
    [NOTE_A]    = 27.50,
    [NOTE_Bb]   = 29.14,
    [NOTE_B]    = 30.87
};

/*------------------------------------------------------------------------------------------------------------------------------*/

static synth ctx __attribute((section(".synth_ctx"))) = {.voices_tab = voices_tab, .dma_flag = true, .type_of_synth = TYPES_OF_SYNTH_IIR};
static synth_envelop_generator eg_ctx;

/*------------------------------------------------------------------------------------------------------------------------------*/

static void synth_set_freq(synth_voice* sin_gen_voice);
static void synth_update_envelop_generator_coef_update(void);
static double synth_envelope_generator(int16_t val, synth_voice* sin_gen_voice);
static int16_t synth_velocity_and_voice_count(double val, synth_voice* sin_gen_voice, uint8_t osc_number);
inline static void synth_write_one_sample(uint32_t current_sample, int16_t value);
static void synth_wavetable_synthesis(synth_voice* sin_gen_voice, uint8_t osc_number);
static void synth_IIR_synthesis(synth_voice* sin_gen_voice, uint8_t osc_number);
static void synth_FM_synthesis(synth_voice* sin_gen_voice, uint8_t osc_number);

/*------------------------------------------------------------------------------------------------------------------------------*/

void Synth_init(void)
{
    ctx.table_ptr = ctx.table;

    Synth_set_envelop_generator(DEF_SUSTAIN_LEVEL, DEF_ATTACK_TIME, DEF_DECAY_TIME, DEF_RELEASE_TIME);
    HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*)ctx.table, PACKED_SIZE * 2U);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Synth_process(void)
 {
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
#pragma GCC unroll 10
        for (uint8_t voice = 0U; voice < VOICE_COUNT; voice++)
        {
            /* if voice status is different than Off then fill table */
            if (VOICE_STATUS_OFF != ctx.voices_tab[voice].voice_status)
            {
                for (uint8_t osc = 0U; osc < OSCILLATOR_COUNTS; osc++)
                {
                    if (ctx.osc[osc].activated)
                    {
                        switch(ctx.type_of_synth)
                        {
                        case TYPES_OF_SYNTH_WAVETABLE:
                            synth_wavetable_synthesis(&ctx.voices_tab[voice], osc);
                            break;
                        case TYPES_OF_SYNTH_IIR:
                            synth_IIR_synthesis(&ctx.voices_tab[voice], osc);
                            break;
                        case TYPES_OF_SYNTH_FM:
                            synth_FM_synthesis(&ctx.voices_tab[voice], osc);
                            break;
                        }
                    }
                }
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

void Synth_set_oscillator(uint8_t oscillator, uint8_t activated, wavetable_shape shape, uint8_t octave_offset, uint16_t phase, uint8_t volume)
{
    static bool prev_activated[OSCILLATOR_COUNTS];
    static wavetable_shape prev_shape[OSCILLATOR_COUNTS];

    prev_activated[oscillator] = ctx.osc[oscillator].activated;
    prev_shape[oscillator] = ctx.osc[oscillator].shape;

    ctx.osc[oscillator].activated = activated;
    ctx.osc[oscillator].shape = shape;
    ctx.osc[oscillator].octave_offset = octave_offset;
    ctx.osc[oscillator].phase = phase;
    ctx.osc[oscillator].volume = volume;

    for (uint32_t voice = 0U; voice < VOICE_COUNT; voice++)
    {
        if (ctx.voices_tab[voice].voice_status != VOICE_STATUS_OFF)
        {
            /* recalculate voice frequencies (they could be different due to octave offset changes) */
            synth_set_freq(&ctx.voices_tab[voice]);
        }
        /* set current sample to ensure correct phase between oscillators */
        ctx.voices_tab[voice].current_sample[oscillator] = (ctx.voices_tab[voice].current_sample[0] + ctx.osc[oscillator].phase * SAMPLE_COUNT / 360U) % SAMPLE_COUNT;
    }

    /* if oscillator shape change then load new wavetable */
    if (prev_activated[oscillator] != ctx.osc[oscillator].activated || prev_shape[oscillator] != ctx.osc[oscillator].shape)
    {
        Wavetable_load_new_wavetable(ctx.osc);
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Synth_set_envelop_generator(uint8_t sustain_level, uint32_t attack_time, uint32_t decay_time, uint32_t release_time)
{
    eg_ctx.sustain_level = sustain_level;
    eg_ctx.attack_time = attack_time * NUMBER_OF_SAMPLES_IN_MILISECOND;
    eg_ctx.decay_time = decay_time * NUMBER_OF_SAMPLES_IN_MILISECOND;
    eg_ctx.release_time = release_time * NUMBER_OF_SAMPLES_IN_MILISECOND;

    synth_update_envelop_generator_coef_update();
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Synth_set_voice_start_play(uint8_t key_number, uint8_t velocity)
{
    /* go through all voices */
    for (uint8_t i = 0U; i < VOICE_COUNT; i++)
    {
        /* search for a free voice */
        if (VOICE_STATUS_OFF == ctx.voices_tab[i].voice_status)
        {
            /* set voice status as Attack */
            ctx.voices_tab[i].voice_status = VOICE_STATUS_ATTACK;
            /* set key number */
            ctx.voices_tab[i].key_number = key_number;
            /* set velocity */
            ctx.voices_tab[i].velocity = velocity;
            /* set other required variables */
            synth_set_freq(&ctx.voices_tab[i]);
            /* if currently synthesis method is based on IIR filter then calculate required coefficients */
            if (TYPES_OF_SYNTH_IIR == ctx.type_of_synth)
            {
                IIR_generator_compute_coef(&ctx.voices_tab[i].IIR_generator, ctx.voices_tab[i].freq[0]); //todo
            }
            /* return from function */
            return;
        }
    }
}
/*------------------------------------------------------------------------------------------------------------------------------*/

void Synth_set_voice_stop_play(uint8_t key_number)
{
    /* go through all voices */
    for (uint8_t i = 0U; i < VOICE_COUNT; i++)
    {
        /* search for a voice with same key number as function argument, voice status can't be Off or Release */
        if (ctx.voices_tab[i].key_number == key_number && VOICE_STATUS_OFF != ctx.voices_tab[i].voice_status && VOICE_STATUS_RELEASE != ctx.voices_tab[i].voice_status)
        {
            /* if voice status is Attack then... */
            if (VOICE_STATUS_ATTACK == ctx.voices_tab[i].voice_status)
            {
                /* get max amplitude level */
                eg_ctx.max_ampl_release_transition = eg_ctx.attack_coef * ctx.voices_tab[i].attack_counter;
                /* reset attack counter */
                ctx.voices_tab[i].attack_counter = 0U;
            }
            /* if voice status is Decay then... */
            else if (VOICE_STATUS_DECAY == ctx.voices_tab[i].voice_status)
            {
                /* get max amplitude level */
                eg_ctx.max_ampl_release_transition = eg_ctx.decay_coef * ctx.voices_tab[i].decay_counter + 1.0;
                /* reset decay counter */
                ctx.voices_tab[i].decay_counter = 0U;
            }
            /* if voice status is Sustain then... */
            else if (VOICE_STATUS_SUSTAIN == ctx.voices_tab[i].voice_status)
            {
                /* max amplitude level will be equal to sustain coefficient */
                eg_ctx.max_ampl_release_transition = eg_ctx.sustain_coef;
            }

            /* compute release coefficient */
            eg_ctx.release_coef = -(double)eg_ctx.max_ampl_release_transition / (double)eg_ctx.release_time;
            /* set voice status as Release */
            ctx.voices_tab[i].voice_status = VOICE_STATUS_RELEASE;

            /* return from function */
            return;
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void synth_set_freq(synth_voice* sin_gen_voice)
{
    uint8_t key_in_oct = 0U;
    int8_t octave = 0U;

    key_in_oct = sin_gen_voice->key_number % 12U;

    for (uint8_t osc = 0U; osc < OSCILLATOR_COUNTS; osc++)
    {
        octave = sin_gen_voice->key_number / 12U + ctx.osc[osc].octave_offset - OSC_OFFSET_COEF;

        if (0U > octave)
        {
            octave = 0U;
        }
        sin_gen_voice->freq[osc] = note_freq[key_in_oct] * pow(2U, octave);
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void synth_update_envelop_generator_coef_update(void)
{
    eg_ctx.attack_coef = 1.0 / (double)eg_ctx.attack_time;
    eg_ctx.decay_coef = ((double)eg_ctx.sustain_level / 100.0 - 1.0) / (double)eg_ctx.decay_time;
    eg_ctx.sustain_coef = (double)eg_ctx.sustain_level / 100.0;
    eg_ctx.max_ampl_release_transition = eg_ctx.sustain_coef;
    eg_ctx.release_coef = -(double)eg_ctx.max_ampl_release_transition / (double)eg_ctx.release_time;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static double synth_envelope_generator(int16_t val, synth_voice* voice)
{
    double new_val = 0.0;

    switch(voice->voice_status)
    {
        case VOICE_STATUS_ATTACK:
            new_val = eg_ctx.attack_coef * (double)voice->attack_counter * (double)val;

            if (++voice->attack_counter == eg_ctx.attack_time)
            {
                voice->voice_status = VOICE_STATUS_DECAY;
                voice->attack_counter = 0U;
            }
            break;
        case VOICE_STATUS_DECAY:
            new_val = (eg_ctx.decay_coef * (double)voice->decay_counter + 1.0) * (double)val;

            if (++voice->decay_counter == eg_ctx.decay_time)
            {
                voice->voice_status = VOICE_STATUS_SUSTAIN;
                voice->decay_counter = 0U;
            }
            break;
        case VOICE_STATUS_SUSTAIN:
            new_val = eg_ctx.sustain_coef * (double)val;
            break;
        case VOICE_STATUS_RELEASE:
            new_val = (eg_ctx.release_coef * (double)voice->release_counter + eg_ctx.max_ampl_release_transition) * (double)val;

            if (++voice->release_counter == eg_ctx.release_time)
            {
                voice->voice_status = VOICE_STATUS_OFF;
                voice->release_counter = 0U;
            }
            break;
        case VOICE_STATUS_OFF:
            break;
    }
    return new_val;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static int16_t synth_velocity_and_voice_count(double val, synth_voice* sin_gen_voice, uint8_t osc_number)
{
    int16_t new_val = 0.0;

    new_val = (int16_t)((val * (double)sin_gen_voice->velocity * (double)ctx.osc[osc_number].volume) /
              (double)(MAX_VELOCITY * MAX_OSCILLATOR_VOLUME));

    return new_val;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void synth_wavetable_synthesis(synth_voice* sin_gen_voice, uint8_t osc_number)
{
    /* fill all samples in table */
    for (uint32_t sample = 0U; table_current_sample < PACKED_SIZE; sample += 2U)
    {
        /* write sample after envelope generator edit it */
        synth_write_one_sample(sample,
                synth_velocity_and_voice_count(synth_envelope_generator(wavetable_ram[osc_number][sin_gen_voice->current_sample[osc_number]], sin_gen_voice), sin_gen_voice, osc_number));

        /* set next sample from wavetable */
        sin_gen_voice->current_sample[osc_number] += (uint32_t)sin_gen_voice->freq[osc_number];

        /* check if current sample variable is bigger than wavetable length */
        if (sin_gen_voice->current_sample[osc_number] >= SAMPLE_COUNT)
        {
            /* if so perform modulo */
            sin_gen_voice->current_sample[osc_number] = sin_gen_voice->current_sample[osc_number] % SAMPLE_COUNT;
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void synth_FM_synthesis(synth_voice* sin_gen_voice, uint8_t osc_number)
{
    /* fill all samples in table */
    for (uint32_t table_current_sample = 0U; table_current_sample < PACKED_SIZE; table_current_sample += 2U)
    {

    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void synth_IIR_synthesis(synth_voice* sin_gen_voice, uint8_t osc_number)
{
    for (uint32_t i = 0U; i < PACKED_SIZE; i += 2U)
    {
        synth_write_one_sample(i, synth_velocity_and_voice_count(synth_envelope_generator(
                (int16_t)(32767U * IIR_generator_get_next_val(&sin_gen_voice->IIR_generator)), sin_gen_voice), sin_gen_voice, osc_number));
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/* Since it is monophonic devices write same data to both channel */
inline static void synth_write_one_sample(uint32_t current_sample, int16_t value)
{
    ctx.table_ptr[current_sample] += value;
    ctx.table_ptr[current_sample + 1U] += value;
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
