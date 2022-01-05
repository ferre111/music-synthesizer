/*
 * DSP_basic_val.h
 *
 *  Created on: Dec 3, 2021
 *      Author: User
 */

#ifndef DSP_SRC_DSP_BASIC_VAL_H_
#define DSP_SRC_DSP_BASIC_VAL_H_

#define VOICE_COUNT 10U

/* size of data (in words) sends by one DMA transfer */
#define PACKED_SIZE 48U
#define OSCILLATOR_COUNTS 2U

/* wavetable sample count */
#define SAMPLE_COUNT 48000U

#define ACCUMULATOR_SIZE 4800000U
#define ACCUMULATOR_COEF 100U // ACCUMULATOR_SIZE / SAMPLE_COUNT

#define MAX_OSCILLATOR_VOLUME 100U
#define MAX_VELOCITY 127U
#define MAX_PITCH_BEND 16383U
#define CENTER_PITCH_BEND 8192U

#define FIRST_OSC   0U
#define SECOND_OSC  1U

#define NUMBER_OF_SAMPLES_IN_MILISECOND 48U

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef enum wavetable_shape_T
{
    WAVETABLE_SHAPE_SINE,
    WAVETABLE_SHAPE_SQUARE,
    WAVETABLE_SHAPE_TRIANGLE,
    WAVETABLE_SHAPE_SAW,

    Wavetable_shape_end
} wavetable_shape;

/*------------------------------------------------------------------------------------------------------------------------------*/

#endif /* DSP_SRC_DSP_BASIC_VAL_H_ */
