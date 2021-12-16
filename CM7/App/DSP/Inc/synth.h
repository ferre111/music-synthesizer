/*
 * sin_gen.h
 *
 *  Created on: Jan 26, 2021
 *      Author: Karol Witusik
 */

#ifndef __SIN_GEN_H
#define __SIN_GEN_H

#include <stdbool.h>
#include "main.h"
#include "IIR_generator.h"
#include "DSP_basic_val.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef enum note_T
{
    NOTE_C,
    NOTE_Db,
    NOTE_D,
    NOTE_Eb,
    NOTE_E,
    NOTE_F,
    NOTE_Gb,
    NOTE_G,
    NOTE_Ab,
    NOTE_A,
    NOTE_Bb,
    NOTE_B
} note;

typedef enum voice_status_T
{
    VOICE_STATUS_ATTACK,
    VOICE_STATUS_DECAY,
    VOICE_STATUS_SUSTAIN,
    VOICE_STATUS_RELEASE,
    VOICE_STATUS_OFF
} voice_status;

typedef enum types_of_synth_T
{
    TYPES_OF_SYNTH_WAVETABLE,
    TYPES_OF_SYNTH_IIR,
    TYPES_OF_SYNTH_FM
} types_of_synth;

typedef enum frequency_mode_T
{
    FREQUENCY_MODE_INDEPENDENT,
    FREQUENCY_MODE_DEPENDENT_X025,
    FREQUENCY_MODE_DEPENDENT_X033,
    FREQUENCY_MODE_DEPENDENT_X05,
    FREQUENCY_MODE_DEPENDENT_X1,
    FREQUENCY_MODE_DEPENDENT_X2,
    FREQUENCY_MODE_DEPENDENT_X3,
    FREQUENCY_MODE_DEPENDENT_X4,

    Dependent_frequency_end
} frequency_mode;

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef struct synth_voice_T
{
    voice_status voice_status;
    double freq[OSCILLATOR_COUNTS];
    int32_t current_sample[OSCILLATOR_COUNTS];
    uint8_t key_number;
    uint8_t velocity;

    uint32_t attack_counter;
    uint32_t decay_counter;
    uint32_t release_counter;

    IIR_generator IIR_generator;

    /* modulation index after scaling, parameter associated with FM modulation */
    double scaled_modulation_index;
    /* determines how much to increase current sample variable related with modulator oscillator, parameter associated with FM modulation */
    uint16_t modulator_increaser;
} synth_voice;

typedef struct synth_oscillator_T
{
    /* determines whether oscillator is active */
    uint8_t activated;
    /* determines oscillator shape */
    wavetable_shape shape;
    /* determines octave offset */
    uint8_t octave_offset;
    /* determines phase relative to first oscillator */
    uint16_t phase;
    /* determines oscillator volume */
    uint8_t volume;
} synth_oscillator;

typedef struct synth_oscillators_fm_T
{
    /* determines carrier oscillator shape */
    wavetable_shape shape_carrier_osc;
    /* determines modulator oscillator shape */
    wavetable_shape shape_modulator_osc;
    /* determines modulation index */
    double modulation_index;
    /* determines frequency mode */
    frequency_mode freq_mode;
    /* determines modulator frequency */
    uint16_t freq;
    /* determines oscillator volume */
    uint8_t volume;

} synth_oscillators_fm;

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

typedef struct synth_envelop_generator_T
{
    /* sustain level, 100 - 100%, 50 - 50% */
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
} synth_envelop_generator;

typedef struct synth_T
{
    /* if this flag is set sin_gen_process should fill buffer */
    volatile bool dma_flag;
    /* make one "double" array, with this we can use DMA in circular mode, and it is not required to again set transmission when whole buffer has been sent */
    int16_t table[PACKED_SIZE * 2];
    /* pointer to currently edited "virtual" buffer */
    int16_t *table_ptr;
    /* pointer to array with structures specifying actual playing waves */
    synth_voice *voices_tab;
    /* with this flag we can check whether current "virtual" buffer has been filled */
    volatile bool buff_ready;
    /* array with structure describing oscillators */
    synth_oscillator osc[OSCILLATOR_COUNTS];
    /* array with structure describing oscillators for FM synthesis */
    synth_oscillators_fm osc_fm;
    /* determines type of actual synthesis method */
    types_of_synth type_of_synth;
    /* determines pitch bend value */
    uint16_t pitch_bend_val;
} synth;


/*------------------------------------------------------------------------------------------------------------------------------*/

void Synth_init(void);
void Synth_process(void);
void Synth_set_voice_start_play(uint8_t key_number, uint8_t velocity);
void Synth_set_voice_stop_play(uint8_t key_number);
void synth_pitch_bend_change(uint16_t pitch_bend);
void Synth_set_oscillator(uint8_t oscillator, uint8_t activated, wavetable_shape shape, uint8_t octave_offset, uint16_t phase, uint8_t volume);
void Synth_set_FM_oscillator(wavetable_shape shape_carrier_osc, wavetable_shape shape_modulator_osc, uint16_t modulation_index, frequency_mode freq_mode, uint16_t freq, uint8_t volume);
void Synth_set_envelop_generator(uint8_t sustain_level, uint32_t attack_time, uint32_t decay_time, uint32_t release_time);

#endif
