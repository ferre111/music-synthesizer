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

#define MAX_OSCILLATOR_VOLUME 100U
#define MAX_VELOCITY 255U

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
